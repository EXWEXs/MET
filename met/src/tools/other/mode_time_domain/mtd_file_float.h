

////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_TIME_DOMAIN_FLOAT_FILE_H__
#define  __MODE_TIME_DOMAIN_FLOAT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "netcdf.hh"

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_grid.h"

#include "mtd_file_base.h"


////////////////////////////////////////////////////////////////////////


class MtdIntFile;   //  forward reference


////////////////////////////////////////////////////////////////////////


class MtdFloatFile : public MtdFileBase {

   protected:

      void float_init_from_scratch();

      void float_assign(const MtdFloatFile &);

      virtual void read  (NcFile &);
      virtual void write (NcFile &) const;


      float * Data;   //  allocated

      float DataMin;
      float DataMax;

      int Radius;   //  = -1 if not a convolved file

   public:

      MtdFloatFile();
      virtual ~MtdFloatFile();
      MtdFloatFile(const MtdFloatFile &);
      MtdFloatFile & operator=(const MtdFloatFile &);

      virtual void clear();

      virtual void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_size(int _nx, int _ny, int _nt);

      void set_data_minmax(double _data_min, double _data_max);

      void set_radius(int);

         //
         //  get stuff
         //

      float data_min() const;
      float data_max() const;

      float operator()(int _x, int _y, int _t) const;

      int radius() const;

      const float * data() const;

         //
         //  do stuff
         //

      void put   (const float, int _x, int _y, int _t);

      bool read  (const char * filename);

      void write (const char * filename) const;

      MtdIntFile threshold(double) const;
      void       threshold(double, MtdIntFile &) const;

      MtdFloatFile const_t_slice(int t) const;

      MtdFloatFile convolve(const int r) const;

};


////////////////////////////////////////////////////////////////////////


inline int MtdFloatFile::radius() const { return ( Radius ); }

inline float MtdFloatFile::data_min() const { return ( DataMin ); }
inline float MtdFloatFile::data_max() const { return ( DataMax ); }

inline const float * MtdFloatFile::data() const { return ( Data ); }

inline float MtdFloatFile::operator()(int _x, int _y, int _t) const

{

return ( Data[mtd_three_to_one(Nx, Ny, Nt, _x, _y, _t)] );

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_TIME_DOMAIN_FLOAT_FILE_H__  */


////////////////////////////////////////////////////////////////////////


