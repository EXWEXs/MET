// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __ECONFIG_CODE_GENERATOR_H__
#define  __ECONFIG_CODE_GENERATOR_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "machine.h"

#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


class CodeGenerator {

   private:

      void init_from_scratch();

      void assign(const CodeGenerator &);

      CodeGenerator(const CodeGenerator &);
      CodeGenerator & operator=(const CodeGenerator &);

      void set_config_filename(const char *);

      void set_string_value(char * &, const char *);

      void do_header();
      void do_header_array_dec       (ostream &, const SymbolTableEntry *);
      void do_header_function_dec    (ostream &, const SymbolTableEntry *);
      void do_header_pwl_dec         (ostream &, const SymbolTableEntry *);

      void do_source();
      void do_source_read            (ostream &, const SymbolTable &);
      void do_member_for_symbol      (ostream &, const SymbolTableEntry *);
      void do_integer_member         (ostream &, const SymbolTableEntry *);
      void do_double_member          (ostream &, const SymbolTableEntry *);
      void do_variable_member        (ostream &, const SymbolTableEntry *);
      void do_pwl_member             (ostream &, const SymbolTableEntry *);
      void do_function_member        (ostream &, const SymbolTableEntry *);
      void do_array_member           (ostream &, const SymbolTableEntry *);
      void do_array_nelements_member (ostream &, const SymbolTableEntry *);

      void warning(ostream &);


      Machine machine;

      char * ClassName;

      char * FilePrefix;
      
      char * RapLib;

      char * ConfigFileName;

      const char * sep;

      unixtime GenerationTime;

      const char * zone_name;   //  not allocated

      int HH;

      int Panic;

   public:

      CodeGenerator();
     ~CodeGenerator();

      void clear();

      void set_class_name(const char *);

      void set_file_prefix(const char *);
      
      void set_raplib(const char *);

      void set_hh();

      void set_nopanic();

      void process(const char * config_filename);

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __ECONFIG_CODE_GENERATOR_H__  */


////////////////////////////////////////////////////////////////////////



