

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "is_grib_file.h"
#include "is_met_nc_file.h"
#include "is_netcdf_file.h"
#include "is_pinterp_file.h"
#include "is_bufr_file.h"

#include "vx_data2d.h"
#include "vx_util.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


//
// List of file name extensions corresponding to these file types
//
static const char *gb_file_ext [] = { "grib", "grb", "gb" };
static const int   n_gb_file_ext  = sizeof(gb_file_ext)/sizeof(*gb_file_ext);

// static const char *nc_file_ext [] = { "netcdf", "ncf", "nc" };
// static const int   n_nc_file_ext  = sizeof(nc_file_ext)/sizeof(*nc_file_ext);

static const char *bf_file_ext [] = { "bufr", "bfr", "prepbufr", "pb" };
static const int   n_bf_file_ext  = sizeof(bf_file_ext)/sizeof(*bf_file_ext);


////////////////////////////////////////////////////////////////////////


static GrdFileType file_type_by_suffix(const char * filename);


////////////////////////////////////////////////////////////////////////

GrdFileType grd_file_type(const char * filename)

{

GrdFileType type = FileType_None;

   //
   //  first check to see if this file exists
   //

if ( ! file_exists(filename) )  {
  
   mlog << Error << "\ngrd_file_type() -> "
        << "file does not exist \"" << filename << "\"\n\n";
   exit(1);

}

   //
   //  try to get the file type from the filename suffix
   //

type = file_type_by_suffix(filename);

if ( type != FileType_None )  return ( type );

   //
   //  suffix failed, so look inside the file
   //

     if ( is_grib1_file   (filename) ) type = FileType_Gb1;
else if ( is_grib2_file   (filename) ) type = FileType_Gb2;
else if ( is_met_nc_file  (filename) ) type = FileType_NcMet;
else if ( is_netcdf_file  (filename) ) type = FileType_General_Netcdf;
else if ( is_pinterp_file (filename) ) type = FileType_NcPinterp;
else if ( is_bufr_file    (filename) ) type = FileType_Bufr;
else                                   type = FileType_None;

   //
   //  done
   //

return ( type );

}


////////////////////////////////////////////////////////////////////////


GrdFileType file_type_by_suffix(const char * filename)

{

int j;
const ConcatString suffix = filename_suffix(filename);

if ( suffix.empty() ) return ( FileType_None );

   //
   //  grib ?
   //

for (j=0; j<n_gb_file_ext; ++j)  {

   if ( suffix == gb_file_ext[j] )  return ( FileType_Gb1 );

}

   //
   //  prepbufr or bufr ?
   //

for (j=0; j<n_bf_file_ext; ++j)  {

   if ( suffix == bf_file_ext[j] )  return ( FileType_Bufr );

}

   //
   //  nope
   //

return ( FileType_None );

}


////////////////////////////////////////////////////////////////////////


