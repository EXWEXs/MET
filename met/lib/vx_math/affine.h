

////////////////////////////////////////////////////////////////////////


#ifndef  __VX_MATH_AFFINE_H__
#define  __VX_MATH_AFFINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


   //
   //  This header file is part of the Verification Group's
   //
   //    Math library code
   //


////////////////////////////////////////////////////////////////////////


enum ViewGravity {

   view_center_gravity,

   view_north_gravity,
   view_south_gravity,
   view_east_gravity,
   view_west_gravity,

   view_northwest_gravity,
   view_northeast_gravity,

   view_southwest_gravity,
   view_southeast_gravity,

   no_view_gravity

};


////////////////////////////////////////////////////////////////////////


class ViewBox {

   private:

      void init_from_scratch();

      void assign(const ViewBox &);

   public:

      ViewBox();
     ~ViewBox();
      ViewBox(const ViewBox &);
      ViewBox & operator=(const ViewBox &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      double x_ll;
      double y_ll;

      double width;
      double height;

      double aspect() const;

};


////////////////////////////////////////////////////////////////////////


class GeneralAffine {

   public:

      GeneralAffine();
      virtual ~GeneralAffine();

      virtual void clear() = 0;

      virtual void dump(ostream &, int depth = 0) const = 0;

      virtual double m11() const = 0;
      virtual double m12() const = 0;
      virtual double m21() const = 0;
      virtual double m22() const = 0;

      virtual double tx () const = 0;
      virtual double ty () const = 0;

      virtual double det() const = 0;

      virtual bool is_conformal() const = 0;


      virtual void set_pin(double, double, double, double) = 0;

      virtual void set_fixed_point(double u, double v) = 0;

      virtual void set_translation(double, double) = 0;

      virtual void move(double, double) = 0;

      virtual void invert() = 0;


      virtual void     forward (double  u, double  v, double &  x, double &  y) const = 0;
      virtual void     reverse (double  x, double  y, double &  u, double &  v) const = 0;

      virtual void der_forward (double du, double dv, double & dx, double & dy) const = 0;
      virtual void der_reverse (double dx, double dy, double & du, double & dv) const = 0;


      virtual GeneralAffine * copy() const = 0;

      virtual void     operator()(double  u, double  v, double &  x, double &  y) const = 0;

};


////////////////////////////////////////////////////////////////////////


class Affine : public GeneralAffine {

      friend Affine operator*(const Affine &, const Affine &);

   protected:

      void init_from_scratch();

      void assign(const Affine &);

      void calc_det();

      double Det;

      double M11;
      double M12;
      double M21;
      double M22;

      double TX;
      double TY;

   public:

      Affine();
      virtual ~Affine();
      Affine(const Affine &);
      Affine & operator=(const Affine &);

      void clear();  //  set to identity

      virtual void dump(ostream &, int depth = 0) const;



      void set_mb(double _m11, double _m12, double _m21, double _m22, double _b1, double _b2);

      void set_three_points(double u1, double v1, double u2, double v2, double u3, double v3, 
                            double x1, double y1, double x2, double y2, double x3, double y3);



         //
         //  from base class
         //

      double m11() const;
      double m12() const;
      double m21() const;
      double m22() const;

      double tx () const;
      double ty () const;

      double det() const;

      bool is_conformal() const;


      void set_pin(double, double, double, double);

      void set_fixed_point(double u, double v);

      void set_translation(double, double);

      void move(double, double);

      void invert();


      void     forward (double  u, double  v, double &  x, double &  y) const;
      void     reverse (double  x, double  y, double &  u, double &  v) const;

      void der_forward (double du, double dv, double & dx, double & dy) const;
      void der_reverse (double dx, double dy, double & du, double & dv) const;

      void     operator()(double  u, double  v, double &  x, double &  y) const;

