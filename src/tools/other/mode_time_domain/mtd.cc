

////////////////////////////////////////////////////////////////////////


static const char default_config_path [] = "MET_BASE/config/MTDConfig_default";

static const bool debug = true;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

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

cline.parse();

cout << "\n  Verbosity = " << mlog.verbosity_level() << "\n";

if ( cline.n() != 0 )  usage();   //  should only be config file left on command line

// fcst_filenames.dump(cout);

   //
   //  read the config file
   //

MtdConfigInfo config;
ConcatString default_config_filename;
ConcatString path;


default_config_filename = replace_path(default_config_path);

// cout << "Default config file = \"" << default_config_filename << "\"\n" << flush;


config.read_config(default_config_filename, local_config_filename);

config.process_config(FileType_NcMet, FileType_NcMet);

// config.fcst_info->dump(cout);

// exit ( 1 );

int j, k;
MtdFloatFile fcst_raw, obs_raw;
MtdFloatFile fcst_conv, obs_conv;
MtdIntFile fcst_mask, obs_mask;
MtdIntFile fcst_obj, obs_obj;
MM_Engine e;
const double ti_thresh = config.total_interest_thresh;


if ( debug )  cout << "\n  total interest threshold = " << ti_thresh << "\n\n";

   //
   //  read the data files
   //

mtd_read_data(config, *(config.fcst_info), fcst_filenames, fcst_raw);
mtd_read_data(config, *(config.obs_info),   obs_filenames,  obs_raw);

if ( fcst_raw.delta_t() != obs_raw.delta_t() )  {

   cerr << "\n\n  " << program_name << ": forecast time difference is different that obervation time difference!\n\n";

   exit ( 1 );

}

config.delta_t_seconds = fcst_raw.delta_t();

if ( debug )  {

   fcst_raw.write("fcst_raw.nc");
    obs_raw.write("obs_raw.nc");

}

// exit ( 1 );

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
   //
   //

// cout << "\n  fcst conv radius = " << (config.fcst_conv_radius) << "\n";
// cout << "\n   obs conv radius = " << (config.obs_conv_radius) << "\n";

   //
   //  convolve
   //

 obs_conv =  obs_raw.convolve(config.obs_conv_radius);
fcst_conv = fcst_raw.convolve(config.fcst_conv_radius);

if ( debug )  {

    obs_conv.write("obs_conv.nc");
   fcst_conv.write("fcst_conv.nc");

}

// exit ( 1 );

   //
   //  threshold
   //

fcst_mask = fcst_conv.threshold(config.fcst_conv_thresh);
 obs_mask =  obs_conv.threshold(config.obs_conv_thresh);

   //
   //  number objects
   //

fcst_obj = fcst_mask;
 obs_obj =  obs_mask;

if ( debug )  cout << "Start split\n" << flush;
fcst_obj.split();
if ( debug )  cout << "mid split\n" << flush;
 obs_obj.split();
if ( debug )  cout << "End split\n" << flush;

if ( debug )  {

   fcst_mask.write("fcst_mask.nc");
    obs_mask.write("obs_mask.nc");

   fcst_obj.write("fcst_obj_notoss.nc");
    obs_obj.write("obs_obj_notoss.nc");

}

   //
   //  toss small objects
   //

fcst_obj.toss_small_objects(config.min_volume);
 obs_obj.toss_small_objects(config.min_volume);

if ( debug )  {

   fcst_obj.write("fcst_obj_toss.nc");
    obs_obj.write("obs_obj_toss.nc");

}

if ( debug )  {

   cout << "\n\n  fcst threshold:\n";
   config.fcst_conv_thresh.dump(cout);

   cout << "\n\n  obs threshold:\n";
   config.obs_conv_thresh.dump(cout);

}

// cout << "\n   n_header_3d_cols = " << n_header_3d_cols << '\n';
// cout << "\n   n_3d_single_cols = " << n_3d_single_cols << '\n';

   //
   //  set up the match/merge engine
   //

e.set_size(fcst_obj.n_objects(), obs_obj.n_objects());

   //
   //  get single attributes
   //

SingleAtt3D att;
SingleAtt3DArray fcst_single_att, obs_single_att;
Object mask;

cout << "calculating fcst single atts\n" << flush;

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   mask = fcst_obj.select(j + 1);   //  1-based

   att = calc_3d_single_atts(mask, fcst_raw, config.model);

   att.set_object_number(j + 1);   //  1-based

   att.set_fcst();

   att.set_simple();

   fcst_single_att.add(att);

}

// fcst_single_att.dump(cout);

cout << "calculating obs single atts\n" << flush;

for (j=0; j<(obs_obj.n_objects()); ++j)  {

   mask = obs_obj.select(j + 1);   //  1-based

   att = calc_3d_single_atts(mask, obs_raw, config.model);

   att.set_object_number(j + 1);   //  1-based

   att.set_obs();

   att.set_simple();

   obs_single_att.add(att);

}


   //
   //  write simple single attributes
   //

do_3d_single_txt_output(fcst_single_att, obs_single_att, config, "s.txt");

   //
   //  get simple pair attributes
   //

