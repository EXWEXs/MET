// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
#include <algorithm>

#include "pair_base.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class PairBase
//
////////////////////////////////////////////////////////////////////////

PairBase::PairBase() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PairBase::~PairBase() {
   clear();
}

////////////////////////////////////////////////////////////////////////

void PairBase::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::clear() {

   msg_typ.clear();
   mask_name.clear();

   mask_dp_ptr  = (DataPlane *)   0;  // Not allocated
   mask_sid_ptr = (StringArray *) 0;  // Not allocated

   interp_mthd = InterpMthd_None;
   interp_dpth = bad_data_int;

   sid_sa.clear();
   lat_na.clear();
   lon_na.clear();
   x_na.clear();
   y_na.clear();
   vld_ta.clear();
   lvl_na.clear();
   elv_na.clear();
   o_na.clear();
   o_qc_sa.clear();
   cmn_na.clear();
   csd_na.clear();
   wgt_na.clear();

   n_obs = 0;

   fcst_ut = 0;

   obs_select = OBS_SUMMARY_NONE;
   obs_perc_value = bad_data_int;

   map_val.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::extend(int n) {

   sid_sa.extend(n);
   lat_na.extend(n);
   lon_na.extend(n);
   x_na.extend(n);
   y_na.extend(n);
   vld_ta.extend(n);
   lvl_na.extend(n);
   elv_na.extend(n);
   o_na.extend(n);
   o_qc_sa.extend(n);
   cmn_na.extend(n);
   csd_na.extend(n);
   wgt_na.extend(n);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_name(const char *c) {

   mask_name = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_dp_ptr(DataPlane *dp_ptr) {

   mask_dp_ptr = dp_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_mask_sid_ptr(StringArray *sid_ptr) {

   mask_sid_ptr = sid_ptr;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_msg_typ(const char *c) {

   msg_typ = c;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_mthd(const char *str) {

   interp_mthd = string_to_interpmthd(str);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_mthd(InterpMthd m) {

   interp_mthd = m;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_interp_dpth(int n) {

   interp_dpth = n;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_fcst_ut(unixtime ut){

   fcst_ut = ut;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_check_unique(bool check){

   check_unique = check;

   return;
}

void PairBase::set_obs_summary(obs_summary_enum o) {

  obs_select = o;
  
  return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs_perc_value(int i) {

  obs_perc_value = i;
  
  return;
}

////////////////////////////////////////////////////////////////////////

int PairBase::has_obs_rec(const char *sid,
                          double lat, double lon,
                          double x, double y,
                          double lvl, double elv, int &i_obs) {
   int i, status = 0;

   //
   // Check for an existing record of this observation
   //
   for(i=0, i_obs=-1; i<n_obs; i++) {

      if(strcmp(sid_sa[i], sid) == 0 &&
         is_eq(lat_na[i], lat) &&
         is_eq(lon_na[i], lon) &&
         is_eq(lvl_na[i], lvl) &&
         is_eq(elv_na[i], elv)) {
         status = 1;
         i_obs = i;
         break;
      }
   } // end for

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool PairBase::add_obs(const char *sid,
                       double lat, double lon, double x, double y,
                       unixtime ut, double lvl, double elv,
                       double o, const char *qc,
                       double cmn, double csd, double wgt) {

   //  build a uniqueness test key
   string sng_key = str_format("%.3f:%.3f:%.2f:%.2f",
			      lat,         //  lat
			      lon,         //  lon
			      lvl,         //  level
			      elv).text(); //  elevation

   //  add a single value reporting string to the reporting map
   if( 3 <= mlog.verbosity_level() ){

      ob_val_t ob_val;
      ob_val.ut = ut;	
      ob_val.val = o;
      // if key exists, add new ob to vector, else add new pair
      map<string,station_values_t>::iterator it = map_val.find(sng_key);
      if(it != map_val.end()) {
	if(check_unique && (*it).second.ut == ut && (*it).second.obs[0].val == o)
	   return false;
        (*it).second.obs.push_back(ob_val);
      } else {
         station_values_t val;
	 val.sid = string(sid);
	 val.ut = fcst_ut;
         val.obs.push_back(ob_val);
         map_val.insert( pair<string,station_values_t>(sng_key, val) );
      }
   }
   
   sid_sa.add(sid);
   lat_na.add(lat);
   lon_na.add(lon);
   x_na.add(x);
   y_na.add(y);
   vld_ta.add(ut);
   lvl_na.add(lvl);
   elv_na.add(elv);
   o_na.add(o);
   o_qc_sa.add(qc);
   cmn_na.add(cmn);
   csd_na.add(csd);
   wgt_na.add(wgt);

   // Increment the number of pairs
   n_obs += 1;

   return true;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_single(string sng_key) {
   station_values_t svt = map_val[sng_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   ob_val_t out = (*it);
   int ut_diff = labs(svt.ut - (*it).ut);
   for(; it != svt.obs.end(); it++) {
      int new_diff = labs(svt.ut - (*it).ut);
      if( ut_diff > new_diff) {
	ut_diff = new_diff;
        out = (*it);
      }
   }
   
   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_min(string sng_key) {
   station_values_t svt = map_val[sng_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   ob_val_t out = (*it);
   for(; it != svt.obs.end(); it++) {
     if( out.val > (*it).val) {
       out = (*it);
      }
   }
   
   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_max(string sng_key) {
   station_values_t svt = map_val[sng_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   ob_val_t out = (*it);
   for(; it != svt.obs.end(); it++) {
     if( out.val < (*it).val) {
       out = (*it);
      }
   }
   
   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_uwmean(string sng_key) {
   double total = 0.0;
   int count = 0;
   station_values_t svt = map_val[sng_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   for(; it != svt.obs.end(); it++) {
     total += (*it).val;
     count++;
   }

   ob_val_t out;
   out.ut = svt.ut;
   out.val = total / double(count);
   return out;
}

////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_twmean(string sng_key) {
   double total = 0.0;
   double weight = 0.0;
   double total_weight = 0.0;
   station_values_t svt = map_val[sng_key];
   vector<ob_val_t>::iterator it = svt.obs.begin();
   for(; it != svt.obs.end(); it++) {
     weight = 1 / pow( labs( svt.ut - (*it).ut ),2);
     total_weight += weight;
     total += (*it).val * weight;
   }

   ob_val_t out;
   out.ut = svt.ut;
   out.val = total / total_weight;
   return out;
}


////////////////////////////////////////////////////////////////////////

ob_val_t PairBase::compute_percentile(string sng_key, int perc) {
   station_values_t svt = map_val[sng_key];
   vector<ob_val_t> obs = svt.obs;

   std::sort(obs.begin(), obs.end(), sort_obs);

   int index = ceil((perc/100.0) * (obs.size()-1));
   
   return obs[index];
}

////////////////////////////////////////////////////////////////////////

void PairBase::print_duplicate_report(){
  if(obs_select == OBS_SUMMARY_NONE) return;

  if( 3 > mlog.verbosity_level() || ! map_val.size() ) return;

  //  iterate over the keys in the unique station id map
  mlog << Debug(3) << "\nDuplicate point observations for -single setting:\n";
  for( map<string,station_values_t>::iterator it_single = map_val.begin();
       it_single != map_val.end(); ++it_single ){

    int num_val = 0;
    string key_single_val = (*it_single).first;

    //  parse the single key string
    char** mat = NULL;
    if( 5 != regex_apply("^([^:]+):([^:]+):([^:]+):([^:]+)$", 5, key_single_val.c_str(), mat) ){
      mlog << Error << "\nPairBase::print_duplicate_report() - regex_apply failed "
	   << "to parse '" << key_single_val.c_str() << "'\n\n";
      exit(1);
    }

    string msg_key = str_format("%s:%s:%s:%s",	 
				mat[1], mat[2], mat[3], mat[4]).text();
    regex_clean(mat);

    ob_val_t single_ob;

    switch(obs_select) {
    case OBS_SUMMARY_SINGLE:
      single_ob = compute_single(msg_key);
      break;
    case OBS_SUMMARY_MIN:
      single_ob = compute_min(msg_key);
      break;
    case OBS_SUMMARY_MAX:
      single_ob = compute_max(msg_key);
      break;
    case OBS_SUMMARY_UWMEAN:
      single_ob = compute_uwmean(msg_key);
      break;
    case OBS_SUMMARY_TWMEAN:
      single_ob = compute_twmean(msg_key);
      break;	      	      	      
    case OBS_SUMMARY_MEDIAN:
      single_ob = compute_percentile(msg_key, 50);
      break;
    case OBS_SUMMARY_PERC:
      single_ob = compute_percentile(msg_key, obs_perc_value);
      break;    	      	      
    }
	 
    int ut_diff = labs((*it_single).second.ut - single_ob.ut);
    string msg_val = str_format("%s (HHMMSS)", sec_to_hhmmss( ut_diff ).text()).text();
 
    string msg = "  " + msg_key + " - used point ob with valid time offset of " + msg_val;

    //  parse and print the point obs information for the current key

    vector<ob_val_t>::iterator o_it = (*it_single).second.obs.begin();

    for(; o_it != (*it_single).second.obs.end(); o_it++) {

      string msg_ob = str_format("[sid: %6s  vld: %s  ob_val: %8d]",
				 (*it_single).second.sid.c_str(),
				 unix_to_yyyymmdd_hhmmss( (*o_it).ut ).text(),
				 (*o_it).val).text();
	   
      msg += "\n    " + msg_ob;

    }

    if( 1 < num_val ) mlog << Debug(3) << msg.c_str() << "\n\n";

  }

  mlog << Debug(3) << "\n";
}

////////////////////////////////////////////////////////////////////////

void PairBase::add_obs(double x, double y, double o,
                       double cmn, double csd, double wgt) {

   sid_sa.add(na_str);
   lat_na.add(bad_data_double);
   lon_na.add(bad_data_double);
   x_na.add(x);
   y_na.add(y);
   vld_ta.add(bad_data_int);
   lvl_na.add(bad_data_double);
   elv_na.add(bad_data_double);
   o_na.add(o);
   o_qc_sa.add(na_str);
   cmn_na.add(cmn);
   csd_na.add(csd);
   wgt_na.add(wgt);

   // Increment the number of observations
   n_obs += 1;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, const char *sid,
                       double lat, double lon, double x, double y,
                       unixtime ut, double lvl, double elv,
                       double o, const char *qc,
                       double cmn, double csd, double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n"
          ;
      exit(1);
   }

   sid_sa.set(i_obs, sid);
   lat_na.set(i_obs, lat);
   lon_na.set(i_obs, lon);
   x_na.set(i_obs, x);
   y_na.set(i_obs, y);
   vld_ta.set(i_obs, ut);
   lvl_na.set(i_obs, lvl);
   elv_na.set(i_obs, elv);
   o_na.set(i_obs, o);
   o_qc_sa.set(i_obs, qc);
   cmn_na.set(i_obs, cmn);
   csd_na.set(i_obs, csd);
   wgt_na.set(i_obs, wgt);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairBase::set_obs(int i_obs, double x, double y,
                       double o, double cmn, double csd, double wgt) {

   if(i_obs < 0 || i_obs >= n_obs) {
      mlog << Error << "\nPairBase::set_obs() -> "
           << "range check error: " << i_obs << " not in (0, "
           << n_obs << ").\n\n"
          ;
      exit(1);
   }

   sid_sa.set(i_obs, na_str);
   lat_na.set(i_obs, bad_data_double);
   lon_na.set(i_obs, bad_data_double);
   x_na.set(i_obs, x);
   y_na.set(i_obs, y);
   vld_ta.set(i_obs, bad_data_int);
   lvl_na.set(i_obs, bad_data_double);
   elv_na.set(i_obs, bad_data_double);
   o_na.set(i_obs, o);
   o_qc_sa.set(i_obs, na_str);
   cmn_na.set(i_obs, cmn);
   csd_na.set(i_obs, csd);
   wgt_na.set(i_obs, wgt);

   return;
}

////////////////////////////////////////////////////////////////////////
