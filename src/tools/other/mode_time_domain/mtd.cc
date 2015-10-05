

////////////////////////////////////////////////////////////////////////


static const char default_config_path          [] = "MET_BASE/config/MTDConfig_default";

static const char txt_2d_suffix                [] = "2d.txt";

static const char txt_3d_single_simple_suffix  [] = "3d_ss.txt";
static const char txt_3d_pair_simple_suffix    [] = "3d_ps.txt";

static const char txt_3d_single_cluster_suffix [] = "3d_sc.txt";
static const char txt_3d_pair_cluster_suffix   [] = "3d_pc.txt";

static const char nc_suffix                    [] = "obj.nc";

static const char default_prefix               [] = "mtd";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_data2d_factory.h"
#include "apply_mask.h"
#include "mtd_config_info.h"
#include "mtd_file.h"
#include "interest_calc.h"
#include "3d_att_single_array.h"
#include "mtd_txt_output.h"
#include "mtd_read_data.h"
#include "mm_engine.h"
#include "mtd_nc_output.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static ConcatString output_directory;

static StringArray fcst_filenames;
static StringArray  obs_filenames;

static ConcatString local_config_filename;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_fcst      (const StringArray &);
static void set_obs       (const StringArray &);
static void set_config    (const StringArray &);

static void set_verbosity (const StringArray &);
static void set_logfile   (const StringArray &);
static void set_outdir    (const StringArray &);

static StringArray parse_file_list(const StringArray &, const GrdFileType);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_fcst,      "-fcst",   -1);
cline.add(set_obs,       "-obs",    -1);
cline.add(set_config,    "-config",  1);
cline.add(set_verbosity, "-v",       1);
cline.add(set_logfile,   "-log",     1);
cline.add(set_outdir,    "-outdir",  1);

cline.parse();

if ( cline.n() != 0 )  usage();

if ( fcst_filenames.n() == 0 )  usage();
if (  obs_filenames.n() == 0 )  usage();

if ( output_directory.empty() )  output_directory = ".";

   //
   //  read the config file
   //

MtdConfigInfo config;
ConcatString default_config_filename;
ConcatString path;


default_config_filename = replace_path(default_config_path);

config.read_config(default_config_filename, local_config_filename);

   //
   //  determine the input file types
   //    - check the config file for the file_type
   //    - check the first data file
   //

GrdFileType ftype, otype;

ftype = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_fcst));
otype = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_obs));

   //
   //  parse the forecast and observation file lists
   //

fcst_filenames = parse_file_list(fcst_filenames, ftype);
obs_filenames  = parse_file_list(obs_filenames,  otype);

   //
   //  retrieve the gridded file types
   //

if ( ftype == FileType_None ) ftype = grd_file_type(fcst_filenames[0]);
if ( otype == FileType_None ) otype = grd_file_type(obs_filenames[0]);

config.process_config(ftype, otype);


int j, k;
MtdFloatFile fcst_raw, obs_raw;
MtdFloatFile fcst_conv, obs_conv;
MtdIntFile fcst_mask, obs_mask;
MtdIntFile fcst_obj, obs_obj;
MM_Engine e;
const double ti_thresh = config.total_interest_thresh;


   //
   //  read the data files
   //

mtd_read_data(config, *(config.fcst_info), fcst_filenames, fcst_raw);
mtd_read_data(config, *(config.obs_info),   obs_filenames,  obs_raw);

if ( fcst_raw.delta_t() != obs_raw.delta_t() )  {

   mlog << Error
        << "\n\n  " << program_name << ": forecast time difference is different that obervation time difference!\n\n";

   exit ( 1 );

}

config.delta_t_seconds = fcst_raw.delta_t();

   //
   //  regrid, if necessary
   //

mlog << Debug(2) << "regridding, if needed ...\n";

const Grid to_grid = parse_vx_grid(config.regrid_info, fcst_raw.grid_p(), obs_raw.grid_p());

fcst_raw.regrid(to_grid, config.regrid_info);
 obs_raw.regrid(to_grid, config.regrid_info);

   //
   //  make the output file prefix
   //

ConcatString prefix;
int year, month, day, hour, minute, second;
char junk[256];

prefix << cs_erase << default_prefix << '_';

if ( config.output_prefix.nonempty() )  prefix << config.output_prefix << '_';

unix_to_mdyhms(obs_raw.start_time(), month, day, year, hour, minute, second);

snprintf(junk, sizeof(junk), "%04d%02d%02d_%02d%02d%02dV", year, month, day, hour, minute, second);

