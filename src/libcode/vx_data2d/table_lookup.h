// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_DATA2D_TABLE_LOOKUP_H__
#define  __VX_DATA2D_TABLE_LOOKUP_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


class Grib1TableEntry {

   private:

      void init_from_scratch();

      void assign(const Grib1TableEntry &);

   public:

      Grib1TableEntry();
     ~Grib1TableEntry();
      Grib1TableEntry(const Grib1TableEntry &);
      Grib1TableEntry & operator=(const Grib1TableEntry &);

      void clear();

      void dump(ostream &, int = 0) const;

      int code;
      int table_number;

      ConcatString parm_name;

      ConcatString full_name;

      ConcatString units;   //  could be empty

         //
         //  set stuff
         //

         //
         //  get stuff
         //

         //
         //  do stuff
         //

      bool parse_line(const char *);

};


////////////////////////////////////////////////////////////////////////


class Grib2TableEntry {

   private:

      void init_from_scratch();

      void assign(const Grib2TableEntry &);

   public:

      Grib2TableEntry();
     ~Grib2TableEntry();
      Grib2TableEntry(const Grib2TableEntry &);
      Grib2TableEntry & operator=(const Grib2TableEntry &);

      void clear();

      void dump(ostream &, int = 0) const;

      int index_a;
      int index_b;
      int index_c;

      ConcatString parm_name;

      ConcatString full_name;

      ConcatString units;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

         //
         //  do stuff
         //

      bool parse_line(const char *);

};


////////////////////////////////////////////////////////////////////////


class TableFlatFile {

   private:

      void init_from_scratch();

      void assign(const TableFlatFile &); 

      bool read_grib1(istream &, const char * filename, const int n);
      bool read_grib2(istream &, const char * filename, const int n);

      void extend_grib1(int);
      void extend_grib2(int);

      Grib1TableEntry ** g1e;   //  elements ... allocated
      Grib2TableEntry ** g2e;   //  elements ... allocated

      int N_grib1_elements;
      int N_grib2_elements;

      int N_grib1_alloc;
      int N_grib2_alloc;

   public:

      TableFlatFile();
      TableFlatFile(int);   //  reads defaults
     ~TableFlatFile();
      TableFlatFile(const TableFlatFile &);
      TableFlatFile & operator=(const TableFlatFile &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      int n_grib1_elements() const;
      int n_grib2_elements() const;

         //
         //  do stuff
         //

      bool read(const char * filename);


      bool lookup_grib1(int code, int table_number, Grib1TableEntry &);

      bool lookup_grib1(int code,               Grib1TableEntry &);   //  assumes table_number is 2
      bool lookup_grib1(const char * parm_name, Grib1TableEntry &);   //  assumes table_number is 2

      bool lookup_grib1(const char * parm_name, int table_number, int code,
                        Grib1TableEntry &, int & n_matches);

      bool lookup_grib2(int a, int b, int c, Grib2TableEntry &);
      bool lookup_grib2(const char * parm_name, Grib2TableEntry &, int & n_matches);
      bool lookup_grib2(const char * parm_name, int a, int b, int c, Grib2TableEntry &, int & n_matches);

};


////////////////////////////////////////////////////////////////////////


inline int TableFlatFile::n_grib1_elements() const { return ( N_grib1_elements ); }
inline int TableFlatFile::n_grib2_elements() const { return ( N_grib2_elements ); }


////////////////////////////////////////////////////////////////////////

   //
   //  Global instance of TableFlatFile
   //

extern TableFlatFile GribTable;


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_DATA2D_TABLE_LOOKUP_H__   */


////////////////////////////////////////////////////////////////////////


