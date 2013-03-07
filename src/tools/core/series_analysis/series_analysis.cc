// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   series_analysis.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/10/12  Halley Gotway   New
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

#include "series_analysis.h"

#include "vx_statistics.h"
#include "vx_nc_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_scores();

static void get_series_data(int, VarInfo *, VarInfo *,
                            DataPlane &, DataPlane &);
static void get_series_entry(int, VarInfo *,
                             const ConcatString &, const GrdFileType,
                             StringArray &, DataPlane &);
static void get_series_entry(int, VarInfo *,
                             const StringArray &, const GrdFileType,
                             StringArray &, DataPlane &);

static void do_cts (int, const NumArray &, const NumArray &);
static void do_mcts(int, const NumArray &, const NumArray &);
static void do_cnt (int, const NumArray &, const NumArray &);
static void do_pct (int, const NumArray &, const NumArray &);

static void store_stat_fho  (int, const ConcatString &, const CTSInfo &);
static void store_stat_ctc  (int, const ConcatString &, const CTSInfo &);
static void store_stat_cts  (int, const ConcatString &, const CTSInfo &);
static void store_stat_mctc (int, const ConcatString &, const MCTSInfo &);
static void store_stat_mcts (int, const ConcatString &, const MCTSInfo &);
static void store_stat_cnt  (int, const ConcatString &, const CNTInfo &);
static void store_stat_sl1l2(int, const ConcatString &, const CNTInfo &);
static void store_stat_pct  (int, const ConcatString &, const PCTInfo &);
static void store_stat_pstd (int, const ConcatString &, const PCTInfo &);
static void store_stat_pjc  (int, const ConcatString &, const PCTInfo &);
static void store_stat_prc  (int, const ConcatString &, const PCTInfo &);
  
static void setup_nc_file(const VarInfo *, const VarInfo *);
static void add_nc_var(const ConcatString &, const ConcatString &,
                       const ConcatString &, const ConcatString &,
                       const ConcatString &, double);
static void put_nc_val(int, const ConcatString &, float);

static void set_range(const unixtime &, unixtime &, unixtime &);
static void set_range(const int &, int &, int &);
         
static void clean_up();

