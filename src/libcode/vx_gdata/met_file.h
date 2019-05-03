

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2010
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_FILE_H__
#define  __MET_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include <netcdf.hh>

#include "vx_wrfdata.h"
#include "long_array.h"
#include "grid.h"
#include "vx_cal.h"

#include "var_info.h"

////////////////////////////////////////////////////////////////////////


class MetNcFile {

   private:

      void init_from_scratch();

      MetNcFile(const MetNcFile &);
      MetNcFile & operator=(const MetNcFile &);

      void get_times(const NcVar *);

      void get_level(VarInfo &);
      void get_units(VarInfo &);
      void get_accum(VarInfo &);


   public:

      MetNcFile();
     ~MetNcFile();

      bool open(const char * filename);

      void close();

      void dump(ostream &, int = 0) const;


      NcFile * Nc;      //  allocated

         //
         //  time
         //

      unixtime ValidTime;

      unixtime InitTime;

      int      lead_time () const;   //  seconds


         //
         //  dimensions
         //

      int Ndims;

      NcDim ** Dim;   //  allocated

      StringArray DimNames;

      NcDim * Xdim;   //  not allocated
      NcDim * Ydim;   //  not allocated

         //
         //  variables
         //

      int Nvars;

      VarInfo * Var;    //  allocated

         //
         //  Grid
         //

      Grid grid;

         //
         //  data
         //

      double data(NcVar *, const LongArray &) const;

      bool data(NcVar *, const LongArray &, WrfData &) const;

      bool data(const char *, const LongArray &, WrfData &,
                ConcatString &, ConcatString &) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_FILE_H__  */


////////////////////////////////////////////////////////////////////////


