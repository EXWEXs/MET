////////////////////////////////////////////////////////////////////////
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "tc_columns.h"
#include "track_point.h"
#include "track_info.h"

#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

int get_tc_col_offset(const char **arr, int n_cols, const char *col_name) {
   int i, offset;
   bool found = false;

   // Search the TC header columns first
   for(i=0; i<n_tc_header_cols; i++) {
      if(strcasecmp(tc_header_cols[i], col_name) == 0) {
         found  = true;
         offset = i;
         break;
      }
   }

   // If not found, search the columns provided
   if(!found) {
      for(i=0; i<n_cols; i++) {

         if(strcasecmp(arr[i], col_name) == 0) {
            found  = true;
            offset = i+n_tc_header_cols;
            break;
         }
      }
   }

   if(!found) {
      mlog << Error
           << "\nget_tc_col_offset() -> "
           << "no match found in the indicated array for the column name "
           << "specified: \"" << col_name << "\"\n\n";
      exit(1);
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

int get_tc_mpr_col_offset(const char *col_name) {
   int i;
   ConcatString s;
   int offset = bad_data_int;

   // Search the TC header columns first
   for(i=0; i<n_tc_header_cols; i++) {
      if(strcasecmp(tc_header_cols[i], col_name) == 0) {
         offset = i;
         break;
      }
   }

   // If not found, search the TC MPR columns:
   //    TOTAL,     INDEX,
   //    LEVEL,     WATCH_WARN, INITIALS,
   //    ALAT,      ALON,
   //    BLAT,      BLON,
   //    TK_ERR,    X_ERR,      Y_ERR,
   //    ALTK_ERR,  CRTK_ERR,
   //    ADLAND,    BDLAND,
   //    AMSLP,     BMSLP,
   //    AMAX_WIND, BMAX_WIND,
   //    AAL_WIND_34/50/64, BAL_WIND_34/50/64,
   //    ANE_WIND_34/50/64, BNE_WIND_34/50/64,
   //    ASE_WIND_34/50/64, BSE_WIND_34/50/64,
   //    ASW_WIND_34/50/64, BSW_WIND_34/50/64,
   //    ANW_WIND_34/50/64, BNW_WIND_34/50/64,
   //    ARADP,     BRADP,
   //    ARRP,      BRRP,
   //    AMRD,      BMRD,
   //    AGUSTS,    BGUSTS,
   //    AEYE,      BEYE,
   //    ADIR,      BDIR,
   //    ASPEED,    BSPEED,
   //    ADEPTH,    BDEPTH

   if(is_bad_data(offset)) {

      // Loop through the TCMPR columns looking for a match
      for(i=0; i<n_tc_mpr_cols; i++) {

         // Check for a match
         if(strcasecmp(tc_mpr_cols[i], col_name) == 0) {
            offset = n_tc_header_cols + i;
            break;
         }
      }
   }

   return(offset);
}

////////////////////////////////////////////////////////////////////////

void open_tc_txt_file(ofstream *&out, const char *file_name) {

   // Create and open the output file stream
   out = new ofstream;
   out->open(file_name);

   if(!(*out)) {
      mlog << Error
           << "\nopen_tc_txt_file()-> "
           << "can't open the output file \"" << file_name
           << "\" for writing!\n\n";
      exit(1);
   }

   out->setf(ios::fixed);

   return;
}

////////////////////////////////////////////////////////////////////////

void close_tc_txt_file(ofstream *&out, const char *file_name) {

   // List the file being closed
   mlog << Debug(1)
        << "Output file: " << file_name << "\n";

   // Close the output file
   out->close();
   delete out;
   out = (ofstream *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_header_row(const char **cols, int n_cols, int hdr_flag,
                         AsciiTable &at, int r, int c) {
   int i;

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_tc_header_cols; i++)
         at.set_entry(r, i+c, tc_header_cols[i]);

      c += n_tc_header_cols;
   }

   // Write the columns names specific to this line type
   for(i=0; i<n_cols; i++)
      at.set_entry(r, i+c, cols[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_header_row(int hdr_flag, AsciiTable &at,
                             int r, int c) {
   int i;
   ConcatString s;

   // Write the header column names if requested
   if(hdr_flag) {
      for(i=0; i<n_tc_header_cols; i++)
         at.set_entry(r, c++, tc_header_cols[i]);
   }

   // Write the tc_mpr header columns
   for(i=0; i<n_tc_mpr_cols; i++) {
      at.set_entry(r, c++, tc_mpr_cols[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_row(TcHdrColumns &hdr, const TrackPairInfo &p,
                      AsciiTable &at, int &i_row) {
   int i;

   // TCMPR line type
   hdr.set_line_type("TCMPR");
   
   // Loop through the TrackPairInfo points
   for(i=0; i<p.n_points(); i++) {

      // Timing information

      // Initialization and lead time for the ADECK
      hdr.set_init(p.adeck().init());
      hdr.set_lead(p.adeck()[i].lead());

      // Valid time for the ADECK and/or BDECK
      hdr.set_valid(p.valid(i));
     
      // Write the header columns
      write_tc_header_cols(hdr, at, i_row);

      // Write the data columns
      write_tc_mpr_cols(p, i, at, i_row, n_tc_header_cols);

      // Increment the row counter
      i_row++;
   }
  
   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_header_cols(const TcHdrColumns &hdr,
                          AsciiTable &at, int r) {
   int c = 0;

   // Header columns:
   //    VERSION,      AMODEL,     BDECK_MODEL,
   //    STORM_ID,     BASIN,      CYCLONE,
   //    STORM_NAME,   INIT,       LEAD,
   //    VALID,        INIT_MASK,  VALID_MASK,
   //    LINE_TYPE

   at.set_entry(r, c++, met_version);
   at.set_entry(r, c++, hdr.adeck_model());
   at.set_entry(r, c++, hdr.bdeck_model());
   at.set_entry(r, c++, hdr.storm_id());
   at.set_entry(r, c++, hdr.basin());
   at.set_entry(r, c++, hdr.cyclone());
   if(!hdr.storm_name().empty()) at.set_entry(r, c++, hdr.storm_name());
   else                          at.set_entry(r, c++, na_str);
   if(hdr.init() > 0)            at.set_entry(r, c++, unix_to_yyyymmdd_hhmmss(hdr.init()));
   else                          at.set_entry(r, c++, na_str);
   if(!is_bad_data(hdr.lead()))  at.set_entry(r, c++, sec_to_hhmmss(hdr.lead()));
   else                          at.set_entry(r, c++, na_str);
   if(hdr.valid() > 0)           at.set_entry(r, c++, unix_to_yyyymmdd_hhmmss(hdr.valid()));
   else                          at.set_entry(r, c++, na_str);
   at.set_entry(r, c++, hdr.init_mask());
   at.set_entry(r, c++, hdr.valid_mask());
   at.set_entry(r, c++, hdr.line_type());

   return;
}

////////////////////////////////////////////////////////////////////////

void write_tc_mpr_cols(const TrackPairInfo &p, int i,
                       AsciiTable &at, int r, int c) {
   int j;

   // Write tc_mpr columns
   at.set_entry(r, c++, p.n_points());
   at.set_entry(r, c++, i+1);
   at.set_entry(r, c++, cyclonelevel_to_string(p.bdeck()[i].level()));
   at.set_entry(r, c++, watchwarntype_to_string(p.bdeck()[i].watch_warn()));
   if(p.adeck().initials().empty()) at.set_entry(r, c++, na_str);
   else                             at.set_entry(r, c++, p.adeck().initials());
   at.set_entry(r, c++, p.adeck()[i].lat());
   at.set_entry(r, c++, p.adeck()[i].lon());
   at.set_entry(r, c++, p.bdeck()[i].lat());
   at.set_entry(r, c++, p.bdeck()[i].lon());
   at.set_entry(r, c++, p.track_err(i));
   at.set_entry(r, c++, p.x_err(i));
   at.set_entry(r, c++, p.y_err(i));
   at.set_entry(r, c++, p.along_track_err(i));
   at.set_entry(r, c++, p.cross_track_err(i));
   at.set_entry(r, c++, p.adeck_dland(i));
   at.set_entry(r, c++, p.bdeck_dland(i));
   at.set_entry(r, c++, p.adeck()[i].mslp());
   at.set_entry(r, c++, p.bdeck()[i].mslp());
   at.set_entry(r, c++, p.adeck()[i].v_max());
   at.set_entry(r, c++, p.bdeck()[i].v_max());

   // Write the wind columns
   for(j=0; j<NWinds; j++) {
      at.set_entry(r, c++, p.adeck()[i][j].al_val());
      at.set_entry(r, c++, p.bdeck()[i][j].al_val());
      at.set_entry(r, c++, p.adeck()[i][j].ne_val());
      at.set_entry(r, c++, p.bdeck()[i][j].ne_val());
      at.set_entry(r, c++, p.adeck()[i][j].se_val());
      at.set_entry(r, c++, p.bdeck()[i][j].se_val());
      at.set_entry(r, c++, p.adeck()[i][j].sw_val());
      at.set_entry(r, c++, p.bdeck()[i][j].sw_val());
      at.set_entry(r, c++, p.adeck()[i][j].nw_val());
      at.set_entry(r, c++, p.bdeck()[i][j].nw_val());
   } // end for j

   // Write remaining columns
   at.set_entry(r, c++, p.adeck()[i].radp());
   at.set_entry(r, c++, p.bdeck()[i].radp());
   at.set_entry(r, c++, p.adeck()[i].rrp());
   at.set_entry(r, c++, p.bdeck()[i].rrp());
   at.set_entry(r, c++, p.adeck()[i].mrd());
   at.set_entry(r, c++, p.bdeck()[i].mrd());
   at.set_entry(r, c++, p.adeck()[i].gusts());
   at.set_entry(r, c++, p.bdeck()[i].gusts());
   at.set_entry(r, c++, p.adeck()[i].eye());
   at.set_entry(r, c++, p.bdeck()[i].eye());
   at.set_entry(r, c++, p.adeck()[i].direction());
   at.set_entry(r, c++, p.bdeck()[i].direction());
   at.set_entry(r, c++, p.adeck()[i].speed());
   at.set_entry(r, c++, p.bdeck()[i].speed());
   at.set_entry(r, c++, systemsdepth_to_string(p.adeck()[i].depth()));
   at.set_entry(r, c++, systemsdepth_to_string(p.bdeck()[i].depth()));
   
   return;
}

////////////////////////////////////////////////////////////////////////

void justify_tc_stat_cols(AsciiTable &at) {

   justify_met_at(at, n_tc_header_cols);

   return;
}

////////////////////////////////////////////////////////////////////////
