// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   gen_dland.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    03/19/12  Halley Gotway   New
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

#include "vx_log.h"
#include "data2d_factory.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "write_netcdf.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

// North-West Hemisphere up to 60N
static const LatLonData NWHemTenthData =
   { "NWHemTenthDegree", 0.0, -180.0, 0.1, 0.1, 601, 1801 };

// Default location of data file
static const char *default_land_data_file =
   "MET_BASE/data/tc_data/aland.dat";

static const double km_per_deg = 111.12;
   
////////////////////////////////////////////////////////////////////////
//    
// Variables for command line arguments
//
////////////////////////////////////////////////////////////////////////

static ConcatString out_filename;
static LatLonData GridData = NWHemTenthData;
static bool latlon_flag = true;
static ConcatString land_data_file = default_land_data_file;

////////////////////////////////////////////////////////////////////////

extern "C" {
   void aland_(const char *, float *, float *, float *);
}

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void process_distances();
static void usage();
static void set_grid(const StringArray &);
static void set_noll(const StringArray &);
static void set_land(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   program_name = get_short_name(argv[0]);

   // Set handler to be called for memory allocation error
   set_new_handler(oom);
   
   // Process the command line arguments
   process_command_line(argc, argv);

   // Process the MODE file
   process_distances();
   
   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   //
   // Check for too few arguments
   //
   if(argc < 1) usage();

   //
   // Parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // Set the usage function
   //
   cline.set_usage(usage);

   //
   // Add the options function calls
   //
   cline.add(set_grid,      "-grid", 6);
   cline.add(set_noll,      "-noll", 0);
   cline.add(set_land,      "-land", 1);
   cline.add(set_logfile,   "-log",  1);
   cline.add(set_verbosity, "-v",    1);
   
   cline.allow_numbers();

   //
   // Parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left.
   //
   if(cline.n() != 1) usage();

   // Store the required arguments
   out_filename  = cline[0];

   return;
}

////////////////////////////////////////////////////////////////////////

void process_distances() {
   int n, x, y, c;
   double latd, lond;
   float latf, lonf1, lonf2, d1, d2;
   float *dland = (float *) 0;
   ConcatString data_file = replace_path(land_data_file);

   // Instantiate the grid
   Grid grid(GridData);
   
   // NetCDF variables
   NcFile *f_out     = (NcFile *) 0;
   NcDim  *lat_dim   = (NcDim *) 0;
   NcDim  *lon_dim   = (NcDim *) 0;
   NcVar  *dland_var = (NcVar *) 0;

   // Create a new NetCDF file and open it
   f_out = new NcFile(out_filename, NcFile::Replace);

   if(!f_out->is_valid()) {
      mlog << Error << "\nprocess_distances() -> "
           << "trouble opening output file " << out_filename
           << "\n\n";
      f_out->close();
      delete f_out;
      f_out = (NcFile *) 0;
      exit(1);
   }

   // Add global attributes
   mlog << Debug(3) << "Writing NetCDF global attributes.\n";
   write_netcdf_global(f_out, out_filename, program_name);

   // Add the projection information
   mlog << Debug(3) << "Writing NetCDF map projection.\n";
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());
   
   // Add the lat/lon variables
   if(latlon_flag) {
      mlog << Debug(3) << "Writing NetCDF lat/lon variables.\n";
      write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);
   }

   // Define Variables
   dland_var = f_out->add_var("dland", ncFloat, lat_dim, lon_dim);
   dland_var->add_att("long_name", "distance to land");
   dland_var->add_att("units", "nm");

   // Allocate memory to store the data values for each grid point
   dland = new float [grid.nx()*grid.ny()];

   // Dump out grid info
   mlog << Debug(2)
        << "Computing distances for " << grid.nx() * grid.ny()
        << " points in grid (" << grid.serialize() << ")\n";
        
   // Loop over the grid and compute the distance to land for each point
   if(mlog.verbosity_level() == 3) cout << "Processing" << flush;
   for(x=0,c=0; x<grid.nx(); x++) {
      for(y=0; y<grid.ny(); y++) {

         if(++c % (int) (grid.nx()*grid.ny()/100) == 0 &&
            mlog.verbosity_level() == 3) cout << "." << flush;
        
         // Call two_to_one
         n = DefaultTO.two_to_one(grid.nx(), grid.ny(), x, y);
        
         // Convert x,y to lat,lon
         grid.xy_to_latlon(x, y, latd, lond);
         latf  = (float) latd;
         lonf1 = (float) rescale_deg(-1.0*lond, -360.0,   0.0);
         lonf2 = (float) rescale_deg(-1.0*lond,    0.0, 360.0);

         // Compute distance to land
         aland_(data_file, &lonf1, &latf, &d1);
         aland_(data_file, &lonf2, &latf, &d2);

         // Convert to nuatical miles and store the minimum
         d1 *= nautical_miles_per_km;
         d2 *= nautical_miles_per_km;
         dland[n] = min(d1, d2);

         mlog << Debug(4)
              << "Lat = "  << latf     << ", "
              << "Lon1 = " << lonf1    << ", Dist1 = " << d1 << " nm, "
              << "Lon2 = " << lonf2    << ", Dist2 = " << d2 << " nm, "
              << "Dist = " << dland[n] << " nm\n";

      } // end for y
   } // end for x

   if(mlog.verbosity_level() >= 3) cout << "done.\n" << flush;

   // Write the computed distances to the output file
   mlog << Debug(3) << "Writing distance to land variable.\n";
   if(!dland_var->put(&dland[0], grid.ny(), grid.nx())) {
      mlog << Error << "\nprocess_distances() -> "
           << "error with dland_var->put\n\n";
      exit(1);
   }

   // Delete allocated memory
   if(dland) { delete dland; dland = (float *) 0; }

   // Close the output NetCDF file
   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   // List the output file
   mlog << Debug(1)
        << "Output NetCDF File:\t" << out_filename << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tout_file\n"
        << "\t[-grid spec]\n"
        << "\t[-noll]\n"
        << "\t[-land file]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"out_file\" is the NetCDF output file containing "
        << "the computed distances to land (required).\n"

        << "\t\t\"-grid spec\" overrides the default 1/10th degree "
        << "NW Hemisphere grid (optional).\n"
        << "\t\t   spec = lat_ll lon_ll delta_lat delta_lon n_lat n_lon\n"

        << "\t\t\"-land file\" overwrites the default land data file (\""
        << replace_path(default_land_data_file) << "\") (optional).\n"

        << "\t\t\"-noll\" skips writing the lat/lon variables in the "
        << "output NetCDF file to reduce the file size (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grid(const StringArray & a) {
   ConcatString filename;

   GridData.name      = "user_defined";
   GridData.lat_ll    = atof(a[0]);
   GridData.lon_ll    = -1.0*atof(a[1]); // switch from deg east to west
   GridData.delta_lat = atof(a[2]);
   GridData.delta_lon = atof(a[3]);
   GridData.Nlat      = atoi(a[4]);
   GridData.Nlon      = atoi(a[5]);
}

////////////////////////////////////////////////////////////////////////

void set_noll(const StringArray & a) {
   latlon_flag = false;
}

////////////////////////////////////////////////////////////////////////

void set_land(const StringArray & a) {
   land_data_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
