

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_3D_ATTRIBUTES_H__
#define  __MTD_3D_ATTRIBUTES_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_util.h"

#include "mtd_file.h"
#include "mtd_config_info.h"


////////////////////////////////////////////////////////////////////////


typedef MtdIntFile   Object;
typedef MtdFloatFile Raw;


////////////////////////////////////////////////////////////////////////


static const char hms_format     [] = "%02d%02d%02d";
static const char ymd_hms_format [] = "%04d%02d%02d_%02d%02d%02d";


////////////////////////////////////////////////////////////////////////


class SingleAtt3D {

      friend SingleAtt3D calc_3d_single_atts(const Object & obj, const Raw & raw, const char * model, int obj_number);

   public:

      void init_from_scratch();

      void assign(const SingleAtt3D &);


      int ObjectNumber;

      int Volume;

      bool IsFcst;
      bool IsSimple;

      double Xbar, Ybar, Tbar;

      double Centroid_Lat, Centroid_Lon;

      int Xmin, Xmax;
      int Ymin, Ymax;
      int Tmin, Tmax;

      double Complexity;

      double Xvelocity;
      double Yvelocity;

      double SpatialAxisAngle;

      double Ptile_10;
      double Ptile_25;
      double Ptile_50;
      double Ptile_75;
      double Ptile_90;

   public:

      SingleAtt3D();
     ~SingleAtt3D();
      SingleAtt3D(const SingleAtt3D &);
      SingleAtt3D & operator=(const SingleAtt3D &);

      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void set_object_number (int);

      void set_volume (int);

      void set_centroid(double _xbar, double _ybar, double _tbar);

      void set_centroid_latlon(double _lat, double _lon);

      void set_bounding_box (int _xmin, int _xmax, 
                             int _ymin, int _ymax, 
                             int _tmin, int _tmax);

      void set_complexity(double);

      void set_velocity(double, double);

      void set_spatial_axis(double);

      void set_fcst (bool = true);
      void set_obs  (bool = true);

      void set_simple  (bool = true);
      void set_cluster (bool = true);

         //
         //  get stuff
         //

      int object_number () const;

      bool is_fcst () const;
      bool is_obs  () const;

      bool is_simple  () const;
      bool is_cluster () const;

      int volume () const;

      void centroid ( double & xbar, double & ybar, double & tbar) const;

      double centroid_lat () const;
      double centroid_lon () const;

      double xbar () const;
      double ybar () const;
      double tbar () const;

      void bounding_box (int & xmin, int & xmax, 
                         int & ymin, int & ymax, 
                         int & tmin, int & tmax) const;

      int xmin() const;
      int xmax() const;

      int ymin() const;
      int ymax() const;

      int tmin() const;
      int tmax() const;

      int n_times() const;

      double complexity() const;

      void velocity(double &, double &) const;

      double speed() const;

      double xdot() const;
      double ydot() const;

      double spatial_axis() const;

      double ptile_10() const;
      double ptile_25() const;
      double ptile_50() const;
      double ptile_75() const;
      double ptile_90() const;

         //
         //  do stuff
         //

      void write_txt(AsciiTable &, const int row) const;

};


////////////////////////////////////////////////////////////////////////


inline int SingleAtt3D::object_number() const { return ( ObjectNumber ); }

inline bool SingleAtt3D::is_fcst () const { return (   IsFcst ); }
inline bool SingleAtt3D::is_obs  () const { return ( ! IsFcst ); }

inline bool SingleAtt3D::is_simple  () const { return (   IsSimple ); }
inline bool SingleAtt3D::is_cluster () const { return ( ! IsSimple ); }

inline void SingleAtt3D::set_object_number(int _n) { ObjectNumber = _n;  return; }

inline int SingleAtt3D::volume() const { return ( Volume ); }

inline void SingleAtt3D::set_volume(int _v) { Volume = _v;  return; }

inline double SingleAtt3D::complexity() const { return ( Complexity ); }

inline void SingleAtt3D::set_complexity(double _v) { Complexity = _v;  return; }


inline double SingleAtt3D::xbar() const { return ( Xbar ); }
inline double SingleAtt3D::ybar() const { return ( Ybar ); }
inline double SingleAtt3D::tbar() const { return ( Tbar ); }

inline double SingleAtt3D::centroid_lat() const { return ( Centroid_Lat ); }
inline double SingleAtt3D::centroid_lon() const { return ( Centroid_Lon ); }

inline int SingleAtt3D::xmin() const { return ( Xmin ); }
inline int SingleAtt3D::xmax() const { return ( Xmax ); }

inline int SingleAtt3D::ymin() const { return ( Ymin ); }
inline int SingleAtt3D::ymax() const { return ( Ymax ); }

