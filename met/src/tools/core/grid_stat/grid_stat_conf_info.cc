// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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

#include "grid_stat_conf_info.h"
#include "configobjecttype_to_string.h"

#include "vx_data2d_factory.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class GridStatConfInfo
//
////////////////////////////////////////////////////////////////////////

GridStatConfInfo::GridStatConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GridStatConfInfo::~GridStatConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::init_from_scratch() {

   // Initialize pointers
   fcst_info   = (VarInfo **)     0;
   obs_info    = (VarInfo **)     0;
   fqc_ta      = (ThreshArray *)  0;
   oqc_ta      = (ThreshArray *)  0;
   fqc_val     = (NumArray *)     0;
   oqc_val     = (NumArray *)     0;
   fcat_ta     = (ThreshArray *)  0;
   ocat_ta     = (ThreshArray *)  0;
   cnt_logic   = (SetLogic *)     0;
   fcnt_ta     = (ThreshArray *)  0;
   ocnt_ta     = (ThreshArray *)  0;
   fwind_ta    = (ThreshArray *)  0;
   owind_ta    = (ThreshArray *)  0;
   wind_logic  = (SetLogic *)     0;
   eclv_points = (NumArray *)     0;
   mask_dp     = (DataPlane *)    0;
   interp_mthd = (InterpMthd *)   0;
   ascii_output_flag = true;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::clear() {
   int i;

   // Set counts to zero
   n_vx_scal = 0;
   n_vx_vect = 0;
   n_vx_prob = 0;
   n_mask    = 0;
   n_interp  = 0;

   max_n_cat_thresh   = 0;
   max_n_cnt_thresh   = 0;
   max_n_wind_thresh  = 0;
   max_n_fprob_thresh = 0;
   max_n_oprob_thresh = 0;

   // Initialize values
   model.clear();
   obtype.clear();
   regrid_info.clear();
   desc.clear();
   climo_cdf_ta.clear();
   mask_name.clear();
   ci_alpha.clear();
   boot_interval = BootIntervalType_None;
   boot_rep_prop = bad_data_double;
   n_boot_rep = bad_data_int;
   boot_rng.clear();
   boot_seed.clear();
   interp_field = FieldType_None;
   interp_thresh = bad_data_double;
   interp_wdth.clear();
   nc_info.set_all_true();
   grid_weight_flag = GridWeightType_None;
   rank_corr_flag = false;
   tmp_dir.clear();
   output_prefix.clear();
   version.clear();
   ascii_output_flag = true;

   for(i=0; i<n_txt; i++) output_flag[i] = STATOutputType_None;

   // Deallocate memory
   if(fcat_ta)     { delete [] fcat_ta;     fcat_ta     = (ThreshArray *) 0; }
   if(ocat_ta)     { delete [] ocat_ta;     ocat_ta     = (ThreshArray *) 0; }

   if(fqc_ta)      { delete [] fqc_ta ;     fqc_ta      = (ThreshArray *) 0; }
   if(oqc_ta)      { delete [] oqc_ta ;     oqc_ta      = (ThreshArray *) 0; }
   if(fqc_val)     { delete [] fqc_val;     fqc_val     = (NumArray *)    0; }
   if(oqc_val)     { delete [] oqc_val;     oqc_val     = (NumArray *)    0; }

   if(fcnt_ta)     { delete [] fcnt_ta;     fcnt_ta     = (ThreshArray *) 0; }
   if(ocnt_ta)     { delete [] ocnt_ta;     ocnt_ta     = (ThreshArray *) 0; }
   if(cnt_logic)   { delete [] cnt_logic;   cnt_logic   = (SetLogic *)    0; }

   if(fwind_ta)    { delete [] fwind_ta;    fwind_ta    = (ThreshArray *) 0; }
   if(owind_ta)    { delete [] owind_ta;    owind_ta    = (ThreshArray *) 0; }
   if(wind_logic)  { delete [] wind_logic;  wind_logic  = (SetLogic *)    0; }

   if(eclv_points) { delete [] eclv_points; eclv_points = (NumArray *)    0; }

   if(mask_dp)     { delete [] mask_dp;     mask_dp     = (DataPlane *)   0; }
   if(interp_mthd) { delete [] interp_mthd; interp_mthd = (InterpMthd *)  0; }

   // Clear fcst_info
   if(fcst_info) {
      for(i=0; i<n_vx; i++)
         if(fcst_info[i]) { delete fcst_info[i]; fcst_info[i] = (VarInfo *) 0; }
      delete fcst_info; fcst_info = (VarInfo **) 0;
   }

   // Clear obs_info
   if(obs_info) {
      for(i=0; i<n_vx; i++)
         if(obs_info[i]) { delete obs_info[i]; obs_info[i] = (VarInfo *) 0; }
      delete obs_info; obs_info = (VarInfo **) 0;
   }

   // Reset count
   n_vx = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::read_config(const char *default_file_name,
                                   const char *user_file_name) {

   // Read the config file constants
   conf.read(replace_path(config_const_filename));

   // Read the default config file
   conf.read(default_file_name);

   // Read the user-specified config file
   conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::process_config(GrdFileType ftype, GrdFileType otype) {
   int i, n;
   ConcatString s;
   StringArray sa;
   VarInfoFactory info_factory;
   map<STATLineType,STATOutputType>output_map;
   Dictionary *fdict = (Dictionary *) 0;
   Dictionary *odict = (Dictionary *) 0;
   Dictionary i_fdict, i_odict;
   BootInfo boot_info;
   InterpInfo interp_info;
   NbrhdInfo nbrhd_info;

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

   // Conf: output_flag
   output_map = parse_conf_output_flag(&conf);

   // Make sure the output_flag is the expected size
   if((signed int) output_map.size() != n_txt) {
      mlog << Error << "\nGridStatConfInfo::process_config() -> "
           << "Unexpected number of entries found in \""
           << conf_key_output_flag << "\" ("
           << (signed int) output_map.size()
           << " != " << n_txt << ").\n\n";
      exit(1);
   }

   // Populate the output_flag array with map values
   for(i=0,n=0; i<n_txt; i++) {
      output_flag[i] = output_map[txt_file_type[i]];
      if(output_flag[i] != STATOutputType_None) n++;
   }

   // Check for at least one output line type
   if(n == 0) {
       // MET-620  Do NOT error out when no STAT type is requested.
       // Instead, generate the requested output file and do NOT
       // generate any ASCII output text files. Set the ascii_output_flat
       // to false, it is set to true by default.
       mlog <<Debug(3)
            <<"\nNo STAT type requested, proceeding with ASCII output flag to false.\n";
       set_ascii_output_flag(false);
   }

   // Conf: fcst.field and obs.field
   fdict = conf.lookup_array(conf_key_fcst_field);
   odict = conf.lookup_array(conf_key_obs_field);

   // Determine the number of fields (name/level) to be verified
   n_vx = parse_conf_n_vx(fdict);

   // Check for a valid number of verification tasks
   if(n_vx == 0 || parse_conf_n_vx(odict) != n_vx) {
      mlog << Error << "\nGridStatConfInfo::process_config() -> "
           << "The number of verification tasks in \""
           << conf_key_obs_field
           << "\" must be non-zero and match the number in \""
           << conf_key_fcst_field << "\".\n\n";
      exit(1);
   }

   // Check climatology fields
   check_climo_n_vx(&conf, n_vx);

   // Conf: climo_cdf_bins
   climo_cdf_ta = parse_conf_climo_cdf_bins(&conf);

   // Allocate space based on the number of verification tasks
   fcst_info   = new VarInfo *   [n_vx];
   obs_info    = new VarInfo *   [n_vx];
   fqc_ta      = new ThreshArray [n_vx];
   oqc_ta      = new ThreshArray [n_vx];
   fqc_val     = new NumArray    [n_vx];
   oqc_val     = new NumArray    [n_vx];
   fcat_ta     = new ThreshArray [n_vx];
   ocat_ta     = new ThreshArray [n_vx];
   fcnt_ta     = new ThreshArray [n_vx];
   ocnt_ta     = new ThreshArray [n_vx];
   cnt_logic   = new SetLogic    [n_vx];
   fwind_ta    = new ThreshArray [n_vx];
   owind_ta    = new ThreshArray [n_vx];
   wind_logic  = new SetLogic    [n_vx];
   eclv_points = new NumArray    [n_vx];

   // Initialize pointers
   for(i=0; i<n_vx; i++) fcst_info[i] = obs_info[i] = (VarInfo *) 0;

   // Parse the fcst and obs field information
   for(i=0; i<n_vx; i++) {

      // Allocate new VarInfo objects
      fcst_info[i] = info_factory.new_var_info(ftype);
      obs_info[i]  = info_factory.new_var_info(otype);

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);
      i_odict = parse_conf_i_vx_dict(odict, i);

      // Set the current dictionaries
      fcst_info[i]->set_dict(i_fdict);
      obs_info[i]->set_dict(i_odict);

      // Conf: desc
      desc.add(parse_conf_string(&i_odict, conf_key_desc));

      // Conf: eclv_points
      eclv_points[i] = parse_conf_eclv_points(&i_odict);

      // Dump the contents of the current VarInfo
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed forecast field number " << i+1 << ":\n";
         fcst_info[i]->dump(cout);
         mlog << Debug(5)
              << "Parsed observation field number " << i+1 << ":\n";
         obs_info[i]->dump(cout);
      }

      // No support for wind direction
      if(fcst_info[i]->is_wind_direction() ||
         obs_info[i]->is_wind_direction()) {
         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "the wind direction field may not be verified "
              << "using grid_stat.\n\n";
         exit(1);
      }

      // Check that the observation field does not contain probabilities
      if(obs_info[i]->is_prob()) {
         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "The observation field cannot contain probabilities.\n\n";
         exit(1);
      }
   } // end for i

   // If VL1L2 or VAL1L2 is requested, check the specified fields and turn
   // on the vflag when UGRD is followed by VGRD at the same level
   if(output_flag[i_vl1l2]  != STATOutputType_None ||
      output_flag[i_val1l2] != STATOutputType_None) {

      for(i=0, n_vx_vect = 0; i<n_vx; i++) {

         if(fcst_info[i]->is_u_wind() && obs_info[i]->is_u_wind()) {

            // Search for corresponding v wind
            for(int j=0; j<n_vx; j++) {
               if(fcst_info[j]->is_v_wind() && obs_info[j]->is_v_wind() &&
                  fcst_info[i]->req_level_name() ==
                  fcst_info[j]->req_level_name() &&
                  obs_info[i]->req_level_name()  ==
                  obs_info[j]->req_level_name()) {

                  fcst_info[i]->set_uv_index(j);
                  obs_info[i]->set_uv_index(j);

                  // Increment the number of vector fields to be verified
                  n_vx_vect++;
               }
            }
         }
         else if(fcst_info[i]->is_v_wind() && obs_info[i]->is_v_wind()) {

            // Search for corresponding u wind
            for(int j=0; j<n_vx; j++) {
               if(fcst_info[j]->is_u_wind() && obs_info[j]->is_u_wind() &&
                  fcst_info[i]->req_level_name() ==
                  fcst_info[j]->req_level_name() &&
                  obs_info[i]->req_level_name()  ==
                  obs_info[j]->req_level_name()) {

                  fcst_info[i]->set_uv_index(j);
                  obs_info[i]->set_uv_index(j);
               }
            }
         }
      } // end for
   } // end if

   // Compute the number of scalar and probability fields to be verified.
   for(i=0, n_vx_prob=0, n_vx_scal=0; i<n_vx; i++) {
      if(fcst_info[i]->is_prob()) n_vx_prob++;
      else                        n_vx_scal++;
   }

   // Initialize maximum threshold counts
   max_n_cat_thresh   = 0;
   max_n_cnt_thresh   = 0;
   max_n_wind_thresh  = 0;
   max_n_fprob_thresh = 0;
   max_n_oprob_thresh = 0;

   // Parse and sanity check thresholds
   for(i=0; i<n_vx; i++) {

      // Get the current dictionaries
      i_fdict = parse_conf_i_vx_dict(fdict, i);
      i_odict = parse_conf_i_vx_dict(odict, i);

      // Conf: qc_thresh
      fqc_ta[i] = i_fdict.lookup_thresh_array(conf_key_qc_thresh);
      oqc_ta[i] = i_odict.lookup_thresh_array(conf_key_qc_thresh);

      // Conf: qc_new_val
      fqc_val[i] = i_fdict.lookup_num_array(conf_key_qc_new_val);
      oqc_val[i] = i_odict.lookup_num_array(conf_key_qc_new_val);

      // Conf: cat_thresh
      fcat_ta[i] = i_fdict.lookup_thresh_array(conf_key_cat_thresh);
      ocat_ta[i] = i_odict.lookup_thresh_array(conf_key_cat_thresh);

      // Conf: cnt_thresh
      fcnt_ta[i] = i_fdict.lookup_thresh_array(conf_key_cnt_thresh);
      ocnt_ta[i] = i_odict.lookup_thresh_array(conf_key_cnt_thresh);

      // Conf: cnt_logic
      cnt_logic[i] = check_setlogic(
         int_to_setlogic(i_fdict.lookup_int(conf_key_cnt_logic)),
         int_to_setlogic(i_odict.lookup_int(conf_key_cnt_logic)));

      // Conf: wind_thresh
      fwind_ta[i] = i_fdict.lookup_thresh_array(conf_key_wind_thresh);
      owind_ta[i] = i_odict.lookup_thresh_array(conf_key_wind_thresh);

      // Conf: wind_logic
      wind_logic[i] = check_setlogic(
         int_to_setlogic(i_fdict.lookup_int(conf_key_wind_logic)),
         int_to_setlogic(i_odict.lookup_int(conf_key_wind_logic)));

      // Dump the contents of the current thresholds
      if(mlog.verbosity_level() >= 5) {
         mlog << Debug(5)
              << "Parsed threshold settings for field number " << i+1 << "...\n"
              // JHG
              << "Forecast categorical thresholds: "  << fcat_ta[i].get_str() << "\n"
              << "Observed categorical thresholds: "  << ocat_ta[i].get_str() << "\n"
              << "Forecast continuous thresholds: "   << fcnt_ta[i].get_str() << "\n"
              << "Observed continuous thresholds: "   << ocnt_ta[i].get_str() << "\n"
              << "Continuous threshold logic: "       << setlogic_to_string(cnt_logic[i]) << "\n"
              << "Forecast wind speed thresholds: "   << fwind_ta[i].get_str() << "\n"
              << "Observed wind speed thresholds: "   << owind_ta[i].get_str() << "\n"
              << "Wind speed threshold logic: "       << setlogic_to_string(wind_logic[i]) << "\n";
      }

      // Verifying a probability field
      if(fcst_info[i]->is_prob()) {
         fcat_ta[i] = string_to_prob_thresh(fcat_ta[i].get_str());
      }

      // Check for equal length of quality control thresholds and replacement values
      if(fqc_ta[i].n_elements() != fqc_val[i].n_elements() ||
         oqc_ta[i].n_elements() != oqc_val[i].n_elements()) {

         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "The number of quality control thresholds in \""
              << conf_key_qc_thresh
              << "\" must match the number of replacement values in \""
              << conf_key_qc_new_val << "\".\n\n";
         exit(1);
      }

      // Check for equal threshold length for non-probability fields
      if(!fcst_info[i]->is_prob() &&
         fcat_ta[i].n_elements() != ocat_ta[i].n_elements()) {

         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "The number of thresholds for each field in \"fcst."
              << conf_key_cat_thresh
              << "\" must match the number of thresholds for each "
              << "field in \"obs." << conf_key_cat_thresh << "\".\n\n";
         exit(1);
      }

      // Add default continuous thresholds until the counts match
      n = max(fcnt_ta[i].n_elements(), ocnt_ta[i].n_elements());
      while(fcnt_ta[i].n_elements() < n) fcnt_ta[i].add(na_str);
      while(ocnt_ta[i].n_elements() < n) ocnt_ta[i].add(na_str);

      // Add default wind speed thresholds until the counts match
      n = max(fwind_ta[i].n_elements(), owind_ta[i].n_elements());
      while(fwind_ta[i].n_elements() < n) fwind_ta[i].add(na_str);
      while(owind_ta[i].n_elements() < n) owind_ta[i].add(na_str);

      // Verifying with multi-category contingency tables
      if(!fcst_info[i]->is_prob() &&
         (output_flag[i_mctc] != STATOutputType_None ||
          output_flag[i_mcts] != STATOutputType_None)) {
         check_mctc_thresh(fcat_ta[i]);
         check_mctc_thresh(ocat_ta[i]);
      }

      // Look for the maximum number of thresholds
      if(!fcst_info[i]->is_prob()) {

         if(fcat_ta[i].n_elements() > max_n_cat_thresh)
            max_n_cat_thresh = fcat_ta[i].n_elements();
         if(fcnt_ta[i].n_elements() > max_n_cnt_thresh)
            max_n_cnt_thresh = fcnt_ta[i].n_elements();
         if(fwind_ta[i].n_elements() > max_n_wind_thresh)
            max_n_wind_thresh = fwind_ta[i].n_elements();
      }
      // Look for the maximum number of thresholds for prob fields
      else {

         if(fcat_ta[i].n_elements() > max_n_fprob_thresh)
            max_n_fprob_thresh = fcat_ta[i].n_elements();
         if(ocat_ta[i].n_elements() > max_n_oprob_thresh)
            max_n_oprob_thresh = ocat_ta[i].n_elements();
      }
   } // end for i

   // Conf: ci_alpha
   ci_alpha = parse_conf_ci_alpha(&conf);

   // Conf: boot
   boot_info     = parse_conf_boot(&conf);
   boot_interval = boot_info.interval;
   boot_rep_prop = boot_info.rep_prop;
   n_boot_rep    = boot_info.n_rep;
   boot_rng      = boot_info.rng;
   boot_seed     = boot_info.seed;

   // Conf: interp
   interp_info   = parse_conf_interp(&conf);
   interp_field  = interp_info.field;
   interp_thresh = interp_info.vld_thresh;
   n_interp      = interp_info.n_interp;
   interp_wdth   = interp_info.width;
   interp_shape  = interp_info.shape;

   // Allocate memory to store the interpolation methods
   interp_mthd = new InterpMthd [n_interp];
   for(i=0; i<n_interp; i++) {

      interp_mthd[i] = string_to_interpmthd(interp_info.method[i]);

      // Check for DW_MEAN, LS_FIT, BILIN and others
      if(interp_mthd[i] == InterpMthd_DW_Mean ||
         interp_mthd[i] == InterpMthd_LS_Fit  ||
         interp_mthd[i] == InterpMthd_Bilin) {
         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "Interpolation methods DW_MEAN, LS_FIT, and BILIN are "
              << "not supported in Grid-Stat.\n\n";
         exit(1);
      }

      // Check for valid interpolation width
      if(interp_wdth[i] < 1 || interp_wdth[i]%2 == 0) {
         mlog << Error << "\nGridStatConfInfo::process_config() -> "
              << "The interpolation width must be set to odd values "
              << "greater than or equal to 1 ("
              << interp_wdth[i] << ").\n\n";
         exit(1);
      }
   } // end for i

   // Conf: nbrhd
   nbrhd_info   = parse_conf_nbrhd(&conf);
   nbrhd_thresh = nbrhd_info.vld_thresh;
   nbrhd_wdth   = nbrhd_info.width;
   nbrhd_cov_ta = nbrhd_info.cov_ta;
   nbrhd_shape  = nbrhd_info.shape;

   // Conf: fourier
   Dictionary * d = conf.lookup_dictionary(conf_key_fourier);
   wave_1d_beg = d->lookup_int_array(conf_key_wave_1d_beg);
   wave_1d_end = d->lookup_int_array(conf_key_wave_1d_end);

   // Check for the same length
   if(wave_1d_beg.n_elements() != wave_1d_end.n_elements()) {
      mlog << Error << "\nGridStatConfInfo::process_config() -> "
           << "The fourier wave_1d_beg and wave_1d_end arrays must have the "
           << "same length (" << wave_1d_beg.n_elements() << " != "
           << wave_1d_end.n_elements() << ").\n\n";
      exit(1);
   }

   // Conf: nc_pairs_flag
   parse_nc_info();

   // Check for at least one output data type
   if(!get_ascii_output_flag() && nc_info.all_false()) {
      mlog<<Error<<"\nGridStatConfInfo::process_config() -> "
          <<"At least one output STAT or NetCDF type must be requested.\n\n";
      exit(1);
   }

   // Conf: grid_weight_flag
   grid_weight_flag = parse_conf_grid_weight_flag(&conf);

   // Conf: rank_corr_flag
   rank_corr_flag = conf.lookup_bool(conf_key_rank_corr_flag);

   // Conf: tmp_dir
   tmp_dir = parse_conf_tmp_dir(&conf);

   // Conf: output_prefix
   output_prefix = conf.lookup_string(conf_key_output_prefix);

   return;
}

////////////////////////////////////////////////////////////////////////


void GridStatConfInfo::parse_nc_info()

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = conf.lookup(conf_key_nc_pairs_flag);

if ( !e )  {

   mlog << Error
        << "\nGridStatConfInfo::parse_nc_info() -> lookup failed for key \""
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
        << "\nGridStatConfInfo::parse_nc_info() -> bad type ("
        << configobjecttype_to_string(type)
        << ") for key \""
        << conf_key_nc_pairs_flag << "\"\n\n";

   exit ( 1 );

}

   //
   //  parse the various entries
   //

Dictionary * d = e->dict_value();

nc_info.do_latlon      = d->lookup_bool(conf_key_latlon_flag);
nc_info.do_raw         = d->lookup_bool(conf_key_raw_flag);
nc_info.do_diff        = d->lookup_bool(conf_key_diff_flag);
nc_info.do_climo       = d->lookup_bool(conf_key_climo_flag);
nc_info.do_weight      = d->lookup_bool(conf_key_weight);
nc_info.do_nbrhd       = d->lookup_bool(conf_key_nbrhd);
nc_info.do_fourier     = d->lookup_bool(conf_key_fourier);
nc_info.do_apply_mask  = d->lookup_bool(conf_key_apply_mask_flag);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::process_masks(const Grid &grid) {
   int i;
   StringArray mask_grid, mask_poly;
   ConcatString name;

   mlog << Debug(2)
        << "Processing masking regions.\n";

   // Retrieve the mask names
   mask_grid = conf.lookup_string_array(conf_key_mask_grid);
   mask_poly = conf.lookup_string_array(conf_key_mask_poly);

   // Get the number of masks
   n_mask = mask_grid.n_elements() + mask_poly.n_elements();

   // Check that at least one verification masking region is provided
   if(n_mask == 0) {

      mlog << Error << "\nGridStatConfInfo::process_masks() -> "
           << "At least one grid or polyline verification masking "
           << "region must be provided.\n\n";
      exit(1);
   }

   // Allocate space to store the masking information
   mask_dp = new DataPlane [n_mask];

   // Parse out the masking grids
   for(i=0; i<mask_grid.n_elements(); i++) {
      mlog << Debug(3)
           << "Processing grid mask: " << mask_grid[i] << "\n";
      parse_grid_mask(mask_grid[i], grid, mask_dp[i], name);
      mask_name.add(name);
   }

   // Parse out the masking polylines
   for(i=0; i<mask_poly.n_elements(); i++) {
      mlog << Debug(3)
           << "Processing poly mask: " << mask_poly[i] << "\n";
      parse_poly_mask(mask_poly[i], grid,
                      mask_dp[i+mask_grid.n_elements()], name);
      mask_name.add(name);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int GridStatConfInfo::n_txt_row(int i_txt_row) {
   int n;

   // Switch on the index of the line type
   switch(i_txt_row) {

      case(i_fho):
      case(i_ctc):
         // Maximum number of FHO or CTC lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds
         n = n_vx_scal * n_mask * n_interp * max_n_cat_thresh;
         break;

      case(i_cts):
         // Maximum number of CTS lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds *
         //    Alphas
         n = n_vx_scal * n_mask * n_interp * max_n_cat_thresh *
             get_n_ci_alpha();
         break;

      case(i_mctc):
         // Maximum number of CTC lines possible =
         //    Fields * Masks * Smoothing Methods
         n = n_vx_scal * n_mask * n_interp;
         break;

      case(i_mcts):
         // Maximum number of CTS lines possible =
         //    Fields * Masks * Smoothing Methods * Alphas
         n = n_vx_scal * n_mask * n_interp * get_n_ci_alpha();
         break;

      case(i_cnt):
         // Maximum number of CNT lines possible =
         //    Fields * Masks * (Smoothing Methods + Fourier Waves) *
         //    Max Thresholds * Alphas
         n = n_vx_scal * n_mask * (n_interp + get_n_wave_1d()) *
             max_n_cnt_thresh * get_n_ci_alpha();
         break;

      case(i_sl1l2):
      case(i_sal1l2):
         // Maximum number of SL1L2 or SAL1L2 lines possible =
         //    Fields * Masks * (Smoothing Methods + Fourier Waves) *
         //    Max Thresholds
         n = n_vx_scal * n_mask * (n_interp + get_n_wave_1d()) *
             max_n_cnt_thresh;
         break;

      case(i_vl1l2):
      case(i_val1l2):
         // Maximum number of VL1L2 or VAL1L2 lines possible =
         //    Fields * Masks * (Smoothing Methods + Fourier Waves) *
         //    Max Thresholds
         n = n_vx_vect * n_mask * (n_interp + get_n_wave_1d()) *
             max_n_wind_thresh;
         break;

      case(i_nbrctc):
         // Maximum number of NBRCTC lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds
         n = n_vx_scal * n_mask * max_n_cat_thresh *
             get_n_nbrhd_wdth() * get_n_cov_thresh();
         break;

      case(i_nbrcts):
         // Maximum number of NBRCTS lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Frac Thresholds * Alphas
         n = n_vx_scal * n_mask * max_n_cat_thresh *
             get_n_nbrhd_wdth() * get_n_cov_thresh() *
             get_n_ci_alpha();
         break;

      case(i_nbrcnt):
         // Maximum number of NBRCNT lines possible =
         //    Fields * Masks * Max Thresholds *
         //    Neighborhoods * Alphas
         n = n_vx_scal * n_mask * max_n_cat_thresh *
             get_n_nbrhd_wdth() * get_n_ci_alpha();
         break;

      case(i_pct):
      case(i_pjc):
      case(i_prc):
         // Maximum number of PCT, PJC, or PRC lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds
         n = n_vx_prob * n_mask * n_interp * max_n_oprob_thresh;
         break;

      case(i_pstd):
         // Maximum number of PSTD lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds *
         //    Alphas
         n = n_vx_prob * n_mask * n_interp * max_n_oprob_thresh *
             get_n_ci_alpha();
         break;

      case(i_eclv):
         // Maximum number of CTC -> ECLV lines possible =
         //    Fields * Masks * Smoothing Methods * Max Thresholds
         n = n_vx_scal * n_mask * n_interp * max_n_cat_thresh;

         // Maximum number of PCT -> ECLV lines possible =
         //    Probability Fields * Masks * Smoothing Methods *
         //    Max Observation Probability Thresholds *
         //    Max Forecast Probability Thresholds
         n += n_vx_prob * n_mask * n_interp * max_n_oprob_thresh *
              max_n_fprob_thresh;
         break;

      default:
         mlog << Error << "\nGridStatConfInfo::n_txt_row(int) -> "
              << "unexpected output type index value: " << i_txt_row
              << "\n\n";
         exit(1);
         break;
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

int GridStatConfInfo::n_stat_row() {
   int i, n;

   // Set the maximum number of STAT output lines by summing the counts
   // for the optional text files that have been requested
   for(i=0, n=0; i<n_txt; i++) {
      if(output_flag[i] != STATOutputType_None) n += n_txt_row(i);
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

void GridStatConfInfo::set_ascii_output_flag(bool val)
{
    ascii_output_flag = val;
}

////////////////////////////////////////////////////////////////////////

bool GridStatConfInfo::get_ascii_output_flag()
{
    return ascii_output_flag;
}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct GridStatNcOutInfo
   //


////////////////////////////////////////////////////////////////////////


GridStatNcOutInfo::GridStatNcOutInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void GridStatNcOutInfo::clear()

{

set_all_true();

return;

}

////////////////////////////////////////////////////////////////////////


bool GridStatNcOutInfo::all_false() const

{

bool status = do_latlon || do_raw   || do_diff    || do_climo ||
              do_weight || do_nbrhd || do_fourier || do_apply_mask;

return ( !status );

}


////////////////////////////////////////////////////////////////////////


void GridStatNcOutInfo::set_all_false()

{

do_latlon     = false;
do_raw        = false;
do_diff       = false;
do_climo      = false;
do_weight     = false;
do_nbrhd      = false;
do_fourier    = false;
do_apply_mask = false;

return;

}


////////////////////////////////////////////////////////////////////////


void GridStatNcOutInfo::set_all_true()

{

do_latlon     = true;
do_raw        = true;
do_diff       = true;
do_climo      = true;
do_weight     = true;
do_nbrhd      = true;
do_fourier    = true;
do_apply_mask = true;

return;

}


////////////////////////////////////////////////////////////////////////