      GeneralAffine * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double Affine::m11() const { return ( M11 ); }
inline double Affine::m12() const { return ( M12 ); }
inline double Affine::m21() const { return ( M21 ); }
inline double Affine::m22() const { return ( M22 ); }

inline double Affine::tx () const { return ( TX ); }
inline double Affine::ty () const { return ( TY ); }

inline double Affine::det() const { return ( Det ); }

inline void Affine::calc_det() { Det = M11*M22 - M21*M12;  return; }

inline void Affine::set_translation(double _tx, double _ty) { TX = _tx;  TY = _ty;  return; }

inline void Affine::set_fixed_point(double _u, double _v) { set_pin(_u, _v, _u, _v);  return; }


inline void Affine::operator()(double  _u, double  _v, double &  _x, double &  _y) const

{  forward(_u, _v, _x, _y);  return; }


////////////////////////////////////////////////////////////////////////


extern Affine operator*(const Affine &, const Affine &);


////////////////////////////////////////////////////////////////////////


   //
   //  conformal, orientation preserving, affine, two-dimensional
   //    coordinate transformation:
   //
   //    x = scale*(  u*cos(theta) + v*sin(theta) )  + tx;
   //
   //    y = scale*( -u*sin(theta) + v*cos(theta) )  + ty;
   //


////////////////////////////////////////////////////////////////////////


class ConformalAffine : public GeneralAffine {

      friend ConformalAffine operator*(const ConformalAffine &, const ConformalAffine &);

   private:

      void init_from_scratch();

      void assign(const ConformalAffine &);


      double CosAngle;
      double SinAngle;

      double Angle;

      double Scale;

      double TX;
      double TY;

   public:

      ConformalAffine();
      virtual ~ConformalAffine();
      ConformalAffine(const ConformalAffine &);
      ConformalAffine & operator=(const ConformalAffine &);

      void clear();

      void dump (ostream &, int depth = 0) const;


      void set_scale(double);
      void set_angle(double);
      void set_translation(double, double);


      void set(const ViewBox & image, const ViewBox & view);

      void set(const ViewBox & image, const ViewBox & view, const ViewGravity);


      Affine affine_equivalent() const;


      ConformalAffine inverse() const;


      double angle() const;

      double scale() const;


         //
         //  from base class
         //

      double m11() const;
      double m12() const;
      double m21() const;
      double m22() const;

      double tx () const;
      double ty () const;

      double det() const;

      bool is_conformal() const;


      void set_pin(double, double, double, double);

      void set_fixed_point(double u, double v);


      // void set_translation(double, double);   // this is already listed above

      void move(double, double);

      void invert();


      void     forward (double  u, double  v, double &  x, double &  y) const;
      void     reverse (double  x, double  y, double &  u, double &  v) const;

      void der_forward (double du, double dv, double & dx, double & dy) const;
      void der_reverse (double dx, double dy, double & du, double & dv) const;

      void     operator()(double  u, double  v, double &  x, double &  y) const;

      GeneralAffine * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double ConformalAffine::angle() const { return ( Angle ); }

inline double ConformalAffine::scale() const { return ( Scale ); }

inline bool   ConformalAffine::is_conformal() const { return ( true ); }


inline double ConformalAffine::m11() const { return (  Scale*CosAngle ); }
inline double ConformalAffine::m12() const { return (  Scale*SinAngle ); }

inline double ConformalAffine::m21() const { return ( -Scale*SinAngle ); }
inline double ConformalAffine::m22() const { return (  Scale*CosAngle ); }

inline double ConformalAffine::tx () const { return ( TX ); }
inline double ConformalAffine::ty () const { return ( TY ); }


inline double ConformalAffine::det() const { return ( Scale*Scale ); }


inline void   ConformalAffine::set_translation(double _tx, double _ty) { TX = _tx;  TY = _ty;  return; }

inline void   ConformalAffine::set_fixed_point(double _u, double _v) { set_pin(_u, _v, _u, _v);  return; }

inline void ConformalAffine::operator()(double  _u, double  _v, double &  _x, double &  _y) const

{ forward(_u, _v, _x, _y);  return; }


////////////////////////////////////////////////////////////////////////


extern ConformalAffine operator*(const ConformalAffine &, const ConformalAffine &);


////////////////////////////////////////////////////////////////////////


extern double calc_aspect(double width, double height);


extern double calc_mag(double image_width, double image_height,
                       double  view_width, double  view_height);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_MATH_AFFINE_H__  */


////////////////////////////////////////////////////////////////////////