inline int SingleAtt3D::tmin() const { return ( Tmin ); }
inline int SingleAtt3D::tmax() const { return ( Tmax ); }

inline double SingleAtt3D::xdot() const { return ( Xvelocity ); }
inline double SingleAtt3D::ydot() const { return ( Yvelocity ); }

inline double SingleAtt3D::spatial_axis() const { return ( SpatialAxisAngle ); }

inline double SingleAtt3D::ptile_10() const { return ( Ptile_10 ); }
inline double SingleAtt3D::ptile_25() const { return ( Ptile_25 ); }
inline double SingleAtt3D::ptile_50() const { return ( Ptile_50 ); }
inline double SingleAtt3D::ptile_75() const { return ( Ptile_75 ); }
inline double SingleAtt3D::ptile_90() const { return ( Ptile_90 ); }


////////////////////////////////////////////////////////////////////////


class PairAtt3D {

      friend PairAtt3D calc_3d_pair_atts(const Object & _fcst_obj, 
                                         const MtdFloatFile & _obs_obj, 
                                         const SingleAtt3D & _fa, 
                                         const SingleAtt3D & _oa);

   public:

      void init_from_scratch();

      void assign(const PairAtt3D &);

      int FcstObjectNumber;
      int ObsObjectNumber;

      int IntersectionVol;
      int UnionVol;

      double TimeCentroidDelta;
      double SpaceCentroidDist;

      double DirectionDifference;

      double SpeedDelta;

      double VolumeRatio;
      double AxisDiff;

         //  these need to be doubles rather than ints
         //   (for the InterestCalculator class).

      double StartTimeDelta;
      double EndTimeDelta;

      double TotalInterest;

   public:

      PairAtt3D();
     ~PairAtt3D();
      PairAtt3D(const PairAtt3D &);
      PairAtt3D & operator=(const PairAtt3D &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      // SingleAtt3D Fcst;
      // SingleAtt3D Obs;

         //
         //  set stuff
         //

      void set_fcst_obj_number (int);
      void set_obs_obj_number  (int);

      void set_intersection_volume (int);
      void set_union_volume        (int);

      void set_space_centroid_dist(double);
      void set_time_centroid_delta(double);

      void set_speed_delta    (double);
      void set_direction_diff (double);

      void set_volume_ratio (double);
      void set_axis_diff    (double);

      void set_start_time_delta (int);
      void set_end_time_delta   (int);

      void set_total_interest (double);

         //
         //  get stuff
         //

      int fcst_obj_number() const;
      int  obs_obj_number() const;

      int intersection_vol () const;
      int union_vol        () const;

      double  time_centroid_delta () const;
      double space_centroid_dist  () const;

      double direction_difference () const; 

      double speed_delta () const;

      double volume_ratio () const;
      double axis_angle_diff () const;

      int start_time_delta () const;
      int end_time_delta   () const;

      double total_interest () const;

         //
         //  do stuff
         //

      void write_txt(AsciiTable &, const int row) const;

};


////////////////////////////////////////////////////////////////////////


inline int PairAtt3D::fcst_obj_number() const { return ( FcstObjectNumber ); }
inline int  PairAtt3D::obs_obj_number() const { return ( ObsObjectNumber ); }

inline int PairAtt3D::intersection_vol () const { return ( IntersectionVol ); }
inline int PairAtt3D::union_vol        () const { return ( UnionVol ); }

inline double PairAtt3D::time_centroid_delta () const { return ( TimeCentroidDelta ); }
inline double PairAtt3D::space_centroid_dist () const { return ( SpaceCentroidDist ); }

inline double PairAtt3D::direction_difference () const { return ( DirectionDifference ); }
inline double PairAtt3D::speed_delta () const { return ( SpeedDelta ); }

inline double PairAtt3D::volume_ratio    () const { return ( VolumeRatio ); }
inline double PairAtt3D::axis_angle_diff () const { return ( AxisDiff ); }

inline int PairAtt3D::start_time_delta () const { return ( StartTimeDelta ); }
inline int PairAtt3D::end_time_delta   () const { return ( EndTimeDelta ); }

inline double PairAtt3D::total_interest () const { return ( TotalInterest ); }


////////////////////////////////////////////////////////////////////////


extern SingleAtt3D calc_3d_single_atts(const Object & obj, const Raw & raw, const char * model, int obj_number);   //  0 based 

extern PairAtt3D   calc_3d_pair_atts(const Object      & _fcst_obj, 
                                     const Object      & _obs_obj, 
                                     const SingleAtt3D & _fa, 
                                     const SingleAtt3D & _oa);

// extern double calc_total_interest(const PairAtt3D &, const MtdConfigInfo &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_3D_ATTRIBUTES_H__  */


////////////////////////////////////////////////////////////////////////



