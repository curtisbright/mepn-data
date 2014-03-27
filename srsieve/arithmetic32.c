/* arithmetic32.c -- (C) Geoffrey Reynolds, April 2006.

   32 bit arithmetic functions: lmod, mod, addmod, mulmod, invmod, powmod
   (see arithmetic.h for inline functions).

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 */

#include <assert.h>
#include "arithmetic.h"

#ifndef HAVE_powmod32
#define HAVE_powmod32
/*
  b^n (mod p)
*/
uint32_t powmod32(uint32_t b, uint32_t n, uint32_t p)
{
  uint32_t a;

  for (a = (n % 2) ? b : 1, n /= 2; n > 0; n /= 2)
  {
    b = mulmod32(b,b,p);
    if (n % 2)
      a = mulmod32(a,b,p);
  }

  return a;
}
#endif

#ifndef HAVE_invmod32
#define HAVE_invmod32
/*
  1/a (mod p) if a > 0, 0 otherwise. Assumes a < p and gcd(a,p)=1.
*/
uint32_t invmod32(uint32_t a, uint32_t p)
{
  /* Thanks to the folks at mersenneforum.org.
     See http://www.mersenneforum.org/showthread.php?p=58252. */

  uint32_t ps1, ps2, parity, dividend, divisor, q, r, t;

  assert(a < p);

  if (a < 3)
    return (a < 2) ? a : (p+1)/2;

  q = p / a;
  r = p % a;

  dividend = a;
  divisor = r;
  ps2 = 1;
  ps1 = q;
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

#ifndef HAVE_legendre32
#define HAVE_legendre32
/*
  Return the value of the Legendre symbol (a/p), or 0 if (a,p) != 1.
*/
int32_t legendre32(uint32_t a, uint32_t p)
{
  uint32_t x, y, t;
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
#endif


/* Misc. arithmetic functions, these don't need to be fast.
 */

uint32_t gcd32(uint32_t a, uint32_t b)
{
  uint32_t c;

  while (b > 0)
  {
    c = a % b;
    a = b;
    b = c;
  }

  return a;
}

uint32_t pow32(uint32_t b, uint32_t n)
{
  uint32_t a = 1;

  while (n > 0)
  {
    if (n % 2)
      a *= b;
    b *= b;
    n /= 2;
  }

  return a;
}
