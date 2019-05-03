// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <regex.h>

#include "string_fxns.h"


////////////////////////////////////////////////////////////////////////


const char * get_short_name(const char * path)

{

if ( !path )  return ( (const char *) 0 );

int j;
const char * short_name = (const char *) 0;


j = strlen(path) - 1;

while ( (j >= 0) && (path[j] != '/') )  --j;

++j;

short_name = path + j;



return ( short_name );

}


////////////////////////////////////////////////////////////////////////


void append_char(char *str, const char c)

{
   char *ptr;

   //
   // If the specified characater does not already exist at the
   // end of the string, add one.
   //
   ptr = str + strlen(str) - 1;

   if(*ptr != c) {
      *(++ptr) = c;
      *(++ptr) = 0;
   }
   else {
      *(++ptr) = 0;
   }

   return;
}


////////////////////////////////////////////////////////////////////////


void strip_char(char *str, const char c)

{
   char *ptr;

   //
   // If the specified character exists at the end of the string,
   // remove it.
   //
   ptr = str + strlen(str) - 1;

   if(*ptr == c) {
      *(ptr) = 0;
   }

   return;
}


////////////////////////////////////////////////////////////////////////


bool check_reg_exp(const char *reg_exp_str, const char *test_str)

{
   bool valid = false;
   regex_t buffer;
   regex_t *preg = &buffer;

   if( regcomp(preg, reg_exp_str, REG_EXTENDED*REG_NOSUB) != 0 ) {
      cerr << "\n\n\tcheck_reg_exp(char *, char *) -> "
           << "regcomp error for \""
           << reg_exp_str << "\" and \"" << test_str << "\"\n\n"
           << flush;

      exit(1);
   }

   if( regexec(preg, test_str, 0, 0, 0) == 0 ) { valid = true; }

   //
   // Free allocated memory.
   //
   regfree( preg );

   return(valid);
}


////////////////////////////////////////////////////////////////////////


int num_tokens(const char *test_str, const char *sep_str)

{
   int n;
   char *temp_str;
   char *c = (char *) 0;

   //
   // Check for an empty string
   //
   if(strlen(test_str) <= 0) return(0);

   //
   // Initialize the temp string for use in tokenizing
   //
   temp_str = new char[strlen(test_str) + 1];

   if(!temp_str) {
      cerr << "\n\nERROR: num_tokens() -> "
           << "memory allocation error\n\n" << flush;
      exit(1);
   }
   strcpy(temp_str, test_str);

   //
   // Compute the number of tokens in the string
   //
   c = strtok(temp_str, sep_str);

   //
   // Check for an empty string
   //
   if(!c) return(0);
   else   n = 1;

   //
   // Parse remaining tokens
   //
   while((c = strtok(0, sep_str)) != NULL) n++;

   if(temp_str) { delete [] temp_str; temp_str = (char *) 0; }

   return(n);
}


////////////////////////////////////////////////////////////////////////
//
// Replace any instances of "old_str" with the contents of "new_str" in
// the "in_str" and write it to the "out_str" which must be of
// sufficient length to store the new value.  If "old_str" is set as an
// environment variable, use it's value rather than the value of
// "new_str".
//
////////////////////////////////////////////////////////////////////////

void replace_string(const char *old_str, const char *new_str,
                    const char *in_str, char *out_str) {
   char *ptr;
   const char *new_ptr;
   char tmp_str[1024];

   //
   // Initialize out_str to the contents of in_str
   //
   strcpy(out_str, in_str);

   //
   // Check to see if old_str is defined as an environment variable.
   // If so, use the value of the environment variable, if not, use the
   // value of new_str.
   //
   if((new_ptr = getenv(old_str)) == NULL) new_ptr = new_str;

   //
   // Replace any instances of old_str with the contents of new_str
   //
   while((ptr = strstr(out_str, old_str)) != NULL) {

      strcpy(tmp_str, ptr+strlen(old_str));

      //
      // Replace the old_str with the contents of new_str and
      // tack on the remaining portion of the string
      //
      sprintf(ptr, "%s%s", new_ptr, tmp_str);
   } // end while

   return;
}


////////////////////////////////////////////////////////////////////////
