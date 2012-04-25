

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

      bool read_grib1(istream &, const char * filename);
      bool read_grib2(istream &, const char * filename);

      Grib1TableEntry * g1e;   //  elements ... allocated
      Grib2TableEntry * g2e;   //  elements ... allocated

      int N_grib1_elements;
      int N_grib2_elements;

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

      bool lookup_grib2(int a, int b, int c, Grib2TableEntry &);
      bool lookup_grib2(const char * parm_name, Grib2TableEntry &, int & n_matches);

};


////////////////////////////////////////////////////////////////////////


inline int TableFlatFile::n_grib1_elements() const { return ( N_grib1_elements ); }
inline int TableFlatFile::n_grib2_elements() const { return ( N_grib2_elements ); }


////////////////////////////////////////////////////////////////////////


extern TableFlatFile GribTable;


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_DATA2D_TABLE_LOOKUP_H__   */


////////////////////////////////////////////////////////////////////////


