

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "calculator.h"
#include "celltype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Machine
   //


////////////////////////////////////////////////////////////////////////


Machine::Machine()

{

dict_stack = 0;

AllocInc = 100;

}


////////////////////////////////////////////////////////////////////////


Machine::~Machine()

{

dict_stack = 0;

}


////////////////////////////////////////////////////////////////////////


Machine::Machine(const Machine & c)

{

NumberStack::operator=(c);

dict_stack = c.dict_stack;

return;

}


////////////////////////////////////////////////////////////////////////


Machine & Machine::operator=(const Machine & c)

{

if ( this == &c )  return ( * this );

NumberStack::operator=(c);

dict_stack = c.dict_stack;


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Machine::do_negate()

{

Number & n = e[Nelements - 1];

if ( n.is_int )  n.i = -(n.i);
else             n.d = -(n.d);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_add()

{

Number a, b;
Number result;


pop2(a, b);

if ( a.is_int && b.is_int )  {

   set_int(result, a.i + b.i);

} else {

   set_double(result, as_double(a) + as_double(b));

}



push(result);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_subtract()

{

Number a, b;
Number result;


pop2(a, b);

if ( a.is_int && b.is_int )  {

   set_int(result, a.i - b.i);

} else {

   set_double(result, as_double(a) - as_double(b));

}



push(result);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_multiply()

{

Number a, b;
Number result;


pop2(a, b);

if ( a.is_int && b.is_int )  {

   set_int(result, (a.i) * (b.i));

} else {

   set_double(result, (as_double(a)) * (as_double(b)));

}



push(result);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_divide()

{

Number a, b;
Number result;


pop2(a, b);

if ( a.is_int && b.is_int )  {

   if ( b.i == 0 )  {

      cerr << "\n\n  Machine::do_divide() -> integer division by zero!\n\n";

      exit ( 1 );

   }

   set_int(result, (a.i) / (b.i));

} else {

   const double denom = as_double(b);

   if ( denom == 0 )  {

      cerr << "\n\n  Machine::do_divide() -> floating-point division by zero!\n\n";

      exit ( 1 );

   }

   set_double(result, (as_double(a)) / denom);

}



push(result);

return;

}



////////////////////////////////////////////////////////////////////////


void Machine::do_power()

{

Number a, b;
Number result;


pop2(a, b);

set_double(result, pow(as_double(a), as_double(b)));


push(result);

return;

}



////////////////////////////////////////////////////////////////////////


void Machine::do_square()

{

Number a = pop();
Number result;


if ( a.is_int )  {

   const int k = a.i;

   set_int(result, k*k);

} else {

   const double x = as_double(a);

   set_double(result, x*x);

}


push(result);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_nint()

{

Number & n = e[Nelements - 1];

if ( n.is_int )  return;

set_int(n, nint(n.d));


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_sign()

{

Number n = pop();
Number result;
int s;

if ( n.is_int )  {

   const int k = n.i;

   if ( k == 0 )  s =  0;
   else           s = ( (k > 0) ? 1 : -1 );

} else {

   const double x = n.d;

   if ( x == 0.0 )  s =  0;
   else             s = ( (x > 0.0) ? 1 : -1 );

}

set_int(result, s);

push(result);

return;

}



////////////////////////////////////////////////////////////////////////


void Machine::do_builtin(int which)

{

const BuiltinInfo & info = binfo[which];

   //
   //  nint and sign are treated differently
   //

if ( info.id == builtin_nint )  { do_nint();  return; }

if ( info.id == builtin_sign )  { do_sign();  return; }

   //
   //
   //

     if ( info.n_args == 1 )  do_builtin_1_arg(info);
else if ( info.n_args == 2 )  do_builtin_2_arg(info);
else {

   cerr << "\n\n  Machine::do_builtin(int) -> bad signature for builtin function \""
        << (info.name) << "\"\n\n";

   exit ( 1 );

}



return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin(int which, const Number * n)

{

int j;
const BuiltinInfo & info = binfo[which];

for (j=0; j<(info.n_args); ++j)  push(n[j]);

do_builtin(which);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin_1_arg(const BuiltinInfo & info)

{

Number a, result;

a = pop();

   //
   //  can we do an integer version of this function?
   //

if ( (info.i1) && (a.is_int) )  {

   int k;

   k = info.i1(a.i);

   set_int(result, k);

   push(result);

   return;

}

   //
   //  nope, do a double
   //

double x, y;


x = as_double(a);

y = info.d1(x);


set_double(result, y);

push(result);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin_2_arg(const BuiltinInfo & info)

{

Number a, b;
Number result;


b = pop();
a = pop();

   //
   //  can we do an integer version of this function?
   //

if ( (info.i2) && (a.is_int) && (b.is_int) )  {

   int k;

   k = info.i2(a.i, b.i);

   set_int(result, k);

   push(result);

   return;

}

   //
   //  nope, do a double
   //

double x, y, z;

x = as_double(a);
y = as_double(b);

z = info.d2(x, y);

set_double(result, z);

push(result);



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_user_func(const DictionaryEntry * e)

{


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::run(const IcodeVector & v)

{

int pos;
IcodeCell cell;

pos = 0;

while ( pos < (v.length()) )  {

   cell = v[pos];

   switch ( cell.type )  {

      case integer:
         push_int(cell.i);
         break;

      case floating_point:
         push_double(cell.d);
         break;

     case op_add:
         do_add();
         break;

      case op_subtract:
         do_subtract();
         break;

      case op_multiply:
         do_multiply();
         break;

      case op_divide:
         do_divide();
         break;

      case op_power:
         do_power();
         break;

      case op_square:
         do_square();
         break;


      case op_negate:
         do_negate();
         break;

/*
      case identifier:
         e = sts.find(cell.name);
         if ( e )   {
            run ( *e );
         } else {
            cerr << "\n\n  Machine::run(const IcodeVector &) -> undefined identifier ... \""
                 << (cell.name) << "\"\n\n";
            exit ( 1 );
         }
         break;
*/


       case builtin_func:
         do_builtin(cell.i);
         break;



       case user_func:
         do_user_func(0);
         break;




      default:
         cerr << "\n\n  Machine::run(const IcodeVector &) -> bad icode cell type ... \"" 
              << celltype_to_string(cell.type)
              << "\"\n\n";
         exit ( 1 );
         break;


   }   //  switch

   ++pos;

}   //  while


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




