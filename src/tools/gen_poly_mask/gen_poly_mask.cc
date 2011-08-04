// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gen_poly_mask.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    10/23/08  Halley Gotway   New
//   001    07/27/10  Halley Gotway  Add lat/lon variables to NetCDF.
//   002    08/04/11  Halley Gotway  Enhance to allow the masking area
//                    to be specified as a lat/lon polyline file or
//                    gridded data file.  Also, add -name option.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "netcdf.hh"
#include "grib_classes.h"

#include "vx_wrfdata.h"
#include "mask_poly.h"
#include "grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_gdata.h"

////////////////////////////////////////////////////////////////////////

static const char * program_name = "gen_poly_mask";

////////////////////////////////////////////////////////////////////////
//
// Variables for command line arguments
//
////////////////////////////////////////////////////////////////////////

// Input data file, polyline file, and output NetCDF file
static ConcatString data_file;
static ConcatString mask_file;
static ConcatString out_file;
static ConcatString mask_name;
static int i_rec = 1;

// Logging level
static int verbosity = 2;

// Masking polyline
static MaskPoly poly_mask;

// Grid on which the data field resides
static Grid grid, grid_mask;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_data_file();
static void process_mask_file();
static void write_netcdf();
static void usage(int, char **);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   // Set handler to be called for memory allocation error
   set_new_handler(oom);

   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the data file and extract the grid information
   process_data_file();

   // Process the mask file
   process_mask_file();

   // Write out the mask file to NetCDF
   write_netcdf();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   int i;

   if(argc < 4) {
      usage(argc, argv);
      exit(1);
   }

   // Store the arguments
   data_file = argv[1];
   mask_file = argv[2];
   out_file  = argv[3];
   mask_name.clear();

   // Parse command line arguments
   for(i=0; i<argc; i++) {

      if(strcmp(argv[i], "-name") == 0) {
         mask_name << argv[i+1];
         i++;
      }
      else if(strcmp(argv[i], "-rec") == 0) {
         i_rec = atoi(argv[i+1]);
         i++;
      }
      else if(strcmp(argv[i], "-v") == 0) {
         verbosity = atoi(argv[i+1]);
         i++;
      }
      else if(argv[i][0] == '-') {
         cerr << "\n\nERROR: process_command_line() -> "
              << "unrecognized command line switch: "
              << argv[i] << "\n\n" << flush;
         exit(1);
      }
   }

   // List the input files
   if(verbosity > 0) {
      cout << "Input Data File:\t" << data_file << "\n"
           << "Input Mask File:\t" << mask_file << "\n" << flush;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_data_file() {
   FileType ftype;
   GribFile gb_file;
   GribRecord rec;
   NcFile *nc_file = (NcFile *) 0;
   WrfData wd;

   // Determine the file type for the data file
   ftype = get_file_type(data_file);

   // Handle the supported file types
   switch(ftype) {

      // GRIB file type
      case(GbFileType):

         // Open the GRIB file
         if(!(gb_file.open(data_file))) {
            cerr << "\n\nERROR: process_data_file() -> "
                 << "can't open GRIB file: "
                 << data_file << "\n\n" << flush;
            exit(1);
         }

         // Process the data file in GRIB format
         read_single_grib_record(gb_file, rec, i_rec-1, wd, grid,
                                 verbosity);
         break;

      // NetCDF file type
      case(NcFileType):

         // Open the NetCDF File
         nc_file = new NcFile(data_file);
         if(!nc_file->is_valid()) {
            cerr << "\n\nERROR: process_data_file() -> "
                 << "can't open NetCDF file: "
                 << data_file << "\n\n" << flush;
            exit(1);
         }

         // Process the data file in NetCDF format
         read_netcdf_grid(nc_file, grid, verbosity);
         break;

      default:
         cerr << "\n\nERROR: process_data_file() -> "
              << "unsupport file type: "
              << data_file << "\n\n" << flush;
         exit(1);
         break;
   }

   if(verbosity > 1) {
      cout << "Parsed Input Grid:\t" << grid.name()
           << " (" << grid.nx() << " x " << grid.ny() << ")\n" << flush;
   }

   // Cleanup
   if(nc_file) { delete nc_file; nc_file = (NcFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_mask_file() {
   FileType ftype;
   GribFile gb_file;
   GribRecord rec;
   NcFile *nc_file = (NcFile *) 0;
   WrfData wd;

   // Determine the file type for the mask file
   ftype = get_file_type(mask_file);

   // Handle the supported file types
   switch(ftype) {

      // GRIB file type
      case(GbFileType):

         // Open the GRIB file
         if(!(gb_file.open(mask_file))) {
            cerr << "\n\nERROR: process_mask_file() -> "
                 << "can't open GRIB file: "
                 << mask_file << "\n\n" << flush;
            exit(1);
         }

         // Process the data file in GRIB format
         read_single_grib_record(gb_file, rec, i_rec-1, wd, grid_mask,
                                 verbosity);

         // If not already specified, retrieve the mask name
         if(mask_name.length() == 0) mask_name = grid_mask.name();
         break;

      // NetCDF file type
      case(NcFileType):

         // Open the NetCDF File
         nc_file = new NcFile(mask_file);
         if(!nc_file->is_valid()) {
            cerr << "\n\nERROR: process_mask_file() -> "
                 << "can't open NetCDF file: "
                 << mask_file << "\n\n" << flush;
            exit(1);
         }

         // Process the data file in NetCDF format
         read_netcdf_grid(nc_file, grid_mask, verbosity);

         // If not already specified, retrieve the mask name
         if(mask_name.length() == 0) mask_name = grid_mask.name();
         break;

      // Otherwise, assume it's an ASCII polyline file
      default:

         // Parse out the polyline from the file
         poly_mask.clear();
         poly_mask.load(mask_file);

         // If not already specified, retrieve the mask name
         if(mask_name.length() == 0) mask_name = poly_mask.name();
         break;
   }

   if(verbosity > 1) {

      if(grid_mask.nx() > 0 && grid_mask.ny() > 0) {
         cout << "Parsed Masking Grid:\t" << mask_name
              << " (" << grid_mask.nx() << " x " << grid_mask.ny()
              << ")\n" << flush;
      }
      else {
         cout << "Parsed Polyline:\t" << mask_name
              << " containing " << poly_mask.n_points() << " points\n"
              << flush;
      }
   }

   // Cleanup
   if(nc_file) { delete nc_file; nc_file = (NcFile *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

void write_netcdf() {
   int n, x, y, in_count, out_count;
   double lat, lon, mask_x, mask_y;
   char var_str[max_str_len];

   int *mask_data = (int *)   0;

   NcFile *f_out   = (NcFile *) 0;
   NcDim *lat_dim  = (NcDim *) 0;
   NcDim *lon_dim  = (NcDim *) 0;
   NcVar *mask_var = (NcVar *) 0;

   // Create a new NetCDF file and open it.
   f_out = new NcFile(out_file, NcFile::Replace);

   if(!f_out->is_valid()) {
      cerr << "\n\nERROR: write_netcdf() -> "
           << "trouble opening output file " << out_file
           << "\n\n" << flush;
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_file.text(), program_name);

   // Add the projection information
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);

   // Define Variables
   mask_var = f_out->add_var(mask_name, ncInt, lat_dim, lon_dim);
   sprintf(var_str, "%s polyline masking region", mask_name.text());
   mask_var->add_att("long_name", var_str);

   // Allocate memory to store the mask values for each grid point
   mask_data = new int [grid.nx()*grid.ny()];

   // Loop through and compute the Lat/Lon and mask value for each grid point
   in_count = out_count = 0;
   for(x=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         // Lat/Lon values for each grid point
         grid.xy_to_latlon(x, y, lat, lon);
         n = two_to_one(grid.nx(), grid.ny(), x, y);

         // Check if Lat/Lon is inside the masking polyline or grid

         // Grid masking
         if(grid_mask.nx() > 0 && grid_mask.ny() > 0) {

            grid_mask.latlon_to_xy(lat, lon, mask_x, mask_y);
            if(mask_x > 0 && mask_x <= grid_mask.nx() &&
               mask_y > 0 && mask_y <= grid_mask.ny()) {
               mask_data[n] = 1;
               in_count++;
            }
            else {
               mask_data[n] = 0;
               out_count++;
            }
         }
         // Polyline masking
         else {

            //Convert the longitude from degrees_east to degrees_west.
            if(poly_mask.latlon_is_inside(lat, (-1.0*lon))) {
               mask_data[n] = 1;
               in_count++;
            }
            else {
               mask_data[n] = 0;
               out_count++;
            }
         }

      } // end for y
   } // end for x

   if(!mask_var->put(&mask_data[0], grid.ny(), grid.nx())) {
      cerr << "\n\nERROR: write_netcdf() -> "
           << "error with mask_var->put\n\n" << flush;
      exit(1);
   }

   // Delete allocated memory
   if(mask_data) { delete mask_data; mask_data = (int *) 0; }

   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   if(verbosity > 1) {
      cout << "Points Inside Mask:\t" << in_count << " of "
           << grid.nx()*grid.ny() << "\n" << flush;
   }

   // List the output file
   if(verbosity > 0) {
      cout << "Output NetCDF File:\t" << out_file  << "\n" << flush;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage(int argc, char *argv[]) {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tdata_file\n"
        << "\tmask_file\n"
        << "\tnetcdf_file\n"
        << "\t[-name str]\n"
        << "\t[-rec i]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"data_file\" is a gridded file in either GRIB "
        << "or NetCDF format from which the grid information is to be "
        << "retrieved (required).\n"

        << "\t\t\"mask_file\" is an ASCII Lat/Lon polyline file "
        << "or gridded data file defining the masking region "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output NetCDF "
        << "mask file to be written (required).\n"

        << "\t\t\"-name str\" specifies the name to be used for the mask"
        << " (optional).\n"

        << "\t\t\"-rec i\" specifies the GRIB record number from which "
        << "the grid information is to be extracted (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n\n"

        << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
