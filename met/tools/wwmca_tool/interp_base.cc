

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "interp_base.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class InterpolationValue
   //


////////////////////////////////////////////////////////////////////////


InterpolationValue::InterpolationValue()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


InterpolationValue::~InterpolationValue()

{

clear();

}


////////////////////////////////////////////////////////////////////////


InterpolationValue::InterpolationValue(const InterpolationValue & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


InterpolationValue & InterpolationValue::operator=(const InterpolationValue & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::clear()

{

ok = false;

value = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void InterpolationValue::assign(const InterpolationValue & i)

{

clear();

ok = i.ok;

value = i.value;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Interpolator
   //


////////////////////////////////////////////////////////////////////////


Interpolator::Interpolator()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Interpolator::~Interpolator()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void Interpolator::init_from_scratch()

{

Data = (InterpolationValue *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Interpolator::clear()

{

if ( Data )  { delete [] Data;  Data = (InterpolationValue *) 0; }

Width = 0;

Wm1o2 = 0;

NgoodNeeded = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void Interpolator::assign(const Interpolator & I)

{

Interpolator::clear();

Width = I.Width;

Wm1o2 = I.Wm1o2;

NgoodNeeded = I.NgoodNeeded;

if ( !(I.Data) )  return;

int j, n;

n = Width*Width;

Data = new InterpolationValue [n];

for (j=0; j<n; ++j)  Data[j] = I.Data[j];

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Interpolator::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Width       = " << Width       << "\n";
out << prefix << "Wm1o2       = " << Wm1o2       << "\n";
out << prefix << "NgoodNeeded = " << NgoodNeeded << "\n";

if ( Data )  out << prefix << "Data has been allocated\n";
else         out << prefix << "Data not allocated\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int Interpolator::two_to_one(int x, int y) const

{

if ( (x < 0) || (x >= Width) || (y < 0) || (y >= Width) )  {

   cerr << "\n\n  Interpolator::two_to_one(int x, int y) -> range check error ... "
        << "Width = " << Width << ",  (x, y) = "
        << '(' << x << ", " << y << ")\n\n";

   exit ( 1 );

}

int n;

n = y*Width + x;

return ( n );

}


////////////////////////////////////////////////////////////////////////


void Interpolator::put_good(int x, int y, double value)

{

if ( !Data )  {

   cerr << "\n\n  Interpolator::put_good(int x, int y, double value) -> null data pointer!\n\n";

   exit ( 1 );

}

int n = two_to_one(x, y);

Data[n].ok = true;

Data[n].value = value;

return;

}


////////////////////////////////////////////////////////////////////////


void Interpolator::put_bad(int x, int y)

{

if ( !Data )  {

   cerr << "\n\n  Interpolator::put_bad(int x, int y, double value) -> null data pointer!\n\n";

   exit ( 1 );

}

int n = two_to_one(x, y);

Data[n].ok = false;

Data[n].value = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void Interpolator::set_size(int W)

{

clear();

if ( (W < 1) || (W%2 == 0) )  {

   cerr << "\n\n  Interpolator::set_size(int) -> bad value ... " << W << "\n\n";

   exit ( 1 );

}

Width = W;

Wm1o2 = (W - 1)/2;

Data = new InterpolationValue [Width*Width];

return;

}


////////////////////////////////////////////////////////////////////////


void Interpolator::set_ngood_needed(int n)

{

if ( (n < 0) || (n >= (Width*Width)) )  {

   cerr << "\n\n  Interpolator::set_ngood_needed(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

NgoodNeeded = n;

return;

}


////////////////////////////////////////////////////////////////////////


int Interpolator::n_good() const

{

if ( !Data )  {

   cerr << "\n\n  Interpolator::n_good() -> null data pointer!\n\n";

   exit ( 1 );

}

int j, n, count;

n = Width*Width;

count = 0;

for (j=0; j<n; ++j)  {

   if ( Data[j].ok )  ++count;

}

return ( count );

}


////////////////////////////////////////////////////////////////////////


int Interpolator::n_bad() const

{

if ( !Data )  {

   cerr << "\n\n  Interpolator::n_bad() -> null data pointer!\n\n";

   exit ( 1 );

}

int j, n, count;

n = Width*Width;

count = 0;

for (j=0; j<n; ++j)  {

   if ( !(Data[j].ok) )  ++count;

}

return ( count );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const InterpolationValue & I)

{

if ( I.ok )  out << (I.value);
else         out << "(bad)";

return ( out );

}


////////////////////////////////////////////////////////////////////////





