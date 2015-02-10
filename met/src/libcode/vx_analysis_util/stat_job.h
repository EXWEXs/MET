// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_JOB_H__
#define  __STAT_JOB_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>

#include "stat_line.h"
#include "mask_poly.h"

#include "vx_cal.h"
#include "vx_grid.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

//
// Defaults to be used if not specified by the user
//
static const double default_alpha          = 0.05;
static const double default_bin_size       = 0.05;
static const int    default_boot_interval  = 1;
static const double default_boot_rep_prop  = 1.0;
static const int    default_n_boot_rep     = 1000;
static const char   default_boot_rng[]     = "mt19937";
static const char   default_boot_seed[]    = "";
static const int    default_rank_corr_flag = 1;
static const int    default_vif_flag       = 0;
static const char   default_tmp_dir[]      = "/tmp";

//
// Dump buffer size
//
static const int    dump_stat_buffer_rows  = 512;
static const int    dump_stat_buffer_cols  = 512;

////////////////////////////////////////////////////////////////////////

//
// Enumerate all the possible STAT Analysis Job Types
//
enum STATJobType {

   stat_job_filter    = 0, // Filter out the STAT data and write the
                           // lines to the filename specified.

   stat_job_summary   = 1, // Compute min, max, mean, stdev and
                           // percentiles for a column of data.

   stat_job_aggr      = 2, // Aggregate the input counts/scores and
                           // generate the same output line type
                           // containing the aggregated counts/scores.

   stat_job_aggr_stat = 3, // Aggregate the input counts/scores and
                           // generate the requested output line type.

   stat_job_go_index  = 4, // Compute the GO Index.

   stat_job_ss_index  = 5, // Compute the Skill Score Index.

   no_stat_job_type   = 6  // Default value
};

static const int n_statjobtypes = 7;

static const char * const statjobtype_str[n_statjobtypes] = {
   "filter",         "summary",  "aggregate",
   "aggregate_stat", "go_index", "ss_index",
   "NA"
};

extern const char *statjobtype_to_string(const STATJobType);
extern void        statjobtype_to_string(const STATJobType, char *);
extern STATJobType string_to_statjobtype(const char *);

////////////////////////////////////////////////////////////////////////

class STATAnalysisJob {

   private:

      void init_from_scratch();

      void assign(const STATAnalysisJob &);

   public:

      STATAnalysisJob();
     ~STATAnalysisJob();
      STATAnalysisJob(const STATAnalysisJob &);
      STATAnalysisJob & operator=(const STATAnalysisJob &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      int  is_keeper(const STATLine &) const;

      void parse_job_command(const char *);

      int  set_job_type (const char *);
      void set_dump_row (const char *);
      void set_stat_file(const char *);
      void set_mask_grid(const char *);
      void set_mask_poly(const char *);
      void set_boot_rng (const char *);
      void set_boot_seed(const char *);

      void open_dump_row_file ();
      void close_dump_row_file();
      void dump_stat_line     (const STATLine &);

      void open_stat_file ();
      void setup_stat_file(const STATLineType lt, int n_row, int n);
      void close_stat_file();

      ConcatString get_case_info(const STATLine &) const;

      ConcatString get_jobstring() const;

      //
      // Job Type
      //
      STATJobType job_type;

      //
      // Variables used to stratify the input STAT lines
      //
      StringArray model;

      IntArray    fcst_lead; // stored in seconds
      IntArray    obs_lead;  // stored in seconds

      unixtime    fcst_valid_beg;
      unixtime    fcst_valid_end;
      IntArray    fcst_valid_hour; // stored in seconds
      
      unixtime    obs_valid_beg;
      unixtime    obs_valid_end;
      IntArray    obs_valid_hour; // stored in seconds

      unixtime    fcst_init_beg;
      unixtime    fcst_init_end;
      IntArray    fcst_init_hour; // stored in seconds
      
      unixtime    obs_init_beg;
      unixtime    obs_init_end;
      IntArray    obs_init_hour;  // stored in seconds

      StringArray fcst_var;
      StringArray obs_var;

      StringArray fcst_lev;
      StringArray obs_lev;

      StringArray obtype;

      StringArray vx_mask;

      StringArray interp_mthd;
      NumArray    interp_pnts;

      ThreshArray fcst_thresh;
      ThreshArray obs_thresh;
      ThreshArray cov_thresh;

      NumArray    alpha;

      StringArray line_type;
      StringArray column;
      NumArray    weight;

      StringArray column_min_name;
      NumArray    column_min_value;

      StringArray column_max_name;
      NumArray    column_max_value;

      map<ConcatString,StringArray> column_str_map;

      //
      // Store the case information for the -by option
      //

      StringArray column_case;

      //
      // Variables used to the store the analysis job specification
      //

      char        *dump_row; // dump rows used to a file
      ofstream    *dr_out;   // output file stream for dump row
      int         n_dump;    // number of lines written to dump row
      AsciiTable  dump_at;   // AsciiTable for buffering dump row data

      char        *stat_file; // dump output statistics to a STAT file
      ofstream    *stat_out;  // output file stream for -out_stat
      AsciiTable  stat_at;    // AsciiTable for buffering output STAT data
      

      STATLineType out_line_type;        // output line type
      ThreshArray  out_fcst_thresh;      // output forecast threshold(s)
      ThreshArray  out_obs_thresh;       // output observation threshold
      SingleThresh out_fcst_wind_thresh; // output forecast wind speed threshold
      SingleThresh out_obs_wind_thresh;  // output observation wind speed threshold
      double       out_alpha;            // output alpha value
      double       out_bin_size;         // output PHIST bin size

      //
      // Variables used for the stat_job_aggr_mpr job type
      //

      char        *mask_grid;
      char        *mask_poly;

      Grid         grid_mask;
      MaskPoly     poly_mask;


      //
      // Variables used for computing bootstrap confidence intervals
      //

      //
      // Type of bootstrap confidence interval method:
      //    0 = BCa, 1 = Percentile (Default = 1)
      //
      int boot_interval;

      //
      // When using the percentile method, this is the proportion
      // of n to be resampled. (Default = 1.0)
      //
      double boot_rep_prop;

      //
      // Number of bootstrap replicates to be done. (Default = 1000)
      //
      int n_boot_rep;

      //
      // Name and seed value for the bootstrap random number generator.
      // (Default = "mnt19937" and "")
      //
      char *boot_rng;
      char *boot_seed;

      //
      // Rank correlation flag
      //
      int rank_corr_flag;

      //
      // Variance Inflation Factor flag
      //
      int vif_flag;

      void process_mask_grid();
      void process_mask_poly();

      int is_in_mask_grid(double, double) const;
      int is_in_mask_poly(double, double) const;
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __STAT_JOB_H__  */

////////////////////////////////////////////////////////////////////////
