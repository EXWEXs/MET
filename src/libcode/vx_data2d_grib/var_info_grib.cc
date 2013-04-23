// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_grib.cc
//
//   Description:
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//
///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include <map>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "var_info.h"
#include "var_info_grib.h"

#include "util_constants.h"
#include "grib_strings.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"
#include "vx_data2d.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoGrib
//
///////////////////////////////////////////////////////////////////////////////

VarInfoGrib::VarInfoGrib() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib::~VarInfoGrib() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib::VarInfoGrib(const VarInfoGrib &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoGrib & VarInfoGrib::operator=(const VarInfoGrib &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::assign(const VarInfoGrib &v) {

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   PTV     = v.ptv();
   Code    = v.code();
   LvlType = v.lvl_type();
   PCode   = v.p_code();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   PTV     = default_grib_ptv;
   Code    = bad_data_int;
   LvlType = bad_data_int;
   PCode   = bad_data_int;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoGrib::dump():\n"
       << "  PTV     = " << PTV     << "\n"
       << "  Code    = " << Code    << "\n"
       << "  LvlType = " << LvlType << "\n"
       << "  PCode   = " << PCode   << "\n";

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_ptv(int v) {
   PTV = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_code(int v) {
   Code = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_lvl_type(int v) {
   LvlType = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_p_code(int v) {
   PCode = v;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_magic(const ConcatString & s) {

   // Validate the magic string
   VarInfo::set_magic(s);

   // Store the magic string
   MagicStr = s;

}

///////////////////////////////////////////////////////////////////////////////

void VarInfoGrib::set_dict(Dictionary & dict) {

   VarInfo::set_dict(dict);

   int tab_match = -1;
   Grib1TableEntry tab;
   ConcatString field_name = dict.lookup_string(conf_key_name,      false);
   int field_ptv           = dict.lookup_int   (conf_key_GRIB1_ptv, false);
   int field_rec           = dict.lookup_int   (conf_key_GRIB1_rec, false);

   //  if the GRIB parameter table version is not specified, default to 2
   if( bad_data_int == field_ptv ) field_ptv = 2;

   //  if the name is specified, use it
   if( !field_name.empty() ){

      //  look up the name in the grib tables
      if( !GribTable.lookup_grib1(field_name, field_ptv, field_rec, tab, tab_match) ){
         mlog << Error << "\nVarInfoGrib::set_dict() - unrecognized GRIB1 field abbreviation '"
              << field_name << "' for table version " << field_ptv << "\n\n";
         exit(1);
      }

   }

   //  if the field name is not specified, look for and use indexes
   else {

      //  if either the field name or the indices are specified, bail
      if( bad_data_int == field_ptv || bad_data_int == field_rec ){
         mlog << Error << "\nVarInfoGrib::set_dict() - either name or GRIB1_ptv "
              << "and GRIB1_rec must be specified in field information\n\n";
         exit(1);
      }

      //  use the specified indexes to look up the field name
      if( !GribTable.lookup_grib1(field_ptv, field_rec, tab) ){
         mlog << Error << "\nVarInfoGrib::set_dict() - no parameter found with matching "
              << "GRIB1_ptv ("     << field_ptv     << ") "
              << "GRIB1_rec ("     << field_rec     << ")\n\n";
         exit(1);
      }

      //  use the lookup parameter name
      field_name = tab.parm_name;
   }

   //  set the matched parameter lookup information
   set_name      ( field_name       );
   set_req_name  ( field_name       );
   set_ptv       ( tab.table_number );
   set_code      ( tab.code         );
   set_units     ( tab.units        );
   set_long_name ( tab.full_name    );

   //  call the parent to set the level information
   set_level_info_grib(dict);

   //  check for a probability boolean setting
   if( dict.lookup_bool(conf_key_prob, false) ){
      set_p_flag( true );
      return;
   }

   //  check for a probability dictionary setting
   Dictionary* dict_prob;
   if( NULL == (dict_prob = dict.lookup_dictionary(conf_key_prob, false)) )
      return;

   //  gather information from the prob dictionary
   ConcatString prob_name = dict_prob->lookup_string(conf_key_name);
   field_ptv              = dict_prob->lookup_int   (conf_key_GRIB1_ptv, false);
   field_rec              = dict_prob->lookup_int   (conf_key_GRIB1_rec, false);
   double thresh_lo       = dict_prob->lookup_double(conf_key_thresh_lo, false);
   double thresh_hi       = dict_prob->lookup_double(conf_key_thresh_hi, false);

   //  if the GRIB parameter table version is not specified, default to 2
   if( bad_data_int == field_ptv ) field_ptv = 2;

   //  look up the probability field abbreviation
   if( !GribTable.lookup_grib1(prob_name, field_ptv, field_rec, tab, tab_match) ){
      mlog << Error << "\nVarInfoGrib::set_dict() - unrecognized GRIB1 probability field "
           << "abbreviation '" << prob_name << "'\n\n";
      exit(1);
   }

   set_p_flag      ( true      );
   set_p_code      ( tab.code  );
   set_p_units     ( tab.units );

   set_prob_info_grib(prob_name, thresh_lo, thresh_hi);

}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_precipitation() const {
   int i;
   bool status = false;

   //
   // The ReqName member contains the requested GRIB code abbreviation.
   // Check to see if it matches the GRIB precipitation abbreviations.
   //
   for(i=0; i<n_grib_precipitation_abbr; i++) {
      if(strcmp(ReqName, grib_precipitation_abbr[i]) == 0) {
         status = true;
         break;
      }
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_specific_humidity() const {
   int i;
   bool status = false;

   //
   // The ReqName meber contains the requested GRIB code abbreviation.
   // Check to see if it matches the GRIB specific humidity abbreviations.
   //
   for(i=0; i<n_grib_specific_humidity_abbr; i++) {
      if(strcmp(ReqName, grib_specific_humidity_abbr[i]) == 0) {
         status = true;
         break;
      }
   }

   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_u_wind() const {
   return(Code == ugrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_v_wind() const {
   return(Code == vgrd_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_wind_speed() const {
   return(Code == wind_grib_code);
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoGrib::is_wind_direction() const {
   return(Code == wdir_grib_code);
}

///////////////////////////////////////////////////////////////////////////////
