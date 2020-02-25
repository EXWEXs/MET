
////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <vector>

#include "combine_boolplanes.h"


////////////////////////////////////////////////////////////////////////

   //
   //  assumes all the input BoolPlanes (and the output BoolPlane) are the same size
   //

void combine_boolplanes(const BoolPlane * bpa, const int n_planes, 
                        BoolCalc & calc, 
                        BoolPlane & bp_out)


{

int j, x, y;
const int nx = bp_out.nx();
const int ny = bp_out.ny();
vector<bool> v(n_planes);
bool tf = false;


for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      for (j=0; j<n_planes; ++j)  {

         v[j] = bpa[j].get(x, y);

      }   //  for j

      tf = calc.run(v);

      bp_out.put(tf, x, y);

   }   //  for y

}   //  for x





   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




