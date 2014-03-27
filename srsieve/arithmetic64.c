/* arithmetic64.c -- (C) Geoffrey Reynolds, April 2006.

   64 bit arithmetic functions: lmod, mod, addmod, mulmod, invmod, powmod
   (see arithmetic.h for inline functions).

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 */

#include <assert.h>
#include "arithmetic.h"

#ifdef GENERIC_mulmod64
double one_over_p;
double b_over_p;
#elif (USE_ASM && !USE_FPU_MULMOD && __x86_64__ && __GNUC__)
double one_over_p attribute ((aligned(16)));
double b_over_p attribute ((aligned(16)));
#endif

#if (USE_ASM && (__i386__ || __x86_64__) && __GNUC__)
uint16_t mod64_rnd; /* Saved FPU/SSE rounding mode */
#endif

#ifndef HAVE_powmod64
#define HAVE_powmod64
/*
  return b^n (mod p)
 */
uint64_t powmod64(uint64_t b, uint64_t n, uint64_t p)
{
  uint64_t a;

  a = 1;
  goto tst;

 mul:
  a = mulmod64(a,b,p);

 sqr:
  b = sqrmod64(b,p);

  n >>= 1;

 tst:
  if (n & 1)
    goto mul;
  if (n > 0)
    goto sqr;

  return a;
}
#endif

#ifndef HAVE_invmod64
#define HAVE_invmod64
/*
  1/a (mod p)  if a > 0, 0 othewise. Assumes a < p and gcd(a,p)=1.
*/
uint64_t invmod64(uint64_t a, uint64_t p)
{
  /* Thanks to the folks at mersenneforum.org.
     See http://www.mersenneforum.org/showthread.php?p=58252. */

  uint64_t ps1, ps2, q, r, t, dividend, divisor;
  uint32_t parity;

  assert(a < p);

  if (a < 3)
    return (a < 2) ? a : (p+1)/2;

  q = p / a;
  r = p % a;

  assert(r > 0);

  dividend = a;
  divisor = r;
  ps1 = q;
  ps2 = 1;
  parity = 0;
	
  while (divisor > 1)
  {
    r = dividend - divisor;
    t = r - divisor;
    if (r >= divisor) {
      q += ps1; r = t; t -= divisor;
      if (r >= divisor) {
        q += ps1; r = t; t -= divisor;
        if (r >= divisor) {
          q += ps1; r = t; t -= divisor;
          if (r >= divisor) {
            q += ps1; r = t; t -= divisor;
            if (r >= divisor) {
              q += ps1; r = t; t -= divisor;
              if (r >= divisor) {
                q += ps1; r = t; t -= divisor;
		if (r >= divisor) {
                  q += ps1; r = t; t -= divisor;
                  if (r >= divisor) {
                    q += ps1; r = t;
                    if (r >= divisor) {
                      q = dividend / divisor;
                      r = dividend % divisor;
                      q *= ps1;
                    } } } } } } } } }
    q += ps2;
    parity = ~parity;
    dividend = divisor;
    divisor = r;
    ps2 = ps1;
    ps1 = q;
  }

  assert(0 < ps1);
  assert(ps1 < p);

  return (parity) ? ps1 : p - ps1;
}

/*
  1/a (mod p) if a > 0, 0 otherwise. Assumes 0 < a < p, gcd(a,p)=1.
*/
uint64_t invmod32_64(uint32_t a, uint64_t p)
{
  /* Thanks to the folks at mersenneforum.org.
     See http://www.mersenneforum.org/showthread.php?p=58252. */

  uint64_t ps1, ps2, q;
  uint32_t r, t, dividend, divisor, parity;

  assert(a < p);

  if (a < 3)
    return (a < 2) ? a : (p+1)/2;

  q = p / a;
  r = p % a;

  assert(r > 0);

  dividend = a;
  divisor = r;
  ps1 = q;
  ps2 = 1;
  parity = 0;
	
  while (divisor > 1)
  {
    r = dividend - divisor;
    t = r - divisor;
    if (r >= divisor) {
      q += ps1; r = t; t -= divisor;
      if (r >= divisor) {
        q += ps1; r = t; t -= divisor;
        if (r >= divisor) {
          q += ps1; r = t; t -= divisor;
          if (r >= divisor) {
            q += ps1; r = t; t -= divisor;
            if (r >= divisor) {
              q += ps1; r = t; t -= divisor;
              if (r >= divisor) {
                q += ps1; r = t; t -= divisor;
		if (r >= divisor) {
                  q += ps1; r = t; t -= divisor;
                  if (r >= divisor) {
                    q += ps1; r = t;
                    if (r >= divisor) {
                      q = dividend / divisor;
                      r = dividend % divisor;
                      q *= ps1;
                    } } } } } } } } }
    q += ps2;
    parity = ~parity;
    dividend = divisor;
    divisor = r;
    ps2 = ps1;
    ps1 = q;
  }

  assert(0 < ps1);
  assert(ps1 < p);

  return (parity) ? ps1 : p - ps1;
}
#endif

