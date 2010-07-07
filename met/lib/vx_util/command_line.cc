

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util/indent.h"
#include "vx_util/string_fxns.h"
#include "vx_util/command_line.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class CLOptionInfo
   //


////////////////////////////////////////////////////////////////////////


CLOptionInfo::CLOptionInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo::~CLOptionInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo::CLOptionInfo(const CLOptionInfo & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo & CLOptionInfo::operator=(const CLOptionInfo & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::clear()

{

option_text.clear();

Nargs = 0;

f = (CLSetFunction) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::assign(const CLOptionInfo & i)

{

clear();

option_text = i.option_text;

Nargs = i.Nargs;

f = i.f;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "option_text = ";

if ( option_text.length() == 0 )  out << "(nul)\n";
else                              out << '\"' << option_text << "\"\n";

out << prefix << "Nargs       = " << Nargs << "\n";

out << prefix << "f           = " << (void *) f << "\n";

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class CLOptionInfoArray
   //


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray::CLOptionInfoArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray::~CLOptionInfoArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray::CLOptionInfoArray(const CLOptionInfoArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray & CLOptionInfoArray::operator=(const CLOptionInfoArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::init_from_scratch()

{

e = (CLOptionInfo *) 0;

AllocInc = 16;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::clear()

{

if ( e )  { delete [] e;  e = (CLOptionInfo *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 16;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::assign(const CLOptionInfoArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::extend(int n)

{

if ( n <= Nalloc )  return;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
CLOptionInfo * u = new CLOptionInfo [n];

if ( !u )  {

   cerr << "CLOptionInfoArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (CLOptionInfo *) 0; }

e = u;

u = (CLOptionInfo *) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   e[j].dump(out, depth + 1);

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::set_alloc_inc(int n)

{

if ( n < 0 )  {

   cerr << "CLOptionInfoArray::set_alloc_int(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = 16;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::add(const CLOptionInfo & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::add(const CLOptionInfoArray & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo & CLOptionInfoArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   cerr << "CLOptionInfoArray::operator[](int) -> range check error ... " << n << "\n\n";

   exit ( 1 );
}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


int CLOptionInfoArray::lookup(const char * name) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( strcmp(name, e[j].option_text) == 0 )  return ( j );

}

   //
   //  nope
   //

return ( -1 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CommandLine
   //


////////////////////////////////////////////////////////////////////////


CommandLine::CommandLine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CommandLine::~CommandLine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CommandLine::CommandLine(const CommandLine & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


CommandLine & CommandLine::operator=(const CommandLine & c)

{

if ( this == &c )  return ( * this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CommandLine::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void CommandLine::clear()

{

args.clear();

options.clear();

ProgramName.clear();

Usage = (UsageFunction) 0;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::assign(const CommandLine & c)

{

clear();

args = c.args;

options = c.options;

ProgramName = c.ProgramName;

Usage = c.Usage;

return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::dump(ostream & out, int depth) const

{

out << "Program Name = ";

if ( ProgramName.length() > 0 )  out << '\"' << ProgramName << "\"\n";
else                             out << "(nul)\n";

out << "Args ...\n";

args.dump(out, depth + 1);

out << "Options ...\n";

options.dump(out, depth + 1);

return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::set(int argc, char ** argv)

{

clear();

int j;

ProgramName = get_short_name(argv[0]);

for (j=1; j<argc; ++j)  {   //  j starts at one here, not zero

   args.add(argv[j]);

}


return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::shift_down(int pos, int k)

{

   //
   //  any necessary error-checking is done by
   //
   //    StringArray::shift_down()
   //

args.shift_down(pos, k);

return;

}


////////////////////////////////////////////////////////////////////////


const char * CommandLine::operator[](int k) const

{

   //
   //  any necessary error-checking is done by
   //
   //    StringArray::operator[](int)
   //

const char * c = (const char *) 0;

c = args[k];

return ( c );

}


////////////////////////////////////////////////////////////////////////


int CommandLine::has_option(int & index) const

{

int status;

status = args.has_option(index);

return ( status );

}


////////////////////////////////////////////////////////////////////////


int CommandLine::next_option() const

{

int j, N;


N = args.n_elements();

for (j=0; j<N; ++j)  {

   if ( args[j][0] == '-' )   return ( j );

}

   //
   //  nope
   //

return ( -1 );

}


////////////////////////////////////////////////////////////////////////


void CommandLine::add(CLSetFunction func, const char * text, int n_args)

{

CLOptionInfo info;

info.option_text = text;

info.Nargs = n_args;

info.f = func;

options.add(info);


return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::do_help()

{

if ( Usage )  Usage();
else {

   cerr << "\n\n  " << ProgramName << ": no usage message available ... sorry.\n\n";

}


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::parse()

{

int j, k, m, index;
StringArray a;
const char *  c = (const char *) 0;
const char * cc = (const char *) 0;


while ( args.has_option(j) )  {

   c = args[j];

      //
      //  see if it's "-help" or "--help"
      //

   if ( (strcmp(c, "-help") == 0) || (strcmp(c, "--help") == 0) )  do_help();

      //
      //  look up entry in options array
      //

   index = options.lookup(c);

   if ( index < 0 )  {

      cerr << "\n\n  " << ProgramName << ": unrecognized command-line switch: \"" << c << "\"\n\n";

      if ( Usage )  Usage();

      exit ( 1 );

   }

      //
      //  set up the arguments
      //

   a.clear();

   for (k=0; k<(options[index].Nargs); ++k)  {

      m = j + k + 1;

      if ( m >= args.n_elements() )  {

         cerr << "\n\n  " << ProgramName << ": too few arguments to command-line switch \"" << c << "\"\n\n";

         if ( Usage )  Usage();

         exit ( 1 );

      }

      cc = args[m];

      if ( *cc == '-' )  {   //  we ran into another command-line switch

         cerr << "\n\n  " << ProgramName << ": too few arguments to command-line switch \"" << c << "\"\n\n";

         if ( Usage )  Usage();

         exit ( 1 );

      }

      a.add(cc);

   }   //  for k

   args.shift_down(j, options[index].Nargs + 1);

      //
      //  call the function
      //

   options[index].f(a);

}   //  while

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////





