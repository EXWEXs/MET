// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "interp_mthd.h"

///////////////////////////////////////////////////////////////////////////////

ConcatString interpmthd_to_string(const InterpMthd m) {
   ConcatString out;

   switch(m) {
      case(InterpMthd_Min):     out = interpmthd_min_str;     break;
      case(InterpMthd_Max):     out = interpmthd_max_str;     break;
      case(InterpMthd_Median):  out = interpmthd_median_str;  break;
      case(InterpMthd_UW_Mean): out = interpmthd_uw_mean_str; break;
      case(InterpMthd_DW_Mean): out = interpmthd_dw_mean_str; break;
      case(InterpMthd_LS_Fit):  out = interpmthd_ls_fit_str;  break;
      case(InterpMthd_Nbrhd):   out = interpmthd_nbrhd_str;   break;
      case(InterpMthd_Bilin):   out = interpmthd_bilin_str;   break;

      case(InterpMthd_None):
      default:                  out = interpmthd_none_str;    break;
   }   //  switch

   return(out);
}

///////////////////////////////////////////////////////////////////////////////

InterpMthd string_to_interpmthd(const char *mthd_str) {
   InterpMthd m;

        if(strcmp(mthd_str, interpmthd_min_str)     == 0) m = InterpMthd_Min;
   else if(strcmp(mthd_str, interpmthd_max_str)     == 0) m = InterpMthd_Max;
   else if(strcmp(mthd_str, interpmthd_median_str)  == 0) m = InterpMthd_Median;
   else if(strcmp(mthd_str, interpmthd_uw_mean_str) == 0) m = InterpMthd_UW_Mean;
   else if(strcmp(mthd_str, interpmthd_dw_mean_str) == 0) m = InterpMthd_DW_Mean;
   else if(strcmp(mthd_str, interpmthd_ls_fit_str)  == 0) m = InterpMthd_LS_Fit;
   else if(strcmp(mthd_str, interpmthd_nbrhd_str)   == 0) m = InterpMthd_Nbrhd;
   else if(strcmp(mthd_str, interpmthd_bilin_str)   == 0) m = InterpMthd_Bilin;
   else                                                   m = InterpMthd_None;

   return(m);
}

////////////////////////////////////////////////////////////////////////

