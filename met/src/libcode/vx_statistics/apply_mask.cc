// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <regex.h>

#include "apply_mask.h"

#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_log.h"

///////////////////////////////////////////////////////////////////////////////

void process_poly_mask(const ConcatString &, const Grid &,
                       DataPlane &, ConcatString&);

///////////////////////////////////////////////////////////////////////////////

void parse_grid_mask(const ConcatString &mask_grid_str, const Grid &grid,
                     DataPlane &mask_dp, ConcatString &mask_name) {
   Grid mask_grid;

   mlog << Debug(4) << "parse_grid_mask() -> "
        << "parsing grid mask \"" << mask_grid_str << "\"\n";

   // Initialize the DataPlane masking object by turning all points on
   mask_dp.set_size(grid.nx(), grid.ny());
   mask_dp.set_constant(mask_on_value);

   // Store the name of the masking grid
   mask_name = mask_grid_str;

   //
   // Check to make sure that we're not using the full domain
   //
   if(strcmp(full_domain_str, mask_grid_str) != 0) {

      //
      // Search for the grid name in the predefined grids
      //
      if(!find_grid_by_name(mask_name, mask_grid)) {
         mlog << Error << "\nparse_grid_mask() -> "
              << "the mask_grid requested \"" << mask_grid_str
              << "\" is not defined.\n\n";
         exit(1);
      }

      apply_grid_mask(grid, mask_grid, mask_dp);
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// The mask_poly_str contains masking regions defined one of 3 ways:
//
// (1) An ASCII file containing a lat/lon polygon.
// (2) The NetCDF output of the gen_poly_mask tool.
// (3) A 2D data file, followed by a description of the field to be used,
//     and optionally, a threshold to be applied to the field.
//
///////////////////////////////////////////////////////////////////////////////

void parse_poly_mask(const ConcatString &mask_poly_str, const Grid &grid,
                     DataPlane &mask_dp, ConcatString &mask_name)

{
   StringArray tokens;
   ConcatString s, file_name, thresh_str;
   SingleThresh st;
   MetConfig config;
   const char *delim = "{}";
   bool append_level, append_thresh;

   mlog << Debug(4) << "parse_poly_mask() -> "
        << "parsing poly mask \"" << mask_poly_str << "\"\n";

   // 2D Data file
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *mtddf = (Met2dDataFile *) 0;

   // VarInfo object
   VarInfoFactory info_factory;
   VarInfo *info = (VarInfo *) 0;

   // Split up the input string
   tokens = mask_poly_str.split(delim);

   // Store masking file name
   if(tokens.n_elements() > 0) file_name = replace_path(tokens[0]);
   file_name.ws_strip();

   // Attempt to open the data file
   mtddf = mtddf_factory.new_met_2d_data_file(file_name);

   // If data file pointer is NULL, assume a lat/lon polyline file
   if(!mtddf) {
      process_poly_mask(file_name, grid, mask_dp, mask_name);
   }
   // Otherwise, process the input data file
   else {

      // Check that the masking grid matches the input grid
      if(!(mtddf->grid() == grid)) {
         mlog << Error << "\nparse_poly_mask() -> "
              << "the masking grid does not match the input grid.\n\n";
         exit(1);
      }
      
      // Create a new VarInfo object
      info = info_factory.new_var_info(mtddf->file_type());

      // Parse the dictionary string
      if(tokens.n_elements() > 1) {
         append_level = true;
         config.read_string(tokens[1]);
      }
      else {
         append_level = false;
         config.read_string(default_mask_dict);
      }

      // Set up the VarInfo object
      info->set_dict(config);

      // Extract the data plane from the input file
      mtddf->data_plane(*info, mask_dp);

      // Parse the threshold string
      if(tokens.n_elements() > 2) {
         append_thresh = true;
         thresh_str = tokens[2];
      }
      else {
         append_thresh = false;
         thresh_str = default_mask_thresh;
      }
      thresh_str.ws_strip();

      // Parse the threshold information
      st.set(thresh_str);

      // Apply threshold to the data plane
      mask_dp.threshold(st);

      // Store the mask name
      mask_name = info->name();

      // Append level info
      if(append_level) mask_name << "_" << info->level_name();

      // Append threshold info
      if(append_thresh) mask_name << st.get_str();
   }

   // Clean up
   if(mtddf) { delete mtddf; mtddf = (Met2dDataFile *) 0; }
   if(info)  { delete info;  info  = (VarInfo *)       0; }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void process_poly_mask(const ConcatString &file_name, const Grid &grid,
                       DataPlane &mask_dp, ConcatString &mask_name) {
   Polyline mask_poly;

   mlog << Debug(4) << "parse_poly_mask() -> "
        << "parsing poly mask file \"" << file_name << "\"\n";

   // Initialize the DataPlane masking object by turning all points on
   mask_dp.set_size(grid.nx(), grid.ny());
   mask_dp.set_constant(mask_on_value);

   // Initialize the masking polyline
   mask_poly.clear();

   // Parse out the polyline from the file
   parse_latlon_poly_file(file_name, mask_poly);

   // Store the name of the masking polyline
   mask_name = mask_poly.name;

   // Apply the polyline mask to the DataPlane object
   apply_poly_mask_latlon(mask_poly, grid, mask_dp);

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// This function is passed a string containing either a single station ID or
// a file name containing a list of station ID's.  For a single station ID,
// store it in the output StringArray, and store it's value as the mask name.
// For a file name, parse the list of stations into the output StringArray,
// and store the first entry in the file as the mask name.
//
///////////////////////////////////////////////////////////////////////////////

void parse_sid_mask(const ConcatString &mask_sid_str,
                    StringArray &mask_sid, ConcatString &mask_name) {
   ifstream in;
   ConcatString tmp_file;
   char sid_str[PATH_MAX];

   // Initialize
   mask_sid.clear();
   mask_name = na_str;

   // Check for an empty length string
   if(mask_sid_str.empty()) return;
   
   // Replace any instances of MET_BASE with it's expanded value
   tmp_file = replace_path(mask_sid_str);

   // Process string as a file name, if possible
   if(file_exists(tmp_file)) {

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "parsing station ID masking file \"" << tmp_file << "\"\n";

      // Open the mask station id file specified
      in.open(tmp_file);

      if(!in) {
         mlog << Error << "\nparse_sid_mask() -> "
              << "Can't open the station ID masking file \""
              << tmp_file << "\".\n\n";
         exit(1);
      }

      // Store the first entry as the name of the mask
      in >> sid_str;
      mask_name = sid_str;

      // Store the rest of the entries as masking station ID's
      while(in >> sid_str) mask_sid.add(sid_str);

      // Close the input file
      in.close();

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "parsed " << mask_sid.n_elements() << " station ID's for the \""
           << mask_name << "\" mask from file \"" << tmp_file << "\"\n";
   }
   // Otherwise, process it as a single station ID
   else {

      mlog << Debug(4) << "parse_sid_mask() -> "
           << "storing single station ID mask \"" << mask_sid_str << "\"\n";

      // Check for embedded whitespace or slashes
      if(check_reg_exp(ws_reg_exp, mask_sid_str) ||
         check_reg_exp("[/]", mask_sid_str)) {
         mlog << Error << "\nparse_sid_mask() -> "
              << "masking station ID name can't contain whitespace or slashes \""
              << mask_sid_str << "\".\n\n";
         exit(1);
      }

      // Store the single station ID
      mask_sid.add(mask_sid_str);
      mask_name = mask_sid_str;
   }
   
   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_grid_mask(const Grid &grid, const Grid &mask_grid, DataPlane &dp) {
   int x, y;
   double lat, lon, mask_x, mask_y;
   int in_count, out_count;

   //
   // If the masking grid's dimensions are zero, there's nothing to be done
   //
   if(mask_grid.nx() == 0 || mask_grid.ny() == 0) {
      mlog << Warning << "\nvoid apply_grid_mask() -> "
           << "no grid masking applied since the masking grid "
           << "specified is empty.\n\n";
      return;
   }

   //
   // For each point of the original grid, convert it to (lat, lon) and then
   // to an (x, y) point on the mask grid. If the (x, y) is within the
   // bounds of the mask grid, retain it's value.  Otherwise, mask it out.
   //
   in_count = out_count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         grid.xy_to_latlon(x, y, lat, lon);
         mask_grid.latlon_to_xy(lat, lon, mask_x, mask_y);

         if(mask_x < 0 || mask_x >= mask_grid.nx() ||
            mask_y < 0 || mask_y >= mask_grid.ny()) {
            out_count++;
            dp.set(mask_off_value, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_poly_mask_xy(const Polyline &poly, DataPlane &dp) {
   int x, y;
   int in_count, out_count;

   //
   // If the polyline contains less than 3 points, no masking region is
   // defined and there is nothing to do
   //
   if(poly.n_points < 3) {
      mlog << Warning << "\nvoid apply_poly_mask_xy() -> "
           << "no polyline masking applied since the masking polyline "
           << "contains fewer than 3 points.\n\n";
      return;
   }

   //
   // Mask out any grid points not inside the masking polygon
   //
   in_count = out_count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         if(!(poly.is_inside(x, y))) {
            out_count++;
            dp.set(mask_off_value, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////

void apply_poly_mask_latlon(const Polyline &poly, const Grid &grid, DataPlane &dp) {
   int x, y;
   double lat, lon;
   int in_count, out_count;

   //
   // If the polyline contains less than 3 points, no masking region is
   // defined and there is nothing to do
   //
   if(poly.n_points < 3) {
      mlog << Warning << "\nvoid apply_poly_mask_latlon() -> "
           << "no polyline masking applied since the masking polyline "
           << "contains fewer than 3 points.\n\n";
      return;
   }

   //
   // Mask out any grid points who's corresponding lat/lon coordinates are
   // not inside the masking lat/lon polygon.
   //
   in_count = out_count = 0;
   for(x=0; x<dp.nx(); x++) {
      for(y=0; y<dp.ny(); y++) {

         grid.xy_to_latlon(x, y, lat, lon);

         if(!(poly.is_inside(rescale_lon(lon), lat))) {
            out_count++;
            dp.set(mask_off_value, x, y);
         }
         else {
            in_count++;
         }
      } // for y
   } // for x

   return;
}

///////////////////////////////////////////////////////////////////////////////
