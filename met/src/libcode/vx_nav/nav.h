// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


//////////////////////////////////////////////////////////////////


#ifndef NAV_H
#define NAV_H


//////////////////////////////////////////////////////////////////

extern double            gc_dist(double lat1, double lon1, double lat2, double lon2);
extern double         gc_bearing(double lat1, double lon1, double lat2, double lon2);

extern double            rl_dist(double lat1, double lon1, double lat2, double lon2);
extern double         rl_bearing(double lat1, double lon1, double lat2, double lon2);

extern void          gc_point_v1(double lat1, double lon1, double lat2, double lon2, 
                                 double dist, double &lat, double &lon);

extern void          gc_point_v2(double lat1, double lon1, double bear, double dist, 
                                 double &lat, double &lon);

extern void          rl_point_v1(double lat1, double lon1, double lat2, double lon2, 
                                 double dist, double &lat, double &lon);

extern void          rl_point_v2(double lat1, double lon1, double bear, double dist, 
                                 double &lat, double &lon);

extern double        haversine(double a);
extern double       ahaversine(double t);
extern double        meridional_parts(double a);


//////////////////////////////////////////////////////////////////


#endif  //  NAV_H


//////////////////////////////////////////////////////////////////


