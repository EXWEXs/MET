// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_NC_OUTPUT_FILE_H__
#define  __MODE_NC_OUTPUT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <netcdf.hh>

#include "data_plane.h"
#include "vx_grid.h"
#include "get_met_grid.h"


////////////////////////////////////////////////////////////////////////


enum ModeObjectField {

   mof_fcst_obj, 
   mof_fcst_comp, 

   mof_obs_obj, 
   mof_obs_comp, 

};


////////////////////////////////////////////////////////////////////////


class ModeNcOutputFile {

   private:

      void init_from_scratch();

      ConcatString Filename;

      int    get_int   (NcVar *, int x, int y) const;
      double get_float (NcVar *, int x, int y) const;

      int count_objects(NcVar *) const;

      DataPlane select_obj(ModeObjectField, int) const;

      void calc_data_range(NcVar *, double & min_value, double & max_value);

      NcFile * f;   //  allocated

      Grid * _Grid;   //  allocated

      NcVar * FcstObjId;   //  NOT allocated
      NcVar * FcstCompId;  //  NOT allocated

      NcVar * ObsObjId;    //  NOT allocated
      NcVar * ObsCompId;   //  NOT allocated

      NcVar * FcstRaw;     //  NOT allocated
      NcVar * ObsRaw;      //  NOT allocated

      ModeNcOutputFile(const ModeNcOutputFile &);
      ModeNcOutputFile & operator=(const ModeNcOutputFile &);

      int Nx;
      int Ny;

      double FcstDataMin;
      double FcstDataMax;

      double ObsDataMin;
      double ObsDataMax;

      bool fcst_data_range_calculated;
      bool  obs_data_range_calculated;

      unixtime ValidTime;
      unixtime InitTime;
      int AccumTime;   //  seconds

      // int NFcstObjs;
      // int NFcstComps;

      // int NObsObjs;
      // int NObsComps;

   public:

      ModeNcOutputFile();
     ~ModeNcOutputFile();

      bool open(const char * path);

      void close();

      void dump(ostream &) const;

      ConcatString filename() const;
      ConcatString short_filename() const;

      int nx() const;
      int ny() const;

      const Grid & grid() const;

      unixtime valid_time () const;
      unixtime init_time  () const;
      int      accum_time () const;   //  seconds

      int n_fcst_simple_objs () const;
      int n_obs_simple_objs  () const;

      int n_fcst_comp_objs () const;
      int n_obs_comp_objs  () const;

      double fcst_raw  (int x, int y) const;
      double  obs_raw  (int x, int y) const;

      int fcst_obj_id  (int x, int y) const;
      int fcst_comp_id (int x, int y) const;

      int obs_obj_id   (int x, int y) const;
      int obs_comp_id  (int x, int y) const;

      DataPlane select_fcst_obj  (int) const;   //  -1 for all
      DataPlane select_fcst_comp (int) const;   //  -1 for all

      DataPlane select_obs_obj   (int) const;   //  -1 for all
      DataPlane select_obs_comp  (int) const;   //  -1 for all

      bool x_line_valid(const int x) const;
      bool y_line_valid(const int y) const;

      void get_fcst_raw_range (double & min_value, double & max_value);
      void get_obs_raw_range  (double & min_value, double & max_value);

};


////////////////////////////////////////////////////////////////////////


inline int ModeNcOutputFile::nx() const { return ( Nx ); }
inline int ModeNcOutputFile::ny() const { return ( Ny ); }

inline const Grid & ModeNcOutputFile::grid() const { return ( *_Grid ); }

inline ConcatString ModeNcOutputFile::filename() const { return ( Filename ); }

inline unixtime ModeNcOutputFile::valid_time() const { return ( ValidTime ); }
inline unixtime ModeNcOutputFile::init_time () const { return (  InitTime ); }

inline int ModeNcOutputFile::accum_time () const { return ( AccumTime ); }

// inline int ModeNcOutputFile::n_fcst_objs () const { return ( NFcstObjs ); }
// inline int ModeNcOutputFile::n_obs_objs  () const { return ( NObsObjs ); }
// 
// inline int ModeNcOutputFile::n_fcst_comps () const { return ( NFcstComps ); }
// inline int ModeNcOutputFile::n_obs_comps  () const { return ( NObsComps ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_NC_OUTPUT_FILE_H__  */


////////////////////////////////////////////////////////////////////////


