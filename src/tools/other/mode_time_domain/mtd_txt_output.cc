

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "ascii_table.h"

#include "mtd_txt_output.h"
#include "3d_att.h"
#include "3d_txt_header.h"
#include "3d_single_columns.h"


////////////////////////////////////////////////////////////////////////


void do_3d_single_txt_output(const SingleAtt3DArray & fcst_att,
                             const SingleAtt3DArray &  obs_att,
                             const MtdConfigInfo & config,
                             const char * output_filename)

{

int j, r, c;
ofstream out;
AsciiTable table;
const int Nobj = fcst_att.n() + obs_att.n();


out.open(output_filename);

if ( ! out )  {

   cerr << "\n\n  do_3d_single_txt_output() -> unable to open output filename \""
        << output_filename << "\'\n\n";

   exit ( 1 );

}

table.set_size(1 + Nobj, n_header_3d_cols + n_3d_single_cols);

table.set_ics(2);

   //
   //  column headings
   //

r = 0;

c = 0;

for (j=0; j<n_header_3d_cols; ++j)  {

   table.set_entry(r, c++, header_3d_cols[j]);

}

for (j=0; j<n_3d_single_cols; ++j)  {

   table.set_entry(r, c++, att_3d_single_cols[j]);

}

   //
   //  leading columns
   //

for (j=0; j<Nobj; ++j)  {

   r = j + 1;

   config.write_header_cols(table, r);

}

   //
   //  attributes
   //

r = 1;

for (j=0; j<(fcst_att.n()); ++j)  {

   fcst_att[j].write_txt(table, r++);

}

for (j=0; j<(obs_att.n()); ++j)  {

   obs_att[j].write_txt(table, r++);

}


   //
   //  done
   //

out << table;

out.close();

return;

}


////////////////////////////////////////////////////////////////////////


