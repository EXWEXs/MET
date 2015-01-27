// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


include "vx_regrid.h"


////////////////////////////////////////////////////////////////////////


DataPlane upp_regrid (const DataPlane & in, const Grid & from_grid, const Grid & to_grid, void * interp_params)

{

DataPlane out;

out.set_size(to_grid.nx(), to_grid.ny());


return ( out );

}


////////////////////////////////////////////////////////////////////////


