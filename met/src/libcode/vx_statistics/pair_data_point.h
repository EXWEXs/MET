// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __PAIR_DATA_POINT_H__
#define  __PAIR_DATA_POINT_H__

////////////////////////////////////////////////////////////////////////

#include "pair_base.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"

////////////////////////////////////////////////////////////////////////
//
// Class to store matched pair data:
//    forecast, observation, and climatology values
//
////////////////////////////////////////////////////////////////////////

class PairDataPoint : public PairBase {

   private:

      void init_from_scratch();
      void assign(const PairDataPoint &);

   public:

      PairDataPoint();
      ~PairDataPoint();
      PairDataPoint(const PairDataPoint &);
      PairDataPoint & operator=(const PairDataPoint &);

      //////////////////////////////////////////////////////////////////

      // Forecast and climatological values
      NumArray f_na;    // Forecast [n_pair]
      NumArray c_na;    // Climatology [n_pair]
      int      n_pair;

      //////////////////////////////////////////////////////////////////

      void clear();

      void add_pair(const char *, double, double, double, double,
                    unixtime, double, double, double, double, double);

      void set_pair(int, const char *, double, double, double, double,
                    unixtime, double, double, double, double, double);
};

////////////////////////////////////////////////////////////////////////
//
// Class to store a variety of PairDataPoint objects for each
// verification task
//
////////////////////////////////////////////////////////////////////////

class VxPairDataPoint {

   private:

      void init_from_scratch();
      void assign(const VxPairDataPoint &);

   public:

      VxPairDataPoint();
      ~VxPairDataPoint();
      VxPairDataPoint(const VxPairDataPoint &);
      VxPairDataPoint & operator=(const VxPairDataPoint &);

      //////////////////////////////////////////////////////////////////
      //
      // Information about the fields to be compared
      //
      //////////////////////////////////////////////////////////////////

      VarInfo     *fcst_info;  // Forecast field, allocated by VarInfoFactory
      VarInfoGrib *obs_info;   // Observation field, allocated by VarInfoFactory

      double interp_thresh;    // Threshold between 0 and 1 used when
                               // interpolating the forecasts to the
                               // observation location.

      //////////////////////////////////////////////////////////////////
      //
      // Forecast and climatological fields falling between the
      // requested levels.  Store the fields in a data plane array.
      //
      //////////////////////////////////////////////////////////////////

      DataPlaneArray fcst_dpa;   // Forecast data plane array
      DataPlaneArray climo_dpa;  // Climatology data plane array

      //////////////////////////////////////////////////////////////////

      unixtime fcst_ut;          // Forecast valid time
      unixtime beg_ut;           // Beginning of valid time window
      unixtime end_ut;           // End of valid time window

      //////////////////////////////////////////////////////////////////

      int      n_msg_typ;        // Number of verifying message types

      int      n_mask;           // Total number of masking regions
                                 // of masking DataPlane fields or SIDs

      int      n_interp;         // Number of interpolation techniques

      //////////////////////////////////////////////////////////////////

      PairDataPoint ***pd;       // 3-Dim Array of PairDataPoint objects
                                 // as [n_msg_typ][n_mask][n_interp]

      //  Counts for observation rejection reason codes
      int n_try;                 // Number of observations processed
      int rej_gc;                // Reject based on GRIB code
      int rej_vld;               // Reject based on valid time
      int rej_obs;               // Reject observation bad data
      int rej_grd;               // Reject based on location
      int rej_lvl;               // Reject based on vertical level

      //  3-Dim Arrays for observation rejection reason codes
      int ***rej_typ;            // Reject based on message type
      int ***rej_mask;           // Reject based on masking region
      int ***rej_fcst;           // Reject forecast bad data

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_fcst_info(VarInfo *);
      void set_obs_info(VarInfoGrib *);
      void set_interp_thresh(double);

      void set_fcst_dpa(const DataPlaneArray &);
      void set_climo_dpa(const DataPlaneArray &);

      void set_fcst_ut(const unixtime);
      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      // Call set_pd_size before set_msg_typ, set_mask_dp, and set_interp
      void set_pd_size(int, int, int);

      void set_msg_typ(int, const char *);
      void set_mask_dp(int, const char *, DataPlane *);
      void set_interp(int, const char *, int);
      void set_interp(int, InterpMthd, int);

      void add_obs(float *, char *, char *, unixtime, float *, Grid &);

      void find_fcst_vert_lvl(double, int &, int &);
      void find_climo_vert_lvl(double, int &, int &);

      int  get_n_pair();

      // Member functions for incrementing the counts
      void inc_count(int ***&, int);
      void inc_count(int ***&, int, int);
      void inc_count(int ***&, int, int, int);

      double compute_fcst_interp(double, double, int, double, int, int);
      double compute_climo_interp(double, double, int, double, int, int);
};

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_DATA_POINT_H__

////////////////////////////////////////////////////////////////////////
