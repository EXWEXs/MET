// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   plot_point_obs.cc
//
//   Description:
//      This tool creates a postscript plot marking the locations of
//      point observations contained in the input NetCDF point
//      observation file.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    06-10-10  Halley Gotway   New
//   001    10/19/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//   002    01/03/12  Holmes          Modified to get a grid definition
//                                    from a data file to use for the
//                                    plot.
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

#include "vx_log.h"
#include "data_plane.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_data2d_grib.h"
#include "vx_data2d_nc_met.h"
#include "vx_data2d_nc_pinterp.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_color.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_render.h"
#include "vx_plot_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char  *program_name = "plot_point_obs";
static const Color  c_map(25, 25, 25);
static const double l_width = 0.5;

static const double margin_size = 36.0;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static Grid         grid("G004");
static Box          grid_bb;
static StringArray  ityp;
static IntArray     ivar;
static ConcatString data_plane_filename;

///////////////////////////////////////////////////////////////////////////////

static void draw_border(PSfile &, Box &);
static void usage();
static void set_grib_code(const StringArray &);
static void set_msg_type(const StringArray &);
static void set_data_filename(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   CommandLine cline;
   ConcatString nc_file, ps_file;
   ConcatString met_data_dir;
   char hdr_typ_str[max_str_len];
   char hdr_sid_str[max_str_len];
   char hdr_vld_str[max_str_len];
   float *hdr_arr, *obs_arr;
   int i, h, v, plot_count, skip_count;
   PSfile plot;
   Box page, view, map_box;
   double lat, lon, grid_x, grid_y, page_x, page_y, mag;
   IntArray ihdr;
   Color black;

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   black.set_gray(0);

   //
   // Initialize the grid bounding box to the whole grid
   //
   grid_bb.set_llwh( 0.0, 0.0, grid.nx(), grid.ny() );

   //
   // Initialize the data directory using MET_BASE
   //
   met_data_dir = replace_path(default_met_data_dir);

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_grib_code, "-gc", 1);
   cline.add(set_msg_type, "-msg_typ", 1);
   cline.add(set_data_filename, "-data_file", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left; the
   // netCDF point observation filename and the output PostScript
   // filename.
   //
   if (cline.n() != 2)
      usage();

   //
   // Store the input arguments
   //
   nc_file = cline[0];
   ps_file = cline[1];

   //
   // check if the user entered a data_plane_filename to use a specific
   // grid definition
   //
   if (data_plane_filename.length() > 0)
   {
     
      mlog << Debug(1) << "Retrieving grid from file: "
           << data_plane_filename << "\n";
     
      //
      // instantiate a Met2dDataFile object using the data_2d_factory
      // and get the grid info from the file.
      //
      Met2dDataFile * met_ptr;
      Met2dDataFileFactory m_factory;
      
      met_ptr = m_factory.new_met_2d_data_file(data_plane_filename);

      if (!met_ptr)
      {
         mlog << Error << "\n" << program_name << " -> file \""
              << data_plane_filename << "\" not a valid data file\n\n";
         exit (1);
      }

      grid = met_ptr->grid();

   } else {
      mlog << Debug(1) << "Using default global grid.\n";
   }

   //
   // Declare NetCDF file, dimensions, and variables
   //
   NcFile *f_in       = (NcFile *) 0;

   NcDim *hdr_arr_dim = (NcDim *) 0;
   NcDim *obs_arr_dim = (NcDim *) 0;
   NcDim *nhdr_dim    = (NcDim *) 0;
   NcDim *nobs_dim    = (NcDim *) 0;
   NcDim *strl_dim    = (NcDim *) 0;

   NcVar *hdr_arr_var = (NcVar *) 0;
   NcVar *hdr_typ_var = (NcVar *) 0;
   NcVar *hdr_sid_var = (NcVar *) 0;
   NcVar *hdr_vld_var = (NcVar *) 0;
   NcVar *obs_arr_var = (NcVar *) 0;

   //
   // Open the netCDF point observation file
   //
   mlog << Debug(1) << "Opening netCDF file: " << nc_file << "\n";

   f_in = new NcFile(nc_file);

   if(!f_in || !f_in->is_valid()) {
      mlog << Error << "\nmain() -> trouble opening netCDF file "
           << nc_file << "\n\n";
      f_in->close();
      delete f_in;
      f_in = (NcFile *) 0;

      exit(1);
   }

   //
   // Retrieve the dimensions and variable from the netCDF file
   //
   hdr_arr_dim = f_in->get_dim("hdr_arr_len");
   obs_arr_dim = f_in->get_dim("obs_arr_len");

   nhdr_dim = f_in->get_dim("nhdr");
   nobs_dim = f_in->get_dim("nobs");

   strl_dim = f_in->get_dim("mxstr");

   hdr_arr_var = f_in->get_var("hdr_arr");
   hdr_typ_var = f_in->get_var("hdr_typ");
   hdr_sid_var = f_in->get_var("hdr_sid");
   hdr_vld_var = f_in->get_var("hdr_vld");
   obs_arr_var = f_in->get_var("obs_arr");

   if(!hdr_arr_dim || !hdr_arr_dim->is_valid() ||
      !obs_arr_dim || !obs_arr_dim->is_valid() ||
      !nhdr_dim    || !nhdr_dim->is_valid()    ||
      !nobs_dim    || !nobs_dim->is_valid()    ||
      !strl_dim    || !strl_dim->is_valid()    ||
      !hdr_arr_var || !hdr_arr_var->is_valid() ||
      !hdr_typ_var || !hdr_typ_var->is_valid() ||
      !hdr_sid_var || !hdr_sid_var->is_valid() ||
      !hdr_vld_var || !hdr_vld_var->is_valid() ||
      !obs_arr_var || !obs_arr_var->is_valid()) {
      mlog << Error << "\nmain() -> "
           << "trouble reading netCDF file " << nc_file << "\n\n";
      exit(1);
   }

   //
   // Allocate space to store the data
   //
   hdr_arr = new float[hdr_arr_dim->size()];
   obs_arr = new float[obs_arr_dim->size()];

   mlog << Debug(1) << "Processing " << nobs_dim->size() << " observations at "
        << nhdr_dim->size() << " locations.\n";

   mlog << Debug(1) << "Observation GRIB codes: ";
   if(ivar.n_elements() == 0) mlog << "ALL\n";
   else {
      for(i=0; i<ivar.n_elements(); i++) mlog << ivar[i] << " ";
      mlog << "\n";
   }

   mlog << Debug(1) << "Observation message types: ";
   if(ityp.n_elements() == 0) mlog << "ALL\n";
   else {
      for(i=0; i<ityp.n_elements(); i++) mlog << ityp[i] << " ";
      mlog << "\n";
   }

   //
   // Setup the min/max lat/lon Bounding Box for the grid
   //
   grid_bb.set_llwh(0, 0, grid.nx(), grid.ny());

   //
   // Create a PostScript file
   //
   mlog << Debug(1) << "Creating postscript file: " << ps_file << "\n";
   plot.open(ps_file, OrientationLandscape);
   plot.pagenumber(1);
   plot.setlinewidth(l_width);

   //
   // Set plotting dimensions
   //
   page.set_llwh(0.0, 0.0, plot.page_width(), plot.page_height());

   view.set_llwh(margin_size, margin_size, page.width() - 2.0 * margin_size, page.height() - 3.0 * margin_size);

      //
      // calculate how much to magnify the map to get it to fill the view box
      // without distorting the map. e.g. it will either bump the top and bottom
      // of the view box or bump the left and right sides of the view box or both.
      //

   mag = calc_mag(grid_bb, view);

   map_box.set_llwh(view.left() + 0.5 * view.width() - 0.5 * mag * grid_bb.width(),
                    view.bottom() + 0.5 * view.height() - 0.5 * mag * grid_bb.height(),
                    mag * grid_bb.width(), mag * grid_bb.height());

   //
   // annotate the plot with the filename
   //
   plot.choose_font(31, 24.0);
   plot.write_centered_text(1, 1, 0.5 * page.width(), map_box.top() + margin_size, 0.5, 0.5,
                            get_short_name(nc_file));

   //
   // draw the map first and then put a border around it
   //
   plot.setrgbcolor(0.0, 0.0, 0.0);
   draw_map(grid, grid_bb, plot, map_box, black, met_data_dir);
   draw_border(plot, map_box);
   
   //
   // set up clipping path so that the circles will not be made
   // outside the map box
   //
   plot.gsave();
   plot.newpath();
   plot.moveto(map_box.x_ll(), map_box.y_ll());
   plot.lineto(map_box.x_ur(), map_box.y_ll());
   plot.lineto(map_box.x_ur(), map_box.y_ur());
   plot.lineto(map_box.x_ll(), map_box.y_ur());
   plot.clip();

   //
   // Loop through the observations looking for the correct observation
   // variable type.
   //
   plot.setrgbcolor(1.0, 0.0, 0.0);
   plot_count = skip_count = 0;
   for(i=0; i<nobs_dim->size(); i++) {

      if(!obs_arr_var->set_cur(i, 0) ||
         !obs_arr_var->get(obs_arr, 1, obs_arr_dim->size())) {
         mlog << Error << "\nmain() -> trouble getting obs_arr\n\n";
         exit(1);
      }
      if(obs_arr[0] >= 1.0E10 && obs_arr[1] >= 1.0E10) break;

      //
      // Get the header index and variable type for this observation.
      //
      h = nint(obs_arr[0]);
      v = nint(obs_arr[1]);

      //
      // Check if we want to plot this variable type.
      //
      if(ivar.n_elements() > 0 && !ivar.has(v)) continue;

      //
      // Get the corresponding header message type
      //
      if(!hdr_typ_var->set_cur(h-1, 0) ||
         !hdr_typ_var->get(hdr_typ_str, 1, strl_dim->size())) {
         mlog << Error << "\nmain() -> "
              << "trouble getting hdr_typ\n\n";
         exit(1);
      }

      //
      // Get the corresponding header station id
      //
      if(!hdr_sid_var->set_cur(h-1, 0) ||
         !hdr_sid_var->get(hdr_sid_str, 1, strl_dim->size())) {
         mlog << Error << "\nmain() -> "
              << "trouble getting hdr_sid\n\n";
         exit(1);
      }

      //
      // Get the corresponding header valid time
      //
      if(!hdr_vld_var->set_cur(h-1, 0) ||
         !hdr_vld_var->get(hdr_vld_str, 1, strl_dim->size())) {
         mlog << Error << "\nmain() -> "
              << "trouble getting hdr_vld\n\n";
         exit(1);
      }

      //
      // Check if we want to plot this message type.
      //
      if(ityp.n_elements() > 0 && !ityp.has(hdr_typ_str)) continue;

      //
      // Only plot a circle if one hasn't been plotted for this
      // location yet.
      //
      if(!ihdr.has(h)) {

         //
         // Get the header for this observation
         //
         hdr_arr_var->set_cur(h-1, 0);
         hdr_arr_var->get(hdr_arr, 1, hdr_arr_dim->size());

         if(hdr_arr[0] >= 1.0E10 && hdr_arr[1] >= 1.0E10) break;

         lat = (double) hdr_arr[0];
         lon = (double) (-1.0*hdr_arr[1]);

         //
         // Convert lat/lon to grid x/y
         //
         grid.latlon_to_xy(lat, lon, grid_x, grid_y);

         //
         // If the current point is off the grid, increment the skip count
         //
         if(grid_x < 0 || grid_x >= grid.nx() ||
            grid_y < 0 || grid_y >= grid.ny()) {
            skip_count++;
            ihdr.add(h);
            continue;
         }

         //
         // Convert grid x/y to page x/y
         //
         gridxy_to_pagexy(grid, grid_bb, grid_x, grid_y, page_x, page_y, map_box);

         //
         // Draw a circle at this point and increment the plot count
         //
         plot.circle(page_x, page_y, 0.5, 0);
         plot.fill();

         //
         // Dump out the location being plotted
         //
         mlog << Debug(3)
              << "[" << plot_count + 1
              << "] Plotting location [ type, sid, valid, lat, lon, elevation ] = [ "
              << hdr_typ_str << ", " << hdr_sid_str << ", " << hdr_vld_str << ", "
              << hdr_arr[0] << ", " << hdr_arr[1] << ", " << hdr_arr[2] << " ]\n";
         
         ihdr.add(h);
         plot_count++;
      }
   } // end for i
   plot.grestore();

   mlog << Debug(2)
        << "Finished plotting " << plot_count << " locations.\n"
        << "Skipped " << skip_count << " locations off the grid.\n";

   plot.showpage();

   plot.close();

   //
   // Deallocate memory and clean up
   //
   if(f_in)    { f_in->close(); delete f_in; f_in = (NcFile *) 0; }
   if(hdr_arr) { delete hdr_arr; hdr_arr = (float *) 0; }
   if(obs_arr) { delete obs_arr; obs_arr = (float *) 0; }

   return(0);
}

