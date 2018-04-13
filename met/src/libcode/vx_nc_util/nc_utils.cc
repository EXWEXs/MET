// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <string.h>
#include <cstring>
#include <sys/stat.h>

#include <netcdf>
using namespace netCDF;
using namespace netCDF::exceptions;

#include "vx_log.h"
#include "nc_utils.h"
#include "util_constants.h"
#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

static const char  level_att_name         [] = "level";
static const char  units_att_name         [] = "units";
static const char  missing_value_att_name [] = "missing_value";
static const char  fill_value_att_name    [] = "_FillValue";


int  hdr_typ_idx_block[NC_BUFFER_SIZE_32K];
int  hdr_sid_idx_block[NC_BUFFER_SIZE_32K];
int  hdr_vld_idx_block[NC_BUFFER_SIZE_32K];
float hdr_lat_block[NC_BUFFER_SIZE_32K];
float hdr_lon_block[NC_BUFFER_SIZE_32K];
float hdr_elv_block[NC_BUFFER_SIZE_32K];
int   hdr_prpt_typ_block[NC_BUFFER_SIZE_32K];
int   hdr_irpt_typ_block[NC_BUFFER_SIZE_32K];
int   hdr_inst_typ_block[NC_BUFFER_SIZE_32K];

float hdr_arr_block[NC_BUFFER_SIZE_32K][HDR_ARRAY_LEN];

////////////////////////////////////////////////////////////////////////

void replace_comma_to_underscore(string *var_name) {
   size_t offset;
   offset = var_name->find(',');
   while (offset != string::npos) {
      var_name->replace(offset, 1, "_");
      offset = var_name->find(',', offset);
   }
   offset = var_name->find('*');
   while (offset != string::npos) {
      var_name->replace(offset, 1, "all");
      offset = var_name->find('*', offset);
   }
}

////////////////////////////////////////////////////////////////////////

int check_nc_dims_vars(const NetcdfObsVars obs_vars) {
   int exit_code = exit_code_no_error;
   if(IS_INVALID_NC(obs_vars.strl_dim) ||
      IS_INVALID_NC(obs_vars.obs_dim)  ||
      IS_INVALID_NC(obs_vars.hdr_dim)) {
      exit_code = exit_code_no_dim;
   }
   else if(IS_INVALID_NC(obs_vars.hdr_typ_var) ||
      IS_INVALID_NC(obs_vars.hdr_sid_var) ||
      IS_INVALID_NC(obs_vars.hdr_vld_var)) {
      exit_code = exit_code_no_hdr_vars;
   }
   else if((IS_INVALID_NC(obs_vars.obs_arr_var) && IS_INVALID_NC(obs_vars.obs_val_var))) {
      exit_code = exit_code_no_obs_vars;
   }
   else if((IS_INVALID_NC(obs_vars.hdr_arr_var) && IS_INVALID_NC(obs_vars.hdr_lat_var))) {
      exit_code = exit_code_no_loc_vars;
   }

   return exit_code;
}

////////////////////////////////////////////////////////////////////////

void clear_header_data(NcHeaderData *header_data) {
   header_data->typ_array.clear();
   header_data->sid_array.clear();
   header_data->vld_array.clear();
   header_data->typ_idx_array.clear();
   header_data->sid_idx_array.clear();
   header_data->vld_idx_array.clear();
   header_data->lat_array.clear();
   header_data->lon_array.clear();
   header_data->elv_array.clear();
   header_data->prpt_typ_array.clear();
   header_data->irpt_typ_array.clear();
   header_data->inst_typ_array.clear();
}

////////////////////////////////////////////////////////////////////////
   
