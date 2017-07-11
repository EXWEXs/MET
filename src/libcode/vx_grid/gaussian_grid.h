

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __GAUSSIAN_GRID_H__
#define  __GAUSSIAN_GRID_H__


////////////////////////////////////////////////////////////////////////


#include "grid_base.h"

#include "gaussian_grid_defs.h"


////////////////////////////////////////////////////////////////////////


class GaussianGrid : public GridRep {

      friend class Grid;

   private:

      GaussianGrid();
     ~GaussianGrid();
      GaussianGrid(const GaussianData &);

      double y_to_lat(int y) const;

      int Nx;
      int Ny;

      double Lon_Zero;   //  longitude that has x = 0

      double Delta_Lon;

      double * North_Latitudes;   //  allocated, increasing order

      int N_north_lats;

      ConcatString Name;

      GaussianData Data;

      void clear();

      double delta_lon() const;

         //
         //  grid interface
         //

      virtual void latlon_to_xy(double lat, double lon, double & x, double & y) const;

      virtual void xy_to_latlon(double x, double y, double & lat, double & lon) const;

      virtual double calc_area(int x, int y) const;

      virtual int nx() const;
      virtual int ny() const;

      double scale_km() const;

      virtual ConcatString name() const;

      void dump(ostream &, int = 0) const;

      ConcatString serialize() const;

      GridInfo info() const;

      double rot_grid_to_earth(int x, int y) const;

      bool is_global() const;

      void shift_right(int);

      GridRep * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double GaussianGrid::delta_lon() const { return ( Nx == 0 ? 0.0 : Delta_Lon ); }

inline double GaussianGrid::scale_km() const { return ( -1.0 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __GAUSSIAN_GRID_H__  */


////////////////////////////////////////////////////////////////////////



