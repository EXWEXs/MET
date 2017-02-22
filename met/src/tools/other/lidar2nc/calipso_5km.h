
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CALIPSO_5KM_DATA__
#define  __CALIPSO_5KM_DATA__


////////////////////////////////////////////////////////////////////////


#include <netcdf>

#include "concat_string.h"
#include "vx_cal.h"

#include "hdf.h"
#include "mfhdf.h"

#include "hdf_utils.h"


////////////////////////////////////////////////////////////////////////


static const float FILL_VALUE = -9999.f;


////////////////////////////////////////////////////////////////////////


struct Calipso_5km_Obs  {

   int n_layers;


   float base_height;
   float base_pressure;

   float top_height;
   float top_pressure;

   int opacity;

   int cad_score;

   unsigned short fclass;

      //

   Calipso_5km_Obs();

   void clear();

   void get_layer_top_record  (int hdr_id, float *);
   void get_layer_base_record (int hdr_id, float *);
   void get_opacity_record    (int hdr_id, float *);
   void get_cad_score_record  (int hdr_id, float *);
   void get_fclass_record     (int hdr_id, float *);

};


////////////////////////////////////////////////////////////////////////


struct Calipso_5km_Vars {

   int  sd_id;   //  the "handle" for the hdf input file

      //
      //  hdf variables we're interested in
      //

   HdfVarInfo  lat;
   HdfVarInfo  lon;
   HdfVarInfo  time;

   HdfVarInfo  top_layer;
   HdfVarInfo  top_pressure;

   HdfVarInfo  base_layer;
   HdfVarInfo  base_pressure;

   HdfVarInfo  opacity_flag;

   HdfVarInfo  cad_score;

   HdfVarInfo  num_layers;

   HdfVarInfo  fclass;

      //
      //  some utility members
      //

   Calipso_5km_Vars();

   void clear();

   void get_vars(const int _hdf_sd_id);


   void get_latlon(int, float & lat, float & lon) const;

   unixtime get_time(int) const;

   void get_obs(int, Calipso_5km_Obs &) const;


};


////////////////////////////////////////////////////////////////////////


#endif   /*  __CALIPSO_5KM_DATA__  */


////////////////////////////////////////////////////////////////////////



