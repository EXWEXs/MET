
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_math/vx_math.h"
#include "vx_util/vx_util.h"
#include "vx_data_grids/st_grid.h"


////////////////////////////////////////////////////////////////////////


static double     st_func(double lat, bool is_north_hemisphere);
static double st_der_func(double lat, bool is_north_hemisphere);

static double st_inv_func(double r, bool is_north_hemisphere);

static void reduce(double & angle);

static double stereographic_segment_area(double u0, double v0, double u1, double v1);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class StereographicGrid
   //


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::~StereographicGrid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid(const StereographicData & data)

{

clear();

Lcen = data.lcen;

Nx = data.nx;
Ny = data.ny;

IsNorthHemisphere = true;

Name = data.name;

   //
   //  calculate Alpha
   //

Alpha = (1.0 + sind(data.scale_lat))*((data.r_km)/(data.d_km));

   //
   //  Calculate Bx, By
   //

double r0, theta0;

r0 = st_func(data.lat_pin, IsNorthHemisphere);

theta0 = Lcen - data.lon_pin;

Bx = data.x_pin - Alpha*r0*sind(theta0);
By = data.y_pin + Alpha*r0*cosd(theta0);

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid(const StereoType2Data & data)

{

clear();

Lcen = data.lcen;

Nx = data.nx;
Ny = data.ny;

switch ( data.hemisphere )  {

   case 'N':  IsNorthHemisphere = true;   break;
   case 'S':  IsNorthHemisphere = false;  break;

   default:
      cerr << "\n\n  StereographicGrid::StereographicGrid(const StereoType2Data &) -> bad hemisphere ...\""
           << (data.hemisphere) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch
 
Name = data.name;

   //
   //  calculate Alpha
   //

Alpha = (1.0 + sind(fabs(data.scale_lat)))*((data.r_km)/(data.d_km));

   //
   //  Calculate Bx, By
   //

double r0, theta0;

r0 = st_func(data.lat_pin, IsNorthHemisphere);

theta0 = Lcen - data.lon_pin;

Bx = data.x_pin - Alpha*r0*sind(theta0);
By = data.y_pin + Alpha*r0*cosd(theta0);

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


StereographicGrid::StereographicGrid(const StereoType3Data & data)

{

clear();

Lcen = data.lcen;

Nx = data.nx;
Ny = data.ny;

Name = data.name;

Alpha = data.alpha;

Bx = data.bx;
By = data.by;

if ( data.hemisphere == 'N' )  IsNorthHemisphere = true;
else                           IsNorthHemisphere = false;

   //
   //  Done
   //

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::clear()

{

Lcen = 0.0;

Bx = 0.0;
By = 0.0;

Alpha = 0.0;

Nx = 0;
Ny = 0;

Name.clear();

IsNorthHemisphere = true;

return;

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::f(double lat) const

{

return ( st_func(lat, IsNorthHemisphere) );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::df(double lat) const

{

return ( st_der_func(lat, IsNorthHemisphere) );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

double r, theta;


reduce(lon);

r = st_func(lat, IsNorthHemisphere);

theta = Lcen - lon;

if ( !IsNorthHemisphere )  theta = -theta;

x = Bx + Alpha*r*sind(theta);

y = By - Alpha*r*cosd(theta);

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

double r, theta;


x = (x - Bx)/Alpha;
y = (y - By)/Alpha;

r = sqrt( x*x + y*y );

lat = st_inv_func(r, IsNorthHemisphere);

if ( fabs(r) < 1.0e-5 )  theta = 0.0;
else                     theta = atan2d(x, -y);   //  NOT atan2d(y, x);

if ( !IsNorthHemisphere )  theta = -theta;

lon = Lcen - theta;

reduce(lon);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::calc_area(int x, int y) const

{

double u[4], v[4];
double sum;


xy_to_uv(x - 0.5, y - 0.5, u[0], v[0]);  //  lower left
xy_to_uv(x + 0.5, y - 0.5, u[1], v[1]);  //  lower right
xy_to_uv(x + 0.5, y + 0.5, u[2], v[2]);  //  upper right
xy_to_uv(x - 0.5, y + 0.5, u[3], v[3]);  //  upper left


sum = uv_closedpolyline_area(u, v, 4);

sum *= earth_radius_km*earth_radius_km;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


int StereographicGrid::nx() const

{

return ( Nx );

}


////////////////////////////////////////////////////////////////////////


int StereographicGrid::ny() const

{

return ( Ny );

}


////////////////////////////////////////////////////////////////////////


ConcatString StereographicGrid::name() const

{

return ( Name );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::uv_closedpolyline_area(const double *u, const double *v, int n) const

{

int j, k;
double sum;


sum = 0.0;

for (j=0; j<n; ++j)  {

   k = (j + 1)%n;

   sum += stereographic_segment_area(u[j], v[j], u[k], v[k]);

}   //  for j

sum = fabs(sum);

return ( sum );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::xy_closedpolyline_area(const double *x, const double *y, int n) const

{

int j;
double sum;
double *u = (double *) 0;
double *v = (double *) 0;

u = new double [n];
v = new double [n];

if ( !u || !v )  {

   cerr << "\n\n  StereographicGrid::xy_closedpolyline_area() -> memory allocation error\n\n";

   exit ( 1 );

}

for (j=0; j<n; ++j)  {

   xy_to_uv(x[j], y[j], u[j], v[j]);

}

sum = uv_closedpolyline_area(u, v, n);

sum *= earth_radius_km*earth_radius_km;

delete [] u;  u = (double *) 0;
delete [] v;  v = (double *) 0;

return ( sum );

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::uv_to_xy(double u, double v, double &x, double &y) const

{

x = Alpha*v + Bx;

y = -Alpha*u + By;

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::xy_to_uv(double x, double y, double &u, double &v) const

{

u = (x - Bx)/Alpha;

v = (y - By)/(-Alpha);

return;

}


////////////////////////////////////////////////////////////////////////


void StereographicGrid::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];



out << prefix << "Name       = ";

if ( Name.length() > 0 )  out << '\"' << Name << '\"';
else                      out << "(nul)\n";

out << '\n';   //  no prefix

out << prefix << "Projection = Stereographic\n";

out << prefix << "Hemisphere = " << (IsNorthHemisphere ? "North" : "South") << "\n";

sprintf(junk, "%.5f", Lcen);
fix_float(junk);
out << prefix << "Lcen       = " << junk << "\n";

sprintf(junk, "%.5f", Alpha);
fix_float(junk);
out << prefix << "Alpha      = " << junk << "\n";

sprintf(junk, "%.5f", Bx);
fix_float(junk);
out << prefix << "Bx         = " << junk << "\n";

sprintf(junk, "%.5f", By);
fix_float(junk);
out << prefix << "By         = " << junk << "\n";

comma_string(Nx, junk);
out << prefix << "Nx         = " << junk << "\n";

comma_string(Ny, junk);
out << prefix << "Ny         = " << junk << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString StereographicGrid::serialize() const

{

ConcatString a;
char junk[256];

a << "Projection: Stereographic";

a << " Nx: " << Nx;
a << " Ny: " << Ny;

a << " IsNorthHemisphere: " << ( IsNorthHemisphere ? "true" : "false");

sprintf(junk, " Lcen: %.3f", Lcen);   a << junk;

sprintf(junk, " Bx: %.3f", Bx);   a << junk;
sprintf(junk, " By: %.3f", By);   a << junk;

sprintf(junk, " Alpha: %.4f", Alpha);   a << junk;

   //
   //  done
   //

return ( a );

}


////////////////////////////////////////////////////////////////////////


double StereographicGrid::rot_grid_to_earth(int x, int y) const

{

double lat, lon, angle;
double diff, hemi;

// Convert to lat/lon
xy_to_latlon((double) x, (double) y, lat, lon);

// Difference between lon and the center longitude
diff = Lcen - lon;

// Figure out if the grid is in the northern or southern hemisphere
// by checking whether the first latitude (p1_deg -> Phi1_radians)
// is greater than zero
// NH -> hemi = 1, SH -> hemi = -1

xy_to_latlon(0.0, 0.0, lat, lon);

if ( lat < 0.0) hemi = -1.0;
else            hemi = 1.0;

// Compute the rotation angle
angle = diff*hemi;

return(angle);

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


double st_func(double lat, bool is_north_hemisphere)

{

double r;

if ( is_north_hemisphere )  r = tand(45.0 - 0.5*lat);
else                        r = tand(45.0 + 0.5*lat);

return ( r );

}


////////////////////////////////////////////////////////////////////////


double st_inv_func(double r, bool is_north_hemisphere)

{

double lat;

lat = 90.0 - 2.0*atand(r);

if ( !is_north_hemisphere )  lat = -lat;

return ( lat );

}


////////////////////////////////////////////////////////////////////////


double st_der_func(double lat, bool is_north_hemisphere)

{

double a;

if ( is_north_hemisphere )  a = -1.0/(1.0 + sind(lat));
else                        a = -1.0/(1.0 - sind(lat));

return ( a );

}


////////////////////////////////////////////////////////////////////////


void reduce(double & angle)

{

angle -= 360.0*floor( (angle/360.0) + 0.5 );

return;

}


////////////////////////////////////////////////////////////////////////


double stereographic_segment_area(double u0, double v0, double u1, double v1)
 
{

cerr << "\n\n  warning -> stereographic_segment_area() -> hasn't been tested yet!\n\n";
 
double b, answer, t1, t2;
 
b =   u0*u0 + v0*v0 + u1*u1 + v1*v1
    + u0*u0*v1*v1 + u1*u1*v0*v0
    - 2.0*( u0*u1 + v0*v1 + u0*v0*u1*v1 );
 
b = 1.0/sqrt(b);
 
t1 =  b*( u1*u1 + v1*v1 - u0*u1 - v0*v1 );
t2 = -b*( u0*u0 + v0*v0 - u0*u1 - v0*v1 );
                                                   
answer = atan(t1) - atan(t2);                      
                                                   
answer *= 2.0*b*( u0*v1 - u1*v0 );                 
                                                   
return ( answer );                                 
                                                   
}

////////////////////////////////////////////////////////////////////////


   //
   //  Grid functions
   //


////////////////////////////////////////////////////////////////////////


Grid::Grid(const StereographicData & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const StereoType2Data & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const StereoType3Data & data)

{

init_from_scratch();

set(data);

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const StereographicData & data)

{

clear();

rep = new StereographicGrid (data);

if ( !rep )  {

   cerr << "\n\n  Grid::set(const StereographicData &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const StereoType2Data & data)

{

clear();

rep = new StereographicGrid (data);

if ( !rep )  {

   cerr << "\n\n  Grid::set(const StereoType2Data &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const StereoType3Data & data)

{

clear();

rep = new StereographicGrid (data);

if ( !rep )  {

   cerr << "\n\n  Grid::set(const StereoType3Data &) -> memory allocation error\n\n";

   exit ( 1 );

}

rep->refCount = 1;

return;

}


////////////////////////////////////////////////////////////////////////






