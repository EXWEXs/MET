// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

#include "mode_conf_info.h"
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ModeConfInfo
//
////////////////////////////////////////////////////////////////////////

ModeConfInfo::ModeConfInfo()

{

   init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

ModeConfInfo::~ModeConfInfo()

{

   clear();
}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::init_from_scratch()

{

   // Initialize pointers
   fcst_info = (VarInfo *) 0;
   obs_info  = (VarInfo *) 0;

   clear();

   return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::clear()

{

   // Initialize values

   model.clear();
   obtype.clear();

   regrid_info.clear();

   mask_missing_flag = FieldType_None;

   fcst_raw_thresh.clear();
   obs_raw_thresh.clear();

   fcst_conv_radius = bad_data_int;
   obs_conv_radius = bad_data_int;

   fcst_conv_thresh.clear();
   obs_conv_thresh.clear();

   fcst_vld_thresh = bad_data_double;
   obs_vld_thresh = bad_data_double;

   fcst_area_thresh.clear();
   obs_area_thresh.clear();

   fcst_inten_perc_value = bad_data_int;
   obs_inten_perc_value = bad_data_int;

   fcst_inten_perc_thresh.clear();
   obs_inten_perc_thresh.clear();

   fcst_merge_thresh.clear();
   obs_merge_thresh.clear();

   fcst_merge_flag = MergeType_None;
   obs_merge_flag = MergeType_None;

   match_flag = MatchType_None;

   max_centroid_dist = bad_data_double;

   mask_grid_name.clear();
   mask_grid_flag = FieldType_None;

   mask_poly_name.clear();
   mask_poly_flag = FieldType_None;

   centroid_dist_wt    = bad_data_double;
   boundary_dist_wt    = bad_data_double;
   convex_hull_dist_wt = bad_data_double;
   angle_diff_wt       = bad_data_double;
   area_ratio_wt       = bad_data_double;
   int_area_ratio_wt   = bad_data_double;
   complexity_ratio_wt = bad_data_double;
   inten_perc_ratio_wt = bad_data_double;

   inten_perc_value = bad_data_int;

   centroid_dist_if    = (PiecewiseLinear *) 0;
   boundary_dist_if    = (PiecewiseLinear *) 0;
   convex_hull_dist_if = (PiecewiseLinear *) 0;
   angle_diff_if       = (PiecewiseLinear *) 0;
   area_ratio_if       = (PiecewiseLinear *) 0;
   int_area_ratio_if   = (PiecewiseLinear *) 0;
   complexity_ratio_if = (PiecewiseLinear *) 0;
   inten_perc_ratio_if = (PiecewiseLinear *) 0;

   total_interest_thresh = bad_data_double;

   print_interest_thresh = bad_data_double;

   met_data_dir.clear();

   zero_border_size = bad_data_int;

   plot_valid_flag = false;
   plot_gcarc_flag = false;
   ps_plot_flag    = false;

   nc_info.set_all_true();

   ct_stats_flag   = false;

   shift_right = 0;

   output_prefix.clear();

   version.clear();

   // Deallocate memory
   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) 0; }
   if(obs_info)  { delete obs_info;  obs_info  = (VarInfo *) 0; }

   return;

}

////////////////////////////////////////////////////////////////////////

void ModeConfInfo::read_config(const char *default_file_name, const char *user_file_name)

{

   // Read the config file constants
   conf.read(replace_path(config_const_filename));
   conf.read(replace_path(config_map_data_filename));

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;

}

////////////////////////////////////////////////////////////////////////


void ModeConfInfo::process_config(GrdFileType ftype, GrdFileType otype)

