// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gsidens2orank.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    06/16/15  Halley Gotway   New
//
////////////////////////////////////////////////////////////////////////

#ifndef  __GSIDENS2ORANK_H__
#define  __GSIDENS2ORANK_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vx_util.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char *program_name = "gsidens2orank";
static const int   rec_pad_length = 4;
static const bool  swap_endian = true;
static const int   bad_setup_qc = -999;

// Defaults for output header
static const char  *default_model       = "GSI";
static const int    default_lead        = 0;
static const char  *default_lev         = na_str;
static const char  *default_obtype      = na_str;
static const char  *default_vx_mask     = na_str;
static const char  *default_interp_mthd = na_str;
static const int    default_interp_wdth = 0;
static const char  *default_thresh      = na_str;
static const double default_alpha       = bad_data_double;
static const char  *default_line_type   = "MPR";

////////////////////////////////////////////////////////////////////////

static const char *conv_extra_columns [] = {
   "OBS_HGT",      // observation height                  (7)

   "OBS_ERR_IN",   // prepbufr inverse observation error  (14)
   "OBS_ERR_ADJ",  // read_prepbufr inverse obs error     (15)
   "OBS_ERR_FIN",  // final inverse observation error     (16)

   "PREP_USE",     // read_prepbufr usage                 (11)
   "ANLY_USE",     // analysis usage                      (12)

   "SETUP_QC",     // setup qc                            (10)
   "QC_WGHT"       // non-linear qc rel weight            (13)
};

static const int n_conv_extra_cols = sizeof(conv_extra_columns)/sizeof(*conv_extra_columns);

////////////////////////////////////////////////////////////////////////

static const char * rad_extra_columns [] = {
   "CHAN_USE",   // channel used
   "SCAN_POS",   // sensor scan position
   "SAT_ZNTH",   // satellite zenith angle (degrees)
   "SAT_AZMTH",  // satellite azimuth angle (degrees)
   "SUN_ZNTH",   // solar zenith angle (degrees)
   "SUN_AZMTH",  // solar azimuth angle (degrees)
   "SUN_GLNT",   // sun glint angle (degrees)
   "FRAC_WTR",   // fractional coverage by water
   "FRAC_LND",   // fractional coverage by land
   "FRAC_ICE",   // fractional coverage by ice
   "FRAC_SNW",   // fractional coverage by snow
   "SFC_TWTR",   // surface temperature over water (K)
   "SFC_TLND",   // surface temperature over land (K)
   "SFC_TICE",   // surface temperature over ice (K)
   "SFC_TSNW",   // surface temperature over snow (K)
   "TSOIL",      // soil temperature (K)
   "SOILM",      // soil moisture
   "LAND_TYPE",  // surface land type
   "FRAC_VEG",   // vegetation fraction
   "SNW_DPTH",   // snow depth
   "SFC_WIND",   // surface wind speed (m/s)
   "FRAC_CLD",   // cloud fraction (%)
   "CTOP_PRS",   // cloud top pressure (hPa)
   "ITREF",      // itref
   "IDTW",       // idtw
   "IDTC",       // idtc
   "ITZ_TR",     // itz_tr
   "OBS_ERR",    // inverse observation error
   "FCST_NOBC",  // Tb with no bias correction (K)
   "SFC_EMIS",   // surface emissivity
   "STABILITY",  // stability index
   "PRS"         // extra pressure
};

static const int n_rad_extra_cols = sizeof(rad_extra_columns)/sizeof(*rad_extra_columns);

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static NumArray       channel;
static StringArray    hdr_name;
static StringArray    hdr_value;
static ConcatString   suffix = ".stat";
static ConcatString   output_directory = ".";
static StatHdrColumns shc;

////////////////////////////////////////////////////////////////////////

#endif   /*  __GSIDENS2ORANK_H__  */

////////////////////////////////////////////////////////////////////////
