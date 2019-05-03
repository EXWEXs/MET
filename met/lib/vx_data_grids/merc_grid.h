// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __DATA_GRIDS_MERCATOR_GRID_H__
#define  __DATA_GRIDS_MERCATOR_GRID_H__


////////////////////////////////////////////////////////////////////////


   //
   //  grid classes by Randy Bullock
   //


////////////////////////////////////////////////////////////////////////


#include <vx_data_grids/grid_base.h>


////////////////////////////////////////////////////////////////////////


class MercatorGrid : public GridRep {

      friend class Grid;

   private:

      MercatorGrid();
     ~MercatorGrid();
      MercatorGrid(const MercatorData &);

         //
         //
         //

      void clear();

      void xy_to_uv(double x, double y, double &u, double &v) const;
      void uv_to_xy(double u, double v, double &x, double &y) const;

      double uv_closedpolyline_area(const double *u, const double *v, int n) const;

      double xy_closedpolyline_area(const double *x, const double *y, int n) const;

      double f(double) const;

      double df(double) const;

      MercatorData mc_data;

      char * Name;

      double Lat_LL_radians;
      double Lon_LL_radians;

      double Lat_UR_radians;
      double Lon_UR_radians;

      double Mx;
      double My;

      double Bx;
      double By;

      int Nx;
      int Ny;

         //
         //  grid interface
         //

      void latlon_to_xy(double lat, double lon, double &x, double &y) const;

      void xy_to_latlon(double x, double y, double &lat, double &lon) const;

      double calc_area(int x, int y) const;
      double calc_area_ll(int x, int y) const;

      int nx() const;
      int ny() const;

      const char * name() const;

      double EarthRadiusKM() const;

      ProjType proj_type() const;

      double rot_grid_to_earth(int x, int y) const;

      void grid_data(GridData &) const;
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __DATA_GRIDS_MERCATOR_GRID_H__  */


////////////////////////////////////////////////////////////////////////
