

%{

////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "scanner_stuff.h"
#include "dictionary.h"
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(const char *);

extern "C" int        configwrap();


extern char *         configtext;

extern FILE *         configin;


   //
   //  definitions that have external linkage
   //


int               LineNumber            = 1;

int               Column                = 1;

const char *      bison_input_filename  = (const char *) 0;

DictionaryStack * dict_stack            = (DictionaryStack *) 0;

bool              is_lhs                = true;


////////////////////////////////////////////////////////////////////////


   //
   //  static definitions
   //

static bool is_array = false;

static Dictionary DArray;


////////////////////////////////////////////////////////////////////////


   //
   //  static declarations
   //

static Number do_op(char op, const Number & a, const Number & b);

static Number do_integer_op(char op, const Number & a, const Number & b);

static Number do_negate(const Number &);


static void do_assign_boolean (const char * name, bool);
static void do_assign_exp     (const char * name, const Number &);
static void do_assign_string  (const char * name, const char * text);

static void do_assign_id      (const char * LHS, const char * RHS);

static void do_assign_dict    (const char * name);

static void do_dict();

static void do_string(const char *);

static void do_array(const char * LHS);

static void do_thresh(const ThreshType, const Number &);


////////////////////////////////////////////////////////////////////////


%}


%union {

   char text[max_id_length];

   Number nval;

   bool bval;

   ThreshType cval;

}


%token IDENTIFIER QUOTED_STRING INTEGER FLOAT BOOLEAN
%token COMPARISON


%type <text> IDENTIFIER QUOTED_STRING assign_prefix array_prefix

%type <nval> INTEGER FLOAT number expression

%type <bval> BOOLEAN

%type <cval> COMPARISON


%left '+' '-'
%left '*' '/'
%left '^'
%nonassoc UNARY_MINUS


%%


assignment_list : assignment                    { is_lhs = true; }
                | assignment_list assignment    { is_lhs = true; }
                ;


assignment : assign_prefix BOOLEAN       ';'            { do_assign_boolean ($1, $2); }
           | assign_prefix QUOTED_STRING ';'            { do_assign_string  ($1, $2); }
           | assign_prefix expression    ';'            { do_assign_exp     ($1, $2); }
           | assign_prefix IDENTIFIER    ';'            { do_assign_id      ($1, $2); }
           | assign_prefix dictionary                   { do_assign_dict($1); }
           | assign_prefix piecewise_linear  ';'        { }

           | array_prefix string_list     ']' ';'       { do_array($1); }
           | array_prefix threshold_list  ']' ';'       { do_array($1); }
           | array_prefix dictionary_list ']' ';'       { do_array($1); }

           ;


assign_prefix : IDENTIFIER '=' { is_lhs = false;  strcpy($$, $1); }
              ;


array_prefix : assign_prefix '['     { is_lhs = false;  is_array = true;  DArray.clear(); strcpy($$, $1); }
             ;


dictionary : '{' assignment_list '}'  { do_dict(); }
           ;


dictionary_list : dictionary
                | dictionary_list ',' dictionary
                ;


string_list : QUOTED_STRING                  { do_string($1); }
            | string_list ',' QUOTED_STRING  { do_string($3); }
            ;


threshold_list : threshold
               | threshold_list ',' threshold
               ;


threshold : COMPARISON number { do_thresh($1, $2); }
          ;


number : INTEGER { }
       | FLOAT   { }
       ;


expression : number                                     { $$ = $1; }
           | expression '+' expression                  { $$ = do_op('+', $1, $3); }
           | expression '-' expression                  { $$ = do_op('-', $1, $3); }
           | expression '*' expression                  { $$ = do_op('*', $1, $3); }
           | expression '/' expression                  { $$ = do_op('/', $1, $3); }
           | expression '^' expression                  { $$ = do_op('^', $1, $3); }
           | '-' expression  %prec UNARY_MINUS          { $$ = do_negate($2); }
           | '(' expression ')'                         { $$ = $2; }
           ;


piecewise_linear : '[' point_list ']'   { }
                 ;


point_list : point              { }
           | point_list point   { }
           ;


point : '(' number ',' number ')'   { }   //  { add_point(); }



%%


////////////////////////////////////////////////////////////////////////


   //
   //  standard yacc stuff
   //


////////////////////////////////////////////////////////////////////////


void yyerror(const char * s)

