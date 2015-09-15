

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_FILE_NETCDF_DEFINES_H__
#define  __MTD_FILE_NETCDF_DEFINES_H__


////////////////////////////////////////////////////////////////////////


static const char         nx_dim_name [] = "Nx";
static const char         ny_dim_name [] = "Ny";
static const char         nt_dim_name [] = "Nt";
static const char       nobj_dim_name [] = "Nobjects";

static const char     data_field_name [] = "MtdData";
static const char        volumes_name [] = "Volumes";
static const char      intensity_name [] = "MaxConvIntensity";

static const char            lat_name [] = "lat";
static const char            lon_name [] = "lon";

static const char start_time_att_name [] = "StartTime";
static const char    delta_t_att_name [] = "DeltaT";
static const char     radius_att_name [] = "Radius";
static const char  threshold_att_name [] = "Threshold";
static const char  min_value_att_name [] = "MinDataValue";
static const char  max_value_att_name [] = "MaxDataValue";
static const char   is_split_att_name [] = "is_split";


////////////////////////////////////////////////////////////////////////


static const char   filetype_att_name    [] = "filetype";

static const char   filetype_raw_name    [] = "raw";
static const char   filetype_conv_name   [] = "conv";
static const char   filetype_mask_name   [] = "mask";
static const char   filetype_object_name [] = "object";


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_FILE_NETCDF_DEFINES_H__  */


////////////////////////////////////////////////////////////////////////


