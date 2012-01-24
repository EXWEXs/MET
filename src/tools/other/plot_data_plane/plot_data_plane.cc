

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////
//
//   Filename:   plot_data_plane.cc
//
//   Description:
//      This tool creates a PostScript plot of a specified variable
//      from a given data file. It overlays the data image with a map.
//      The variable to plot is specified on the command line using
//      the config file magic string notation. The color table used
//      for the image can be specified on the command line. It
//      defaults to the met_default.ctable file in the MET data
//      directory. Optionally, the user may specify a title for the
//      plot on the command line. The user may also override the
//      default paper size of "Letter" and change it to "A4" by
//      setting the environment variable MET_PAPER_SIZE.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    12/19/11  Holmes          New
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
#include "vx_plot_util.h"
#include "data_plane_plot.h"


////////////////////////////////////////////////////////////////////////


static const char *program_name = "plot_data_plane";


////////////////////////////////////////////////////////////////////////


static ConcatString InputFilename;
static ConcatString OutputFilename;
static ConcatString MagicString;
static ConcatString ColorTableName;
static ConcatString TitleString;

static double PlotRangeMin = 0.0, PlotRangeMax = 0.0;


////////////////////////////////////////////////////////////////////////


static void process_command_line(int, char **);
static void usage();
static void set_colortable_name(const StringArray &);
static void set_plot_range(const StringArray &);
static void set_title_string(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv[])
{
   Met2dDataFile * met_ptr;
   Met2dDataFileFactory m_factory;
   VarInfo * var_ptr;
   VarInfoFactory v_factory;
   DataPlane data_plane;
   Grid grid;
   ColorTable color_table;
   double data_min, data_max;

      //
      // set the default color table
      //
   ColorTableName << MET_BASE << "/data/colortables/met_default.ctable";

      //
      // process the command line arguments
      //
   process_command_line(argc, argv);

      //
      // instantiate the Met2dDataFile object using the data_2d_factory
      // and the VarInfo object using the var_info_factory
      //
   met_ptr = m_factory.new_met_2d_data_file(InputFilename);

   if (!met_ptr)
   {
      mlog << Error << "\n" << program_name << " -> file \""
           << InputFilename << "\" not a valid data file\n\n";
      exit (1);
   }

   var_ptr = v_factory.new_var_info(met_ptr->file_type());

   if (!var_ptr)
   {
      mlog << Error << "\n" << program_name << " -> unable to determine filetype of \""
           << InputFilename << "\"\n\n";
      exit (1);
   }

      //
      // populate the var_info object from the magic string
      //
   var_ptr->set_magic(MagicString);

      //
      // open the file
      //
   if (!met_ptr->open(InputFilename))
   {
      mlog << Error << "\n" << program_name << " -> can't open file \""
           << InputFilename << "\"\n\n";
      exit (1);
   }

      //
      // get the data plane from the file for this VarInfo object
      //
   if (!met_ptr->data_plane(*var_ptr, data_plane))
   {
      mlog << Error << "\n" << program_name << " -> trouble getting \""
           << MagicString << "\" from file \"" << InputFilename << "\"\n\n";
      exit (1);
   }

      //
      // get the grid info from the Met2dDataFile object
      //
   grid = met_ptr->grid();

      //
      // read in the color table file and scale the color table to fit
      // the data
      //
   color_table.read(ColorTableName);

   if (is_eq(color_table.data_min(bad_data_double), 0.0) &&
       is_eq(color_table.data_max(bad_data_double), 1.0))
   {
         //
         // Need to rescale. First get the min and max values of the data.
         //
      data_plane.data_range(data_min, data_max);

         //
         // Next check if the user has given a data range to use.
         //
      if ((PlotRangeMin != 0.0) || (PlotRangeMax != 0.0))
      {
         data_min = PlotRangeMin;
         data_max = PlotRangeMax;
      }

      color_table.rescale(data_min, data_max, bad_data_double);
   }

      //
      // plot the image
      //
   data_plane_plot(InputFilename, OutputFilename, grid, TitleString, color_table, data_plane);

      //
      // done
      //
   exit (0);

}


////////////////////////////////////////////////////////////////////////


void process_command_line(int argc, char **argv)
{
   CommandLine cline;

      //
      // check for zero arguments
      //
   if (argc == 1)
      usage();

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
   cline.add(set_colortable_name, "-color_table", 1);
   cline.add(set_plot_range, "-plot_range", 2);
   cline.add(set_title_string, "-title", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);

      //
      // parse the command line
      //
   cline.parse();

      //
      // Check for error. There should be three arguments left:
      // the input filename, the output filename, and the magic
      // string.
      //
   if (cline.n() != 3)
      usage();

      //
      // store the filenames and magic string.
      //
   InputFilename = cline[0];
   OutputFilename = cline[1];
   MagicString = cline[2];

}


////////////////////////////////////////////////////////////////////////


void usage()
{
   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tinput_filename\n"
        << "\toutput_filename\n"
        << "\tmagic_string\n"
        << "\t[-color_table color_table_name]\n"
        << "\t[-plot_range min max]\n"
        << "\t[-title title_string]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"input_filename\" is the name of a "
        << " gridded data file to be plotted (required).\n"

        << "\t\t\"output_filename\" is the name of the output "
        << "PostScript file to be written (required).\n"

        << "\t\t\"magic_string\" defines the data to be plotted "
        << "from the input file (required).\n"

        << "\t\t\"-color_table color_table_name\" overrides the "
        << "default color table "
        << "(\"$(MET_BASE_DIR)/data/colortables/met_default.ctable\") "
        << "(optional).\n"

        << "\t\t\"-plot_range min max\" defines the range of the "
        << "data to be plotted (optional).\n"

        << "\t\t\"-title title_string\" specifies the plot title "
        << "string (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit (1);

}


////////////////////////////////////////////////////////////////////////


void set_colortable_name(const StringArray & a)
{
   ColorTableName = a[0];

}


////////////////////////////////////////////////////////////////////////


void set_title_string(const StringArray & a)
{
   TitleString = a[0];

}


////////////////////////////////////////////////////////////////////////


void set_plot_range(const StringArray & a)
{
   PlotRangeMin = atof(a[0]);
   PlotRangeMax = atof(a[1]);

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


