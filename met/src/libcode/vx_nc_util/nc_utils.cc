// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <string.h>
#include <cstring>

//#include <netcdf.hh>
#include <netcdf>
using namespace netCDF;

#include "vx_log.h"
#include "nc_utils.h"
#include "util_constants.h"
#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

static const char  level_att_name         [] = "level";
static const char  units_att_name         [] = "units";
static const char  missing_value_att_name [] = "missing_value";
static const char  fill_value_att_name    [] = "_FillValue";

//template <typename T,unsigned S>
//inline unsigned arraysize(const T (&v)[S]) { return S; }
//#define arraysize(arr)  sizeof(arr)/sizeof(*arr)

////////////////////////////////////////////////////////////////////////

//bool find_att(const NcFile *nc, const char *att_name, NcAtt *att) {
//   bool status = false;
//
//   multimap<string,NcGroupAtt>::iterator itAtt;
//   map<string,NcGroupAtt> mapAttrs = var->getAtts();
//   for (itAtt = mapAttrs.begin(); itAtt != mapAttrs.end(); ++itAtt) {
//      if (0 == (strcmp(att_name, ((*itAtt).first).c_str()))) {
//         NcVarAtt attF = (*itAtt).second;
//         att = &attF;
//         status = true 
//         break;
//      }
//   }
//
//   return status;
//}
//
//bool find_att(const NcVar *var, const char *att_name, NcAtt *att) {
//   bool status = false;
//
//   multimap<string,NcVarAtt>::iterator itAtt;
//   map<string,NcVarAtt> mapAttrs = var->getAtts();
//   for (itAtt = mapAttrs.begin(); itAtt != mapAttrs.end(); ++itAtt) {
//      if (0 == (strcmp(att_name, ((*itAtt).first).c_str()))) {
//         NcVarAtt attF = (*itAtt).second;
//         att = &attF;
//         status = true 
//         break;
//      }
//   }
//
//   return status;
//}


bool get_att_value(const NcAtt *att, ConcatString &value) {
   bool status = false;
   if ((0 != att) && !att->isNull()) {
      //CHECK ME !!!!
      //char *att_value = new char[4096];
      //att->getValues(att_value);
      //value = att_value;
      att->getValues(&value);
      status = true;
   }
   return status;
}

int get_att_value_int(const NcAtt *att) {
   int value;
   att->getValues(&value);
   return value;
}

bool get_att_value_chars(const NcAtt *att, ConcatString &value) {
   bool status = false;
   if (!att->isNull()) {
      int att_size = att->getAttLength();
      char *att_value = new char[att_size+1];
      att->getValues(att_value);
      att_value[att_size] = '\0';
      value = att_value;
      //cout << "  nc_utils.cc get_att_value_chars(), att_size: " << att_size << ", att local: " << value << ", att_value: " << att_value << "\n";
      status = true;
   }
   return status;
}

long long get_att_value_llong(const NcAtt *att) {
   long long value;
   att->getValues(&value);
   return value;
}

double get_att_value_double(const NcAtt *att) {
   double value;
   att->getValues(&value);
   return value;
}

double *get_att_value_doubles(const NcAtt *att) {
   double *values = new double[att->getAttLength()];
   att->getValues(values);
   return values;
}

float get_att_value_float(const NcAtt *att) {
   float value;
   att->getValues(&value);
   return value;
}

unixtime get_att_value_unixtime(const NcAtt *att) {
   unixtime time_value = -1;
   switch ( GET_NC_TYPE_ID_P(att) )  {
      case NC_INT:
         time_value = get_att_value_int(att);
         break;

      case NC_CHAR:
         ConcatString s;
         get_att_value_chars(att, s);
         time_value = string_to_unixtime(s);
         break;

      //default:
      //   break;
   }   //  switch
   return time_value;
}

bool get_att_value_string(const NcVar *var, const ConcatString &att_name, ConcatString &value) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_chars(&att, value);
}

int  get_att_value_int   (const NcVar *var, const ConcatString &att_name) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_int(&att);
}
long long  get_att_value_llong (const NcVar *var, const ConcatString &att_name) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_llong(&att);
}
double get_att_value_double(const NcVar *var, const ConcatString &att_name) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_double(&att);
}

bool get_att_value_string(const NcFile *nc, const ConcatString &att_name, ConcatString &value) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_chars(&att, value);
}
int  get_att_value_int   (const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_int(&att);
}
long long  get_att_value_llong (const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_llong(&att);
}
double  get_att_value_double(const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_double(&att);
}


//long long get_att_value_long_long(const NcAtt *att) {
//   long long value;
//   att->getValues(&value);
//   return value;
//}

//char * get_att_value_float(const NcAtt *att) {
//   float value;
//   att->getValues(&value);
//   return value.c_str();
//}

////////////////////////////////////////////////////////////////////////


NcVarAtt get_nc_att(const NcVar * var, const ConcatString &att_name, bool exit_on_error) {
   NcVarAtt att;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   if(IS_INVALID_NC_P(var)) {
      mlog << Error << "\nget_nc_att(NcVar) -> "
           << "can't read attribute \"" << att_name
           << "\" from because variable is invalid.\n\n";
   }
   else {
      multimap<string,NcVarAtt>::iterator itAtt;
      map<string,NcVarAtt> mapAttrs = var->getAtts();
      for (itAtt = mapAttrs.begin(); itAtt != mapAttrs.end(); ++itAtt) {
         if (0 == (strcmp(att_name, ((*itAtt).first).c_str()))) {
            att = (*itAtt).second;
            break;
         }
      }

      if(IS_INVALID_NC(att) && exit_on_error) {
         mlog << Error << "\nget_nc_att(NcVar) -> "
              << "can't read attribute \"" << att_name
              << "\" from \"" << var->getName() << "\" variable.\n\n";
         if (exit_on_error) exit(1);
      }
   }
   return(att);
}

NcGroupAtt get_nc_att(const NcFile * nc, const ConcatString &att_name, bool exit_on_error) {
   NcGroupAtt att;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   if(IS_INVALID_NC_P(nc)) {
      mlog << Error << "\nget_nc_att(NcFile) -> "
           << "can't read attribute \"" << att_name
           << "\" from because NC is invalid.\n\n";
   }
   else {
      multimap<string,NcGroupAtt>::iterator itAtt;
      multimap<string,NcGroupAtt> mapAttrs = nc->getAtts();
      for (itAtt = mapAttrs.begin(); itAtt != mapAttrs.end(); ++itAtt) {
         if (0 == (strcmp(att_name, ((*itAtt).first).c_str()))) {
            att = (*itAtt).second;
            break;
         }
      }

      if(IS_INVALID_NC(att) && exit_on_error) {
         mlog << Error << "\nget_nc_att(NcVar) -> "
              << "can't read attribute \"" << att_name
              << "\" from \"" << nc->getName() << "\".\n\n";
         if (exit_on_error) exit(1);
      }
   }
   return(att);
}

