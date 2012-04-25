// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_CONSTANTS_H__
#define __CONFIG_CONSTANTS_H__

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

//
// Enumeration for output_flag configuration parameter
//

enum STATOutputType {
   STATOutputType_None, // Do not output this line type
   STATOutputType_Stat, // Write output to the .stat file
   STATOutputType_Both  // Write output to .stat and .txt files
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for all the possible STAT line types
//

enum STATLineType {
   stat_sl1l2,
   stat_sal1l2,
   stat_vl1l2,
   stat_val1l2,
   stat_fho,
   stat_ctc,
   stat_cts,
   stat_mctc,
   stat_mcts,
   stat_cnt,
   stat_pct,
   stat_pstd,
   stat_pjc,
   stat_prc,
   stat_mpr,
   stat_nbrctc,
   stat_nbrcts,
   stat_nbrcnt,
   stat_isc,
   stat_wdir,
   stat_rhist,
   stat_orank,
   no_stat_line_type
};

////////////////////////////////////////////////////////////////////////

//
// Corresponding line type strings
//

static const char stat_sl1l2_str[]  = "SL1L2";
static const char stat_sal1l2_str[] = "SAL1L2";
static const char stat_vl1l2_str[]  = "VL1L2";
static const char stat_val1l2_str[] = "VAL1L2";
static const char stat_fho_str[]    = "FHO";
static const char stat_ctc_str[]    = "CTC";
static const char stat_cts_str[]    = "CTS";
static const char stat_mctc_str[]   = "MCTC";
static const char stat_mcts_str[]   = "MCTS";
static const char stat_cnt_str[]    = "CNT";
static const char stat_pct_str[]    = "PCT";
static const char stat_pstd_str[]   = "PSTD";
static const char stat_pjc_str[]    = "PJC";
static const char stat_prc_str[]    = "PRC";
static const char stat_mpr_str[]    = "MPR";
static const char stat_nbrctc_str[] = "NBRCTC";
static const char stat_nbrcts_str[] = "NBRCTS";
static const char stat_nbrcnt_str[] = "NBRCNT";
static const char stat_isc_str[]    = "ISC";
static const char stat_wdir_str[]   = "WDIR";
static const char stat_rhist_str[]  = "RHIST";
static const char stat_orank_str[]  = "ORANK";
static const char stat_na_str[]     = "NA";

////////////////////////////////////////////////////////////////////////

//
// Enumeration for bootstrapping interval configuration parameter
//

enum BootIntervalType {
   BootIntervalType_None,      // Default 
   BootIntervalType_BCA,       // Bias-Corrected and adjusted method
   BootIntervalType_Percentile // Percentile method
};

//
// Struct to store bootstrapping information
//

struct BootInfo {
   BootIntervalType interval; // Bootstrap interval type
   double           rep_prop; // Proportion of sample for replicates
   int              n_rep;    // Number of replicates
   ConcatString     rng;      // GSL random number generator
   ConcatString     seed;     // RNG seed value
};


////////////////////////////////////////////////////////////////////////

//
// Struct to store interpolation information
//

struct InterpInfo {
   double      thresh;   // Valid data interpolation threshold
   int         n_interp; // Number of interpolation types   
   StringArray method;   // Interpolation methods
   IntArray    width;    // Interpolation widths
};

////////////////////////////////////////////////////////////////////////

//
// Enumeration for duplicate_flag configuration parameter
//

enum DuplicateType {
   DuplicateType_None,   // Apply no logic for duplicate point obs
   DuplicateType_Unique, // Filter our duplicate observation values
   DuplicateType_Single  // Keep only a single observation per station
};

////////////////////////////////////////////////////////////////////////

static const char config_const_filename[] =
   "MET_BASE/data/config/ConfigConstants";

//
// Constants for parameter names used in configuartion files
//

static const char conf_version[]          = "version";
static const char conf_model[]            = "model";
static const char conf_output_flag[]      = "output_flag";
static const char conf_fcst_field[]       = "fcst.field";
static const char conf_obs_field[]        = "obs.field";
static const char conf_level[]            = "level";
static const char conf_message_type[]     = "message_type";
static const char conf_thresh[]           = "thresh";
static const char conf_fcst_wind_thresh[] = "fcst.wind_thresh";
static const char conf_obs_wind_thresh[]  = "obs.wind_thresh";
static const char conf_mask_grid[]        = "mask.grid";
static const char conf_mask_poly[]        = "mask.poly";
static const char conf_mask_sid[]         = "mask.sid";
static const char conf_ci_alpha[]         = "ci_alpha";
static const char conf_boot_interval[]    = "boot.interval";
static const char conf_boot_rep_prop[]    = "boot.rep_prop";
static const char conf_boot_n_rep[]       = "boot.n_rep";
static const char conf_boot_rng[]         = "boot.rng";
static const char conf_boot_seed[]        = "boot.seed";
static const char conf_interp_thresh[]    = "interp.thresh";
static const char conf_interp_type[]      = "interp.type";
static const char conf_method[]           = "method";
static const char conf_width[]            = "width";
static const char conf_duplicate_flag[]   = "duplicate_flag";
static const char conf_rank_corr_flag[]   = "rank_corr_flag";
static const char conf_tmp_dir[]          = "tmp_dir";
static const char conf_output_prefix[]    = "output_prefix";

//
// Constants for parameter values used in configuartion files
//

// File types
static const char conf_grib1[]       = "GRIB1";
static const char conf_grib2[]       = "GRIB2";
static const char conf_netcdf_met[]  = "NETCDF_MET";
static const char conf_netcdf_pint[] = "NETCDF_PINT";

// Output flag values
static const char conf_none[] = "NONE";
static const char conf_stat[] = "STAT";

// Bootstrapping interval type
static const char conf_pctile[] = "PCTILE";
static const char conf_bca[]    = "BCA";

// Interpolation types
static const char conf_fcst[] = "FCST";
static const char conf_obs[]  = "OBS";
static const char conf_both[] = "BOTH";

// Duplicate flag values
static const char conf_unique[] = "UNIQUE";
static const char conf_single[] = "SINGLE";

////////////////////////////////////////////////////////////////////////

#endif   //  __CONFIG_CONSTANTS_H__

////////////////////////////////////////////////////////////////////////
