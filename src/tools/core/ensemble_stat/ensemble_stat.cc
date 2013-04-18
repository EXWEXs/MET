// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   ensemble_stat.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    02/01/10  Halley Gotway   New
//   001    09/09/11  Halley Gotway   Call set_grid after reading
//                    gridded observation files.
//   002    10/13/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//   003    10/18/11  Halley Gotway   When the user requests verification
//                    in the config file, check that obs files have been
//                    specified on the command line.
//   004    11/14/11  Holmes          Added code to enable reading of
//                                    multiple config files.
//   005    12/09/11  Halley Gotway   When gridded observations are
//          missing, print a warning and continue rather than exiting
//          with error status.
//   006    05/08/12  Halley Gotway   Switch to using vx_config library.
//   007    05/08/12  Halley Gotway   Move -ens_valid, -ens_lead,
//                    and -obs_lead command line options to config file.
//   008    05/18/12  Halley Gotway   Modify logic to better handle
//                    missing files, fields, and data values.
//   008    08/30/12  Oldenburg       Added spread/skill functionality
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ensemble_stat.h"

#include "vx_nc_util.h"
#include "vx_data2d_nc_met.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line  (int, char **);
static void process_ensemble      ();
static void process_vx            ();

static void process_point_vx      ();
static void process_point_obs     (int);
static int  process_point_ens     (int);
static void process_point_scores  ();

static void process_grid_vx       ();
static void process_grid_scores   (DataPlane *&, DataPlane &, DataPlane &,
                                   DataPlane &, PairDataEnsemble &);

static void parse_ens_file_list(const char *);
static void set_grid(const Grid &);
static void clear_counts(const DataPlane &, int);
static void track_counts(const DataPlane &, int);

static void setup_nc_file   (unixtime, int, const char *);
static void setup_txt_files ();
static void setup_table     (AsciiTable &);

static void build_outfile_name(unixtime, const char *,
                               ConcatString &);
static void write_ens_nc(int, DataPlane &);
static void write_ens_var_float(int, float *, DataPlane &,
                                const char *, const char *);
static void write_ens_var_int(int, int *, DataPlane &,
                              const char *, const char *);

static void write_orank_nc(PairDataEnsemble &, DataPlane &, int, int, int);
static void write_orank_var_float(int, int, int, float *, DataPlane &,
                                  const char *, const char *);
static void write_orank_var_int(int, int, int, int *, DataPlane &,
                                const char *, const char *);

static void add_var_att(int, NcVar *, bool is_int, DataPlane &,
                        const char *, const char *);

static void finish_txt_files();
static void clean_up();

static void usage();

static void set_grid_obs(const StringArray &);
static void set_point_obs(const StringArray &);
static void set_ssvar_mean(const StringArray & a);
static void set_obs_valid_beg(const StringArray &);
static void set_obs_valid_end(const StringArray &);
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the ensemble
   process_ensemble();

   // Only perform verification if requested
   if(vx_flag) process_vx();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv)

{

   int i;
   struct stat results;
   GrdFileType etype, otype;
   CommandLine cline;
   ConcatString default_config_file;

   // Set default output directory
   // out_dir << MET_BASE << "/out/ensemble_stat";
   out_dir = ".";

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_grid_obs, "-grid_obs", 1);
   cline.add(set_point_obs, "-point_obs", 1);
   cline.add(set_ssvar_mean, "-ssvar_mean", 1);
   cline.add(set_obs_valid_beg, "-obs_valid_beg", 1);
   cline.add(set_obs_valid_end, "-obs_valid_end", 1);
   cline.add(set_outdir, "-outdir", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);

   //
   // parse the command line
   //
   cline.parse();
   
   //
   // Check for error. There should be either a number followed by that
   // number of filenames or a single filename and a config filename.
   //
   if(cline.n() < 2) usage();

   if(cline.n() == 2) {

      //
      // It should be a filename and then a config filename
      //
      if(is_integer(cline[0]) == 0)
         parse_ens_file_list(cline[0]);
      else
         usage();

   }
   else {
     
      //
      // More than two arguments. Check that the first is an integer
      // followed by that number of filenames and a config filename.
      //
      if(is_integer(cline[0]) == 1) {
        
         n_ens = atoi(cline[0]);

         if(n_ens <= 0) {
            mlog << Error << "\nprocess_command_line() -> "
                 << "the number of ensemble member files must be >= 1 ("
                 << n_ens << ")\n\n";
            exit(1);
         }

         if((cline.n() - 2) == n_ens) {

            //
            // Add each of the ensemble members to the list of files.
            //
            for(i = 1; i <= n_ens; i++) ens_file_list.add(cline[i]);
         }
         else {
            usage();
         }
      }
   }

   // Check that the end_ut >= beg_ut
   if(obs_valid_beg_ut != (unixtime) 0 &&
      obs_valid_end_ut != (unixtime) 0 &&
      obs_valid_beg_ut > obs_valid_end_ut) {

      mlog << Error << "\nprocess_command_line() -> "
           << "the ending time ("
           << unix_to_yyyymmdd_hhmmss(obs_valid_end_ut)
           << ") must be greater than the beginning time ("
           << unix_to_yyyymmdd_hhmmss(obs_valid_beg_ut)
           << ").\n\n";
      exit(1);
   }

   //
   // Store the config file name
   //
   config_file = cline[cline.n() - 1];

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file, config_file);
   
   // Get the ensemble file type from config, if present
   etype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_ens));

   // Read the first input ensemble file
   if(!(ens_mtddf = mtddf_factory.new_met_2d_data_file(ens_file_list[0], etype))) {
      mlog << Error << "\nprocess_command_line() -> "
           << "Trouble reading ensemble file \""
           << ens_file_list[0] << "\"\n\n";
      exit(1);
   }

   // Store the input ensemble file type
   etype = ens_mtddf->file_type();

   // Determine the input observation file type
   if(point_obs_flag)      otype = FileType_Gb1;
   else if(!grid_obs_flag) otype = FileType_None;
   else {
     
      // Get the observation file type from config, if present
      otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

      // Read the first gridded observation file
      if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(grid_obs_file_list[0]))) {
         mlog << Error << "\nprocess_command_line() -> "
              << "Trouble reading gridded observation file \""
              << grid_obs_file_list[0] << "\"\n\n";
         exit(1);
      }
      
      // Store the gridded observation file type
      otype = obs_mtddf->file_type();
   }
   
   // Process the configuration
   conf_info.process_config(etype, otype);

   // Set the model name
   shc.set_model(conf_info.model);

   // Allocate arrays to store threshold counts
   thresh_count_na = new NumArray [conf_info.get_max_n_thresh()];

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.rng_type, conf_info.rng_seed);

   // List the input ensemble files
   mlog << Debug(1) << "Ensemble Files["
        << ens_file_list.n_elements() << "]:\n";
   for(i=0; i<ens_file_list.n_elements(); i++) {
      mlog << "   " << ens_file_list[i]  << "\n";
   }

   // List the input gridded observations files
   if(grid_obs_file_list.n_elements() > 0) {
      mlog << Debug(1) << "Gridded Observation Files["
           << grid_obs_file_list.n_elements() << "]:\n" ;
      for(i=0; i<grid_obs_file_list.n_elements(); i++) {
         mlog << "   " << grid_obs_file_list[i] << "\n" ;
      }
   }

   // List the input point observations files
   if(point_obs_file_list.n_elements() > 0) {
      mlog << Debug(1) << "Point Observation Files["
           << point_obs_file_list.n_elements() << "]:\n" ;
      for(i=0; i<point_obs_file_list.n_elements(); i++) {
         mlog << "   " << point_obs_file_list[i] << "\n" ;
      }
   }

   // Check for missing ensemble files
   for(i=0; i<ens_file_list.n_elements(); i++) {

      if(stat(ens_file_list[i], &results) != 0) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "can't open input ensemble file: "
              << ens_file_list[i] << "\n\n";
         ens_file_vld.add(0);
      }
      else {
         ens_file_vld.add(1);
      }
   }

   // Set n_rank as one more than the number of valid forecasts
   n_rank = ens_file_vld.sum() + 1;

   // Set flag to indicate whether verification is to be performed
   if((point_obs_flag || grid_obs_flag) &&
      (conf_info.get_n_vx() > 0) &&
      (conf_info.output_flag[i_orank] != STATOutputType_None ||
       conf_info.output_flag[i_rhist] != STATOutputType_None ||
       conf_info.output_flag[i_ssvar] != STATOutputType_None)) vx_flag = 1;
   else                                                        vx_flag = 0;

   // Check the spread/skill configuration information
   conf_info.ens_ssvar_flag = 0;
   bool ssvar_out = (conf_info.output_flag[i_ssvar] != STATOutputType_None);
   if(ens_ssvar_mean && strcmp(ens_ssvar_mean, "")) {

      if(!ssvar_out) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "ignoring input -ssvar_mean file because "
              << "SSVAR line type is set to NONE\n\n";
      }
      else if(stat(ens_ssvar_mean, &results)) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "can't open input spread/skill mean file: "
              << ens_ssvar_mean << "\n\n";
         ens_ssvar_mean = "";
      }
      else if(!vx_flag) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "ignoring input -ssvar_mean file because "
              << "no observations have been specified\n\n";
      }
      else {
         conf_info.ens_ssvar_flag = 1;
      }

   }
   else if(ssvar_out) {

      if(!vx_flag) {
         mlog << Warning << "\nprocess_command_line() -> "
              << "disabling ensemble spread/skill calculation "
              << "because no observations have been specified\n\n";
      }
      else {
         conf_info.ens_ssvar_flag = 1;

         if(!conf_info.ensemble_flag[i_nc_mean]) {
            mlog << Warning << "\nprocess_command_line() -> "
                 << "enabling ensemble mean to facilitate calculation "
                 << "of ensemble spread/skill\n\n";
            conf_info.ensemble_flag[i_nc_mean] = true;
         }
      }
   }
   conf_info.ens_ssvar_mean = ens_ssvar_mean;

   return;
}

