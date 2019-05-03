

////////////////////////////////////////////////////////////////////////


#ifndef  __AFWA_CLOUD_PCT_FILE_H__
#define  __AFWA_CLOUD_PCT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "vx_grid.h"

#include "afwa_file.h"


////////////////////////////////////////////////////////////////////////


class AfwaCloudPctFile : public AfwaDataFile {

   private:

      void init_from_scratch();

      void assign(const AfwaCloudPctFile &);

      unsigned char * Buf;

   public:

      AfwaCloudPctFile();
      virtual ~AfwaCloudPctFile();
      AfwaCloudPctFile(const AfwaCloudPctFile &);
      AfwaCloudPctFile & operator=(const AfwaCloudPctFile &);

      void clear();

      int cloud_pct(int x, int y) const;

      virtual int operator()(int x, int y) const;

      virtual bool read(const char * filename);

};


////////////////////////////////////////////////////////////////////////


inline int AfwaCloudPctFile::operator()(int x, int y) const { return ( cloud_pct(x, y) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __AFWA_CLOUD_PCT_FILE_H__  */


////////////////////////////////////////////////////////////////////////


