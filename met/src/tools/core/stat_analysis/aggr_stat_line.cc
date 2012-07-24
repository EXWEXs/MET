// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   aggr_stat_line.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/17/08  Halley Gotway   New
//   001    05/24/10  Halley Gotway   Add aggr_rhist_lines and
//                    aggr_orank_lines.
//   002    06/09/10  Halley Gotway   Add aggr_mctc_lines.
//   003    06/21/10  Halley Gotway   Add support for vif_flag.
//   004    07/28/10  Halley Gotway   Write out lines prior to error
//                    checking and add input line to error messages.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>

#include "vx_log.h"
#include "vx_data2d_grib.h"
#include "vx_data2d_nc_pinterp.h"

#include "aggr_stat_line.h"
#include "parse_stat_line.h"

////////////////////////////////////////////////////////////////////////

extern bool is_precip_var_name(const ConcatString &s);

////////////////////////////////////////////////////////////////////////
//
// The aggr_contable_lines routine should only be called when the
// -line_type option has been used exactly once.
//
////////////////////////////////////////////////////////////////////////

void aggr_contable_lines(const char *jobstring, LineDataFile &f,
                         STATAnalysisJob &j, CTSInfo &cts_info,
                         STATLineType lt, int &n_in, int &n_out) {
   STATLine line;
   TTContingencyTable ct;
   CTSInfo cts_tmp;
   int fy_oy, fy_on, fn_oy, fn_on, n, n_ties;

   // Keep track of scores for each time for computing VIF
   unixtime ut;
   NumArray valid_ts, baser_ts, fmean_ts, acc_ts;
   NumArray pody_ts, podn_ts, pofd_ts, far_ts, csi_ts, hk_ts;

   //
   // Initialize the Contingency Table counts
   //
   fy_oy = fy_on = fn_oy = fn_on = 0;
   cts_info.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         //
         // Zero out the contingecy table object
         //
         ct.zero_out();

         //
         // Switch on the line type looking only for contingency
         // table types of lines
         //
         switch(line.type()) {

            case(stat_fho):
               parse_fho_ctable(line, ct);
               break;

            case(stat_ctc):
               parse_ctc_ctable(line, ct);
               break;

            case(stat_nbrctc):
               parse_nbrctc_ctable(line, ct);
               break;

            default:
               mlog << Error << "\naggr_contable_lines() -> "
                    << "line type value of " << statlinetype_to_string(line.type())
                    << " not currently supported for the aggregation job!\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         //
         // Increment the ctable counts
         //
         fy_oy += ct.fy_oy();
         fy_on += ct.fy_on();
         fn_oy += ct.fn_oy();
         fn_on += ct.fn_on();

         //
         // Keep track of scores for each time step for VIF
         //
         if(j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = yyyymmdd_hhmmss_to_unix(line.get_item(fcst_valid_beg_offset));

            if(valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_contable_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            cts_tmp.clear();
            cts_tmp.cts = ct;
            cts_tmp.compute_stats();

            //
            // Append the stats
            //
            baser_ts.add(cts_tmp.baser.v);
            fmean_ts.add(cts_tmp.fmean.v);
            acc_ts.add(cts_tmp.acc.v);
            pody_ts.add(cts_tmp.pody.v);
            podn_ts.add(cts_tmp.podn.v);
            pofd_ts.add(cts_tmp.pofd.v);
            far_ts.add(cts_tmp.far.v);
            csi_ts.add(cts_tmp.csi.v);
            hk_ts.add(cts_tmp.hk.v);
         }

         n_out++;
      }
   } // end while

   //
   // Store the ctable sums
   //
   cts_info.cts.set_fy_oy(fy_oy);
   cts_info.cts.set_fy_on(fy_on);
   cts_info.cts.set_fn_oy(fn_oy);
   cts_info.cts.set_fn_on(fn_on);

   //
   // Check for the minimum length of time series
   //
   if(j.vif_flag && valid_ts.n_elements() < min_time_series) {
      mlog << Warning << "\naggr_contable_lines() -> "
           << "the variance inflation factor adjustment can only "
           << "be computed for at least " << min_time_series
           << " unique valid times.\n\n";
      j.vif_flag = 0;
   }

   //
   // Compute the auto-correlations for VIF
   //
   if(j.vif_flag) {

      //
      // Sort the valid times
      //
      n = valid_ts.rank_array(n_ties);

      if(n_ties > 0 || n != valid_ts.n_elements()) {
         mlog << Error << "\naggr_contable_lines() -> "
              << "should be no ties in the valid time array!\n\n";
         throw(1);
      }

      //
      // Sort the stats into time order
      //
      baser_ts.reorder(valid_ts);
      fmean_ts.reorder(valid_ts);
      acc_ts.reorder(valid_ts);
      pody_ts.reorder(valid_ts);
      podn_ts.reorder(valid_ts);
      pofd_ts.reorder(valid_ts);
      far_ts.reorder(valid_ts);
      csi_ts.reorder(valid_ts);
      hk_ts.reorder(valid_ts);

      //
      // Compute the lag 1 autocorrelation
      //
      cts_info.baser.vif = compute_vif(baser_ts);
      cts_info.fmean.vif = compute_vif(fmean_ts);
      cts_info.acc.vif   = compute_vif(acc_ts);
      cts_info.pody.vif  = compute_vif(pody_ts);
      cts_info.podn.vif  = compute_vif(podn_ts);
      cts_info.pofd.vif  = compute_vif(pofd_ts);
      cts_info.far.vif   = compute_vif(far_ts);
      cts_info.csi.vif   = compute_vif(csi_ts);
      cts_info.hk.vif    = compute_vif(hk_ts);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mctc_lines(const char *jobstring, LineDataFile &f,
                     STATAnalysisJob &j, MCTSInfo &mcts_info,
                     STATLineType lt, int &n_in, int &n_out) {
   STATLine line;
   ContingencyTable mct;
   MCTSInfo mcts_tmp;
   int i, k, cur, n, n_ties;

   // Keep track of scores for each time for computing VIF
   unixtime ut;
   NumArray valid_ts, acc_ts;

   //
   // Initialize
   //
   mcts_info.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         //
         // Initialize
         //
         mct.clear();

         //
         // Switch on the line type looking only for multi-category
         // contingency table types of lines
         //
         switch(line.type()) {

            case(stat_mctc):
               parse_mctc_ctable(line, mct);
               break;

            default:
               mlog << Error << "\naggr_mctc_lines() -> "
                    << "line type value of " << statlinetype_to_string(line.type())
                    << " not currently supported for the aggregation job!\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         //
         // Store the first mulit-category contingency table
         //
         if(mcts_info.cts.total() == 0) mcts_info.cts = mct;
         //
         // Increment the multi-category contingency table counts
         //
         else {

            //
            // The size of the contingency table must remain the same
            //
            if(mcts_info.cts.nrows() != mct.nrows()) {
               mlog << Error << "\naggr_mctc_lines() -> "
                    << "when aggregating MCTC lines the size of the "
                    << "contingency table must remain the same for all "
                    << "lines.  Try setting \"-column_eq N_CAT n\", "
                    << mcts_info.cts.nrows() << " != "
                    << mct.nrows() << "\n\n";
               throw(1);
            }

            for(i=0; i<mcts_info.cts.nrows(); i++) {
               for(k=0; k<mcts_info.cts.ncols(); k++) {

                  //
                  // Increment the counts
                  //
                  cur = mcts_info.cts.entry(i, k);
                  mcts_info.cts.set_entry(i, k, cur + mct.entry(i, k));
               } //end for k
            } // end for i
         } // end else

         //
         // Keep track of scores for each time step for VIF
         //
         if(j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = yyyymmdd_hhmmss_to_unix(line.get_item(fcst_valid_beg_offset));

            if(valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_mctc_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            mcts_tmp.clear();
            mcts_tmp.cts = mct;
            mcts_tmp.compute_stats();

            //
            // Append the stats
            //
            acc_ts.add(mcts_tmp.acc.v);
         }

         n_out++;
      }
   } // end while

   //
   // Check for the minimum length of time series
   //
   if(j.vif_flag && valid_ts.n_elements() < min_time_series) {
      mlog << Warning << "\naggr_mctc_lines() -> "
           << "the variance inflation factor adjustment can only "
           << "be computed for at least " << min_time_series
           << " unique valid times.\n\n";
      j.vif_flag = 0;
   }

   //
   // Compute the auto-correlations for VIF
   //
   if(j.vif_flag) {

      //
      // Sort the valid times
      //
      n = valid_ts.rank_array(n_ties);

      if(n_ties > 0 || n != valid_ts.n_elements()) {
         mlog << Error << "\naggr_mctc_lines() -> "
              << "should be no ties in the valid time array!\n\n";
         throw(1);
      }

      //
      // Sort the stats into time order
      //
      acc_ts.reorder(valid_ts);

      //
      // Compute the lag 1 autocorrelation
      //
      mcts_info.acc.vif = compute_vif(acc_ts);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_nx2_contable_lines(const char *jobstring, LineDataFile &f,
                             STATAnalysisJob &j, PCTInfo &pct_info,
                             STATLineType lt, int &n_in, int &n_out) {
   STATLine line;
   Nx2ContingencyTable pct;
   PCTInfo pct_tmp;
   int i, oy, on, n, n_ties;

   // Keep track of scores for each time for computing VIF
   unixtime ut;
   NumArray valid_ts, baser_ts, brier_ts;

   //
   // Initialize
   //
   pct_info.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         //
         // Initialize
         //
         pct.clear();

         //
         // Switch on the line type looking only for probabilistic
         // contingency table types of lines
         //
         switch(line.type()) {

            case(stat_pct):
               parse_nx2_ctable(line, pct);
               break;

            default:
               mlog << Error << "\naggr_nx2_contable_lines() -> "
                    << "line type value of " << statlinetype_to_string(line.type())
                    << " not currently supported for the aggregation job!\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         //
         // Store the first Nx2 Contingency Table
         //
         if(pct_info.pct.n() == 0) pct_info.pct = pct;
         //
         // Increment the Nx2 Contingency Table counts
         //
         else {

            //
            // The number of thresholds must remain the same
            //
            if(pct_info.pct.nrows() != pct.nrows()) {
               mlog << Error << "\naggr_nx2_contable_lines() -> "
                    << "when aggregating PCT lines the number of "
                    << "thresholds must remain the same for all lines, "
                    << pct_info.pct.nrows() << " != " << pct.nrows()
                    << "\n\n";
               throw(1);
            }

            for(i=0; i<pct_info.pct.nrows(); i++) {

               //
               // The threshold values must remain the same
               //
               if(!is_eq(pct_info.pct.threshold(i), pct.threshold(i))) {
                  mlog << Error << "\naggr_nx2_contable_lines() -> "
                       << "when aggregating PCT lines the threshold "
                       << "values must remain the same for all lines, "
                       << pct_info.pct.threshold(i) << " != "
                       << pct.threshold(i)
                       << "\n\n";
                  throw(1);
               }

               //
               // Increment the counts
               //
               oy = pct_info.pct.event_count_by_row(i);
               on = pct_info.pct.nonevent_count_by_row(i);

               pct_info.pct.set_entry(i, nx2_event_column,
                                      oy+pct.event_count_by_row(i));
               pct_info.pct.set_entry(i, nx2_nonevent_column,
                                      on+pct.nonevent_count_by_row(i));
            } // end for i
         } // end else

         //
         // Keep track of scores for each time step for VIF
         //
         if(j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = yyyymmdd_hhmmss_to_unix(line.get_item(fcst_valid_beg_offset));

            if(valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_nx2_contable_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            pct_tmp.clear();
            pct_tmp.pct = pct;
            pct_tmp.compute_stats();

            //
            // Append the stats
            //
            baser_ts.add(pct_tmp.baser.v);
            brier_ts.add(pct_tmp.brier.v);
         }

         n_out++;
      }
   } // end while

   //
   // Compute the auto-correlations for VIF
   //
   if(j.vif_flag) {

      //
      // Sort the valid times
      //
      n = valid_ts.rank_array(n_ties);

      if(n_ties > 0 || n != valid_ts.n_elements()) {
         mlog << Error << "\naggr_nx2_contable_lines() -> "
              << "should be no ties in the valid time array!\n\n";
         throw(1);
      }

      //
      // Sort the stats into time order
      //
      baser_ts.reorder(valid_ts);
      brier_ts.reorder(valid_ts);

      //
      // Compute the lag 1 autocorrelation
      //
      pct_info.baser.vif = compute_vif(baser_ts);
      pct_info.brier.vif = compute_vif(brier_ts);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_partial_sum_lines(const char *jobstring, LineDataFile &f,
                            STATAnalysisJob &j, SL1L2Info &sl1l2_info,
                            VL1L2Info &vl1l2_info, CNTInfo &cnt_info,
                            NBRCNTInfo &nbrcnt_info, STATLineType lt,
                            int &n_in, int &n_out) {
   STATLine line;
   SL1L2Info s;
   VL1L2Info v;
   NBRCNTInfo c;
   int n, n_ties;

   // Keep track of scores for each time for computing VIF
   unixtime ut;
   NumArray valid_ts, fbar_ts, obar_ts, me_ts;
   CNTInfo cnt_tmp;

   //
   // Initialize the partial sums
   //
   sl1l2_info.clear();
   vl1l2_info.clear();
   nbrcnt_info.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         //
         // Switch on the line type.
         // For each partial sum line type, clear out the object,
         // parse the new line, and add it to running sum.
         //
         switch(line.type()) {

            case(stat_sl1l2):
               s.clear();
               parse_sl1l2_line(line, s);
               sl1l2_info += s;
               break;

            case(stat_sal1l2):
               s.clear();
               parse_sal1l2_line(line, s);
               sl1l2_info += s;
               break;

            case(stat_vl1l2):
               v.clear();
               parse_vl1l2_line(line, v);
               vl1l2_info += v;
               break;

            case(stat_val1l2):
               v.clear();
               parse_val1l2_line(line, v);
               vl1l2_info += v;
               break;

            case(stat_nbrcnt):
               c.clear();
               parse_nbrcnt_line(line, c);
               nbrcnt_info += c;
               break;

            default:
               mlog << Error << "\naggr_partial_sum_lines() -> "
                    << "should only encounter partial sum line types!\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         //
         // Keep track of scores for each time step for VIF
         //
         if(line.type() == stat_sl1l2 && j.vif_flag) {

            //
            // Cannot compute VIF when the times are not unique
            //
            ut = yyyymmdd_hhmmss_to_unix(line.get_item(fcst_valid_beg_offset));

            if(valid_ts.has((double) ut)) {
               mlog << Warning << "\naggr_partial_sum_lines() -> "
                    << "the variance inflation factor adjustment can "
                    << "only be computed for time series with unique "
                    << "valid times.\n\n";
               j.vif_flag = 0;
            }
            else {
               valid_ts.add((double) ut);
            }

            //
            // Compute the stats for the current time
            //
            compute_cntinfo(s, 0, cnt_tmp);

            //
            // Append the stats
            //
            fbar_ts.add(cnt_tmp.fbar.v);
            obar_ts.add(cnt_tmp.obar.v);
            me_ts.add(cnt_tmp.me.v);
         }

         n_out++;
      }
   } // end while


   //  initialize the aggregated nbrcnt CIs
   nbrcnt_info.allocate_n_alpha(1);

   //
   // Compute the auto-correlations for VIF
   //
   if(j.out_line_type == stat_cnt && j.vif_flag) {

      //
      // Sort the valid times
      //
      n = valid_ts.rank_array(n_ties);

      if(n_ties > 0 || n != valid_ts.n_elements()) {
         mlog << Error << "\naggr_partial_sum_lines() -> "
              << "should be no ties in the valid time array!\n\n";
         throw(1);
      }

      //
      // Sort the stats into time order
      //
      fbar_ts.reorder(valid_ts);
      obar_ts.reorder(valid_ts);
      me_ts.reorder(valid_ts);

      //
      // Compute the lag 1 autocorrelation
      //
      cnt_info.fbar.vif = compute_vif(fbar_ts);
      cnt_info.obar.vif = compute_vif(obar_ts);
      cnt_info.me.vif   = compute_vif(me_ts);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_vl1l2_wdir(const char *jobstring, LineDataFile &f,
                     STATAnalysisJob &j,
                     VL1L2Info &vl1l2_info,
                     NumArray &uf_na, NumArray &vf_na,
                     NumArray &uo_na, NumArray &vo_na,
                     STATLineType lt, int &n_in, int &n_out) {
   STATLine line;
   VL1L2Info v_info;
   double u, v;

   //
   // Initialize
   //
   vl1l2_info.clear();
   uf_na.clear();
   vf_na.clear();
   uo_na.clear();
   vo_na.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         //
         // Switch on the line type.
         // For each partial sum line type, clear out the object,
         // parse the new line, and add it to running sum.
         //
         switch(line.type()) {

            case(stat_vl1l2):
               v_info.clear();
               parse_vl1l2_line(line, v_info);
               vl1l2_info += v_info;

               // Convert U and V component of winds to unit vectors,
               // and store the values.
               convert_u_v_to_unit(v_info.ufbar, v_info.vfbar, u, v);
               uf_na.add(u);
               vf_na.add(v);

               convert_u_v_to_unit(v_info.uobar, v_info.vobar, u, v);
               uo_na.add(u);
               vo_na.add(v);
               break;

            case(stat_val1l2):
               v_info.clear();
               parse_val1l2_line(line, v_info);
               vl1l2_info += v_info;

               // Convert U and V component of winds to unit vectors,
               // and store the values.
               convert_u_v_to_unit(v_info.ufabar, v_info.vfabar, u, v);
               uf_na.add(u);
               vf_na.add(v);

               convert_u_v_to_unit(v_info.uoabar, v_info.voabar, u, v);
               uo_na.add(u);
               vo_na.add(v);
               break;

            default:
               mlog << Error << "\naggr_vl1l2_wdir() -> "
                    << "should only encounter partial sum line types!\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void read_mpr_lines(const char *jobstring, LineDataFile &f,
                    STATAnalysisJob &j,
                    ConcatString &fcst_var, ConcatString &obs_var,
                    NumArray &f_na, NumArray &o_na, NumArray &c_na,
                    int &n_in, int &n_out) {
   STATLine line;
   MPRData m;

   //
   // Initialize the NumArray objects
   //
   f_na.clear();
   o_na.clear();
   c_na.clear();
   fcst_var.clear();
   obs_var.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         //
         // Switch on the line type.
         //
         switch(line.type()) {

            case(stat_mpr):

               parse_mpr_line(line, m);

               //
               // Check the grid and poly masks if specified.
               // Convert degrees_east to degrees_west.
               //
               if(!j.is_in_mask_grid(m.obs_lat, (-1.0)*m.obs_lon) ||
                  !j.is_in_mask_poly(m.obs_lat, (-1.0)*m.obs_lon))
                  continue;

               //
               // Check for bad data.
               //
               if(is_bad_data(m.fcst) || is_bad_data(m.obs)) continue;

               //
               // Store or check the variable names
               //
               if(fcst_var.empty() || obs_var.empty()) {
                  fcst_var = m.fcst_var;
                  obs_var  = m.obs_var;
               }
               else if(fcst_var != m.fcst_var || obs_var  != m.obs_var) {
                  mlog << Error << "\nread_mpr_lines() -> "
                       << "both the forecast variable type and observation "
                       << "variable type must remain constant!  Try setting "
                       << "\"-fcst_var\" and/or \"-obs_var\".\n"
                       << "ERROR occurred on STAT line:\n" << line << "\n\n";
                  throw(1);
               }

               //
               // Store the values
               //
               f_na.add(m.fcst);
               o_na.add(m.obs);
               c_na.add(m.climo);

               break;

            default:
               mlog << Error << "\nread_mpr_lines() -> "
                    << "should only encounter MPR line types!\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_ctc(STATAnalysisJob &j,
                        const NumArray &f_na,
                        const NumArray &o_na,
                        CTSInfo &cts_info) {
   int i;
   int n = f_na.n_elements();
   SingleThresh ft = j.out_fcst_thresh[0];
   SingleThresh ot = j.out_obs_thresh[0];

   //
   // Update the contingency table counts
   //
   for(i=0; i<n; i++) {

      if(      ft.check(f_na[i]) &&  ot.check(o_na[i]))
         cts_info.cts.inc_fy_oy();
      else if( ft.check(f_na[i]) && !ot.check(o_na[i]))
         cts_info.cts.inc_fy_on();
      else if(!ft.check(f_na[i]) &&  ot.check(o_na[i]))
         cts_info.cts.inc_fn_oy();
      else if(!ft.check(f_na[i]) && !ot.check(o_na[i]))
         cts_info.cts.inc_fn_on();
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_cts(STATAnalysisJob &j,
                        const NumArray &f_na, const NumArray &o_na,
                        CTSInfo &cts_info, const char *tmp_dir) {
   CTSInfo *cts_info_ptr = (CTSInfo *) 0;
   gsl_rng *rng_ptr = (gsl_rng *) 0;

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Store the out_alpha value
   //
   cts_info.allocate_n_alpha(1);
   cts_info.alpha[0] = j.out_alpha;

   //
   // Store the thresholds to be applied.
   //
   if(j.out_fcst_thresh.n_elements() == 0) {
      mlog << Error << "\naggr_mpr_lines_cts() -> "
           << "when computing CTS lines, \"-out_fcst_thresh\" must be "
           << "used.\n\n";
      throw(1);
   }
   else {
      cts_info.cts_fcst_thresh = j.out_fcst_thresh[0];
   }

   if(j.out_obs_thresh.n_elements() == 0) {
      mlog << Error << "\naggr_mpr_lines_cts() -> "
           << "when computing CTS lines, \"-out_obs_thresh\" must be "
           << "used.\n\n";
      throw(1);
   }
   else {
      cts_info.cts_obs_thresh = j.out_obs_thresh[0];
   }

   //
   // Set up the random number generator and seed value
   //
   rng_set(rng_ptr, j.boot_rng, j.boot_seed);

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   cts_info_ptr = &cts_info;
   if(j.boot_interval == boot_bca_flag) {
      compute_cts_stats_ci_bca(rng_ptr, f_na, o_na,
         j.n_boot_rep,
         cts_info_ptr, 1, 1,
         j.rank_corr_flag, tmp_dir);
   }
   else {
      compute_cts_stats_ci_perc(rng_ptr, f_na, o_na,
         j.n_boot_rep, j.boot_rep_prop,
         cts_info_ptr, 1, 1,
         j.rank_corr_flag, tmp_dir);
   }

   //
   // Deallocate memory for the random number generator
   //
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_mctc(STATAnalysisJob &j,
                         const NumArray &f_na,
                         const NumArray &o_na,
                         MCTSInfo &mcts_info) {
   int i;
   int n = f_na.n_elements();

   //
   // Initialize
   //
   mcts_info.cts.set_size(j.out_fcst_thresh.n_elements() + 1);
   mcts_info.cts_fcst_ta = j.out_fcst_thresh;
   mcts_info.cts_obs_ta  = j.out_obs_thresh;

   //
   // Update the contingency table counts
   //
   for(i=0; i<n; i++) mcts_info.add(f_na[i], o_na[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_mcts(STATAnalysisJob &j,
                         const NumArray &f_na, const NumArray &o_na,
                         MCTSInfo &mcts_info, const char *tmp_dir) {
   gsl_rng *rng_ptr = (gsl_rng *) 0;

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Initialize
   //
   mcts_info.cts.set_size(j.out_fcst_thresh.n_elements() + 1);
   mcts_info.cts_fcst_ta = j.out_fcst_thresh;
   mcts_info.cts_obs_ta  = j.out_obs_thresh;

   //
   // Store the out_alpha value
   //
   mcts_info.allocate_n_alpha(1);
   mcts_info.alpha[0] = j.out_alpha;

   //
   // Set up the random number generator and seed value
   //
   rng_set(rng_ptr, j.boot_rng, j.boot_seed);

   //
   // Compute the counts, stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(j.boot_interval == boot_bca_flag) {
      compute_mcts_stats_ci_bca(rng_ptr, f_na, o_na,
         j.n_boot_rep,
         mcts_info, 1,
         j.rank_corr_flag, tmp_dir);
   }
   else {
      compute_mcts_stats_ci_perc(rng_ptr, f_na, o_na,
         j.n_boot_rep, j.boot_rep_prop,
         mcts_info, 1,
         j.rank_corr_flag, tmp_dir);
   }

   //
   // Deallocate memory for the random number generator
   //
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_cnt(STATAnalysisJob &j,
                        const ConcatString fcst_var, const ConcatString obs_var,
                        const NumArray &f_na, const NumArray &o_na,
                        CNTInfo &cnt_info, const char *tmp_dir) {
   gsl_rng *rng_ptr = (gsl_rng *) 0;
   bool precip_flag = false;

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Set the precip flag based on fcst_var and obs_var
   //
   if(is_precip_var_name(fcst_var) && is_precip_var_name(obs_var))
      precip_flag = true;

   //
   // Store the out_alpha value
   //
   cnt_info.allocate_n_alpha(1);
   cnt_info.alpha[0] = j.out_alpha;

   //
   // Set up the random number generator and seed value
   //
   rng_set(rng_ptr, j.boot_rng, j.boot_seed);

   //
   // Compute the stats, normal confidence intervals, and
   // bootstrap confidence intervals
   //
   if(j.boot_interval == boot_bca_flag) {

      compute_cnt_stats_ci_bca(rng_ptr, f_na, o_na,
         precip_flag, j.n_boot_rep,
         cnt_info, 1, j.rank_corr_flag, tmp_dir);
   }
   else {

      compute_cnt_stats_ci_perc(rng_ptr, f_na, o_na,
         precip_flag, j.n_boot_rep, j.boot_rep_prop,
         cnt_info, 1, j.rank_corr_flag, tmp_dir);
   }

   //
   // Deallocate memory for the random number generator
   //
   rng_free(rng_ptr);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_psums(STATAnalysisJob &j,
               const NumArray &f_na,
               const NumArray &o_na,
               const NumArray &c_na,
               SL1L2Info &s_info) {
   int i;
   int n = f_na.n_elements();
   int scount, sacount;
   double f, o, c;
   double f_sum,  o_sum,  ff_sum,  oo_sum,  fo_sum;
   double fa_sum, oa_sum, ffa_sum, ooa_sum, foa_sum;

   //
   // Initialize the SL1L2Info object and counts
   //
   s_info.clear();
   scount = sacount = 0;
   f_sum  = o_sum  =  ff_sum  = oo_sum  = fo_sum  = 0.0;
   fa_sum = oa_sum =  ffa_sum = ooa_sum = foa_sum = 0.0;

   //
   // Update the partial sums
   //
   for(i=0; i<n; i++) {

      //
      // Update the counts for this matched pair
      //
      f = f_na[i];
      o = o_na[i];
      c = c_na[i];

      f_sum   += f;
      o_sum   += o;
      ff_sum  += f*f;
      oo_sum  += o*o;
      fo_sum  += f*o;
      scount  += 1;

      //
      // Check c for valid data
      //
      if(!is_bad_data(c)) {

         fa_sum  += f-c;
         oa_sum  += o-c;
         ffa_sum += (f-c)*(f-c);
         ooa_sum += (o-c)*(o-c);
         foa_sum += (f-c)*(o-c);
         sacount += 1;
      }
   } // end for

   if(scount != 0) {
      s_info.scount  = scount;
      s_info.fbar    = f_sum/scount;
      s_info.obar    = o_sum/scount;
      s_info.fobar   = fo_sum/scount;
      s_info.ffbar   = ff_sum/scount;
      s_info.oobar   = oo_sum/scount;
   }

   if(sacount != 0) {
      s_info.sacount = sacount;
      s_info.fabar   = fa_sum/sacount;
      s_info.oabar   = oa_sum/sacount;
      s_info.foabar  = foa_sum/sacount;
      s_info.ffabar  = ffa_sum/sacount;
      s_info.ooabar  = ooa_sum/sacount;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_mpr_lines_pct(STATAnalysisJob &j,
                        const NumArray &f_na,
                        const NumArray &o_na,
                        PCTInfo &pct_info) {
   int pstd_flag;

   //
   // If there are no matched pairs to process, return
   //
   if(f_na.n_elements() == 0 || o_na.n_elements() == 0) return;

   //
   // Set up the PCTInfo thresholds and alpha values
   //
   pct_info.pct_fcst_thresh = j.out_fcst_thresh;
   pct_info.pct_obs_thresh  = j.out_obs_thresh[0];
   pct_info.allocate_n_alpha(1);
   pct_info.alpha[0] = j.out_alpha;

   if(j.out_line_type == stat_pstd) pstd_flag = 1;
   else                             pstd_flag = 0;

   //
   // Compute the probabilistic counts and statistics
   //
   compute_pctinfo(f_na, o_na, pstd_flag, pct_info);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_isc_lines(const char *jobstring, LineDataFile &ldf,
               STATAnalysisJob &j, ISCInfo &isc_aggr,
               int &n_in, int &n_out) {
   STATLine line;
   ISCInfo isc_info;

   int i, k, n_scale, iscale;
   double total, w, den, baser_fbias_sum;
   NumArray *total_na = (NumArray *) 0;
   NumArray *mse_na   = (NumArray *) 0;
   NumArray *fen_na   = (NumArray *) 0;
   NumArray *oen_na   = (NumArray *) 0;
   NumArray *baser_na = (NumArray *) 0;
   NumArray *fbias_na = (NumArray *) 0;

   //
   // Initialize the ISCInfo objects
   //
   isc_aggr.clear();

   //
   // Process the STAT lines
   //
   while(ldf >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         //
         // Switch on the line type.
         //
         switch(line.type()) {

            case(stat_isc):

               parse_isc_line(line, isc_info, iscale);

               //
               // Check for bad data
               //
               if(is_bad_data(isc_info.total) ||
                  is_bad_data(isc_info.mse)   ||
                  is_bad_data(isc_info.fen)   ||
                  is_bad_data(isc_info.oen)   ||
                  is_bad_data(isc_info.baser) ||
                  is_bad_data(isc_info.fbias)) continue;

               //
               // After reading the first ISC line, setup the isc_info
               // object to store the data.  Also, store the number
               // of scales and make sure that it doesn't change.
               //
               if(isc_aggr.n_scale == 0) {

                  n_scale = isc_info.n_scale;

                  // Allocate room to store results for each scale
                  isc_aggr.allocate_n_scale(n_scale);
                  isc_aggr.zero_out();

                  //
                  // Initialize tile_dim, tile_xll, and tile_yll.
                  // If they stay the same over all the lines, write them out.
                  // Otherwise, write out bad data.
                  //
                  isc_aggr.tile_dim = isc_info.tile_dim;
                  isc_aggr.tile_xll = isc_info.tile_xll;
                  isc_aggr.tile_yll = isc_info.tile_yll;

                  // Allocate room to store values for each scale
                  total_na = new NumArray [n_scale + 2];
                  mse_na   = new NumArray [n_scale + 2];
                  fen_na   = new NumArray [n_scale + 2];
                  oen_na   = new NumArray [n_scale + 2];
                  baser_na = new NumArray [n_scale + 2];
                  fbias_na = new NumArray [n_scale + 2];
               }
               else {

                  if(isc_aggr.n_scale != isc_info.n_scale) {
                     mlog << Error << "\naggr_isc_lines() -> "
                          << "the number of scales must remain constant "
                          << "when aggregating ISC lines.  Use the "
                          << "\"-column_min NSCALE n\" and "
                          << "\"-column_max NSCALE n\" options to "
                          << "filter out only those lines you'd like "
                          << "to aggregate!\n"
                          << "ERROR occurred on STAT line:\n" << line << "\n\n";
                     throw(1);
                  }

                  //
                  // Check to see if tile_dim, tile_xll, or tile_yll has changed.
                  // If so, write out bad data.
                  //
                  if(isc_aggr.tile_dim != bad_data_int &&
                     isc_aggr.tile_dim != isc_info.tile_dim) {
                     isc_aggr.tile_dim = bad_data_int;
                  }

                  if( (isc_aggr.tile_xll != bad_data_int &&
                       isc_aggr.tile_xll != isc_info.tile_xll) ||
                      (isc_aggr.tile_yll != bad_data_int &&
                       isc_aggr.tile_yll != isc_info.tile_yll) ) {
                     isc_aggr.tile_xll = bad_data_int;
                     isc_aggr.tile_yll = bad_data_int;
                  }
               }

               //
               // Store the data for this ISC line
               //
               total_na[iscale].add(isc_info.total);
               mse_na[iscale].add(isc_info.mse);
               fen_na[iscale].add(isc_info.fen);
               oen_na[iscale].add(isc_info.oen);
               baser_na[iscale].add(isc_info.baser);
               fbias_na[iscale].add(isc_info.fbias);

               break;

            default:
               mlog << Error << "\naggr_isc_lines() -> "
                    << "should only encounter ISC line types!\n"
                    << "ERROR occurred on STAT line:\n" << line << "\n\n";
               throw(1);
         } // end switch

         n_out++;
      }
   } // end while

   //
   // Return if no lines were read
   //
   if(n_out == 0) return;

   //
   // Get the sum of the totals, compute the weight, and sum the
   // weighted scores
   //
   for(i=0; i<n_scale+2; i++) {

      // Total number of points for this scale
      total = total_na[i].sum();

      // Initialize
      baser_fbias_sum = 0.0;

      // Loop through all scores for this scale
      for(k=0; k<total_na[i].n_elements(); k++) {

         // Compute the weight for each score to be aggregated
         // based on the number of points it represents
         w = total_na[i][k]/total;

         // Sum scores for the binary fields
         if(i == 0) {
            isc_aggr.mse    += w*mse_na[0][k];
            isc_aggr.fen    += w*fen_na[0][k];
            isc_aggr.oen    += w*oen_na[0][k];
            isc_aggr.baser  += w*baser_na[0][k];
            baser_fbias_sum += w*baser_na[0][k]*fbias_na[0][k];
         }
         // Weighted sum of scores for each scale
         else {
            isc_aggr.mse_scale[i-1] += w*mse_na[i][k];
            isc_aggr.fen_scale[i-1] += w*fen_na[i][k];
            isc_aggr.oen_scale[i-1] += w*oen_na[i][k];
         }
      }

      //
      // Compute the aggregated scores for the binary fields
      //
      if(i == 0) {

         // Total
         isc_aggr.total = nint(total_na[0].sum());

         // Aggregated FBIAS
         isc_aggr.fbias = baser_fbias_sum/isc_aggr.baser;

         // Compute the aggregated ISC score.  For the binary fields
         // do not divide by the number of scales.
         den = (isc_aggr.fbias*isc_aggr.baser*(1.0 - isc_aggr.baser) +
                isc_aggr.baser*(1.0 - isc_aggr.fbias*isc_aggr.baser));

         if(is_bad_data(isc_aggr.fbias) ||
            is_bad_data(isc_aggr.baser) ||
            is_eq(den, 0.0)) {
            isc_aggr.isc = bad_data_double;
         }
         else {
            isc_aggr.isc = 1.0 - isc_aggr.mse/den;
         }
      }
      //
      // Compute the aggregated scores for each scale
      //
      else {

         // Compute the aggregated ISC score.  For each scale, divide
         // by the number of scales.
         den = (isc_aggr.fbias*isc_aggr.baser*(1.0 - isc_aggr.baser) +
                isc_aggr.baser*(1.0 - isc_aggr.fbias*isc_aggr.baser))
               /(isc_aggr.n_scale+1);

         if(is_bad_data(isc_aggr.fbias) ||
            is_bad_data(isc_aggr.baser) ||
            is_eq(den, 0.0)) {
            isc_aggr.isc_scale[i-1] = bad_data_double;
         }
         else {
            isc_aggr.isc_scale[i-1] = 1.0 - isc_aggr.mse_scale[i-1]/den;
         }
      }
   } // end for i

   //
   // Deallocate memory
   //
   if(total_na) { delete [] total_na; total_na = (NumArray *) 0; }
   if(mse_na  ) { delete [] mse_na;   mse_na   = (NumArray *) 0; }
   if(fen_na  ) { delete [] fen_na;   fen_na   = (NumArray *) 0; }
   if(oen_na  ) { delete [] oen_na;   oen_na   = (NumArray *) 0; }
   if(baser_na) { delete [] baser_na; baser_na = (NumArray *) 0; }
   if(fbias_na) { delete [] fbias_na; fbias_na = (NumArray *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_rhist_lines(const char *jobstring, LineDataFile &f,
                      STATAnalysisJob &j, PairDataEnsemble &ens_pd,
                      int &n_in, int &n_out) {
   STATLine line;
   RHISTData r_data;
   double crps_num, crps_den, ign_num, ign_den;
   int i;

   //
   // Initialize
   //
   ens_pd.clear();
   crps_num = crps_den = ign_num = ign_den = 0.0;

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         if(line.type() != stat_rhist) {
            mlog << Error << "\naggr_rhist_lines() -> "
                 << "should only encounter ranked histogram "
                 << "(RHIST) line types!\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current RHIST line
         //
         parse_rhist_line(line, r_data);

         //
         // Check for N_RANK remaining constant
         //
         if(ens_pd.rhist_na.n_elements() > 0 &&
            ens_pd.rhist_na.n_elements() != r_data.n_rank) {
            mlog << Error << "\naggr_rhist_lines() -> "
                 << "the \"N_RANK\" column must remain constant ("
                 << ens_pd.rhist_na.n_elements() << " != " << r_data.n_rank
                 << ").  Try setting \"-column_eq N_RANK n\".\n\n";
            throw(1);
         }

         //
         // Aggregate the ranked histgram counts
         //
         if(ens_pd.rhist_na.n_elements() == 0) {
            ens_pd.rhist_na = r_data.rank_na;
         }
         else {
            for(i=0; i<ens_pd.rhist_na.n_elements(); i++) {
               ens_pd.rhist_na.set(i, ens_pd.rhist_na[i] + r_data.rank_na[i]);
            }
         }

         //
         // Store running sums for CRPS
         //
         if(!is_bad_data(r_data.crps)) {
            crps_num += r_data.total * r_data.crps;
            crps_den += r_data.total;
         }

         //
         // Store running sums for IGN
         //
         if(!is_bad_data(r_data.ign)) {
            ign_num += r_data.total * r_data.ign;
            ign_den += r_data.total;
         }

         n_out++;
      }
   } // end while

   // Compute weighted-mean for CRPS
   if(crps_den > 0) ens_pd.crps_na.add(crps_num/crps_den);
   else             ens_pd.crps_na.add(bad_data_double);

   // Compute weighted-mean for IGN
   if(ign_den > 0) ens_pd.ign_na.add(ign_num/ign_den);
   else            ens_pd.ign_na.add(bad_data_double);

   return;
}

////////////////////////////////////////////////////////////////////////

void aggr_orank_lines(const char *jobstring, LineDataFile &f,
                      STATAnalysisJob &j, PairDataEnsemble &ens_pd,
                      int &n_in, int &n_out) {
   STATLine line;
   ORANKData o_data;
   int i;
   double crps, ign, pit;

   //
   // Initialize
   //
   ens_pd.clear();

   //
   // Process the STAT lines
   //
   while(f >> line) {

      n_in++;

      if(j.is_keeper(line)) {

         //
         // Write line to dump file
         //
         if(j.dr_out) *(j.dr_out) << line;

         if(line.type() != stat_orank) {
            mlog << Error << "\naggr_orank_lines() -> "
                 << "should only encounter observation rank "
                 << "(ORANK) line types!\n"
                 << "ERROR occurred on STAT line:\n" << line << "\n\n";
            throw(1);
         }

         //
         // Parse the current RHIST line
         //
         parse_orank_line(line, o_data);

         //
         // Skip missing data
         //
         if(is_bad_data(o_data.rank)) continue;

         //
         // Check for N_ENS remaining constant
         //
         if(ens_pd.rhist_na.n_elements() > 0 &&
            ens_pd.rhist_na.n_elements() != o_data.n_ens+1) {
            mlog << Error << "\naggr_orank_lines() -> "
                 << "the \"N_ENS\" column must remain constant.  "
                 << "Try setting \"-column_eq N_ENS n\".\n\n";
            throw(1);
         }

         //
         // Initialize the ranked histogram counts
         //
         if(ens_pd.rhist_na.n_elements() == 0) {
            for(i=0; i<o_data.n_ens+1; i++) ens_pd.rhist_na.add(0);
         }

         //
         // Aggregate the ranks
         //
         i = o_data.rank - 1;
         ens_pd.rhist_na.set(i, ens_pd.rhist_na[i] + 1);

         //
         // Store the CRPS and IGN values
         //
         compute_crps_ign_pit(o_data.obs, o_data.ens_na, crps, ign, pit);
         ens_pd.crps_na.add(crps);
         ens_pd.ign_na.add(ign);
         ens_pd.pit_na.add(pit);

         n_out++;
      }
   } // end while

   return;
}

////////////////////////////////////////////////////////////////////////

double compute_vif(NumArray &na) {
   double corr, vif;

   // Compute the lag1 autocorrelation
   corr = stats_lag1_autocorrelation(na);

   // Compute the variance inflation factor
   vif = 1 + 2.0*fabs(corr) - 2.0*fabs(corr)/na.n_elements();

   return(vif);
}

////////////////////////////////////////////////////////////////////////

bool is_precip_var_name(const ConcatString &s) {
  
   bool match = has_prefix(pinterp_precipitation_names,
                           n_pinterp_precipitation_names, s) ||
                has_prefix(grib_precipitation_abbr,
                           n_grib_precipitation_abbr, s);

   return(match);
}

////////////////////////////////////////////////////////////////////////
