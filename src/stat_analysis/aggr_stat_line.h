// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "vx_analysis_util/vx_analysis_util.h"
#include "vx_util/vx_util.h"
#include "vx_met_util/vx_met_util.h"
#include "vx_contable/vx_contable.h"

////////////////////////////////////////////////////////////////////////

extern void aggr_contable_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &, CTSInfo &,
               STATLineType, int &, int &, int);

extern void aggr_mctc_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &, MCTSInfo &,
               STATLineType, int &, int &, int);

extern void aggr_nx2_contable_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &, PCTInfo &,
               STATLineType, int &, int &, int);

extern void aggr_partial_sum_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &, SL1L2Info &, VL1L2Info &,
               STATLineType, int &, int &, int);

extern void aggr_vl1l2_wdir(
               const char *, LineDataFile &, STATAnalysisJob &,
               VL1L2Info &,
               NumArray &, NumArray &, NumArray &, NumArray &,
               STATLineType, int &, int &, int);

extern void read_mpr_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &,
               int &, int &,
               NumArray &, NumArray &, NumArray &,
               int &, int &, int);

extern void aggr_mpr_lines_ctc(
               STATAnalysisJob &,
               const NumArray &, const NumArray &,
               CTSInfo &);

extern void aggr_mpr_lines_cts(
               STATAnalysisJob &,
               const NumArray &, const NumArray &,
               CTSInfo &, const char *);

extern void aggr_mpr_lines_mctc(
               STATAnalysisJob &,
               const NumArray &, const NumArray &,
               MCTSInfo &);

extern void aggr_mpr_lines_mcts(
               STATAnalysisJob &,
               const NumArray &, const NumArray &,
               MCTSInfo &, const char *);

extern void aggr_mpr_lines_cnt(
               STATAnalysisJob &,
               int, int,
               const NumArray &, const NumArray &,
               CNTInfo &, const char *);

extern void aggr_mpr_lines_psums(
               STATAnalysisJob &,
               const NumArray &, const NumArray &, const NumArray &,
               SL1L2Info &);

extern void aggr_mpr_lines_pct(
               STATAnalysisJob &,
               const NumArray &, const NumArray &,
               PCTInfo &);

extern void aggr_isc_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &, ISCInfo &,
               int &, int &, int);

extern void aggr_rhist_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &, NumArray &,
               int &, int &, int);

extern void aggr_orank_lines(
               const char *, LineDataFile &,
               STATAnalysisJob &, NumArray &,
               int &, int &, int);

////////////////////////////////////////////////////////////////////////

#endif   //  __AGGR_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////