////////////////////////////////////////////////////////////////////////

static void draw_border(PSfile &p, Box &dim) {

   p.gsave();
   p.setlinewidth(l_width);
   p.newpath();
   p.moveto(dim.x_ll(), dim.y_ll());
   p.lineto(dim.x_ur(), dim.y_ll());
   p.lineto(dim.x_ur(), dim.y_ur());
   p.lineto(dim.x_ll(), dim.y_ur());
   p.closepath();
   p.stroke();
   p.grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: " << program_name << "\n"
        << "\tnc_file\n"
        << "\tps_file\n"
        << "\t[-gc code]\n"
        << "\t[-msg_typ name]\n"
        << "\t[-data_file name]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"nc_file\" is the netCDF point observation file "
        << "to be plotted.\n"
        << "\t\t\"ps_file\" is the name for the output postscript "
        << "file to be generated.\n"
        << "\t\t\"-gc code\" specifies the GRIB code(s) to be plotted "
        << "(optional).\n"
        << "\t\t\"-msg_typ name\" is the message type(s) to be "
        << "plotted (optional).\n"
        << "\t\t\"-data_file name\" specifies a data file whose grid "
        << "should be used for the plot (optional).\n"
        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grib_code(const StringArray & a)
{
   ivar.add(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

void set_msg_type(const StringArray & a)
{
   ityp.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_data_filename(const StringArray & a)
{
   data_plane_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

