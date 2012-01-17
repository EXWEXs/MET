// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   data2d.cc
//
//   Description:
//      Contains the definition of the DataPlane class.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-01-11  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include "data_plane.h"

#include "vx_log.h"
#include "vx_math.h"
#include "vx_cal.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Begin Code for class DataPlane
//
///////////////////////////////////////////////////////////////////////////////

DataPlane::DataPlane() {

   init_from_scratch();

}

///////////////////////////////////////////////////////////////////////////////

DataPlane::~DataPlane() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

DataPlane::DataPlane(const DataPlane &d) {

   init_from_scratch();

   assign(d);
}

///////////////////////////////////////////////////////////////////////////////

DataPlane & DataPlane::operator=(const DataPlane &d) {

   if(this == &d) return(*this);

   assign(d);

   return(*this);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::init_from_scratch() {

   Data = (double *) 0;

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::assign(const DataPlane &d) {

   clear();

   set_size(d.nx(), d.ny());

   memcpy(Data, d.Data, Nxy*sizeof(double *));

   InitTime  = d.init();
   ValidTime = d.valid();
   LeadTime  = d.lead();
   AccumTime = d.accum();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::clear() {

   if(Data) { delete [] Data;  Data = (double *) 0; }

   Nx = 0;
   Ny = 0;

   Nxy = 0;

   InitTime = ValidTime = (unixtime) 0;
   LeadTime = AccumTime = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////


void DataPlane::dump(ostream & out, int depth) const {
   ConcatString time_str;

   Indent prefix(depth);

   out << prefix << "Nx        = " << Nx  << '\n';
   out << prefix << "Ny        = " << Ny  << '\n';
   out << prefix << "Nxy       = " << Nxy << '\n';
   
   time_str = unix_to_yyyymmdd_hhmmss(InitTime);
   out << prefix << "InitTime  = " << time_str << " (" << InitTime  << ")\n";
   
   time_str = unix_to_yyyymmdd_hhmmss(ValidTime);
   out << prefix << "ValidTime = " << time_str << " (" << ValidTime << ")\n";
   
   time_str = sec_to_hhmmss(LeadTime);
   out << prefix << "LeadTime  = " << time_str << " (" << LeadTime  << ")\n";

   time_str = sec_to_hhmmss(AccumTime);
   out << prefix << "AccumTime = " << time_str << " (" << AccumTime << ")\n";

   //
   //  done
   //

   out.flush();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_size(int x, int y) {

   Nx = x;
   Ny = y;

   Nxy = Nx*Ny;

   Data = new double [Nxy];

   if(!Data) {
      mlog << Error << "\n\nvoid DataPlane::set_size() -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   memset(Data, 0.0, Nxy*sizeof(double));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set(double v, int x, int y) {
   int n;

   n = two_to_one(x, y);

   Data[n] = v;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_constant(double v) {

   // memset(Data, v, Nx*Ny*sizeof(double));

   if(!Data) {
      mlog << Error << "\n\n  DataPlane::set_constant(double) -> "
           << "no data buffer allocated!\n\n";
      exit(1);
   }

   int j;

   for(j=0; j<Nxy; ++j) Data[j] = v;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_init(unixtime ut) {
   InitTime = ut;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_valid(unixtime ut) {
   ValidTime = ut;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_lead(int s) {
   LeadTime = s;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::set_accum(int s) {
   AccumTime = s;
   return;
}

///////////////////////////////////////////////////////////////////////////////

double DataPlane::get(int x, int y) const {
   int n;

   n = two_to_one(x, y);

   return(Data[n]);
}

///////////////////////////////////////////////////////////////////////////////


void DataPlane::threshold(const SingleThresh &st) {
   int j;

   //
   // Loop through the data and apply the threshold to all valid values
   //   1.0 if it meets the threshold criteria
   //   0.0 if it does not
   //
   
   for(j=0; j<Nxy; ++j) {
     
      if( is_bad_data(Data[j]) )  continue;
      if( st.check(Data[j]) )     Data[j] = 1.0;
      else                        Data[j] = 0.0;
      
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int DataPlane::two_to_one(int x, int y) const {
   int n;

   if((x < 0) || (x >= Nx) || (y < 0) || (y >= Ny)) {
      mlog << Error << "\n\nDataPlane::two_to_one() -> "
           << "range check error: (Nx, Ny) = (" << Nx << ", " << Ny
           << "), (x, y) = (" << x << ", " << y << ")\n\n";
      exit(1);
   }

   n = y*Nx + x;

   return(n);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::one_to_two(int n, int &x, int &y) const {

   if(n < 0 || n >= Nxy) {
      mlog << Error << "\n\nDataPlane::one_to_two() -> "
           << "range check error: n = " << n << "but Nx*Ny = " << Nxy
           << "\n\n";
      exit(1);
   }

   x = n%Nx;
   y = n/Nx;

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool DataPlane::s_is_on(int x, int y) const {

   //
   // Return true if the current point is non-zero.
   //
   
   return( !is_eq(get(x, y), 0) );
}

///////////////////////////////////////////////////////////////////////////////

bool DataPlane::f_is_on(int x, int y) const {

   //
   // Consider the box defined by (x,y) as the upper-right corner.
   // Return true if any corner of that box is non-zero.
   //
  
   if( s_is_on(x, y) )                                return(true);

   if( (x > 0) && s_is_on(x - 1, y) )                 return(true);

   if( (x > 0) && (y > 0) && s_is_on(x - 1, y - 1) )  return(true);

   if( (y > 0) && s_is_on(x, y - 1) )                 return(true);

   return(false);
}

///////////////////////////////////////////////////////////////////////////////

void DataPlane::data_range(double & data_min, double & data_max) const {
   int j;
   double value;
   bool first_set = true;

   data_min = data_max = bad_data_double;

   for(j=0; j<Nxy; ++j) {

      value = Data[j];

      if(value == bad_data_double) continue;

      if(first_set) {
         data_min = data_max = value;
         first_set = false;
      }
      else {
         data_min = min(value, data_min);
         data_max = max(value, data_max);
      }
   }   //  for j

   return;
}


///////////////////////////////////////////////////////////////////////////////
//
//  End Code for class DataPlane
//
///////////////////////////////////////////////////////////////////////////////

   //
   //  Code for class DataPlaneArray
   //

///////////////////////////////////////////////////////////////////////////////


DataPlaneArray::DataPlaneArray()

{

init_from_scratch();

}


///////////////////////////////////////////////////////////////////////////////


DataPlaneArray::~DataPlaneArray()

{

clear();

}


///////////////////////////////////////////////////////////////////////////////


DataPlaneArray::DataPlaneArray(const DataPlaneArray & a)

{

init_from_scratch();

assign(a);

}


///////////////////////////////////////////////////////////////////////////////


DataPlaneArray & DataPlaneArray::operator=(const DataPlaneArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::init_from_scratch()

{

Lower = (double *) 0;
Upper = (double *) 0;

Plane = (DataPlane **) 0;

Nplanes = 0;

clear();

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::clear()

{

if ( Nplanes > 0 )  {

   int j;

   for (j=0; j<Nplanes; ++j)  {

      if ( Plane[j] )  { delete Plane[j];  Plane[j] = (DataPlane *) 0; }

   }

   delete [] Plane;  Plane = (DataPlane **) 0;

}

Nplanes = 0;
Nalloc  = 0;

AllocInc = dataplane_default_alloc_inc;

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::assign(const DataPlaneArray & a)

{

clear();

if ( a.Nplanes == 0 )  return;

extend(a.Nplanes);

AllocInc = a.AllocInc;

int j;

for (j=0; j<a.Nplanes; ++j)  {

   add( *(a.Plane[j]), a.Lower[j], a.Upper[j] );

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::extend(int n)

{

if ( Nalloc >= n )  return;

int j, k;
DataPlane ** p = (DataPlane **) 0;
double * b = (double *) 0;
double * t = (double *) 0;

k = (n + AllocInc - 1)/AllocInc;

n = k*AllocInc;

p = new DataPlane * [n];
b = new double      [n];
t = new double      [n];

for (j=0; j<n; ++j)  {

   p[j] = (DataPlane *) 0;

   b[j] = t[j] = 0.0;

}

if ( Plane )  {

   for (j=0; j<Nplanes; ++j)  {

      p[j] = Plane[j];

      b[j] = Lower[j];

      t[j] = Upper[j];

   }   //  for j;

   delete [] Plane;  Plane = (DataPlane **) 0;
   delete [] Lower;  Lower = (double *)     0;
   delete [] Upper;  Upper = (double *)     0;
}

Plane = p;

Lower = b;

Upper = t;

p = (DataPlane **) 0;
b = (double *)     0;
t = (double *)     0;

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::add(const DataPlane & p, double _low, double _up)

{

check_xy_size(p);

extend(Nplanes + 1);

Plane[Nplanes] = new DataPlane;

*(Plane[Nplanes]) = p;

Lower[Nplanes] = _low;
Upper[Nplanes] = _up;

++Nplanes;


   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::check_xy_size(const DataPlane & p) const

{

if ( Nplanes == 0 )  return;

if ( (p.nx() != Plane[0]->nx()) || (p.ny() != Plane[0]->ny()) )  {

   mlog << Error << "\n\n  DataPlaneArray::check_xy_size(const DataPlane &) const -> wrong size!\n\n";

   exit ( 1 );

}

return;

}


///////////////////////////////////////////////////////////////////////////////


double DataPlaneArray::data(int p, int x, int y) const

{

if ( (p < 0) || (p >= Nplanes) )  {

   mlog << Error << "\n\n  DataPlaneArray::data() -> range check error!\n\n";

   exit ( 1 );

}

double value = Plane[p]->get(x, y);

return ( value );

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::set_alloc_inc(int n)

{

if ( n <= 0 )  {

   mlog << Error << "\n\n  void DataPlaneArray::set_alloc_inc(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

AllocInc = n;

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::set_levels(int n, double _low, double _up)

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\n\n  DataPlaneArray::set_levels(int, double, double) -> bad level index ... " << n << "\n\n";

   exit ( 1 );

}

if ( _low > _up )  {

   mlog << Error << "\n\n  DataPlaneArray::set_levels(int, double, double) -> lowtom level > up level!\n\n";

   exit ( 1 );
}

Lower[n] = _low;
Upper[n] = _up;

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::levels(int n, double & _low, double & _up) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\n\n  DataPlaneArray::level(int, double &, double &) -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

_up  = Upper [n];
_low = Lower [n];

return;

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::level_range(double & _low, double & _up) const

{

int j;

_low = _up = bad_data_int;

for (j=0; j<Nplanes; ++j)  {

   if ( is_bad_data(_low) || Lower[j] <= _low ) _low = Lower[j];
   if ( is_bad_data(_up)  || Upper[j] >= _up  ) _up  = Upper[j];

}

return;

}


///////////////////////////////////////////////////////////////////////////////


int DataPlaneArray::nx() const

{

if ( Nplanes == 0 )  {

   mlog << Error << "\n\n  DataPlaneArray::nx() const -> array is empty!\n\n";

   exit ( 1 );

}

return ( Plane[0]->nx() );

}


///////////////////////////////////////////////////////////////////////////////


int DataPlaneArray::ny() const

{

if ( Nplanes == 0 )  {

   mlog << Error << "\n\n  DataPlaneArray::ny() const -> array is empty!\n\n";

   exit ( 1 );

}

return ( Plane[0]->ny() );

}


///////////////////////////////////////////////////////////////////////////////


void DataPlaneArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nplanes  = " << Nplanes    << '\n';
out << prefix << "Nalloc   = " << Nalloc     << '\n';
out << prefix << "AllocInc = " << AllocInc   << '\n';
out << prefix << "Nx       = " << ((Nplanes > 0) ? nx() : 0) << '\n';
out << prefix << "Ny       = " << ((Nplanes > 0) ? ny() : 0) << '\n';

int j;

for (j=0; j<Nplanes; ++j)  {

   out << prefix << "Level " << j << "  = "
       << '[' << Lower[j] << ", " << Upper[j] << ']'
       << '\n';

}   //  for j



   //
   //  done
   //

out.flush();

return;

}


///////////////////////////////////////////////////////////////////////////////


double DataPlaneArray::lower(int n) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\n\n  DataPlaneArray::lower(int) const -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

return ( Lower[n] );

}


///////////////////////////////////////////////////////////////////////////////


double DataPlaneArray::upper(int n) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\n\n  DataPlaneArray::upper(int) const -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

return ( Upper[n] );

}


///////////////////////////////////////////////////////////////////////////////


DataPlane & DataPlaneArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nplanes) )  {

   mlog << Error << "\n\n  DataPlaneArray::operator[](int) const -> bad plane index ... " << n << "\n\n";

   exit ( 1 );

}

return ( *(Plane[n]) );

}


///////////////////////////////////////////////////////////////////////////////




