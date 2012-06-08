

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <vector>

#include "table_lookup.h"
#include "vx_util.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


   //
   //  This needs external linkage
   //

TableFlatFile GribTable (0);


////////////////////////////////////////////////////////////////////////


static const char table_data_dir   [] = "MET_BASE/data/table_files"; //  relative to MET_BASE

static const char grib1_table_file [] = "nceptab_flat.txt";          //  relative to table_data_dir

static const char grib2_table_file [] = "grib2_vars_flat.txt";       //  relative to table_data_dir


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Grib1TableEntry
   //


////////////////////////////////////////////////////////////////////////


Grib1TableEntry::Grib1TableEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Grib1TableEntry::~Grib1TableEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Grib1TableEntry::Grib1TableEntry(const Grib1TableEntry & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


Grib1TableEntry & Grib1TableEntry::operator=(const Grib1TableEntry & e)

{

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::clear()

{

code = table_number = -1;

parm_name.clear();

full_name.clear();

units.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::assign(const Grib1TableEntry & e)

{

clear();

code = e.code;
table_number = e.table_number;

parm_name = e.parm_name;

full_name = e.full_name;

units = e.units;

return;

}


////////////////////////////////////////////////////////////////////////


void Grib1TableEntry::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Index values = (" 
              << code << ", "
              << table_number << ")\n";

out << prefix << "parm_name = " << parm_name.contents() << "\n";

out << prefix << "full_name = " << full_name.contents() << "\n";

out << prefix << "units     = " << units.contents()     << "\n";

return;

}


////////////////////////////////////////////////////////////////////////


bool Grib1TableEntry::parse_line(const char * line)

{

clear();

int j, n;
int * i[3];
const char i_delim [] = "{} \"";
const char s_delim [] = "\"";
const char * c = (const char *) 0;
char * line2 = (char *) 0;
char * L = (char *) 0;

n = strlen(line);

line2 = new char [1 + n];

memset(line2, 0, 1 + n);

strncpy(line2, line, n);

L = line2;

clear();

i[0] = &code;
i[1] = &table_number;

   //
   //  grab the first 2 ints
   //

for (j=0; j<2; ++j)  {

   c = strtok(L, i_delim);

   *(i[j]) = atoi(c);

   L = (char *) 0;

}   //  while

   //
   //  parm_name
   //

c = strtok(0, s_delim);

parm_name = c;

c = strtok(0, s_delim);

   //
   //  full_name
   //

c = strtok(0, s_delim);

full_name = c;

c = strtok(0, s_delim);

   //
   //  units (may be empty)
   //

c = strtok(0, s_delim);

if ( *c == ' ' )  units.clear();
else              units = c;

   //
   //  done
   //

delete [] line2;  line2 = (char *) 0;

return ( true );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Grib2TableEntry
   //


////////////////////////////////////////////////////////////////////////


Grib2TableEntry::Grib2TableEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Grib2TableEntry::~Grib2TableEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Grib2TableEntry::Grib2TableEntry(const Grib2TableEntry & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


Grib2TableEntry & Grib2TableEntry::operator=(const Grib2TableEntry & e)

{

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::clear()

{

index_a = index_b = index_c = -1;

parm_name.clear();

full_name.clear();

units.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::assign(const Grib2TableEntry & e)

{

clear();

index_a = e.index_a;
index_b = e.index_b;
index_c = e.index_c;

parm_name = e.parm_name;

full_name = e.full_name;

units = e.units;

return;

}


////////////////////////////////////////////////////////////////////////


void Grib2TableEntry::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Index values = (" 
              << index_a << ", "
              << index_b << ", "
              << index_c << ")\n";

out << prefix << "parm_name = " << parm_name.contents() << "\n";

out << prefix << "full_name = " << full_name.contents() << "\n";

out << prefix << "units     = " << units.contents()     << "\n";

return;

}


////////////////////////////////////////////////////////////////////////


bool Grib2TableEntry::parse_line(const char * line)

{

clear();

int j, n;
int * i[3];
ConcatString * s[3];
const char i_delim [] = "{} \"";
const char s_delim [] = "\"";
const char * c = (const char *) 0;
char * line2 = (char *) 0;
char * L = (char *) 0;

n = strlen(line);

line2 = new char [1 + n];

strncpy(line2, line, n);

L = line2;

clear();

i[0] = &index_a;
i[1] = &index_b;
i[2] = &index_c;

s[0] = &parm_name;
s[1] = &full_name;
s[2] = &units;

   //
   //  grab the first 3 ints
   //

for (j=0; j<3; ++j)  {

   c = strtok(L, i_delim);

   *(i[j]) = atoi(c);

   L = (char *) 0;

}   //  while

   //
   //  grab the strings
   //

for (j=0; j<3; ++j)  {

   c = strtok(L, s_delim);
   c = strtok(L, s_delim);

   *(s[j]) = c;

   L = (char *) 0;

}   //  while

   //
   //  done
   //

delete [] line2;  line2 = (char *) 0;

return ( true );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TableFlatFile
   //


////////////////////////////////////////////////////////////////////////


TableFlatFile::TableFlatFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TableFlatFile::TableFlatFile(int)

{

init_from_scratch();

ConcatString path;

   //
   //  read the default grib1 table file, expanding MET_BASE
   //

path << cs_erase << table_data_dir << '/' << grib1_table_file;

path = replace_path(path);

if ( ! read(path) )  {

   mlog << Error
        << "TableFlatFile::TableFlatFile(int) -> unable to read table file \"" << path << "\"\n\n";

   exit ( 1 );

}

   //
   //  read the default grib2 table file, expanding MET_BASE
   //

path << cs_erase << table_data_dir << '/' << grib2_table_file;

path = replace_path(path);

if ( ! read(path) )  {

   mlog << Error
        << "TableFlatFile::TableFlatFile(int) -> unable to read table file \"" << path << "\"\n\n";

   exit ( 1 );

}


   //
   //  done
   //

}


////////////////////////////////////////////////////////////////////////


TableFlatFile::~TableFlatFile()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TableFlatFile::TableFlatFile(const TableFlatFile & f)

{

init_from_scratch();

assign(f);

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::init_from_scratch()

{

g1e = (Grib1TableEntry **) 0;
g2e = (Grib2TableEntry **) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::clear()

{

int j;

if ( g1e )  { 

   for (j=0; j<N_grib1_elements; ++j)  {

      if ( g1e[j] )  { delete g1e[j];  g1e[j] = (Grib1TableEntry *) 0; }

   }

   delete [] g1e; g1e = (Grib1TableEntry **) 0;

}


if ( g2e )  { 

   for (j=0; j<N_grib2_elements; ++j)  {

      if ( g2e[j] )  { delete g2e[j];  g2e[j] = (Grib2TableEntry *) 0; }

   }

   delete [] g2e; g2e = (Grib2TableEntry* *) 0;

}

N_grib1_elements = 0;
N_grib2_elements = 0;

N_grib1_alloc = 0;
N_grib2_alloc = 0;

return;

}

////////////////////////////////////////////////////////////////////////


void TableFlatFile::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);


out << prefix << "N_grib1_elements = " << N_grib1_elements << "\n";

for (j=0; j<N_grib1_elements; ++j)  {

   out << prefix << "Grib1 Element # " << j << " ...\n";

   g1e[j]->dump(out, depth + 1);

}

out << prefix << "N_grib2_elements = " << N_grib2_elements << "\n";

for (j=0; j<N_grib2_elements; ++j)  {

   out << prefix << "Grib2 Element # " << j << " ...\n";

   g2e[j]->dump(out, depth + 1);

}


return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::assign(const TableFlatFile & f)

{

clear();

int j;


if ( f.N_grib1_elements != 0 )  {

   N_grib1_elements = N_grib1_alloc = f.N_grib1_elements;

   g1e = new Grib1TableEntry * [N_grib1_elements];

   for (j=0; j<N_grib1_elements; ++j)  {

      g1e[j] = new Grib1TableEntry;

      *(g1e[j]) = *(f.g1e[j]);

   }

}

if ( f.N_grib2_elements != 0 )  {

   N_grib2_elements = N_grib2_alloc = f.N_grib2_elements;

   g2e = new Grib2TableEntry * [N_grib2_elements];

   for (j=0; j<N_grib2_elements; ++j)  {

      g2e[j] = new Grib2TableEntry;

      *(g2e[j]) = *(f.g2e[j]);

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::extend_grib1(int n)

{

if ( n <= N_grib1_alloc )  return;

int j;
Grib1TableEntry ** u = (Grib1TableEntry **) 0;

u = new Grib1TableEntry * [n];

for (j=0; j<n; ++j)  u[j] = (Grib1TableEntry *) 0;

if ( N_grib1_elements > 0 )  {

   for (j=0; j<N_grib1_elements; ++j)  u[j] = g1e[j];

   delete [] g1e;  g1e = (Grib1TableEntry **) 0;

}

g1e = u;

u = (Grib1TableEntry **) 0;

   //
   //  done
   //

N_grib1_alloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void TableFlatFile::extend_grib2(int n)

{

if ( n <= N_grib2_alloc )  return;

int j;
Grib2TableEntry ** u = (Grib2TableEntry **) 0;

u = new Grib2TableEntry * [n];

for (j=0; j<n; ++j)  u[j] = (Grib2TableEntry *) 0;

if ( N_grib2_elements > 0 )  {

   for (j=0; j<N_grib2_elements; ++j)  u[j] = g2e[j];

   delete [] g2e;  g2e = (Grib2TableEntry **) 0;

}

g2e = u;

u = (Grib2TableEntry **) 0;

   //
   //  done
   //

N_grib2_alloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read(const char * filename)

{

// clear();

ifstream in;
ConcatString line;
int n_lines;

if ( empty(filename) )  {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> empty filename!\n\n";

   exit ( 1 );

}

n_lines = file_linecount(filename);

in.open(filename);

if ( !in )  {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  get first line for format
   //

line.read_line(in);

line.chomp('\n');

     if ( line == "GRIB1" )  { return ( read_grib1(in, filename, n_lines - 1) ); }
else if ( line == "GRIB2" )  { return ( read_grib2(in, filename, n_lines - 1) ); }
else {

   mlog << Error 
        << "TableFlatFile::read(const char *) -> unable unrecognized format spec \""
        << line << "\" in file \"" << filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

in.close();

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib1(istream & in, const char * filename, const int n)

{

int j;
ConcatString line;
bool status = false;

   //
   //  make room for the new elements
   //

extend_grib1(N_grib1_elements + n);

   //
   //  shuffle all the old elements to the end of the array, 
   //
   //   so that the new stuff gets prepended 
   //

for (j=(N_grib1_elements - 1); j>=0; --j)  {

   g1e[j + n] = g1e[j];

}

for (j=0; j<n; ++j)  g1e[j] = (Grib1TableEntry *) 0;

   //
   //  read the new elements
   //

for (j=0; j<n; ++j)  {

   status = line.read_line(in);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib1(istream &) -> trouble reading file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

   g1e[j] = new Grib1TableEntry;

   status = g1e[j]->parse_line(line);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib1(istream &) -> trouble parsing line \""
           << line << "\" from input file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

}   //  for j


   //
   //  done
   //

N_grib1_elements += n;

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::read_grib2(istream & in, const char * filename, const int n)

{

int j;
ConcatString line;
bool status = false;

   //
   //  make room for the new elements
   //

extend_grib2(N_grib2_elements + n);

   //
   //  shuffle all the old elements to the end of the array, 
   //
   //   so that the new stuff gets prepended 
   //

for (j=(N_grib2_elements - 1); j>=0; --j)  {

   g2e[j + n] = g2e[j];

}

for (j=0; j<n; ++j)  g2e[j] = (Grib2TableEntry *) 0;

   //
   //  read the new elements
   //

for (j=0; j<n; ++j)  {

   status = line.read_line(in);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib2(istream &) -> trouble reading file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

   g2e[j] = new Grib2TableEntry;

   status = g2e[j]->parse_line(line);

   if ( ! status )  {

      mlog << Error
           << "\n\n  TableFlatFile::read_grib2(istream &) -> trouble parsing line \""
           << line << "\" from input file \"" << filename << "\"\n\n";

      exit ( 1 );

   }

}   //  for j


   //
   //  done
   //

N_grib2_elements += n;

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(int code, int table_number, Grib1TableEntry & e)

{

int j;

e.clear();

for (j=0; j<N_grib1_elements; ++j)  {

   if ( (g1e[j]->code == code) && (g1e[j]->table_number == table_number) )  {

      e = *(g1e[j]);

      return ( true );

   }

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(int code, Grib1TableEntry & e)   //  assumes table_number = 2;

{

const int table_number = 2;
bool status = false;

status = lookup_grib1(code, table_number, e);

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(const char * parm_name, int table_number, int code,
                                 Grib1TableEntry & e, int & n_matches)

{

   //  clear the by-reference arguments
   e.clear();
   n_matches = 0;

   //  build a list of matches
   vector<Grib1TableEntry*> matches;
   for(int j=0; j < N_grib1_elements; j++){

      if( g1e[j]->parm_name != parm_name ||
          (bad_data_int != table_number && g1e[j]->table_number != table_number) ||
          (bad_data_int != code         && g1e[j]->code         != code        ) )
         continue;

      if( n_matches++ == 0 ) e = *(g1e[j]);
      matches.push_back( g1e[j] );

   }

   //  if there are multiple matches, print a descriptive warning
   if( 1 < n_matches ){

      ConcatString msg;
      msg << "Multiple GRIB1 table entries match lookup criteria ("
          << "parm_name = " << parm_name;
      if( bad_data_int != table_number ) msg << ", table_number = " << table_number;
      if( bad_data_int != code         ) msg << ", code = "         << code;
      msg << "):\n";
      mlog << Warning << "\n" << msg;

      for(vector<Grib1TableEntry*>::iterator it = matches.begin();
          it < matches.end(); it++)
         mlog << Warning << "  parm_name: "      << (*it)->parm_name
                         << ", table_number = "  << (*it)->table_number
                         << ", code = "          << (*it)->code << "\n";

      mlog << Warning << "Using: "
           << "  parm_name: "      << e.parm_name
           << ", table_number = "  << e.table_number
           << ", code = "          << e.code << "\n\n";

   }

   return (n_matches > 0);

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib1(const char * parm_name, Grib1TableEntry & e)   //  assumes table number is 2

{

   int n_matches = -1;
   return lookup_grib1(parm_name, 2, bad_data_int, e, n_matches);

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(int a, int b, int c, Grib2TableEntry & e)

{

int j;

e.clear();

for (j=0; j<N_grib2_elements; ++j)  {

   if ( (g2e[j]->index_a == a) && (g2e[j]->index_b == b) && (g2e[j]->index_c == c) )  {

      e = *(g2e[j]);

      return ( true );

   }

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(const char * parm_name, int a, int b, int c,
                                 Grib2TableEntry & e, int & n_matches)

{

   //  clear the by-reference arguments
   e.clear();
   n_matches = 0;

   //  build a list of matches
   vector<Grib2TableEntry*> matches;
   for(int j=0; j<N_grib2_elements; ++j){

      if( g2e[j]->parm_name != parm_name ||
          (bad_data_int != a && g2e[j]->index_a != a) ||
          (bad_data_int != b && g2e[j]->index_b != b) ||
          (bad_data_int != c && g2e[j]->index_c != c) )
         continue;

      if( n_matches++ == 0 ) e = *(g2e[j]);
      matches.push_back( g2e[j] );

   }

   //  if there are multiple matches, print a descriptive warning
   if( 1 < n_matches ){

      ConcatString msg;
      msg << "Multiple GRIB2 table entries match lookup criteria ("
          << "parm_name = " << parm_name;
      if( bad_data_int != a ) msg << ", index_a = " << a;
      if( bad_data_int != b ) msg << ", index_b = " << b;
      if( bad_data_int != c ) msg << ", index_c = " << c;
      msg << "):\n";
      mlog << Warning << "\n" << msg;

      for(vector<Grib2TableEntry*>::iterator it = matches.begin();
          it < matches.end(); it++)
         mlog << Warning << "  parm_name: " << (*it)->parm_name
                         << ", index_a = "  << (*it)->index_a
                         << ", index_b = "  << (*it)->index_b
                         << ", index_c = "  << (*it)->index_c << "\n";

      mlog << Warning << "Using: "
           << "  parm_name: " << e.parm_name
           << ", index_a = "  << e.index_a
           << ", index_b = "  << e.index_b
           << ", index_c = "  << e.index_c << "\n\n";

   }

   return (n_matches > 0);

}


////////////////////////////////////////////////////////////////////////


bool TableFlatFile::lookup_grib2(const char * parm_name, Grib2TableEntry & e, int & n_matches)

{

   return lookup_grib2(parm_name, bad_data_int, bad_data_int, bad_data_int, e, n_matches);

}


////////////////////////////////////////////////////////////////////////

