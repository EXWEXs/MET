// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __VX_CONVERSIONS_H__
#define  __VX_CONVERSIONS_H__

////////////////////////////////////////////////////////////////////////

#include "vx_wrfdata/vx_wrfdata.h"

////////////////////////////////////////////////////////////////////////

extern void   rescale_probability(WrfData &);
extern double rescale_lon(double);
extern double rescale_deg(double, double, double);
extern double angle_between(double, double);
extern double angle_difference(double, double, double, double);

extern double convert_gpm_to_msl(double, double);
extern double convert_p_t_z_to_prmsl(double, double, double, double);
extern double convert_q_to_w(double);
extern double convert_p_w_to_vp(double, double);
extern double convert_vp_to_dpt(double);
extern double convert_t_to_svp(double);
extern double convert_vp_svp_to_rh(double, double);
extern double convert_p_q_t_to_rh(double, double, double);
extern double convert_u_v_to_wdir(double, double);
extern double convert_u_v_to_wind(double, double);
extern void   convert_u_v_to_unit(double, double, double &, double &);
extern void   convert_u_v_to_unit(double &, double &);
extern double gop_by_lat(double);

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_CONVERSIONS_H__

////////////////////////////////////////////////////////////////////////
