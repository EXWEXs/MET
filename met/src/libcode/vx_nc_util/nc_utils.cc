// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <string.h>

#include <netcdf.hh>

#include "nc_utils.h"
#include "util_constants.h"
#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

static const char  level_att_name         [] = "level";
static const char  units_att_name         [] = "units";
static const char  missing_value_att_name [] = "missing_value";
static const char  fill_value_att_name    [] = "_FillValue";

////////////////////////////////////////////////////////////////////////

bool get_var_att(const NcVar *var, const ConcatString &att_name,
                 ConcatString &att_val) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;
   
   // Initialize
   att_val.clear();

   n = var->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = var->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val << att->as_string(0);
         status = true;
         break;
      }
   }
   
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_var_att_double(const NcVar *var, const ConcatString &att_name,
                        double &att_val) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;
   
   // Initialize
   att_val = bad_data_double;

   n = var->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = var->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val = att->as_double(0);
         status = true;
         break;
      }
   }
   
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_var_units(const NcVar *var, ConcatString &att_val) {
  
   return(get_var_att(var, units_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

bool get_var_level(const NcVar *var, ConcatString &att_val) {

   return(get_var_att(var, level_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

double get_var_missing_value(const NcVar *var) {
   double v;
   
   if(!get_var_att_double(var, missing_value_att_name, v)) {
      v = bad_data_double;
   }
   
   return(v);
}

////////////////////////////////////////////////////////////////////////

double get_var_fill_value(const NcVar *var) {
   double v;
   
   if(!get_var_att_double(var, fill_value_att_name, v)) {
      v = bad_data_double;
   }
   
   return(v);
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcFile * nc, const char * var_name, int index) {
   int k;
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);

   var->set_cur(index);

   var->get(&k, 1);

   return(k);
}

////////////////////////////////////////////////////////////////////////

double get_double_var(NcFile * nc, const char * var_name, int index) {
   double k;
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);

   var->set_cur(index);

   var->get(&k, 1);

   return(k);
}

////////////////////////////////////////////////////////////////////////

bool args_ok(const LongArray & a) {
   int j, k;

   for (j=0; j<(a.n_elements()); ++j)  {
     
      k = a[j];

      if ( (k < 0) && (k != vx_data2d_star) ) return (false);
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

NcVar* has_var(NcFile *nc, const char * var_name) {
   for(int i=0; i < nc->num_vars(); i++){
      NcVar* v = nc->get_var(i);
      if( !strcmp(v->name(), var_name) ) return v;
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////

NcDim* has_dim(NcFile *nc, const char * dim_name) {
   for(int i=0; i < nc->num_dims(); i++){
      NcDim* d = nc->get_dim(i);
      if( !strcmp(d->name(), dim_name) ) return d;
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////

bool get_file_att(const NcFile *nc, const ConcatString &att_name,
                  ConcatString &att_val) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;
   
   // Initialize
   att_val.clear();

   n = nc->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = nc->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val << att->as_string(0);
         status = true;
         break;
      }
   }
   
   return(status);
}

////////////////////////////////////////////////////////////////////////