{

int j, j1, j2;
int line_len, text_len;
int c;
char line[512];
ifstream in;


c = (int) (Column - strlen(configtext));

cout << "\n\n"
     << "   yyerror() -> syntax error in file \"" << bison_input_filename << "\"\n\n"
     << "      line   = " << LineNumber << "\n\n"
     << "      column = " << c << "\n\n"
     << "      text   = \"" << configtext << "\"\n\n";

in.open(bison_input_filename);

for (j=1; j<LineNumber; ++j)  {   //  j starts at one here, not zero

   in.getline(line, sizeof(line));

}

in.getline(line, sizeof(line));

in.close();




cout << "\n\n"
     << line
     << "\n";

line_len = strlen(line);

text_len = strlen(configtext);

j1 = c;
j2 = c + text_len - 1;


for (j=1; j<=line_len; ++j)  {   //  j starts at one here, not zero

   if ( (j >= j1) && (j <= j2) )  cout.put('^');
   else                           cout.put('_');

}


cout << "\n\n";

cout.flush();

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int configwrap()

{

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


Number do_op(char op, const Number & a, const Number & b)

{

Number c;
double A, B, C;

   //
   //  treat ^ as a special case
   //

if ( op == '^' )  {

   A = as_double(a);   
   B = as_double(b);   

   C = pow(A, B);

   set_double(c, C);

   return ( c );

}

   //
   //  for all other operators, see if we're doing integer arithmetic
   //

if ( a.is_int && b.is_int )  {

   c = do_integer_op(op, a, b);

   return ( c );

}

   //
   //  do floating-point arithmetic
   //

A = as_double(a);   
B = as_double(b);   

switch ( op )  {

   case '+':  C = A + B;  break;
   case '-':  C = A - B;  break;
   case '*':  C = A * B;  break;
   case '/':
      if ( B == 0.0 )  {
         cerr << "\n\n  do_op() -> division by zero!\n\n";
         exit ( 1 );
      }
      C = A / B;
      break;

   default:
      cerr << "\n\n  do_op() -> bad operator ... \"" << op << "\"\n\n";
      exit ( 1 );
      break;

}

set_double(c, C);

   //
   //  done
   //

return ( c );

}


////////////////////////////////////////////////////////////////////////


Number do_integer_op(char op, const Number & a, const Number & b)

{

Number c;
int A, B, C;

A = a.i;
B = b.i;

switch ( op )  {

   case '+':  C = A + B;  break;
   case '-':  C = A - B;  break;
   case '*':  C = A * B;  break;
   case '/':
      if ( B == 0 )  {
         cerr << "\n\n  do_integer_op() -> division by zero!\n\n";
         exit ( 1 );
      }
      C = A / B;
      break;

   default:
      cerr << "\n\n  do_integer_op() -> bad operator ... \"" << op << "\"\n\n";
      exit ( 1 );
      break;

}

set_int(c, C);

   //
   //  done
   //

return ( c );

}


////////////////////////////////////////////////////////////////////////


Number do_negate(const Number & a)

{

Number b;

if ( a.is_int )  set_int    (b, -(a.i));
else             set_double (b, -(a.d));

return ( b );

}


////////////////////////////////////////////////////////////////////////


void do_assign_boolean(const char * name, bool tf)

{

DictionaryEntry entry;

entry.set_boolean(name, tf);

dict_stack->store(entry);

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_exp(const char * name, const Number & n)

{

DictionaryEntry entry;

if ( n.is_int )  entry.set_int    (name, n.i);
else             entry.set_double (name, n.d);

dict_stack->store(entry);


return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_string(const char * name, const char * text)

{

DictionaryEntry entry;

entry.set_string(name, text);

dict_stack->store(entry);

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_id(const char * LHS, const char * RHS)

{

const DictionaryEntry * e = dict_stack->lookup(RHS);

if ( !e )  {

   cerr << "\n\n  do_assign_id() -> identifier \"" << RHS << "\" not defined in this scope!\n\n";

   exit ( 1 );

}

DictionaryEntry ee = *e;

ee.set_name(LHS);

dict_stack->store(ee);

return;

}


////////////////////////////////////////////////////////////////////////


void do_assign_dict (const char * name)

{

dict_stack->pop(name);

return;

}


////////////////////////////////////////////////////////////////////////


void do_dict()

{

if ( ! is_array )  return;

DictionaryEntry e;
ConcatString name;

name << "entry_" << DArray.n_entries();

e.set_dict(name, *(dict_stack->top()));

dict_stack->erase_top();

DArray.store(e);

return;

}


////////////////////////////////////////////////////////////////////////


void do_array(const char * LHS)

{

DictionaryEntry e;

e.set_array(LHS, DArray);

dict_stack->store(e);

DArray.clear();

is_array = false;

return;

}


////////////////////////////////////////////////////////////////////////


void do_string(const char * text)

{

if ( ! is_array )  return;

DictionaryEntry e;
ConcatString name;

name << "entry_" << DArray.n_entries();

e.set_string(name, text);

DArray.store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void do_thresh(const ThreshType t, const Number & n)

{

if ( !is_array )  {

   cerr << "\n\n  do_thresh(const ThreshType, const Number &) -> thresholds should only appear in arrays!\n\n";

   exit ( 1 );

}

DictionaryEntry e;
SingleThresh T;
ConcatString name;

name << "entry_" << DArray.n_entries();

T.set(as_double(n), t);

e.set_threshold(name, T);

DArray.store(e);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