//NcVarAtt *get_nc_att(const NcVar * var, const ConcatString &att_name, bool exit_on_error) {
//   NcVarAtt *att;
//   //NcVarAtt att;
//   NcVarAtt attT;
//   
//   //
//   // Retrieve the NetCDF variable attribute.
//   //
//   if(IS_INVALID_NC_P(var)) {
//      mlog << Error << "\nget_nc_att(NcVar) -> "
//           << "can't read attribute \"" << att_name
//           << "\" from because variable is invalid.\n\n";
//   }
//   else {
//      attT = get_nc_att(var, att_name);
//      att = &attT;
//      if(IS_INVALID_NC_P(att) && exit_on_error) {
//         mlog << Error << "\nget_nc_att(NcVar) -> "
//              << "can't read attribute \"" << att_name
//              << "\" from \"" << var->getName() << "\" variable.\n\n";
//         if (exit_on_error) exit(1);
//      }
//   }
//   return(att);
//}

//NcGroupAtt *get_nc_att(const NcFile * nc, const ConcatString &att_name, bool exit_on_error) {
//   //NcGroupAtt *att = (NcGroupAtt *) 0;
//   NcGroupAtt *att;
//   NcGroupAtt attT;
//   
//   //
//   // Retrieve the NetCDF variable attribute.
//   //
//   if(IS_INVALID_NC_P(nc)) {
//      mlog << Error << "\nget_nc_att(NcVar) -> "
//           << "can't read attribute \"" << att_name
//           << "\" from because NC is invalid.\n\n";
//   }
//   else {
//      attT = get_nc_att(nc, att_name);
//      att = &attT;
//      if(IS_INVALID_NC_P(att) && exit_on_error) {
//         mlog << Error << "\nget_nc_att(NcFile) -> "
//              << "can't read attribute \"" << att_name
//              << "\" from \"" << nc->getName() << "\" variable.\n\n";
//         if (exit_on_error) exit(1);
//      }
//   }
//   return(att);
//}

////////////////////////////////////////////////////////////////////////

bool get_nc_att(const NcVar *var, const ConcatString &att_name,
                ConcatString &att_val, bool exit_on_error) {
   bool status = false;
   //NcVarAtt *att = (NcVarAtt *) 0;
   NcVarAtt att;

   // Initialize
   att_val.clear();

   att = get_nc_att(var, att_name);

   // Look for a match
   if(!att.isNull()) {
      string attr_value;
      att.getValues(attr_value);
      att_val = attr_value.c_str();
      status = true;
   }

   return(status);
}


////////////////////////////////////////////////////////////////////////

