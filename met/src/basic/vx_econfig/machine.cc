

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
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

#include "celltype_to_string.h"
#include "stetype_to_string.h"

#include "builtin.h"
#include "is_bad_data.h"
#include "vx_log.h"
#include "pwl.h"
#include "machine.h"


////////////////////////////////////////////////////////////////////////


extern Machine * bison_machine;

extern int econfigparse();

extern FILE *econfigin;

extern int econfigdebug;

extern const char * econfig_bison_input_filename;

extern int econfig_LineNumber;

extern int econfig_ColumnNumber;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Machine
   //


////////////////////////////////////////////////////////////////////////


Machine::Machine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Machine::~Machine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Machine::Machine(const Machine & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


Machine & Machine::operator=(const Machine & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Machine::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::clear()

{

sts.clear();

cstack.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::assign(const Machine & m)

{

clear();

sts = m.sts;

cstack = m.cstack;


return;

}


////////////////////////////////////////////////////////////////////////


IcodeCell Machine::pop()

{

if ( depth() == 0 )  {

   mlog << Error << "\nMachine::pop() -> stack empty!\n\n";

   exit ( 1 );

}

IcodeCell cell = cstack.pop();

return ( cell );

}


////////////////////////////////////////////////////////////////////////


void Machine::push(const IcodeCell & cell)

{

cstack.push(cell);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::run(const IcodeVector & v)

{

int pos;
IcodeCell cell;
SymbolTableEntry * entry = (SymbolTableEntry *) 0;


pos = 0;



while ( pos < v.length() )  {

   cell = v[pos];

   switch ( cell.type )  {


      case integer:
         cstack.push(cell);
         break;

      case boolean:
         cstack.push(cell);
         break;

      case floating_point:
         cstack.push(cell);
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

      case identifier:
         entry = sts.find(cell.name);
         if ( entry )   {
            run ( *entry );
         } else {
            mlog << Error << "\nMachine::run(const IcodeVector &) -> undefined identifier ... \""
                 << (cell.name) << "\"\n\n";
            exit ( 1 );
         }
         break;

       case builtin_func:
         do_builtin(cell.val);
         break;

      case character_string:
         cstack.push(cell);
         break;


      case op_store:
      case op_recall:
      default:
         mlog << Error << "\nMachine::run(const IcodeVector &) -> "
              << "bad icode cell type ... \"" << celltype_to_string(cell.type) << "\"\n\n";
         exit ( 1 );
         break;

   }   //   switch

   ++pos;

}   //  while









   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::run(const SymbolTableEntry & entry)

{

IcodeCell cell;



switch ( entry.type )  {

   case ste_integer:
      cell.set_integer(entry.i);
      cstack.push(cell);
      break;

   case ste_double:
      cell.set_double(entry.d);
      cstack.push(cell);
      break;

   case ste_variable:
      run( *(entry.v) );
      break;

   case ste_pwl:
      do_pwl( *(entry.pl) );
      break;

   case ste_function:
      do_function_call(entry);
      break;

   case ste_array:
      do_array(entry);
      break;

   default:
      mlog << Error << "\nvoid Machine::run(const SymbolTableEntry &) -> "
           << "bad type ... \"" << stetype_to_string(entry.type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch



return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_add()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   mlog << Error << "\nvoid Machine::do_add() -> can't add non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   result.set_integer( i + j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;


result.set_double(x + y);

cstack.push(result);



   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_subtract()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   mlog << Error << "\nvoid Machine::do_add() -> can't subtract non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   result.set_integer( i - j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;


result.set_double(x - y);

cstack.push(result);



   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_multiply()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   mlog << Error << "\nvoid Machine::do_add() -> can't multiply non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   result.set_integer( i*j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;


result.set_double(x*y);

cstack.push(result);






   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_divide()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   mlog << Error << "\nvoid Machine::do_add() -> can't divide non-numeric types!\n\n";

   exit ( 1 );

}

   //
   //  are both integers?
   //

if ( (operand1.type == integer) && (operand2.type == integer) )  {

   int i, j;

   i = operand1.val;
   j = operand2.val;

   if ( j == 0 )  {

      mlog << Error << "\nvoid Machine::do_divide() -> integer division by zero!\n\n";

      exit ( 1 );

   }

   result.set_integer( i/j );

   cstack.push(result);

   return;

}

   //
   //  at least one is a double, so make them both doubles
   //

double x, y;

if ( operand1.type == integer )   x = (double) (operand1.val);
else                              x = operand1.d;

if ( operand2.type == integer )   y = (double) (operand2.val);
else                              y = operand2.d;

if ( is_eq(y, 0.0) )  {

   mlog << Error << "\nvoid Machine::do_divide() -> floating-point division by zero!\n\n";

   exit ( 1 );

}

result.set_double(x/y);

cstack.push(result);






   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_power()

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();

if ( !operand1.is_numeric() || !operand2.is_numeric() )  {

   mlog << Error << "\nvoid Machine::do_power() -> can't exponentiate non-numeric types!\n\n";

   exit ( 1 );

}

double x, y, z;

x = operand1.as_double();
y = operand2.as_double();

z = pow(x, y);

result.set_double(z);

cstack.push(result);

   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_square()

{

IcodeCell operand;
IcodeCell result;


operand = cstack.pop();

if ( !operand.is_numeric() )  {

   mlog << Error << "\nvoid Machine::do_power() -> can't square non-numeric types!\n\n";

   exit ( 1 );

}

if ( operand.type == integer )  {

   int i = operand.val;

   result.set_integer( i*i );

   cstack.push(result);

   return;

}




double x;


x = operand.d;


result.set_double( x*x );

cstack.push(result);

   //
   //  return
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_negate()

{

IcodeCell operand, result;


operand = cstack.pop();

if ( !operand.is_numeric() )  {

   mlog << Error << "\nvoid Machine::do_add() -> can't negate non-numeric type!\n\n";

   exit ( 1 );

}

   //
   //  is it an integer?
   //

if ( operand.type == integer )  {

   int j;

   j = operand.val;

   result.set_integer( -j );

   cstack.push(result);

}

   //
   //  nope, is's floating-point
   //

double x;

x = operand.d;

result.set_double( -x );

cstack.push(result);






   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::st_dump(ostream & out, int indent_depth) const

{


sts.dump(out, indent_depth);


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::store(const SymbolTableEntry & entry)

{


sts.store(entry);


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::dump_cell(ostream & out, int n, int indent) const

{

cstack.dump_cell(out, n, indent);


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_pwl(const PiecewiseLinear & p)

{

IcodeCell operand, result;


operand = cstack.pop();

if ( !operand.is_numeric() )  {

   mlog << Error << "\nMachine::do_pwl(const PL *) -> can't apply pwl to non-numeric argument!\n\n";

   exit ( 1 );

}


double x, y;


x = operand.as_double();


y = p(x);


result.set_double(y);

cstack.push(result);



   //
   //  done
   //


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_function_call(const SymbolTableEntry & entry)

{

int j, n;
IcodeCell cell;
SymbolTableEntry ee;
SymbolTableEntry * u = (SymbolTableEntry *) 0;
IdentifierArray & a = *(entry.local_vars);



n = a.n_elements();

   //
   //  push the temporary scope onto the symbol table stack
   //

sts.push(entry.st);

   //
   //  store the values, starting from the bottom of the stack, 
   //     into the function's local variables
   //

for (j=(n - 1); j>=0; --j)  {   //  reverse order here

   start:

   cell = cstack.pop();

   switch ( cell.type )  {

      case integer:
         sts.store_as_int(a[j].name, cell.val);
         break;

      case floating_point:
         sts.store_as_double(a[j].name, cell.d);
         break;

      case identifier:
         u = sts.find(cell.name);
         if ( !u )  {
            mlog << Error << "\nMachine::do_function_call(const SymbolTableEntry &) -> "
                 << "undefined name on stack ... \"" << (cell.name) << "\"\n\n";
            exit ( 1 );
         }
         eval(cell.name);
         goto start;

      default:
         mlog << Error << "\nMachine::do_function_call(const SymbolTableEntry &) -> "
              << "bad cell type on stack ... \"" << celltype_to_string(cell.type) << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  for j

   //
   //  run the function program
   //

run( *(entry.v) );

   //
   //  remove the temporary scope
   //

sts.pop();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_array(const SymbolTableEntry & entry)

{

int j, dim;
int indices[max_array_dim];
const IcodeVector * v = (const IcodeVector *) 0;
IcodeCell cell;
SymbolTableEntry * u = (SymbolTableEntry *) 0;


memset(indices, 0, sizeof(indices));

   //
   //  get the values, starting from the bottom of the stack,
   //     of the indices
   //

dim = entry.ai->dim();

for (j=(dim - 1); j>=0; --j)  {   //  reverse order here

   start:

   cell = cstack.pop();

   switch ( cell.type )  {

      case integer:
         indices[j] = cell.val;
         break;

      case floating_point:
         indices[j] = my_nint(cell.d);
         break;

      case identifier:
         u = sts.find(cell.name);
         if ( !u )  {
            mlog << Error << "\nMachine::do_array(const SymbolTableEntry &) -> "
                 << "undefined name on stack ... \"" << (cell.name) << "\"\n\n";
            exit ( 1 );
         }
         eval(cell.name);
         goto start;

      default:
         mlog << Error << "\nMachine::do_array(const SymbolTableEntry &) -> "
              << "bad cell type on stack ... \"" << celltype_to_string(cell.type) << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  for j


   //
   //  get the formula for the array entry
   //

v = entry.ai->get(indices);

   //
   //  run the formula
   //

run(*v);

   //
   //
   //

return;

}


////////////////////////////////////////////////////////////////////////


double Machine::func(const char * Name, double x)

{

int index;
double y;
IcodeCell operand, result;
SymbolTableEntry * entry = (SymbolTableEntry *) 0;


operand.set_double(x);

cstack.push(operand);

   //
   //  is it a builtin function?
   //

if ( is_builtin(Name, index) )  {

   if ( binfo[index].n_vars != 1 )  {

      mlog << Error << "\nMachine::func(const char * Name, double) -> bad signature\n\n";

      exit ( 1 );

   }

   do_builtin(index);

   result = cstack.pop();

} else {

   entry = sts.find(Name);

   if ( !entry )  {

      mlog << Error << "\nMachine::func(const char * Name, double) -> function \"" << Name << "\" not defined!\n\n";

      exit ( 1 );

   }

   switch ( entry->type )  {

      case ste_function:
         do_function_call(*entry);
         result = cstack.pop();
         break;

      case ste_pwl:
         do_pwl( *(entry->pl) );
         result = cstack.pop();
         break;

      default:
         mlog << Error << "\nMachine::func(const char * Name, double) -> function \"" << Name << "\" not a function!\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  else



if ( !result.is_numeric() )  {

   mlog << Error << "\nMachine::func(const char * Name, double) -> non-numeric result!\n\n";

   exit ( 1 );

}

y = result.as_double();


   //
   //  done
   //

return ( y );

}


////////////////////////////////////////////////////////////////////////


double Machine::func(const char * Name, double x, double y)

{

int index;
double z;
IcodeCell operand, result;
SymbolTableEntry * entry = (SymbolTableEntry *) 0;


operand.set_double(x);
cstack.push(operand);

operand.set_double(y);
cstack.push(operand);

   //
   //  is it a builtin function?
   //

if ( is_builtin(Name, index) )  {

   if ( binfo[index].n_vars != 2 )  {

      mlog << Error << "\nMachine::func(const char * Name, double) -> bad signature\n\n";

      exit ( 1 );

   }

   do_builtin(index);

   result = cstack.pop();

} else {

   entry = sts.find(Name);

   if ( !entry )  {

      mlog << Error << "\nMachine::func(const char * Name, double) -> function \"" << Name << "\" not defined!\n\n";

      exit ( 1 );

   }

   switch ( entry->type )  {

      case ste_function:
         do_function_call(*entry);
         result = cstack.pop();
         break;

      case ste_pwl:
         do_pwl( *(entry->pl) );
         result = cstack.pop();
         break;

      default:
         mlog << Error << "\nMachine::func(const char * Name, double) -> function \"" << Name << "\" not a function!\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  else



if ( !result.is_numeric() )  {

   mlog << Error << "\nMachine::func(const char * Name, double) -> non-numeric result!\n\n";

   exit ( 1 );

}

z = result.as_double();


   //
   //  done
   //

return ( z );

}


////////////////////////////////////////////////////////////////////////


double Machine::func(const char * Name, double * x, int n)

{

int j;
int index;
double z;
IcodeCell operand, result;
SymbolTableEntry * entry = (SymbolTableEntry *) 0;


for (j=0; j<n; ++j)  {

   operand.set_double(x[j]);
   cstack.push(operand);

}

   //
   //  is it a builtin function?
   //

if ( is_builtin(Name, index) )  {

   if ( binfo[index].n_vars != n )  {

      mlog << Error << "\nMachine::func(const char * Name, double) -> bad signature\n\n";

      exit ( 1 );

   }

   do_builtin(index);

   result = cstack.pop();

} else {

   entry = sts.find(Name);

   if ( !entry )  {

      mlog << Error << "\nMachine::func(const char * Name, double) -> function \"" << Name << "\" not defined!\n\n";

      exit ( 1 );

   }

   switch ( entry->type )  {

      case ste_function:
         do_function_call(*entry);
         result = cstack.pop();
         break;

      case ste_pwl:
         do_pwl( *(entry->pl) );
         result = cstack.pop();
         break;

      default:
         mlog << Error << "\nMachine::func(const char * Name, double) -> function \"" << Name << "\" not a function!\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  else



if ( !result.is_numeric() )  {

   mlog << Error << "\nMachine::func(const char * Name, double) -> non-numeric result!\n\n";

   exit ( 1 );

}

z = result.as_double();


   //
   //  done
   //

return ( z );

}


////////////////////////////////////////////////////////////////////////


void Machine::eval(const char * Name)

{

IcodeCell cell;
SymbolTableEntry * u = (SymbolTableEntry *) 0;


u = sts.find(Name);

if ( !u )  {

   mlog << Error << "\nMachine::eval(const char *) -> undefined identifier ... \""
        << Name << "\"\n\n";

   exit ( 1 );

}


run( *u );

u = (SymbolTableEntry *) 0;


return;

}


////////////////////////////////////////////////////////////////////////


void Machine::algebraic_dump(ostream & out) const

{

sts.algebraic_dump(out);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::read(const char * filename)

{

bison_machine = this;

econfig_bison_input_filename = filename;

econfig_LineNumber   = 1;

econfig_ColumnNumber = 1;

// econfigdebug = 0;

if ( (econfigin = fopen(filename, "r")) == NULL )  {

   mlog << Error << "\nMachine::read() -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}


int parse_status;

parse_status = econfigparse();

if ( parse_status != 0 )  {

   mlog << Error << "\nMachine::read() -> parse status is nonzero! ... = " << parse_status << "\n\n";

   exit ( 1 );

}


   //
   //  done
   //

bison_machine = (Machine *) 0;

econfig_bison_input_filename = (const char *) 0;

fclose(econfigin);

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin(int k)

{

const BuiltinInfo & info = binfo[k];

   //
   //  nint and sign are treated differently
   //

if ( info.id == builtin_nint )  { do_nint();  return; }

if ( info.id == builtin_sign )  { do_sign();  return; }

   //
   //
   //

     if ( info.n_vars == 1 )  do_builtin_1(info);
else if ( info.n_vars == 2 )  do_builtin_2(info);
else {

   mlog << Error << "\nMachine::do_builtin(int) -> bad signature\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin_1(const BuiltinInfo & info)

{

IcodeCell operand;
IcodeCell result;


operand = cstack.pop();

if ( !(operand.is_numeric()) )  {

   mlog << Error << "\nMachine::do_builtin_1(const BuiltinInfo &) -> non-numeric operand!\n\n";

   exit ( 1 );

}

   //
   //  can we do an integer version of this function?
   //

if ( (info.i1) && (operand.type == integer) )  {

   int k;

   k = info.i1(operand.val);

   result.set_integer(k);

   cstack.push(result);

   return;

}

   //
   //  nope, do a double
   //

double x, y;


x = operand.as_double();

y = info.d1(x);


result.set_double(y);

cstack.push(result);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_builtin_2(const BuiltinInfo & info)

{

IcodeCell operand1, operand2;
IcodeCell result;


operand2 = cstack.pop();
operand1 = cstack.pop();


if ( !(operand1.is_numeric()) || !(operand2.is_numeric()) )  {

   mlog << Error << "\nMachine::do_builtin_2(const BuiltinInfo &) -> non-numeric operand!\n\n";

   exit ( 1 );

}

   //
   //  can we do an integer version of this function?
   //

if ( (info.i2) && (operand1.type == integer) && (operand2.type == integer) )  {

   int k;

   k = info.i2(operand1.val, operand2.val);   

   result.set_integer(k);

   cstack.push(result);

   return;

}

   //
   //  nope, do a double
   //

double x, y, z;

x = operand1.as_double();
y = operand2.as_double();

z = info.d2(x, y);

result.set_double(z);

cstack.push(result);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_nint()

{

IcodeCell operand;


operand = cstack.pop();

if ( !(operand.is_numeric()) )  {

   mlog << Error << "\nMachine::do_nint() -> non-numeric operand!\n\n";

   exit ( 1 );

}

   //
   //  Is it an integer?
   //  If so, just return it
   //

if ( operand.type == integer )  {

   cstack.push(operand);

   return;

}

   //
   //  it's a double
   //

int j;
double x;

x = operand.as_double();

j = my_nint(x);

operand.set_integer(j);

cstack.push(operand);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::do_sign()

{

IcodeCell operand;
IcodeCell result;


operand = cstack.pop();

if ( !(operand.is_numeric()) )  {

   mlog << Error << "\nMachine::do_sign() -> non-numeric operand!\n\n";

   exit ( 1 );

}

int k;

if ( operand.type == integer )  {

   k = my_isign(operand.val);

} else {

   k = my_dsign(operand.d);

}

result.set_integer(k);

cstack.push(result);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Machine::dump_constants(ostream & out)

{

int t, j;
int k, max_len;
int count;
IcodeCell cell;
const SymbolTableEntry * entry = (const SymbolTableEntry *) 0;




max_len = 0;

for (t=0; t<(sts.n_tables()); ++t)  {

   for (j=0; j<(sts.table(t).n_entries()); ++j)  {

      entry = sts.table(t).entry(j);

      switch ( entry->type )  {

         case ste_integer:
         case ste_double:
         case ste_variable:
            k = strlen(entry->name);
            if ( k > max_len )  max_len = k;
         break;

         default:
            break;

      }   //  switch

   }

}

// max_len += 1;


count = 0;

for (t=0; t<(sts.n_tables()); ++t)  {

   for (j=0; j<(sts.table(t).n_entries()); ++j)  {

      entry = sts.table(t).entry(j);

      switch ( entry->type )  {

         case ste_integer:
            out << (entry->name);
            for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
            out << " = "
                << (entry->i)
                << "\n";
            ++count;
            if ( (count%4) == 0 )  out << "\n";
            break;

         case ste_double:
            out << (entry->name);
            for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
            out << " = "
                << (entry->d)
                << "\n";
            ++count;
            if ( (count%4) == 0 )  out << "\n";
            break;

         case ste_variable:
            eval(entry->name);
            cell = pop();
            if ( !(cell.is_numeric()) )  {
               mlog << Error << "\nvoid Machine::dump_numerical_constants() const -> non-numeric result!\n\n";
               exit ( 1 );
            }
            if ( cell.type == integer )  {
               out << " (i)  " << (entry->name);
               for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
               out << " = " << (cell.val);
            }
            if ( cell.type == floating_point )  {
               out << " (d)  " << (entry->name);
               for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');
               out << " = " << (cell.d);
            }
            out << "\n";
            ++count;
            if ( (count%4) == 0 )  out << "\n";
            break;




         default:
            break;

      }   //  switch


   }

}


return;

}


////////////////////////////////////////////////////////////////////////


SymbolTableEntry * Machine::find(const char * name) const

{

return ( sts.find(name) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////





