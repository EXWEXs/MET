

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
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
#include <string>

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "nccf_file.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


static const char x_dim_name          [] = "lon";
static const char y_dim_name          [] = "lat";

static const char valid_time_att_name [] = "valid_time_ut";
static const char  init_time_att_name [] = "init_time_ut";
static const char accum_time_att_name [] = "accum_time_sec";

static const char name_att_name       [] = "name";
static const char long_name_att_name  [] = "long_name";
static const char level_att_name      [] = "level";
static const char units_att_name      [] = "units";

static const int  max_met_args           = 30;


const double NcCfFile::DELTA_TOLERANCE = 15.0;

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class NcCfFile
   //


////////////////////////////////////////////////////////////////////////


NcCfFile::NcCfFile()
{
  init_from_scratch();
}


////////////////////////////////////////////////////////////////////////


NcCfFile::~NcCfFile()
{
  close();
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::init_from_scratch()

{
  // Initialize the pointers

  _ncFile = (NcFile *) 0;
  _dims = (NcDim **) 0;
  Var = (NcVarInfo *) 0;

  // Close any existing file

  close();

  return;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::close()
{

  // Reclaim the file pointer

  if (_ncFile)
  {
    delete _ncFile;
    _ncFile = (NcFile *)0;
  }

  // Reclaim the dimension pointers

  if (_dims)
  {
    delete [] _dims;
    _dims = (NcDim **)0;
  }

  _numDims = 0;

  _dimNames.clear();

  _xDim = _yDim = (NcDim *) 0;

  // Reclaim the variable pointers

  if (Var)
  {
    delete [] Var;
    Var = (NcVarInfo *)0;
  }

  Nvars = 0;

  // Reset the time values

  ValidTime = (unixtime)0;
  InitTime  = (unixtime)0;

  //  done

  return;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::open(const char * filename)
{
  // Close any open files and clear out the associated members

  close();

  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program. In the case of this example, we just exit with
  // an NC_ERR error code.
  
  NcError err(NcError::silent_nonfatal);

  // Open the file

  _ncFile = new NcFile(filename);

  if (!(_ncFile->is_valid()))
  {
    close();
    return false;
  }

  // Pull out the dimensions

  _numDims = _ncFile->num_dims();

  _dims = new NcDim*[_numDims];

  _dimNames.extend(_numDims);

  for (int j = 0; j < _numDims; ++j)
  {
    _dims[j] = _ncFile->get_dim(j);

    _dimNames.add(_dims[j]->name());
  }

  // Pull out the variables

  Nvars = _ncFile->num_vars();

  Var = new NcVarInfo [Nvars];

  for (int j = 0; j < Nvars; ++j)
  {
    NcVar *v = _ncFile->get_var(j);

    Var[j].var = v;

    Var[j].name = v->name();

    Var[j].Ndims = v->num_dims();

    Var[j].Dims = new NcDim * [Var[j].Ndims];

    //  parse the variable attributes
   
    int init_int;
    int valid_int;
    
    get_att_str( Var[j], name_att_name,       Var[j].name_att      );
    get_att_str( Var[j], long_name_att_name,  Var[j].long_name_att );
    get_att_str( Var[j], level_att_name,      Var[j].level_att     );
    get_att_str( Var[j], units_att_name,      Var[j].units_att     );
    get_att_int( Var[j], accum_time_att_name, Var[j].AccumTime     );
    get_att_int( Var[j], init_time_att_name,  init_int             );
    get_att_int( Var[j], valid_time_att_name, valid_int            );

    InitTime  = (unixtime)init_int;
    ValidTime = (unixtime)valid_int;

  }   //  for j

  // Pull out the grid.  This must be done after pulling out the dimension
  // and variable information since this information is used to pull out the
  // grid.  This call sets the _xDim and _yDim pointers.

  read_netcdf_grid();

  // Now go back through the variables and use _xDim and _yDim to set the
  // slots.

  for (int j = 0; j < Nvars; ++j)
  {
    NcVar *v = _ncFile->get_var(j);

    for (int k = 0; k < Var[j].Ndims; ++k)
    {
      Var[j].Dims[k] = v->get_dim(k);

      if (Var[j].Dims[k] == _xDim)
	Var[j].x_slot = k;
      if (Var[j].Dims[k] == _yDim)
	Var[j].y_slot = k;
    }   //  for k

  }   //  for j

  //  done
  
  return true;
}


////////////////////////////////////////////////////////////////////////


void NcCfFile::dump(ostream & out, int depth) const
{
  char junk[256];

  Indent prefix(depth);
  Indent p2(depth + 1);
  Indent p3(depth + 2);
  
  out << prefix << "Grid ...\n";

  grid.dump(out, depth + 1);

  out << prefix << "\n";

  out << prefix << "Nc = " << (_ncFile ? "ok" : "(nul)") << "\n";

  out << prefix << "\n";

  out << prefix << "Ndims = " << _numDims << "\n";

  for (int j = 0; j < _numDims; ++j)
  {
    out << p2 << "Dim # " << j << " = " << _dimNames[j] << "   ("
	<< (_dims[j]->size()) << ")\n";
  }   //  for j

  out << prefix << "\n";

  out << prefix << "Xdim = " << (_xDim ? _xDim->name() : "(nul)") << "\n";
  out << prefix << "Ydim = " << (_yDim ? _yDim->name() : "(nul)") << "\n";

  out << prefix << "\n";

  out << prefix << "Init Time = ";

  int month, day, year, hour, minute, second;
  
  unix_to_mdyhms(InitTime, month, day, year, hour, minute, second);

  sprintf(junk, "%s %d, %d   %2d:%02d:%02d",
	  short_month_name[month], day, year, hour, minute, second);

  out << junk << "\n";

  out << prefix << "\n";

  out << prefix << "Nvars = " << Nvars << "\n";

  for (int j = 0; j < Nvars; ++j)
  {
    out << p2 << "Var # " << j << " = " << (Var[j].name) << "  (";

    for (int k = 0; k < Var[j].Ndims; ++k)
    {
      if (Var[j].Dims[k] == _xDim)
	out << 'X';
      else if (Var[j].Dims[k] == _yDim)
	out << 'Y';
      else
	out << Var[j].Dims[k]->name();

      if (k < Var[j].Ndims - 1)
	out << ", ";
    }   //  for k

    out << ")\n";

    out << p3 << "Slots (X, Y, Z, T) = (";

    if (Var[j].x_slot >= 0)
      out << Var[j].x_slot;
    else
      out << '_';  out << ", ";

    if (Var[j].y_slot >= 0)
      out << Var[j].y_slot;
    else
      out << '_';

    out << ")\n";
   
    out << p2 << "\n";

  }   //  for j

  //  done

  out.flush();

  return;
}


////////////////////////////////////////////////////////////////////////


int NcCfFile::lead_time() const
{
  unixtime dt = ValidTime - InitTime;

  return (int) dt;
}


////////////////////////////////////////////////////////////////////////


double NcCfFile::data(NcVar * var, const LongArray & a) const
{
  if (!args_ok(a))
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
	 << "bad arguments:\n";

    a.dump(cerr);

    exit(1);
  }

  if (var->num_dims() != a.n_elements())
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
	 << "needed " << (var->num_dims()) << " arguments for variable "
	 << (var->name()) << ", got " << (a.n_elements()) << "\n\n";

    exit(1);
  }

  if (var->num_dims() >= max_met_args)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
	 << " too may arguments for variable \"" << (var->name()) << "\"\n\n";

    exit(1);
  }

  long counts[max_met_args];

  for (int j = 0; j < (a.n_elements()); ++j)
    counts[j] = 1;

  if (!(var->set_cur((long *)a)))
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
	 << " can't set corner for variable \"" << (var->name()) << "\"\n\n";

    exit(1);
  }

  bool status;
  double d[2];
  
  switch (var->type())
  {
  case ncInt:
  {
    int i[2];
    
    status = var->get(i, counts);
    d[0] = (double) (i[0]);
    break;
  }
  
  case ncFloat:
  {
    float f[2];
    
    status = var->get(f, counts);
    d[0] = (double) (f[0]);
    break;
  }
  
  case ncDouble:
  {
    status = var->get(d, counts);
    break;
  }
  
  default:
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
	 << " bad type for variable \"" << (var->name()) << "\"\n\n";
    exit(1);
    break;
  }
  }   //  switch

  if (!status)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &) const -> "
	 << " bad status for var->get()\n\n";

    exit(1);
  }

  //  done

  return d[0];
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::data(NcVar * v, const LongArray & a, DataPlane & plane) const
{
  if (!args_ok(a))
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
	 << "bad arguments:\n";

    a.dump(cerr);

    exit(1);
  }

  if (v->num_dims() != a.n_elements())
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) -> "
	 << "needed " << (v->num_dims()) << " arguments for variable "
	 << (v->name()) << ", got " << (a.n_elements()) << "\n\n";

    exit(1);
  }

  if (v->num_dims() >= max_met_args)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) -> "
	 << " too may arguments for variable \"" << (v->name()) << "\"\n\n";

    exit(1);
  }

  //  find varinfo's

  bool found = false;
  NcVarInfo *var = (NcVarInfo *)0;
  
  for (int j = 0; j < Nvars; ++j)
  {
    if (Var[j].var == v)
    {
      found = true;
      var = Var + j;
      break;
    }
  }

  if (!found)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
	 << "variable " << (v->name()) << " not found!\n\n";

    exit(1);
  }

  //  check star positions and count

  int count = 0;

  for (int j = 0; j < a.n_elements(); ++j)
  {
    if (a[j] == vx_data2d_star)
    {
      ++count;

      if ((j != var->x_slot) && (j != var->y_slot))
      {
	mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
	     << " star found in bad slot\n\n";

	exit(1);
      }
    }
  }

  if (count != 2)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
	 << " bad star count ... " << count << "\n\n";

    exit(1);
  }

  //  check slots

  const int x_slot = var->x_slot;
  const int y_slot = var->y_slot;

  if (x_slot < 0 || y_slot < 0)
  {
    mlog << Error << "\nNcCfFile::data(NcVar *, const LongArray &, DataPlane &) const -> "
	 << " bad x|y|z slot\n\n";

    exit(1);
  }

  //  set up the DataPlane object

  const int nx = grid.nx();
  const int ny = grid.ny();
  
  plane.clear();
  plane.set_size(nx, ny);

  //  get the data

  LongArray b = a;
  
  for (int x = 0; x < nx; ++x)
  {
    b[x_slot] = x;

    for (int y = 0; y < ny; ++y)
    {
      b[y_slot] = y;

      double value = data(v, b);

      plane.set(value, x, y);

    }   //  for y
  }   //  for x

  //  done

  return true;
}


