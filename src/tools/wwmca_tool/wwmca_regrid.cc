

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <cmath>

#include "vx_util.h"

#include "wwmca_regrid_Conf.h"
#include "wwmca_ref.h"


////////////////////////////////////////////////////////////////////////


static const char * default_config_filename = "MET_BASE/data/config/WWMCARegridConfig_default";


////////////////////////////////////////////////////////////////////////


   //
   //  default valued for command-line switches
   //


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static ConcatString cp_nh_filename;
static ConcatString cp_sh_filename;

static ConcatString pt_nh_filename;
static ConcatString pt_sh_filename;

static ConcatString output_filename;


////////////////////////////////////////////////////////////////////////


static CommandLine cline;

static WwmcaRegridder regridder;

static wwmca_regrid_Conf config;

static ConcatString config_filename;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_nh_filename (const StringArray &);
static void set_sh_filename (const StringArray &);
static void set_outfile     (const StringArray &);
static void set_config      (const StringArray &);

static void sanity_check();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

char default_conf_file[PATH_MAX];


program_name = get_short_name(argv[0]);

if ( argc == 1 )  usage();

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_nh_filename, "-nh",     2);
cline.add(set_sh_filename, "-sh",     2);
cline.add(set_outfile,     "-out",    1);
cline.add(set_config,      "-config", 1);

cline.parse();

if ( cline.n() != 0 )  usage();

sanity_check();

   //
   // read the default config file first and then read the user's
   //

replace_string(met_base_str, MET_BASE, default_config_filename, default_conf_file);

cout << "Reading Default Config File: " << default_conf_file << "\n" << flush;

config.read(default_conf_file);

cout << "Reading User Config File: " << config_filename << "\n" << flush;

config.read(config_filename);

   //
   //  load up regridder
   //

if ( cp_nh_filename.length() > 0 )  regridder.set_cp_nh_file(cp_nh_filename);
if ( cp_sh_filename.length() > 0 )  regridder.set_cp_sh_file(cp_sh_filename);
if ( pt_nh_filename.length() > 0 )  regridder.set_pt_nh_file(pt_nh_filename);
if ( pt_sh_filename.length() > 0 )  regridder.set_pt_sh_file(pt_sh_filename);

regridder.set_config(config, config_filename);

   //
   //  done
   //

// regridder.dump(cout);

regridder.do_output(output_filename);









   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name
     << " -out filename"
     << " -config filename"
     << " [ -nh cp_filename pt_filename ]"
     << " [ -sh cp_filename pt_filename ]"
     << "\n\n";


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_nh_filename(const StringArray & a)

{

cp_nh_filename = a[0];
pt_nh_filename = a[1];

return;

}


////////////////////////////////////////////////////////////////////////


void set_sh_filename(const StringArray & a)

{

cp_sh_filename = a[0];
pt_sh_filename = a[1];

return;

}


////////////////////////////////////////////////////////////////////////


void set_outfile(const StringArray & a)

{

output_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_config(const StringArray & a)

{

config_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void sanity_check()

{

bool trouble = false;

   //
   //  check that a config file was given
   //

if ( config_filename.length() == 0 )  {

   trouble = true;

   cerr << "\n\n  " << program_name << ": no config file set!\n\n";

   // exit ( 1 );

}

   //
   //  check that an output file was given
   //

if ( output_filename.length() == 0 )  {

   trouble = true;

   cerr << "\n\n  " << program_name << ": no output file set!\n\n";

   // exit ( 1 );

}

   //
   //  check that at least one input file was given
   //

if ( (cp_nh_filename.length() == 0) && (cp_sh_filename.length() == 0) &&
     (pt_nh_filename.length() == 0) && (pt_sh_filename.length() == 0))  {

   trouble = true;

   cerr << "\n\n  " << program_name << ": no input file(s) set!\n\n";

   // exit ( 1 );

}

if (cp_nh_filename.length() != 0)  {

   if (pt_nh_filename.length() == 0)  {

      trouble = true;

      cerr << "\n\n  " << program_name << ": missing the pixel time file!\n\n";

      // exit ( 1 );

   }
}

if (cp_sh_filename.length() != 0)  {

   if (pt_sh_filename.length() == 0)  {

      trouble = true;

      cerr << "\n\n  " << program_name << ": missing the pixel time file!\n\n";

      // exit ( 1 );

   }
}


   //
   //  done
   //

if ( trouble )  usage();

return;

}


////////////////////////////////////////////////////////////////////////