bool get_nc_att(const NcVar *var, const ConcatString &att_name,
                int &att_val, bool exit_on_error) {
   bool status = true;
   //NcVarAtt *att = (NcVarAtt *) 0;
   NcVarAtt att;

   // Initialize
   att_val = bad_data_int;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   att = get_nc_att(var, att_name);
   if(att.isNull()) {
      status = false;
      mlog << Error << "\nget_nc_att(int) -> "
           << "can't read attribute \"" << att_name
           << "\" from \"" << var->getName() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   att.getValues(&att_val);

   return(status);
}


////////////////////////////////////////////////////////////////////////
bool get_nc_att(const NcVar *var, const ConcatString &att_name,
                float &att_val, bool exit_on_error) {
   bool status = true;
   NcVarAtt att;

   // Initialize
   att_val = bad_data_float;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   att = get_nc_att(var, att_name);
   if (IS_INVALID_NC(att)) {
      status = false;
      mlog << Error << "\nget_nc_att(float) -> "
           << "can't read attribute \"" << att_name
           << "\" from \"" << var->getName() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   else {
      att.getValues(&att_val);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att(const NcVarAtt *att, ConcatString &att_val) {
   bool status = false;

   // Initialize
   att_val.clear();

   // Look for a match
   if(!att->isNull()) {
      string attr_value;
      att->getValues(attr_value);
      att_val = attr_value.c_str();
      status = true;
   }

   return(status);
}
////////////////////////////////////////////////////////////////////////

bool get_nc_att(const NcVarAtt *att, int &att_val, bool exit_on_error) {
   bool status = true;
   string attr_value;

   // Initialize
   att_val = bad_data_int;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   if(att->isNull()) {
      status = false;
      mlog << Error << "\nget_nc_att(int) -> "
           << "can't read attribute \"" << att->getName()
           << "\" from \"" << att->getParentVar().getName() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   att->getValues(&att_val);

   return(status);
}


////////////////////////////////////////////////////////////////////////
bool get_nc_att(const NcVarAtt *att, float &att_val, bool exit_on_error) {
   bool status = true;

   // Initialize
   att_val = bad_data_float;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   if(att->isNull()) {
      status = false;
      mlog << Error << "\nget_nc_att(float) -> "
           << "can't read attribute \"" << att->getName()
           << "\" from \"" << att->getParentVar().getName() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   att->getValues(&att_val);

   return(status);
}

////////////////////////////////////////////////////////////////////////
bool get_nc_att(const NcVarAtt *att, double &att_val, bool exit_on_error) {
   bool status = true;

   // Initialize
   att_val = bad_data_double;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   if(att->isNull()) {
      status = false;
      mlog << Error << "\nget_nc_att(double) -> "
           << "can't read attribute \"" << att->getName()
           << "\" from \"" << att->getParentVar().getName() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   att->getValues(&att_val);

   return(status);
}

////////////////////////////////////////////////////////////////////////
NcVarAtt *get_var_att(const NcVar *var, const ConcatString &att_name, bool exit_on_error) {
   NcVarAtt *att = (NcVarAtt *) 0;
   NcVarAtt attT;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   attT = get_nc_att(var, att_name);
   att = &attT;
   if(att->isNull()) {
      mlog << Error << "\nget_var_att(float) -> "
           << "can't read attribute \"" << att_name
           << "\" from \"" << var->getName() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   return(att);
}


///////////////////////////////////////////////////////////////////////////////

bool has_att(NcFile * ncfile, const char * att_name, bool exit_on_error)
{
   bool status = false;
   //NcGroupAtt *att = (NcGroupAtt *) 0;
   NcGroupAtt att;

   att = get_nc_att(ncfile, att_name);
   if ( !att.isNull()) {
      status = true;
   } else if(exit_on_error)  {
      mlog << Error << "\nhas_att() -> "
           << "can't find global NetCDF attribute " << att_name << ".\n\n";
      exit ( 1 );
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcGroupAtt *att, ConcatString &att_val) {
   bool status = false;

   // Initialize
   att_val.clear();

   // Look for a match
   if(!att->isNull()) {
      string attr_value;
      att->getValues(attr_value);
      att_val = attr_value.c_str();
      status = true;
   }

   return(status);
}


////////////////////////////////////////////////////////////////////////

//bool get_global_att(const NcFile *nc, NcGroupAtt *att,
//                    const char *att_name, bool error_out) {
//   bool status = true;
//   NcGroupAtt attT;
//
//   attT = get_nc_att(nc, att_name);
//   //att = &attT;
//   *att = attT;
//   // Check error_out status
//   if(error_out && att->isNull()) {
//      status = false;
//      mlog << Error << "\nget_global_att() -> "
//           << "can't find global NetCDF attribute \"" << att_name
//           << "\".\n\n";
//      exit(1);
//   }
//
//   return(status);
//}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const ConcatString &att_name,
                    ConcatString &att_val, bool error_out) {
   bool status = false;
   NcGroupAtt att;

   // Initialize
   att_val.clear();

   att = get_nc_att(nc, att_name);
   if(!att.isNull()) {
      string attr_val;
      att.getValues(attr_val);
      att_val = attr_val.c_str();
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att(ConcatString) -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const char *att_name,
                    int &att_val, bool error_out) {
   //NcGroupAtt att;
   bool status = false;
   ConcatString att_value;

   // Initialize
   att_val = bad_data_int;

   //att = get_nc_att(nc, att_name);

   if(get_global_att(nc, att_name, att_value, error_out)) {
      att_val = atoi(att_value);
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att(int) -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const char *att_name,
                    float &att_val, bool error_out) {
   //NcGroupAtt att;
   bool status = false;
   ConcatString att_value;

   // Initialize
   att_val = bad_data_float;

   if (get_global_att(nc, att_name, att_value, error_out)) {
      att_val = atof(att_value);
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att(float) -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const char *att_name,
                    double &att_val, bool error_out) {
   //NcGroupAtt *att = (NcGroupAtt *) 0;
   //NcGroupAtt att_T;
   bool status = false;
   char values[1024];
   ConcatString att_value;

   // Initialize
   att_val = bad_data_double;

   if (get_global_att(nc, att_name, att_value, error_out)) {
      att_val = (double)atof(att_value);
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att_double() -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   //return(get_global_att_double(nc, att_name, att_val, error_out));
   return (status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att_double(const NcFile *nc, const ConcatString &att_name,
                           double &att_val, bool error_out) {
   NcGroupAtt *att = (NcGroupAtt *) 0;
   NcGroupAtt att_T;
   bool status = false;

   // Initialize
   att_val = bad_data_double;

   att_T = get_nc_att(nc, att_name);
   att = &att_T;

   if(!att->isNull()) {
      att->getValues(&att_val);
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att_double() -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcFile *nc, const string att_name, const int att_val) {
   nc->putAtt(att_name, NcType::nc_INT, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcFile *nc, const string att_name, const string att_val) {
   nc->putAtt(att_name, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcFile *nc, const string att_name, const char *att_val) {
   nc->putAtt(att_name, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcVar *var, const string att_name, const string att_val) {
   var->putAtt(att_name, att_val);
}

////////////////////////////////////////////////////////////////////////
void add_att(NcVar *var, const string att_name, const int att_val) {
   var->putAtt(att_name, NcType::nc_INT, att_val);
}


////////////////////////////////////////////////////////////////////////
void add_att(NcVar *var, const string att_name, const float att_val) {
   var->putAtt(att_name, NcType::nc_FLOAT, att_val);
}

/////////
void add_att(NcVar *var, const string att_name, const double att_val) {
   var->putAtt(att_name, NcType::nc_DOUBLE, att_val);
}


////////////////////////////////////////////////////////////////////////

int get_var_names(NcFile *nc, StringArray *varNames) {
    
   int i, varCount;
   NcVar var;

   varCount = nc->getVarCount();
   //NcDim **dim_array = new NcDim*[dimCount];
   //dimArray = dim_array;
   varNames->extend(varCount);
   
   i = 0;
   multimap<string,NcVar>::iterator itVar;
   multimap<string,NcVar> mapVar = nc->getVars();
   for (itVar = mapVar.begin(); itVar != mapVar.end(); ++itVar) {
      var = (*itVar).second;
      varNames->add(var.getName().c_str());
      i++;
   }

   if (i != varCount) {
      mlog << Error << "\n\tget_var_names() -> "
           << "does not match array, allocated " << varCount << " but assigned " << i << ".\n\n";
   }
   return(varCount);
}

////////////////////////////////////////////////////////////////////////

bool get_var_att_double(const NcVar *var, const ConcatString &att_name,
                        double &att_val) {
   //NcVarAtt *att = (NcVarAtt *) 0;
   NcVarAtt att;
   bool status = false;

   // Initialize
   att_val = bad_data_double;

   att = get_nc_att(var, att_name);

   // Look for a match
   if (!IS_INVALID_NC(att)) {
      att.getValues(&att_val);
      status = true;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_var_att_float(const NcVar *var, const ConcatString &att_name,
                       float &att_val) {
   NcVarAtt att;
   bool status = false;

   // Initialize
   att_val = bad_data_float;

   att = get_nc_att(var, att_name);

   // Look for a match
   if (!IS_INVALID_NC(att)) {
      att.getValues(&att_val);
      status = true;
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

char get_char_val(NcFile * nc, const char * var_name, const int index) {
   //NcVar * var = (NcVar *) 0;

   //*var = nc->getVar(var_name);
   NcVar var = get_var(nc, var_name);
   return (get_char_val(&var, index));
}

////////////////////////////////////////////////////////////////////////

char get_char_val(NcVar *var, const int index) {
   char k;
   std::vector<size_t> start;
   std::vector<size_t> count;

   //
   // Retrieve the character array value from the NetCDF variable.
   //
   start.push_back(index);
   count.push_back(1);
   var->getVar(start, count, &k);
   //if(!var->set_cur(index) || !var->get(&k, 1)) {
   //   mlog << Error << "\n\tget_char_val() -> "
   //        << "can't read data from \"" << var->getName()
   //        << "\" variable.\n\n";
   //   exit(1);
   //}

   return (k);
}

////////////////////////////////////////////////////////////////////////

ConcatString* get_string_val(NcFile * nc, const char * var_name, const int index,
                    const int len, ConcatString &tmp_cs) {
   //NcVar * var = (NcVar *) 0;

   //*var = nc->getVar(var_name);
   NcVar var = get_var(nc, var_name);

   return (get_string_val(&var, index, len, tmp_cs));
}

////////////////////////////////////////////////////////////////////////

ConcatString* get_string_val(NcVar *var, const int index,
                    const int len, ConcatString &tmp_cs) {
   char tmp_str[len];
   std::vector<size_t> start;
   std::vector<size_t> count;

   //
   // Retrieve the character array value from the NetCDF variable.
   //
   start.push_back(index);
   start.push_back(0);
   count.push_back(1);
   count.push_back(len);
   var->getVar(start, count, &tmp_str);
   //if(!var->set_cur(index) || !var->get(&tmp_str[0], 1, len)) {
   //   mlog << Error << "\n\tget_string_val(ConcatString) -> "
   //        << "can't read data from \"" << var->getName()
   //        << "\" variable.\n\n";
   //   exit(1);
   //}

   //
   // Store the character array as a ConcatString
   //
   tmp_cs = tmp_str;

   return (&tmp_cs);
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcFile * nc, const char * var_name, const int index) {
   //NcVar * var = (NcVar *) 0;

   //*var = nc->getVar(var_name);
   NcVar var = get_var(nc, var_name);
   return(get_int_var(&var, index));
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcVar * var, const int index) {
   int k;
   std::vector<size_t> start;
   std::vector<size_t> count;

   k = bad_data_int;
   if (!var->isNull()) {
      start.push_back(index);
      count.push_back(1);
      var->getVar(start, count, &k);
      //if(!var->set_cur(index) || !var->get(&k, 1)) {
      //   mlog << Error << "\n\tget_int_var() -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //}
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

double get_double_var(NcFile * nc, const char * var_name, const int index) {
   //NcVar * var = (NcVar *) 0;

   //*var = nc->getVar(var_name);
   NcVar var = get_var(nc, var_name);
   return(get_double_var(&var, index));
}

////////////////////////////////////////////////////////////////////////

double get_double_var(NcVar * var, const int index) {
   double k;
   std::vector<size_t> start;
   std::vector<size_t> count;

   k = bad_data_double;
   if (!var->isNull()) {
      start.push_back(index);
      count.push_back(1);
      var->getVar(start, count, &k);
      //if(!var->set_cur(index) || !var->get(&k, 1)) {
      //   mlog << Error << "\n\tget_double_var() -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //}
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

float get_float_var(NcFile * nc, const char * var_name, const int index) {
   //NcVar * var = (NcVar *) 0;

   //*var = nc->getVar(var_name);
   NcVar var = get_var(nc, var_name);
   return(get_float_var(&var, index));
}

////////////////////////////////////////////////////////////////////////

float get_float_var(NcVar * var, const int index) {
   float k;
   std::vector<size_t> start;
   std::vector<size_t> count;

   k = bad_data_float;
   if (!var->isNull()) {
      start.push_back(index);
      count.push_back(1);
      var->getVar(start, count, &k);
      //if(!var->set_cur(index) || !var->get(&k, 1)) {
      //   mlog << Error << "\n\tget_float_var() -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //}
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, int *data,
                 const long *dim, const long *cur) {
   
   //
   // Retrieve the input variables
   //
   NcVar var    = get_var(nc, var_name);
   return get_nc_data(&var, data, dim, cur);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, time_t *data) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(int) is called\n";

   if (!var->isNull()) {
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(int) is called\n";

   if (!var->isNull()) {
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data, const long *cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(int) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)1);
      }
      
      *data = bad_data_int;
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data, const long dim, const long cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(int) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;
      start.push_back((size_t)cur);
      count.push_back((size_t)dim);
      
      for (int idx1=0; idx1<dim; idx1++) {
         data[idx1] = bad_data_int;
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

bool get_nc_data(NcVar *var, int *data, const long *dim, const long *cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(int) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data(int *) at nc_util.cc  dim: " << dimC <<"\n";
      //cout << "   get_nc_data(int *) at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
      }
      
      int x_dim_index = (dimC < 2) ? 0 : dimC-2;
      for (int idx1=0; idx1<dim[x_dim_index]; idx1++) {
         if (dimC >= 2) {
           for (int idx2=0; idx2<dim[dimC-1]; idx2++) {
              data[(idx1*dim[dimC-1])+idx2] = bad_data_float;
           }
         }
         else{
            data[idx1] = bad_data_int;
         }
      }
      
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      //if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
      //   mlog << Error << "\nget_nc_data(int *) -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //   exit(1);
      //}
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, short *data, const long *cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(short) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data(float) at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)1);
      }
      *data = bad_data_int;
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, short *data, const long *dim, const long *cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(short) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
      }
      
      int x_dim_index = (dimC < 2) ? 0 : dimC-2;
      for (int idx1=0; idx1<dim[x_dim_index]; idx1++) {
         if (dimC >= 2) {
            for (int idx2=0; idx2<dim[dimC-1]; idx2++) {
               data[(idx1*dim[dimC-1])+idx2] = bad_data_int;
            }
         }
         else{
            data[idx1] = bad_data_int;
         }
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, float *data,
                 const long *dim, const long *cur) {
   
   //
   // Retrieve the input variables
   //
   NcVar var    = get_var(nc, var_name);
   return get_nc_data(&var, data, dim, cur);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(float) is called\n";

   if (!var->isNull()) {
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data, const long *cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(float) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data(float) at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)1);
      }
      *data = bad_data_double;
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data, const long *dim, const long *cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(float) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data(float *) at nc_util.cc  dim: " << dimC <<"\n";
      //cout << "   get_nc_data(float *) at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
      }
      
      int x_dim_index = (dimC < 2) ? 0 : dimC-2;
      for (int idx1=0; idx1<dim[x_dim_index]; idx1++) {
         if (dimC >= 2) {
           for (int idx2=0; idx2<dim[dimC-1]; idx2++) {
              data[(idx1*dim[dimC-1])+idx2] = bad_data_float;
           }
         }
         else{
            data[idx1] = bad_data_double;
         }
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      //if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
      //   mlog << Error << "\nget_nc_data(float *) -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //   exit(1);
      //}
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data, const long dim, const long cur) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(float) is called\n";

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;
      start.push_back((size_t)cur);
      count.push_back((size_t)dim);
      
      for (int idx1=0; idx1<dim; idx1++) {
         data[idx1] = bad_data_double;
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, double *data,
                 const long *dim, const long *cur) {
   
   //
   // Retrieve the input variables
   //
   NcVar var    = get_var(nc, var_name);
   return get_nc_data(&var, data, dim, cur);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data) {
   bool return_status = false;

   if (!var->isNull()) {
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data, const long *cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data(double *) at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)1);
      }
      *data = bad_data_double;
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data, const long dim, const long cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;
      start.push_back((size_t)cur);
      count.push_back((size_t)dim);
      
      for (int idx1=0; idx1<dim; idx1++) {
         data[idx1] = bad_data_double;
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}
////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data, const long *dim, const long *cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data(double *) at nc_util.cc  dim: " << dimC <<"\n";
      //cout << "   get_nc_data(double *) at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
      }
      
      int x_dim_index = (dimC < 2) ? 0 : dimC-2;
      for (int idx1=0; idx1<dim[x_dim_index]; idx1++) {
         if (cur[1] > 0) {
           for (int idx2=0; idx2<dim[dimC-1]; idx2++) {
              data[(idx1*dim[dimC-1])+idx2] = bad_data_double;
           }
         }
         else{
            data[idx1] = bad_data_double;
         }
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(start, count, data);
      //if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
      //   mlog << Error << "\nget_nc_data(float *) -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //   exit(1);
      //}
      return_status = true;
   }
   return(return_status);
}


////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, char *data) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(int) is called\n";

   if (!var->isNull()) {
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      var->getVar(data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, char *data,
                 const long *dim, const long *cur) {
   
   //
   // Retrieve the input variables
   //
   NcVar var    = get_var(nc, var_name);
   return get_nc_data(&var, data, dim, cur);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, char *data, const long dim, const long cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;
      start.push_back((size_t)cur);
      count.push_back((size_t)dim);
      
      for (int idx1=0; idx1<dim; idx1++) {
         data[idx1] = bad_data_char;
      }
      
      //
      // Retrieve the char value from the NetCDF variable.
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, char *data, const long *dim, const long *cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data() at nc_util.cc  dim: " << dimC <<"\n";
      //cout << "   get_nc_data() at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
      }
      
      int x_dim_index = (dimC < 2) ? 0 : dimC-2;
      for (int idx1=0; idx1<dim[x_dim_index]; idx1++) {
         if (dimC >= 2) {
           for (int idx2=0; idx2<dim[dimC-1]; idx2++) {
              data[(idx1*dim[dimC-1])+idx2] = bad_data_char;
           }
         }
         else{
            data[idx1] = bad_data_char;
         }
      }
      
      //
      // Retrieve the char value from the NetCDF variable.
      //
      var->getVar(start, count, data);
      //if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
      //   mlog << Error << "\nget_nc_data(char **) -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //   exit(1);
      //}
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, ncbyte *data,
                 const long *dim, const long *cur) {
   
   //
   // Retrieve the input variables
   //
   NcVar var    = get_var(nc, var_name);
   return get_nc_data(&var, data, dim, cur);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, ncbyte *data, const long dim, const long cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;
      start.push_back((size_t)cur);
      count.push_back((size_t)dim);
      
      for (int idx1=0; idx1<dim; idx1++) {
         data[idx1] = bad_data_char;
      }
      
      //
      // Retrieve the char value from the NetCDF variable.
      //
      var->getVar(start, count, data);
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, ncbyte *data, const long *dim, const long *cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
      //cout << "   get_nc_data() at nc_util.cc  dim: " << dimC <<"\n";
      //cout << "   get_nc_data() at nc_util.cc  dim: " << dimC <<"\n";
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
      }
      
      int x_dim_index = (dimC < 2) ? 0 : dimC-2;
      for (int idx1=0; idx1<dim[x_dim_index]; idx1++) {
         if (dimC >= 2) {
           for (int idx2=0; idx2<dim[dimC-1]; idx2++) {
              data[(idx1*dim[dimC-1])+idx2] = bad_data_char;
           }
         }
         else{
            data[idx1] = bad_data_char;
         }
      }
      
      //
      // Retrieve the char value from the NetCDF variable.
      //
      var->getVar(start, count, data);
      //if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
      //   mlog << Error << "\nget_nc_data(char **) -> "
      //        << "can't read data from \"" << var->getName()
      //        << "\" variable.\n\n";
      //   exit(1);
      //}
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const int data,    long offset0, long offset1, long offset2) {
   vector<size_t> offsets;
   offsets.push_back((size_t)offset0);
   if (0 <= offset1) {
     offsets.push_back((size_t)offset1);
   }
   if (0 <= offset2) {
     offsets.push_back((size_t)offset2);
   }
   var->putVar(offsets, data);
   return true;
}
bool put_nc_data(NcVar *var, const char data,   long offset0, long offset1, long offset2) {
   vector<size_t> offsets;
   offsets.push_back((size_t)offset0);
   if (0 <= offset1) {
     offsets.push_back((size_t)offset1);
   }
   if (0 <= offset2) {
     offsets.push_back((size_t)offset2);
   }
   var->putVar(offsets, data);
   return true;
}
bool put_nc_data(NcVar *var, const float data , long offset0, long offset1, long offset2) {
   vector<size_t> offsets;
   offsets.push_back((size_t)offset0);
   if (0 <= offset1) {
     offsets.push_back((size_t)offset1);
   }
   if (0 <= offset2) {
     offsets.push_back((size_t)offset2);
   }
   var->putVar(offsets, data);
   return true;
}
bool put_nc_data(NcVar *var, const double data, long offset0, long offset1, long offset2) {
   vector<size_t> offsets;
   offsets.push_back((size_t)offset0);
   if (0 <= offset1) {
     offsets.push_back((size_t)offset1);
   }
   if (0 <= offset2) {
     offsets.push_back((size_t)offset2);
   }
   var->putVar(offsets, data);
   return true;
}
bool put_nc_data(NcVar *var, const ncbyte data, long offset0, long offset1, long offset2) {
   vector<size_t> offsets;
   offsets.push_back((size_t)offset0);
   if (0 <= offset1) {
     offsets.push_back((size_t)offset1);
   }
   if (0 <= offset2) {
     offsets.push_back((size_t)offset2);
   }
   var->putVar(offsets, data);
   return true;
}


bool put_nc_data(NcVar *var, const int *data    ) {
   var->putVar(data);
   return true;
}
bool put_nc_data(NcVar *var, const char *data   ) {
   var->putVar(data);
   return true;
}
bool put_nc_data(NcVar *var, const float *data  ) {
   var->putVar(data);
   return true;
}
bool put_nc_data(NcVar *var, const double *data ) {
   var->putVar(data);
   return true;
}
bool put_nc_data(NcVar *var, const ncbyte *data ) {
   var->putVar(data);
   return true;
}

bool put_nc_data(NcVar *var, const int *data,    const long length, const long offset) {
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}
bool put_nc_data(NcVar *var, const char *data,   const long length, const long offset) {
   //cout << "  put_nc_data(const char *, long offset, long length)" << "\n";
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   //cout << " 1 put_nc_data(const char *, long offset, long length)" << "\n";
   //cout << " 1     data: " << data << "\n";
   //cout << " 1   offset: " << offset << "\n";
   //cout << " 1   length: " << length << "\n";
   //cout << " 1   var: " << var << "\n";
   //cout << " 1   var_name: " << var->getName() << "\n";
   //cout << " 1    getType: " << var->getType().getName() << "\n";
   //cout << " 1     dim(0): " << var->getDim(0).getSize() << "\n";
   //cout << " 1     dim(1): " << var->getDim(1).getSize() << "\n";
   //var->putVar(data);
   //cout << " 2 put_nc_data(const char *, long offset, long length)" << "\n";
   //cout << " 11 put_nc_data(const char *, long offset, long length)" << "\n";
   var->putVar(offsets, counts, data);
   //cout << " 22 put_nc_data(const char *, long offset, long length)" << "\n";
   return true;
}
bool put_nc_data(NcVar *var, const float *data , const long length, const long offset) {
   vector<size_t> offsets, counts;
   int dim_count = get_dim_count(var);
   offsets.push_back(offset);
   if (dim_count == 2) {
      offsets.push_back(0);
      counts.push_back(1);
   }
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}
bool put_nc_data(NcVar *var, const double *data, const long length, const long offset) {
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}
bool put_nc_data(NcVar *var, const ncbyte *data, const long length, const long offset) {
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}
bool put_nc_data(NcVar *var, const float *data , const long *length, const long *offset) {
   int dim = get_dim_count(var);
   vector<size_t> offsets, counts;
   for (int idx = 0 ; idx < dim; idx++) {
      offsets.push_back(offset[idx]);
      //cout << "    nc_utils.cc put_nc_data(): offset=" << offset[idx] << "\n";
   }
   for (int idx = 0 ; idx < dim; idx++) {
      counts.push_back(length[idx]);
      //cout << "    nc_utils.cc put_nc_data(): length=" << length[idx] << "\n";
   }
   var->putVar(offsets, counts, data);
   return true;
}
bool put_nc_data(NcVar *var, const char *data , const long *length, const long *offset) {
   int dim = get_dim_count(var);
   vector<size_t> offsets, counts;
   for (int idx = 0 ; idx < dim; idx++) {
      offsets.push_back(offset[idx]);
   }
   for (int idx = 0 ; idx < dim; idx++) {
      counts.push_back(length[idx]);
   }
   var->putVar(offsets, counts, data);
   return true;
}
bool put_nc_data(NcVar *var, const int *data , const long *length, const long *offset) {
   int dim = get_dim_count(var);
   vector<size_t> offsets, counts;
   for (int idx = 0 ; idx < dim; idx++) {
      offsets.push_back(offset[idx]);
   }
   for (int idx = 0 ; idx < dim; idx++) {
      counts.push_back(length[idx]);
   }
   var->putVar(offsets, counts, data);
   return true;
}


bool put_nc_data_with_dims(NcVar *var, const int *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

bool put_nc_data_with_dims(NcVar *var, const int *data,
                           const long len0, const long len1, const long len2) {
   vector<size_t> offsets, counts;
   if (0 < len0) {
     offsets.push_back(0);
     counts.push_back(len0);
   }
   if (0 < len1) {
     offsets.push_back(0);
     counts.push_back(len1);
   }
   if (0 < len2) {
     offsets.push_back(0);
     counts.push_back(len2);
   }
   var->putVar(offsets, counts, data);
   return true;
}

bool put_nc_data_with_dims(NcVar *var, const float *data,
                           const int len0, const int len1, const int len2) {
   //vector<size_t> offsets, counts;
   //if (0 < len0) {
   //  offsets.push_back(0);
   //  counts.push_back(len0);
   //}
   //if (0 < len1) {
   //  offsets.push_back(0);
   //  counts.push_back(len1);
   //}
   //if (0 < len2) {
   //  offsets.push_back(0);
   //  counts.push_back(len2);
   //}
   //var->putVar(offsets, counts, data);
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

bool put_nc_data_with_dims(NcVar *var, const float *data,
                           const long len0, const long len1, const long len2) {
   vector<size_t> offsets, counts;
   if (0 < len0) {
     offsets.push_back(0);
     counts.push_back(len0);
   }
   if (0 < len1) {
     offsets.push_back(0);
     counts.push_back(len1);
   }
   if (0 < len2) {
     offsets.push_back(0);
     counts.push_back(len2);
   }
   var->putVar(offsets, counts, data);
   return true;
}

bool put_nc_data_with_dims(NcVar *var, const double *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

bool put_nc_data_with_dims(NcVar *var, const double *data,
                           const long len0, const long len1, const long len2) {
   vector<size_t> offsets, counts;
   if (0 < len0) {
     offsets.push_back(0);
     counts.push_back(len0);
   }
   if (0 < len1) {
     offsets.push_back(0);
     counts.push_back(len1);
   }
   if (0 < len2) {
     offsets.push_back(0);
     counts.push_back(len2);
   }
   var->putVar(offsets, counts, data);
   return true;
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

NcVar get_var(NcFile *nc, const char *var_name) {
   //
   // Retrieve the variable from the NetCDF file.
   //
   NcVar var;
   //NcVar var = nc->getVar(var_name);
   //if(var.isNull()) {
   //   mlog << Error << "\nget_var() -> "
   //        << "can't read \"" << var_name << "\" variable.\n\n";
   //   exit(1);
   //}
   multimap<string,NcVar> varMap = nc->getVars();
   multimap<string,NcVar>::iterator it = varMap.find(var_name);
   if (it != varMap.end()) {
     NcVar tmpVar = it->second;
     if(tmpVar.isNull()) {
        mlog << Error << "\nget_var() -> "
             << "can't read \"" << var_name << "\" variable.\n\n";
        exit(1);
     }
   
     var = tmpVar;
   }

   return(var);
}

NcVar get_nc_var(NcFile *nc, const char *var_name) {
   //
   // Retrieve the variable from the NetCDF file.
   //
   NcVar var;
   //NcVar var = nc->getVar(var_name);
   //if(var.isNull()) {
   //   mlog << Error << "\nget_var() -> "
   //        << "can't read \"" << var_name << "\" variable.\n\n";
   //   exit(1);
   //}
   multimap<string,NcVar> varMap = nc->getVars();
   multimap<string,NcVar>::iterator it = varMap.find(var_name);
   if (it != varMap.end()) {
     NcVar tmpVar = it->second;
     if(tmpVar.isNull()) {
        mlog << Error << "\nget_var() -> "
             << "can't read \"" << var_name << "\" variable.\n\n";
        exit(1);
     }
   
     var = tmpVar;
   }

   return(var);
}

////////////////////////////////////////////////////////////////////////

//NcVar* get_nc_var(NcFile *nc, const char *var_name) {
//   NcVar *var = (NcVar *) 0;
//   //NcVar *var = new NcVar();
//
//   //
//   // Retrieve the variable from the NetCDF file.
//   //
//   
//   multimap<string,NcVar> varMap = nc->getVars();
//   multimap<string,NcVar>::iterator it = varMap.find(var_name);
//   if (it != varMap.end()) {
//     NcVar tmpVar = it->second;
//     if(tmpVar.isNull()) {
//        mlog << Error << "\nget_nc_var() -> "
//             << "can't read \"" << var_name << "\" variable.\n\n";
//        exit(1);
//     }
//   
//     var = &tmpVar;
//   }
//
//   //*var = nc->getVar(var_name);
//   //if(var->isNull()) {
//   //   mlog << Error << "\nget_nc_var() -> "
//   //        << "can't read \"" << var_name << "\" variable.\n\n";
//   //   exit(1);
//   //}
//
//   return(var);
//}


////////////////////////////////////////////////////////////////////////

bool has_var(NcFile *nc, const char * var_name) {
   //NcVar *v = (NcVar *) 0;
   //*v = nc->getVar(var_name);
   NcVar v = get_var(nc, var_name);
   //if( ! v->isNull() ) return v;
   return !v.isNull();
}

//int get_var_count(NcFile *nc) {
//   return nc->getVarCount();
//}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType, const int deflate_level) {
   std::vector<NcDim> ncDimVector;
   NcVar var = nc->addVar(var_name, ncType, ncDimVector);
   //return nc->addVar(var_name, ncType, ncDim);
   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType,
              const NcDim ncDim, const int deflate_level) {
   NcVar var = nc->addVar(var_name, ncType, ncDim);
   //return nc->addVar(var_name, ncType, ncDim);
   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType,
              const NcDim ncDim1, const NcDim ncDim2, const int deflate_level) {
   vector<NcDim> ncDims;
   ncDims.push_back(ncDim1);
   ncDims.push_back(ncDim2);
   return add_var(nc, var_name, ncType, ncDims, deflate_level);
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType,
              const NcDim ncDim1, const NcDim ncDim2, const NcDim ncDim3, const int deflate_level) {
   vector<NcDim> ncDims;
   ncDims.push_back(ncDim1);
   ncDims.push_back(ncDim2);
   ncDims.push_back(ncDim3);
   return add_var(nc, var_name, ncType, ncDims, deflate_level);
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType,
              const NcDim ncDim1, const NcDim ncDim2, const NcDim ncDim3,
              const NcDim ncDim4, const int deflate_level) {
   vector<NcDim> ncDims;
   ncDims.push_back(ncDim1);
   ncDims.push_back(ncDim2);
   ncDims.push_back(ncDim3);
   ncDims.push_back(ncDim4);
   return add_var(nc, var_name, ncType, ncDims, deflate_level);
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType,
              const vector<NcDim> ncDims, const int deflate_level) {
   //return nc->addVar(var_name, ncType, ncDims);
   NcVar var = nc->addVar(var_name, ncType, ncDims);
   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }
   return var;
}


////////////////////////////////////////////////////////////////////////

NcDim add_dim(NcFile *nc, string dim_name) {
//   NcDim d = nc->addDim(dim_name);
//cout  << "  d.getName(): " << d.getName() << "\n";
//cout  << "  bbb1 d.getName(): " << d.getName() << " d.isNull(): " << d.isNull() << "\n";
   return nc->addDim(dim_name);;
}

////////////////////////////////////////////////////////////////////////

NcDim add_dim(NcFile *nc, string dim_name, size_t dim_size) {
//   NcDim d = nc->addDim(dim_name, dim_size);
//cout  << "  bbb d.getName(): " << d.getName() << " d.isNull(): " << d.isNull() << "\n";
   return nc->addDim(dim_name, dim_size);
}

////////////////////////////////////////////////////////////////////////

//NcDim add_dim(NcFile *nc, string dim_name, long dim_size) {
//   return nc->addDim(dim_name, dim_size);
//}

////////////////////////////////////////////////////////////////////////

bool has_dim(NcFile *nc, const char * dim_name) {
   NcDim d = nc->getDim(dim_name);
   return !d.isNull();
}

////////////////////////////////////////////////////////////////////////

bool get_dim(const NcFile *nc, const ConcatString &dim_name,
             int &dim_val, bool error_out) {
   NcDim dim;
   bool status = false;

   // Initialize
   dim_val = bad_data_int;

   dim = nc->getDim((string)dim_name);

   if(!dim.isNull()) {
      dim_val = (int) (dim.getSize());
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_dim() -> "
           << "can't find NetCDF dimension \"" << dim_name << "\".\n\n";
      exit(1);
   }

   return(status);
}


////////////////////////////////////////////////////////////////////////

int get_dim_count(const NcFile *nc) {
   return(nc->getDimCount());
}

////////////////////////////////////////////////////////////////////////

int get_dim_count(const NcVar *var) {
   return(var->getDimCount());
}

////////////////////////////////////////////////////////////////////////

int get_dim_size(const NcDim *dim) {
   return(dim->getSize());
}

////////////////////////////////////////////////////////////////////////

int get_dim_value(const NcFile *nc, string dim_name, bool error_out) {
   NcDim dim;
   int dim_val;
   bool status = false;

   // Initialize
   dim_val = bad_data_int;

   dim = nc->getDim((string)dim_name);

   if(!dim.isNull()) {
      dim_val = (int) (dim.getSize());
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_dim() -> "
           << "can't find NetCDF dimension \"" << dim_name << "\".\n\n";
      exit(1);
   }

   return(dim_val);
}


////////////////////////////////////////////////////////////////////////

NcDim get_nc_dim(const NcFile *nc, string dim_name) {
   //NcDim d = nc->getDim(dim_name);
   //return &d;
   return nc->getDim(dim_name);
}

////////////////////////////////////////////////////////////////////////

NcDim get_nc_dim(const NcVar *var, string dim_name) {
   NcDim d;
   int dimCount = var->getDimCount();
   for (int idx=0; idx<dimCount; idx++) {
      NcDim dim = var->getDim(idx);
      if (strcmp(dim.getName().c_str(), dim_name.c_str()) == 0) {
         d = dim;
         break;  
      }
   }
   return d;
}

////////////////////////////////////////////////////////////////////////

NcDim get_nc_dim(const NcVar *var, int dim_offset) {
   //NcDim* d = new NcDim();
   //*d = var->getDim(dim_offset);
   //return d;
   return var->getDim(dim_offset);
}


////////////////////////////////////////////////////////////////////////

//bool get_dims(const NcVar *var, int *dim_count, NcDim **dimArray, StringArray *dimNames) {
//    
//   int i, dimCount;
//   NcDim dim;
//   const char * c = (const char *) 0;
//   bool status = false;
//
//   dimCount = var->getDimCount();
//   //NcDim **dim_array = new NcDim*[dimCount];
//   //dimArray = dim_array;
//   dimNames->extend(dimCount);
//   *dim_count = dimCount;
//   
//   i = 0;
//   vector<NcDim>::iterator itDim;
//   vector<NcDim> dims = var->getDims();
//   for (itDim = dims.begin(); itDim != dims.end(); ++itDim) {
//      dim = (*itDim);
//      dimArray[i] = &dim;
//      c = dim.getName().c_str();
//      dimNames->add(c);
//      ++i;
//   }
//   if (i != dimCount) {
//      mlog << Error << "\n\tget_dims() -> "
//           << "does not match array, allocated " << dimCount << " but assigned " << i << ".\n\n";
//   }
//
//   return(status);
//}

bool get_dim_names(const NcFile *nc, StringArray *dimNames) {
    
   int i, dimCount;
   //NcDim dim;
   bool status = false;

   dimCount = nc->getDimCount();
   dimNames->extend(dimCount);
   
   i = 0;
   multimap<string, NcDim>::iterator itDim;
   multimap<string, NcDim> dims = nc->getDims();
   for (itDim = dims.begin(); itDim != dims.end(); ++itDim) {
      //dim = (*itDim.second);
      //dimNames->add(dim.getName().c_str());
      dimNames->add((*itDim).first.c_str());
      i++;
   }

   if (i != dimCount) {
      mlog << Error << "\n\tget_dim_names(nc) -> "
           << "does not match array, allocated " << dimCount << " but assigned " << i << ".\n\n";
   }
   return(status);
}


bool get_dim_names(const NcVar *var, StringArray *dimNames) {
    
   int i, dimCount;
   NcDim dim;
   bool status = false;

   dimCount = var->getDimCount();
   dimNames->extend(dimCount);
   
   i = 0;
   vector<NcDim>::iterator itDim;
   vector<NcDim> dims = var->getDims();
   for (itDim = dims.begin(); itDim != dims.end(); ++itDim) {
      dim = (*itDim);
      dimNames->add(dim.getName().c_str());
      i++;
   }

   if (i != dimCount) {
      mlog << Error << "\n\tget_dim_names(var) -> "
           << "does not match array, allocated " << dimCount << " but assigned " << i << ".\n\n";
   }
   return(status);
}

vector<NcDim> get_dims(const NcVar *var, int *dim_count) {
    
   int dimCount;

   dimCount = var->getDimCount();
   *dim_count = dimCount;
   
   return var->getDims();
}


////////////////////////////////////////////////////////////////////////

//bool get_global_dims(const NcFile *nc, int *dim_count, NcDim **dimArray, StringArray *dimNames) {
//    
//   int i, dimCount;
//   NcDim dim;
//   const char * c = (const char *) 0;
//   bool status = false;
//
//   dimCount = nc->getDimCount();
//   //NcDim **dim_array = new NcDim*[dimCount];
//   //dimArray = dim_array;
//   dimNames->extend(dimCount);
//   *dim_count = dimCount;
//   
//   i = 0;
//   multimap<string,NcDim>::iterator itDim;
//   multimap<string,NcDim> mapDim = nc->getDims();
//   for (itDim = mapDim.begin(); itDim != mapDim.end(); ++itDim) {
//      dim = (*itDim).second;
//      dimArray[i] = &dim;
//      c = dim.getName().c_str();
//      dimNames->add(c);
//      ++i;
//   }
//
//   if (i != dimCount) {
//      mlog << Error << "\n\tget_global_dims(nc) -> "
//           << "does not match array, allocated " << dimCount << " but assigned " << i << ".\n\n";
//   }
//   return(status);
//}

//multimap<string,NcDim> get_global_dims(const NcFile *nc, int *dim_count) {
//    
//   int dimCount;
//
//   dimCount = nc->getDimCount();
//   *dim_count = dimCount;
//   
//   return nc->getDims();
//}


////////////////////////////////////////////////////////////////////////

NcFile *open_ncfile(const char * nc_name, bool write) {
   //return new NcFile(nc_name, file_mode);
   NcFile *nc;
   if (write) {
      nc = new NcFile(nc_name, NcFile::replace, NcFile::nc4);
   }
   else {
      nc = new NcFile(nc_name, NcFile::read);
   }
   return nc;
}

////////////////////////////////////////////////////////////////////////
// Implement the old API var->num_vals()

int get_data_size(NcVar *var) {
   int dimSize = 0;
   int dimCount = 0;
   int data_size = 1;
   
   dimCount = var->getDimCount();
   for (int i=0; i<dimCount; i++) {
      data_size *= var->getDim(i).getSize();
   }
   return data_size;
}

////////////////////////////////////////////////////////////////////////

