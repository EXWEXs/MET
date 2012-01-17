

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "pinterp_file.h"
#include "get_pinterp_grid.h"
#include "data2d_nc_pinterp_utils.h"


////////////////////////////////////////////////////////////////////////


static const char x_dim_name           [] = "west_east";
static const char y_dim_name           [] = "south_north";
static const char z_dim_name           [] = "num_metgrid_levels";
static const char t_dim_name           [] = "Time";

static const char  month_var_name      [] = "month";
static const char    day_var_name      [] = "day";
static const char   year_var_name      [] = "year";
static const char   hour_var_name      [] = "hour";
static const char minute_var_name      [] = "minute";
static const char second_var_name      [] = "second";

static const char pressure_var_name    [] = "pressure";

static const char init_time_att_name   [] = "SIMULATION_START_DATE";

static const char description_att_name [] = "description";
static const char units_att_name       [] = "units";

static const int max_pinterp_args         = 30;

static const double pinterp_missing       = 1.0e35;

static const char *accum_var_names     [] = { "ACGRDFLX", "CUPPT",
                                              "RAINC",    "RAINNC",
                                              "SNOWNC",   "GRAUPELNC",
                                              "ACHFX",    "ACLHF" };
static const int n_accum_var_names        = sizeof(accum_var_names)/sizeof(*accum_var_names);

////////////////////////////////////////////////////////////////////////


static unixtime parse_init_time(const char *);

static bool is_bad_data_pinterp(double);

