// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __READ_CLIMO_H__
#define  __READ_CLIMO_H__

////////////////////////////////////////////////////////////////////////

#include "vx_config.h"
#include "vx_grid.h"
#include "vx_cal.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

extern void read_climo_data_plane(
               Dictionary *, int, unixtime, const Grid &,
               DataPlane &cmn_dp, DataPlane &csd_dp);

extern void read_climo_data_plane_array(
                Dictionary *, int, unixtime, const Grid &,
                DataPlaneArray &cmn_dpa, DataPlaneArray &csd_dpa);

////////////////////////////////////////////////////////////////////////

#endif   // __READ_CLIMO_H__

////////////////////////////////////////////////////////////////////////