static void usage();
static void set_fcst_files(const StringArray &);
static void set_obs_files(const StringArray &);
static void set_both_files(const StringArray &);
static void set_out_file(const StringArray &);
static void set_config_file(const StringArray &);
static void set_log_file(const StringArray &);
static void set_verbosity(const StringArray &);
static void parse_file_list(const StringArray &, StringArray &);
static void parse_ascii_file_list(const char *, StringArray &);
static void parse_long_names();

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Compute the scores and write them out
   process_scores();

   // Close the text files and deallocate memory
   clean_up();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;
   CommandLine cline;
   ConcatString default_config_file;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_fcst_files,  "-fcst",  -1);
   cline.add(set_obs_files,   "-obs",   -1);
   cline.add(set_both_files,  "-both",  -1);
   cline.add(set_config_file, "-config", 1);
   cline.add(set_out_file,    "-out",    1);
   cline.add(set_log_file,    "-log",    1);
   cline.add(set_verbosity,   "-v",      1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be zero arguments left.
   if(cline.n() != 0) usage();

   // Check that the required arguments have been set.
   if(fcst_files.n_elements() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the forecast file list must be set using the "
           << "\"-fcst\" or \"-both\" option.\n\n";
      usage();
   }
   if(obs_files.n_elements() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the observation file list must be set using the "
           << "\"-obs\" or \"-both\" option.\n\n";
      usage();
   }
   if(config_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the configuration file must be set using the "
           << "\"-config\" option.\n\n";
      usage();
   }
   if(out_file.length() == 0) {
      mlog << Error << "\nprocess_command_line() -> "
           << "the output NetCDF file must be set using the "
           << "\"-out\" option.\n\n";
      usage();
   }

   // Create the default config file name
   default_config_file = replace_path(default_config_filename);

   // List the config files
   mlog << Debug(1)
        << "Default Config File: " << default_config_file << "\n"
        << "User Config File: "    << config_file << "\n";

   // Read the config files
   conf_info.read_config(default_config_file, config_file);

   // Get the forecast and observation file types from config, if present
   ftype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_fcst));
   otype = parse_conf_file_type(conf_info.conf.lookup_dictionary(conf_key_obs));

   // Read forecast file
   if(!(fcst_mtddf = mtddf_factory.new_met_2d_data_file(fcst_files[0], ftype))) {
      mlog << Error << "\nTrouble reading forecast file \""
           << fcst_files[0] << "\"\n\n";
      exit(1);
   }

   // Read observation file
   if(!(obs_mtddf = mtddf_factory.new_met_2d_data_file(obs_files[0], otype))) {
      mlog << Error << "\nTrouble reading observation file \""
           << obs_files[0] << "\"\n\n";
      exit(1);
   }

   // Store the input data file types
   ftype = fcst_mtddf->file_type();
   otype = obs_mtddf->file_type();

   // Process the configuration
   conf_info.process_config(ftype, otype);

   // Check that the grids match
   if(!(fcst_mtddf->grid() == obs_mtddf->grid())) {

      mlog << Error << "\nprocess_command_line() -> "
           << "The forecast and observation grids do not match: "
           << fcst_mtddf->grid().serialize() << " != "
           << obs_mtddf->grid().serialize() << "\n\n";
      exit(1);
   }
   // If they do, store the grid
   else {
      grid = fcst_mtddf->grid();
      conf_info.process_masks(grid);
   }

   // Set the random number generator and seed value to be used when
   // computing bootstrap confidence intervals
   rng_set(rng_ptr, conf_info.boot_rng, conf_info.boot_seed);

   // List the lengths of the series options
   mlog << Debug(1)
        << "Length of configuration \"fcst.field\" = " << conf_info.get_n_fcst() << "\n"
        << "Length of configuration \"obs.field\"  = " << conf_info.get_n_obs() << "\n"
        << "Length of forecast file list         = " << fcst_files.n_elements() << "\n"
        << "Length of observation file list      = " << obs_files.n_elements() << "\n";

   // Deteremine the length of the series to be analyzed.  Series is
   // defined by the first parameter of length greater than one:
   // - Configuration fcst.field
   // - Configuration obs.field
   // - Forecast file list
   // - Observation file list
   if(conf_info.get_n_fcst() > 1) {
      series_type = SeriesType_Fcst_Conf;
      n_series = conf_info.get_n_fcst();
      mlog << Debug(1)
           << "Series defined by the \"fcst.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(conf_info.get_n_obs() > 1) {
      series_type = SeriesType_Obs_Conf;
      n_series = conf_info.get_n_obs();
      mlog << Debug(1)
           << "Series defined by the \"obs.field\" configuration entry "
           << "of length " << n_series << ".\n";
   }
   else if(fcst_files.n_elements() > 1) {
      series_type = SeriesType_Fcst_Files;
      n_series = fcst_files.n_elements();
      mlog << Debug(1)
           << "Series defined by the forecast file list of length "
           << n_series << ".\n";
   }
   else if(obs_files.n_elements() > 1) {
      series_type = SeriesType_Obs_Files;
      n_series = obs_files.n_elements();
      mlog << Debug(1)
           << "Series defined by the observation file list of length "
           << n_series << ".\n";
   }
   else {
      mlog << Error << "\nprocess_command_line() -> "
           << "cannot define series when the \"fcst.field\" and "
           << "\"obs.field\" configuration entries and the \"-fcst\" and "
           << "\"-obs\" command line options all have length one.\n\n";
      exit(1);
   }

   // Initialize the series file names
   for(i=0; i<n_series; i++) {
      found_fcst_files.add("");
      found_obs_files.add("");
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_scores() {
   int nxny, i, x, y, i_read, n_reads, i_series, i_point;
   VarInfo *fcst_info = (VarInfo *) 0;
   VarInfo *obs_info  = (VarInfo *) 0;
   NumArray *f_na = (NumArray *) 0;
   NumArray *o_na = (NumArray *) 0;
   DataPlane fcst_dp, obs_dp;

   // Determine the block size
   nxny    = grid.nx() * grid.ny();
   n_reads = nint(ceil((double) nxny / conf_info.block_size));

   // Allocate space to store the pairs for each grid point
   f_na = new NumArray [conf_info.block_size];
   o_na = new NumArray [conf_info.block_size];

   mlog << Debug(2)
        << "Computing statistics using a block size of "
        << conf_info.block_size << ", requiring " << n_reads
        << " passes through the " << grid.nx() << " x "
        << grid.ny() << " grid.\n";

   // Loop over the data reads
   for(i_read=0; i_read<n_reads; i_read++) {
     
      // Starting grid point
      i_point = i_read*conf_info.block_size;

      mlog << Debug(2)
           << "Processing data pass number " << i_read + 1 << " of "
           << n_reads << " for grid points " << i_point + 1 << " to "
           << min(i_point + conf_info.block_size, nxny) << ".\n";

      // Loop over the series variable
      for(i_series=0; i_series<n_series; i_series++) {

         // Store the current VarInfo objects
         fcst_info = (conf_info.get_n_fcst() > 1 ?
                      conf_info.fcst_info[i_series] :
                      conf_info.fcst_info[0]);
         obs_info  = (conf_info.get_n_obs() > 1 ?
                      conf_info.obs_info[i_series] :
                      conf_info.obs_info[0]);

         // Retrieve the data planes for the current series entry
         get_series_data(i_series, fcst_info, obs_info, fcst_dp, obs_dp);

         // Setup the output NetCDF file on the first pass
         if(nc_out == (NcFile *) 0) setup_nc_file(fcst_info, obs_info);

         // Update timing info
         set_range(fcst_dp.init(),  fcst_init_beg,  fcst_init_end);
         set_range(fcst_dp.valid(), fcst_valid_beg, fcst_valid_end);
         set_range(fcst_dp.lead(),  fcst_lead_beg,  fcst_lead_end);
         set_range(obs_dp.init(),   obs_init_beg,   obs_init_end);
         set_range(obs_dp.valid(),  obs_valid_beg,  obs_valid_end);
         set_range(obs_dp.lead(),   obs_lead_beg,   obs_lead_end);
                          
         // Store matched pairs for each grid point
         for(i=0; i<conf_info.block_size && (i_point+i)<nxny; i++) {

            // Convert n to x, y
            DefaultTO.one_to_two(grid.nx(), grid.ny(), i_point+i, x, y);

            // Store valid fcst/obs pairs where the mask is on
            if(!is_bad_data(fcst_dp(x,y)) &&
               !is_bad_data(obs_dp(x,y)) &&
               !is_bad_data(conf_info.mask_dp(x,y))) {
               f_na[i].add(fcst_dp(x, y));
               o_na[i].add(obs_dp(x, y));
            }

         } // end for i

      } // end for i_series

      // Compute statistics for each grid point in the block
      for(i=0; i<conf_info.block_size && (i_point+i)<nxny; i++) {

         // Determine x,y location
         DefaultTO.one_to_two(grid.nx(), grid.ny(), i_point+i, x, y);
        
         // Check for the required number of matched pairs
         if(f_na[i].n_elements()/(double) n_series < conf_info.vld_data_thresh) {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Skipping point (" << x << ", " << y << ") with "
                 << f_na[i].n_elements() << " matched pairs.\n";
            continue;
         }
         else {
            mlog << Debug(4)
                 << "[" << i+1 << " of " << conf_info.block_size
                 << "] Processing point (" << x << ", " << y << ") with "
                 << f_na[i].n_elements() << " matched pairs.\n";
         }
        
         // Compute contingency table counts and statistics
         if(!conf_info.fcst_info[0]->p_flag() &&
            (conf_info.output_stats[stat_fho].n_elements() +
             conf_info.output_stats[stat_ctc].n_elements() +
             conf_info.output_stats[stat_cts].n_elements()) > 0) {
            do_cts(i_point+i, f_na[i], o_na[i]);
         }

         // Compute multi-category contingency table counts and statistics
         if(!conf_info.fcst_info[0]->p_flag() &&
            (conf_info.output_stats[stat_mctc].n_elements() +
             conf_info.output_stats[stat_mcts].n_elements()) > 0) {
            do_mcts(i_point+i, f_na[i], o_na[i]);
         }

         // Compute continuous statistics
         if(!conf_info.fcst_info[0]->p_flag() &&
            (conf_info.output_stats[stat_cnt].n_elements() +
             conf_info.output_stats[stat_sl1l2].n_elements()) > 0) {
            do_cnt(i_point+i, f_na[i], o_na[i]);
         }

         // Compute probabilistics counts and statistics
         if(conf_info.fcst_info[0]->p_flag() &&
            (conf_info.output_stats[stat_pct].n_elements() +
             conf_info.output_stats[stat_pstd].n_elements() +
             conf_info.output_stats[stat_pjc].n_elements() +
             conf_info.output_stats[stat_prc].n_elements()) > 0) {
            do_pct(i_point+i, f_na[i], o_na[i]);
         }
      } // end for i

      // Empty out the NumArray objects
      for(i=0; i<conf_info.block_size; i++) {
         f_na[i].empty();
         o_na[i].empty();
      }
     
   } // end for i_read

   // Add time range information to the global NetCDF attributes
   nc_out->add_att("fcst_init_beg",  unix_to_yyyymmdd_hhmmss(fcst_init_beg));
   nc_out->add_att("fcst_init_end",  unix_to_yyyymmdd_hhmmss(fcst_init_end));
   nc_out->add_att("fcst_valid_beg", unix_to_yyyymmdd_hhmmss(fcst_valid_beg));
   nc_out->add_att("fcst_valid_end", unix_to_yyyymmdd_hhmmss(fcst_valid_end));
   nc_out->add_att("fcst_lead_beg",  sec_to_hhmmss(fcst_lead_beg));
   nc_out->add_att("fcst_lead_end",  sec_to_hhmmss(fcst_lead_end));
   nc_out->add_att("obs_init_beg",   unix_to_yyyymmdd_hhmmss(obs_init_beg));
   nc_out->add_att("obs_init_end",   unix_to_yyyymmdd_hhmmss(obs_init_end));
   nc_out->add_att("obs_valid_beg",  unix_to_yyyymmdd_hhmmss(obs_valid_beg));
   nc_out->add_att("obs_valid_end",  unix_to_yyyymmdd_hhmmss(obs_valid_end));
   nc_out->add_att("obs_lead_beg",   sec_to_hhmmss(obs_lead_beg));
   nc_out->add_att("obs_lead_end",   sec_to_hhmmss(obs_lead_end));
   
   // Deallocate and clean up
   if(f_na) { delete [] f_na; f_na = (NumArray *) 0; }
   if(o_na) { delete [] o_na; o_na = (NumArray *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void get_series_data(int i_series,
                     VarInfo *fcst_info, VarInfo *obs_info,
                     DataPlane &fcst_dp, DataPlane &obs_dp) {
  
   mlog << Debug(2)
        << "Processing series entry " << i_series + 1 << " of "
        << n_series << ": " << fcst_info->magic_str()
        << " versus " << obs_info->magic_str() << "\n";
  
   // Switch on the series type
   switch(series_type) {

      case SeriesType_Fcst_Conf:
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp);
         if(conf_info.get_n_obs() == 1) {
            obs_info->set_valid(fcst_dp.valid());
         }
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp);
         break;

      case SeriesType_Obs_Conf:
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp);
         if(conf_info.get_n_fcst() == 1) {
            fcst_info->set_valid(obs_dp.valid());
         }
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp);
         break;
         
      case SeriesType_Fcst_Files:
         get_series_entry(i_series, fcst_info, fcst_files[i_series],
                          ftype, found_fcst_files, fcst_dp);
         obs_info->set_valid(fcst_dp.valid());
         get_series_entry(i_series, obs_info, obs_files,
                          otype, found_obs_files, obs_dp);
         break;

      case SeriesType_Obs_Files:
         get_series_entry(i_series, obs_info, obs_files[i_series],
                          otype, found_obs_files, obs_dp);
         fcst_info->set_valid(obs_dp.valid());
         get_series_entry(i_series, fcst_info, fcst_files,
                          ftype, found_fcst_files, fcst_dp);
         break;

      default:
         mlog << Error << "\nget_series_data() -> "
              << "unexpected SeriesType value: "
              << series_type << "\n\n";
         exit(1);
         break;
   }

   // For probability fields, check to see if they need to be
   // rescaled from [0, 100] to [0, 1]
   if(conf_info.fcst_info[0]->p_flag()) rescale_probability(fcst_dp);

   return;
}

