// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "pair_data_ensemble.h"

#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_util.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class PairDataEnsemble
//
////////////////////////////////////////////////////////////////////////

PairDataEnsemble::PairDataEnsemble() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairDataEnsemble::~PairDataEnsemble() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PairDataEnsemble::PairDataEnsemble(const PairDataEnsemble &pd) {

   init_from_scratch();

   assign(pd);
}

////////////////////////////////////////////////////////////////////////

PairDataEnsemble & PairDataEnsemble::operator=(const PairDataEnsemble &pd) {

   if(this == &pd) return(*this);

   assign(pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::init_from_scratch() {

   e_na   = (NumArray *) 0;
   n_pair = 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::clear() {
   int i;

   PairBase::clear();

   for(i=0; i<n_pair; i++) e_na[i].clear();
   if(e_na) { delete [] e_na; e_na = (NumArray *) 0; }

   v_na.clear();
   r_na.clear();
   crps_na.clear();
   ign_na.clear();
   pit_na.clear();
   rhist_na.clear();

   n_pair = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::assign(const PairDataEnsemble &pd) {
   int i;

   clear();

   set_mask_name(pd.mask_name);
   set_mask_dp_ptr(pd.mask_dp_ptr);
   set_msg_typ(pd.msg_typ);

   set_interp_mthd(pd.interp_mthd);
   set_interp_dpth(pd.interp_dpth);

   sid_sa   = pd.sid_sa;
   lat_na   = pd.lat_na;
   lon_na   = pd.lon_na;
   x_na     = pd.x_na;
   y_na     = pd.y_na;
   vld_ta   = pd.vld_ta;
   lvl_na   = pd.lvl_na;
   elv_na   = pd.elv_na;
   n_pair   = pd.n_pair;
   o_na     = pd.o_na;
   v_na     = pd.v_na;
   r_na     = pd.r_na;
   crps_na  = pd.crps_na;
   ign_na   = pd.ign_na;
   pit_na   = pd.pit_na;
   rhist_na = pd.rhist_na;

   n_obs  = pd.n_obs;
   n_pair = pd.n_pair;

   set_size();

   for(i=0; i<n_pair; i++) e_na[i] = pd.e_na[i];

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::add_ens(int i, double v) {

   e_na[i].add(v);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::set_size() {

   n_pair = n_obs;

   // Allocate a NumArray to store ensemble values for each observation
   e_na = new NumArray [n_pair];

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_rank(int n_vld_ens, const gsl_rng *rng_ptr) {
   int i, j, k, n_vld, n_bel, n_tie;
   NumArray src_na, dest_na;

   // Compute the rank for each observation
   for(i=0; i<o_na.n_elements(); i++) {

      // Compute the number of ensemble values less than the observation
      for(j=0, n_vld=0, n_bel=0, n_tie=0; j<e_na[i].n_elements(); j++) {

         // Skip bad data
         if(!is_bad_data(e_na[i][j])) {

            // Increment the valid count
            n_vld++;

            // Keep track of the number of ties and the number below
            if(is_eq(e_na[i][j], o_na[i])) n_tie++;
            else if(e_na[i][j] < o_na[i])  n_bel++;
         }
      } // end for j

      // Store the number of valid ensemble values
      v_na.add(n_vld);

      // Store the observation rank only when the number of valid
      // values matches the number of valid ensembles
      if(n_vld == n_vld_ens) {

         // With no ties, the rank is the number below plus 1
         if(n_tie == 0) {
            r_na.add(n_bel+1);
         }
         // With ties present, randomly assign the rank in:
         //    [n_bel+1, n_bel+n_tie+1]
         else {

            // Initialize
            dest_na.clear();
            src_na.clear();
            for(k=n_bel+1; k<=n_bel+n_tie+1; k++) src_na.add(k);

            // Randomly choose one of the ranks
            ran_choose(rng_ptr, src_na, dest_na, 1);

            // Store the rank
            r_na.add(nint(dest_na[0]));
         }

      }
      // Can't compute the rank when there is data missing
      else {
         r_na.add(bad_data_int);
      }

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_rhist(int n_vld_ens) {
   int i, rank;

   // Clear the ranked histogram
   rhist_na.clear();

   // Initialize the histogram counts to 0
   for(i=0; i<=n_vld_ens; i++) rhist_na.add(0);

   // The compute_rank() routine should have already been called.
   // Loop through the ranks and populate the histogram.
   for(i=0; i<r_na.n_elements(); i++) {

      // Get the current rank
      rank = r_na[i];

      // Increment the histogram counts
      if(!is_bad_data(rank)) rhist_na.set(rank-1, rhist_na[rank-1]+1);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataEnsemble::compute_stats(int n_vld_ens) {
   int i;
   double crps, ign, pit;

   // Clear the CRPS array
   crps_na.clear();

   // Loop through the pairs and compute CRPS for each
   for(i=0; i<n_pair; i++) {

      // Don't compute if any of the ensemble members are missing
      if(nint(v_na[i]) != n_vld_ens) {
         crps_na.add(bad_data_double);
         ign_na.add(bad_data_double);
         pit_na.add(bad_data_double);
         continue;
      }

      // Compute the stats
      compute_crps_ign_pit(o_na[i], e_na[i], crps, ign, pit);

      // Store the stats
      crps_na.add(crps);
      ign_na.add(ign);
      pit_na.add(pit);

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class VxPairDataEnsemble
//
////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble::VxPairDataEnsemble() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble::~VxPairDataEnsemble() {
   clear();
}

////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble::VxPairDataEnsemble(const VxPairDataEnsemble &vx_pd) {

   init_from_scratch();

   assign(vx_pd);
}

////////////////////////////////////////////////////////////////////////

VxPairDataEnsemble & VxPairDataEnsemble::operator=(const VxPairDataEnsemble &vx_pd) {

   if(this == &vx_pd) return(*this);

   assign(vx_pd);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::init_from_scratch() {

   fcst_info    = (VarInfo *) 0;
   obs_info     = (VarInfoGrib *) 0;
   pd           = (PairDataEnsemble ***) 0;

   n_msg_typ    = 0;
   n_mask       = 0;
   n_interp     = 0;
   
   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::clear() {
   int i, j, k;

   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *)     0; }
   if(obs_info)  { delete obs_info;  obs_info  = (VarInfoGrib *) 0; }

   interp_thresh = 0;

   fcst_dpa.clear();
   
   fcst_ut       = (unixtime) 0;
   beg_ut        = (unixtime) 0;
   end_ut        = (unixtime) 0;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].clear();
         }
      }
   }

   n_msg_typ     = 0;
   n_mask        = 0;
   n_interp      = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::assign(const VxPairDataEnsemble &vx_pd) {
   int i, j, k;

   clear();

   set_fcst_info(vx_pd.fcst_info);
   set_obs_info(vx_pd.obs_info);

   fcst_ut = vx_pd.fcst_ut;
   beg_ut  = vx_pd.beg_ut;
   end_ut  = vx_pd.end_ut;

   interp_thresh = vx_pd.interp_thresh;

   fcst_dpa = vx_pd.fcst_dpa;
   
   set_pd_size(vx_pd.n_msg_typ, vx_pd.n_mask, vx_pd.n_interp);

   for(i=0; i<vx_pd.n_msg_typ; i++) {
      for(j=0; j<vx_pd.n_mask; j++) {
         for(k=0; k<vx_pd.n_interp; k++) {

            pd[i][j][k] = vx_pd.pd[i][j][k];
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_fcst_info(VarInfo *info) {
   VarInfoFactory f;

   // Deallocate, if necessary
   if(fcst_info) { delete fcst_info; fcst_info = (VarInfo *) 0; }

   // Perform a deep copy
   fcst_info = f.new_var_info(info->file_type());
   *fcst_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_obs_info(VarInfoGrib *info) {

   // Deallocate, if necessary
   if(obs_info) { delete obs_info; obs_info = (VarInfoGrib *) 0; }

   // Perform a deep copy
   obs_info = new VarInfoGrib;
   *obs_info = *info;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_interp_thresh(double t) {

   interp_thresh = t;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_fcst_dpa(const DataPlaneArray &dpa) {

   fcst_dpa = dpa;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_fcst_ut(const unixtime ut) {

   fcst_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_beg_ut(const unixtime ut) {

   beg_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_end_ut(const unixtime ut) {

   end_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_pd_size(int types, int masks, int interps) {
   int i, j;

   // Store the dimensions for the PairData array
   n_msg_typ = types;
   n_mask    = masks;
   n_interp  = interps;

   // Allocate space for the PairData array
   pd = new PairDataEnsemble ** [n_msg_typ];

   for(i=0; i<n_msg_typ; i++) {
      pd[i] = new PairDataEnsemble * [n_mask];

      for(j=0; j<n_mask; j++) {
         pd[i][j] = new PairDataEnsemble [n_interp];
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_msg_typ(int i_msg_typ, const char *name) {
   int i, j;

   for(i=0; i<n_mask; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i_msg_typ][i][j].set_msg_typ(name);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_mask_dp(int i_mask, const char *name,
                             DataPlane *dp_ptr) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_interp; j++) {
         pd[i][i_mask][j].set_mask_name(name);
         pd[i][i_mask][j].set_mask_dp_ptr(dp_ptr);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_interp(int i_interp,
                                    const char *interp_mthd_str,
                                    int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(interp_mthd_str);
         pd[i][j][i_interp].set_interp_dpth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_interp(int i_interp, InterpMthd mthd,
                                    int wdth) {
   int i, j;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         pd[i][j][i_interp].set_interp_mthd(mthd);
         pd[i][j][i_interp].set_interp_dpth(wdth);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::set_ens_size() {
   int i, j, k;

   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            pd[i][j][k].set_size();
         }
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::add_obs(float *hdr_arr, const char *hdr_typ_str,
                                 const char  *hdr_sid_str, unixtime hdr_ut,
                                 float *obs_arr, Grid &gr) {
   int i, j, k, x, y;
   double hdr_lat, hdr_lon;
   double obs_x, obs_y, obs_lvl, obs_hgt;
   double obs_v;

   // Check whether the GRIB code for the observation matches
   // the specified code
   if(obs_info->code() != nint(obs_arr[1])) return;

   // Check whether the observation time falls within the valid time
   // window
   if(hdr_ut < beg_ut || hdr_ut > end_ut) return;

   hdr_lat = hdr_arr[0];
   hdr_lon = hdr_arr[1];

   obs_lvl = obs_arr[2];
   obs_hgt = obs_arr[3];
   obs_v   = obs_arr[4];

   // Check whether the observation value contains valid data
   if(is_bad_data(obs_v)) return;

   // Convert the lat/lon value to x/y
   gr.latlon_to_xy(hdr_lat, -1.0*hdr_lon, obs_x, obs_y);
   x = nint(obs_x);
   y = nint(obs_y);

   // Check if the observation's lat/lon is on the grid
   if(x < 0 || x >= gr.nx() ||
      y < 0 || y >= gr.ny()) return;

   // For pressure levels, check if the observation pressure level
   // falls in the requsted range.
   if(obs_info->level().type() == LevelType_Pres) {

      if(obs_lvl < obs_info->level().lower() ||
         obs_lvl > obs_info->level().upper()) return;
   }
   // For accumulations, check if the observation accumulation interval
   // matches the requested interval.
   else if(obs_info->level().type() == LevelType_Accum) {

      if(obs_lvl < obs_info->level().lower() ||
         obs_lvl > obs_info->level().upper()) return;
   }
   // For vertical levels, check for a surface message type or if the
   // observation height falls within the requested range.
   else if(obs_info->level().type() == LevelType_Vert) {

      if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL &&
         (obs_hgt < obs_info->level().lower() ||
          obs_hgt > obs_info->level().upper())) return;
   }
   // For all other level types (RecNumber, NoLevel), check
   // if the observation height falls within the requested range.
   else {
      if(obs_hgt < obs_info->level().lower() ||
         obs_hgt > obs_info->level().upper()) return;
   }

   // Look through all of the PairData objects to see if the observation
   // should be added.

   // Check the message types
   for(i=0; i<n_msg_typ; i++) {

      //
      // Check for a matching PrepBufr message type
      //

      // Handle ANYAIR
      if(strcmp(anyair_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anyair_msg_typ_str, hdr_typ_str) == NULL ) continue;
      }

      // Handle ANYSFC
      else if(strcmp(anysfc_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(anysfc_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle ONLYSF
      else if(strcmp(onlysf_str, pd[i][0][0].msg_typ) == 0) {
         if(strstr(onlysf_msg_typ_str, hdr_typ_str) == NULL) continue;
      }

      // Handle all other message types
      else {
         if(strcmp(hdr_typ_str, pd[i][0][0].msg_typ) != 0) continue;
      }

      // Check the masking areas and points
      for(j=0; j<n_mask; j++) {

         // Check for the obs falling within the masking region
         if(pd[i][j][0].mask_dp_ptr != (DataPlane *) 0) {
            if(!pd[i][j][0].mask_dp_ptr->s_is_on(x, y)) continue;
         }
         // Otherwise, check for the obs Station ID matching the
         // masking SID
         else {
            if(strcmp(hdr_sid_str, pd[i][j][0].mask_name) != 0)
               continue;
         }

         // Add the observation for each interpolation method
         for(k=0; k<n_interp; k++) {

            // Add the observation value
            pd[i][j][k].add_obs(hdr_sid_str, hdr_lat, hdr_lon,
                                obs_x, obs_y, hdr_ut, obs_lvl,
                                obs_hgt, obs_v);
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::add_ens() {
   int i, j, k, l;
   int fcst_lvl_below, fcst_lvl_above;
   double fcst_v;

   // Loop through all the PairDataEnsemble objects and interpolate
   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {

            // Process each of the observations
            for(l=0; l<pd[i][j][k].n_pair; l++) { 

               // For a single forecast field
               if(fcst_dpa.n_planes() == 1) {
                  fcst_lvl_below = 0;
                  fcst_lvl_above = 0;
               }
               // For multiple forecast fields, find the levels above and below
               // the observation point.
               else {

                  // Interpolate using the pressure value
                  if(fcst_info->level().type() == LevelType_Pres) {
                     find_vert_lvl(pd[i][j][k].lvl_na[l], fcst_lvl_below, fcst_lvl_above);
                  }
                  // Interpolate using the height value
                  else {
                     find_vert_lvl(pd[i][j][k].elv_na[l], fcst_lvl_below, fcst_lvl_above);
                 }
               }

               // Compute the interpolated ensemble value
               fcst_v = compute_interp(pd[i][j][k].x_na[l], pd[i][j][k].y_na[l], k,
                                       pd[i][j][k].lvl_na[l], fcst_lvl_below, fcst_lvl_above);

               // Add the ensemble value, even if it's bad data
               pd[i][j][k].add_ens(l, fcst_v);

            }
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::add_miss() {
   int i, j, k, l;

   // Loop through all the PairDataEnsemble objects
   for(i=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {
            for(l=0; l<pd[i][j][k].n_pair; l++) { 

               // Add bad data as a placeholder for missing file
               pd[i][j][k].add_ens(l, bad_data_double);

            } // end for l
         } // end for k
      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void VxPairDataEnsemble::find_vert_lvl(double obs_lvl,
                                  int &i_below, int &i_above) {
   int i;
   double dist, dist_below, dist_above;

   if(fcst_dpa.n_planes() == 0) {
      i_below = i_above = bad_data_int;
      return;
   }

   // Find the closest pressure levels above and below the observation
   dist_below = dist_above = 1.0e30;
   for(i=0; i<fcst_dpa.n_planes(); i++) {

      dist = obs_lvl - fcst_dpa.lower(i);

      // Check for the closest level below.
      // Levels below contain higher values of pressure.
      if(dist <= 0 && abs((long double) dist) < dist_below) {
         dist_below = abs((long double) dist);
         i_below = i;
      }

      // Check for the closest level above.
      // Levels above contain lower values of pressure.
      if(dist >= 0 && abs((long double) dist) < dist_above) {
         dist_above = abs((long double) dist);
         i_above = i;
      }
   }

   // Check if the observation is above the forecast range
   if(is_eq(dist_below, 1.0e30) && !is_eq(dist_above, 1.0e30)) {

      // Set the index below to the index above and perform
      // no vertical interpolation
      i_below = i_above;
   }
   // Check if the observation is below the forecast range
   else if(!is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {

      // Set the index above to the index below and perform
      // no vertical interpolation
      i_above = i_below;
   }
   // Check if an error occurred
   else if(is_eq(dist_below, 1.0e30) && is_eq(dist_above, 1.0e30)) {
      mlog << Error << "\nVxPairDataEnsemble::find_vert_lvl() -> "
           << "could not find a level above and/or below the "
           << "observation level of " << obs_lvl << ".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

int VxPairDataEnsemble::get_n_pair() {
   int n, i, j, k;

   for(i=0, n=0; i<n_msg_typ; i++) {
      for(j=0; j<n_mask; j++) {
         for(k=0; k<n_interp; k++) {

            n += pd[i][j][k].n_pair;
         }
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////

double VxPairDataEnsemble::compute_interp(double obs_x, double obs_y,
                                          int i_interp, double to_lvl,
                                          int i_below, int i_above) {
   double v, v_below, v_above, t;

   // Check for no data
   if(fcst_dpa.n_planes() == 0) return(bad_data_double);

   v_below = compute_horz_interp(fcst_dpa[i_below], obs_x, obs_y,
                                 pd[0][0][i_interp].interp_mthd,
                                 pd[0][0][i_interp].interp_dpth,
                                 interp_thresh);

   if(i_below == i_above) {
      v = v_below;
   }
   else {
      v_above = compute_horz_interp(fcst_dpa[i_above], obs_x, obs_y,
                                    pd[0][0][i_interp].interp_mthd,
                                    pd[0][0][i_interp].interp_dpth,
                                    interp_thresh);

      // If verifying specific humidity, do vertical interpolation in
      // the natural log of q
      if(fcst_info->is_specific_humidity() &&
          obs_info->is_specific_humidity()) {
         t = compute_vert_pinterp(log(v_below), fcst_dpa.lower(i_below),
                                  log(v_above), fcst_dpa.lower(i_above),
                                  to_lvl);
         v = exp(t);
      }
      // Vertically interpolate to the observation pressure level
      else if(fcst_info->level().type() == LevelType_Pres) {
         v = compute_vert_pinterp(v_below, fcst_dpa.lower(i_below),
                                  v_above, fcst_dpa.lower(i_above),
                                  to_lvl);
      }
      // Vertically interpolate to the observation height
      else {
         v = compute_vert_zinterp(v_below, fcst_dpa.lower(i_below),
                                  v_above, fcst_dpa.lower(i_above),
                                  to_lvl);
      }
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////
//
// Utility function for computing the continuous ranked probability
// score (CRPS), the ignorance score (IGN), and the probability
// itegral transform (PIT)
//
////////////////////////////////////////////////////////////////////////

void compute_crps_ign_pit(double obs, const NumArray &ens_na,
                          double &crps, double &ign, double &pit) {
   double m, s, z;

   // Mean and standard deviation of the ensemble values
   ens_na.compute_mean_stdev(m, s);

   // Check for divide by zero
   if(is_bad_data(m) || is_bad_data(s) || is_eq(s, 0.0)) {
      crps = bad_data_double;
      ign  = bad_data_double;
      pit  = bad_data_double;
   }
   else {

      z = (obs - m)/s;

      // Compute CRPS
      crps = s*(z*(2.0*znorm(z) - 1) + 2.0*dnorm(z) - 1.0/sqrt(pi));

      // Compute IGN
      ign = 0.5*log(2.0*pi*s*s) + (obs - m)*(obs - m)/(2.0*s*s);

      // Compute PIT
      pit = normal_cdf(obs, m, s);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
