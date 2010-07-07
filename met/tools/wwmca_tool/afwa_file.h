

////////////////////////////////////////////////////////////////////////


#ifndef  __AFWA_DATA_FILE_H__
#define  __AFWA_DATA_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal/vx_cal.h"
#include "vx_util/vx_util.h"
#include "vx_data_grids/grid.h"


////////////////////////////////////////////////////////////////////////


class AfwaCloudPctFile {

   private:

      void init_from_scratch();

      void assign(const AfwaCloudPctFile &);

      int two_to_one(int, int) const;


      const Grid * grid;   //  allocated

      unsigned char * Buf;

      ConcatString Filename;

      char Hemisphere;  //  'N' or 'S'

      unixtime Valid;

   public:

      AfwaCloudPctFile();
     ~AfwaCloudPctFile();
      AfwaCloudPctFile(const AfwaCloudPctFile &);
      AfwaCloudPctFile & operator=(const AfwaCloudPctFile &);

      bool read(const char * filename);

      void clear();

      int operator()(int x, int y) const;

      int get_value(int x, int y) const;

      bool xy_is_ok(int x, int y) const;

      int nx() const;
      int ny() const;

      unixtime valid() const;

      char hemisphere() const;

};


////////////////////////////////////////////////////////////////////////


inline unixtime AfwaCloudPctFile::valid      () const { return ( Valid ); }
inline char     AfwaCloudPctFile::hemisphere () const { return ( Hemisphere ); }

inline int      AfwaCloudPctFile::operator()(int x, int y) const { return ( get_value(x, y) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __AFWA_DATA_FILE_H__  */


////////////////////////////////////////////////////////////////////////


