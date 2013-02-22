// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   aggr_stat_line.h
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add aggr_rhist_lines and
//                    aggr_orank_lines.
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

#ifndef  __AGGR_STAT_LINE_H__
#define  __AGGR_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cmath>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "vx_analysis_util.h"
#include "vx_gsl_prob.h"
#include "vx_util.h"
#include "vx_statistics.h"

////////////////////////////////////////////////////////////////////////

static const int min_time_series = 10;

////////////////////////////////////////////////////////////////////////

struct AggrCTCInfo {
   CTSInfo cts_info;
   NumArray valid_ts, baser_ts, fmean_ts, acc_ts;
   NumArray pody_ts, podn_ts, pofd_ts, far_ts, csi_ts, hk_ts;
};

struct AggrMCTCInfo {
   MCTSInfo mcts_info;
   NumArray valid_ts, acc_ts;
};

struct AggrPCTInfo {
   PCTInfo pct_info;
   NumArray valid_ts, baser_ts, brier_ts;
};

struct AggrPSumInfo {
   SL1L2Info  sl1l2_info;
   VL1L2Info  vl1l2_info;
   CNTInfo    cnt_info;
   NBRCNTInfo nbrcnt_info;
   NumArray valid_ts, fbar_ts, obar_ts, me_ts;
};

struct AggrWindInfo {
   VL1L2Info vl1l2_info;
   NumArray uf_na, vf_na, uo_na, vo_na;
};

struct AggrMPRInfo {
   NumArray f_na, o_na, c_na;
   ConcatString fcst_var, obs_var;
};

struct AggrISCInfo {
   ISCInfo isc_info;
   NumArray *total_na, *mse_na, *fen_na, *oen_na, *baser_na, *fbias_na;
};

struct AggrRHISTInfo {
   PairDataEnsemble ens_pd;
   double crps_num, crps_den, ign_num, ign_den;
};

struct AggrORANKInfo {
   PairDataEnsemble ens_pd;
};

////////////////////////////////////////////////////////////////////////

extern void aggr_ctc_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrCTCInfo> &,
               int &, int &);

extern void aggr_mctc_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrMCTCInfo> &,
               int &, int &);

extern void aggr_pct_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrPCTInfo> &,
               int &, int &);

extern void aggr_psum_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrPSumInfo> &,
               int &, int &);

extern void aggr_wind_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrWindInfo> &,
               int &, int &);

extern void aggr_mpr_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrMPRInfo> &,
               int &, int &);
               
extern void aggr_isc_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrISCInfo> &,
               int &, int &);

extern void aggr_rhist_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrRHISTInfo> &,
               int &, int &);

extern void aggr_orank_lines(
               LineDataFile &, STATAnalysisJob &,
               map<ConcatString, AggrORANKInfo> &,
               int &, int &);

////////////////////////////////////////////////////////////////////////

extern void mpr_to_ctc(
               STATAnalysisJob &, const AggrMPRInfo &,
               CTSInfo &);

extern void mpr_to_cts(
               STATAnalysisJob &, const AggrMPRInfo &,
               CTSInfo &, const char *);

extern void mpr_to_mctc(
               STATAnalysisJob &, const AggrMPRInfo &,
               MCTSInfo &);

extern void mpr_to_mcts(
               STATAnalysisJob &, const AggrMPRInfo &,
               MCTSInfo &, const char *);

extern void mpr_to_cnt(
               STATAnalysisJob &, const AggrMPRInfo &,
               CNTInfo &, const char *);
               

extern void mpr_to_psum(STATAnalysisJob &, const AggrMPRInfo &,
               SL1L2Info &);

extern void mpr_to_pct(
               STATAnalysisJob &, const AggrMPRInfo &,
               PCTInfo &);

////////////////////////////////////////////////////////////////////////
               
extern double compute_vif(NumArray &);

extern void write_case_cols(const ConcatString &, AsciiTable &,
                            int &, int &);

////////////////////////////////////////////////////////////////////////

#endif   //  __AGGR_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////
