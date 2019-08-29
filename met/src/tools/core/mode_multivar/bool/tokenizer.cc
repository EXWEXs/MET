

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>

#include "empty_string.h"

#include "tokenizer.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Tokenizer
   //


////////////////////////////////////////////////////////////////////////


Tokenizer::Tokenizer()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Tokenizer::~Tokenizer()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void Tokenizer::init_from_scratch()

{

source = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Tokenizer::clear()

{

if ( source )  { delete [] source;  source = 0; }

pos = -1;

return;

}


////////////////////////////////////////////////////////////////////////


void Tokenizer::set(const char * input)

{

if ( empty(input) )  {

   cout << "\n\n  Tokenizer::set() -> empty input string!\n\n";

   exit ( 1 );

}

const int N = strlen(input);

char * c = new char [N + 1];

memcpy(c, input, N);

c[N] = (char) 0;

source = c;

pos = 0;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int Tokenizer::get_number()

{

int value = 0;
char c;

// ++pos;

while ( (c = source[pos]) != 0 )  {

   if ( isdigit(c) )  { value = 10*value + (c - '0');  ++pos; }
   else               break;

}

return ( value );

}


////////////////////////////////////////////////////////////////////////


Token Tokenizer::next_token()

{

int k, old_pos;
Token tok;
char c;

   //
   //  skip whitespace
   //

while ( 1 )  {

   c = source[pos];

   if ( c == 0 )  {

      tok.set_eof();

      return ( tok );

   }

   if ( !isspace(c) )  break;

   ++pos;

}   //  while


old_pos = pos;

c = source[pos++];

// if ( verbose )  cout << "tokenizer: c = " << c << "\n";

switch ( c )  {

   case mark_char:
      tok.set_mark(old_pos);
      break;


   case unmark_char:
      tok.set_unmark(old_pos);
      break;


   case union_char:
      tok.set_union(old_pos);
      break;


   case intersection_char:
      tok.set_intersection(old_pos);
      break;


   case negation_char:
      tok.set_negation(old_pos);
      break;


   case local_var_char:
      k = get_number();
      tok.set_local_var(k, old_pos);
      break;



   default:
      cerr << "\n\n  Tokenizer::next_token() -> unrecognized character: " << c << "\n\n";
      exit ( 1 );
      break;

}   //  switch



// if ( verbose )  { cout << "tokenizer final: ";  tok.dump(cout); }

return ( tok );

}


////////////////////////////////////////////////////////////////////////




