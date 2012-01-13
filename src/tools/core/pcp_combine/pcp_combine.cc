
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   pcp_combine.cc
//
//   Description:
//      Based on user specified time periods, this tool combines
//      one or more grib precipitation files into a single
//      precipitation file and will dump out the output in NetCDF
//      format.
//
//      The user must specify the following on the command line:
//      init_time_time, in_accum_time, valid_time, out_accum_time,
//      and out_filename
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-24-07  Halley Gotway  New
//
//   001    12-07-07  Halley Gotway  Change time format from
//                                   YYYY-MM-DD_HH:MM:SS to YYYYMMDD_HHMMSS
//
//   002    01-31-08  Halley Gotway  Add support for the -add and
//                                   -subtract options
//
//   003    09/23/08  Halley Gotway  Change argument sequence for the
//                                   GRIB record access routines.
//
//   004    02/20/09  Halley Gotway  Append _HH to the variable name
//                                   for non-zero accumulation times.
//
//   005    12/23/09  Halley Gotway  Call the library read_pds routine.
//
//   006    05/21/10  Halley Gotway  Enhance to search multiple
//                                   -pcp_dir directory arguments.
//
//   007    06/25/10  Halley Gotway  Allow times to be specified in
//                                   HH[MMSS] and YYYYMMDD[_HH[MMSS]] format.
//
//   008    06/30/10  Halley Gotway  Enhance grid equality checks.
//
//   009    07/27/10  Halley Gotway  Enhance to allow addition of any
//                                   number of input files/accumulation intervals.
//                                   Add lat/lon variables to NetCDF.
//
//   010    04/19/11  Halley Gotway  Bugfix for -add option.
//
//   011    10/20/11  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//
//   012    11/14/11  Halley Gotway  Bugfix for -add option when
//                                   when handling missing data values.
//
//   013    12/21/11  Bullock        Ported to new repository.
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
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "grib_classes.h"
// #include "vx_gdata.h"
// #include "vx_met_util.h"
#include "vx_nc_util.h"
#include "vx_grid.h"
#include "vx_statistics.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

static const char zero_time_str   [] = "00000000_000000";
static const char default_pcp_dir [] = ".";
static const char default_reg_exp [] = ".*";

// Run Command enumeration
enum RunCommand { sum = 0, add = 1, sub = 2 };

// Variables for top-level command line arguments
static RunCommand run_command = sum;
static int grib_code = apcp_grib_code;
static int grib_ptv  = 2;
// static GCInfo gc_info;
static int verbosity = 1;

// Variables common to all commands
static ConcatString out_filename;
// static float       *pcp_data = (float *) 0;

// Variables for the sum command
static unixtime     init_time;
static int          in_accum;
static unixtime     valid_time;
static int          out_accum;
static StringArray  pcp_dir;
static ConcatString pcp_reg_exp = default_reg_exp;

// Variables for the add and subtract commands
static ConcatString *in_file = (ConcatString *) 0;
static int          *accum = (int *) 0;
static int           n_files;

///////////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);

static void process_sum_args(const CommandLine &);
static void process_add_sub_args(const CommandLine &);

static void do_sum_command();
static void do_add_command();
static void do_sub_command();

static void sum_grib_files(Grid &, DataPlane &);
static int  search_pcp_dir(const char *, const unixtime, ConcatString &);
static void check_file_time(const char *, unixtime, int &);

static void get_field(const char * filename, 
                      int get_accum, unixtime & init_ut, unixtime & valid_ut, 
                      Grid & grid, 
                      DataPlane & plane);

static void write_netcdf(unixtime, unixtime, int, const Grid &, const DataPlane &);

static bool is_timestring(const char *);

static ConcatString make_magic(int gribcode, int ptv, int accum_interval);

static void usage();
static void set_sum(const StringArray &);
static void set_add(const StringArray &);
static void set_subtract(const StringArray &);
static void set_grib_code(const StringArray &);
static void set_ptv_num(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_pcpdir(const StringArray &);
static void set_pcprx(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])

