// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "temp_file.h"

////////////////////////////////////////////////////////////////////////

static bool temp_file_exists(const char *);

////////////////////////////////////////////////////////////////////////

ConcatString make_temp_file_name(const char *prefix, const char *suffix) {
   int i, pid;
   ConcatString s;
   const int max_tries = 1000;

   //
   // Retrieve the current process id
   //
   pid = (int) getpid();

   i = -1;

   do {
      i++;

      if(i > max_tries) {
         cerr << "\n\nERROR: make_temp_file_name() -> "
              << "failed to make temporary file name:\n"
              << s << "\n\n" << flush;
         exit(1);
      }

      //
      // Initialize
      //
      s.erase();

      //
      // Build the file name
      //
      if(prefix) s << prefix << '_';
      s << pid << '_' << i;
      if(suffix) s << '_' << suffix;

   } while(temp_file_exists(s));

   return(s);
}

////////////////////////////////////////////////////////////////////////

void remove_temp_file(const char *file_name) {
   int errno;

   //
   // Attempt to remove the file and print out any error message
   //
   if((errno = remove(file_name)) != 0) {
      cerr << "\n\nERROR: remove_temp_file() -> "
           << "can't delete temporary file:\n"
           << file_name << "\n"
           << strerror(errno) << "\n" << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

bool temp_file_exists(const char *path) {
   int status;

   status = access(path, F_OK);

   if(status == 0) return(true);

   return(false);
}

////////////////////////////////////////////////////////////////////////
