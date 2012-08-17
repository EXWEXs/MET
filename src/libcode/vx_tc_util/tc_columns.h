// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_COLUMNS_H__
#define  __TC_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "vx_util.h"

#include "tc_hdr_columns.h"
#include "track_pair_info.h"

////////////////////////////////////////////////////////////////////////

static const char * tc_header_cols [] = {
   "VERSION",      "AMODEL",     "BMODEL",
   "BASIN",        "CYCLONE",    "STORM_NAME",
   "INIT",         "LEAD",       "VALID",
   "INIT_MASK",    "VALID_MASK", "LINE_TYPE"
};

// The last 10 columns repeat for each wind intensity value
static const char * tc_mpr_cols [] = {
   "TOTAL",       "INDEX",      "LEVEL",
   "WATCH_WARN",  "ALAT",       "ALON",
   "BLAT",        "BLON",
   "TK_ERR",      "X_ERR",      "Y_ERR",
   "ALTK_ERR",    "CRTK_ERR",
   "ADLAND",      "BDLAND",
   "AMSLP",       "BMSLP",
   "AMAX_WIND",   "BMAX_WIND",
   "AAL_WIND_",   "BAL_WIND_",
   "ANE_WIND_",   "BNE_WIND_",
   "ASE_WIND_",   "BSE_WIND_",
   "ASW_WIND_",   "BSW_WIND_",
   "ANW_WIND_",   "BNW_WIND_"
};

////////////////////////////////////////////////////////////////////////

static const int n_tc_header_cols = sizeof(tc_header_cols)/sizeof(*tc_header_cols);
static const int n_tc_mpr_var     = 10;
static const int n_tc_mpr_static  = sizeof(tc_mpr_cols)/sizeof(*tc_mpr_cols) - n_tc_mpr_var;
static const int n_tc_mpr_cols    = n_tc_mpr_static + (n_tc_mpr_var * NWinds);

////////////////////////////////////////////////////////////////////////

static const char * tc_cols_track [] = {
  "TK_ERR", "ALTK_ERR", "CRTK_ERR"
};
static const int n_tc_cols_track = sizeof(tc_cols_track)/sizeof(*tc_cols_track);

static const char * tc_cols_wind [] = {
   "ABS(AAL_WIND_34-BAL_WIND_34)",
   "ABS(ANE_WIND_34-BNE_WIND_34)",
   "ABS(ASE_WIND_34-BSE_WIND_34)",
   "ABS(ASW_WIND_34-BSW_WIND_34)",
   "ABS(ANW_WIND_34-BNW_WIND_34)",
   "ABS(AAL_WIND_50-BAL_WIND_50)",
   "ABS(ANE_WIND_50-BNE_WIND_50)",
   "ABS(ASE_WIND_50-BSE_WIND_50)",
   "ABS(ASW_WIND_50-BSW_WIND_50)",
   "ABS(ANW_WIND_50-BNW_WIND_50)",
   "ABS(AAL_WIND_64-BAL_WIND_64)",
   "ABS(ANE_WIND_64-BNE_WIND_64)",
   "ABS(ASE_WIND_64-BSE_WIND_64)",
   "ABS(ASW_WIND_64-BSW_WIND_64)",
   "ABS(ANW_WIND_64-BNW_WIND_64)"
};
static const int n_tc_cols_wind = sizeof(tc_cols_wind)/sizeof(*tc_cols_wind);

static const char * tc_cols_ti [] = {
  "ABS(TK_ERR)", "ABS(AMAX_WIND-BMAX_WIND)"
};
static const int n_tc_cols_ti = sizeof(tc_cols_ti)/sizeof(*tc_cols_ti);

static const char * tc_cols_ac [] = {
  "ABS(ALTK_ERR)", "ABS(CRTK_ERR)"
};
static const int n_tc_cols_ac = sizeof(tc_cols_ac)/sizeof(*tc_cols_ac);

static const char * tc_cols_xy [] = {
  "ABS(X_ERR)", "ABS(Y_ERR)"
};
static const int n_tc_cols_xy = sizeof(tc_cols_xy)/sizeof(*tc_cols_xy);

////////////////////////////////////////////////////////////////////////

extern int get_tc_col_offset    (const char **, int, const char *);
extern int get_tc_mpr_col_offset(const char *);
extern int parse_wind_intensity (const char *);

////////////////////////////////////////////////////////////////////////

extern void open_tc_txt_file (ofstream *&,  const char *);
extern void close_tc_txt_file(ofstream *&,  const char *);

////////////////////////////////////////////////////////////////////////

// Write out the header row for fixed length line types
extern void write_tc_header_row(const char **, int, int,
                                AsciiTable &, int, int);

// Write out the header row for variable length line types
extern void write_tc_mpr_header_row(int, AsciiTable &, int, int);

// Write the TCMPR line type
extern void write_tc_mpr_row(TcHdrColumns &, const TrackPairInfo &,
                             AsciiTable &, int &);

// Write out the header entries
extern void write_tc_header_cols(const TcHdrColumns &, AsciiTable &, int);

// Write out the TCMPR entries
extern void write_tc_mpr_cols(const TrackPairInfo &, int, AsciiTable &, int, int);

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