{

program_name = get_short_name(argv[0]);

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Process the command line arguments
   //
   process_command_line(argc, argv);

   //
   // Perform the requested job command
   //
   if     (run_command == sum) do_sum_command();
   else if(run_command == add) do_add_command();
   else                        do_sub_command();

   return(0);
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
   // Default to running the sum command
   //
   run_command = sum;

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
   cline.add(set_sum,       "-sum",      0);
   cline.add(set_add,       "-add",      0);
   cline.add(set_subtract,  "-subtract", 0);

   cline.add(set_grib_code, "-gc",       1);
   cline.add(set_ptv_num,   "-ptv",      1);
   cline.add(set_logfile,   "-log",      1);
   cline.add(set_verbosity, "-v",        1);
   cline.add(set_pcpdir,    "-pcpdir",   1);
   cline.add(set_pcprx,     "-pcprx",    1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // set the verbosity level
   //
   mlog.set_verbosity_level(verbosity);

   //
   // Check for error. Depending on the type of command, there should
   // be a different number of arguments left. For the sum command
   // there should be five arguments left: the init_time, the in_accum,
   // the valid_time, the out_accum, and the out_file. For the add
   // command there should be at least three arguments left: in_file1,
   // accum1, [in_file2, accum2, ..., in_filen, accumn], and the
   // out_file. For the subtract command there should be five
   // arguments left: in_file1, accum1, in_file2, accum2, and
   // out_file.
   //
   if (run_command == sum)
   {
      if (cline.n() != 5)
         usage();

   }
   else if (run_command == add)
   {
      if (cline.n() < 3)
         usage();

   }
   else
   {
      if (cline.n() != 5)
         usage();

   }

   //
   // Process the specific command arguments
   //
   if(run_command == sum) process_sum_args(cline);
   else                   process_add_sub_args(cline);

   //
   // If pcp_dir is not set, set it to the current directory.
   //
   if(pcp_dir.n_elements() == 0) pcp_dir.add(default_pcp_dir);

   //
   //  done
   //

   return;
}

////////////////////////////////////////////////////////////////////////

void process_sum_args(const CommandLine & cline)

{

   //
   // Parse the sum arguments
   //

   //
   // Init time
   //
   if(strcmp(cline[0], zero_time_str) == 0) {
      init_time = (unixtime) 0;
   }
   else {
      init_time = timestring_to_unix(cline[0]);
   }

   //
   // Input accumulation
   //
   in_accum = timestring_to_sec(cline[1]);

   //
   // Valid time
   //
   if(strcmp(cline[2], zero_time_str) == 0) {
      valid_time = (unixtime) 0;
   }
   else {
      valid_time = timestring_to_unix(cline[2]);
   }

   //
   // Output accumulation
   //
   out_accum = timestring_to_sec(cline[3]);

   //
   // Out file
   //
   out_filename = cline[4];

   //
   // Check that accumulation intervals are greater than zero
   //
   if(in_accum <= 0 || out_accum <= 0) {
      mlog << Error << "\n\n  process_sum_args() -> "
           << "The input accumulation interval (" << cline[1]
           << ") and output accumulation interval (" << cline[3]
           << ") must be greater than zero.\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void process_add_sub_args(const CommandLine & cline)

{

   int i;

   //
   // Figure out the number of files provided
   //
   for(i=0, n_files=0; i<(cline.n() - 1); i+=2) { // Only check accumulation interval for enough args

      if ( ! is_timestring(cline[i + 1]) )  break;

      n_files++; // Increment file count

   } // end for i

   //
   // Allocate memory for file names and accumulations
   //
   in_file = new ConcatString [n_files];
   accum   = new int [n_files];

   //
   // Store the input files and accumulations
   //
   for(i=0; i<n_files; i++) {
      in_file[i] = cline[i*2];
      accum[i]   = timestring_to_sec(cline[i*2 + 1]);
   }

   //
   // Store the output file
   //
   out_filename = cline[n_files*2];

   return;
}

////////////////////////////////////////////////////////////////////////

void do_sum_command()

{

   DataPlane plane;
   Grid grid;
   int lead_time;
   ConcatString init_time_str, valid_time_str;
   ConcatString in_accum_str, out_accum_str;
   ConcatString lead_time_str;

   //
   // Compute the lead time
   //
   lead_time = valid_time - init_time;

   //
   // Build time strings
   //
   if(init_time != 0) init_time_str = unix_to_yyyymmdd_hhmmss(init_time);
   else               init_time_str = zero_time_str;

   in_accum_str   = sec_to_hhmmss(in_accum);
   valid_time_str = unix_to_yyyymmdd_hhmmss(valid_time);
   out_accum_str  = sec_to_hhmmss(out_accum);
   lead_time_str  = sec_to_hhmmss(lead_time);

   mlog << Debug(2)
        << "Performing sum command: "
        << "Init/In_Accum/Valid/Out_Accum Times = "
        << init_time_str  << "/" << in_accum_str  << "/"
        << valid_time_str << "/" << out_accum_str << "\n";
   //
   // Check that the output accumulation time is not greater than
   // the lead time, except when init_time = 0 for observations.
   //
   if(out_accum > lead_time && init_time != (unixtime) 0) {
      mlog << Error << "\n\n  do_sum_command() -> "
           << "The output accumulation time (" << out_accum_str
           << ") cannot be greater than the lead time ("
           << lead_time_str << ").\n\n";
      exit(1);
   }

   //
   // Check that the output accumulation time is divisible by the input
   // accumulation time.
   //
   if(out_accum%in_accum != 0) {
      mlog << Error << "\n\n  do_sum_command() -> "
           << "The output accumulation time (" << out_accum_str
           << ") must be divisible by the input accumulation "
           << "time (" << in_accum_str << ").\n\n";
      exit(1);
   }

   //
   // Check that the lead time is divisible by the the input
   // accumulation time except when init_time = 0 for observations.
   //
   if(lead_time%in_accum != 0 && init_time != (unixtime) 0) {
      mlog << Error << "\n\n  do_sum_command() -> "
           << "The lead time (" << lead_time_str
           << ") must be divisible by the input accumulation time ("
           << in_accum_str << ").\n\n";
      exit(1);
   }

   //
   // Find and sum up the matching precipitation files
   //
   sum_grib_files(grid, plane);

   //
   // Write the combined precipitation field out in NetCDF format
   //
   mlog << Debug(1)
        << "Writing output file: " << out_filename << "\n";

   write_netcdf(init_time, valid_time, out_accum, grid, plane);

   return;
}

////////////////////////////////////////////////////////////////////////

void sum_grib_files(Grid & grid, DataPlane & plane)

{

   int i, j, x, y;
   ConcatString valid_str;
   ConcatString in_accum_str, out_accum_str;
   DataPlane part;
   double v_sum, v_part;
   Grid gr;
   unixtime     * pcp_times = (unixtime) 0;
   int          * pcp_recs  = (int *) 0;
   ConcatString * pcp_files = (ConcatString *) 0;

   //
   // Build time strings
   //
   in_accum_str  = sec_to_hhmmss(in_accum);
   out_accum_str = sec_to_hhmmss(out_accum);

   //
   // Compute the number of forecast precipitation files to be found,
   // and allocate memory to store their names and times
   //
   n_files   = out_accum/in_accum;
   pcp_times = new unixtime [n_files];
   pcp_recs  = new int [n_files];
   pcp_files = new ConcatString [n_files];

   mlog << Debug(2)
        << "Searching for " << n_files << " files "
        << "with accumulation times of " << in_accum_str
        << " to sum to a total accumulation time of "
        << out_accum_str << ".\n";

   //
   // Compute the valid times for the precipitation files
   // to be found.
   //
   for(i=0; i<n_files; i++) {
      pcp_times[i] = valid_time - i*in_accum;
   }

   //
   // Search for each file time.
   //
   for(i=0; i<n_files; i++) {

      //
      // Search in each directory for the current file time.
      //
      for(j=0; j<pcp_dir.n_elements(); j++) {

         pcp_recs[i] = search_pcp_dir(pcp_dir[j], pcp_times[i], pcp_files[i]);

         if( pcp_recs[i] != -1 )  {

            valid_str = unix_to_yyyymmdd_hhmmss(pcp_times[i]);

            mlog << Debug(1)
                 << "[" << (i+1) << "] File " << pcp_files[i]
                 << " matches valid time of " << valid_str
                 << "\n";

            break;

         } // if

      } // end for j

      //
      // Check for no matching file found
      //
      if(pcp_recs[i] == -1) {

         valid_str = unix_to_yyyymmdd_hhmmss(pcp_times[i]);

         mlog << Error << "\n\n  sum_grib_files() -> "
              << "Cannot find a file with a valid time of "
              << valid_str << " and accumulation time of "
              << in_accum_str << " matching the regular "
              << "expression \"" << pcp_reg_exp << "\"\n\n";
         exit(1);
      }

   } // end for i

    /////////////////////////////

   //
   // Open each of the files found and parse the data.
   //
   for(i=0; i<n_files; i++) {

      mlog << Debug(1)
           << "[" << (i+1) << "] Reading input file: " << pcp_files[i]
           << "\n";

      //
      // Read data for the file.
      //
      get_field(pcp_files[i], in_accum, init_time, pcp_times[i], gr, part);

      //
      // For the first file processed store the grid, allocate memory
      // to store the precipitation sums, and initialize the sums
      //
      if ( i == 0 )  {

         grid = gr;

         plane = part;

      } else {

         //
         // Check to make sure the grid stays the same
         //
         if(!(grid == gr)) {
            mlog << Error << "\n\n  sum_grib_files() -> "
                 << "The grid must remain the same for all "
                 << "data files.\n\n";
            exit(1);
         }

         //
         // Increment the precipitation sums keeping track of the bad data values
         //
         for(x=0; x<grid.nx(); x++) {

            for(y=0; y<grid.ny(); y++) {

               v_sum = plane(x, y);

               if ( is_bad_data(v_sum) )  continue;

               v_part = part(x, y);

               if ( is_bad_data(v_part) ) {

                  plane.set(bad_data_double, x, y);

                  continue;

               }

               plane.set(v_sum + v_part, x, y);

            }   //  for y

         }   //  for x

      } // end else

   } // end for i

   //
   // Deallocate any memory that was allocated above
   //
   if(pcp_files) { delete [] pcp_files; pcp_files = (ConcatString *) 0; }
   if(pcp_times) { delete [] pcp_times; pcp_times = (unixtime *) 0; }
   if(pcp_recs ) { delete [] pcp_recs;  pcp_recs  = (int *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

int search_pcp_dir(const char *cur_dir, const unixtime cur_ut, ConcatString & cur_file)

{

   int i_rec;
   struct dirent *dirp = (struct dirent *) 0;
   DIR *dp = (DIR *) 0;

   //
   // Find the files matching the specified regular expression with
   // the correct valid and accumulation times.
   //
   if((dp = opendir(cur_dir)) == NULL ) {
      mlog << Error << "\n\n  search_pcp_dir() -> "
           << "Cannot open precipitation directory "
           << cur_dir << "\n\n";
      exit(1);
   }

   //
   // Initialize the record index to not found.
   //
   i_rec = -1;

   //
   // Process each file contained in the directory.
   //
   while((dirp = readdir(dp)) != NULL) {

      //
      // Ignore any hidden files.
      //
      if(dirp->d_name[0] == '.') continue;

      //
      // Check the file name for a matching regular expression.
      //
      if(check_reg_exp(pcp_reg_exp, dirp->d_name) == true) {

         //
         // Check the current file for matching initialization,
         // valid, lead, and accumulation times.
         //
         cur_file << cs_erase << cur_dir << '/' << dirp->d_name;
         check_file_time(cur_file, cur_ut, i_rec);

         if(i_rec != -1) break;
      } // end if
   } // end while

   if(closedir(dp) < 0) {
      mlog << Error << "\n\n  search_pcp_dir() -> "
           << "Cannot close precipitation directory "
           << cur_dir << "\n\n";
      exit(1);
   }

   return(i_rec);
}

////////////////////////////////////////////////////////////////////////

void check_file_time(const char *file, unixtime pcp_valid, int &i_gc)

{

   GribFile grib_file;
   GribRecord rec;
   int i, bms_flag, file_accum;
   unixtime file_init, file_valid;

   i_gc = -1;

   //
   // Open the precipitation grib file
   //
   if( !(grib_file.open(file)) ) {
      mlog << Warning << "\n\n  check_file_time() -> "
           << "can't open precipitation grib file: "
           << file << "\n\n";
      return;
   }

   //
   // Find the grib record containing the accumulated precipitation.
   // A Grib file may contain multiple records of accumulated
   // precipitation.  Search grid records for one with the expected
   // accumulation interval.
   //
   for(i=0; i< grib_file.n_records(); i++) {

      if(grib_file.gribcode(i) != grib_code)  continue;

      //
      // Read the record containing accumulated precip
      //
      grib_file.seek_record(i);
      grib_file >> rec;

      //
      // Read the Product Description Section
      //
      read_pds(rec, bms_flag, file_init, file_valid, file_accum);

      //
      // The file matches if the issuance times, valid times, and
      // accumulation times match
      //
      if((file_init == init_time || init_time == (unixtime) 0) &&
         (file_valid == pcp_valid) &&
         (file_accum == in_accum)) {
         i_gc = i;
         break;
      }

   } // end for

   grib_file.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void do_add_command()

{

   Grid grid1, grid2;
   DataPlane total, part;
   double total_value, part_value;
   unixtime init_time1, init_time2;
   unixtime valid_time1, valid_time2;
   unixtime nc_init_time, nc_valid_time;
   int i, x, y, nc_accum;
   ConcatString init_time1_str, init_time2_str;
   ConcatString valid_time1_str, valid_time2_str;
   ConcatString accum1_str, accum2_str;

   mlog << Debug(2)
        << "Performing addition command for " << n_files
        << " files.\n";

   //
   //  initialize some stuff with info from the first file
   //

i = 0;

   //
   // Read current field
   //
   mlog << Debug(1) 
        << "Reading input file: " << in_file[0] << "\n";

   get_field(in_file[i], accum[i], init_time1, valid_time1, grid1, total);

   //
   // Build time strings
   //
   init_time1_str  = unix_to_yyyymmdd_hhmmss(init_time1);
   valid_time1_str = unix_to_yyyymmdd_hhmmss(valid_time1);
   accum1_str      = sec_to_hhmmss(accum[i]);

   // Initialize output times
   nc_init_time  = init_time1;
   nc_valid_time = valid_time1;
   nc_accum      = accum[i];

   //
   // Loop through the rest of the input files
   //
   for(i=1; i<n_files; i++) {   //  i starts at one here, not zero

      //
      // Read current field
      //
      mlog << Debug(1) << "Reading input file: " << in_file[i] << "\n";

      get_field(in_file[i].text(), accum[i], init_time2, valid_time2, grid2, part);

      //
      // Build time strings
      //
      init_time2_str  = unix_to_yyyymmdd_hhmmss(init_time2);
      valid_time2_str = unix_to_yyyymmdd_hhmmss(valid_time2);
      accum2_str      = sec_to_hhmmss(accum[i]);

      //
      // Check for the same grid dimensions
      //
      if( grid1 != grid2 ) {
         mlog << Error << "\n\n  do_add_command() -> "
              << "the two input fields must be on the same grid.\n\n";
         exit(1);
      }

      // Output init time
      if(nc_init_time != init_time2) nc_init_time = (unixtime) 0;

      // Output valid time
      if(nc_valid_time < valid_time2) nc_valid_time = valid_time2;

      // Output accumulation time
      nc_accum += accum[i];

      //
      // Increment sums for each grid point
      //

      total.set_constant(bad_data_float);

      for(x=0; x<(part.nx()); x++) {

         for(y=0; y<(part.ny()); y++) {

            total_value = total(x, y);

            if ( is_bad_data(total_value) )  continue;

            part_value = part(x, y);

            if ( is_bad_data(part_value) )  continue;

            total.set(total_value + part_value, x, y);

         } // end for y

      } // end for x

   } // end for i

   //
   // Write the combined precipitation field out in NetCDF format
   //
   mlog << Debug(1) 
        << "Writing output file: " << out_filename << "\n";

   write_netcdf(nc_init_time, nc_valid_time, nc_accum, grid1, total);

   return;

}

////////////////////////////////////////////////////////////////////////

void do_sub_command()

{

   DataPlane plus, minus, difference;
   Grid grid1, grid2;
   unixtime init_time1, init_time2;
   unixtime valid_time1, valid_time2;
   unixtime nc_init_time, nc_valid_time;
   int x, y, nc_accum;
   double v_plus, v_minus;
   ConcatString init_time1_str, init_time2_str;
   ConcatString valid_time1_str, valid_time2_str;
   ConcatString accum1_str, accum2_str;

   //
   // Check for exactly two input files
   //
   if(n_files != 2) {
      mlog << Error << "\n\n  do_sub_command() -> "
           << "you must specify exactly two input files for subtraction.\n\n";
      exit(1);
   }

   //
   // Read the two specified Grib files
   //
   mlog << Debug(1) 
        << "Reading input file: " << in_file[0] << "\n";

   get_field(in_file[0], accum[0], init_time1, valid_time1, grid1, plus);

   mlog << Debug(1) 
        << "Reading input file: " << in_file[1] << "\n";

   get_field(in_file[1], accum[1], init_time2, valid_time2, grid2, minus);

   //
   // Build time strings
   //
   init_time1_str  = unix_to_yyyymmdd_hhmmss(init_time1);
   init_time2_str  = unix_to_yyyymmdd_hhmmss(init_time2);
   valid_time1_str = unix_to_yyyymmdd_hhmmss(valid_time1);
   valid_time2_str = unix_to_yyyymmdd_hhmmss(valid_time2);
   accum1_str      = sec_to_hhmmss(accum[0]);
   accum2_str      = sec_to_hhmmss(accum[1]);

   //
   // Check for the same grid dimensions
   //
   if( grid1 != grid2 ) {
      mlog << Error << "\n\n  do_sub_command() -> "
           << "the two input fields must be on the same grid.\n\n";
      exit(1);
   }

   //
   // Compute output accumulation, initialization, and valid times
   // for the subtract command.
   //
   mlog << Debug(2) << "Performing subtraction command.\n";

   //
   // Output valid time
   //
   nc_valid_time = valid_time1;

   //
   // Output initialization time
   // Error if init_time1 != init_time2.
   //
   if(init_time1 != init_time2) {
      mlog << Error << "\n\n  do_sub_command() -> "
           << "init_time1 (" << init_time1_str
           <<  ") must be equal to init_time2 (" << init_time2_str
           << ") for subtraction.\n\n";
      exit(1);
   }
   nc_init_time = init_time1;

   //
   // Output accumulation time
   // Error if accum1 < accum2.
   //
   if(accum[0] < accum[1]) {
      mlog << Error << "\n\n  do_sub_command() -> "
           << "accum1 (" << accum1_str
           <<  ") must be greater than accum2 ("
           << accum2_str << ") for subtraction.\n\n";
      exit(1);
   }
   nc_accum = accum[0] - accum[1];

   //
   // Allocate space to store the differences
   //
   difference.set_size(plus.nx(), plus.ny());

   //
   // Perform the specified command for each grid point
   //

   difference.set_constant(bad_data_float);

   for(x=0; x<(plus.nx()); x++) {

      for(y=0; y<(plus.ny()); y++) {

         v_plus = plus(x, y);

         if ( is_bad_data(v_plus) )  continue;

         v_minus = minus(x, y);

         if ( is_bad_data(v_minus) )  continue;

         difference.set(v_plus - v_minus, x, y);

      } // end for y

   } // end for x

   //
   // Write the combined precipitation field out in NetCDF format
   //
   mlog << Debug(1) << "Writing output file: " << out_filename << "\n";
   write_netcdf(nc_init_time, nc_valid_time, nc_accum, grid1, difference);

   return;

}

////////////////////////////////////////////////////////////////////////

void get_field(const char * filename, 
               int get_accum, unixtime & init_ut, unixtime & valid_ut, 
               Grid & grid, 
               DataPlane & plane)

{

Met2dDataFileFactory factory;
Met2dDataFile * datafile = (Met2dDataFile *) 0;
VarInfoGrib var;

datafile = factory.new_met_2d_data_file(filename);

if ( !datafile )  {

   mlog << Error << "\n\n  get_field() -> "
        << "can't open data file \"" << filename
        << "\"\n\n";

   exit ( 1 );

}

var.set_magic(make_magic(grib_code, grib_ptv, get_accum));

var.set_valid  (valid_ut);
var.set_init   (init_ut);

if ( ! datafile->data_plane(var, plane) )  {

   mlog << Error << "\n\n  get_field() -> "
        << "can't get data plane from file \"" << filename
        << "\"\n\n";

   exit ( 1 );
}

grid = datafile->grid();

   //
   //  done
   //

if ( datafile )  { delete datafile;  datafile = (Met2dDataFile *) 0; }

return;

}

////////////////////////////////////////////////////////////////////////

void write_netcdf(unixtime nc_init, unixtime nc_valid, int nc_accum,
                  const Grid &grid, const DataPlane & plane)

{

   ConcatString var_str;
   ConcatString tmp_str, tmp2_str;
   ConcatString command_str;
   ConcatString accum1_str, accum2_str;

   NcFile *f_out   = (NcFile *) 0;
   NcDim  *lat_dim = (NcDim *)  0;
   NcDim  *lon_dim = (NcDim *)  0;
   NcVar  *pcp_var = (NcVar *)  0;

   // Create a new NetCDF file and open it.
   f_out = new NcFile(out_filename, NcFile::Replace);

   if(!f_out->is_valid()) {
      mlog << Error << "\n\n  write_netcdf() -> "
           << "trouble opening output file " << out_filename
           << "\n\n";
      f_out->close();
      delete f_out;  f_out = (NcFile *) 0;

      exit(1);
   }

   // Add global attributes
   write_netcdf_global(f_out, out_filename, program_name);

   if(run_command == sum) {

      accum1_str = sec_to_hhmmss(in_accum);

      command_str << cs_erase 
                  << "Sum: " << n_files << " files with accumulations of " 
                  << accum1_str << '.';

   } else if(run_command == add) {

      accum1_str = sec_to_hhmmss(in_accum);

      command_str << cs_erase 
                  << "Addition: " << n_files << " files.";

   }
   else { // run_command == subtract

      accum1_str = sec_to_hhmmss(accum[0]);

      accum2_str = sec_to_hhmmss(accum[1]);

      command_str << cs_erase
                  << "Subtraction: "
                  << in_file[0]
                  << " with accumulation of "
                  << accum1_str
                  << " minus "
                  << in_file[1]
                  << " with accumulation of "
                  << accum2_str << '.';

   }

   f_out->add_att("RunCommand", (const char *) command_str);

   //
   // Add the projection information
   //
   write_netcdf_proj(f_out, grid);

   // Define Dimensions
   lat_dim = f_out->add_dim("lat", (long) grid.ny());
   lon_dim = f_out->add_dim("lon", (long) grid.nx());

   // Add the lat/lon variables
   write_netcdf_latlon(f_out, lat_dim, lon_dim, grid);

   // Define a name for the variable
   // If the accumulation time is non-zero, append it to the variable name
   tmp_str = get_grib_code_abbr(grib_code, grib_ptv);

   // For no accumulation interval, append nothing
   if(nc_accum <= 0) {
      var_str = tmp_str;
   }
   // For an hourly accumulation interval, append _HH
   else if(nc_accum % sec_per_hour == 0) {
      var_str.set_precision(2);
      var_str << cs_erase << tmp_str << '_' << HH(nc_accum/sec_per_hour);
   }
   // For any other accumulation interval, append _HHMMSS
   else {
      tmp2_str = sec_to_hhmmss(nc_accum);
      var_str << cs_erase << tmp_str << '_' << tmp2_str;
   }

   // Define Variable
   pcp_var = f_out->add_var((const char *) var_str, ncFloat, lat_dim, lon_dim);

   // Add variable attributes
   var_str = get_grib_code_abbr(grib_code, grib_ptv);
   pcp_var->add_att("name",  (const char *) var_str);
   var_str = get_grib_code_name(grib_code, grib_ptv);
   pcp_var->add_att("long_name", (const char *) var_str);

   // Ouput level string
   if(nc_accum%sec_per_hour == 0) {
      var_str << cs_erase << 'A' << (nc_accum/sec_per_hour);
   } else {
      var_str << cs_erase << 'A' << sec_to_hhmmss(nc_accum);
   }

   pcp_var->add_att("level", (const char *) var_str);

   var_str = get_grib_code_unit(grib_code, grib_ptv);
   pcp_var->add_att("units",      (const char *) var_str);
   pcp_var->add_att("grib_code",  grib_code);
   pcp_var->add_att("_FillValue", bad_data_float);

   //
   // Add initialization, valid, and accumulation time info as attributes to
   // the pcp_var
   //
   if(nc_init == (unixtime) 0) nc_init = nc_valid;

   //
   // Write out the times
   //
   write_netcdf_var_times(pcp_var, nc_init, nc_valid, nc_accum);

   //
   // Write the precip data
   //
   if(!pcp_var->put(plane.data(), plane.ny(), plane.nx())) {

      mlog << Error << "\n\n  write_netcdf() -> "
           << "error with pcp_var->put()\n\n";
      exit(1);
   }

   f_out->close();
   delete f_out;
   f_out = (NcFile *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////


bool is_timestring(const char * text)

{

if ( is_hh(text) ) return ( true );

if ( is_hhmmss(text) ) return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


ConcatString make_magic(int gribcode, int ptv, int accum_interval)

{

ConcatString s;

s << get_grib_code_abbr(gribcode, ptv) << "/A" << sec_to_hhmmss(accum_interval);

return ( s );

}


////////////////////////////////////////////////////////////////////////

void usage()

{

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\t[[-sum] sum_args] | [-add add_args] | [-subtract subtract_args]\n"
        << "\t[-gc code]\n"
        << "\t[-ptv number]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-sum sum_args\" indicates that accumulations "
        << "from multiple files should be summed up using the "
        << "arguments provided.\n"

        << "\t\t\"-add add_args\" indicates that accumulations from "
        << "one or more files should be added together using the "
        << "arguments provided.\n"

        << "\t\t\"-subtract subtract_args\" indicates that "
        << "accumulations from two files should be subtracted using "
        << "the arguments provided.\n"

        << "\t\t\"-gc code\" overrides the default GRIB code ("
        << grib_code << ") to be used (optional).\n"

        << "\t\t\"-ptv number\" overrides the default GRIB parameter "
        << "table version number (" << grib_ptv
        << ") to be used (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << verbosity << ") (optional).\n\n"

        << "\t\tNote: Specifying \"-sum\" is not required since it is "
        << "the default behavior.\n\n"

        << "\tSUM_ARGS:\n"
        << "\t\tinit_time\n"
        << "\t\tin_accum\n"
        << "\t\tvalid_time\n"
        << "\t\tout_accum\n"
        << "\t\tout_file\n"
        << "\t\t[-pcpdir path]\n"
        << "\t\t[-pcprx reg_exp]\n\n"

        << "\t\twhere\t\"init_time\" indicates the initialization "
        << "time of the input GRIB files in YYYYMMDD[_HH[MMSS]] format "
        << "(required).\n"

        << "\t\t\t\"in_accum\" indicates the accumulation interval "
        << "of the input GRIB files in HH[MMSS] format (required).\n"

        << "\t\t\t\"valid_time\" indicates the desired valid "
        << "time in YYYYMMDD[_HH[MMSS]] format (required).\n"

        << "\t\t\t\"out_accum\" indicates the desired accumulation "
        << "interval for the output NetCDF file in HH[MMSS] format (required).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written consisting of the sum of the accumulation intervals "
        << "(required).\n"

        << "\t\t\t\"-pcpdir path\" overrides the default precipitation directory ("
        << default_pcp_dir << ") (optional).\n"

        << "\t\t\t\"-pcprx reg_exp\" overrides the default regular expression for "
        << "precipitation file naming convention (" << default_reg_exp
        << ") (optional).\n\n"

        << "\t\t\tNote: Set init_time to 00000000_000000 when summing "
        << "observation files.\n\n"

        << "\tADD_ARGS:\n"
        << "\t\tin_file1\n"
        << "\t\taccum1\n"
        << "\t\t...\n"
        << "\t\tin_filen\n"
        << "\t\taccumn\n"
        << "\t\tout_file\n\n"

        << "\t\twhere\t\"in_file1\" indicates the name of the first input GRIB "
        << "file to be used (required).\n"

        << "\t\t\t\"accum1\" indicates the accumulation interval "
        << "to be used from in_file1 in HH[MMSS] format (required).\n"

        << "\t\t\t\"in_filen\" indicates additional input GRIB files to be "
        << "added together (optional).\n"

        << "\t\t\t\"accumn\" indicates the accumulation interval "
        << "to be used in HH[MMSS] format (optional).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written (required).\n\n"

        << "\tSUBTRACT_ARGS:\n"
        << "\t\tin_file1\n"
        << "\t\taccum1\n"
        << "\t\tin_file2\n"
        << "\t\taccum2\n"
        << "\t\tout_file\n\n"

        << "\t\twhere\t\"in_file1\" indicates the name of the first input GRIB "
        << "file to be used (required).\n"

        << "\t\t\t\"accum1\" indicates the accumulation interval "
        << "to be used from in_file1 in HH[MMSS] format (required).\n"

        << "\t\t\t\"in_file2\" indicates the name of the second input GRIB "
        << "file to be subtracted from in_file1 (required).\n"

        << "\t\t\t\"accum2\" indicates the accumulation interval "
        << "to be used from in_file2 in HH[MMSS] format (required).\n"

        << "\t\t\t\"out_file\" indicates the name of the output NetCDF file to "
        << "be written (required).\n"

        << "\n" << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_sum(const StringArray &)
{
   run_command = sum;
}

////////////////////////////////////////////////////////////////////////

void set_add(const StringArray &)
{
   run_command = add;
}

////////////////////////////////////////////////////////////////////////

void set_subtract(const StringArray &)
{
   run_command = sub;
}

////////////////////////////////////////////////////////////////////////

void set_grib_code(const StringArray & a)
{
   grib_code = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_ptv_num(const StringArray & a)
{
   grib_ptv = atoi(a[0]);
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
   verbosity = atoi(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_pcpdir(const StringArray & a)
{
   pcp_dir.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_pcprx(const StringArray & a)
{
   pcp_reg_exp = a[0];
}

////////////////////////////////////////////////////////////////////////