{

   VarInfoFactory info_factory;
   Dictionary *fcst_dict = (Dictionary *) 0;
   Dictionary *obs_dict  = (Dictionary *) 0;
   Dictionary *dict      = (Dictionary *) 0;
   PlotInfo plot_info;

      // Dump the contents of the config file

   if(mlog.verbosity_level() >= 5) conf.dump(cout);

      // Initialize

   clear();

      // Conf: version

   version = parse_conf_version(&conf);

      // Conf: model

   model = parse_conf_string(&conf, conf_key_model);

      // Conf: obtype

   obtype = parse_conf_string(&conf, conf_key_obtype);

      // Conf: regrid

   regrid_info = parse_conf_regrid(&conf);

      // Conf: fcst and obs

   fcst_dict = conf.lookup_dictionary(conf_key_fcst);
   obs_dict  = conf.lookup_dictionary(conf_key_obs);

      // Allocate new VarInfo objects

   fcst_info = info_factory.new_var_info(ftype);
   obs_info  = info_factory.new_var_info(otype);

      // Set the dictionaries

   fcst_info->set_dict(*(fcst_dict->lookup_dictionary(conf_key_field)));
   obs_info->set_dict(*(obs_dict->lookup_dictionary(conf_key_field)));

      // Dump the contents of the VarInfo objects

   if(mlog.verbosity_level() >= 5) {
      mlog << Debug(5)
           << "Parsed forecast field:\n";
      fcst_info->dump(cout);
      mlog << Debug(5)
           << "Parsed observation field:\n";
      obs_info->dump(cout);
   }

      // No support for wind direction

   if(fcst_info->is_wind_direction() || obs_info->is_wind_direction()) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "the wind direction field may not be verified "
           << "using MODE.\n\n";
      exit(1);
   }

      // Conf: fcst.raw_thresh and obs.raw_thresh

   fcst_raw_thresh = fcst_dict->lookup_thresh(conf_key_raw_thresh);
   obs_raw_thresh  = obs_dict->lookup_thresh(conf_key_raw_thresh);

      // Conf: fcst.conv_radius and obs.conv_radius

   fcst_conv_radius = nint(fcst_dict->lookup_double(conf_key_conv_radius));
   obs_conv_radius  = nint(obs_dict->lookup_double(conf_key_conv_radius));

      // Check that fcst_conv_radius and obs_conv_radius are non-negative

   if(fcst_conv_radius < 0 || obs_conv_radius < 0) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "fcst_conv_radius (" << fcst_conv_radius
           << ") and obs_conv_radius (" << obs_conv_radius
           << ") must be non-negative\n\n";
      exit(1);
   }

      // Conf: fcst.conv_thresh and obs.conv_thresh

   fcst_conv_thresh = fcst_dict->lookup_thresh(conf_key_conv_thresh);
   obs_conv_thresh  = obs_dict->lookup_thresh(conf_key_conv_thresh);

      // Conf: fcst.vld_thresh and obs.vld_thresh

   fcst_vld_thresh = fcst_dict->lookup_double(conf_key_vld_thresh);
   obs_vld_thresh  = obs_dict->lookup_double(conf_key_vld_thresh);

      // Conf: fcst.area_thresh and obs.area_thresh

   fcst_area_thresh = fcst_dict->lookup_thresh(conf_key_area_thresh);
   obs_area_thresh  = obs_dict->lookup_thresh(conf_key_area_thresh);

      // Conf: fcst.inten_perc and obs.inten_perc

   fcst_inten_perc_value = fcst_dict->lookup_int(conf_key_inten_perc_value);
   obs_inten_perc_value  = obs_dict->lookup_int(conf_key_inten_perc_value);

      // Conf: fcst.inten_perc_thresh and obs.inten_perc_thresh

   fcst_inten_perc_thresh = fcst_dict->lookup_thresh(conf_key_inten_perc_thresh);
   obs_inten_perc_thresh  = obs_dict->lookup_thresh(conf_key_inten_perc_thresh);

      // Conf: fcst.merge_thresh and obs.merge_thresh

   fcst_merge_thresh = fcst_dict->lookup_thresh(conf_key_merge_thresh);
   obs_merge_thresh  = obs_dict->lookup_thresh(conf_key_merge_thresh);

      // Conf: fcst.merge_flag and obs.merge_flag

   fcst_merge_flag = int_to_mergetype(fcst_dict->lookup_int(conf_key_merge_flag));
   obs_merge_flag  = int_to_mergetype(obs_dict->lookup_int(conf_key_merge_flag));

      // Conf: mask_missing_flag

   mask_missing_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_missing_flag));

      // Conf: match_flag

   match_flag = int_to_matchtype(conf.lookup_int(conf_key_match_flag));

      // Check that match_flag is set between 0 and 3

   if(match_flag == MatchType_None &&
      (fcst_merge_flag != MergeType_None || obs_merge_flag  != MergeType_None) ) {
      mlog << Warning << "\nModeConfInfo::process_config() -> "
           << "When matching is disabled (match_flag = "
           << matchtype_to_string(match_flag)
           << ") but merging is requested (fcst_merge_flag = "
           << mergetype_to_string(fcst_merge_flag)
           << ", obs_merge_flag = "
           << mergetype_to_string(obs_merge_flag)
           << ") any merging information will be discarded.\n\n";
   }

      // Conf: max_centroid_dist

   max_centroid_dist = conf.lookup_double(conf_key_max_centroid_dist);

      // Check that max_centroid_dist is > 0

   if(max_centroid_dist <= 0) {
      mlog << Warning << "\nModeConfInfo::process_config() -> "
           << "max_centroid_dist (" << max_centroid_dist
           << ") should be set > 0\n\n";
   }

      // Conf: mask.grid

   mask_grid_name = conf.lookup_string(conf_key_mask_grid);
   mask_grid_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_grid_flag));

      // Conf: mask.poly

   mask_poly_name = conf.lookup_string(conf_key_mask_poly);
   mask_poly_flag = int_to_fieldtype(conf.lookup_int(conf_key_mask_poly_flag));

      // Conf: weight

   dict = conf.lookup_dictionary(conf_key_weight);

      // Parse the weights

   centroid_dist_wt    = dict->lookup_double(conf_key_centroid_dist);
   boundary_dist_wt    = dict->lookup_double(conf_key_boundary_dist);
   convex_hull_dist_wt = dict->lookup_double(conf_key_convex_hull_dist);
   angle_diff_wt       = dict->lookup_double(conf_key_angle_diff);
   area_ratio_wt       = dict->lookup_double(conf_key_area_ratio);
   int_area_ratio_wt   = dict->lookup_double(conf_key_int_area_ratio);
   complexity_ratio_wt = dict->lookup_double(conf_key_complexity_ratio);
   int_area_ratio_wt   = dict->lookup_double(conf_key_int_area_ratio);
   inten_perc_ratio_wt = dict->lookup_double(conf_key_inten_perc_ratio);
   inten_perc_value    = dict->lookup_int(conf_key_inten_perc_value);

      // Check that intensity_percentile >= 0 and <= 100

   if(inten_perc_value < 0 || inten_perc_value > 100) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "inten_perc_value (" << inten_perc_value
           << ") must be set between 0 and 100.\n\n";
      exit(1);
   }

      // Check that the fuzzy engine weights are non-negative

   if(centroid_dist_wt    < 0 || boundary_dist_wt    < 0 ||
      convex_hull_dist_wt < 0 || angle_diff_wt       < 0 ||
      area_ratio_wt       < 0 || int_area_ratio_wt   < 0 ||
      complexity_ratio_wt < 0 || inten_perc_ratio_wt < 0) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "All of the fuzzy engine weights must be >= 0.\n\n";
      exit(1);
   }

      // Check that the sum of the weights is non-zero for matching

   if(match_flag != MatchType_None &&
      is_eq(centroid_dist_wt    + boundary_dist_wt   +
            convex_hull_dist_wt + angle_diff_wt      +
            area_ratio_wt       + int_area_ratio_wt  +
            complexity_ratio_wt + inten_perc_ratio_wt, 0.0)) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "When matching is requested, the sum of the fuzzy engine "
           << "weights cannot be 0\n\n";
      exit(1);
   }

      // Conf: interest_function

   dict = conf.lookup_dictionary(conf_key_interest_function);

      // Parse the interest functions

   centroid_dist_if    = dict->lookup_pwl(conf_key_centroid_dist);
   boundary_dist_if    = dict->lookup_pwl(conf_key_boundary_dist);
   convex_hull_dist_if = dict->lookup_pwl(conf_key_convex_hull_dist);
   angle_diff_if       = dict->lookup_pwl(conf_key_angle_diff);
   area_ratio_if       = dict->lookup_pwl(conf_key_area_ratio);
   int_area_ratio_if   = dict->lookup_pwl(conf_key_int_area_ratio);
   complexity_ratio_if = dict->lookup_pwl(conf_key_complexity_ratio);
   int_area_ratio_if   = dict->lookup_pwl(conf_key_int_area_ratio);
   inten_perc_ratio_if = dict->lookup_pwl(conf_key_inten_perc_ratio);

      // Conf: total_interest_thresh

   total_interest_thresh = conf.lookup_double(conf_key_total_interest_thresh);

      // Check that total_interest_thresh is between 0 and 1.

   if(total_interest_thresh < 0 || total_interest_thresh > 1) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "total_interest_thresh (" << total_interest_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

      // Conf: print_interest_thresh

   print_interest_thresh = conf.lookup_double(conf_key_print_interest_thresh);

      // Check that print_interest_thresh is between 0 and 1.

   if(print_interest_thresh < 0 || print_interest_thresh > 1) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "print_interest_thresh (" << print_interest_thresh
           << ") must be set between 0 and 1.\n\n";
      exit(1);
   }

      // Conf: met_data_dir

   met_data_dir = replace_path(conf.lookup_string(conf_key_met_data_dir));

      // Conf: fcst_raw_plot

   fcst_raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_fcst_raw_plot));

      // Conf: obs_raw_plot

   obs_raw_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_obs_raw_plot));

      // Conf: object_plot

   object_pi = parse_conf_plot_info(conf.lookup_dictionary(conf_key_object_plot));

      // Conf: zero_border_size

   zero_border_size = conf.lookup_int(conf_key_zero_border_size);

      // Check that zero_border_size >= 1

   if(zero_border_size < 1) {
      mlog << Error << "\nModeConfInfo::process_config() -> "
           << "zero_border_size (" << zero_border_size
           << ") must be set >= 1.\n\n";
      exit(1);
   }

      // Conf: plot_valid_flag

   plot_valid_flag = conf.lookup_bool(conf_key_plot_valid_flag);

      // Conf: plot_gcarc_flag

   plot_gcarc_flag = conf.lookup_bool(conf_key_plot_gcarc_flag);

      // Conf: ps_plot_flag

   ps_plot_flag = conf.lookup_bool(conf_key_ps_plot_flag);

      // Conf: nc_pairs_flag

   parse_nc_info();

      // Conf: ct_stats_flag

   ct_stats_flag = conf.lookup_bool(conf_key_ct_stats_flag);

      // Conf: output_prefix

   output_prefix = conf.lookup_string(conf_key_output_prefix);

      // Conf: shift_right

   shift_right = fcst_dict->lookup_int(conf_key_shift_right_value);

   return;

}


