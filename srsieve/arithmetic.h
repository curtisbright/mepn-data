/* arithmetic.h -- (C) Geoffrey Reynolds, April 2006.

   arithmetic functions: lmod, mod, addmod, mulmod, invmod, powmod.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 */

#ifndef _ARITHMETIC_H
#define _ARITHMETIC_H

#include <stdint.h>
#include "config.h"

#if USE_ASM
# define NEED_lmod32
# define NEED_mod32
# define NEED_umod32
# define NEED_addmod32
# define NEED_mulmod32
# define NEED_powmod32
# define NEED_invmod32
# define NEED_legendre32
# define NEED_lmod64
# define NEED_mod64
# define NEED_umod64
# define NEED_addmod64
# define NEED_sqrmod64
# define NEED_mulmod64
# define NEED_powmod64
# define NEED_invmod64
# define NEED_legendre64
# if (__i386__ && __GNUC__)
#  include "asm-i386-gcc.h"
# elif defined(__ppc64__) || defined(__powerpc64__)
#  include "asm-ppc64.h"
# elif (__x86_64__ && __GNUC__)
#  include "asm-x86-64-gcc.h"
# endif
#endif

/*
  Use generic code for any functions not defined above in assembler.
*/

#ifndef HAVE_lmod32
#define HAVE_lmod32
/*
  a (mod p)
*/
static inline uint32_t lmod32(int32_t a, uint32_t p)
{
#if 0
  return (a >= 0) ? a % p : p - (-a % p);
#else
  if (a >= 0)
    return (a < p) ? a : a%p;
  else
    return (-a < p) ? p-(-a) : p-(-a%p);
#endif
}

/*
  a (mod p)
*/
static inline uint32_t lmod64_32(int64_t a, uint32_t p)
{
#if 0
  return (a >= 0) ? a % p : p - (-a % p);
#else
  if (a >= 0)
    return (a < p) ? a : a%p;
  else
    return (-a < p) ? p-(-(int32_t)a) : p-(-a%p);
#endif
}
#endif

#ifndef HAVE_mod32
#define HAVE_mod32
/*
  a (mod p), assuming -p < a < p
*/
static inline uint32_t mod32(int32_t a, uint32_t p)
{
  return (a >= 0) ? a : a + p;
}
#endif

#ifndef HAVE_umod32
#define HAVE_umod32
/*
  a (mod p), assuming 0 <= a
*/
static inline uint32_t umod32(uint32_t a, uint32_t p)
{
#if 0
  return a%p;
#else
  return (a < p) ? a : a%p;
#endif
}

/*
  a (mod p), assuming 0 <= a
*/
static inline uint32_t umod64_32(uint64_t a, uint32_t p)
{
#if 0
  return a%p;
#else
  return (a < p) ? a : a%p;
#endif
}
#endif

#ifndef HAVE_addmod32
#define HAVE_addmod32
/*
  a + b (mod p), assuming 0 <= a,b < p
*/
static inline uint32_t addmod32(uint32_t a, uint32_t b, uint32_t p)
{
  return (p-b <= a) ? a+b-p : a+b;
}
#endif

#ifndef HAVE_mulmod32
#define HAVE_mulmod32
/*
  a * b (mod p)
*/
static inline uint32_t mulmod32(uint32_t a, uint32_t b, uint32_t p)
{
  return (uint64_t)a * (uint64_t)b % p;
}
#endif


/* arithmetic32.c */

#ifndef HAVE_powmod32
/*
  b^n (mod p)
*/
uint32_t powmod32(uint32_t b, uint32_t n, uint32_t p) attribute ((const));
#endif

#ifndef HAVE_invmod32
/*
  1/a (mod p), assuming the inverse exists.
*/
uint32_t invmod32(uint32_t a, uint32_t p) attribute ((const));
#endif

#ifndef HAVE_legendre32
/*
  Return the value of the Legendre symbol (a/p). Assumes 0 < a < p, p prime.
*/
int32_t legendre32(uint32_t a, uint32_t p) attribute ((const));
#endif


#ifndef HAVE_lmod64
#define HAVE_lmod64
/*
  a (mod p)
*/
static inline uint64_t lmod64(int64_t a, uint64_t p)
{
#if 0
  return (a >= 0) ? a % p : p - (-a % p);
#else
  if (a >= 0)
    return (a < p) ? a : a%p;
  else
    return (-a < p) ? p-(-a) : p-(-a%p);
#endif
}
#endif