static bool is_accumulation(const char *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PinterpFile
   //


////////////////////////////////////////////////////////////////////////


PinterpFile::PinterpFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PinterpFile::~PinterpFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void PinterpFile::init_from_scratch()

{

Nc = (NcFile *) 0;

Dim = (NcDim **) 0;

Var = (NcVarInfo *) 0;

Time = (unixtime *) 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void PinterpFile::close()

{

if ( Nc )  { delete Nc;  Nc = (NcFile *) 0; }

if ( Dim )  { delete [] Dim;  Dim = (NcDim **) 0; }

if ( Time )  { delete [] Time;  Time = (unixtime *) 0; }

Ndims = 0;

DimNames.clear();

Xdim = Ydim = Zdim = Tdim = (NcDim *) 0;

Nvars = 0;

if ( Var )  { delete [] Var;  Var = (NcVarInfo *) 0; }

InitTime = (unixtime) 0;

Ntimes = 0;

PressureIndex = -1;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool PinterpFile::open(const char * filename)

{

int j, k;
int month, day, year, hour, minute, second;
const char * c = (const char *) 0;
NcVar * v   = (NcVar *) 0;
NcAtt * att = (NcAtt *) 0;


close();

Nc = new NcFile (filename);

if ( !(Nc->is_valid()) )  { close();  return ( false ); }

   //
   //  grid
   //

if ( ! get_pinterp_grid(*Nc, grid) )  { close();  return ( false ); }

   //
   //  dimensions
   //

Ndims = Nc->num_dims();

Dim = new NcDim * [Ndims];

DimNames.extend(Ndims);

for (j=0; j<Ndims; ++j)  {

   Dim[j] = Nc->get_dim(j);

   if ( strcmp(Dim[j]->name(), "Time") == 0 )  Ntimes = Dim[j]->size();

   DimNames.add(Dim[j]->name());

}

for (j=0; j<Ndims; ++j)  {

   c = Dim[j]->name();

   if ( strcmp(c, x_dim_name) == 0 )  Xdim = Dim[j];
   if ( strcmp(c, y_dim_name) == 0 )  Ydim = Dim[j];
   if ( strcmp(c, z_dim_name) == 0 )  Zdim = Dim[j];
   if ( strcmp(c, t_dim_name) == 0 )  Tdim = Dim[j];

}

   //
   //  times
   //

if ( Ntimes == 0 )  { close();  return ( false ); }

Time = new unixtime [Ntimes];

for (j=0; j<Ntimes; ++j)  {

   month  = get_int_var(Nc,  month_var_name, j);
   day    = get_int_var(Nc,    day_var_name, j);
   year   = get_int_var(Nc,   year_var_name, j);
   hour   = get_int_var(Nc,   hour_var_name, j);
   minute = get_int_var(Nc, minute_var_name, j);
   second = get_int_var(Nc, second_var_name, j);

   Time[j] = mdyhms_to_unix(month, day, year, hour, minute, second);

}   //  for j

att = Nc->get_att(init_time_att_name);

InitTime = parse_init_time(att->as_string(0));

   //
   //  variables
   //

Nvars = Nc->num_vars();

Var = new NcVarInfo [Nvars];

for (j=0; j<Nvars; ++j)  {

   v = Nc->get_var(j);

   Var[j].var = v;

   Var[j].name = v->name();

   Var[j].Ndims = v->num_dims();

   Var[j].Dims = new NcDim * [Var[j].Ndims];

   //
   //  parse the variable attributes
   //

   get_att_str( Var[j], description_att_name, Var[j].long_name_att );
   get_att_str( Var[j], units_att_name,       Var[j].units_att     );

   if ( strcmp(v->name(), pressure_var_name) == 0 )  PressureIndex = j;

   for (k=0; k<(Var[j].Ndims); ++k)  {

      Var[j].Dims[k] = v->get_dim(k);

      if ( Var[j].Dims[k] == Xdim )  Var[j].x_slot = k;
      if ( Var[j].Dims[k] == Ydim )  Var[j].y_slot = k;
      if ( Var[j].Dims[k] == Zdim )  Var[j].z_slot = k;
      if ( Var[j].Dims[k] == Tdim )  Var[j].t_slot = k;

   }   //  for k

}   //  for j

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void PinterpFile::dump(ostream & out, int depth) const

{

int j, k;
int month, day, year, hour, minute, second;
char junk[256];
Indent prefix(depth);
Indent p2(depth + 1);
Indent p3(depth + 2);


out << prefix << "Grid ...\n";

grid.dump(out, depth + 1);

out << prefix << "\n";

out << prefix << "Nc = " << (Nc ? "ok" : "(nul)") << "\n";

out << prefix << "\n";

out << prefix << "Ndims = " << Ndims << "\n";

for (j=0; j<Ndims; ++j)  {

   out << p2 << "Dim # " << j << " = " << DimNames[j] << "   (" << (Dim[j]->size()) << ")\n";

}   //  for j

out << prefix << "\n";

out << prefix << "Xdim = " << (Xdim ? Xdim->name() : "(nul)") << "\n";
out << prefix << "Ydim = " << (Ydim ? Ydim->name() : "(nul)") << "\n";
out << prefix << "Zdim = " << (Zdim ? Zdim->name() : "(nul)") << "\n";
out << prefix << "Tdim = " << (Zdim ? Tdim->name() : "(nul)") << "\n";

out << prefix << "\n";

out << prefix << "Ntimes = " << Ntimes << "\n";

for (j=0; j<Ntimes; ++j)  {

   out << p2 << "Time # " << j << " = ";

   unix_to_mdyhms(Time[j], month, day, year, hour, minute, second);

   sprintf(junk, "%s %d, %d   %2d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

   out << junk << "\n";

}

out << prefix << "\n";

out << prefix << "Init Time = ";

unix_to_mdyhms(InitTime, month, day, year, hour, minute, second);

sprintf(junk, "%s %d, %d   %2d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);

out << junk << "\n";

out << prefix << "\n";

out << prefix << "Nvars = " << Nvars << "\n";

for (j=0; j<Nvars; ++j)  {

   out << p2 << "Var # " << j << " = " << (Var[j].name) << "  (";

   for (k=0; k<(Var[j].Ndims); ++k)  {

           if ( Var[j].Dims[k] == Xdim )  out << 'X';
      else if ( Var[j].Dims[k] == Ydim )  out << 'Y';
      else if ( Var[j].Dims[k] == Zdim )  out << 'Z';
      else if ( Var[j].Dims[k] == Tdim )  out << 'T';
      else                                out << Var[j].Dims[k]->name();

      if ( k < Var[j].Ndims - 1)  out << ", ";

   }   //  for k

   out << ")\n";

   out << p3 << "Slots (X, Y, Z, T) = (";

   if ( Var[j].x_slot >= 0 ) out << Var[j].x_slot; else out << '_';  out << ", ";
   if ( Var[j].y_slot >= 0 ) out << Var[j].y_slot; else out << '_';  out << ", ";
   if ( Var[j].z_slot >= 0 ) out << Var[j].z_slot; else out << '_';  out << ", ";
   if ( Var[j].t_slot >= 0 ) out << Var[j].t_slot; else out << '_';

   out << ")\n";

   out << p2 << "\n";

}   //  for j



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


unixtime PinterpFile::valid_time(int n) const

{

if ( (n < 0) || (n >= Ntimes) )  {

   mlog << Error << "\n\n  PinterpFile::valid_time(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( Time [n] );

}


////////////////////////////////////////////////////////////////////////


int PinterpFile::lead_time(int n) const

{

if ( (n < 0) || (n >= Ntimes) )  {

   mlog << Error << "\n\n  PinterpFile::lead_time(int) const -> range check error\n\n";

   exit ( 1 );

}

unixtime dt = Time[n] - InitTime;

return ( (int) dt );

}


////////////////////////////////////////////////////////////////////////


double PinterpFile::data(NcVar * var, const LongArray & a) const

{

if ( !args_ok(a) )  {

   mlog << Error << "\n\n  PinterpFile::data(NcVar *, const LongArray &) const -> "
        << "bad arguments:\n";

   a.dump(cerr);

   exit ( 1 );

}

if ( var->num_dims() != a.n_elements() )  {

   mlog << Error << "\n\n  PinterpFile::data(NcVar *, const LongArray &) const -> "
        << "needed " << (var->num_dims()) << " arguments for variable "
        << (var->name()) << ", got " << (a.n_elements()) << "\n\n";

   exit ( 1 );

}

if ( var->num_dims() >= max_pinterp_args )  {

   mlog << Error << "\n\n  PinterpFile::data(NcVar *, const LongArray &) const -> "
        << " too may arguments for variable \"" << (var->name()) << "\"\n\n";

   exit ( 1 );

}

int j;
float f[2];
double d[2];
bool status;
long counts[max_pinterp_args];

for (j=0; j<(a.n_elements()); ++j) counts[j] = 1;

if ( !(var->set_cur((long *) a)) )  {

   mlog << Error << "\n\n  PinterpFile::data(NcVar *, const LongArray &) const -> "
        << " can't set corner for variable \"" << (var->name()) << "\"\n\n";

   exit ( 1 );

}

switch ( var->type() )  {

   case ncFloat:
      status = var->get(f, counts);
      d[0] = (double) (f[0]);
      break;

   case ncDouble:
      status = var->get(d, counts);
      break;

   default:
      mlog << Error << "\n\n  PinterpFile::data(NcVar *, const LongArray &) const -> "
           << " bad type for variable \"" << (var->name()) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch



   //
   //  done
   //

return ( d[0] );

}


////////////////////////////////////////////////////////////////////////


bool PinterpFile::data(NcVar * v, const LongArray & a, DataPlane & plane, double & pressure) const

{

if ( !args_ok(a) )  {

   mlog << Warning
        << "\n\n  PinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << "bad arguments:\n";

   a.dump(cerr);

   return ( false );

}

if ( v->num_dims() != a.n_elements() )  {

   mlog << Warning
        << "\n\n  PinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << "needed " << (v->num_dims()) << " arguments for variable "
        << (v->name()) << ", got " << (a.n_elements()) << "\n\n";

   return ( false );

}

if ( v->num_dims() >= max_pinterp_args )  {

   mlog << Warning
        << "\n\n  PinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << " too may arguments for variable \"" << (v->name()) << "\"\n\n";

   return ( false );

}


int j, count;
int x, y;
double value;
bool found = false;
NcVarInfo * var = (NcVarInfo *) 0;
NcVarInfo * P   = (NcVarInfo *) 0;
const int Nx = grid.nx();
const int Ny = grid.ny();
LongArray b = a;

pressure = bad_data_double;

   //
   //  find varinfo's
   //

if ( PressureIndex >= 0 )  P = Var + PressureIndex;

found = false;

for (j=0; j<Nvars; ++j)  {

   if ( Var[j].var == v )  { found = true;  var = Var + j;  break; }

}

if ( !found )  {

   mlog << Warning
        << "\n\n  PinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << "variable " << (v->name()) << " not found!\n\n";

   return ( false );

}

   //
   //  check star positions and count
   //

count = 0;

for (j=0; j<(a.n_elements()); ++j)  {

   if ( a[j] == vx_data2d_star )  {

      ++count;

      if ( (j != var->x_slot) && (j != var->y_slot) )  {

         mlog << Warning
              << "\n\n  PinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
              << " star found in bad slot\n\n";

         return ( false );

      }

   }

}

if ( count != 2 )  {

   mlog << Warning
        << "\n\n  PinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << " bad star count ... " << count << "\n\n";

   return ( false );

}

   //
   //  check slots
   //

const int x_slot = var->x_slot;
const int y_slot = var->y_slot;
const int z_slot = var->z_slot;
// const int t_slot = var->t_slot;

if ( (x_slot < 0) || (y_slot < 0) )  {

   mlog << Warning
        << "\n\n  PinterpFile::data(NcVar *, const LongArray &, DataPlane &, double &) const -> "
        << " bad x|y slot\n\n";

   return ( false );

}

   //
   //  set up the DataPlane object
   //

plane.clear();
plane.set_size(Nx, Ny);

   //
   //  get the data
   //

for (x=0; x<Nx; ++x)  {

   b[x_slot] = x;

   for (y=0; y<Ny; ++y)  {

      b[y_slot] = y;

      value = data(v, b);

      if ( is_bad_data_pinterp( value ) ) value = bad_data_double;

      plane.set(value, x, y);

   }   //  for y

}   //  for x

   //
   //  get the pressure
   //

if ( P && z_slot > 0 )  {

   LongArray c;

   c.add(a[z_slot]);

   pressure = data(P->var, c);

}

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool PinterpFile::data(const char * var_name, const LongArray & a, DataPlane & plane,
                       double & pressure, NcVarInfo *&info) const

{

int j, time_index;
bool found = false;

for (j=0; j<Nvars; ++j)  {

   if ( Var[j].name == var_name )  {
      found = true;
      info = &Var[j];
      break;
   }

}

if ( !found )  return ( false );

found = data(Var[j].var, a, plane, pressure);

   //
   //  store the times
   //

time_index = a[Var[j].t_slot];

plane.set_init  ( InitTime );
plane.set_valid ( valid_time(time_index) );
plane.set_lead  ( lead_time(time_index) );

   //
   //  since Pinterp files only contain WRF-ARW output, it is always a
   //  a runtime accumulation
   //

if ( is_accumulation(var_name) )  {
  
   plane.set_accum ( lead_time(time_index) );

} else  {

   plane.set_accum ( 0 );

}

   //
   //  done
   //

return ( found );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


unixtime parse_init_time(const char * s)

{

int j;
unixtime t;
int month, day, year, hour, minute, second;


j = sscanf(s, "%4d-%2d-%2d_%2d:%2d:%2d", 
               &year, &month, &day, &hour, &minute, &second);

if ( j != 6 )  {

   mlog << Error << "\n\n  parse_init_time(const char *) -> bad time string ... \"" << s << "\"\n\n";

   exit ( 1 );

}

   //
   //  done
   //

t = mdyhms_to_unix(month, day, year, hour, minute, second);

return ( t );

}


////////////////////////////////////////////////////////////////////////


bool is_bad_data_pinterp(double v)

{

if ( v < pinterp_missing )  return ( false );
else                        return ( true  );

}


////////////////////////////////////////////////////////////////////////


bool is_accumulation(const char * s)

{

int j;
bool found = false;

for ( j=0; j<n_accum_var_names; ++j )  {

   if ( strcmp(s, accum_var_names[j]) == 0 )  { found = true;  break; }

}

return ( found );

}


////////////////////////////////////////////////////////////////////////
