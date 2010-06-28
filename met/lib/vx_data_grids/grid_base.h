

////////////////////////////////////////////////////////////////////////


#ifndef  __GRID_BASE_H__
#define  __GRID_BASE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


#include "vx_util/vx_util.h"

#include "vx_data_grids/st_grid_defs.h"
#include "vx_data_grids/lc_grid_defs.h"
#include "vx_data_grids/latlon_grid_defs.h"
#include "vx_data_grids/merc_grid_defs.h"


////////////////////////////////////////////////////////////////////////


class Grid;   //  forward reference


////////////////////////////////////////////////////////////////////////


class GridInfo {

   private:

      void init_from_scratch();

      void assign(const GridInfo &);

   public:

      GridInfo();
     ~GridInfo();
      GridInfo(const GridInfo &);
      GridInfo & operator=(const GridInfo &);

      void clear();

      bool ok() const;

      void set(const LambertData       &);
      void set(const StereographicData &);
      void set(const LatLonData        &);
      void set(const MercatorData      &);

      void create_grid(Grid &) const;

         //
         //  at most ONE of these should be nonzero
         //

      const LambertData       * lc;   //  allocated
      const StereographicData * st;   //  allocated
      const LatLonData        * ll;   //  allocated
      const MercatorData      * m;    //  allocated

};


////////////////////////////////////////////////////////////////////////


class Integrand {

   public:

      Integrand();
      virtual ~Integrand();

      virtual double operator()(double) const = 0;

};


////////////////////////////////////////////////////////////////////////


class GridInterface {   //  pure abstract class for grid public interface

   public:

      GridInterface();
      virtual ~GridInterface();


      virtual void latlon_to_xy(double lat, double lon, double &x, double &y) const = 0;

      virtual void xy_to_latlon(double x, double y, double &lat, double &lon) const = 0;

      virtual double calc_area(int x, int y) const = 0;

      virtual int nx() const = 0;
      virtual int ny() const = 0;

      virtual ConcatString name() const = 0;

      virtual void dump(ostream &, int = 0) const = 0;

      virtual GridInfo info() const = 0;

      virtual double rot_grid_to_earth(int x, int y) const = 0;

};


////////////////////////////////////////////////////////////////////////


class GridRep : public GridInterface {

      friend class Grid;

   private:

      int refCount;

      GridRep(const GridRep &);
      GridRep & operator=(const GridRep &);

   public:

      GridRep();
      virtual ~GridRep();

      virtual void dump(ostream &, int = 0) const = 0;

      virtual ConcatString serialize() const = 0;

      virtual GridInfo info() const = 0;

      virtual double rot_grid_to_earth(int x, int y) const = 0;

};


////////////////////////////////////////////////////////////////////////


class Grid : public GridInterface {

   friend bool operator==(const Grid &, const Grid &);

   protected:

      void init_from_scratch();

      void assign(const Grid &);

      GridRep * rep;

      void detach();

      void attach(GridRep *);

   public:

      Grid();
      Grid(const char *);   //  lookup by name
      Grid(const LambertData       &);
      Grid(const StereographicData &);
      Grid(const LatLonData        &);
      Grid(const MercatorData      &);
      virtual ~Grid();
      Grid(const Grid &);
      Grid & operator=(const Grid &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set (const char *);   //  lookup by name
      void set (const LambertData       &);
      void set (const StereographicData &);
      void set (const LatLonData        &);
      void set (const MercatorData      &);

      void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      double calc_area(int x, int y) const;

      int nx() const;
      int ny() const;

      ConcatString name() const;

      ConcatString serialize() const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

};


////////////////////////////////////////////////////////////////////////


extern bool operator==(const Grid &, const Grid &);


////////////////////////////////////////////////////////////////////////


#endif   //  __GRID_BASE_H__


////////////////////////////////////////////////////////////////////////



