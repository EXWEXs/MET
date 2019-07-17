// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "tc_gen_conf_info.h"

#include "vx_nc_util.h"
#include "apply_mask.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCGenVxOpt
//
////////////////////////////////////////////////////////////////////////

TCGenVxOpt::TCGenVxOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCGenVxOpt::~TCGenVxOpt() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCGenVxOpt::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenVxOpt::clear() {

   Desc.clear();
   Model.clear();
   BestTechnique.clear();
   OperTechnique.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   InitBeg = InitEnd = (unixtime) 0;
   InitInc.clear();
   InitExc.clear();
   ValidBeg = ValidEnd = (unixtime) 0;
   InitHour.clear();
   Lead.clear();
   VxMaskName.clear();
   VxPolyMask.clear();
   VxGridMask.clear();
   VxAreaMask.clear();
   DLandThresh.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenVxOpt::process_config(Dictionary &dict) {
   int i, j;
   Dictionary *dict2 = (Dictionary *) 0;
   ConcatString file_name;
   StringArray sa;

   // Conf: Desc
   Desc = parse_conf_string(&dict, conf_key_desc);

   // Conf: Model
   Model = dict.lookup_string_array(conf_key_model);

   // Conf: BestTechnique and OperTechnique
   BestTechnique = dict.lookup_string(conf_key_best_technique);
   OperTechnique = dict.lookup_string(conf_key_oper_technique);

   // Conf: StormId
   StormId = dict.lookup_string_array(conf_key_storm_id);

   // Conf: Basin
   Basin = dict.lookup_string_array(conf_key_basin);

   // Conf: Cyclone
   Cyclone = dict.lookup_string_array(conf_key_cyclone);

   // Conf: StormName
   StormName = dict.lookup_string_array(conf_key_storm_name);

   // Conf: InitBeg, InitEnd
   InitBeg = dict.lookup_unixtime(conf_key_init_beg);
   InitEnd = dict.lookup_unixtime(conf_key_init_end);

   // Conf: InitInc
   sa = dict.lookup_string_array(conf_key_init_inc);
   for(i=0; i<sa.n_elements(); i++)
      InitInc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: InitExc
   sa = dict.lookup_string_array(conf_key_init_exc);
   for(i=0; i<sa.n_elements(); i++)
      InitExc.add(timestring_to_unix(sa[i].c_str()));

   // Conf: ValidBeg, ValidEnd
   ValidBeg = dict.lookup_unixtime(conf_key_valid_beg);
   ValidEnd = dict.lookup_unixtime(conf_key_valid_end);

   // Conf: InitHour
   sa = dict.lookup_string_array(conf_key_init_hour);
   for(i=0; i<sa.n_elements(); i++) {
      InitHour.add(timestring_to_sec(sa[i].c_str()));
   }

   // Conf: Lead
   sa = dict.lookup_string_array(conf_key_lead);
   for(i=0; i<sa.n_elements(); i++) {
      Lead.add(timestring_to_sec(sa[i].c_str()));
   }

   // Conf: VxMask
   if(nonempty(dict.lookup_string(conf_key_vx_mask).c_str())) {
      file_name = replace_path(dict.lookup_string(conf_key_vx_mask));
      mlog << Debug(2) << "Masking File: " << file_name << "\n";
      parse_poly_mask(file_name, VxPolyMask, VxGridMask, VxAreaMask,
                      VxMaskName);
   }

   // Conf: DLandThresh
   DLandThresh = dict.lookup_thresh(conf_key_dland_thresh);

   // Conf: GenesisBeg and GenesisEnd
   int beg, end;
   dict2 = dict.lookup_dictionary(conf_key_genesis_window);
   parse_conf_range_int(dict2, beg, end);
   GenesisBeg = beg*sec_per_hour;
   GenesisEnd = end*sec_per_hour;

   // Conf: GenesisRadius
   GenesisRadius = dict.lookup_double(conf_key_genesis_radius);

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCGenVxOpt::is_keeper(const GenesisInfo &g) {
   bool keep = true;
   int m, d, y, h, mm, s;

   // Model, BestTechnique, and OperTechnique are checked elsewhere

   // Parse init time into components
   unix_to_mdyhms(g.init(), m, d, y, h, mm, s);

   // Check storm id
   if(StormId.n() > 0 &&
      !has_storm_id(StormId, g.basin(), g.cyclone(), g.init()))
      keep = false;

   // Check basin
   else if(Basin.n() > 0 && !Basin.has(g.basin()))
      keep = false;

   // Check cyclone
   else if(Cyclone.n() > 0 &&
           !Cyclone.has(g.cyclone()))
      keep = false;

   // Check storm name
   else if(StormName.n() > 0 &&
           !StormName.has(g.storm_name()))
      keep = false;

   // Initialization time window
   else if((InitBeg     > 0 &&  InitBeg > g.init())    ||
           (InitEnd     > 0 &&  InitEnd < g.init())    ||
           (InitInc.n() > 0 && !InitInc.has(g.init())) ||
           (InitExc.n() > 0 &&  InitExc.has(g.init())))
      keep = false;

   // Valid time window
   else if((ValidBeg > 0 && ValidBeg > g.valid_min()) ||
           (ValidEnd > 0 && ValidEnd < g.valid_max()))
      keep = false;

   // Initialization hours
   else if(InitHour.n() > 0 && !InitHour.has(hms_to_sec(h, mm, s)))
      keep = false;

   // Lead times
   else if(Lead.n() > 0 && !Lead.has(g.lead_time()))
      keep = false;

   // Poly masking
   else if(VxPolyMask.n_points() > 0 &&
           !VxPolyMask.latlon_is_inside(g.lat(), g.lon()))
      keep = false;

   // Area masking
   else if(!VxAreaMask.is_empty() ) {
      double x, y;
      VxGridMask.latlon_to_xy(g.lat(), g.lon(), x, y);
      if(x < 0 || x >= VxGridMask.nx() ||
         y < 0 || y >= VxGridMask.ny()) {
         keep = false;
      }
      else {
         keep = VxAreaMask(nint(x), nint(y));
      }
   }

   // Distance to land
   else if((DLandThresh.get_type() != no_thresh_type) &&
           (is_bad_data(g.dland()) || !DLandThresh.check(g.dland())))
      keep = false;

   // Return the keep status
   return(keep);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCGenConfInfo
//
////////////////////////////////////////////////////////////////////////

TCGenConfInfo::TCGenConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCGenConfInfo::~TCGenConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::clear() {

   for(size_t i=0; i<VxOpt.size(); i++) VxOpt[i].clear();
   InitFreq = bad_data_int;
   LeadBeg = bad_data_int;
   LeadEnd = bad_data_int;
   MinDur = bad_data_int;
   EventCategory.clear();
   EventVMaxThresh.clear();
   EventMSLPThresh.clear();
   DLandFile.clear();
   DLandGrid.clear();
   DLandData.clear();
   Version.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::read_config(const char *default_file_name,
                                const char *user_file_name) {

   // Read the config file constants
   Conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   Conf.read(default_file_name);

   // Read the user-specified config file
   Conf.read(user_file_name);

   // Process the configuration file
   process_config();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::process_config() {
   Dictionary *dict = (Dictionary *) 0;
   TCGenVxOpt vx_opt;
   StringArray sa;
   int i, beg, end;

   // Conf: init_freq
   InitFreq = Conf.lookup_int(conf_key_init_freq);

   // Conf: lead_window
   dict = Conf.lookup_dictionary(conf_key_lead_window);
   parse_conf_range_int(dict, beg, end);
   LeadBeg = beg*sec_per_hour;
   LeadEnd = end*sec_per_hour;

   // Conf: min_duration
   MinDur = Conf.lookup_int(conf_key_min_duration);

   // Conf: EventCategory
   sa = Conf.lookup_string_array(conf_key_event_category);
   for(i=0; i<sa.n(); i++) {
      EventCategory.push_back(string_to_cyclonelevel(sa[i].c_str()));
   }

   // Conf: EventVMaxThresh
   EventVMaxThresh = Conf.lookup_thresh(conf_key_event_vmax_thresh);

   // Conf: EventMSLPThresh
   EventMSLPThresh = Conf.lookup_thresh(conf_key_event_mslp_thresh);

   // Conf: DLandFile
   DLandFile = Conf.lookup_string(conf_key_dland_file);

   // Conf: Version
   Version = Conf.lookup_string(conf_key_version);
   check_met_version(Version.c_str());

   // Conf: Filter
   dict = Conf.lookup_array(conf_key_filter, false);

   // If no filters are specified, use the top-level settings
   if(dict->n_entries() == 0) {
      vx_opt.process_config(Conf);
      VxOpt.push_back(vx_opt);
   }
   // Loop through the array entries
   else {
      for(i=0; i<dict->n_entries(); i++) {
         vx_opt.clear();
         vx_opt.process_config(*((*dict)[i]->dict_value()));
         VxOpt.push_back(vx_opt);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

double TCGenConfInfo::compute_dland(double lat, double lon) {
   double x_dbl, y_dbl, dist;
   int x, y;

   // Load the distance to land data, if needed.
   if(DLandData.is_empty()) {
      load_dland(DLandFile, DLandGrid, DLandData);
   }

   // Convert lat,lon to x,y
   DLandGrid.latlon_to_xy(lat, lon, x_dbl, y_dbl);

   // Round to nearest int
   x = nint(x_dbl);
   y = nint(y_dbl);

   // Check range
   if(x < 0 || x >= DLandGrid.nx() ||
      y < 0 || y >= DLandGrid.ny())   dist = bad_data_double;
   else                               dist = DLandData.get(x, y);

   return(dist);
}

////////////////////////////////////////////////////////////////////////