#ifndef HAVE_legendre64
#define HAVE_legendre64
/*
  Return the value of the Legendre symbol (a/p), or 0 if (a,p) != 1.
*/
int32_t legendre64(uint64_t a, uint64_t p)
{
  uint64_t x, y, t;
  int32_t sign;

  for (sign = 1, x = a, y = p; x > 0; x %= y)
  {
    for ( ; x % 2 == 0; x /= 2)
      if (y % 8 == 3 || y % 8 == 5)
        sign = -sign;

    t = x, x = y, y = t;

    if (x % 4 == 3 && y % 4 == 3)
      sign = -sign;
  }

  return (y == 1) ? sign : 0;
}

/*
  Return the value of the Legendre symbol (a/p). Assumes 0 < a < p, p prime.
*/
int32_t legendre32_64(uint32_t a, uint64_t p)
{
  uint32_t x, y, t;
  int32_t sign;

  assert(0 < a);
  assert(a < p);

  for (sign = 1, y = a; y % 2 == 0; y /= 2)
    if ((uint_fast32_t)p % 8 == 3 || (uint_fast32_t)p % 8 == 5)
      sign = -sign;

  if ((uint_fast32_t)p % 4 == 3 && y % 4 == 3)
    sign = -sign;

  for (x = p % y; x > 0; x %= y)
  {
    for ( ; x % 2 == 0; x /= 2)
      if (y % 8 == 3 || y % 8 == 5)
        sign = -sign;

    t = x, x = y, y = t;

    if (x % 4 == 3 && y % 4 == 3)
      sign = -sign;
  }

  assert(y == 1);

  return sign;
}
#endif


uint64_t gcd64(uint64_t a, uint64_t b)
{
  uint64_t c;

  while (b > 0)
  {
    c = a % b;
    a = b;
    b = c;
  }

  return a;
}

/* Return the least positive d such that n/d is a square.
 */
#include <math.h>
uint64_t core64(uint64_t n)
{
  /* TODO: Improve the trial division algorithm (use sieve). */

  uint64_t c = 1;
  uint_fast32_t d, q, r;

  assert(n > 0);

  while (n % 2 == 0)
  {
    n /= 2;
    if (n % 2 != 0)
      c *= 2;
    else
      n /= 2;
  }
  while (n % 3 == 0)
  {
    n /= 3;
    if (n % 3 != 0)
      c *= 3;
    else
      n /= 3;
  }
  r = sqrtl(n);
  if ((uint64_t)r*r == n)
    return c;
  for (q = 5, d = 4; q <= r; d = 6-d, q += d)
    if (n % q == 0)
    {
      do
      {
        n /= q;
        if (n % q != 0)
          c *= q;
        else
          n /= q;
      }
      while (n % q == 0);
      r = sqrtl(n);
      if ((uint64_t)r*r == n)
        return c;
    }

  return c * n;
}

/* Setup for Mark Rodenkirch's PPC mulmod/expmod functions. See
   mulmod-ppc64.s and expmod-ppc64.s
*/
#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
uint64_t  pMagic;
uint64_t  pShift;

void getMagic(uint64_t d)
{
   uint64_t two63 = 0x8000000000000000;

   uint64_t t = two63;
   uint64_t anc = t - 1 - t%d;    /* Absolute value of nc. */
   uint64_t p = 63;            /* Init p. */
   uint64_t q1 = two63/anc;       /* Init q1 = 2**p/|nc|. */
   uint64_t r1 = two63 - q1*anc;  /* Init r1 = rem(2**p, |nc|). */
   uint64_t q2 = two63/d;        /* Init q2 = 2**p/|d|. */
   uint64_t r2 = two63- q2*d;      /* Init r2 = rem(2**p, |d|). */
   uint64_t delta, mag;

   do {
      p = p + 1;
      q1 = 2*q1;                  /* Update q1 = 2**p/|nc|. */
      r1 = 2*r1;                  /* Update r1 = rem(2**p, |nc|. */
      if (r1 >= anc) {      /* (Must be an unsigned */
         q1 = q1 + 1;        /* comparison here). */
         r1 = r1 - anc;
      }
      q2 = 2*q2;                  /* Update q2 = 2**p/|d|. */
      r2 = 2*r2;                  /* Update r2 = rem(2**p, |d|. */
      if (r2 >= d) {          /* (Must be an unsigned */
         q2 = q2 + 1;        /* comparison here). */
         r2 = r2 - d;
      }
      delta = d - r2;
   } while (q1 < delta || (q1 == delta && r1 == 0));

   mag = q2 + 1;

   pShift = p - 64;             /* shift amount to return. */

   pMagic = mag;
}
#endif