bool get_att_value(const NcAtt *att, ConcatString &value) {
   bool status = false;
   if ((0 != att) && !att->isNull()) {
      att->getValues(&value);
      status = true;
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

int get_att_value_int(const NcAtt *att) {
   int value;
   att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

bool get_att_value_chars(const NcAtt *att, ConcatString &value) {
   bool status = false;
   if (!att->isNull()) {
      int att_size = att->getAttLength();
      nc_type attType = att->getType().getId();
      if (attType == NC_CHAR) {
         char *att_value = new char[att_size+1];
         att->getValues(att_value);
         att_value[att_size] = '\0';
         value = att_value;
      }
      else { // MET-788: to handle a custom modified NetCDF
         mlog << Error << "\nget_att_value_chars(NcAtt) -> "
              << "Please convert data type of \"" << GET_NC_NAME_P(att)
              << "\" to NC_CHAR type.\n\n";
         exit(1);
      }
      status = true;
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

long long get_att_value_llong(const NcAtt *att) {
   long long value;
   att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

double get_att_value_double(const NcAtt *att) {
   double value;
   att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

void get_att_value_doubles(const NcAtt *att, NumArray &value) {
   value.erase();
   double *values = new double[att->getAttLength()];
   att->getValues(values);
   for(unsigned int i=0; i<=att->getAttLength(); i++) value.add(values[i]);
   if(values) { delete [] values; values = 0; }
   return;
}

double *get_att_value_doubles(const NcAtt *att) {
   double *values = new double[att->getAttLength()];
   att->getValues(values);
   return values;
}

////////////////////////////////////////////////////////////////////////

float get_att_value_float(const NcAtt *att) {
   float value;
   att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

bool get_att_value_string(const NcVar *var, const ConcatString &att_name, ConcatString &value) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_chars(&att, value);
}

////////////////////////////////////////////////////////////////////////

int  get_att_value_int   (const NcVar *var, const ConcatString &att_name) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_int(&att);
}

////////////////////////////////////////////////////////////////////////

long long  get_att_value_llong (const NcVar *var, const ConcatString &att_name) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_llong(&att);
}

////////////////////////////////////////////////////////////////////////

double get_att_value_double(const NcVar *var, const ConcatString &att_name) {
   NcVarAtt att = get_nc_att(var, att_name);
   return get_att_value_double(&att);
}

////////////////////////////////////////////////////////////////////////

bool get_att_value_string(const NcFile *nc, const ConcatString &att_name, ConcatString &value) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_chars(&att, value);
}

////////////////////////////////////////////////////////////////////////

int  get_att_value_int   (const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_int(&att);
}

////////////////////////////////////////////////////////////////////////

long long  get_att_value_llong (const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_llong(&att);
}

////////////////////////////////////////////////////////////////////////

double  get_att_value_double(const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt att = get_nc_att(nc, att_name);
   return get_att_value_double(&att);
}

////////////////////////////////////////////////////////////////////////

bool    get_att_no_leap_year(const NcVar *var) {
   bool no_leap_year = false;
   NcVarAtt calendar_att = get_nc_att(var, "calendar", false);
   if (!IS_INVALID_NC(calendar_att)) {
      ConcatString calendar_value;
      if (get_att_value_chars(&calendar_att, calendar_value)) {
         no_leap_year = (strcmp("noleap",calendar_value) == 0)
                        || (strcmp("365_day",calendar_value) == 0)
                        || (strcmp("365 days",calendar_value) == 0);
      }
   }
   return no_leap_year;
}

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

////////////////////////////////////////////////////////////////////////

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

bool get_global_att(const char *nc_name, const ConcatString &att_name,
                    ConcatString &att_val) {
   bool status = false;

   // Initialize
   att_val.clear();
   
   NcFile *nc = open_ncfile(nc_name);
   if (0 != nc && !IS_INVALID_NC_P(nc)) {
      status = get_global_att(nc, att_name, att_val, false);
      delete nc;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const char *nc_name, const ConcatString &att_name,
                    bool &att_val) {
   bool status = false;

   // Initialize
   NcFile *nc = open_ncfile(nc_name);
   if (0 != nc && !IS_INVALID_NC_P(nc)) {
      status = get_global_att(nc, att_name, att_val, false);
      delete nc;
   }

   return(status);
}

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
                    bool &att_val, bool error_out) {
   //NcGroupAtt att;
   bool status = false;
   ConcatString att_value;

   // Initialize
   att_val = false;
   if(get_global_att(nc, att_name, att_value, error_out)) {
      att_val = (0 == strcmp("true", att_value.text()))
             || (0 == strcmp("yes", att_value.text()));
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
   bool status = false;
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

int get_version_no(const NcFile *nc) {
   int version_no = 0;
   float att_version_no;
   bool status = get_global_att(nc, (const char *)nc_att_obs_version, att_version_no);
   version_no = (int)(att_version_no * 100);
   //cout << "    att_version_no: " << att_version_no << "  version_no: " << version_no << endl;
   return version_no;
}

////////////////////////////////////////////////////////////////////////

bool is_version_less_than_1_02(const NcFile *nc) {
   int version_no = get_version_no(nc);
   float att_version_no;
   bool status = get_global_att(nc, (const char *)nc_att_obs_version, att_version_no);
   version_no = (int)(att_version_no * 100);
   return (version_no < 102);
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

////////////////////////////////////////////////////////////////////////

void add_att(NcVar *var, const string att_name, const double att_val) {
   var->putAtt(att_name, NcType::nc_DOUBLE, att_val);
}


////////////////////////////////////////////////////////////////////////

int get_var_names(NcFile *nc, StringArray *varNames) {

   int i, varCount;
   NcVar var;

   varCount = nc->getVarCount();
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

   return (k);
}

////////////////////////////////////////////////////////////////////////

ConcatString* get_string_val(NcFile * nc, const char * var_name, const int index,
                    const int len, ConcatString &tmp_cs) {
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

   //
   // Store the character array as a ConcatString
   //
   tmp_cs = tmp_str;

   return (&tmp_cs);
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcFile * nc, const char * var_name, const int index) {
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
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

double get_double_var(NcFile * nc, const char * var_name, const int index) {
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
      
      int vi;
      short vs;
      float vf;
      int dataType = GET_NC_TYPE_ID_P(var);
      switch (dataType) {
         case NC_DOUBLE:
            var->getVar(start, count, &k);
            break;
         case NC_FLOAT:
            var->getVar(start, count, &vf);
            k = (double)vf;
            break;
         case NC_SHORT:
            var->getVar(start, count, &vs);
            k = (double)vs;
            break;
         case NC_INT:
            var->getVar(start, count, &vi);
            k = (double)vi;
            break;
         default:
            mlog << Error << "\nget_double_var() -> "
                 << "data type mismatch (double vs. \"" << GET_NC_TYPE_NAME_P(var)
                 << "\").\nIt won't be converted because of dimension issue.\n"
                 << "Please correct the data type to double for variable \"" << GET_NC_NAME_P(var) << "\".\n\n";
            exit(1);
            break;
      }
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

float get_float_var(NcFile * nc, const char * var_name, const int index) {
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

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data, const long *dim, const long *cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      int data_size = 1;
      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
         data_size *= dim[idx];
      }

      for (int idx1=0; idx1<data_size; idx1++) {
         data[idx1] = bad_data_char;
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

bool get_nc_data(NcVar *var, short *data, const long *cur) {
   bool return_status = false;

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

bool get_nc_data(NcVar *var, short *data, const long *dim, const long *cur) {
   bool return_status = false;

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      int data_size = 1;
      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
         data_size *= dim[idx];
      }

      for (int idx1=0; idx1<data_size; idx1++) {
         data[idx1] = bad_data_char;
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

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      const int dimC = get_dim_count(var);
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

   if (!var->isNull()) {
      std::vector<size_t> start;
      std::vector<size_t> count;

      int data_size = 1;
      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
         data_size *= dim[idx];
      }

      for (int idx1=0; idx1<data_size; idx1++) {
         data[idx1] = bad_data_char;
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

bool get_nc_data(NcVar *var, float *data, const long dim, const long cur) {
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

      int data_size = 1;
      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
         data_size *= dim[idx];
      }

      for (int idx1=0; idx1<data_size; idx1++) {
         data[idx1] = bad_data_char;
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

bool get_nc_data(NcVar *var, char *data) {
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

      int data_size = 1;
      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
         data_size *= dim[idx];
      }

      for (int idx1=0; idx1<data_size; idx1++) {
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

bool get_nc_data(NcFile *nc, const char *var_name, ncbyte *data,
                 const long *dim, const long *cur) {

   //
   // Retrieve the input variables
   //
   NcVar var = get_var(nc, var_name);
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

      int data_size = 1;
      const int dimC = get_dim_count(var);
      for (int idx = 0 ; idx < dimC; idx++) {
         start.push_back((size_t)cur[idx]);
         count.push_back((size_t)dim[idx]);
         data_size *= dim[idx];
      }

      for (int idx1=0; idx1<data_size; idx1++) {
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

NcHeaderData get_nc_hdr_data(NetcdfObsVars obs_vars) {
   NcHeaderData header_data;
   long nhdr_count  = get_dim_size(&obs_vars.hdr_dim);
   int  strl_len    = get_dim_size(&obs_vars.strl_dim);
   int  strl2_len   = strl_len;
   int  typ_len = strl_len;
   int  sid_len = strl_len;
   int  vld_len = strl_len;
   int  hdr_arr_len = IS_INVALID_NC(obs_vars.hdr_arr_dim)
         ? 0 : get_dim_size(&obs_vars.hdr_arr_dim);
   bool has_array_vars = IS_INVALID_NC(obs_vars.hdr_typ_tbl_var);

   if (!IS_INVALID_NC(obs_vars.strl2_dim)) {
      NcDim str_dim;
      strl2_len = get_dim_size(&obs_vars.strl2_dim);
      string dim_name = GET_NC_NAME(obs_vars.strl2_dim);
      str_dim = get_nc_dim((IS_INVALID_NC(obs_vars.hdr_typ_tbl_var)
            ? &obs_vars.hdr_typ_var : &obs_vars.hdr_typ_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) typ_len = strl2_len;
      
      str_dim = get_nc_dim((IS_INVALID_NC(obs_vars.hdr_sid_tbl_var)
            ? &obs_vars.hdr_sid_var : &obs_vars.hdr_sid_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) sid_len = strl2_len;
      
      str_dim = get_nc_dim((IS_INVALID_NC(obs_vars.hdr_vld_tbl_var)
            ? &obs_vars.hdr_vld_var : &obs_vars.hdr_vld_tbl_var), dim_name);
      if (!IS_INVALID_NC(str_dim)) vld_len = strl2_len;
   }
   
   header_data.typ_len   = typ_len;
   header_data.sid_len   = sid_len;
   header_data.vld_len   = vld_len;
   header_data.strl_len  = strl_len;
   header_data.strll_len = strl2_len;
   if (has_array_vars) {
      header_data.typ_array.extend(nhdr_count);
      header_data.sid_array.extend(nhdr_count);
      header_data.vld_array.extend(nhdr_count);
   }
   else {
      header_data.typ_idx_array.extend(nhdr_count);
      header_data.sid_idx_array.extend(nhdr_count);
      header_data.vld_idx_array.extend(nhdr_count);

      int tmp_dim_size;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_typ_dim);
      header_data.typ_array.extend(tmp_dim_size);
      tmp_dim_size = get_dim_size(&obs_vars.hdr_sid_dim);
      header_data.sid_array.extend(tmp_dim_size);
      tmp_dim_size = get_dim_size(&obs_vars.hdr_vld_dim);
      header_data.vld_array.extend(tmp_dim_size);
      mlog << Debug(7)
           << "    tbl dims: messge_type: " << get_dim_size(&obs_vars.hdr_typ_dim)
           << "  station id: " << get_dim_size(&obs_vars.hdr_sid_dim)
           << "  valid_time: " << get_dim_size(&obs_vars.hdr_vld_dim) << "\n";
   }
   header_data.lat_array.extend(nhdr_count);
   header_data.lon_array.extend(nhdr_count);
   header_data.elv_array.extend(nhdr_count);
   
   int buf_size = ((nhdr_count > NC_BUFFER_SIZE_32K)
        ? NC_BUFFER_SIZE_32K : (nhdr_count));
   
   //
   // Allocate space to store the data
   //
   char hdr_typ_str[typ_len+1];
   char hdr_sid_str[sid_len+1];
   char hdr_vld_str[vld_len+1];
   char hdr_typ_block[buf_size][typ_len];
   char hdr_sid_block[buf_size][sid_len];
   char hdr_vld_block[buf_size][vld_len];

   long offsets[2] = { 0, 0 };
   long lengths[2] = { 1, 1 };
   long offsets_1D[1] = { 0 };
   long lengths_1D[1] = { 1 };
   
   //lengths[0] = buf_size;
   //lengths[1] = strl_len;
   for(int i_start=0; i_start<nhdr_count; i_start+=buf_size) {
      buf_size = ((nhdr_count-i_start) > NC_BUFFER_SIZE_32K)
            ? NC_BUFFER_SIZE_32K : (nhdr_count-i_start);
      
      offsets[0] = i_start;
      lengths[0] = buf_size;
      offsets_1D[0] = i_start;
      lengths_1D[0] = buf_size;
   
      //
      // Get the corresponding header message type
      //
      if (has_array_vars) {
         lengths[1] = typ_len;
         if(!get_nc_data(&obs_vars.hdr_typ_var,
               (char *)&hdr_typ_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
      
         //
         // Get the corresponding header station id
         //
         lengths[1] = sid_len;
         if(!get_nc_data(&obs_vars.hdr_sid_var,
               (char *)&hdr_sid_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_sid\n\n";
            exit(1);
         }
         
         //
         // Get the corresponding header valid time
         //
         lengths[1] = vld_len;
         if(!get_nc_data(&obs_vars.hdr_vld_var,
               (char *)&hdr_vld_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_vld\n\n";
            exit(1);
         }
         
         //
         // Get the header for this observation
         //
         lengths[1] = hdr_arr_len;
         if(!get_nc_data(&obs_vars.hdr_arr_var,
               (float *)&hdr_arr_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_arr\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            header_data.typ_array.add(hdr_typ_block[hIndex]);
            header_data.sid_array.add(hdr_sid_block[hIndex]);
            header_data.vld_array.add(hdr_vld_block[hIndex]);
            header_data.lat_array.add(hdr_arr_block[hIndex][0]);
            header_data.lon_array.add(hdr_arr_block[hIndex][1]);
            header_data.elv_array.add(hdr_arr_block[hIndex][2]);
         }
      }
      else {
         // Get the corresponding header message type (index, not string)
         if(!get_nc_data(&obs_vars.hdr_typ_var,
               hdr_typ_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
      
         // Get the corresponding header station id (index, not string)
         if(!get_nc_data(&obs_vars.hdr_sid_var,
               hdr_sid_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_sid\n\n";
            exit(1);
         }
         
         // Get the corresponding header valid time (index, not string)
         if(!get_nc_data(&obs_vars.hdr_vld_var,
               hdr_vld_idx_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_vld\n\n";
            exit(1);
         }
         
         //
         // Get the header for this observation
         //
         if(!get_nc_data(&obs_vars.hdr_lat_var,
               hdr_lat_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_lat\n\n";
            exit(1);
         }
         if(!get_nc_data(&obs_vars.hdr_lon_var,
               hdr_lon_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_lon\n\n";
            exit(1);
         }
         if(!get_nc_data(&obs_vars.hdr_elv_var,
               hdr_elv_block, lengths_1D, offsets_1D)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_elv\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            header_data.typ_idx_array.add(hdr_typ_idx_block[hIndex]);
            header_data.sid_idx_array.add(hdr_sid_idx_block[hIndex]);
            header_data.vld_idx_array.add(hdr_vld_idx_block[hIndex]);
            header_data.lat_array.add(hdr_lat_block[hIndex]);
            header_data.lon_array.add(hdr_lon_block[hIndex]);
            header_data.elv_array.add(hdr_elv_block[hIndex]);
         }
      }
   }
   
   if (!has_array_vars) {
      int tmp_dim_size;
      
      lengths[1] = typ_len;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_typ_dim);
      buf_size = ((tmp_dim_size > NC_BUFFER_SIZE_32K)
           ? NC_BUFFER_SIZE_32K : (tmp_dim_size));
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;
      
         // Get the corresponding header message type (string)
         if(!get_nc_data(&obs_vars.hdr_typ_tbl_var,
               (char *)&hdr_typ_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            header_data.typ_array.add(hdr_typ_block[hIndex]);
         }
      }
      
      lengths[1] = sid_len;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_sid_dim);
      buf_size = ((tmp_dim_size > NC_BUFFER_SIZE_32K)
           ? NC_BUFFER_SIZE_32K : (tmp_dim_size));
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;
      
         // Get the corresponding header station id (string)
         if(!get_nc_data(&obs_vars.hdr_sid_tbl_var,
               (char *)&hdr_sid_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            header_data.sid_array.add(hdr_sid_block[hIndex]);
         }
      }

      lengths[1] = vld_len;
      tmp_dim_size = get_dim_size(&obs_vars.hdr_vld_dim);
      int buf_size = ((tmp_dim_size > NC_BUFFER_SIZE_32K)
           ? NC_BUFFER_SIZE_32K : (tmp_dim_size));
      for(int i_start=0; i_start<tmp_dim_size; i_start+=buf_size) {
         buf_size = ((tmp_dim_size-i_start) > NC_BUFFER_SIZE_32K)
               ? NC_BUFFER_SIZE_32K : (tmp_dim_size-i_start);
         offsets[0] = i_start;
         lengths[0] = buf_size;
      
         // Get the corresponding header valid time (string)
         if(!get_nc_data(&obs_vars.hdr_vld_tbl_var,
               (char *)&hdr_vld_block[0], lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_typ\n\n";
            exit(1);
         }
         for (int hIndex = 0; hIndex < buf_size; hIndex++) {
            header_data.vld_array.add(hdr_vld_block[hIndex]);
         }
      }
   }
   if (!IS_INVALID_NC(obs_vars.pb_hdr_dim)) {
      get_nc_pb_hdr_data(obs_vars, &header_data);
      int last_prpt_typ = header_data.prpt_typ_array.n_elements() - 1;
      int last_irpt_typ = header_data.irpt_typ_array.n_elements() - 1; 
      int last_inst_typ = header_data.inst_typ_array.n_elements() - 1; 
      mlog << Debug(10)
           << "    prpt_typ[0,-1] " << header_data.prpt_typ_array[0]
           << "," << header_data.prpt_typ_array[last_prpt_typ]
           << "    irpt_typ[0,-1] " << header_data.irpt_typ_array[0]
           << "," << header_data.irpt_typ_array[last_irpt_typ]
           << "    inst_typ[0,-1] " << header_data.inst_typ_array[0]
           << "," << header_data.inst_typ_array[last_inst_typ] << "\n";
   }
   return header_data;
}


////////////////////////////////////////////////////////////////////////

void get_nc_pb_hdr_data(NetcdfObsVars obs_vars, NcHeaderData *header_data) {

   long offsets[1] = { 0 };
   long lengths[1] = { 1 };
   int pb_hdr_count = get_dim_size(&obs_vars.pb_hdr_dim);
   
   // Read PB report type
   int buf_size = ((pb_hdr_count > NC_BUFFER_SIZE_32K)
         ? NC_BUFFER_SIZE_32K : (pb_hdr_count));
   for(int i_start=0; i_start<pb_hdr_count; i_start+=buf_size) {
      buf_size = pb_hdr_count - i_start;
      if (buf_size > NC_BUFFER_SIZE_32K) buf_size = NC_BUFFER_SIZE_32K;
      offsets[0] = i_start;
      lengths[0] = buf_size;
      
      if (!IS_INVALID_NC(obs_vars.hdr_prpt_typ_var)) {
         // Get the corresponding header PB message type (string)
         if(!get_nc_data(&obs_vars.hdr_prpt_typ_var,
               hdr_prpt_typ_block, lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_prpt_typ\n\n";
            exit(1);
         }
      }
         
      if (!IS_INVALID_NC(obs_vars.hdr_irpt_typ_var)) {
         // Get the corresponding header In message type (string)
         if(!get_nc_data(&obs_vars.hdr_irpt_typ_var,
               hdr_irpt_typ_block, lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_irpt_typ\n\n";
            exit(1);
         }
      }
         
      if (!IS_INVALID_NC(obs_vars.hdr_inst_typ_var)) {
         // Get the corresponding header instrument type (string)
         if(!get_nc_data(&obs_vars.hdr_inst_typ_var,
               hdr_inst_typ_block, lengths, offsets)) {
            mlog << Error << "\nget_nc_header() -> "
                 << "trouble getting hdr_inst_typ\n\n";
            exit(1);
         }
      }
         
      for (int hIndex = 0; hIndex < buf_size; hIndex++) {
         header_data->prpt_typ_array.add(hdr_prpt_typ_block[hIndex]);
         header_data->irpt_typ_array.add(hdr_irpt_typ_block[hIndex]);
         header_data->inst_typ_array.add(hdr_inst_typ_block[hIndex]);
      }
   }
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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const int *data    ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const char *data   ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const float *data  ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const double *data ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const ncbyte *data ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const int *data,    const long length, const long offset) {
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const char *data,   const long length, const long offset) {
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const double *data, const long length, const long offset) {
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const ncbyte *data, const long length, const long offset) {
   vector<size_t> offsets, counts;
   offsets.push_back(offset);
   offsets.push_back(0);
   counts.push_back(1);
   counts.push_back(length);
   var->putVar(offsets, counts, data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const float *data , const long *length, const long *offset) {
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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const int *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const float *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const double *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

////////////////////////////////////////////////////////////////////////

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

NcVar get_nc_var(NcFile *nc, const char *var_name) {
   //
   // Retrieve the variable from the NetCDF file.
   //
   NcVar var;
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

bool has_var(NcFile *nc, const char * var_name) {
   NcVar v = get_var(nc, var_name);
   return !v.isNull();
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType, const int deflate_level) {
   std::vector<NcDim> ncDimVector;
   string new_var_name = var_name;
   replace_comma_to_underscore(&new_var_name);
   NcVar var = nc->addVar(new_var_name, ncType, ncDimVector);

   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string var_name, const NcType ncType,
              const NcDim ncDim, const int deflate_level) {
   string new_var_name = var_name;
   replace_comma_to_underscore(&new_var_name);
   NcVar var = nc->addVar(new_var_name, ncType, ncDim);

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
   string new_var_name = var_name;
   replace_comma_to_underscore(&new_var_name);
   NcVar var = nc->addVar(new_var_name, ncType, ncDims);
   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcDim add_dim(NcFile *nc, string dim_name) {
   return nc->addDim(dim_name);;
}

////////////////////////////////////////////////////////////////////////

NcDim add_dim(NcFile *nc, string dim_name, size_t dim_size) {
   return nc->addDim(dim_name, dim_size);
}

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
   return var->getDim(dim_offset);
}


////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

vector<NcDim> get_dims(const NcVar *var, int *dim_count) {

   int dimCount;

   dimCount = var->getDimCount();
   *dim_count = dimCount;

   return var->getDims();
}

////////////////////////////////////////////////////////////////////////

NcFile *open_ncfile(const char * nc_name, bool write) {
   NcFile *nc = (NcFile *)0;

   try {
      if (write) {
         nc = new NcFile(nc_name, NcFile::replace, NcFile::nc4);
      }
      else {
         struct stat fileInfo;
         if (stat(nc_name, &fileInfo) == 0) {
            nc = new NcFile(nc_name, NcFile::read);
         }
      }
   }
   catch(NcException& e) {
   }
   return nc;
}

////////////////////////////////////////////////////////////////////////
// Implement the old API var->num_vals()

int get_data_size(NcVar *var) {
   int dimCount = 0;
   int data_size = 1;

   dimCount = var->getDimCount();
   for (int i=0; i<dimCount; i++) {
      data_size *= var->getDim(i).getSize();
   }
   return data_size;
}

////////////////////////////////////////////////////////////////////////