////////////////////////////////////////////////////////////////////////

void process_ensemble() {
   int i, j, n_miss, n_vld;
   bool found = false;
   double t;
   DataPlane ens_dp;

   // Loop through each of the ensemble fields to be processed
   for(i=0; i<conf_info.get_n_ens_var(); i++) {

      mlog << Debug(2) << "\n" << sep_str << "\n\n"
           << "Processing ensemble field: "
           << conf_info.ens_info[i]->magic_str() << "\n";

      // Loop through each of the input forecast files
      for(j=0, n_miss=0, n_vld=0; j<ens_file_list.n_elements(); j++) {

         // If the current forecast file is valid, read the field
         if(ens_file_vld[j]) {

            // Deallocate the data file pointer, if necessary
            if(ens_mtddf) { delete ens_mtddf; ens_mtddf = (Met2dDataFile *) 0; }
           
            // Read the current ensemble file
            if(!(ens_mtddf = mtddf_factory.new_met_2d_data_file(ens_file_list[j]))) {
               mlog << Error << "\nprocess_ensemble() -> "
                    << "Trouble reading ensemble file \""
                    << ens_file_list[j] << "\"\n\n";
               exit(1);
            }
           
            // Read the gridded data from the current ensemble file
            found = ens_mtddf->data_plane(*conf_info.ens_info[i], ens_dp);

            // Track the missing and valid counts
            if(!found) {
               mlog << Warning << "\nprocess_ensemble() -> "
                    << conf_info.ens_info[i]->magic_str()
                    << " not found in file \"" << ens_file_list[j]
                    << "\"\n\n";
               n_miss++;
               continue;
            }
            else {              
               n_vld++;
            }

            // Set the grid
            set_grid(ens_mtddf->grid());

            // Store the ensemble valid time, if not already set
            if(ens_valid_ut == (unixtime) 0) {
               ens_valid_ut = ens_dp.valid();
            }
            // Check to make sure that the valid time doesn't change
            else {

               if(ens_valid_ut != ens_dp.valid()) {

                  mlog << Warning << "\nprocess_ensemble() -> "
                       << "The valid time has changed, "
                       << unix_to_yyyymmdd_hhmmss(ens_valid_ut)
                       << " != " << unix_to_yyyymmdd_hhmmss(ens_dp.valid())
                       << " in \"" << ens_file_list[j] << "\"\n\n";
               }
            }

            // Store the lead time
            ens_lead_na.add(ens_dp.lead());
         }
         else {

            mlog << Warning << "\nprocess_ensemble() -> "
                 << "Cannot read ensemble file \""
                 << ens_file_list[j] << "\"\n\n";
            found = false;
            n_miss++;
         }

         // Check if the number of missing fields exceeds the threshold
         t = 1.0 - conf_info.vld_ens_thresh;
         if((double) n_miss/n_ens > t) {
            mlog << Error << "\nprocess_ensemble_files() -> "
                 << n_miss << " missing fields exceeds the maximum "
                 << "allowable specified by \"ens.ens_thresh\" "
                 << "in the configuration file.\n\n";
            exit(1);
         }

         // Continue if the current field is missing
         if(!found) continue;

         // Create a NetCDF file to store the ensemble output
         if(nc_out == (NcFile *) 0)
            setup_nc_file(ens_dp.valid(), ens_dp.lead(), "_ens.nc");

         // Reset the running sums and counts
         if(n_vld == 1) clear_counts(ens_dp, i);

         // Apply current data to the running sums and counts
         track_counts(ens_dp, i);

      } // end for j

      // Check if the number of missing fields exceeds the threshold
      t = 1.0 - conf_info.vld_ens_thresh;
      if((double) n_miss/n_ens > t) {
         mlog << Error << "\nprocess_ensemble_files() -> "
              << n_miss << " missing fields exceeds the maximum "
              << "allowable specified by \"ens.ens_thresh\" "
              << "in the configuration file.\n\n";
         exit(1);
      }

      // Store the number of valid members for the current field
      n_ens_vld.add(n_vld);

      // Write out the ensemble information to a NetCDF file
      write_ens_nc(i, ens_dp);

      // Store the ensemble output file for spread/skill analysis
      conf_info.ens_ssvar_file = out_nc_file_list[ out_nc_file_list.n_elements() - 1 ];
   }

   // Close the output NetCDF file
   if(nc_out) { nc_out->close(); delete nc_out; nc_out = (NcFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_vx() {

   if(conf_info.get_n_vx() > 0) {

      if(point_obs_file_list.n_elements() == 0 &&
         grid_obs_file_list.n_elements()  == 0) {
         mlog << Error << "\nprocess_vx() -> "
              << " when \"fcst_field\" is non-empty, you must use "
              << "\"-point_obs\" and/or \"-grid_obs\" to specify the "
              << "verifying observations.\n\n";
         exit(1);
      }

      // Process masks Grids and Polylines in the config file
      conf_info.process_masks(grid);

      // Setup the GCPairData objects
      conf_info.set_vx_pd();

      // Process the point observations
      if(point_obs_flag) process_point_vx();

      // Process the gridded observations
      if(grid_obs_flag) process_grid_vx();
   }

   return;
}

////////////////////////////////////////////////////////////////////////
      
void process_point_vx() {
   int i, n_miss, miss_flag;
   double t;

   // Set the matching time window.  If obs_valid_beg_ut and obs_valid_end_ut
   // were not set on the command line, use beg_ds and end_ds.
   if(obs_valid_beg_ut == (unixtime) 0) {
      obs_valid_beg_ut = ens_valid_ut + conf_info.beg_ds;
   }
   if(obs_valid_end_ut == (unixtime) 0) {
      obs_valid_end_ut = ens_valid_ut + conf_info.end_ds;
   }

   // Store the valid time window
   for(i=0; i<conf_info.get_n_vx(); i++) {
      conf_info.vx_pd[i].set_fcst_ut(ens_valid_ut);
      conf_info.vx_pd[i].set_beg_ut(obs_valid_beg_ut);
      conf_info.vx_pd[i].set_end_ut(obs_valid_end_ut);
   }

   // Process each point observation NetCDF file
   for(i=0; i<point_obs_file_list.n_elements(); i++) {
      process_point_obs(i);
   }

   // Set the size to store the ensemble data
   for(i=0; i<conf_info.get_n_vx(); i++) {
      conf_info.vx_pd[i].set_ens_size();
   }

   // Process each ensemble file
   for(i=0, n_miss=0; i<ens_file_list.n_elements(); i++) {

      // If the current forecast file is valid, process it
      if(ens_file_vld[i]) miss_flag = process_point_ens(i);
      else                miss_flag = 1;

      // Increment the missing count
      n_miss += miss_flag;

      // Check if the number of missing fields exceeds the threshold
      t = 1.0 - conf_info.vld_ens_thresh;
      if((double) n_miss/n_ens > t) {
         mlog << Error << "\nprocess_point_vx() -> "
              << n_miss << " missing fields exceeds the maximum "
              << "allowable specified in the configuration file.\n\n";
         exit(1);
      }
   } // end for i

   // Process the ensemble mean, if spread/skill is activated
   if(conf_info.ens_ssvar_flag) process_point_ens(-1);

   // Compute the scores and write them out
   process_point_scores();

   return;
}

////////////////////////////////////////////////////////////////////////

void process_point_obs(int i_nc) {
   int i_obs, j;
   float obs_arr[obs_arr_len], hdr_arr[hdr_arr_len];
   char hdr_typ_str[max_str_len];
   char hdr_sid_str[max_str_len];
   char hdr_vld_str[max_str_len];
   char obs_qty_str[max_str_len];
   unixtime hdr_ut;
   NcFile *obs_in = (NcFile *) 0;

   mlog << Debug(2) << "\n" << sep_str << "\n\n"
        << "Processing point observation file: "
        << point_obs_file_list[i_nc] << "\n";

   // Open the observation file as a NetCDF file.
   obs_in = new NcFile(point_obs_file_list[i_nc]);

   if(!obs_in->is_valid()) {
      obs_in->close();
      delete obs_in;
      obs_in = (NcFile *) 0;

      mlog << Warning << "\nprocess_point_obs() -> "
           << "can't open observation netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      return;
   }

   // Define dimensions
   NcDim *strl_dim    = (NcDim *) 0; // Maximum string length
   NcDim *obs_dim     = (NcDim *) 0; // Number of observations
   NcDim *hdr_dim     = (NcDim *) 0; // Number of PrepBufr messages

   // Define variables
   NcVar *obs_arr_var = (NcVar *) 0;
   NcVar *obs_qty_var = (NcVar *) 0;
   NcVar *hdr_typ_var = (NcVar *) 0;
   NcVar *hdr_sid_var = (NcVar *) 0;
   NcVar *hdr_vld_var = (NcVar *) 0;
   NcVar *hdr_arr_var = (NcVar *) 0;

   // Read the dimensions
   strl_dim = obs_in->get_dim("mxstr");
   obs_dim  = obs_in->get_dim("nobs");
   hdr_dim  = obs_in->get_dim("nhdr");

   if(!strl_dim || !strl_dim->is_valid() ||
      !obs_dim  || !obs_dim->is_valid()  ||
      !hdr_dim  || !hdr_dim->is_valid()) {
      mlog << Error << "\nprocess_point_obs() -> "
           << "can't read \"mxstr\", \"nobs\" or \"nmsg\" "
           << "dimensions from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      exit(1);
   }

   // Read the variables
   obs_arr_var = obs_in->get_var("obs_arr");
   hdr_typ_var = obs_in->get_var("hdr_typ");
   hdr_sid_var = obs_in->get_var("hdr_sid");
   hdr_vld_var = obs_in->get_var("hdr_vld");
   hdr_arr_var = obs_in->get_var("hdr_arr");
   obs_qty_var = has_var(obs_in, "obs_qty");

   if(!obs_arr_var || !obs_arr_var->is_valid() ||
      !hdr_typ_var || !hdr_typ_var->is_valid() ||
      !hdr_sid_var || !hdr_sid_var->is_valid() ||
      !hdr_vld_var || !hdr_vld_var->is_valid() ||
      !hdr_arr_var || !hdr_arr_var->is_valid()) {
      mlog << Error << "\nprocess_point_obs() -> "
           << "can't read \"obs_arr\", \"hdr_typ\", \"hdr_sid\", "
           << "\"hdr_vld\", or \"hdr_arr\" variables from netCDF file: "
           << point_obs_file_list[i_nc] << "\n\n";
      exit(1);
   }

   if(!obs_qty_var || !obs_qty_var->is_valid())
      mlog << Debug(3) << "Quality marker information not found input file.\n";

   mlog << Debug(2) << "Searching " << obs_dim->size()
        << " observations from " << hdr_dim->size()
        << " header messages.\n";

   // Process each observation in the file
   for(i_obs=0; i_obs<obs_dim->size(); i_obs++) {

      // Read the current observation message
      if(!obs_arr_var->set_cur((long) i_obs) ||
         !obs_arr_var->get(obs_arr, 1, obs_arr_len)) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "can't read the record for observation "
              << "number " << i_obs << "\n\n";
         exit(1);
      }

      // Read the corresponding header array for this observation
      if(!hdr_arr_var->set_cur((long) (obs_arr[0])) ||
         !hdr_arr_var->get(hdr_arr, 1, hdr_arr_len)) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "can't read the header array record for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the corresponding header type for this observation
      if(!hdr_typ_var->set_cur((long) (obs_arr[0])) ||
         !hdr_typ_var->get(hdr_typ_str, 1, strl_dim->size())) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "can't read the message type record for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the corresponding header Station ID for this observation
      if(!hdr_sid_var->set_cur((long) (obs_arr[0])) ||
         !hdr_sid_var->get(hdr_sid_str, 1, strl_dim->size())) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "can't read the station ID record for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the corresponding valid time for this observation
      if(!hdr_vld_var->set_cur((long) (obs_arr[0])) ||
         !hdr_vld_var->get(hdr_vld_str, 1, strl_dim->size())) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "can't read the valid time for header "
              << "number " << obs_arr[0] << "\n\n";
         exit(1);
      }

      // Read the current observation quality flag
      strcpy(obs_qty_str, "");
      if((obs_qty_var && obs_qty_var->is_valid()) &&
         (!obs_qty_var->set_cur((long) i_obs) ||
          !obs_qty_var->get(obs_qty_str, 1, strl_dim->size()))) {
         mlog << Error << "\nprocess_point_obs() -> "
              << "can't read the quality flag for observation "
              << "index " << i_obs << "\n\n";
         exit(1);
      }

      // Convert string to a unixtime
      hdr_ut = timestring_to_unix(hdr_vld_str);

      // Check each conf_info.vx_pd object to see if this observation
      // should be added
      for(j=0; j<conf_info.get_n_vx(); j++) {

         // Attempt to add the observation to the conf_info.vx_pd object
         conf_info.vx_pd[j].add_obs(hdr_arr, hdr_typ_str, hdr_sid_str,
                                    hdr_ut, obs_qty_str, obs_arr, grid);
      }
   } // end for i_obs

   // Print the duplicate report
   for(j=0; j < conf_info.get_n_vx(); j++) conf_info.vx_pd[j].print_duplicate_report();

   // Deallocate and clean up
   if(obs_in) { obs_in->close(); delete obs_in; obs_in = (NcFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

int process_point_ens(int i_ens) {
   int i, n_fcst;
   DataPlaneArray fcst_dpa;
   NumArray fcst_lvl_na;
   VarInfo *info = (VarInfo *) 0;

   ConcatString ens_file;
   bool ens_mn = (-1 == i_ens);
   const char *file_type = ens_mn ? "mean" : "ensemble";

   // Determine the correct file to process
   if(!ens_mn) ens_file = ConcatString(ens_file_list[i_ens]);
   else        ens_file = (ens_ssvar_mean && strcmp(ens_ssvar_mean,"")) ?
                           ens_ssvar_mean : conf_info.ens_ssvar_file;

   mlog << Debug(2) << "\n" << sep_str << "\n\n"
        << "Processing " << file_type << " file: " << ens_file << "\n";

   // Loop through each of the fields to be verified and extract
   // the forecast fields for verification
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // For spread/skill, use the calculated mean file if none was specified
      if(ens_mn && (!ens_ssvar_mean || !strcmp(ens_ssvar_mean, ""))) {
         fcst_dpa.clear();
         ConcatString ens_magic;
         ens_magic << conf_info.vx_pd[i].fcst_info->name() << "_"
                   << conf_info.vx_pd[i].fcst_info->level_name() << "_"
                   << "ENS_MEAN(*,*)";
         mlog << Debug(4) << "Generated mean field: " << ens_magic << "\n";
         info = new VarInfoNcMet();
         info->set_magic(ens_magic);
      }
      else {
         info = conf_info.vx_pd[i].fcst_info;
      }

      // Deallocate the data file pointer, if necessary
      if(ens_mtddf) { delete ens_mtddf; ens_mtddf = (Met2dDataFile *) 0; }

      // Read the current ensemble file
      if(!(ens_mtddf = mtddf_factory.new_met_2d_data_file(ens_file))) {
         mlog << Error << "\nprocess_point_ens() -> "
              << "Trouble reading " << file_type << " file \""
              << ens_file << "\"\n\n";
         exit(1);
      }

      // Read the gridded data from the input forecast file
      n_fcst = ens_mtddf->data_plane_array(*info, fcst_dpa);

      // Check for zero fields
      if(n_fcst == 0) {
         mlog << Error << "\nprocess_point_ens() -> "
              << "no fields matching " << info->magic_str()
              << " found in file: " << ens_file << "\n\n";
         exit(1);
      }

      // Set the grid
      set_grid(ens_mtddf->grid());

      // Dump out the number of levels found
      mlog << Debug(2) << "For " << info->magic_str()
           << " found " << n_fcst<< " forecast levels.\n";

      // Clean up the allocated VarInfo object, if necessary
      if(ens_mn && (!ens_ssvar_mean || !strcmp(ens_ssvar_mean, ""))) delete info;

      // Store information for the raw forecast fields
      conf_info.vx_pd[i].set_fcst_dpa(fcst_dpa);

      // Compute forecast values for this ensemble member
      conf_info.vx_pd[i].add_ens(ens_mn);

   } // end for i

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_point_scores() {
   PairDataEnsemble *pd_ptr = (PairDataEnsemble *) 0;
   int i, j, k, l, m;

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Create output text files as requested in the config file
   setup_txt_files();

   // Store the forecast lead time
   shc.set_fcst_lead_sec(nint(ens_lead_na.mode()));

   // Store the forecast valid time
   shc.set_fcst_valid_beg(ens_valid_ut);
   shc.set_fcst_valid_end(ens_valid_ut);

   // Loop through the Ensemble pair data objects, compute the scores
   // requested, and write the output.
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Store the forecast variable name
      shc.set_fcst_var(conf_info.vx_pd[i].fcst_info->name().text());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.vx_pd[i].fcst_info->level_name().text());

      // Store the observation variable name
      shc.set_obs_var(conf_info.vx_pd[i].obs_info->name().text());

      // Set the observation level name
      shc.set_obs_lev(conf_info.vx_pd[i].obs_info->level_name().text());

      // Set the observation lead time
      shc.set_obs_lead_sec(0);

      // Set the observation valid time
      shc.set_obs_valid_beg(conf_info.vx_pd[i].beg_ut);
      shc.set_obs_valid_end(conf_info.vx_pd[i].end_ut);

      // Loop through the message types
      for(j=0; j<conf_info.get_n_msg_typ(i); j++) {

         // Store the message type
         shc.set_msg_typ(conf_info.msg_typ[i][j]);

         // Loop through the verification masking regions
         for(k=0; k<conf_info.get_n_mask(); k++) {

            // Store the verification masking region
            shc.set_mask(conf_info.mask_name[k]);

            // Loop through the interpolation methods
            for(l=0; l<conf_info.get_n_interp(); l++) {

               // Store the interpolation method and width being applied
               shc.set_interp_mthd(conf_info.interp_mthd[l]);
               shc.set_interp_wdth(conf_info.interp_wdth[l]);

               pd_ptr = &conf_info.vx_pd[i].pd[j][k][l];

               // Determine the number of valid ensemble members
               pd_ptr->set_n_ens();
               
               // Compute the ranks for the observations
               pd_ptr->compute_rank(rng_ptr);

               mlog << Debug(2)
                    << "Processing point verification "
                    << conf_info.vx_pd[i].fcst_info->magic_str()
                    << " versus "
                    << conf_info.vx_pd[i].obs_info->magic_str()
                    << ", for observation type " << pd_ptr->msg_typ
                    << ", over region " << pd_ptr->mask_name
                    << ", for interpolation method "
                    << shc.get_interp_mthd_str() << "("
                    << shc.get_interp_pnts_str()
                    << "), using " << pd_ptr->n_pair << " pairs.\n";

               // Continue if there are no points
               if(pd_ptr->n_pair == 0) continue;

               // Compute ensemble statistics
               pd_ptr->compute_rhist();
               pd_ptr->compute_stats();

               // Write out the ORANK lines
               if(conf_info.output_flag[i_orank] != STATOutputType_None) {

                  write_orank_row(shc, pd_ptr,
                     conf_info.output_flag[i_orank],
                     stat_at, i_stat_row,
                     txt_at[i_orank], i_txt_row[i_orank]);

                  // Reset the observation valid time
                  shc.set_obs_valid_beg(conf_info.vx_pd[i].beg_ut);
                  shc.set_obs_valid_end(conf_info.vx_pd[i].end_ut);
               }

               // Compute RHIST scores
               if(conf_info.output_flag[i_rhist] != STATOutputType_None) {

                  write_rhist_row(shc, pd_ptr,
                     conf_info.output_flag[i_rhist],
                     stat_at, i_stat_row,
                     txt_at[i_rhist], i_txt_row[i_rhist]);
               }

               // Compute SSVAR scores
               if(conf_info.output_flag[i_ssvar] != STATOutputType_None) {
                  pd_ptr->compute_ssvar();

                  // Add rows to the output AsciiTables for SSVAR
                  stat_at.add_rows(pd_ptr->ssvar_bins[0].n_bin *
                                   conf_info.ci_alpha.n_elements());

                  if(conf_info.output_flag[i_ssvar] == STATOutputType_Both) {
                     txt_at[i_ssvar].add_rows(pd_ptr->ssvar_bins[0].n_bin *
                                              conf_info.ci_alpha.n_elements());
                  }

                  // Write the SSVAR data for each alpha value
                  for(m=0; m<conf_info.ci_alpha.n_elements(); m++) {
                     write_ssvar_row(shc, pd_ptr, conf_info.ci_alpha[m],
                        conf_info.output_flag[i_ssvar],
                        stat_at, i_stat_row,
                        txt_at[i_ssvar], i_txt_row[i_ssvar]);
                  }
               }

            } // end for l
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid_vx() {
   int i, j, k, l, n_miss;
   bool found;
   double t;
   DataPlane *fcst_dp = (DataPlane *) 0;
   DataPlane *fcst_dp_smooth = (DataPlane *) 0;
   DataPlane  obs_dp, obs_dp_smooth;
   DataPlane  mn_dp, mn_dp_smooth;
   PairDataEnsemble pd;

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Allocate space to store the forecast fields
   fcst_dp        = new DataPlane [n_ens];
   fcst_dp_smooth = new DataPlane [n_ens];

   // Loop through each of the fields to be verified
   for(i=0; i<conf_info.get_n_vx(); i++) {

      // Set the forecast lead time
      shc.set_fcst_lead_sec(nint(ens_lead_na.mode()));

      // Set the forecast valid time
      shc.set_fcst_valid_beg(ens_valid_ut);
      shc.set_fcst_valid_end(ens_valid_ut);

      // Set the forecast variable name
      shc.set_fcst_var(conf_info.vx_pd[i].fcst_info->name().text());

      // Set the forecast level name
      shc.set_fcst_lev(conf_info.vx_pd[i].fcst_info->level_name().text());

      // Set the observation variable name
      shc.set_obs_var(conf_info.vx_pd[i].obs_info->name().text());

      // Set the observation level name
      shc.set_obs_lev(conf_info.vx_pd[i].obs_info->level_name().text());

      // Setup the obtype column
      if(conf_info.vx_pd[i].fcst_info->is_precipitation() &&
         conf_info.vx_pd[i].obs_info->is_precipitation()) {
         shc.set_msg_typ("MC_PCP");
      }
      else {
         shc.set_msg_typ("ANALYS");
      }

      // Loop through each of the input ensemble files
      for(j=0, n_miss=0; j<ens_file_list.n_elements(); j++) {

         // Initialize
         fcst_dp[j].clear();

         // If the current ensemble file is valid, read the field
         if(ens_file_vld[j]) {

            // Deallocate the data file pointer, if necessary
            if(ens_mtddf) { delete ens_mtddf; ens_mtddf = (Met2dDataFile *) 0; }

            // Read the current ensemble file
            if(!(ens_mtddf = mtddf_factory.new_met_2d_data_file(ens_file_list[j]))) {
               mlog << Error << "\nprocess_grid_vx() -> "
                    << "Trouble reading ensemble file \""
                    << ens_file_list[j] << "\"\n\n";
               exit(1);
            }

            // Read the gridded data from the current ensemble file
            found = ens_mtddf->data_plane(*conf_info.vx_pd[i].fcst_info, fcst_dp[j]);

            // Count the number of missing files
            if(!found) {
               n_miss++;
               continue;
            }

            // Set the grid
            set_grid(ens_mtddf->grid());
         }
         else {
            n_miss++;
         }

         // Check if the number of missing fields exceeds the threshold
         t = 1.0 - conf_info.vld_ens_thresh;
         if((double) n_miss/n_ens > t) {
            mlog << Error << "\nprocess_grid_vx() -> "
                 << n_miss << " missing fields exceeds the maximum "
                 << "allowable specified in the configuration file.\n\n";
            exit(1);
         }
      } // end for j

      // If requested in the config file, create a NetCDF file to store
      // the verification matched pairs
      if(conf_info.ensemble_flag[i_nc_orank] &&
         nc_out == (NcFile *) 0)
         setup_nc_file(fcst_dp[j].valid(), fcst_dp[j].lead(),
                       "_orank.nc");

      // Read the observation file
      for(j=0, n_miss=0; j<grid_obs_file_list.n_elements(); j++) {

         // Deallocate the data file pointer, if necessary
         if(obs_mtddf) { delete obs_mtddf; obs_mtddf = (Met2dDataFile *) 0; }

         // Read the current gridded observation file
         if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(grid_obs_file_list[j]))) {
            mlog << Error << "\nprocess_grid_vx() -> "
                 << "Trouble reading gridded observation file \""
                 << grid_obs_file_list[j] << "\"\n\n";
            exit(1);
         }

         // Read the gridded data from the current observation file
         found = obs_mtddf->data_plane(*conf_info.vx_pd[i].obs_info, obs_dp);

         // If found, break out of the loop
         if(!found) n_miss++;
         else {

            // Set the grid
            set_grid(obs_mtddf->grid());

            break;
         }
      }

      // Check if the observation field was found
      if(n_miss == grid_obs_file_list.n_elements()) {
         mlog << Warning << "\nprocess_grid_vx() -> "
              << conf_info.vx_pd[i].obs_info->magic_str()
              << " not found in observation files.\n";
         continue;
      }

      // Set the observation lead time
      shc.set_obs_lead_sec(obs_dp.lead());

      // Set the observation valid time
      shc.set_obs_valid_beg(obs_dp.valid());
      shc.set_obs_valid_end(obs_dp.valid());

      // If spread/skill is activated, read the ensemble mean file
      if(conf_info.ens_ssvar_flag) {

         VarInfo *info = (VarInfo *) 0;
         ConcatString mn_file = (ens_ssvar_mean && strcmp(ens_ssvar_mean,"")) ?
                                 ens_ssvar_mean : conf_info.ens_ssvar_file;

         mlog << Debug(2) << "\n" << sep_str << "\n\n"
              << "Processing ensemble mean file: " << mn_file << "\n";

         // For spread/skill, use the calculated mean file if none was specified
         if(!ens_ssvar_mean || !strcmp(ens_ssvar_mean, "")) {
            ConcatString ens_magic;
            ens_magic << conf_info.ens_info[i]->name() << "_"
                      << conf_info.ens_info[i]->level_name() << "_"
                      << "ENS_MEAN(*,*)";
            mlog << Debug(4) << "Generated mean field: " << ens_magic << "\n";
            info = new VarInfoNcMet();
            info->set_magic(ens_magic);
         }
         else {
            info = conf_info.vx_pd[i].fcst_info;
         }

         // Deallocate the data file pointer, if necessary
         if(ens_mtddf) { delete ens_mtddf; ens_mtddf = (Met2dDataFile *) 0; }

         // Read the current gridded observation file
         if(!(ens_mtddf = mtddf_factory.new_met_2d_data_file(mn_file))) {
            mlog << Error << "\nprocess_grid_vx() -> "
                 << "Trouble reading gridded ensemble mean file \""
                 << mn_file << "\"\n\n";
            exit(1);
         }

         // Read the gridded data from the mean file
         if(!ens_mtddf->data_plane(*info, mn_dp)) {
            mlog << Error << "\nprocess_grid_vx() -> "
                 << "Trouble reading field " << info->magic_str()
                 << " from file " << mn_file << "\n\n";
            exit(1);
         }
      }

      // Loop through and apply each of the smoothing operations
      for(j=0; j<conf_info.get_n_interp(); j++) {

         // Check for allowable smoothing operation
         if(conf_info.interp_mthd[j] == InterpMthd_DW_Mean ||
            conf_info.interp_mthd[j] == InterpMthd_LS_Fit  ||
            conf_info.interp_mthd[j] == InterpMthd_Bilin   ||
            conf_info.interp_mthd[j] == InterpMthd_Nbrhd) {

            mlog << Warning << "\nprocess_grid_vx() -> "
                 << interpmthd_to_string(conf_info.interp_mthd[j])
                 << " smoothing option not supported for gridded observations.\n";
            continue;
         }

         // Set the interpolation method and width
         shc.set_interp_mthd(conf_info.interp_mthd[j]);
         shc.set_interp_wdth(conf_info.interp_wdth[j]);

         // If requested in the config file, smooth the forecast fields
         for(k=0; k<ens_file_list.n_elements(); k++) {
         
            if(conf_info.interp_field == FieldType_Fcst ||
               conf_info.interp_field == FieldType_Both) {
               smooth_field(fcst_dp[k], fcst_dp_smooth[k],
                            conf_info.interp_mthd[j],
                            conf_info.interp_wdth[j],
                            conf_info.interp_thresh);

               if(conf_info.ens_ssvar_flag) {
                  smooth_field(mn_dp, mn_dp_smooth,
                               conf_info.interp_mthd[j],
                               conf_info.interp_wdth[j],
                               conf_info.interp_thresh);
               }


            }
            // Do not smooth the forecast field
            else {
               fcst_dp_smooth[k] = fcst_dp[k];
               if(conf_info.ens_ssvar_flag) mn_dp_smooth = mn_dp;
            }
         } // end for k

         // If requested in the config file, smooth the observation field
         if(conf_info.interp_field == FieldType_Obs ||
            conf_info.interp_field == FieldType_Both) {
            smooth_field(obs_dp, obs_dp_smooth,
                         conf_info.interp_mthd[j],
                         conf_info.interp_wdth[j],
                         conf_info.interp_thresh);
         }
         // Do not smooth the observation field
         else {
            obs_dp_smooth = obs_dp;
         }

         // Loop through the masks to be applied
         for(k=0; k<conf_info.get_n_mask_area(); k++) {

            // Set the mask name
            shc.set_mask(conf_info.mask_name[k]);

            // Initialize
            pd.clear();

            // Apply the current mask to the fields and compute the pairs
            process_grid_scores(fcst_dp_smooth, obs_dp_smooth,
                                mn_dp_smooth,
                                conf_info.mask_dp[k], pd);

            mlog << Debug(2)
                 << "Processing gridded verification "
                 << conf_info.vx_pd[i].fcst_info->magic_str()
                 << " versus "
                 << conf_info.vx_pd[i].obs_info->magic_str()
                 << ", for observation type " << shc.get_msg_typ()
                 << ", over region " << shc.get_mask()
                 << ", for interpolation method "
                 << shc.get_interp_mthd_str() << "("
                 << shc.get_interp_pnts_str()
                 << "), using " << pd.n_pair << " pairs.\n";

            // Continue if there are no points
            if(pd.n_pair == 0) continue;

            // Compute ensemble statistics
            pd.compute_rhist();
            pd.compute_stats();

            if(i == 0) setup_txt_files();

            // Compute RHIST scores
            if(conf_info.output_flag[i_rhist] != STATOutputType_None) {

               write_rhist_row(shc, &pd,
                  conf_info.output_flag[i_rhist],
                  stat_at, i_stat_row,
                  txt_at[i_rhist], i_txt_row[i_rhist]);
            }

            // Write out the smoothed forecast, observation, and difference
            if(conf_info.ensemble_flag[i_nc_orank])
               write_orank_nc(pd, obs_dp_smooth, i, j, k);

            // Compute SSVAR scores
            if(conf_info.output_flag[i_ssvar] != STATOutputType_None) {
               pd.ssvar_bin_size = conf_info.ens_ssvar_bin_size[i];
               pd.compute_ssvar();

               // Add rows to the output AsciiTables for SSVAR
               stat_at.add_rows(pd.ssvar_bins[0].n_bin *
                                conf_info.ci_alpha.n_elements());

               if(conf_info.output_flag[i_ssvar] == STATOutputType_Both) {
                  txt_at[i_ssvar].add_rows(pd.ssvar_bins[0].n_bin *
                                           conf_info.ci_alpha.n_elements());
               }

               // Write the SSVAR data for each alpha value
               for(l=0; l<conf_info.ci_alpha.n_elements(); l++) {
                  write_ssvar_row(shc, &pd, conf_info.ci_alpha[l],
                     conf_info.output_flag[i_ssvar],
                     stat_at, i_stat_row,
                     txt_at[i_ssvar], i_txt_row[i_ssvar]);
               }
            }

         } // end for k
      } // end for j
   } // end for i

   // Delete allocated DataPlane objects
   if(fcst_dp)        { delete [] fcst_dp;        fcst_dp        = (DataPlane *) 0; }
   if(fcst_dp_smooth) { delete [] fcst_dp_smooth; fcst_dp_smooth = (DataPlane *) 0; }

   // Close the output NetCDF file
   if(nc_out) { nc_out->close(); delete nc_out; nc_out = (NcFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_grid_scores(DataPlane *&fcst_dp, DataPlane &obs_dp,
                         DataPlane &mn_dp,
                         DataPlane &mask_dp, PairDataEnsemble &pd) {
   int i, j, x, y;
   double v;

   // Loop through the observation field
   for(x=0; x<obs_dp.nx(); x++) {
      for(y=0; y<obs_dp.ny(); y++) {

         // Skip any grid points containing bad data values or where the
         // verification masking region is turned off
         if(is_bad_data(obs_dp.get(x, y)) || !mask_dp.s_is_on(x, y) ) continue;

         // Add the observation point
         pd.add_obs(x, y, obs_dp.get(x, y));

      } // end for y
   } // end for x

   // Loop through the mean field, adding all the points
   for(x=0; x<mn_dp.nx(); x++)
      for(y=0; y<mn_dp.ny(); y++)
         pd.mn_na.add(mn_dp.get(x, y));

   // Allocate space for the ensemble data
   pd.set_size();

   // Loop through the observation points
   for(i=0; i<pd.n_pair; i++) {

      x = nint(pd.x_na[i]);
      y = nint(pd.y_na[i]);

      // Loop through each of the ensemble members
      for(j=0; j<n_ens; j++) {

         // Skip missing data
         if(fcst_dp[j].nx() == 0 || fcst_dp[j].ny() == 0) {
            continue;
         }
         // Get the ensemble value
         else {
            v = fcst_dp[j].get(x, y);
         }

         // Store the ensemble value
         pd.add_ens(i, v);
      } // end for j
   } // end for i
   
   if(pd.n_pair > 0) {
     
      // Determine the number of valid ensemble members
      pd.set_n_ens();

      // Compute the ranks for the observations
      pd.compute_rank(rng_ptr);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_ens_file_list(const char *fcst_file) {
   char ens_file_name[PATH_MAX];
   ifstream f_in;

   // Open the ensemble file list
   f_in.open(fcst_file);
   if(!f_in) {
      mlog << Error << "\nparse_ens_file_list() -> "
           << "can't open input ensemble file list \"" << fcst_file
           << "\" for reading\n\n";
      exit(1);
   }

   // Read each ensemble member listed in the file
   while(f_in >> ens_file_name) {
      ens_file_list.add(ens_file_name);
      n_ens++;
   }

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void set_grid(const Grid &gr) {

   // Set the grid, if not already set
   if(grid.nx() == 0 && grid.ny() == 0) {
      grid = gr;
   }
   // Check to make sure that the grid doesn't change
   else {
      if(!(grid == gr)) {
         mlog << Error << "\nset_grid() -> "
              << "All data files must be on the same grid!\n\n";
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clear_counts(const DataPlane &dp, int i_vx) {
   int i, j;

   // Clear arrays
   count_na.clear();
   min_na.clear();
   max_na.clear();
   sum_na.clear();
   sum_sq_na.clear();
   for(i=0; i<conf_info.get_max_n_thresh(); i++) {
      thresh_count_na[i].clear();
   }

   // Initialize arrays
   for(i=0; i<dp.nx()*dp.ny(); i++) {
      count_na.add(0);
      min_na.add(bad_data_double);
      max_na.add(bad_data_double);
      sum_na.add(0.0);
      sum_sq_na.add(0.0);
      for(j=0; j<conf_info.get_max_n_thresh(); j++) {
         thresh_count_na[j].add(0);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void track_counts(const DataPlane &dp, int i_vx) {
   int x, y, n, i;
   double v;

   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         n = dp.two_to_one(x, y);
         v = dp.get(x, y);

         // Skip the bad data value
         if(is_bad_data(v)) continue;

         // Otherwise, add to counts and sums
         else {
            count_na.set(n, count_na[n] + 1);

            if(v <= min_na[n] || is_bad_data(min_na[n])) min_na.set(n, v);
            if(v >= max_na[n] || is_bad_data(max_na[n])) max_na.set(n, v);

            sum_na.set(n, sum_na[n] + v);
            sum_sq_na.set(n, sum_sq_na[n] + v*v);

            for(i=0; i<conf_info.ens_ta[i_vx].n_elements(); i++) {
               if(conf_info.ens_ta[i_vx][i].check(v))
                  thresh_count_na[i].set(n, thresh_count_na[i][n] + 1);
            }
         }

      } // end for y
   } // end for x

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(unixtime valid_ut, int lead_sec, const char *suffix) {
   ConcatString out_nc_file;

   // Create output NetCDF file name
   build_outfile_name(ens_valid_ut, suffix, out_nc_file);

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_nc_file, NcFile::Replace);

   if(!nc_out->is_valid()) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_nc_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_nc_file.text(), program_name);

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = nc_out->add_dim("lat",   (long) grid.ny());
   lon_dim = nc_out->add_dim("lon",   (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, lat_dim, lon_dim, grid);

   // Append to the list of output files
   out_nc_file_list.add(out_nc_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_txt_files() {
   int  i, max_col;
   ConcatString tmp_str;

   // Check to see if the text files have already been set up
   if(stat_at.nrows() > 0 || stat_at.ncols() > 0) return;

   // Create output file names for the stat file and optional text files
   build_outfile_name(ens_valid_ut, "", tmp_str);

   /////////////////////////////////////////////////////////////////////
   //
   // Setup the output STAT file
   //
   /////////////////////////////////////////////////////////////////////

   // Get the maximum number of data columns
   max_col =  max(get_n_orank_columns(n_ens+1),
                  get_n_rhist_columns(n_ens));
   max_col =  max(max_col, n_ssvar_columns);
   max_col += n_header_columns;

   // Initialize file stream
   stat_out = (ofstream *) 0;

   // Build the file name
   stat_file << tmp_str << stat_file_ext;

   // Create the output STAT file
   open_txt_file(stat_out, stat_file);
   
   // Setup the STAT AsciiTable
   stat_at.set_size(conf_info.n_stat_row() + 1, max_col);
   setup_table(stat_at);

   // Write the text header row
   write_header_row((const char **) 0, 0, 1, stat_at, 0, 0);

   // Initialize the row index to 1 to account for the header
   i_stat_row = 1;

   /////////////////////////////////////////////////////////////////////
   //
   // Setup each of the optional output text files
   //
   /////////////////////////////////////////////////////////////////////

   // Loop through output line type
   for(i=0; i<n_txt; i++) {

      // Only set it up if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Only create ORANK file when using point observations
         if(i == i_orank && !point_obs_flag) continue;

         // Initialize file stream
         txt_out[i] = (ofstream *) 0;

         // Build the file name
         txt_file[i] << tmp_str << "_" << txt_file_abbr[i]
                     << txt_file_ext;

         // Create the output text file
         open_txt_file(txt_out[i], txt_file[i]);

         // Get the maximum number of columns for this line type
         switch(i) {

            case(i_rhist):
               max_col = get_n_rhist_columns(n_rank) + n_header_columns + 1;
               break;

            case(i_orank):
               max_col = get_n_orank_columns(n_ens) + n_header_columns + 1;
               break;

            default:
               max_col = n_txt_columns[i]  + n_header_columns + 1;
               break;
         } // end switch

         // Setup the text AsciiTable
         txt_at[i].set_size(conf_info.n_txt_row(i) + 1, max_col);
         setup_table(txt_at[i]);

         // Write the text header row
         switch(i) {

            case(i_rhist):
               write_rhist_header_row(1, n_rank, txt_at[i], 0, 0);
               break;

            case(i_orank):
               write_orank_header_row(1, n_ens, txt_at[i], 0, 0);
               break;

            default:
               write_header_row(txt_columns[i], n_txt_columns[i], 1,
                  txt_at[i], 0, 0);
               break;
         } // end switch

         // Initialize the row index to 1 to account for the header
         i_txt_row[i] = 1;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Left-justify all columns
   at.set_table_just(LeftJust);

   // Set the precision
   at.set_precision(default_precision);

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

void build_outfile_name(unixtime ut, const char *suffix, ConcatString &str) {
   int mon, day, yr, hr, min, sec;
   char tmp_str[max_str_len];

   //
   // Create output file name
   //

   // Append the output directory and program name
   str << cs_erase << out_dir.text() << "/" << program_name;

   // Append the output prefix, if defined
   if(conf_info.output_prefix.nonempty())
      str << "_" << conf_info.output_prefix;

   // Append the timing information
   unix_to_mdyhms(ut, mon, day, yr, hr, min, sec);
   sprintf(tmp_str, "%.4i%.2i%.2i_%.2i%.2i%.2iV",
           yr, mon, day, hr, min, sec);
   str << "_" << tmp_str;

   // Append the suffix
   str << suffix;

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_nc(int i_vx, DataPlane &dp) {
   int i, j;
   double t, v;
   char thresh_str[max_str_len], type_str[max_str_len];


   // Arrays for storing ensemble data
   float *ens_mean  = (float *) 0;
   float *ens_stdev = (float *) 0;
   float *ens_minus = (float *) 0;
   float *ens_plus  = (float *) 0;
   float *ens_min   = (float *) 0;
   float *ens_max   = (float *) 0;
   float *ens_range = (float *) 0;
   int   *ens_vld   = (int   *) 0;
   float *ens_freq  = (float *) 0;

   // Allocate memory for storing ensemble data
   ens_mean  = new float [grid.nx()*grid.ny()];
   ens_stdev = new float [grid.nx()*grid.ny()];
   ens_minus = new float [grid.nx()*grid.ny()];
   ens_plus  = new float [grid.nx()*grid.ny()];
   ens_min   = new float [grid.nx()*grid.ny()];
   ens_max   = new float [grid.nx()*grid.ny()];
   ens_range = new float [grid.nx()*grid.ny()];
   ens_vld   = new int   [grid.nx()*grid.ny()];
   ens_freq  = new float [grid.nx()*grid.ny()];

   // Store the threshold for the ratio of valid data points
   t = conf_info.vld_data_thresh;

   // Store the data
   for(i=0; i<count_na.n_elements(); i++) {

      // Valid data count
      ens_vld[i] = nint(count_na[i]);

      // Check for too much missing data
      if((double) (count_na[i]/n_ens_vld[i_vx]) < t) {
         ens_mean[i]  = bad_data_float;
         ens_stdev[i] = bad_data_float;
         ens_minus[i] = bad_data_float;
         ens_plus[i]  = bad_data_float;
         ens_min[i]   = bad_data_float;
         ens_max[i]   = bad_data_float;
         ens_range[i] = bad_data_float;
      }
      else {

         // Compute ensemble summary
         ens_mean[i]  = (float) (sum_na[i]/count_na[i]);
         ens_stdev[i] = (float) compute_stdev(sum_na[i], sum_sq_na[i], ens_vld[i]);
         ens_minus[i] = (float) ens_mean[i] - ens_stdev[i];
         ens_plus[i]  = (float) ens_mean[i] + ens_stdev[i];
         ens_min[i]   = (float) min_na[i];
         ens_max[i]   = (float) max_na[i];
         v = max_na[i] - min_na[i];
         if(is_eq(v, 0.0)) v = 0;
         ens_range[i] = (float) v;
      }
   } // end for i

   // Add the ensemble mean if requested
   if(conf_info.ensemble_flag[i_nc_mean]) {
      write_ens_var_float(i_vx, ens_mean, dp,
                          "ENS_MEAN",
                          "Ensemble Mean");
   }

   // Add the ensemble standard deviation if requested
   if(conf_info.ensemble_flag[i_nc_stdev]) {
      write_ens_var_float(i_vx, ens_stdev, dp,
                          "ENS_STDEV",
                          "Ensemble Standard Deviation");
   }

   // Add the ensemble mean minus one standard deviation if requested
   if(conf_info.ensemble_flag[i_nc_minus]) {
      write_ens_var_float(i_vx, ens_minus, dp,
                          "ENS_MINUS",
                          "Ensemble Mean Minus 1 Standard Deviation");
   }

   // Add the ensemble mean plus one standard deviation if requested
   if(conf_info.ensemble_flag[i_nc_plus]) {
      write_ens_var_float(i_vx, ens_plus, dp,
                          "ENS_PLUS",
                          "Ensemble Mean Plus 1 Standard Deviation");
   }

   // Add the ensemble minimum value if requested
   if(conf_info.ensemble_flag[i_nc_min]) {
      write_ens_var_float(i_vx, ens_min, dp,
                          "ENS_MIN",
                          "Ensemble Minimum");
   }

   // Add the ensemble maximum value if requested
   if(conf_info.ensemble_flag[i_nc_max]) {
      write_ens_var_float(i_vx, ens_max, dp,
                          "ENS_MAX",
                          "Ensemble Maximum");
   }

   // Add the ensemble range if requested
   if(conf_info.ensemble_flag[i_nc_range]) {
      write_ens_var_float(i_vx, ens_range, dp,
                          "ENS_RANGE",
                          "Ensemble Range");
   }

   // Add the ensemble valid data count if requested
   if(conf_info.ensemble_flag[i_nc_vld]) {
      write_ens_var_int(i_vx, ens_vld, dp,
                        "ENS_VLD",
                        "Ensemble Valid Data Count");
   }

   // Add the ensemble relative frequencies if requested
   if(conf_info.ensemble_flag[i_nc_freq]) {

      // Loop through each threshold
      for(i=0; i<conf_info.ens_ta[i_vx].n_elements(); i++) {

         conf_info.ens_ta[i_vx][i].get_abbr_str(thresh_str);
         sprintf(type_str, "ENS_FREQ_%s", thresh_str);

         // Store the data
         for(j=0; j<count_na.n_elements(); j++) {

            // Check for too much missing data
            if((double) (count_na[j]/n_ens_vld[i_vx]) < t) {
               ens_freq[j] = bad_data_float;
            }
            else {
               ens_freq[j] = (float) (thresh_count_na[i][j]/count_na[j]);
            }

         } // end for j

         // Write the ensemble relative frequency
         write_ens_var_float(i_vx, ens_freq, dp,
                             type_str,
                             "Ensemble Relative Frequency");

      } // end for i
   } // end if

   // Deallocate and clean up
   if(ens_mean)  { delete [] ens_mean;  ens_mean  = (float *) 0; }
   if(ens_stdev) { delete [] ens_stdev; ens_stdev = (float *) 0; }
   if(ens_minus) { delete [] ens_minus; ens_minus = (float *) 0; }
   if(ens_plus)  { delete [] ens_plus;  ens_plus  = (float *) 0; }
   if(ens_min)   { delete [] ens_min;   ens_min   = (float *) 0; }
   if(ens_max)   { delete [] ens_max;   ens_max   = (float *) 0; }
   if(ens_range) { delete [] ens_range; ens_range = (float *) 0; }
   if(ens_vld)   { delete [] ens_vld;   ens_vld   = (int   *) 0; }
   if(ens_freq)  { delete [] ens_freq;  ens_freq  = (float *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_float(int i_vx, float *ens_data, DataPlane &dp,
                         const char *type_str,
                         const char *long_name_str) {
   NcVar *ens_var = (NcVar *) 0;
   ConcatString ens_var_name, name_str;

   // Construct the variable name
   ens_var_name << cs_erase
                << conf_info.ens_info[i_vx]->name() << "_"
                << conf_info.ens_info[i_vx]->level_name() << "_"
                << type_str;
   ens_var = nc_out->add_var(ens_var_name, ncFloat, lat_dim, lon_dim);

   //
   // Construct the variable name attribute
   // For the ensemble mean, just use the variable name.
   // For all other fields, append the field type.
   //
   if(strcmp(type_str, "ENS_MEAN") == 0) {
      name_str << cs_erase
               << conf_info.ens_info[i_vx]->name();
   }
   else {
      name_str << cs_erase
               << conf_info.ens_info[i_vx]->name() << "_"
               << type_str;
   }
   
   // Add the variable attributes
   add_var_att(i_vx, ens_var, false, dp, name_str, long_name_str);

   // Write the data
   if(!ens_var->put(&ens_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_ens_var_float() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_ens_var_int(int i_vx, int *ens_data, DataPlane &dp,
                       const char *type_str,
                       const char *long_name_str) {
   NcVar *ens_var = (NcVar *) 0;
   ConcatString ens_var_name, name_str;

   // Construct the variable name
   ens_var_name << cs_erase
                << conf_info.ens_info[i_vx]->name() << "_"
                << conf_info.ens_info[i_vx]->level_name() << "_"
                << type_str;
   ens_var = nc_out->add_var(ens_var_name, ncInt, lat_dim, lon_dim);

   // Construct the variable name attribute
   name_str << cs_erase
            << conf_info.ens_info[i_vx]->name() << "_"
            << type_str;
   
   // Add the variable attributes
   add_var_att(i_vx, ens_var, true, dp, name_str, long_name_str);

   // Write the data
   if(!ens_var->put(&ens_data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_ens_var_int() -> "
           << "error in ens_var->put for the " << ens_var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_nc(PairDataEnsemble &pd, DataPlane &dp,
                    int i_vx, int i_interp, int i_mask) {
   int i, n;

   // Arrays for storing observation rank data
   float *obs_v    = (float *) 0;
   int   *obs_rank = (int *)   0;
   float *obs_pit  = (float *) 0;
   int   *ens_vld  = (int *)   0;

   // Allocate memory for storing ensemble data
   obs_v    = new float [grid.nx()*grid.ny()];
   obs_rank = new int   [grid.nx()*grid.ny()];
   obs_pit  = new float [grid.nx()*grid.ny()];
   ens_vld  = new int   [grid.nx()*grid.ny()];

   // Initialize
   for(i=0; i<grid.nx()*grid.ny(); i++) {
      obs_v[i]    = bad_data_float;
      obs_rank[i] = bad_data_int;
      obs_pit[i]  = bad_data_float;
      ens_vld[i]  = bad_data_int;
   }

   // Loop over all the pairs
   for(i=0; i<pd.n_pair; i++) {
     
      n = DefaultTO.two_to_one(grid.nx(), grid.ny(),
                               nint(pd.x_na[i]), nint(pd.y_na[i]));

      // Store the observation value, rank, and number of valid ensembles
      obs_v[n]    = (float) pd.o_na[i];
      obs_rank[n] = nint(pd.r_na[i]);
      obs_pit[n]  = (float) pd.pit_na[i];
      ens_vld[n]  = nint(pd.v_na[i]);

   } // end for i

   // Add the observation values
   write_orank_var_float(i_vx, i_interp, i_mask, obs_v, dp,
                         "OBS",
                         "Observation Value");

   // Add the observation ranks
   write_orank_var_int(i_vx, i_interp, i_mask, obs_rank, dp,
                       "OBS_RANK",
                       "Observation Rank");

   // Add the probability integral transforms
   write_orank_var_float(i_vx, i_interp, i_mask, obs_pit, dp,
                         "OBS_PIT",
                         "Probability Integral Transform");

   // Add the number of valid ensemble members
   write_orank_var_int(i_vx, i_interp, i_mask, ens_vld, dp,
                       "ENS_VLD",
                       "Ensemble Valid Data Count");

   // Deallocate and clean up
   if(obs_v)    { delete [] obs_v;    obs_v    = (float *) 0; }
   if(obs_rank) { delete [] obs_rank; obs_rank = (int   *) 0; }
   if(obs_pit)  { delete [] obs_pit;  obs_pit  = (float *) 0; }
   if(ens_vld)  { delete [] ens_vld;  ens_vld  = (int   *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_var_float(int i_vx, int i_interp, int i_mask,
                           float *data, DataPlane &dp,
                           const char *type_str,
                           const char *long_name_str) {
   NcVar *nc_var = (NcVar *) 0;
   int wdth;
   ConcatString mthd_str, var_name, name_str;

   // Get the interpolation method string and width
   mthd_str = interpmthd_to_string(conf_info.interp_mthd[i_interp]);
   wdth     = conf_info.interp_wdth[i_interp];

   // Build the orank variable name
   var_name << cs_erase
            << conf_info.vx_pd[i_vx].obs_info->name() << "_"
            << conf_info.vx_pd[i_vx].obs_info->level_name() << "_"
            << type_str << "_"
            << conf_info.mask_name[i_mask];

   // Construct the variable name attribute
   name_str << cs_erase
            << conf_info.ens_info[i_vx]->name() << "_"
            << type_str << "_"
            << conf_info.mask_name[i_mask];

   // Append smoothing information
   if((wdth > 1) &&
      (conf_info.interp_field == FieldType_Obs ||
       conf_info.interp_field == FieldType_Both)) {
      var_name << "_" << mthd_str << "_" << wdth*wdth;
      name_str << "_" << mthd_str << "_" << wdth*wdth;
   }

   // Define the variable
   nc_var = nc_out->add_var(var_name, ncFloat, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att(i_vx, nc_var, false, dp, name_str, long_name_str);

   // Write the data
   if(!nc_var->put(&data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_orank_var_float() -> "
           << "error in nc_var->put for the " << var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_orank_var_int(int i_vx, int i_interp, int i_mask,
                         int *data, DataPlane &dp,
                         const char *type_str,
                         const char *long_name_str) {
   NcVar *nc_var = (NcVar *) 0;
   int wdth;
   ConcatString mthd_str, var_name, name_str;

   // Get the interpolation method string and width
   mthd_str = interpmthd_to_string(conf_info.interp_mthd[i_interp]);
   wdth     = conf_info.interp_wdth[i_interp];

   // Build the orank variable name
   var_name << cs_erase
            << conf_info.vx_pd[i_vx].obs_info->name() << "_"
            << conf_info.vx_pd[i_vx].obs_info->level_name() << "_"
            << type_str << "_"
            << conf_info.mask_name[i_mask];

   // Construct the variable name attribute
   name_str << cs_erase
            << conf_info.ens_info[i_vx]->name() << "_"
            << type_str << "_"
            << conf_info.mask_name[i_mask];

   // Append smoothing information
   if((wdth > 1) &&
      (conf_info.interp_field == FieldType_Obs ||
       conf_info.interp_field == FieldType_Both)) {
      var_name << "_" << mthd_str << "_" << wdth*wdth;
      name_str << "_" << mthd_str << "_" << wdth*wdth;
   }

   // Define the variable
   nc_var = nc_out->add_var(var_name, ncInt, lat_dim, lon_dim);

   // Add the variable attributes
   add_var_att(i_vx, nc_var, true, dp, name_str, long_name_str);

   // Write the data
   if(!nc_var->put(&data[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nwrite_orank_var_int() -> "
           << "error in nc_var->put for the " << var_name
           << " field.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void add_var_att(int i_vx, NcVar *nc_var, bool is_int, DataPlane &dp,
                 const char *name_str, const char *long_name_str) {
   ConcatString att_str;

   // Construct the long name
   att_str << cs_erase
           << conf_info.ens_info[i_vx]->name() << " at "
           << conf_info.ens_info[i_vx]->level_name() << " "
           << long_name_str;

   // Add variable attributes
   nc_var->add_att("name", name_str);
   nc_var->add_att("long_name", att_str);
   nc_var->add_att("level", conf_info.ens_info[i_vx]->level_name());
   nc_var->add_att("units", conf_info.ens_info[i_vx]->units());

   if(is_int) nc_var->add_att("_FillValue", bad_data_int);
   else       nc_var->add_att("_FillValue", bad_data_float);

   // Write out times
   write_netcdf_var_times(nc_var, dp);

   return;
}

////////////////////////////////////////////////////////////////////////

void finish_txt_files() {
   int i;

   // Write out the contents of the STAT AsciiTable and
   // close the STAT output files
   if(stat_out) {
      *stat_out << stat_at;
      close_txt_file(stat_out, stat_file);
   }

   // Finish up each of the optional text files
   for(i=0; i<n_txt; i++) {

      // Only write the table if requested in the config file
      if(conf_info.output_flag[i] == STATOutputType_Both) {

         // Write the AsciiTable to a file
         if(txt_out[i]) {
            *txt_out[i] << txt_at[i];
            close_txt_file(txt_out[i], txt_file[i]);
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {
   int i;

   mlog << Debug(2) << "\n" << sep_str << "\n\n";

   // Close the output text files that were open for writing
   if(vx_flag) finish_txt_files();

   // List the output NetCDF files
   for(i=0; i<out_nc_file_list.n_elements(); i++) {
      mlog << Debug(1)
           << "Output file: " << out_nc_file_list[i] << "\n";
   }

   // Deallocate threshold count arrays
   if(thresh_count_na) {
      for(i=0; i<conf_info.get_max_n_thresh(); i++) {
         thresh_count_na[i].clear();
      }
      delete [] thresh_count_na; thresh_count_na = (NumArray *) 0;
   }

   // Deallocate memory for data files
   if(ens_mtddf) { delete ens_mtddf; ens_mtddf = (Met2dDataFile *) 0; }
   if(obs_mtddf) { delete obs_mtddf; obs_mtddf = (Met2dDataFile *) 0; }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tn_ens ens_file_1 ... ens_file_n | ens_file_list\n"
        << "\tconfig_file\n"
        << "\t[-grid_obs file]\n"
        << "\t[-point_obs file]\n"
        << "\t[-ssvar_mean file]\n"
        << "\t[-obs_valid_beg time]\n"
        << "\t[-obs_valid_end time]\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"n_ens ens_file_1 ... ens_file_n\" is the number "
        << "of ensemble members followed by a list of ensemble member "
        << "file names (required).\n"

        << "\t\t\"ens_file_list\" is an ASCII file containing a list "
        << "of ensemble member file names (required).\n"

        << "\t\t\"config_file\" is an EnsembleStatConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-grid_obs file\" specifies a gridded observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-point_obs file\" specifies a NetCDF point observation file. "
        << "May be used multiple times (optional).\n"

        << "\t\t\"-ssvar_mean file\" specifies an ensemble mean model data file. "
        << "Used in conjunction with the SSVAR output line type.\n"

        << "\t\t\"-obs_valid_beg time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "beginning of the matching time window (optional).\n"

        << "\t\t\"-obs_valid_end time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "end of the matching time window (optional).\n"

        << "\t\t\"-outdir path\" overrides the default output directory "
        << "(" << out_dir << ") (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\n\tNOTE: The ensemble members and gridded observations "
        << "must be on the same grid.\n\n";

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grid_obs(const StringArray & a)
{
   grid_obs_file_list.add(a[0]);
   grid_obs_flag = 1;
}

////////////////////////////////////////////////////////////////////////

void set_point_obs(const StringArray & a)
{
   point_obs_file_list.add(a[0]);
   point_obs_flag = 1;
}

////////////////////////////////////////////////////////////////////////

void set_ssvar_mean(const StringArray & a)
{
   ens_ssvar_mean = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_beg(const StringArray & a)
{
   obs_valid_beg_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_obs_valid_end(const StringArray & a)
{
   obs_valid_end_ut = timestring_to_unix(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a)
{
   out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];
   
   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