#ifndef HAVE_mod64
#define HAVE_mod64
/*
  a (mod p), assuming -p < a < p
*/
static inline uint64_t mod64(int64_t a, uint64_t p)
{
  return (a >= 0) ? a : a + p;
}
#endif

#ifndef HAVE_umod64
#define HAVE_umod64
/*
  a (mod p), assuming 0 <= a
*/
static inline uint64_t umod64(uint64_t a, uint64_t p)
{
#if 0
  return a%p;
#else
  return (a < p) ? a : a%p;
#endif
}
#endif

#ifndef HAVE_addmod64
#define HAVE_addmod64
/*
  a + b (mod p), assuming 0 <= a,b < p
*/
static inline uint64_t addmod64(uint64_t a, uint64_t b, uint64_t p)
{
  return (p-b <= a) ? a+b-p : a+b;
}
#endif


/* arithmetic64.c */

#ifndef HAVE_mulmod64
#define HAVE_mulmod64
#define GENERIC_mulmod64
extern double one_over_p;

#define MOD64_GREATEST_PRIME UINT64_C(4503599627370449)
#define MOD64_SIEVE_LIMIT "2^52"
/*
  a * b (mod p)
*/
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t p)
{
  int64_t tmp, ret;
  register double x, y;

  x = (int64_t)a;
  y = (int64_t)b;
  tmp = (int64_t)p * (int64_t)(x*y*one_over_p);
  ret = a*b - tmp;

  if (ret < 0)
    ret += p;
  else if (ret >= p)
    ret -= p;

  return ret;
}

static inline void mod64_init(uint64_t p)
{
  one_over_p = 1.0/p;
}

static inline void mod64_fini(void)
{
}

extern double b_over_p;

static inline uint64_t PRE2_MULMOD64(uint64_t a, uint64_t b, uint64_t p)
{
  int64_t tmp, ret;
  register double x;

  x = (int64_t)a;
  tmp = (int64_t)p * (int64_t)(x*b_over_p);
  ret = a*b - tmp;

  if (ret < 0)
    ret += p;
  else if (ret >= p)
    ret -= p;

  return ret;
}

static inline void PRE2_MULMOD64_INIT(uint64_t b)
{
  b_over_p = (double)((int64_t)b) * one_over_p;
}

static inline void PRE2_MULMOD64_FINI(void)
{
}
#endif

#ifndef HAVE_sqrmod64
#define HAVE_sqrmod64
/*
  b^2 (mod p)
*/
#ifdef GENERIC_mulmod64
static inline uint64_t sqrmod64(uint64_t b, uint64_t p)
{
  int64_t tmp, ret;
  register double x;

  x = (int64_t)b;
  tmp = (int64_t)p * (int64_t)(x*x*one_over_p);
  ret = b*b - tmp;

  if (ret < 0)
    ret += p;
  else if (ret >= p)
    ret -= p;

  return ret;
}
#else
/* Assume the custom mulmod is faster than a generic sqrmod. */
static inline uint64_t sqrmod64(uint64_t b, uint64_t p)
{
  return mulmod64(b,b,p);
}
#endif
#endif

#ifndef HAVE_powmod64
/*
  b^n (mod p)
*/
uint64_t powmod64(uint64_t b, uint64_t n, uint64_t p) attribute ((pure));
#endif

#ifndef HAVE_invmod64
/*
  1/a (mod p), 0 <= a < p, p prime.
*/
uint64_t invmod64(uint64_t a, uint64_t p) attribute ((const));
/*
  1/a (mod p), 0 < a < p, p prime.
*/
uint64_t invmod32_64(uint32_t a, uint64_t p) attribute ((const));
#endif

#ifndef HAVE_legendre64
/*
  Return the value of the Legendre symbol (a/p). Assumes 0 < a < p, p prime.
*/
int32_t legendre64(uint64_t a, uint64_t p) attribute ((const));
int32_t legendre32_64(uint32_t a, uint64_t p) attribute ((const));
#endif

uint32_t gcd32(uint32_t a, uint32_t b);
uint32_t pow32(uint32_t b, uint32_t n);

uint64_t gcd64(uint64_t a, uint64_t b);
uint64_t core64(uint64_t n);

#endif /* _ARITHMETIC_H */