PairAtt3DArray pa_simple;
PairAtt3D p;
MtdIntFile fo, oo;

cout << "\n\n  calculating pair atts ... (Nf = "
     << (fcst_obj.n_objects()) << ", No = "
     << (obs_obj.n_objects())  << ")\n\n";

for (j=0; j<(fcst_obj.n_objects()); ++j)  {

   fo = fcst_obj.select(j + 1);

   for (k=0; k<(obs_obj.n_objects()); ++k)  {

      oo = obs_obj.select(k + 1);

      p = calc_3d_pair_atts(fo, oo, fcst_single_att[j], obs_single_att[k]);

      p.set_total_interest(e.calc(p));

      cout << "   (F_" << j << ", O_" << k << ")   "
           << p.total_interest() << '\n' << flush;

      pa_simple.add(p);

   }

   cout.put('\n');

}


// if ( debug )  pa.dump(cout);

   //
   //  write simple pair attributes
   //

do_3d_pair_txt_output(pa_simple, config, "p.txt");


   //
   //  create graph
   //

for (j=0; j<(pa_simple.n()); ++j)  {

   if ( pa_simple.total_interest(j) < ti_thresh )  continue;

   e.graph.set_fo_edge(pa_simple.fcst_obj_number(j) - 1, pa_simple.obs_obj_number(j) - 1);

}   //  for j

if ( debug )  e.graph.dump_as_table(cout);

e.do_match_merge();

if ( debug )  e.partition_dump(cout);


IntArray a;
const int n_clusters = e.n_composites();

cout << "N clusters = " << n_clusters << '\n';

for (j=0; j<n_clusters; ++j)  {

   cout << "Fcst objects in composite " << j << ":  ";

   a = e.fcst_composite(j);

   a.dump_one_line(cout);

   cout << "Obs  objects in composite " << j << ":  ";

   a = e.obs_composite(j);

   a.dump_one_line(cout);

   cout << '\n';

}

cout << "N composites = " << e.n_composites() << "\n";

   //
   //  get single cluster attributes
   //

SingleAtt3DArray fcst_cluster_att, obs_cluster_att;


cout << "calculating fcst cluster atts\n" << flush;

for (j=0; j<n_clusters; ++j)  {

   a = e.fcst_composite(j);   //  0-based

   a.increment(1);

   mask = fcst_obj.select(a);   //  1-based

   att = calc_3d_single_atts(mask, fcst_raw, config.model);

   att.set_object_number(j + 1);   //  1-based

   att.set_fcst();

   att.set_cluster();

   fcst_cluster_att.add(att);

}

fcst_cluster_att.dump(cout);

cout << "calculating obs cluster atts\n" << flush;

for (j=0; j<n_clusters; ++j)  {

   a = e.obs_composite(j);   //  0-based

   a.increment(1);

   mask = obs_obj.select(a);   //  1-based

   att = calc_3d_single_atts(mask, obs_raw, config.model);

   // if ( att.Xvelocity > 20.0 )  mask.write("w.nc");

   att.set_object_number(j + 1);   //  1-based

   att.set_obs();

   att.set_cluster();

   obs_cluster_att.add(att);

}

obs_cluster_att.dump(cout);


   //
   //  write cluster single attributes
   //

do_3d_single_txt_output(fcst_cluster_att, obs_cluster_att, config, "cs.txt");


   //
   //  get cluster pair attributes
   //

PairAtt3DArray pa_cluster;
IntArray b;

cout << "\n\n  calculating cluster pair atts ... (Nf = "
     << n_clusters << ", No = "
     << n_clusters << ")\n\n";

for (j=0; j<n_clusters; ++j)  {

   a = e.fcst_composite(j);   //  0-based

   a.increment(1);
   
   fo = fcst_obj.select(a);   //  1-based

   for (k=0; k<n_clusters; ++k)  {

      b = e.obs_composite(k);   //  0-based

      b.increment(1);

      oo = obs_obj.select(b);   //  1-based

      p = calc_3d_pair_atts(fo, oo, fcst_cluster_att[j], obs_cluster_att[k]);

      // p.set_total_interest(e.calc(p));
      p.set_total_interest(-1.0);

      cout << "   (F_" << j << ", O_" << k << ")   "
           << p.total_interest() << '\n' << flush;

      pa_cluster.add(p);

   }

   cout.put('\n');

}


// if ( debug )  pa_cluster.dump(cout);

   //
   //  write cluster pair attributes
   //

do_3d_pair_txt_output(pa_cluster, config, "cp.txt");

   //
   //  netcdf output
   //

do_mtd_nc_output(config.nc_info, e, fcst_raw, obs_raw, fcst_obj, obs_obj, config, "c.nc");


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

cout << "Usage: " << program_name << '\n';

cout << tab << "-fcst   file_list\n";
cout << tab << "-obs    file_list\n";
cout << tab << "-config config_file\n";
cout << tab << "[ -log  file ]\n";
cout << tab << "[ -v    level ]\n";


   //
   //  done
   //

cout << "\n\n";

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





