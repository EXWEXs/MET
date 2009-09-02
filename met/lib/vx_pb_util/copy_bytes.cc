

//////////////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_pb_util/copy_bytes.h"


//////////////////////////////////////////////////////////////////////////////////


static const int buf_size = 4096;

static unsigned char buf[buf_size];


//////////////////////////////////////////////////////////////////////////////////


void copy_bytes(int in, int out, int N)

{

int bytes, bytes_left;
int n_read, n_written;


bytes_left = N;

while ( bytes_left > 0 )  {

   bytes = bytes_left;

   if ( bytes > buf_size )  bytes = buf_size;

   if ( (n_read = read(in, buf, bytes)) != bytes )  {

      cerr << "\n\nERROR: copy_bytes() -> "
           << "read error, n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   if ( (n_written = write(out, buf, bytes)) != bytes )  {

      cerr << "\n\nERROR: copy_bytes() -> "
           << "write error, n_written = " << n_written << "\n\n";

      exit ( 1 );

   }

   bytes_left -= bytes;

}   //  while


   //
   //  done
   //

return;

}


//////////////////////////////////////////////////////////////////////////////////
