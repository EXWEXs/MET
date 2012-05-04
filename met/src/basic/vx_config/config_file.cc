

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "indent.h"
#include "vx_log.h"
#include "empty_string.h"
#include "bool_to_string.h"
#include "is_bad_data.h"

#include "config_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  external linkage
   //


extern int configparse();

extern FILE * configin;

extern int configdebug;

extern const char * bison_input_filename;

extern DictionaryStack * dict_stack;

extern int LineNumber;

extern int Column;

extern bool is_lhs;

extern void start_string_scan  (const char *);
extern void finish_string_scan ();


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetConfig
   //


////////////////////////////////////////////////////////////////////////


MetConfig::MetConfig()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetConfig::~MetConfig()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MetConfig::MetConfig(const MetConfig & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


MetConfig::MetConfig(const char *filename)

{

init_from_scratch();

read(filename);

}


////////////////////////////////////////////////////////////////////////


void MetConfig::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::clear()

{

Filename.clear();

Dictionary::clear();

Debug = false;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::assign(const MetConfig & c)

{

clear();

Filename = c.Filename;

Dictionary::assign(c);

Debug = c.Debug;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Filename ... \n";

Filename.dump(out, depth + 1);

out << prefix << "\n";

out << prefix << "Debug    = "   << bool_to_string(Debug) << "\n";

out << prefix << "Config File Entries ...\n";

Dictionary::dump(out, depth + 1);


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::set_debug(bool tf)

{

Debug = tf;

return;

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read(const char * name)

{

if ( empty(name) )  {

   mlog << Error
        << "\nMetConfig::read(const char *) -> "
        << "empty filename!\n\n";

   exit ( 1 );

}

// clear();

DictionaryStack DS(*this);

bison_input_filename = name;

dict_stack = &DS;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

Filename.add(bison_input_filename);

configdebug = (Debug ? 1 : 0);

if ( (configin = fopen(bison_input_filename, "r")) == NULL )  {

   mlog << Error
        << "\nMetConfig::read(const char *) -> "
        << "unable to open input file \"" << name << "\"\n\n";

   exit ( 1 );

}

int parse_status;

parse_status = configparse();

if ( parse_status != 0 )  {

   return ( false );

}

// DS.pop_dict("base");

if ( DS.n_elements() != 1 )  {

   mlog << Error 
        << "\nMetConfig::read(const char *) -> "
        << "should be only one dictionary left after parsing! ...("
        << DS.n_elements() << ")\n\n";

   DS.dump(cout);
   DS.dump_config_format(cout);

   mlog << Error 
        << "\n"
        << "  parse failed!\n"
        << "\n";

   exit ( 1 );

}

   //
   //  done
   //

patch_parents();

bison_input_filename = (const char *) 0;

dict_stack = (DictionaryStack *) 0;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read_string(const char * s)

{

if ( empty(s) )  {

   mlog << Error
        << "\nMetConfig::read_string() -> "
        << "empty string!\n\n";

   exit ( 1 );

}

// clear();

DictionaryStack DS(*this);

bison_input_filename = "string";

dict_stack = &DS;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

Filename.add(bison_input_filename);

configdebug = (Debug ? 1 : 0);

start_string_scan(s);

/*
if ( (configin = fopen(bison_input_filename, "r")) == NULL )  {

   mlog << Error
        << "\nMetConfig::read(const char *) -> "
        << "unable to open input file \"" << name << "\"\n\n";

   exit ( 1 );

}
*/

int parse_status;

parse_status = configparse();

if ( parse_status != 0 )  {

   return ( false );

}

finish_string_scan ();

// yy_delete_buffer(bp);

// DS.pop_dict("base");

if ( DS.n_elements() != 1 )  {

   mlog << Error 
        << "\nMetConfig::read(const char *) -> "
        << "should be only one dictionary left after parsing! ...("
        << DS.n_elements() << ")\n\n";

   DS.dump(cout);
   DS.dump_config_format(cout);

   mlog << Error 
        << "\n"
        << "  parse failed!\n"
        << "\n";

   exit ( 1 );

}

   //
   //  done
   //

bison_input_filename = (const char *) 0;

dict_stack = (DictionaryStack *) 0;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

return ( true );

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * MetConfig::lookup(const char * name)

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = Dictionary::lookup(name);

LastLookupStatus = (e != 0);

return ( e );

}


////////////////////////////////////////////////////////////////////////
