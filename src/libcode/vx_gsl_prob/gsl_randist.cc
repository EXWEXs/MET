// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "gsl_randist.h"

////////////////////////////////////////////////////////////////////////

static int get_seed();

////////////////////////////////////////////////////////////////////////
//
// Allocate a random number generator using the name and seed specified.
// If the seed string is of zero length, use the unixtime as the seed.
// Otherwise, use the seed specified.
//
////////////////////////////////////////////////////////////////////////

void rng_set(gsl_rng *&r, const char *rng_name, const char *rng_seed) {
   const gsl_rng_type * T;
   int seed;
   char seed_str[256];

   // Set the GSL_RNG_TYPE environment variable
   setenv("GSL_RNG_TYPE", rng_name, 1);

   // Set the GSL_RNG_SEED environment variable
   if(strlen(rng_seed) > 0) setenv("GSL_RNG_SEED", rng_seed, 1);

   // If rng_seed is not set, set the seed using the current time
   else {

      // Get the seed based on the current time
      seed = get_seed();

      sprintf(seed_str, "%i", seed);
      setenv("GSL_RNG_SEED", seed_str, 1);
   }

   // Get the random number generator setup from the environment vars
   gsl_rng_env_setup();

   // Get the rng type set by the environment vars
   T = gsl_rng_default;
   r = gsl_rng_alloc(T);

   if(r == NULL) {
      cerr << "\n\nrng_set() -> "
           << "error allocating the random number generator!\n\n"
           << flush;
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Free any memory associated with this random number generator.
//
////////////////////////////////////////////////////////////////////////

void rng_free(gsl_rng *r) {

   gsl_rng_free(r);

   r = (gsl_rng *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Produce a random permutation of the values in the array base.
//
////////////////////////////////////////////////////////////////////////

void ran_shuffle(const gsl_rng *r, double *base, int n) {

   gsl_ran_shuffle(r, base, n, sizeof(double));

   return;
}

////////////////////////////////////////////////////////////////////////

void ran_shuffle(const gsl_rng *r, NumArray &na) {
   int n, i;
   double *arr;

   n   = na.n_elements();
   arr = new double [n];

   for(i=0; i<n; i++) arr[i] = na[i];

   ran_shuffle(r, arr, n);

   for(i=0; i<n; i++) na.set(i, arr[i]);

   if(arr) { delete arr; arr = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Produce a random subsample of length k from the source array of
// length n WITHOUT replacement.  k must be less than n.
//
////////////////////////////////////////////////////////////////////////

void ran_choose(const gsl_rng *r, double *src,  int n,
                                  double *dest, int k) {

   if(k > n) {
      cerr << "\n\nran_choose() -> "
           << "the length of the subsample (" << k
           << ") must be less than or equal to the array length ("
           << n << ")!\n\n" << flush;
      exit(1);
   }

   gsl_ran_choose(r, dest, k, src, n, sizeof(double));

   return;
}

////////////////////////////////////////////////////////////////////////

void ran_choose(const gsl_rng *r, NumArray &src_na,
                                  NumArray &dest_na, int k) {
   int n, i;
   double *src, *dest;

   n    = src_na.n_elements();
   src  = new double [n];
   dest = new double [k];

   for(i=0; i<n; i++) src[i] = src_na[i];

   ran_choose(r, src, n, dest, k);

   dest_na.clear();
   for(i=0; i<k; i++) dest_na.add(dest[i]);

   if(src)  { delete src;  src  = (double *) 0; }
   if(dest) { delete dest; dest = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Produce a random subsample of length k from the source array of
// length n WITH replacement.  k need not be less than n.
//
////////////////////////////////////////////////////////////////////////

void ran_sample(const gsl_rng *r, double *src,  int n,
                                  double *dest, int k) {

   gsl_ran_sample(r, dest, k, src, n, sizeof(double));

   return;
}

////////////////////////////////////////////////////////////////////////

void ran_sample(const gsl_rng *r, NumArray &src_na,
                                  NumArray &dest_na, int k) {
   int n, i;
   double *src, *dest;

   n    = src_na.n_elements();
   src  = new double [n];
   dest = new double [k];

   for(i=0; i<n; i++) src[i] = src_na[i];

   ran_sample(r, src, n, dest, k);

   dest_na.clear();
   for(i=0; i<k; i++) dest_na.add(dest[i]);

   if(src)  { delete src;  src  = (double *) 0; }
   if(dest) { delete dest; dest = (double *) 0; }

   return;
}

////////////////////////////////////////////////////////////////////////

int get_seed() {
   int curr_time;
   unsigned char *u, t;

   // Get the current time
   curr_time = time(NULL);

   // Swap the first and fourth bytes and the second and third bytes
   u    = (unsigned char *) &(curr_time);
   t    = u[0];
   u[0] = u[3];
   u[3] = t;
   t    = u[1];
   u[1] = u[2];
   u[2] = t;

   return(curr_time);
}

////////////////////////////////////////////////////////////////////////
