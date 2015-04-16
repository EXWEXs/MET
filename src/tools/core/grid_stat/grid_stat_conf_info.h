// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_STAT_CONF_INFO_H__
#define  __GRID_STAT_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"
#include "vx_statistics.h"
#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////

// Indices for the output flag types in the configuration file
static const int i_fho    = 0;
static const int i_ctc    = 1;
static const int i_cts    = 2;
static const int i_mctc   = 3;
static const int i_mcts   = 4;
static const int i_cnt    = 5;
static const int i_sl1l2  = 6;
static const int i_vl1l2  = 7;
static const int i_pct    = 8;
static const int i_pstd   = 9;
static const int i_pjc    = 10;
static const int i_prc    = 11;
static const int i_nbrctc = 12;
static const int i_nbrcts = 13;
static const int i_nbrcnt = 14;
static const int n_txt    = 15;

// Text file type
static const STATLineType txt_file_type[n_txt] = {
   stat_fho,    stat_ctc,    stat_cts,
   stat_mctc,   stat_mcts,   stat_cnt,
   stat_sl1l2,  stat_vl1l2,  stat_pct,
   stat_pstd,   stat_pjc,    stat_prc,
   stat_nbrctc, stat_nbrcts, stat_nbrcnt
};

////////////////////////////////////////////////////////////////////////


struct GridStatNcOutInfo {

   bool do_latlon;

   bool do_raw;

   bool do_diff;

      //////////////////

   GridStatNcOutInfo();

   void clear();   //  sets everything to true

   bool all_false() const;

   void set_all_false();
   void set_all_true();

};


////////////////////////////////////////////////////////////////////////

class GridStatConfInfo {

   private:

      void init_from_scratch();

      // Counts based on the contents of the config file
      int n_vx;          // Number of fields to be verified
      int n_vx_scal;     // Number of scalar fields to be verified
      int n_vx_vect;     // Number of vector fields to be verified
      int n_vx_prob;     // Number of probability fields to be verified
      int n_mask;        // Number of masking regions
      int n_interp;      // Number of interpolation methods

      int max_n_scal_thresh;      // Maximum number of scalar thresholds
      int max_n_prob_fcst_thresh; // Maximum fcst prob thresholds
      int max_n_prob_obs_thresh;  // Maximum obs prob thresholds

   public:

      // Grid-Stat configuration object
      MetConfig conf;

      // Store data parsed from the Grid-Stat configuration object
      ConcatString      model;              // Model name
      ConcatString      obtype;             // Observation type
      RegridInfo        regrid_info;        // Regridding information
      VarInfo **        fcst_info;          // Array of pointers for fcst VarInfo [n_vx]
      VarInfo **        obs_info;           // Array of pointers for obs VarInfo [n_vx]
      ThreshArray *     fcst_ta;            // Array for fcst thresholds [n_vx]
      ThreshArray *     obs_ta;             // Array for obs thresholds [n_vx]
      ThreshArray       fcst_wind_ta;       // Wind speed fcst thresholds
      ThreshArray       obs_wind_ta;        // Wind speed obs thresholds
      StringArray       mask_name;          // Masking region names [n_mask]
      DataPlane *       mask_dp;            // Array for masking regions [n_mask]
      NumArray          ci_alpha;           // Alpha value for confidence intervals
      BootIntervalType  boot_interval;      // Bootstrap CI type
      double            boot_rep_prop;      // Bootstrap replicate proportion
      int               n_boot_rep;         // Number of bootstrap replicates
      ConcatString      boot_rng;           // GSL random number generator
      ConcatString      boot_seed;          // GSL RNG seed value
      FieldType         interp_field;       // How to apply interpolation options
      double            interp_thresh;      // Proportion of valid data values
      InterpMthd *      interp_mthd;        // Array for interpolation methods [n_interp]
      IntArray          interp_wdth;        // Array for interpolation widths [n_interp]
      double            nbrhd_thresh;       // Proportion of valid data values
      IntArray          nbrhd_wdth;         // Array for neighborhood widths
      ThreshArray       nbrhd_cov_ta;       // Neighborhood coverage thresholds
      STATOutputType    output_flag[n_txt]; // Flag for each output line type
      GridStatNcOutInfo nc_info;            // Output NetCDF pairs file contents
      bool              rank_corr_flag;     // Flag for computing rank correlations
      ConcatString      tmp_dir;            // Directory for temporary files
      ConcatString      output_prefix;      // String to customize output file name
      ConcatString      version;            // Config file version

      GridStatConfInfo();
     ~GridStatConfInfo();

      void clear();

      void read_config   (const char *, const char *);
      void process_config(GrdFileType, GrdFileType);
      void process_masks (const Grid &);

      void parse_nc_info();

      // Dump out the counts
      int get_n_vx()          const;
      int get_n_vx_scal()     const;
      int get_n_vx_vect()     const;
      int get_n_vx_prob()     const;
      int get_n_mask()        const;
      int get_n_wind_thresh() const;
      int get_n_interp()      const;
      int get_n_nbrhd_wdth()  const;
      int get_n_cov_thresh()  const;
      int get_n_ci_alpha()    const;
      int get_vflag()         const;
      int get_pflag()         const;

      int get_max_n_scal_thresh()      const;
      int get_max_n_prob_fcst_thresh() const;
      int get_max_n_prob_obs_thresh()  const;

      // Compute the maximum number of output lines possible based
      // on the contents of the configuration file
      int n_txt_row (int i);
      int n_stat_row();
};

////////////////////////////////////////////////////////////////////////

inline int GridStatConfInfo::get_n_vx()          const { return(n_vx);                      }
inline int GridStatConfInfo::get_n_vx_scal()     const { return(n_vx_scal);                 }
inline int GridStatConfInfo::get_n_vx_vect()     const { return(n_vx_vect);                 }
inline int GridStatConfInfo::get_n_vx_prob()     const { return(n_vx_prob);                 }
inline int GridStatConfInfo::get_n_mask()        const { return(n_mask);                    }
inline int GridStatConfInfo::get_n_wind_thresh() const { return(fcst_wind_ta.n_elements()); }
inline int GridStatConfInfo::get_n_interp()      const { return(n_interp);                  }
inline int GridStatConfInfo::get_n_nbrhd_wdth()  const { return(nbrhd_wdth.n_elements());   }
inline int GridStatConfInfo::get_n_cov_thresh()  const { return(nbrhd_cov_ta.n_elements()); }
inline int GridStatConfInfo::get_n_ci_alpha()    const { return(ci_alpha.n_elements());     }
inline int GridStatConfInfo::get_vflag()         const { return(n_vx_vect > 0);             }
inline int GridStatConfInfo::get_pflag()         const { return(n_vx_prob > 0);             }

inline int GridStatConfInfo::get_max_n_scal_thresh() const {
   return(max_n_scal_thresh);
}
inline int GridStatConfInfo::get_max_n_prob_fcst_thresh() const {
   return(max_n_prob_fcst_thresh);
}
inline int GridStatConfInfo::get_max_n_prob_obs_thresh() const {
   return(max_n_prob_obs_thresh);
}

////////////////////////////////////////////////////////////////////////

#endif   /*  __GRID_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
