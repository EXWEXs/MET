
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __LATLON_GRID_H__
#define  __LATLON_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "vx_data_grids/grid_base.h"


////////////////////////////////////////////////////////////////////////


class LatLonGrid : public GridRep {

      friend class Grid;

   private:

      LatLonGrid();
     ~LatLonGrid();
      LatLonGrid(const LatLonData &);

      double lat_ll_deg;
      double lon_ll_deg;

      double delta_lat_deg;
      double delta_lon_deg;

      int Nx;
      int Ny;

      ConcatString Name;

      void clear();

         //
         //  grid interface
         //

      virtual void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      virtual void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      virtual double calc_area(int x, int y) const;

      virtual int nx() const;
      virtual int ny() const;

      virtual ConcatString name() const;

      void dump(ostream &, int = 0) const;

      ConcatString serialize() const;

};


////////////////////////////////////////////////////////////////////////


#endif   //  __LATLON_GRID_H__


////////////////////////////////////////////////////////////////////////



