

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "mtd_file.h"
#include "mtd_partition.h"
#include "mtd_nc_defs.h"
#include "nc_grid.h"
#include "nc_utils.h"
#include "mtdfiletype_to_string.h"

#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MtdFileBase
   //


////////////////////////////////////////////////////////////////////////


MtdFileBase::MtdFileBase()

{

base_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MtdFileBase::~MtdFileBase()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::base_init_from_scratch()

{

G = (Grid *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::clear()

{

if ( G )  { delete G;  G = (Grid *) 0; }

Nx = Ny = Nt = 0;

StartTime = (unixtime) 0;

DeltaT = 0;

Filename.clear();

FileType = no_mtd_file_type;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::base_assign(const MtdFileBase & f)

{

clear();

Nx         = f.Nx;
Ny         = f.Ny;
Nt         = f.Nt;

StartTime  = f.StartTime;

DeltaT     = f.DeltaT;

Filename   = f.Filename;

FileType   = f.FileType;

if ( f.G )  set_grid ( *(f.G) );


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::dump(ostream & out, int depth) const

{

Indent prefix(depth);
int month, day, year, hour, minute, second;
char junk[256];

out << prefix << "Filename          = ";

if ( Filename.nonempty() )  out << '\"' << Filename << "\"\n";
else                        out << "(nul)\n";

unix_to_mdyhms(StartTime, month, day, year, hour, minute, second);

sprintf(junk, "%s %d, %d   %02d:%02d:%02d", short_month_name[month], day, year, hour, minute, second);


out << prefix << "StartTime         = " << StartTime << " ... (" << junk << ")\n";
out << prefix << "DeltaT            = " << DeltaT    << '\n';
out << prefix << "FileType          = " << mtdfiletype_to_string(FileType) << '\n';

out << prefix << "(Nx, Ny, Nt)      = (" << Nx << ", " << Ny << ", " << Nt << ")\n";

if ( G )  {

   out << prefix << "Grid ...\n";

   G->dump(out, depth + 1);

   // out << prefix << (G->xml_serialize()) << '\n';

} else {

   out << prefix << "Grid              = 0\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_filetype(MtdFileType t)

{

FileType = t;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_grid(const Grid & g)

{

if ( G )  { delete G;  G = (Grid *) 0; }

G = new Grid ( g );

return;

}


////////////////////////////////////////////////////////////////////////


Grid MtdFileBase::grid() const

{

if ( !G )  {

   mlog << Error << "\n\n  MtdFileBase::grid() const -> no grid!\n\n";

   exit ( 1 );

}


return ( *G );

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::latlon_to_xy(double lat, double lon, double & x, double & y) const

{

if ( !G )  {

   mlog << Error << "\n\n  MtdFileBase::latlon_to_xy() -> no grid!\n\n";

   exit ( 1 );

}

G->latlon_to_xy(lat, lon, x, y);

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_start_time(unixtime t)

{

StartTime = t;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::set_delta_t(int seconds)

{

DeltaT = seconds;

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::xy_to_latlon(double x, double y, double & lat, double & lon) const

{

if ( !G )  {

   mlog << Error << "\n\n  MtdFileBase::xy_to_latlon() -> no grid!\n\n";

   exit ( 1 );

}

G->xy_to_latlon(x, y, lat, lon);

return;

}


////////////////////////////////////////////////////////////////////////


unixtime MtdFileBase::valid_time(int t) const

{

if ( (t < 0) || ( t >= Nt) )  {

   mlog << Error << "\n\n  MtdFileBase::valid_time(int t) -> range check error\n\n";

   exit ( 1 );

}


return ( StartTime + t*DeltaT );

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::read(NcFile & f)

{

NcDim * dim = 0;

   //  Nx, Ny, Nt

dim = f.get_dim(nx_dim_name);
Nx  = dim->size();

dim = f.get_dim(ny_dim_name);
Ny  = dim->size();

dim = f.get_dim(nt_dim_name);
Nt  = dim->size();

dim = 0;

   //  Grid

G = new Grid;

read_nc_grid(f, *G);

   //  timestamp info

StartTime = parse_start_time(string_att(f, start_time_att_name));

DeltaT    = string_att_as_int (f, delta_t_att_name);

   //  FileType

ConcatString s = string_att(f, filetype_att_name);
bool status = false;

status = string_to_mtdfiletype(s.text(), FileType);

if ( ! status )  {

   mlog << Error << "\n\n  MtdFileBase::read(NcFile &) -> unable to parse filetype string \""
        << s << "\"\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void MtdFileBase::write(NcFile & f) const

{

char junk[256];
ConcatString s;

   //  Nx, Ny, Nt

f.add_dim(nx_dim_name, Nx);
f.add_dim(ny_dim_name, Ny);
f.add_dim(nt_dim_name, Nt);

   //  Grid

write_nc_grid(f, *G);

   //  timestamp info

s = start_time_string(StartTime);

f.add_att(start_time_att_name, s.text());


sprintf(junk, "%d", DeltaT);

f.add_att(delta_t_att_name, junk);

   //  FileType

s = mtdfiletype_to_string(FileType);

f.add_att(filetype_att_name, s.text());



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////