prefix << junk;

   //
   //  set up the total interest calculator
   //

e.calc.add(config.space_centroid_dist_wt, config.space_centroid_dist_if, &PairAtt3D::SpaceCentroidDist);
e.calc.add(config.time_centroid_delta_wt, config.time_centroid_delta_if, &PairAtt3D::TimeCentroidDelta);
e.calc.add(config.speed_delta_wt,         config.speed_delta_if,         &PairAtt3D::SpeedDelta);
e.calc.add(config.direction_diff_wt,      config.direction_diff_if,      &PairAtt3D::DirectionDifference);
e.calc.add(config.volume_ratio_wt,        config.volume_ratio_if,        &PairAtt3D::VolumeRatio);
e.calc.add(config.axis_angle_diff_wt,     config.axis_angle_diff_if,     &PairAtt3D::AxisDiff);
e.calc.add(config.start_time_delta_wt,    config.start_time_delta_if,    &PairAtt3D::StartTimeDelta);
e.calc.add(config.end_time_delta_wt,      config.end_time_delta_if,      &PairAtt3D::EndTimeDelta);

   //
   //  make sure that not all of the weights are zero
   //

e.calc.check();

   //
   //  convolve
   //

 obs_conv =  obs_raw.convolve(config.obs_conv_radius);
fcst_conv = fcst_raw.convolve(config.fcst_conv_radius);

   //
   //  threshold
   //

fcst_mask = fcst_conv.threshold(config.fcst_conv_thresh);
 obs_mask =  obs_conv.threshold(config.obs_conv_thresh);

   //
   //  number the objects
   //

fcst_obj = fcst_mask;
 obs_obj =  obs_mask;

mlog << Debug(2) << "Splitting fcst object field\n";
   fcst_obj.split();
mlog << Debug(2) << "Done splitting fcst\n";
mlog << Debug(2) << "Splitting obs object field\n";
    obs_obj.split();
mlog << Debug(2) << "Done splitting obs\n";

   //
   //  toss small objects
   //

fcst_obj.toss_small_objects(config.min_volume);
 obs_obj.toss_small_objects(config.min_volume);

   //
   //  set up the match/merge engine
   //

e.set_size(fcst_obj.n_objects(), obs_obj.n_objects());

   //
   //  get single attributes
   //

SingleAtt3D att_3;
SingleAtt3DArray fcst_single_att, obs_single_att;
Object mask;

mlog << Debug(2)
     << "Calculating 3D fcst single attributes\n";

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   mask = fcst_obj.select(j + 1);   //  1-based

   att_3 = calc_3d_single_atts(mask, fcst_raw, config.model);

   att_3.set_object_number(j + 1);   //  1-based

   att_3.set_fcst();

   att_3.set_simple();

   fcst_single_att.add(att_3);

}



mlog << Debug(2)
     << "Calculating 3D obs single attributes\n";

for (j=0; j<(obs_obj.n_objects()); ++j)  {

   mask = obs_obj.select(j + 1);   //  1-based

   att_3 = calc_3d_single_atts(mask, obs_raw, config.model);

   att_3.set_object_number(j + 1);   //  1-based

   att_3.set_obs();

   att_3.set_simple();

   obs_single_att.add(att_3);

}



   //
   //  get simple pair attributes
   //

PairAtt3DArray pa_simple;
PairAtt3D p;
MtdIntFile fo, oo;

// mlog << Debug(5) << "\n\n  Calculating pair attributes ... (Nf = "
//      << (fcst_obj.n_objects()) << ", No = "
//      << (obs_obj.n_objects())  << ")\n\n";

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   fo = fcst_obj.select(j + 1);

   for (k=0; k<(obs_obj.n_objects()); ++k)  {

      oo = obs_obj.select(k + 1);

      p = calc_3d_pair_atts(fo, oo, fcst_single_att[j], obs_single_att[k]);

      p.set_total_interest(e.calc(p));

      p.set_simple();

      // mlog << Debug(5) << "   (F_" << j << ", O_" << k << ")   "
      //      << p.total_interest() << '\n';

      pa_simple.add(p);

   }

   // cout.put('\n');

}



   //
   //  calculate 2d attributes
   //

int t;
SingleAtt2DArray fcst_att_2d, obs_att_2d;
SingleAtt2D att_2;
MtdIntFile mask_2d;

   //   fcst objects

