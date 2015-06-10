// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   reformat_gsi.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    6/9/15    Bullock         New
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"
#include "config_constants.h"
#include "vx_stat_out.h"
#include "vx_log.h"

#include "read_fortran_binary.h"
#include "conv_offsets.h"
#include "conv_record.h"
#include "rad_offsets.h"
#include "rad_record.h"
#include "reformat_gsi.h"

////////////////////////////////////////////////////////////////////////

static void setup_header();
static void setup_table(AsciiTable &);

static void process_conv(const char *conv_filename, const char *output_filename);
static void process_rad (const char *rad_filename, const char *output_filename);

static void write_mpr_row_conv(AsciiTable & at, int row, ConvRecord & r, const int j, const char *);
static void write_mpr_row_rad (AsciiTable & at, int row, RadRecord  & r, const int chan);

static void usage();
static void set_channel(const StringArray &);
static void set_hdr(const StringArray &);
static void set_suffix(const StringArray &);
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []) {
   CommandLine cline;
   ConcatString output_filename;
   
   // Parse the command line into tokens
   cline.set(argc, argv);
   
   // Set the usage
   cline.set_usage(usage);

   // Add options
   cline.add(set_channel,   "-channel", 1);
   cline.add(set_hdr,       "-set_hdr", 2);
   cline.add(set_suffix,    "-suffix",  1);
   cline.add(set_outdir,    "-outdir",  1);
   cline.add(set_logfile,   "-log",     1);
   cline.add(set_verbosity, "-v",       1);

   // Parse the command line
   cline.parse();

   // Check for zero files to process
   if(cline.n() == 0) usage();
   
   // Process each remaining argument
   for(int i=0; i<(cline.n()); i++) {

      mlog << Debug(1)
           << "\nReading: " << cline[i] << " ... " << (i + 1)
           << " of " << cline.n() << "\n";

      // Concstruct output file name
      output_filename << cs_erase
                      << output_directory << '/'
                      << get_short_name(cline[i]) << suffix;

      // Initialize output StatHdrColumns
      setup_header();

      // Determine file type by the file name
      if(strstr(get_short_name(cline[i]), "conv") != (char *) 0) {
         process_conv(cline[i], output_filename);
      }
      else {
         process_rad(cline[i], output_filename);
      }
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////
//
// Initialize output StatHdrColumns.
//
////////////////////////////////////////////////////////////////////////

void setup_header() {
   int index;
   SingleThresh st;
   
   // MODEL
   if(hdr_name.has("MODEL", index)) {
      shc.set_model(hdr_value[index]);
   }
   else {
      shc.set_model(default_model);
   }

   // FCST_LEAD
   if(hdr_name.has("FCST_LEAD", index)) {
      shc.set_fcst_lead_sec(timestring_to_sec(hdr_value[index]));
   }
   else {
      shc.set_fcst_lead_sec(default_lead);
   }

   // FCST_VALID_BEG, FCST_VALID_END
   if(hdr_name.has("FCST_VALID_BEG", index)) {
      shc.set_fcst_valid_beg(timestring_to_unix(hdr_value[index]));
   }
   if(hdr_name.has("FCST_VALID_END", index)) {
      shc.set_fcst_valid_end(timestring_to_unix(hdr_value[index]));
   }

   // OBS_LEAD
   if(hdr_name.has("OBS_LEAD", index)) {
      shc.set_obs_lead_sec(timestring_to_sec(hdr_value[index]));
   }
   else {
      shc.set_obs_lead_sec(default_lead);
   }

   // OBS_VALID_BEG, OBS_VALID_END
   if(hdr_name.has("OBS_VALID_BEG", index)) {
      shc.set_obs_valid_beg(timestring_to_unix(hdr_value[index]));
   }
   if(hdr_name.has("OBS_VALID_END", index)) {
      shc.set_obs_valid_end(timestring_to_unix(hdr_value[index]));
   }

   // FCST_VAR
   if(hdr_name.has("FCST_VAR", index)) {
      shc.set_fcst_var(hdr_value[index]);
   }

   // FCST_LEV
   if(hdr_name.has("FCST_LEV", index)) {
      shc.set_fcst_lev(hdr_value[index]);
   }
   else {
      shc.set_fcst_lev(default_lev);
   }

   // OBS_VAR
   if(hdr_name.has("OBS_VAR", index)) {
      shc.set_obs_var(hdr_value[index]);
   }

   // OBS_LEV
   if(hdr_name.has("OBS_LEV", index)) {
      shc.set_obs_lev(hdr_value[index]);
   }
   else {
      shc.set_obs_lev(default_lev);
   }

   // OBTYPE
   if(hdr_name.has("OBTYPE", index)) {
      shc.set_obtype(hdr_value[index]);
   }
   else {
      shc.set_obtype(default_obtype);
   }

   // VX_MASK
   if(hdr_name.has("VX_MASK", index)) {
      shc.set_mask(hdr_value[index]);
   }
   else {
      shc.set_mask(default_vx_mask);
   }

   // INTERP_MTHD
   if(hdr_name.has("INTERP_MTHD", index)) {
      shc.set_interp_mthd(hdr_value[index]);
   }
   else {
      shc.set_interp_mthd(default_interp_mthd);
   }

   // INTERP_PNTS
   if(hdr_name.has("INTERP_PNTS", index)) {
      shc.set_interp_wdth(nint(sqrt(atoi(hdr_value[index]))));
   }
   else {
      shc.set_interp_wdth(default_interp_wdth);
   }

   // FCST_THRESH
   if(hdr_name.has("FCST_THRESH", index)) st.set(hdr_value[index]);
   else                                   st.set(default_thresh);
   shc.set_fcst_thresh(st);
   
   // OBS_THRESH
   if(hdr_name.has("OBS_THRESH", index))  st.set(hdr_value[index]);
   else                                   st.set(default_thresh);
   shc.set_obs_thresh(st);   
   
   // COV_THRESH
   if(hdr_name.has("COV_THRESH", index))  st.set(hdr_value[index]);
   else                                   st.set(default_thresh);
   shc.set_cov_thresh(st);

   // ALPHA
   if(hdr_name.has("ALPHA", index)) {
      shc.set_alpha(atof(hdr_value[index]));
   }
   else {
      shc.set_alpha(default_alpha);
   }

   // LINE_TYPE
   if(hdr_name.has("LINE_TYPE", index)) {
      shc.set_line_type(hdr_value[index]);
   }
   else {
      shc.set_line_type(default_line_type);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at) {

   // Justify the STAT AsciiTable objects
   justify_stat_cols(at);

   // Set the precision
   at.set_precision(default_precision);

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Process conventional GSI data.
//
////////////////////////////////////////////////////////////////////////

void process_conv(const char *conv_filename, const char *output_filename) {
   int i, j;
   int n_in, n_out;
   ofstream out;
   AsciiTable at;
   bool uv_flag;

   ConvFile f;
   ConvRecord r;
   
   // Setup output AsciiTable
   at.set_size(1, n_header_columns + n_mpr_columns + n_conv_extra_cols);
   setup_table(at);
   
   // Write header row
   write_header_row(mpr_columns, n_mpr_columns, 1, at, 0, 0);
   write_header_row(conv_extra_columns, n_conv_extra_cols, 0, at, 0,
                    n_header_columns + n_mpr_columns);

   // Open input file
   if(!(f.open(conv_filename))) {
      mlog << Error << "\nprocess_conv() -> "
           << "can't open input file \"" << conv_filename << "\"\n\n";
      exit(1);
   }

   mlog << Debug(1)
        << "Writing: " << output_filename << "\n";

   // Open output file
   out.open(output_filename);
   if(!out) {
      mlog << Error << "\nprocess_conv() -> "
           << "can't open output file \"" << output_filename << "\"\n\n";
      exit(1);
   }

   // Process each record
   n_in  = 0;
   n_out = 1; // 1 for header line
   while(f >> r) {

      uv_flag = (str_trim(r.variable) == "uv");
      at.add_rows((uv_flag ? 2 : 1)*r.ii); // double for uv lines
      
      for (j=0; j<(r.ii); ++j)  {
         if(uv_flag) {
            write_mpr_row_conv(at, n_out++, r, j, "u");
            write_mpr_row_conv(at, n_out++, r, j, "v");
         }
         else {
            write_mpr_row_conv(at, n_out++, r, j, str_trim(r.variable));
         }
      }

      n_in++;
   } // end while

   mlog << Debug(2) << "Read " << n_in << " records and wrote "
        << n_out << " lines.\n";

   // Write AsciiTable to output file
   out << at;

   // Close files
   f.close();
   out.close();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Process GSI radiance data.
//
////////////////////////////////////////////////////////////////////////

void process_rad(const char *rad_filename, const char *output_filename) {
   int i, j, k;
   int n_in, n_out;
   ofstream out;
   AsciiTable at;
   NumArray do_channel;
   ConcatString cs;

   RadFile f;
   RadRecord r;

   // Setup output AsciiTable
   at.set_size(1, n_header_columns + n_mpr_columns + n_rad_extra_cols);
   setup_table(at);

   // Write header row
   write_header_row(mpr_columns, n_mpr_columns, 1, at, 0, 0);
   write_header_row(rad_extra_columns, n_rad_extra_cols, 0, at, 0,
                    n_header_columns + n_mpr_columns);

   // Open input file
   if(!(f.open(rad_filename))) {
      mlog << Error << "\nprocess_rad() -> "
           << "can't open input file \"" << rad_filename << "\"\n\n";
      exit(1);
   }

   mlog << Debug(1)
        << "Writing: " << output_filename << "\n";

   // Open output file
   out.open(output_filename);
   if(!out) {
      mlog << Error << "\nprocess_rad() -> "
           << "can't open output file \"" << output_filename << "\"\n\n";
      exit(1);
   }

   // Process all channels, if not otherwise specified
   if(channel.n_elements() == 0) {
      for(i=0; i<f.n_channels(); i++) do_channel.add(i+1);      
      mlog << Debug(2)
           << "Processing all " << do_channel.n_elements() << " channels.\n";
   }
   else {
      do_channel = channel;
      cs << cs_erase << "Processing " << do_channel.n_elements()
         << " requested channels: " << nint(do_channel[0]);
      for(i=1; i<do_channel.n_elements(); i++) cs << ", " << nint(do_channel[i]);
      mlog << Debug(2) << cs << "\n";
   }

   // Process each record
   n_in  = 0;
   n_out = 1; // 1 for header line
   while(f >> r)  {

      at.add_rows(do_channel.n_elements());
      for(i=0; i<do_channel.n_elements(); i++) {
         write_mpr_row_rad(at, n_out++, r, do_channel[i]-1);
      }

      n_in++;
   } // end while

   mlog << Debug(2) << "Read " << n_in << " records and wrote "
        << n_out << " lines.\n";

   // Write AsciiTable to output file
   out << at;

   // Close files
   f.close();
   out.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mpr_row_conv(AsciiTable &at, int row, ConvRecord & r,
                        const int j, const char *variable) {
   unixtime ut;
   int col;

   double lat       = r.rdiag_get_2d(      conv_lat_index - 1, j);
   double lon       = r.rdiag_get_2d(      conv_lon_index - 1, j);
   double pressure  = r.rdiag_get_2d( conv_pressure_index - 1, j);
   double obs_value = (strcmp(variable, "v") == 0 ?
                      r.rdiag_get_2d(conv_obs_v_data_index - 1, j) :
                      r.rdiag_get_2d(conv_obs_data_index   - 1, j));
   double elevation = r.rdiag_get_2d(conv_elevation_index  - 1, j);

   ConcatString obs_subtype = nint(r.rdiag_get_2d (  conv_obssubtype_index - 1, j));
   double pb_inv_error      = r.rdiag_get_2d(   conv_pb_inverse_index - 1, j);
   double final_inv_error   = r.rdiag_get_2d(conv_final_inverse_index - 1, j);

   double guess    = (strcmp(variable, "v") == 0 ?
                      r.rdiag_get_guess_v(j) : r.rdiag_get_guess(j));

   int    obs_hgt  = nint(r.rdiag_get_2d(         conv_height_index - 1, j));
   double obs_time = r.rdiag_get_2d(      conv_obs_hours_index - 1, j);
   int    obs_qc   = nint(r.rdiag_get_2d(       conv_input_qc_index - 1, j));

   int    setup_qc  = nint(r.rdiag_get_2d(       conv_setup_qc_index - 1, j));
   int    prep_use  = nint(r.rdiag_get_2d(          conv_usage_index - 1, j));
   int    analy_use = nint(r.rdiag_get_2d(   conv_analysis_use_index - 1, j));

   double qc_wght   = r.rdiag_get_2d(      conv_qc_weight_index - 1, j);
   double oberr_adj = r.rdiag_get_2d(conv_read_pb_inverse_index - 1, j);

   // Update header for current data   
   if(!hdr_name.has("FCST_VALID_BEG")) shc.set_fcst_valid_beg(r.date);
   if(!hdr_name.has("FCST_VALID_END")) shc.set_fcst_valid_end(r.date);
   ut = nint(r.date + (obs_time * sec_per_hour));
   if(!hdr_name.has("OBS_VALID_BEG"))  shc.set_obs_valid_beg(ut);
   if(!hdr_name.has("OBS_VALID_END"))  shc.set_obs_valid_end(ut);
   if(!hdr_name.has("FCST_VAR"))       shc.set_fcst_var(variable);
   if(!hdr_name.has("OBS_VAR"))        shc.set_obs_var(variable);
   if(!hdr_name.has("OBTYPE"))         shc.set_obtype(obs_subtype);

   // Range check the height
   if(obs_hgt < 0) obs_hgt = bad_data_int;
   
   // Range check setup_qc
   if(setup_qc == bad_setup_qc) setup_qc = bad_data_int;

   // Set pressure for precipitable water to bad data
   if(variable == "pw") pressure = bad_data_double;

   // Write header columns
   write_header_cols(shc, at, row);
   col = n_header_columns;

   // Write MPR columns
   at.set_entry(row, col++, 1);                 // TOTAL
   at.set_entry(row, col++, 0);                 // INDEX

   at.set_entry(row, col++, r.station_name(j)); // OBS_SID
   at.set_entry(row, col++, lat);               // OBS_LAT
   at.set_entry(row, col++, rescale_lon(lon));  // OBS_LON
   at.set_entry(row, col++, pressure);          // OBS_LVL
   at.set_entry(row, col++, elevation);         // OBS_ELV

   at.set_entry(row, col++, guess);             // FCST
   at.set_entry(row, col++, obs_value);         // OBS
   at.set_entry(row, col++, na_str);            // CLIMO
   at.set_entry(row, col++, obs_qc);            // OBS_QC

   // Write extra columns
   at.set_entry(row, col++, obs_hgt);           // OBS_HGT

   at.set_entry(row, col++, pb_inv_error);      // OBS_ERR_IN
   at.set_entry(row, col++, oberr_adj);         // OBS_ERR_ADJ
   at.set_entry(row, col++, final_inv_error);   // OBS_ERR_FIN

   at.set_entry(row, col++, prep_use);          // PREP_USE
   at.set_entry(row, col++, analy_use);         // ANLY_USE

   at.set_entry(row, col++, setup_qc);          // SETUP_QC
   at.set_entry(row, col++, qc_wght);           // QC_WGHT

   return;
}

////////////////////////////////////////////////////////////////////////

void write_mpr_row_rad(AsciiTable &at, int row, RadRecord & r, const int chan) {
   ConcatString variable;
   unixtime ut;
   int col;

   const double lat             = r.diag_data(rad_lat_index - 1);
   const double lon             = r.diag_data(rad_lon_index - 1);
   const double elev            = r.diag_data(rad_elevation_index - 1);
   const double obs_time        = r.diag_data(rad_dtime_index - 1);

   const double obs_value       = r.diagchan_data( rad_btemp_chan_index - 1, chan);
   const double obs_minus_guess = r.diagchan_data(rad_omg_bc_chan_index - 1, chan);

   const double inv_obs_error   = r.diagchan_data(rad_inv_chan_index - 1, chan);
   const double surf_em         = r.diagchan_data(rad_surf_em_index  - 1, chan);

   const double guess           = obs_value - obs_minus_guess;

   variable.format("TB_%02d", chan + 1);

   // Update header for current data   
   if(!hdr_name.has("FCST_VALID_BEG")) shc.set_fcst_valid_beg(r.date());
   if(!hdr_name.has("FCST_VALID_END")) shc.set_fcst_valid_end(r.date());
   ut = nint(r.date() + (obs_time * sec_per_hour));
   if(!hdr_name.has("OBS_VALID_BEG"))  shc.set_obs_valid_beg(ut);
   if(!hdr_name.has("OBS_VALID_END"))  shc.set_obs_valid_end(ut);
   if(!hdr_name.has("FCST_VAR"))       shc.set_fcst_var(variable);
   if(!hdr_name.has("OBS_VAR"))        shc.set_obs_var(variable);

   // Write header columns
   write_header_cols(shc, at, row);
   col = n_header_columns;

   // Write MPR columns
   at.set_entry(row, col++, 1);                 // TOTAL
   at.set_entry(row, col++, 0);                 // INDEX

   at.set_entry(row, col++, na_str);            // OBS_SID
   at.set_entry(row, col++, lat);               // OBS_LAT
   at.set_entry(row, col++, rescale_lon(lon));  // OBS_LON
   at.set_entry(row, col++, na_str);            // OBS_LVL
   at.set_entry(row, col++, elev);              // OBS_ELV

   at.set_entry(row, col++, guess);             // FCST
   at.set_entry(row, col++, obs_value);         // OBS
   at.set_entry(row, col++, na_str);            // CLIMO
   at.set_entry(row, col++, na_str);            // OBS_QC

   // Write extra columns
   at.set_entry(row, col++, inv_obs_error);     // INV_OBS_ERR
   at.set_entry(row, col++, surf_em);           // SURF_EMIS

   return;
}
   
////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tgsi_file1 [gsi_file2 gsi_file3 ... gsi_filen]\n"
        << "\t[-channel n]\n"
        << "\t[-set_hdr col_name value]\n"        
        << "\t[-suffix string]\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"gsi_file\" is a binary GSI diagnostic file "
        << "(conventional or radiance) to be reformatted (required).\n"
        << "\t\t\"-channel n\" overrides the default processing of all "
        << "radiance channels with a comma-separated list (optional).\n"
        << "\t\t\"-set_hdr col_name value\" specifies what should be "
        << "written to the output header columns (optional).\n"
        << "\t\t\"-suffix string\" overrides the default output filename "
        << "suffix (" << suffix << ") (optional).\n"
        << "\t\t\"-outdir path\" overrides the default output directory ("
        << output_directory << ") (optional).\n"
        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_channel(const StringArray & a) {
   channel.add_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_hdr(const StringArray & a) {
   hdr_name.add_css(to_upper(a[0]));
   hdr_value.add_css(a[1]);
}

////////////////////////////////////////////////////////////////////////

void set_suffix(const StringArray & a) {
   suffix = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a) {
   output_directory = a[0];
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