////////////////////////////////////////////////////////////////////////


bool NcCfFile::data(const char *var_name,
		    const LongArray &a, DataPlane &plane,
		    NcVarInfo *&info) const
{
  info = find_var_name(var_name);
  if (info == 0)
    return false;
  
  bool found = data(info->var, a, plane);

  //  store the times

  plane.set_init(ValidTime - lead_time());
  plane.set_valid(ValidTime);
  plane.set_lead(lead_time());
  plane.set_accum(info->AccumTime);

  //  done

  return found;
}

////////////////////////////////////////////////////////////////////////

NcVarInfo* NcCfFile::find_var_name(const char * var_name) const
{
  for (int i = 0; i < Nvars; i++)
    if (Var[i].name == var_name)
      return &Var[i];

  return 0;
}


////////////////////////////////////////////////////////////////////////

void NcCfFile::read_netcdf_grid()
{
  // Loop through the variables looking for the first gridded variable.  We
  // will use this variable to pull out the grid information.  The CF
  // description allows for different fields in the same file to have different
  // grids, but with how the gridded information is used in MET, I'm making the
  // assumption that all fields are on the same grid.

  NcVar *data_var = 0;
  
  for (int i = 0; i < Nvars; ++i)
  {
    // Get a pointer to the variable

    NcVar *var = Var[i].var;
    
    // A gridded data variable can have anywhere from 2 to 4 dimensions (the
    // fourth being time).  Any other variables can be ignored

    int num_dims = var->num_dims();

    if (num_dims < 2 || num_dims > 4)
      continue;
    
    // Skip the latitude and longitude variables, if they are present

    NcAtt *std_name_att = var->get_att("standard_name");

    if (std_name_att != 0)
    {
      char *std_name = std_name_att->as_string(0);
		      
      if (std_name == 0 ||
	  strcmp(std_name, "latitude") == 0 ||
	  strcmp(std_name, "longitude") == 0)
	continue;
    }
    
    // If we get here, this should be a gridded data variable

    data_var = var;
    break;
    
  } /* endfor - i */
  
  // Pull the grid projection from the variable information.  First, look for
  // a grid_mapping attribute.

  NcAtt *grid_mapping_att = data_var->get_att("grid_mapping");

  if (grid_mapping_att != 0)
  {
    get_grid_from_grid_mapping(grid_mapping_att);
    return;
  }
  
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_from_grid_mapping(const NcAtt *grid_mapping_att)
{
  static const string method_name = "NcCfFile::get_grid_from_grid_mapping()";
  
  // The grid_mapping attribute gives the name of the variable that
  // contains the grid mapping information.  Find that variable.

  char *mapping_name = grid_mapping_att->as_string(0);
  if (mapping_name == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Cannot extract grid mapping name from netCDF file.\n\n";
    exit(1);
  }
    
  NcVar *grid_mapping_var = 0;
    
  for (int i = 0; i < Nvars; ++i)
  {
    if (strcmp(Var[i].name, mapping_name) == 0)
    {
      grid_mapping_var = Var[i].var;
      break;
    }
  } /* endfor - i */

  if (grid_mapping_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Cannot extract grid mapping variable(" << mapping_name
	 << " from netCDF file.\n\n";
    exit(1);
  }
  
  // Get the name of the grid mapping

  NcAtt *grid_mapping_name_att = grid_mapping_var->get_att("grid_mapping_name");
  
  if (grid_mapping_name_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Cannot get coordinate system name from netCDF file.\n\n";
    exit(1);
  }

  string grid_mapping_name = grid_mapping_name_att->as_string(0);
  
  // Handle each mapping type defined in the standard

  if (grid_mapping_name == "albers_conical_equal_area")
  {
    get_grid_mapping_albers_conical_equal_area(grid_mapping_var);
  }
  else if (grid_mapping_name == "azimuthal_equidistant")
  {
    get_grid_mapping_azimuthal_equidistant(grid_mapping_var);
  }
  else if (grid_mapping_name == "lambert_azimuthal_equal_area")
  {
    get_grid_mapping_lambert_azimuthal_equal_area(grid_mapping_var);
  }
  else if (grid_mapping_name == "lambert_conformal_conic")
  {
    get_grid_mapping_lambert_conformal_conic(grid_mapping_var);
  }
  else if (grid_mapping_name == "lambert_cylindrical_equal_area")
  {
    get_grid_mapping_lambert_cylindrical_equal_area(grid_mapping_var);
  }
  else if (grid_mapping_name == "latitude_longitude")
  {
    get_grid_mapping_latitude_longitude(grid_mapping_var);
  }
  else if (grid_mapping_name == "mercator")
  {
    get_grid_mapping_mercator(grid_mapping_var);
  }
  else if (grid_mapping_name == "orthographic")
  {
    get_grid_mapping_orthographic(grid_mapping_var);
  }
  else if (grid_mapping_name == "polar_stereographic")
  {
    get_grid_mapping_polar_stereographic(grid_mapping_var);
  }
  else if (grid_mapping_name == "rotated_latitude_longitude")
  {
    get_grid_mapping_rotated_latitude_longitude(grid_mapping_var);
  }
  else if (grid_mapping_name == "stereographic")
  {
    get_grid_mapping_stereographic(grid_mapping_var);
  }
  else if (grid_mapping_name == "transverse_mercator")
  {
    get_grid_mapping_transverse_mercator(grid_mapping_var);
  }
  else if (grid_mapping_name == "vertical_perspective")
  {
    get_grid_mapping_vertical_perspective(grid_mapping_var);
  }
  else
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Unknown grid mapping name (" << grid_mapping_name
	 << " found in netCDF file.\n\n";
    exit(1);
  }
  
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_albers_conical_equal_area(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_albers_conical_equal_area()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Albers conical equal area grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_azimuthal_equidistant(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_azimuthal_equidistant()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Azimuthal equidistant grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_lambert_azimuthal_equal_area(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_lambert_azimuthal_equal_area()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Lambert azimuthal equal area grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_lambert_conformal_conic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_lambert_conformal_conic()";

  // standard_parallel -- there can be 1 or 2 of these

  NcAtt *std_parallel_att = grid_mapping_var->get_att("standard_parallel");
  if (std_parallel_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Cannot get standard_parallel attribute from "
	 << grid_mapping_var->name() << " variable.\n\n";
    exit(1);
  }
  
  // longitude_of_central_meridian

  NcAtt *central_lon_att =
    grid_mapping_var->get_att("longitude_of_central_meridian");
  if (central_lon_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Cannot get longitude_of_central_meridian attribute from "
	 << grid_mapping_var->name() << " variable.\n\n";
    exit(1);
  }
  
  // latitude_of_projection_origin

  NcAtt *proj_origin_lat_att =
    grid_mapping_var->get_att("latitude_of_projection_origin");
  if (proj_origin_lat_att == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Cannot get latitude_of_projection_origin attribute from "
	 << grid_mapping_var->name() << " variable.\n\n";
    exit(1);
  }
  
  // Look for the x/y dimensions

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // Get the standard name for the coordinate variable

    const NcVar *coord_var = _ncFile->get_var(_dims[dim_num]->name());
    if (coord_var == 0)
      continue;
    
    const NcAtt *std_name_att = coord_var->get_att("standard_name");
    if (std_name_att == 0)
      continue;
    
    const char *dim_std_name = std_name_att->as_string(0);
    if (dim_std_name == 0)
      continue;
    // See if this is an X or Y dimension

    if (strcmp(dim_std_name, "projection_x_coordinate") == 0)
    {
      _xDim = _dims[dim_num];

      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
	if (strcmp(Var[var_num].name, _xDim->name()) == 0)
	{
	  _xCoordVar = Var[var_num].var;
	  break;
	}
      }
    }
    
    if (strcmp(dim_std_name, "projection_y_coordinate") == 0)
    {
      _yDim = _dims[dim_num];

      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
	if (strcmp(Var[var_num].name, _yDim->name()) == 0)
	{
	  _yCoordVar = Var[var_num].var;
	  break;
	}
      }
    }
    
  }

  if (_xDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find X dimension (projection_x_coordinate) in netCDF file.\n\n";
    exit(1);
  }
  
  if (_yDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find Y dimension (projection_y_coordinate) in netCDF file.\n\n";
    exit(1);
  }
  
  if (_xCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find X coord variable (" << _xDim->name()
	 << ") in netCDF file.\n\n";
    exit(1);
  }
  
  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find Y coord variable (" << _yDim->name()
	 << ") in netCDF file.\n\n";
    exit(1);
  }
  
  if (_xCoordVar->num_vals() != _xDim->size() ||
      _yCoordVar->num_vals() != _yDim->size())
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }
  
  // Figure out the dx/dy  and x/y pin values from the dimension variables

  double x_values[_xDim->size()];
  long x_counts = _xDim->size();
  
  _xCoordVar->get(x_values, &x_counts);
  
  double y_values[_yDim->size()];
  long y_counts = _yDim->size();
  
  _yCoordVar->get(y_values, &y_counts);
  
  // Calculate dx and dy assuming they are constant.  MET requires that dx be
  // equal to dy

  double dx_m = (x_values[_xDim->size()-1] - x_values[0]) / (_xDim->size() - 1);
  double dy_m = (y_values[_yDim->size()-1] - y_values[0]) / (_yDim->size() - 1);
  
  if (fabs(dx_m - dy_m) > DELTA_TOLERANCE)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "MET can only process Lambert Conformal files where the x-axis and y-axis deltas are the same\n\n";
    exit(1);
  }
  
  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < _xDim->size(); ++i)
  {
    double curr_delta = x_values[i] - x_values[i-1];
    if (fabs(curr_delta - dx_m) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
	   << "MET can only process Lambert Conformal files where the delta along the x-axis is constant\n\n";
      exit(1);
    }
  }
  
  for (int i = 1; i < _yDim->size(); ++i)
  {
    double curr_delta = y_values[i] - y_values[i-1];
    if (fabs(curr_delta - dy_m) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
	   << "MET can only process Lambert Conformal files where the delta along the y-axis is constant\n\n";
      exit(1);
    }
  }
  
  // Calculate the pin indices.  The pin will be located at the grid's reference
  // location since that's the only lat/lon location we know about.

  double x_pin = -(x_values[0] / dx_m);
  double y_pin = -(y_values[0] / dy_m);
  
  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  LambertData data;
  data.name = lambert_proj_type;
  data.scale_lat_1 = std_parallel_att->as_double(0);
  if (std_parallel_att->num_vals() == 1)
    data.scale_lat_2 = data.scale_lat_1;
  else
    data.scale_lat_2 = std_parallel_att->as_double(1);
  data.lat_pin = proj_origin_lat_att->as_double(0);
  data.lon_pin = -central_lon_att->as_double(0);
  data.x_pin = x_pin;
  data.y_pin = y_pin;
  data.lon_orient = -central_lon_att->as_double(0);
  data.d_km = dx_m / 1000.0;
  data.r_km = 6371.20;
  data.nx = _xDim->size();
  data.ny = _yDim->size();
  
  grid.set(data);
  
//  grid.dump(cerr);
  
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_lambert_cylindrical_equal_area(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_lambert_cynlindrical_equal_area()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Lambert cylindrical equal area grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_latitude_longitude(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_latitude_longitude()";
  
  // Look for the lat/lon dimensions

  for (int dim_num = 0; dim_num < _numDims; ++dim_num)
  {
    // The lat/lon dimensions are identified by their units

    const NcVar *coord_var = _ncFile->get_var(_dims[dim_num]->name());
    if (coord_var == 0)
      continue;
    
    const NcAtt *units_att = coord_var->get_att("units");
    if (units_att == 0)
      continue;
    
    const char *dim_units = units_att->as_string(0);
    if (dim_units == 0)
      continue;

    // See if this is a lat or lon dimension

    if (strcmp(dim_units, "degrees_north") == 0 ||
	strcmp(dim_units, "degree_north") == 0 ||
	strcmp(dim_units, "degree_N") == 0 ||
	strcmp(dim_units, "degrees_N") == 0 ||
	strcmp(dim_units, "degreeN") == 0 ||
	strcmp(dim_units, "degreesN") == 0)
    {
      _yDim = _dims[dim_num];

      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
	if (strcmp(Var[var_num].name, _yDim->name()) == 0)
	{
	  _yCoordVar = Var[var_num].var;
	  break;
	}
      }
    }
    
    if (strcmp(dim_units, "degrees_east") == 0 ||
	strcmp(dim_units, "degree_east") == 0 ||
	strcmp(dim_units, "degree_E") == 0 ||
	strcmp(dim_units, "degrees_E") == 0 ||
	strcmp(dim_units, "degreeE") == 0 ||
	strcmp(dim_units, "degreesE") == 0)
    {
      _xDim = _dims[dim_num];

      for (int var_num = 0; var_num < Nvars; ++var_num)
      {
	if (strcmp(Var[var_num].name, _xDim->name()) == 0)
	{
	  _xCoordVar = Var[var_num].var;
	  break;
	}
      }
    }
    
  }

  if (_xDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find X dimension (degrees_east) in netCDF file.\n\n";
    exit(1);
  }
  
  if (_yDim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find Y dimension (degrees_north) in netCDF file.\n\n";
    exit(1);
  }
  
  if (_xCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find X coord variable (" << _xDim->name()
	 << ") in netCDF file.\n\n";
    exit(1);
  }
  
  if (_yCoordVar == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Didn't find Y coord variable (" << _yDim->name()
	 << ") in netCDF file.\n\n";
    exit(1);
  }
  
  if (_xCoordVar->num_vals() != _xDim->size() ||
      _yCoordVar->num_vals() != _yDim->size())
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "Coordinate variables don't match dimension sizes in netCDF file.\n\n";
    exit(1);
  }
  
  // Figure out the dlat/dlon values from the dimension variables

  double lat_values[_yDim->size()];
  long lat_counts = _yDim->size();
  
  _yCoordVar->get(lat_values, &lat_counts);
  
  double lon_values[_xDim->size()];
  long lon_counts = _xDim->size();
  
  _xCoordVar->get(lon_values, &lon_counts);
  
  // Calculate dlat and dlon assuming they are constant.  MET requires that
  // dlat be equal to dlon

  double dlat = (lat_values[_yDim->size()-1] - lat_values[0]) /
    (_yDim->size() - 1);
  double dlon = (lon_values[_xDim->size()-1] - lon_values[0]) /
    (_xDim->size() - 1);
  
  if (fabs(dlat - dlon) > DELTA_TOLERANCE)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "MET can only process Latitude/Longitude files where the delta lat and delta lon are the same\n\n";
    exit(1);
  }
  
  // As a sanity check, make sure that the deltas are constant through the
  // entire grid.  CF compliancy doesn't require this, but MET does.

  for (int i = 1; i < _yDim->size(); ++i)
  {
    double curr_delta = lat_values[i] - lat_values[i-1];
    if (fabs(curr_delta - dlat) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
	   << "MET can only process Latitude/Longitude files where the lat delta is constant\n\n";
      exit(1);
    }
  }
  
  for (int i = 1; i < _xDim->size(); ++i)
  {
    double curr_delta = lon_values[i] - lon_values[i-1];
    if (fabs(curr_delta - dlon) > DELTA_TOLERANCE)
    {
      mlog << Error << "\n" << method_name << " -> "
	   << "MET can only process Latitude/Longitude files where the lon delta is constant\n\n";
      exit(1);
    }
  }
  
  // Fill in the data structure.  Remember to negate the longitude
  // values since MET uses the mathematical coordinate system centered on
  // the center of the earth rather than the regular map coordinate system.

  LatLonData data;
  
  data.name = latlon_proj_type;
  data.lat_ll = lat_values[0];
  data.lon_ll = -lon_values[0];
  data.delta_lat = dlat;
  data.delta_lon = dlon;
  data.Nlat = _yDim->size();
  data.Nlon = _xDim->size();

  grid.set(data);
  
//  grid.dump(cerr);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_mercator(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_mercator()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Mercator grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_orthographic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_orthographic()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Orthographic grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_polar_stereographic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_polar_stereographic()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Polar stereographic grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_rotated_latitude_longitude(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_rotated_latitude_longitude()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Rotated latitude longitude grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_stereographic(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_stereographic()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Stereographic grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_transverse_mercator(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_transverse_mercator()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Transverse mercator grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////

void NcCfFile::get_grid_mapping_vertical_perspective(const NcVar *grid_mapping_var)
{
  static const string method_name = "NcCfFile::get_grid_mapping_vertical_perspective()";
  
  mlog << Error << "\n" << method_name << " -> "
       << "Vertical perspective grid not handled in MET.\n\n";
  exit(1);
}

////////////////////////////////////////////////////////////////////////
