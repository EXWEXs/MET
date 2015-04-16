// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __NC_UTILS_H__
#define  __NC_UTILS_H__

////////////////////////////////////////////////////////////////////////

#include <netcdf.hh>

#include "concat_string.h"
#include "long_array.h"

////////////////////////////////////////////////////////////////////////

extern bool get_var_att(const NcVar *, const ConcatString &, ConcatString &);

extern bool get_var_att_double(const NcVar *, const ConcatString &, double &);
   
extern bool get_var_units(const NcVar *, ConcatString &);

extern bool get_var_level(const NcVar *, ConcatString &);

extern double get_var_missing_value(const NcVar *);

extern double get_var_fill_value(const NcVar *);

extern bool args_ok(const LongArray &);

extern int get_int_var(NcFile *, const char * var_name, int index);

extern double get_double_var(NcFile *, const char * var_name, int index = 0);

extern NcVar* has_var(NcFile *, const char * var_name);

extern NcDim* has_dim(NcFile *, const char * dim_name);

extern bool get_file_att(const NcFile *, const ConcatString &, ConcatString &);

////////////////////////////////////////////////////////////////////////

#endif   /*  __NC_UTILS_H__  */

////////////////////////////////////////////////////////////////////////