mlog << Debug(2) << "Calculating 2D fcst attributes\n";

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   att_3 = fcst_single_att[j];

   for (t=(att_3.tmin()); t<=(att_3.tmax()); ++t)  {

      mask_2d = fcst_obj.const_t_mask(t, j + 1);   //  1-based

      att_2 = calc_2d_single_atts(mask_2d, j + 1);

      att_2.set_fcst();

      att_2.set_object_number(j + 1);   //  1-based

      att_2.set_time_index(t);

      fcst_att_2d.add(att_2);

   }   //  for k

}   //  for j

   //   obs objects

mlog << Debug(2) << "Calculating 2D obs attributes\n";

for (j=0; j<(obs_obj.n_objects()); ++j)  {

   att_3 = obs_single_att[j];

   for (t=(att_3.tmin()); t<=(att_3.tmax()); ++t)  {

      mask_2d = obs_obj.const_t_mask(t, j + 1);   //  1-based

      att_2 = calc_2d_single_atts(mask_2d, j + 1);

      att_2.set_obs();

      att_2.set_object_number(j + 1);   //  1-based

      att_2.set_time_index(t);

      obs_att_2d.add(att_2);

   }   //  for k

}   //  for j



   //
   //  create graph
   //

for (j=0; j<(pa_simple.n()); ++j)  {

   if ( pa_simple.total_interest(j) < ti_thresh )  continue;

   e.graph.set_fo_edge(pa_simple.fcst_obj_number(j) - 1, pa_simple.obs_obj_number(j) - 1);

}   //  for j

e.graph.dump_as_table(6);

e.do_match_merge();

e.partition_dump(6);


IntArray a;
const int n_clusters = e.n_composites();

// mlog << Debug(2) << "N clusters = " << n_clusters << '\n';

if ( mlog.verbosity_level() > 5 )  {

   for (j=0; j<n_clusters; ++j)  {

      // mlog << Debug(5) << "Fcst objects in composite " << j << ":  ";

      a = e.fcst_composite(j);

      // a.dump_one_line(cout);

      // mlog << Debug(5) << "Obs  objects in composite " << j << ":  ";

      a = e.obs_composite(j);

      // a.dump_one_line(cout);

      // mlog << Debug(5) << '\n';

   }

}   //  if

// mlog << Debug(2) << "N composites = " << e.n_composites() << "\n";

   //
   //  get single cluster attributes
   //

SingleAtt3DArray fcst_cluster_att, obs_cluster_att;


mlog << Debug(2)
     << "Calculating 3D fcst cluster attributes\n";

for (j=0; j<n_clusters; ++j)  {

   a = e.fcst_composite(j);   //  0-based

   a.increment(1);

   mask = fcst_obj.select(a);   //  1-based

   att_3 = calc_3d_single_atts(mask, fcst_raw, config.model);

   att_3.set_object_number(j + 1);   //  1-based

   att_3.set_fcst();

   att_3.set_cluster();

   fcst_cluster_att.add(att_3);

}

// if ( mlog.verbosity_level() > 5 )  fcst_cluster_att.dump(cout);

mlog << Debug(2)
     << "Calculating 3D obs cluster attributes\n";

for (j=0; j<n_clusters; ++j)  {

   a = e.obs_composite(j);   //  0-based

   a.increment(1);

   mask = obs_obj.select(a);   //  1-based

   att_3 = calc_3d_single_atts(mask, obs_raw, config.model);

   // if ( att.Xvelocity > 20.0 )  mask.write("w.nc");

   att_3.set_object_number(j + 1);   //  1-based

   att_3.set_obs();

   att_3.set_cluster();

   obs_cluster_att.add(att_3);

}

// obs_cluster_att.dump(cout);

   //
   //  get cluster pair attributes
   //

PairAtt3DArray pa_cluster;
IntArray b;

mlog << Debug(2)
     << "Calculating 3D cluster pair attributes\n";

for (j=0; j<n_clusters; ++j)  {

   a = e.fcst_composite(j);   //  0-based

   a.increment(1);

   fo = fcst_obj.select(a);   //  1-based

   for (k=0; k<n_clusters; ++k)  {

      b = e.obs_composite(k);   //  0-based

      b.increment(1);

      oo = obs_obj.select(b);   //  1-based

      p = calc_3d_pair_atts(fo, oo, fcst_cluster_att[j], obs_cluster_att[k]);

      p.set_cluster();

      // p.set_total_interest(e.calc(p));
      p.set_total_interest(-1.0);

      // mlog << Debug(5) << "   (F_" << j << ", O_" << k << ")   "
      //      << p.total_interest() << '\n';

      pa_cluster.add(p);

   }

   // cout.put('\n');

}

   //
   //  patch the cluster ids
   //

