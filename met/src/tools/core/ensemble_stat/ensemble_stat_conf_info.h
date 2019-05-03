// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __ENSEMBLE_STAT_CONF_INFO_H__
#define  __ENSEMBLE_STAT_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "ensemble_stat_Conf.h"

#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gsl_prob.h"
#include "vx_statistics.h"
#include "result.h"

////////////////////////////////////////////////////////////////////////

// Indices for the output flag types in the configuration file
static const int i_rhist    = 0;
static const int i_orank    = 1;
static const int i_nc_mean  = 2;
static const int i_nc_stdev = 3;
static const int i_nc_minus = 4;
static const int i_nc_plus  = 5;
static const int i_nc_min   = 6;
static const int i_nc_max   = 7;
static const int i_nc_range = 8;
static const int i_nc_vld   = 9;
static const int i_nc_freq  = 10;
static const int i_nc_orank = 11;

static const int n_txt      = 2;
static const int n_out      = 12;

// Enumeration to store possible output flag values
enum OutputFlag {
   flag_no_out   = 0,
   flag_stat_out = 1,
   flag_txt_out  = 2
};

////////////////////////////////////////////////////////////////////////

class EnsembleStatConfInfo {

   private:

      void init_from_scratch();

      // Ensemble processing
      int n_ens_var;    // Number of ensemble fields to be processed
      int max_n_thresh; // Maximum number of ensemble thresholds

      // Ensemble verification
      int n_vx;         // Number of ensemble fields to be verified
      int n_msg_typ;    // Number of verifying message types
      int n_interp;     // Number of interpolation methods
      int n_mask;       // Number of masking regions
      int n_mask_area;  // Number of masking areas
      int n_mask_sid;   // Number of masking station id's

   public:

      // Ensemble-Stat configuration object
      ensemble_stat_Conf conf;

      // Various objects to store the data that's parsed from the
      // Ensemble-Stat configuration object
      VarInfo     **ens_info;     // Array of pointers for ensemble [n_ens_var]
      ThreshArray  *ens_ta;       // Array for ensemble thresholds [n_ens_var]

      VxPairDataEnsemble *vx_pd;  // Array for ensemble pair data [n_vx]

      ConcatString *msg_typ;      // Array of message types [n_msg_typ]

      InterpMthd   *interp_mthd;  // Array for interpolation methods [n_interp]
      int          *interp_wdth;  // Array for interpolation widths [n_interp]

      DataPlane    *mask_dp;      // Array for masking regions [n_mask]
      ConcatString *mask_name;    // Masking region names [n_mask]
      StringArray   mask_sid;     // Masking station id's

      EnsembleStatConfInfo();
     ~EnsembleStatConfInfo();

      void clear();

      void read_config   (const char *, const char *,
                          GrdFileType, unixtime, int,
                          GrdFileType, unixtime, int);
      void process_config(GrdFileType, unixtime, int,
                          GrdFileType, unixtime, int);
      void process_masks (const Grid &);
      void set_vx_pd     ();

      // Dump out the counts
      int get_n_ens_var()     const;
      int get_max_n_thresh()  const;
      int get_n_vx()          const;
      int get_n_msg_typ()     const;
      int get_n_interp()      const;
      int get_n_mask()        const;
      int get_n_mask_area()   const;
      int get_n_mask_sid()    const;

      // Compute the maximum number of output lines possible based
      // on the contents of the configuration file
      int n_txt_row(int i);
      int n_stat_row();
};

////////////////////////////////////////////////////////////////////////

inline int EnsembleStatConfInfo::get_n_ens_var()     const { return(n_ens_var);    }
inline int EnsembleStatConfInfo::get_max_n_thresh()  const { return(max_n_thresh); }
inline int EnsembleStatConfInfo::get_n_vx()          const { return(n_vx);         }
inline int EnsembleStatConfInfo::get_n_msg_typ()     const { return(n_msg_typ);    }
inline int EnsembleStatConfInfo::get_n_interp()      const { return(n_interp);     }
inline int EnsembleStatConfInfo::get_n_mask()        const { return(n_mask);       }
inline int EnsembleStatConfInfo::get_n_mask_area()   const { return(n_mask_area);  }
inline int EnsembleStatConfInfo::get_n_mask_sid()    const { return(n_mask_sid);   }

////////////////////////////////////////////////////////////////////////

#endif   /*  __ENSEMBLE_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
