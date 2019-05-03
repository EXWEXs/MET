// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __IS_BAD_DATA_H__
#define  __IS_BAD_DATA_H__


////////////////////////////////////////////////////////////////////////


#include <cmath>
#include "vx_math/constants.h"


////////////////////////////////////////////////////////////////////////


inline int is_bad_data(int a)  {
   if(a == bad_data_int) return(1);
   else                  return(0);
}

inline int is_bad_data(double a) {
   if(abs((long double) (a - bad_data_double)) < 10E-5) return(1);
   else                                                 return(0);
}

inline int is_bad_data(float a) {
   if(abs((long double) (a - bad_data_float)) < 10E-5) return(1);
   else                                                return(0);
}

inline int is_eq(double a, double b) {
   if(abs((long double) (a - b)) < 10E-5) return(1);
   else                                   return(0);
}


////////////////////////////////////////////////////////////////////////


#endif   //  __IS_BAD_DATA_H__


////////////////////////////////////////////////////////////////////////




