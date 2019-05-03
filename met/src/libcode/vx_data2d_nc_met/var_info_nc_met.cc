// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   var_info_nc_met.cc
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
#include "var_info_nc_met.h"

#include "vx_math.h"
#include "util_constants.h"
#include "vx_log.h"
#include "grib_strings.h"

///////////////////////////////////////////////////////////////////////////////

static bool is_grib_code_abbr_match(const ConcatString &, int);

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class VarInfoNcMet
//
///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet::VarInfoNcMet() {

   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet::~VarInfoNcMet() {

   clear();
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet::VarInfoNcMet(const VarInfoNcMet &f) {

   init_from_scratch();

   assign(f);
}

///////////////////////////////////////////////////////////////////////////////

VarInfoNcMet & VarInfoNcMet::operator=(const VarInfoNcMet &f) {

   if ( this == &f )  return ( *this );

   assign(f);

   return ( *this );
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::init_from_scratch() {

   // First call the parent's initialization
   VarInfo::init_from_scratch();

   clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::assign(const VarInfoNcMet &v) {
   int i;

   // First call the parent's assign
   VarInfo::assign(v);

   // Copy
   Dimension.clear();
   for(i=0; i<v.n_dimension(); i++) Dimension.add(v.dimension(i));

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::clear() {

   // First call the parent's clear
   VarInfo::clear();

   // Initialize
   Dimension.clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::dump(ostream &out) const {

   // Dump out the contents
   out << "VarInfoNcMet::dump():\n"
       << "  Dimension:\n";
   Dimension.dump(out);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::add_dimension(int dim) {
   Dimension.add(dim);
   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::set_pair(const ConcatString &key, const ConcatString &val) {

   // First call the parent's set_pair function.
   VarInfo::set_pair(key, val);

   // Look for NetCDF keywords.
   if(strcasecmp(key, CONFIG_NetCDF_Dimension) == 0) { Dimension.add(atoi(val)); }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void VarInfoNcMet::set_magic(const ConcatString &s) {
   char tmp_str[max_str_len];
   char *ptr, *ptr2, *ptr3, *save_ptr;

   // Validate the magic string
   VarInfo::set_magic(s);

   // Store the magic string
   MagicStr = s;

   // Initialize the temp string
   strcpy(tmp_str, s);

   // Retreive the NetCDF variable name
   if((ptr = strtok_r(tmp_str, "()/", &save_ptr)) == NULL) {
      mlog << Error << "\nVarInfoNcMet::set_magic() -> "
           << "bad NetCDF variable name specified \""
           << s << "\".\n\n";
      exit(1);
   }

   // Set the requested name and default output name
   set_req_name(ptr);
   set_name(ptr);

   // If there's no level specification, assume (*, *)
   if(strchr(s, '(') == NULL) {
      Level.set_req_name("*,*");
      Level.set_name("*,*");
      Dimension.clear();
      Dimension.add(vx_data2d_star);
      Dimension.add(vx_data2d_star);
   }
   // Parse the level specification
   else {

      // Retreive the NetCDF level specification
      ptr = strtok_r(NULL, "()", &save_ptr);

      // Set the level name
      Level.set_req_name(ptr);
      Level.set_name(ptr);

      // If dimensions are specified, clear the default value
      if(strchr(ptr, ',') != NULL) Dimension.clear();

      // Parse the dimensions
      while((ptr2 = strtok_r(ptr, ",", &save_ptr)) != NULL) {

         // Check for wildcards
         if(strchr(ptr2, '*') != NULL) Dimension.add(vx_data2d_star);
         else {

            // Check for a range of levels
            if((ptr3 = strchr(ptr2, '-')) != NULL) {

               // Check if a range has already been supplied
               if(Dimension.has(range_flag)) {
                  mlog << Error << "\nVarInfoNcMet::set_magic() -> "
                       << "only one dimension can have a range for NetCDF variable \""
                       << s << "\".\n\n";
                  exit(1);
               }
               // Store the dimension of the range and limits
               else {
                  Dimension.add(range_flag);
                  Level.set_lower(atoi(ptr2));
                  Level.set_upper(atoi(++ptr3));
               }
            }
            // Single level
            else {
               Dimension.add(atoi(ptr2));
            }
         }

         // Set ptr to NULL for next call to strtok
         ptr = NULL;
      } // end while

   } // end else

   // Check for "/PROB" to indicate a probability forecast
   if(strstr(s, "/PROB") != NULL) PFlag = 1;

   // Set the long name
   sprintf(tmp_str, "%s(%s)", req_name().text(), Level.req_name().text());
   set_long_name(tmp_str);

   // Set the units
   set_units(na_str);

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_precipitation() const {

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any precipitation variables.
   //
   return(has_prefix(grib_precipitation_abbr,
                     n_grib_precipitation_abbr,
                     Name));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_specific_humidity() const {

   //
   // Check to see if the VarInfo name begins with the GRIB code abbreviation
   // for any specific humidity variables.
   //
   return(has_prefix(grib_specific_humidity_abbr,
                     n_grib_specific_humidity_abbr,
                     Name));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_u_wind() const {
   return(is_grib_code_abbr_match(Name, ugrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_v_wind() const {
   return(is_grib_code_abbr_match(Name, vgrd_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_wind_speed() const {
   return(is_grib_code_abbr_match(Name, wind_grib_code));
}

///////////////////////////////////////////////////////////////////////////////

bool VarInfoNcMet::is_wind_direction() const {
   return(is_grib_code_abbr_match(Name, wdir_grib_code));
}

///////////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous utility functions
//
///////////////////////////////////////////////////////////////////////////////

bool is_grib_code_abbr_match(const ConcatString &str, int grib_code) {
   ConcatString abbr_str;
   bool match = false;

   //
   // Use the default GRIB1 parameter table version number 2
   //
   abbr_str = get_grib_code_abbr(grib_code, 2);

   //
   // Consider it a match if the search string begins with the GRIB code
   // abbreviation, ignoring case.
   //
   if(strncasecmp(str, abbr_str, strlen(abbr_str)) == 0) match = true;

   return(match);
}

///////////////////////////////////////////////////////////////////////////////