fcst_single_att.patch_cluster_numbers(e);
 obs_single_att.patch_cluster_numbers(e);

fcst_cluster_att.patch_cluster_numbers(e);
 obs_cluster_att.patch_cluster_numbers(e);

fcst_att_2d.patch_cluster_numbers(e);
 obs_att_2d.patch_cluster_numbers(e);

pa_simple.patch_cluster_numbers(e);
pa_cluster.patch_cluster_numbers(e);

   //
   //  write 2d attributes for each simple object for each time slice
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_2d_suffix;

mlog << Debug(2)
     << "Creating 2D constant-time slice attributes file: \""
     << path << "\"\n";

do_2d_txt_output(fcst_att_2d, obs_att_2d, config, path);

   //
   //  write simple single attributes
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_3d_single_simple_suffix;

mlog << Debug(2)
     << "Creating 3D single simple attributes file: \""
     << path << "\"\n";

do_3d_single_txt_output(fcst_single_att, obs_single_att, config, path);

   //
   //  write simple pair attributes
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_3d_pair_simple_suffix;

mlog << Debug(2)
     << "Creating 3D pair simple attributes file: \""
     << path << "\"\n";

do_3d_pair_txt_output(pa_simple, config, path);

   //
   //  write cluster single attributes
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_3d_single_cluster_suffix;

mlog << Debug(2)
     << "Creating 3D cluster single attributes file: \""
     << path << "\"\n";

do_3d_single_txt_output(fcst_cluster_att, obs_cluster_att, config, path);

   //
   //  write cluster pair attributes
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << txt_3d_pair_cluster_suffix;

mlog << Debug(2)
     << "Creating 3D cluster pair attributes file: \""
     << path << "\"\n";

do_3d_pair_txt_output(pa_cluster, config, path);

   //
   //  netcdf output
   //

path << cs_erase
     << output_directory << '/'
     << prefix << '_' << nc_suffix;

mlog << Debug(2)
     << "Creating NetCDF file: \""
     << path << "\"\n";


do_mtd_nc_output(config.nc_info, e, fcst_raw, obs_raw, fcst_obj, obs_obj, config, path);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

ConcatString tab;

tab.set_repeat(' ', 10 + program_name.length());

mlog << Error
     << "\n*** Model Evaluation Tools (MET" << met_version << ") ***\n\n"
     << "Usage: " << program_name << "\n"
     << tab << "-fcst     file1 ... file_n | file_list\n"
     << tab << "-obs      file1 ... file_n | file_list\n"
     << tab << "-config   config_file\n"
     << tab << "[ -log    file ]\n"
     << tab << "[ -v      level ]\n"
     << tab << "[ -outdir path ]\n"
     << "\n";


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_fcst (const StringArray & a)

{

fcst_filenames = a;

return;

}


////////////////////////////////////////////////////////////////////////


void set_obs  (const StringArray & a)

{

obs_filenames = a;

return;

}


////////////////////////////////////////////////////////////////////////


void set_config  (const StringArray & a)

{

local_config_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_verbosity  (const StringArray & a)

{

int k = atoi(a[0]);

mlog.set_verbosity_level(k);

return;

}


////////////////////////////////////////////////////////////////////////


void set_logfile  (const StringArray & a)

{

ConcatString filename;

filename = a[0];

mlog.open_log_file(filename);

return;

}


////////////////////////////////////////////////////////////////////////


void set_outdir  (const StringArray & a)

{

output_directory = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


StringArray parse_file_list  (const StringArray & a, const GrdFileType type)

{

int i;
Met2dDataFile *mtddf = (Met2dDataFile *) 0;
Met2dDataFileFactory factory;
StringArray list;

   //
   //  check for empty list
   //
if(a.n_elements() == 0) {
   mlog << Error << "\nparse_file_list() -> "
        << "empty list!\n\n";
   exit(1);
}

   //
   //  attempt to read the first file as a gridded data file
   //
mtddf = factory.new_met_2d_data_file(a[0], type);

   //
   //  if the read was successful, store the list of gridded files.
   //  otherwise, process entries as ASCII files.
   //
if(mtddf)                            list.add(a);
else for(i=0; i<a.n_elements(); i++) list = parse_ascii_file_list(a[0]);

return ( list );

}


////////////////////////////////////////////////////////////////////////

