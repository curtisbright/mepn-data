/* srtest.c -- (C) Geoffrey Reynolds, November 2006.

   $ ./srtest [num_tests [max_errors [seed]]]

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include "srsieve.h"
#include "arithmetic.h"

/* $ ./srtest [num_tests [max_errors [seed]]]

   Calculate a^n (mod p) for num_tests random values a,n < p < sieve limit,
   checking the results against GMP's mpz_powm(). Bail out after max_errors.

   num_tests defaults to 1 million.
   max_errors defaults to 0.
   seed defaults to the value returned by time().
*/

static mpz_t tmp64;

static uint64_t mpz_get_uint64(const mpz_t OP)
{
  if (sizeof(unsigned long) == sizeof(uint64_t))
    return mpz_get_ui(OP);
  else
  {
    unsigned long l, h;

    l = mpz_get_ui(OP);
    mpz_fdiv_q_2exp(tmp64,OP,32);
    h = mpz_get_ui(tmp64);

    return ((uint64_t)h << 32) + l;
  }
}

static void mpz_set_uint64(mpz_t ROP, uint64_t a)
{
  if (sizeof(unsigned long) == sizeof(uint64_t))
    mpz_set_ui(ROP,a);
  else
  {
    mpz_set_ui(ROP,a>>32);
    mpz_mul_2exp(ROP,ROP,32);
    mpz_add_ui(ROP,ROP,a);
  }
}

int main(int argc, char **argv)
{
  uint64_t a, n, p, r;
  mpz_t A, N, P, R, M;
  gmp_randstate_t state;
  unsigned long i, j, errors;
  unsigned long num_tests, max_errors, seed;

  mpz_init(A);
  mpz_init(N);
  mpz_init(P);
  mpz_init(R);
  mpz_init(M);
  mpz_init2(tmp64,64);

  num_tests  = (argc > 1) ? strtoul(argv[1],NULL,0) : 1000000;
  max_errors = (argc > 2) ? strtoul(argv[2],NULL,0) : 0;
  seed       = (argc > 3) ? strtoul(argv[3],NULL,0) : time(NULL);

  gmp_randinit_default(state);
  gmp_randseed_ui(state,seed);

  printf("Testing calculation of a^n (mod p) for random a,n < p < %s"
         ", seed=%lu.\n", MOD64_SIEVE_LIMIT, seed);
  mpz_set_uint64(M,MOD64_GREATEST_PRIME);

  errors = 0;

#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
  getMagic(13);
#endif
  mod64_init(13);
  r = powmod64(5,0,13);
  mod64_fini();
  if (r != 1)
  {
    printf("*** 5^0 (mod 13): expected 1, got %"PRIu64".\n",r);
    ++errors;
  }

  for (i = 0; i < num_tests; )
  {
    for (j = MIN(i+10000,num_tests); i < j; i++)
    {
      mpz_urandomm(P,state,M);
      p = mpz_get_uint64(P);
      mpz_urandomm(A,state,P);
      a = mpz_get_uint64(A);
      mpz_urandomm(N,state,P);
      n = mpz_get_uint64(N);
      mpz_powm(R,A,N,P);
#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
      getMagic(p);
#endif
      mod64_init(p);
      if (n >= 6)
      {
        /* Compute a^n as a^(n-6)*a*a*a*a*a^2 to exercise mulmod/sqrmod. */
        r = powmod64(a,n-6,p);
        r = mulmod64(r,a,p);
        r = mulmod64(r,a,p);
        PRE2_MULMOD64_INIT(a);
        r = PRE2_MULMOD64(r,a,p);
        r = PRE2_MULMOD64(r,a,p);
        PRE2_MULMOD64_FINI();
        r = mulmod64(r,sqrmod64(a,p),p);
      }
      else
      {
        r = powmod64(a,n,p);
      }
      mod64_fini();

      if (r != mpz_get_uint64(R))
      {
        gmp_printf("*** %Zd^%Zd (mod %Zd):\n"
                   "    expected %Zd, got %"PRIu64".\n",
                   A,N,P,R,r);
        if (++errors > max_errors)
        {
          printf("Stopping after %lu errors.\n", errors);
          exit(1);
        }
      }
    }

    printf("Test %lu of %lu (%lu errors)\r", i, num_tests, errors);
    fflush(stdout);
  }

  printf("\n");

  return (errors > 0);
}
