

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_MERCATOR_GRID_H__
#define  __MET_MERCATOR_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"


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

      void xy_to_uv(double x, double y, double & u, double & v) const;
      void uv_to_xy(double u, double v, double & x, double & y) const;

      double f(double) const;

      double df(double) const;

      ConcatString Name;

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

      MercatorData Data;

         //
         //  grid interface
         //

      void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      int nx() const;
      int ny() const;

      ConcatString name() const;

      void dump(ostream &, int = 0) const;

      ConcatString serialize() const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool is_global() const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_MERCATOR_GRID_H__  */


////////////////////////////////////////////////////////////////////////



