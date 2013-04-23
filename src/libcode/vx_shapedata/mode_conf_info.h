// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MODE_CONF_INFO_H__
#define  __MODE_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

class ModeConfInfo {

   private:

      void init_from_scratch();

   public:

      ModeConfInfo();
     ~ModeConfInfo();

      void clear();

      void read_config    (const char * default_filename, const char * user_filename);

      void process_config (GrdFileType ftype, GrdFileType otype);

         // Store data parsed from the MODE configuration object

      MetConfig conf;                          // MODE configuration object

      ConcatString     model;                  // Model name

      VarInfo *        fcst_info;              // allocated
      VarInfo *        obs_info;               // allocated

      SingleThresh     fcst_raw_thresh;        // Values not meeting threshold set to bad data
      SingleThresh     obs_raw_thresh;

      int              fcst_conv_radius;       // Convolution radius in grid squares
      int              obs_conv_radius;

      SingleThresh     fcst_conv_thresh;       // Convolution threshold to define objects
      SingleThresh     obs_conv_thresh;

      double           fcst_vld_thresh;        // Minimum ratio of valid data points in the convolution area
      double           obs_vld_thresh;

      SingleThresh     fcst_area_thresh;       // Discard objects whose area doesn't meet threshold
      SingleThresh     obs_area_thresh;

      int              fcst_inten_perc_value;  // Intensity percentile of interest
      int              obs_inten_perc_value;

      SingleThresh     fcst_inten_perc_thresh; // Discard objects whose percentile intensity doesn't meet threshold
      SingleThresh     obs_inten_perc_thresh;

      SingleThresh     fcst_merge_thresh;      // Lower convolution threshold used for double merging method
      SingleThresh     obs_merge_thresh;

      MergeType        fcst_merge_flag;        // Define which merging methods should be employed
      MergeType        obs_merge_flag;

      FieldType        mask_missing_flag;      // Mask missing data between fcst and obs      

      MatchType        match_flag;             // Define which matching methods should be employed

      double           max_centroid_dist;      // Only compare objects whose centroids are close enough (in grid squares)

      ConcatString     mask_grid_name;         // Path for masking grid area
      FieldType        mask_grid_flag;         // Define which fields should be masked out

      ConcatString     mask_poly_name;         // Path for masking poly area
      FieldType        mask_poly_flag;         // Define which fields should be masked out

      double           centroid_dist_wt;       // Weights used as input to the fuzzy engine
      double           boundary_dist_wt;
      double           convex_hull_dist_wt;
      double           angle_diff_wt;
      double           area_ratio_wt;
      double           int_area_ratio_wt;
      double           complexity_ratio_wt;
      double           inten_perc_ratio_wt;

      int              inten_perc_value;       // Intensity percentile used for the intensity percentile ratio

      PiecewiseLinear * centroid_dist_if;      // Interest functions used as input to the fuzzy engine
      PiecewiseLinear * boundary_dist_if;      // not allocated
      PiecewiseLinear * convex_hull_dist_if;
      PiecewiseLinear * angle_diff_if;
      PiecewiseLinear * area_ratio_if;
      PiecewiseLinear * int_area_ratio_if;
      PiecewiseLinear * complexity_ratio_if;
      PiecewiseLinear * inten_perc_ratio_if;

      double           total_interest_thresh;  // Total interest threshold defining significance

      double           print_interest_thresh;  // Only write output for pairs with this interest

      ConcatString     met_data_dir;           // MET data directory

      PlotInfo         fcst_raw_pi;            // Raw forecast plotting info
      PlotInfo         obs_raw_pi;             // Raw observation plotting info
      PlotInfo         object_pi;              // Object plotting info

      int              zero_border_size;       // Zero out edge rows and columns for object definition      

      bool             plot_valid_flag;        // Zoom up plot to the sub-region of valid data
      bool             plot_gcarc_flag;        // Plot lines as great-circle arcs
      bool             ps_plot_flag;           // Flag for the output PostScript image file      
      bool             nc_pairs_flag;          // Flag for the output NetCDF pairs file
      bool             ct_stats_flag;          // Flag for the output contingency table statistics file

      ConcatString     output_prefix;          // String to customize output file name
      ConcatString     version;                // Config file version

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __MODE_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