////////////////////////////////////////////////////////////////////////


void ModeConfInfo::parse_nc_info()

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = conf.lookup(conf_key_nc_pairs_flag);

if ( !e )  {

   mlog << Error
        << "\n\n  ModeConfInfo::parse_nc_info() -> lookup failed for key \""
        << conf_key_nc_pairs_flag << "\"\n\n";

   exit ( 1 );

}

const ConfigObjectType type = e->type();

if ( type == BooleanType )  {

   bool value = e->b_value();

   if ( ! value )  nc_info.set_all_false();

   return;

}

   //
   //  it should be a dictionary
   //

if ( type != DictionaryType )  {

   mlog << Error
        << "\n\n  ModeConfInfo::parse_nc_info() -> bad type ("
        << configobjecttype_to_string(type)
        << ") for key \""
        << conf_key_nc_pairs_flag << "\"\n\n";

   exit ( 1 );

}

   //
   //  parse the various entries
   //

Dictionary * d = e->dict_value();

nc_info.do_latlon     = d->lookup_bool(conf_key_latlon_flag);
nc_info.do_raw        = d->lookup_bool(conf_key_raw_flag);
nc_info.do_object_raw = d->lookup_bool(conf_key_object_raw_flag);
nc_info.do_object_id  = d->lookup_bool(conf_key_object_id_flag);
nc_info.do_cluster_id = d->lookup_bool(conf_key_cluster_id_flag);
nc_info.do_polylines  = d->lookup_bool(conf_key_polylines_flag);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct ModeNcOutInfo
   //


////////////////////////////////////////////////////////////////////////


ModeNcOutInfo::ModeNcOutInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutInfo::clear()

{

set_all_true();

return;

}


////////////////////////////////////////////////////////////////////////


bool ModeNcOutInfo::all_false() const

{

bool status = do_latlon || do_raw || do_object_raw || do_object_id || do_cluster_id || do_polylines;

return ( !status );

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutInfo::set_all_false()

{

do_latlon     = false;
do_raw        = false;
do_object_raw = false;
do_object_id  = false;
do_cluster_id = false;
do_polylines  = false;

return;

}


////////////////////////////////////////////////////////////////////////


void ModeNcOutInfo::set_all_true()

{

do_latlon     = true;
do_raw        = true;
do_object_raw = true;
do_object_id  = true;
do_cluster_id = true;
do_polylines  = true;

return;

}


////////////////////////////////////////////////////////////////////////