////////////////////////////////////////////////////////////////////////

void get_series_entry(int i_series, VarInfo *info,
                      const ConcatString &file,
                      const GrdFileType type,
                      StringArray &found_files, DataPlane &dp) {
   StringArray search_files;

   search_files.add(file);

   get_series_entry(i_series, info, search_files, type, found_files, dp);

   return;
}

////////////////////////////////////////////////////////////////////////

void get_series_entry(int i_series, VarInfo *info,
                      const StringArray &search_files,
                      const GrdFileType type,
                      StringArray &found_files, DataPlane &dp) {
   int i, j;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   ConcatString cur_file;
   bool found = false;

   // Search for match in the file list
   for(i=0; i<search_files.n_elements(); i++) {

      // Start the search with the value of i_series
      j = (i_series + i) % search_files.n_elements();

      // Check if the file for this series entry has already been found
      if(strlen(found_files[i_series]) > 0)
         cur_file = found_files[i_series];
      else
         cur_file = search_files[j];

      mlog << Debug(3)
           << "Searching file " << cur_file << "\n";
      
      // Open the data file
      mtddf = mtddf_factory.new_met_2d_data_file(cur_file, type);

      // Check that the grid does not change
      if(mtddf->grid() != grid) {
         mlog << Error << "\nget_series_entry() -> "
              << "the grid has changed in file \"" << cur_file << "\":\n"
              << "Old: " << grid.serialize() << "\n"
              << "New: " << mtddf->grid().serialize() << "\n\n";
         exit(1);
      }

      // Attempt to read the gridded data from the current file
      found = mtddf->data_plane(*info, dp);
      
      // Close the data file
      delete mtddf; mtddf = (Met2dDataFile *) 0;

      // Check if the series entry was found
      if(found) {
         mlog << Debug(2)
              << "Found data for " << info->magic_str()
              << " in " << search_files[j] << "\n";

         // Store the found file
         found_files.set(i_series, cur_file);

         // Break out of the loop
         break;
      }
   }

   // Check if the data plane was found
   if(!found) {
      mlog << Error << "\nget_series_entry() -> "
           << "Could not find data for " << info->magic_str()
           << " in file list:\n";
      for(i=0; i<search_files.n_elements(); i++)
         mlog << Error << "   " << search_files[i] << "\n";
      mlog << Error << "\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cts(int n, const NumArray &f_na, const NumArray &o_na) {
   int i, j;

   mlog << Debug(4) << "Computing Categorical Statistics.\n";

   // Allocate objects to store categorical statistics
   int n_cts = conf_info.fcst_cat_ta.n_elements();
   CTSInfo *cts_info = new CTSInfo [n_cts];

   // Setup CTSInfo objects
   for(i=0; i<n_cts; i++) {
      cts_info[i].cts_fcst_thresh = conf_info.fcst_cat_ta[i];
      cts_info[i].cts_obs_thresh  = conf_info.obs_cat_ta[i];

      cts_info[i].allocate_n_alpha(conf_info.ci_alpha.n_elements());      
      for(j=0; j<conf_info.ci_alpha.n_elements(); j++) {
         cts_info[i].alpha[j] = conf_info.ci_alpha[j];
      }
   }

   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_cts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         cts_info, n_cts, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         cts_info, n_cts, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }

   // Loop over the categorical thresholds
   for(i=0; i<n_cts; i++) {

      // Add statistic value for each possible FHO column
      for(j=0; j<conf_info.output_stats[stat_fho].n_elements(); j++) {
         store_stat_fho(n, conf_info.output_stats[stat_fho][j],
                        cts_info[i]);
      }

      // Add statistic value for each possible CTC column
      for(j=0; j<conf_info.output_stats[stat_ctc].n_elements(); j++) {
         store_stat_ctc(n, conf_info.output_stats[stat_ctc][j],
                        cts_info[i]);
      }

      // Add statistic value for each possible CTS column
      for(j=0; j<conf_info.output_stats[stat_cts].n_elements(); j++) {
         store_stat_cts(n, conf_info.output_stats[stat_cts][j],
                        cts_info[i]);
      }
   } // end for i

   // Deallocate memory
   if(cts_info) { delete [] cts_info; cts_info = (CTSInfo *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_mcts(int n, const NumArray &f_na, const NumArray &o_na) {
   int i;

   mlog << Debug(4) << "Computing Multi-Category Statistics.\n";
   
   // Object to store multi-category statistics
   MCTSInfo mcts_info;

   // Setup the MCTSInfo object
   mcts_info.cts.set_size(conf_info.fcst_cat_ta.n_elements() + 1);
   mcts_info.cts_fcst_ta = conf_info.fcst_cat_ta;
   mcts_info.cts_obs_ta  = conf_info.obs_cat_ta;

   mcts_info.allocate_n_alpha(conf_info.ci_alpha.n_elements());
   for(i=0; i<conf_info.ci_alpha.n_elements(); i++) {
      mcts_info.alpha[i] = conf_info.ci_alpha[i];
   }

   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_mcts_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep,
         mcts_info, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         mcts_info, true,
         conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   
   // Add statistic value for each possible MCTC column
   for(i=0; i<conf_info.output_stats[stat_mctc].n_elements(); i++) {
      store_stat_mctc(n, conf_info.output_stats[stat_mctc][i],
                      mcts_info);
   }

   // Add statistic value for each possible MCTS column
   for(i=0; i<conf_info.output_stats[stat_mcts].n_elements(); i++) {
      store_stat_mcts(n, conf_info.output_stats[stat_mcts][i],
                      mcts_info);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_cnt(int n, const NumArray &f_na, const NumArray &o_na) {
   int i;
  
   mlog << Debug(4) << "Computing Continuous Statistics.\n";

   // Object to store continuous statistics
   CNTInfo cnt_info;

   // Setup the CNTInfo object
   cnt_info.allocate_n_alpha(conf_info.ci_alpha.n_elements());
   for(i=0; i<conf_info.ci_alpha.n_elements(); i++) {
      cnt_info.alpha[i] = conf_info.ci_alpha[i];
   }
      
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   if(conf_info.boot_interval == BootIntervalType_BCA) {
      compute_cnt_stats_ci_bca(rng_ptr, f_na, o_na,
         conf_info.fcst_info[0]->is_precipitation() &
         conf_info.obs_info[0]->is_precipitation(),
         conf_info.n_boot_rep,
         cnt_info, true, conf_info.rank_corr_flag, conf_info.tmp_dir);
   }
   else {
      compute_cnt_stats_ci_perc(rng_ptr, f_na, o_na,
         conf_info.fcst_info[0]->is_precipitation() &
         conf_info.obs_info[0]->is_precipitation(),
         conf_info.n_boot_rep, conf_info.boot_rep_prop,
         cnt_info, true, conf_info.rank_corr_flag, conf_info.tmp_dir);
   }

   // Add statistic value for each possible CNT column
   for(i=0; i<conf_info.output_stats[stat_cnt].n_elements(); i++) {
      store_stat_cnt(n, conf_info.output_stats[stat_cnt][i],
                     cnt_info);
   }

   // Add statistic value for each possible SL1L2 column
   for(i=0; i<conf_info.output_stats[stat_sl1l2].n_elements(); i++) {
      store_stat_sl1l2(n, conf_info.output_stats[stat_sl1l2][i],
                       cnt_info);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void do_pct(int n, const NumArray &f_na, const NumArray &o_na) {
   int i, j;

   mlog << Debug(4) << "Computing Probabilistic Statistics.\n";

   // Object to store probabilistic statistics
   PCTInfo pct_info;

   // Setup the PCTInfo object
   pct_info.pct_fcst_thresh = conf_info.fcst_cat_ta;
   pct_info.allocate_n_alpha(conf_info.ci_alpha.n_elements());

   for(i=0; i<conf_info.ci_alpha.n_elements(); i++) {
      pct_info.alpha[i] = conf_info.ci_alpha[i];
   }

   // Compute PCTInfo for each observation threshold
   for(i=0; i<conf_info.obs_cat_ta.n_elements(); i++) {

      // Set the current observation threshold
      pct_info.pct_obs_thresh = conf_info.obs_cat_ta[i];

      // Compute the probabilistic counts and statistics
      compute_pctinfo(f_na, o_na, true, pct_info);

      // Add statistic value for each possible PCT column
      for(j=0; j<conf_info.output_stats[stat_pct].n_elements(); j++) {
         store_stat_pct(n, conf_info.output_stats[stat_pct][j],
                        pct_info);
      }

      // Add statistic value for each possible PSTD column
      for(j=0; j<conf_info.output_stats[stat_pstd].n_elements(); j++) {
         store_stat_pstd(n, conf_info.output_stats[stat_pstd][j],
                         pct_info);
      }

      // Add statistic value for each possible PJC column
      for(j=0; j<conf_info.output_stats[stat_pjc].n_elements(); j++) {
         store_stat_pjc(n, conf_info.output_stats[stat_pjc][j],
                        pct_info);
      }

      // Add statistic value for each possible PRC column
      for(j=0; j<conf_info.output_stats[stat_prc].n_elements(); j++) {
         store_stat_prc(n, conf_info.output_stats[stat_prc][j],
                        pct_info);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_fho(int n, const ConcatString &col,
                    const CTSInfo &cts_info) {
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Get the column value
        if(c == "TOTAL")  { v = (double) cts_info.cts.n(); }
   else if(c == "F_RATE") { v = cts_info.cts.f_rate();     }
   else if(c == "H_RATE") { v = cts_info.cts.h_rate();     }
   else if(c == "O_RATE") { v = cts_info.cts.o_rate();     }
   else {
     mlog << Error << "\nstore_stat_fho() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_fho_" << c << "_";

   // Append threshold information
   if(cts_info.cts_fcst_thresh == cts_info.cts_obs_thresh) {
      var_name << cts_info.cts_fcst_thresh.get_abbr_str();
   }
   else {
      var_name << "fcst" << cts_info.cts_fcst_thresh.get_abbr_str()
               << "_obs" << cts_info.cts_obs_thresh.get_abbr_str();
   }

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "FHO_" << c;
     
      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 cts_info.cts_fcst_thresh.get_str(),
                 cts_info.cts_obs_thresh.get_str(),
                 bad_data_double);
   }

   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_ctc(int n, const ConcatString &col,
                    const CTSInfo &cts_info) {
   int v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Get the column value
        if(c == "TOTAL") { v = cts_info.cts.n();     }
   else if(c == "FY_OY") { v = cts_info.cts.fy_oy(); }
   else if(c == "FY_ON") { v = cts_info.cts.fy_on(); }
   else if(c == "FN_OY") { v = cts_info.cts.fn_oy(); }
   else if(c == "FN_ON") { v = cts_info.cts.fn_on(); }
   else {
     mlog << Error << "\nstore_stat_ctc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_ctc_" << c << "_";

   // Append threshold information
   if(cts_info.cts_fcst_thresh == cts_info.cts_obs_thresh) {
      var_name << cts_info.cts_fcst_thresh.get_abbr_str();
   }
   else {
      var_name << "fcst" << cts_info.cts_fcst_thresh.get_abbr_str()
               << "_obs" << cts_info.cts_obs_thresh.get_abbr_str();
   }
   
   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "CTC_" << c;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 cts_info.cts_fcst_thresh.get_str(),
                 cts_info.cts_obs_thresh.get_str(),
                 bad_data_double);
   }
   
   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_cts(int n, const ConcatString &col,
                    const CTSInfo &cts_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c, "_NC") || strstr(c, "_BC")) n_ci = cts_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {

      // Get the column value
           if(c == "TOTAL")     { v = (double) cts_info.cts.n(); }
      else if(c == "BASER")     { v = cts_info.baser.v;          }
      else if(c == "BASER_NCL") { v = cts_info.baser.v_ncl[i];   }
      else if(c == "BASER_NCU") { v = cts_info.baser.v_ncu[i];   }
      else if(c == "BASER_BCL") { v = cts_info.baser.v_bcl[i];   }
      else if(c == "BASER_BCU") { v = cts_info.baser.v_bcu[i];   }
      else if(c == "FMEAN")     { v = cts_info.fmean.v;          }
      else if(c == "FMEAN_NCL") { v = cts_info.fmean.v_ncl[i];   }
      else if(c == "FMEAN_NCU") { v = cts_info.fmean.v_ncu[i];   }
      else if(c == "FMEAN_BCL") { v = cts_info.fmean.v_bcl[i];   }
      else if(c == "FMEAN_BCU") { v = cts_info.fmean.v_bcu[i];   }
      else if(c == "ACC")       { v = cts_info.acc.v;            }
      else if(c == "ACC_NCL")   { v = cts_info.acc.v_ncl[i];     }
      else if(c == "ACC_NCU")   { v = cts_info.acc.v_ncu[i];     }
      else if(c == "ACC_BCL")   { v = cts_info.acc.v_bcl[i];     }
      else if(c == "ACC_BCU")   { v = cts_info.acc.v_bcu[i];     }
      else if(c == "FBIAS")     { v = cts_info.fbias.v;          }
      else if(c == "FBIAS_BCL") { v = cts_info.fbias.v_bcl[i];   }
      else if(c == "FBIAS_BCU") { v = cts_info.fbias.v_bcu[i];   }
      else if(c == "PODY")      { v = cts_info.pody.v;           }
      else if(c == "PODY_NCL")  { v = cts_info.pody.v_ncl[i];    }
      else if(c == "PODY_NCU")  { v = cts_info.pody.v_ncu[i];    }
      else if(c == "PODY_BCL")  { v = cts_info.pody.v_bcl[i];    }
      else if(c == "PODY_BCU")  { v = cts_info.pody.v_bcu[i];    }
      else if(c == "PODN")      { v = cts_info.podn.v;           }
      else if(c == "PODN_NCL")  { v = cts_info.podn.v_ncl[i];    }
      else if(c == "PODN_NCU")  { v = cts_info.podn.v_ncu[i];    }
      else if(c == "PODN_BCL")  { v = cts_info.podn.v_bcl[i];    }
      else if(c == "PODN_BCU")  { v = cts_info.podn.v_bcu[i];    }
      else if(c == "POFD")      { v = cts_info.pofd.v;           }
      else if(c == "POFD_NCL")  { v = cts_info.pofd.v_ncl[i];    }
      else if(c == "POFD_NCU")  { v = cts_info.pofd.v_ncu[i];    }
      else if(c == "POFD_BCL")  { v = cts_info.pofd.v_bcl[i];    }
      else if(c == "POFD_BCU")  { v = cts_info.pofd.v_bcu[i];    }
      else if(c == "FAR")       { v = cts_info.far.v;            }
      else if(c == "FAR_NCL")   { v = cts_info.far.v_ncl[i];     }
      else if(c == "FAR_NCU")   { v = cts_info.far.v_ncu[i];     }
      else if(c == "FAR_BCL")   { v = cts_info.far.v_bcl[i];     }
      else if(c == "FAR_BCU")   { v = cts_info.far.v_bcu[i];     }
      else if(c == "CSI")       { v = cts_info.csi.v;            }
      else if(c == "CSI_NCL")   { v = cts_info.csi.v_ncl[i];     }
      else if(c == "CSI_NCU")   { v = cts_info.csi.v_ncu[i];     }
      else if(c == "CSI_BCL")   { v = cts_info.csi.v_bcl[i];     }
      else if(c == "CSI_BCU")   { v = cts_info.csi.v_bcu[i];     }
      else if(c == "GSS")       { v = cts_info.gss.v;            }
      else if(c == "GSS_BCL")   { v = cts_info.gss.v_bcl[i];     }
      else if(c == "GSS_BCU")   { v = cts_info.gss.v_bcu[i];     }
      else if(c == "HK")        { v = cts_info.hk.v;             }
      else if(c == "HK_NCL")    { v = cts_info.hk.v_ncl[i];      }
      else if(c == "HK_NCU")    { v = cts_info.hk.v_ncu[i];      }
      else if(c == "HK_BCL")    { v = cts_info.hk.v_bcl[i];      }
      else if(c == "HK_BCU")    { v = cts_info.hk.v_bcu[i];      }
      else if(c == "HSS")       { v = cts_info.hss.v;            }
      else if(c == "HSS_BCL")   { v = cts_info.hss.v_bcl[i];     }
      else if(c == "HSS_BCU")   { v = cts_info.hss.v_bcu[i];     }
      else if(c == "ODDS")      { v = cts_info.odds.v;           }
      else if(c == "ODDS_NCL")  { v = cts_info.odds.v_ncl[i];    }
      else if(c == "ODDS_NCU")  { v = cts_info.odds.v_ncu[i];    }
      else if(c == "ODDS_BCL")  { v = cts_info.odds.v_bcl[i];    }
      else if(c == "ODDS_BCU")  { v = cts_info.odds.v_bcu[i];    }
      else {
        mlog << Error << "\nstore_stat_cts() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_cts_" << c << "_";

      // Append threshold information
      if(cts_info.cts_fcst_thresh == cts_info.cts_obs_thresh) {
         var_name << cts_info.cts_fcst_thresh.get_abbr_str();
      }
      else {
         var_name << "fcst" << cts_info.cts_fcst_thresh.get_abbr_str()
                  << "_obs" << cts_info.cts_obs_thresh.get_abbr_str();
      }

      // Append confidence interval alpha value
      if(n_ci > 1) var_name << "_a"  << cts_info.alpha[i];
      
      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         lty_stat << "CTS_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    cts_info.cts_fcst_thresh.get_str(),
                    cts_info.cts_obs_thresh.get_str(),
                    (n_ci > 1 ? cts_info.alpha[i] : bad_data_double));
      }
   
      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_mctc(int n, const ConcatString &col,
                     const MCTSInfo &mcts_info) {
   int i, j;
   double v;
   ConcatString lty_stat, var_name;
   StringArray sa;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;

   // Get the column value
        if(c == "TOTAL") { v = (double) mcts_info.cts.total(); }
   else if(c == "N_CAT") { v = (double) mcts_info.cts.nrows(); }
   else if(check_reg_exp("F[0-9]*_O[0-9]*", c)) {               

      d = "FI_OJ";

      // Parse column name to retrieve index values
      sa = c.split("_");
      i  = atoi(sa[0]+1) - 1;
      j  = atoi(sa[1]+1) - 1;
      
      // Range check
      if(i < 0 || i >= mcts_info.cts.nrows() ||
         j < 0 || j >= mcts_info.cts.ncols()) {
         mlog << Error << "\nstore_stat_mctc() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }

      // Retrieve the value
      v = (double) mcts_info.cts.entry(i, j); 
   }
   else {
     mlog << Error << "\nstore_stat_mctc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_mctc_" << c;

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "MCTC_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 mcts_info.cts_fcst_ta.get_str(","),
                 mcts_info.cts_obs_ta.get_str(","),
                 bad_data_double);
   }
   
   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_mcts(int n, const ConcatString &col,
                     const MCTSInfo &mcts_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c, "_NC") || strstr(c, "_BC")) n_ci = mcts_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {

      // Get the column value
           if(c == "TOTAL")   { v = (double) mcts_info.cts.total(); }
      else if(c == "N_CAT")   { v = (double) mcts_info.cts.nrows(); }
      else if(c == "ACC")     { v = mcts_info.acc.v;                }
      else if(c == "ACC_NCL") { v = mcts_info.acc.v_ncl[i];         }
      else if(c == "ACC_NCU") { v = mcts_info.acc.v_ncu[i];         }
      else if(c == "ACC_BCL") { v = mcts_info.acc.v_bcl[i];         }
      else if(c == "ACC_BCU") { v = mcts_info.acc.v_bcu[i];         }
      else if(c == "HK")      { v = mcts_info.hk.v;                 }
      else if(c == "HK_BCL")  { v = mcts_info.hk.v_bcl[i];          }
      else if(c == "HK_BCU")  { v = mcts_info.hk.v_bcu[i];          }
      else if(c == "HSS")     { v = mcts_info.hss.v;                }
      else if(c == "HSS_BCL") { v = mcts_info.hss.v_bcl[i];         }
      else if(c == "HSS_BCU") { v = mcts_info.hss.v_bcu[i];         }
      else if(c == "GER")     { v = mcts_info.ger.v;                }
      else if(c == "GER_BCL") { v = mcts_info.ger.v_bcl[i];         }
      else if(c == "GER_BCU") { v = mcts_info.ger.v_bcu[i];         }
      else {
        mlog << Error << "\nstore_stat_mcts() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_mcts_" << c;

      // Append confidence interval alpha value
      if(n_ci > 1) var_name << "_a"  << mcts_info.alpha[i];

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {
         
         // Build key
         lty_stat << "MCTS_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    mcts_info.cts_fcst_ta.get_str(","),
                    mcts_info.cts_obs_ta.get_str(","),
                    (n_ci > 1 ? mcts_info.alpha[i] : bad_data_double));
      }

      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_cnt(int n, const ConcatString &col,
                    const CNTInfo &cnt_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   
   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c, "_NC") || strstr(c, "_BC")) n_ci = cnt_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {
   
      // Get the column value
           if(c == "TOTAL")       { v = (double) cnt_info.n;       }
      else if(c == "FBAR")        { v = cnt_info.fbar.v;           }
      else if(c == "FBAR_NCL")    { v = cnt_info.fbar.v_ncl[i];    }
      else if(c == "FBAR_NCU")    { v = cnt_info.fbar.v_ncu[i];    }
      else if(c == "FBAR_BCL")    { v = cnt_info.fbar.v_bcl[i];    }
      else if(c == "FBAR_BCU")    { v = cnt_info.fbar.v_bcu[i];    }
      else if(c == "FSTDEV")      { v = cnt_info.fstdev.v;         }
      else if(c == "FSTDEV_NCL")  { v = cnt_info.fstdev.v_ncl[i];  }
      else if(c == "FSTDEV_NCU")  { v = cnt_info.fstdev.v_ncu[i];  }
      else if(c == "FSTDEV_BCL")  { v = cnt_info.fstdev.v_bcl[i];  }
      else if(c == "FSTDEV_BCU")  { v = cnt_info.fstdev.v_bcu[i];  }
      else if(c == "OBAR")        { v = cnt_info.obar.v;           }
      else if(c == "OBAR_NCL")    { v = cnt_info.obar.v_ncl[i];    }
      else if(c == "OBAR_NCU")    { v = cnt_info.obar.v_ncu[i];    }
      else if(c == "OBAR_BCL")    { v = cnt_info.obar.v_bcl[i];    }
      else if(c == "OBAR_BCU")    { v = cnt_info.obar.v_bcu[i];    }
      else if(c == "OSTDEV")      { v = cnt_info.ostdev.v;         }
      else if(c == "OSTDEV_NCL")  { v = cnt_info.ostdev.v_ncl[i];  }
      else if(c == "OSTDEV_NCU")  { v = cnt_info.ostdev.v_ncu[i];  }
      else if(c == "OSTDEV_BCL")  { v = cnt_info.ostdev.v_bcl[i];  }
      else if(c == "OSTDEV_BCU")  { v = cnt_info.ostdev.v_bcu[i];  }
      else if(c == "PR_CORR")     { v = cnt_info.pr_corr.v;        }
      else if(c == "PR_CORR_NCL") { v = cnt_info.pr_corr.v_ncl[i]; }
      else if(c == "PR_CORR_NCU") { v = cnt_info.pr_corr.v_ncu[i]; }
      else if(c == "PR_CORR_BCL") { v = cnt_info.pr_corr.v_bcl[i]; }
      else if(c == "PR_CORR_BCU") { v = cnt_info.pr_corr.v_bcu[i]; }
      else if(c == "SP_CORR")     { v = cnt_info.sp_corr.v;        }
      else if(c == "KT_CORR")     { v = cnt_info.kt_corr.v;        }
      else if(c == "RANKS")       { v = cnt_info.n_ranks;          }
      else if(c == "FRANK_TIES")  { v = cnt_info.frank_ties;       }
      else if(c == "ORANK_TIES")  { v = cnt_info.orank_ties;       }
      else if(c == "ME")          { v = cnt_info.me.v;             }
      else if(c == "ME_NCL")      { v = cnt_info.me.v_ncl[i];      }
      else if(c == "ME_NCU")      { v = cnt_info.me.v_ncu[i];      }
      else if(c == "ME_BCL")      { v = cnt_info.me.v_bcl[i];      }
      else if(c == "ME_BCU")      { v = cnt_info.me.v_bcu[i];      }
      else if(c == "ESTDEV")      { v = cnt_info.estdev.v;         }
      else if(c == "ESTDEV_NCL")  { v = cnt_info.estdev.v_ncl[i];  }
      else if(c == "ESTDEV_NCU")  { v = cnt_info.estdev.v_ncu[i];  }
      else if(c == "ESTDEV_BCL")  { v = cnt_info.estdev.v_bcl[i];  }
      else if(c == "ESTDEV_BCU")  { v = cnt_info.estdev.v_bcu[i];  }
      else if(c == "MBIAS")       { v = cnt_info.mbias.v;          }
      else if(c == "MBIAS_BCL")   { v = cnt_info.mbias.v_bcl[i];   }
      else if(c == "MBIAS_BCU")   { v = cnt_info.mbias.v_bcu[i];   }
      else if(c == "MAE")         { v = cnt_info.mae.v;            }
      else if(c == "MAE_BCL")     { v = cnt_info.mae.v_bcl[i];     }
      else if(c == "MAE_BCU")     { v = cnt_info.mae.v_bcu[i];     }
      else if(c == "MSE")         { v = cnt_info.mse.v;            }
      else if(c == "MSE_BCL")     { v = cnt_info.mse.v_bcl[i];     }
      else if(c == "MSE_BCU")     { v = cnt_info.mse.v_bcu[i];     }
      else if(c == "BCMSE")       { v = cnt_info.bcmse.v;          }
      else if(c == "BCMSE_BCL")   { v = cnt_info.bcmse.v_bcl[i];   }
      else if(c == "BCMSE_BCU")   { v = cnt_info.bcmse.v_bcu[i];   }
      else if(c == "RMSE")        { v = cnt_info.rmse.v;           }
      else if(c == "RMSE_BCL")    { v = cnt_info.rmse.v_bcl[i];    }
      else if(c == "RMSE_BCU")    { v = cnt_info.rmse.v_bcu[i];    }
      else if(c == "E10")         { v = cnt_info.e10.v;            }
      else if(c == "E10_BCL")     { v = cnt_info.e10.v_bcl[i];     }
      else if(c == "E10_BCU")     { v = cnt_info.e10.v_bcu[i];     }
      else if(c == "E25")         { v = cnt_info.e25.v;            }
      else if(c == "E25_BCL")     { v = cnt_info.e25.v_bcl[i];     }
      else if(c == "E25_BCU")     { v = cnt_info.e25.v_bcu[i];     }
      else if(c == "E50")         { v = cnt_info.e50.v;            }
      else if(c == "E50_BCL")     { v = cnt_info.e50.v_bcl[i];     }
      else if(c == "E50_BCU")     { v = cnt_info.e50.v_bcu[i];     }
      else if(c == "E75")         { v = cnt_info.e75.v;            }
      else if(c == "E75_BCL")     { v = cnt_info.e75.v_bcl[i];     }
      else if(c == "E75_BCU")     { v = cnt_info.e75.v_bcu[i];     }
      else if(c == "E90")         { v = cnt_info.e90.v;            }
      else if(c == "E90_BCL")     { v = cnt_info.e90.v_bcl[i];     }
      else if(c == "E90_BCU")     { v = cnt_info.e90.v_bcu[i];     }
      else {
        mlog << Error << "\nstore_stat_cnt() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_cnt_" << c;
      if(n_ci > 1) var_name << "_a"  << cnt_info.alpha[i];

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {
         
         // Build key
         lty_stat << "CNT_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    "", "",
                    (n_ci > 1 ? cnt_info.alpha[i] : bad_data_double));
      }

      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_sl1l2(int n, const ConcatString &col,
                      const CNTInfo &cnt_info) {
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Get the column value
        if(c == "TOTAL") { v = (double) cnt_info.n; }
   else if(c == "FBAR")  { v = cnt_info.fbar.v;     }
   else if(c == "OBAR")  { v = cnt_info.obar.v;     }
   else if(c == "FOBAR") { v = cnt_info.fobar;      }
   else if(c == "FFBAR") { v = cnt_info.ffbar;      }
   else if(c == "OOBAR") { v = cnt_info.oobar;      }
   else {
     mlog << Error << "\nstore_stat_sl1l2() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_sl1l2_" << c;

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "SL1L2_" << c;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 "", "",
                 bad_data_double);
   }
   
   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_pct(int n, const ConcatString &col,
                    const PCTInfo &pct_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;

   // Get index value for variable column numbers
   if(check_reg_exp("_[0-9]", c)) {

      // Parse the index value from the column name
      i = atoi(strrchr(c, '_') + 1) - 1;
      
      // Range check
      if(i < 0 || i >= pct_info.pct.nrows()) {
         mlog << Error << "\nstore_stat_pct() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }
   }  // end if
   
   // Get the column value
        if(c == "TOTAL")                     { v = (double) pct_info.pct.n();                      }
   else if(c == "N_THRESH")                  { v = (double) pct_info.pct.nrows() + 1;              }
   else if(check_reg_exp("THRESH_[0-9]", c)) { v = pct_info.pct.threshold(i);                      }
   else if(check_reg_exp("OY_[0-9]", c))     { v = (double) pct_info.pct.event_count_by_row(i);
                                               d = "OY_I";                                         }
   else if(check_reg_exp("ON_[0-9]", c))     { v = (double) pct_info.pct.nonevent_count_by_row(i);
                                               d = "ON_I";                                         }
   else {
     mlog << Error << "\nstore_stat_pct() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_pct_" << c
            << "_obs" << pct_info.pct_obs_thresh.get_abbr_str();

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "PCT_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 pct_info.pct_fcst_thresh.get_str(","),
                 pct_info.pct_obs_thresh.get_str(),
                 bad_data_double);
   }
            
   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_pstd(int n, const ConcatString &col,
                     const PCTInfo &pct_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   int n_ci = 1;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);

   // Check for columns with normal or bootstrap confidence limits
   if(strstr(c, "_NC") || strstr(c, "_BC")) n_ci = pct_info.n_alpha;

   // Loop over the alpha values, if necessary
   for(i=0; i<n_ci; i++) {

      // Get the column value
           if(c == "TOTAL")       { v = (double) pct_info.pct.n();         }
      else if(c == "N_THRESH")    { v = (double) pct_info.pct.nrows() + 1; }
      else if(c == "BASER")       { v = pct_info.baser.v;                  }
      else if(c == "BASER_NCL")   { v = pct_info.baser.v_ncl[i];           }
      else if(c == "BASER_NCU")   { v = pct_info.baser.v_ncu[i];           }
      else if(c == "RELIABILITY") { v = pct_info.pct.reliability();        }
      else if(c == "RESOLUTION")  { v = pct_info.pct.resolution();         }
      else if(c == "UNCERTAINTY") { v = pct_info.pct.uncertainty();        }
      else if(c == "ROC_AUC")     { v = pct_info.pct.roc_auc();            }
      else if(c == "BRIER")       { v = pct_info.brier.v;                  }
      else if(c == "BRIER_NCL")   { v = pct_info.brier.v_ncl[i];           }
      else if(c == "BRIER_NCU")   { v = pct_info.brier.v_ncu[i];           }
      else {
        mlog << Error << "\nstore_stat_pstd() -> "
             << "unsupported column name requested \"" << c
             << "\"\n\n";
        exit(1);
      }

      // Construct the NetCDF variable name
      var_name << cs_erase << "series_pstd_" << c;

      // Append confidence interval alpha value
      if(n_ci > 1) var_name << "_a"  << pct_info.alpha[i];

      // Add map for this variable name
      if(stat_data.count(var_name) == 0) {

         // Build key
         lty_stat << "PSTD_" << c;

         // Add new map entry
         add_nc_var(var_name, c, stat_long_name[lty_stat],
                    pct_info.pct_fcst_thresh.get_str(","),
                    pct_info.pct_obs_thresh.get_str(),
                    (n_ci > 1 ? pct_info.alpha[i] : bad_data_double));
      }

      // Store the statistic value
      put_nc_val(n, var_name, (float) v);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_pjc(int n, const ConcatString &col,
                    const PCTInfo &pct_info) {
   int i, tot;
   double v;
   ConcatString lty_stat, var_name;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;
   
   // Get index value for variable column numbers
   if(check_reg_exp("_[0-9]", c)) {

      // Parse the index value from the column name
      i = atoi(strrchr(c, '_') + 1) - 1;

      // Range check
      if(i < 0 || i >= pct_info.pct.nrows()) {
         mlog << Error << "\nstore_stat_pjc() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }
   }  // end if

   // Store the total count
   tot = pct_info.pct.n();

   // Get the column value
        if(c == "TOTAL")                          { v = (double) tot;                                       }
   else if(c == "N_THRESH")                       { v = (double) pct_info.pct.nrows() + 1;                  }
   else if(check_reg_exp("THRESH_[0-9]", c))      { v = pct_info.pct.threshold(i);
                                                    d = "THRESH_I";                                         }
   else if(check_reg_exp("OY_TP_[0-9]", c))       { v = pct_info.pct.event_count_by_row(i)/(double) tot;
                                                    d = "OY_TP_I";                                          }
   else if(check_reg_exp("ON_TP_[0-9]", c))       { v = pct_info.pct.nonevent_count_by_row(i)/(double) tot;
                                                    d = "ON_TP_I";                                          }
   else if(check_reg_exp("CALIBRATION_[0-9]", c)) { v = pct_info.pct.row_calibration(i);
                                                    d = "CALIBRATION_I";                                    }
   else if(check_reg_exp("REFINEMENT_[0-9]", c))  { v = pct_info.pct.row_refinement(i);
                                                    d = "REFINEMENT_I";                                     }
   else if(check_reg_exp("LIKELIHOOD_[0-9]", c))  { v = pct_info.pct.row_event_likelihood(i);
                                                    d = "LIKELIHOOD_I";                                     }
   else if(check_reg_exp("BASER_[0-9]", c))       { v = pct_info.pct.row_obar(i);
                                                    d = "BASER_I";                                          }
   else {
     mlog << Error << "\nstore_stat_pjc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Construct the NetCDF variable name
   var_name << cs_erase << "series_pjc_" << c
            << "_obs" << pct_info.pct_obs_thresh.get_abbr_str();

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "PJC_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 pct_info.pct_fcst_thresh.get_str(","),
                 pct_info.pct_obs_thresh.get_str(),
                 bad_data_double);
   }
            
   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void store_stat_prc(int n, const ConcatString &col,
                    const PCTInfo &pct_info) {
   int i;
   double v;
   ConcatString lty_stat, var_name;
   TTContingencyTable ct;

   // Set the column name to all upper case
   ConcatString c = to_upper(col);
   ConcatString d = c;

   // Get index value for variable column numbers
   if(check_reg_exp("_[0-9]", c)) {

      // Parse the index value from the column name
      i = atoi(strrchr(c, '_') + 1) - 1;

      // Range check
      if(i < 0 || i >= pct_info.pct.nrows()) {
         mlog << Error << "\nstore_stat_prc() -> "
              << "range check error for column name requested \"" << c
              << "\"\n\n";
         exit(1);
      }

      // Get the 2x2 contingency table for this row
      ct = pct_info.pct.roc_point_by_row(i);
      
   }  // end if

   // Get the column value
        if(c == "TOTAL")                     { v = (double) pct_info.pct.n();         }
   else if(c == "N_THRESH")                  { v = (double) pct_info.pct.nrows() + 1; }
   else if(check_reg_exp("THRESH_[0-9]", c)) { v = pct_info.pct.threshold(i);
                                               d = "THRESH_I";                        }
   else if(check_reg_exp("PODY_[0-9]", c))   { v = ct.pod_yes();
                                               d = "PODY_I";                          }
   else if(check_reg_exp("POFD_[0-9]", c))   { v = ct.pofd();
                                               d = "POFD_I";                          }
   else {
     mlog << Error << "\nstore_stat_prc() -> "
          << "unsupported column name requested \"" << c
          << "\"\n\n";
     exit(1);
   }

   // Add map for this variable name
   if(stat_data.count(var_name) == 0) {

      // Build key
      lty_stat << "PRC_" << d;

      // Add new map entry
      add_nc_var(var_name, c, stat_long_name[lty_stat],
                 pct_info.pct_fcst_thresh.get_str(","),
                 pct_info.pct_obs_thresh.get_str(),
                 bad_data_double);
   }
            
   // Store the statistic value
   put_nc_val(n, var_name, (float) v);

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_nc_file(const VarInfo *fcst_info, const VarInfo *obs_info) {

   // Create a new NetCDF file and open it
   nc_out = new NcFile(out_file, NcFile::Replace);

   if(!nc_out->is_valid()) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "trouble opening output NetCDF file "
           << out_file << "\n\n";
      exit(1);
   }

   // Add global attributes
   write_netcdf_global(nc_out, out_file, program_name);
   nc_out->add_att("model",      conf_info.model);
   nc_out->add_att("mask_grid",  (conf_info.mask_grid_name ?
                                  conf_info.mask_grid_name : na_str));
   nc_out->add_att("mask_poly",  (conf_info.mask_poly_name ?
                                  conf_info.mask_poly_name : na_str));
   nc_out->add_att("fcst_var",   fcst_info->name());
   nc_out->add_att("fcst_lev",   fcst_info->level_name());
   nc_out->add_att("fcst_units", fcst_info->units());
   nc_out->add_att("obs_var",    obs_info->name());
   nc_out->add_att("obs_lev",    obs_info->level_name());
   nc_out->add_att("obs_units",  obs_info->units());

   // Add the projection information
   write_netcdf_proj(nc_out, grid);

   // Define Dimensions
   lat_dim = nc_out->add_dim("lat", (long) grid.ny());
   lon_dim = nc_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(nc_out, lat_dim, lon_dim, grid);

   // Add the series length variable
   NcVar * var = nc_out->add_var("n_series", ncInt);
   var->add_att("long_name", "length of series");

   if(!var->put(&n_series)) {
      mlog << Error << "\nsetup_nc_file() -> "
           << "error writing the series length variable.\n\n";
            exit(1);
   }

   // Load the long name descriptions for each column name
   parse_long_names();

   return;
}

////////////////////////////////////////////////////////////////////////
                             
void add_nc_var(const ConcatString &var_name,
                const ConcatString &name,
                const ConcatString &long_name,
                const ConcatString &fcst_thresh,
                const ConcatString &obs_thresh,
                double alpha) {
   NcVarData d;

   // Add a new variable to the NetCDF file
   d.var = nc_out->add_var(var_name, ncFloat, lat_dim, lon_dim);

   // Add variable attributes
   d.var->add_att("_FillValue", bad_data_float);
   if(name.length() > 0)        d.var->add_att("name", name);
   if(long_name.length() > 0)   d.var->add_att("long_name", long_name);
   if(fcst_thresh.length() > 0) d.var->add_att("fcst_thresh", fcst_thresh);
   if(obs_thresh.length() > 0)  d.var->add_att("obs_thresh", obs_thresh);
   if(!is_bad_data(alpha))      d.var->add_att("alpha", alpha);
   
   // Store the new NcVarData object in the map
   stat_data[var_name] = d;

   return;
}

////////////////////////////////////////////////////////////////////////

void put_nc_val(int n, const ConcatString &var_name, float v) {
   int x, y;

   // Determine x,y location
   DefaultTO.one_to_two(grid.nx(), grid.ny(), n, x, y);
   
   // Check for key in the map
   if(stat_data.count(var_name) == 0) {
      mlog << Error << "\nput_nc_val() -> "
           << "variable name \"" << var_name
           << "\" does not exist in the map.\n\n";
      exit(1);
   }
   
   // Get the NetCDF variable to be written
   NcVar *var = stat_data[var_name].var;

   // Store the current value
   if(!var->set_cur(y, x) || !var->put(&v, 1, 1)) {
      mlog << Error << "\nput_nc_val() -> "
           << "error writing to variable " << var_name
           << " for point (" << x << ", " << y << ").\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_range(const unixtime &t, unixtime &beg, unixtime &end) {

   beg = (beg == (unixtime) 0 || t < beg ? t : beg);
   end = (end == (unixtime) 0 || t > end ? t : end);

   return;
}

////////////////////////////////////////////////////////////////////////

void set_range(const int &t, int &beg, int &end) {

   beg = (is_bad_data(beg) || t < beg ? t : beg);
   end = (is_bad_data(end) || t > end ? t : end);

   return;
}

////////////////////////////////////////////////////////////////////////

void clean_up() {

   // Close the output NetCDF file
   if(nc_out) {

      // List the NetCDF file after it is finished
      mlog << Debug(1) << "Output file: " << out_file << "\n";

      nc_out->close();
      delete nc_out;
      nc_out = (NcFile *) 0;
   }

   // Deallocate memory for data files
   if(fcst_mtddf) { delete fcst_mtddf; fcst_mtddf = (Met2dDataFile *) 0; }
   if(obs_mtddf)  { delete obs_mtddf;  obs_mtddf  = (Met2dDataFile *) 0; }

   // Deallocate memory for the random number generator
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t-fcst file_1 ... file_n\n"
        << "\t-obs file_1 ... file_n\n"
        << "\t[-both file_1 ... file_n]\n"
        << "\t-out file\n"
        << "\t-config file\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-fcst file_1 ... file_n\" are the gridded "
        << "forecast files or ASCII files containing lists of file "
        << "names to be used (required).\n"

        << "\t\t\"-obs file_1 ... file_n\" are the gridded "
        << "observation files or ASCII files containing lists of file "
        << "names to be used (required).\n"

        << "\t\t\"-both file_1 ... file_n\" sets the \"-fcst\" and "
        << "\"-obs\" options to the same set of files (optional).\n"

        << "\t\t\"-out file\" is the NetCDF output file containing "
        << "computed statistics (required).\n"

        << "\t\t\"-config file\" is a SeriesAnalysisConfig file "
        << "containing the desired configuration settings (required).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\n\tNOTE: All fields must be on the same grid.\n\n";

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_fcst_files(const StringArray & a) {
   parse_file_list(a, fcst_files);
}

////////////////////////////////////////////////////////////////////////

void set_obs_files(const StringArray & a) {
   parse_file_list(a, obs_files);
}

////////////////////////////////////////////////////////////////////////

void set_both_files(const StringArray & a) {
   set_fcst_files(a);
   set_obs_files(a);
}

////////////////////////////////////////////////////////////////////////

void set_out_file(const StringArray & a) {
   out_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_config_file(const StringArray & a) {
   config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_log_file(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

void parse_file_list(const StringArray & a, StringArray & list) {
   int i;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;
   GrdFileType type = FileType_None;

   // Check for empty list
   if(a.n_elements() == 0) {
      mlog << Error << "\nparse_file_list() -> "
           << "empty list!\n\n";
      exit(1);
   }

   // Attempt to read the first file as a gridded data file
   mtddf = mtddf_factory.new_met_2d_data_file(a[0], type);

   // If the read was successful, store the list of gridded files.
   // Otherwise, process entries as ASCII files.
   if(mtddf)                            list.add(a);
   else for(i=0; i<a.n_elements(); i++) parse_ascii_file_list(a[0], list);

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_ascii_file_list(const char *file_list, StringArray & list) {
   ifstream f_in;
   char file_name[PATH_MAX];

   mlog << Debug(1)
        << "Reading ASCII file list: " << file_list << "\n";
   
   // Open the file list
   f_in.open(file_list);
   if(!f_in) {
      mlog << Error << "\nparse_ascii_file_list() -> "
           << "can't open the ASCII file list \"" << file_list
           << "\" for reading\n\n";
      exit(1);
   }

   // Read and store the file names
   while(f_in >> file_name) list.add(file_name);

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_long_names() {
   ifstream f_in;
   ConcatString line, key;
   StringArray sa;
   ConcatString file_name = replace_path(stat_long_name_file);

   mlog << Debug(1)
        << "Reading stat column descriptions: " << file_name << "\n";

   // Open the data file
   f_in.open(file_name);
   if(!f_in) {
      mlog << Error << "\nparse_long_names() -> "
           << "can't open the ASCII file \"" << file_name
           << "\" for reading\n\n";
      exit(1);
   }

   // Read the lines in the file
   while(line.read_line(f_in)) {

      // Parse the line
      sa = line.split("\"");

      // Skip any lines without enough elements
      if(sa.n_elements() < 2) continue;

      // Store the description
      key = sa[0];
      key.ws_strip();
      stat_long_name[key] = sa[1];
   }

   // Close the input file
   f_in.close();

   return;
}

////////////////////////////////////////////////////////////////////////
