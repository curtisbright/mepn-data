/* asm-i386-gcc.h -- (C) Geoffrey Reynolds, November 2006.

   Inline i386 assember routines for GCC.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _ASM_I386_GCC_H
#define _ASM_I386_GCC_H

#include "config.h"

/* Transparent unions are a GCC extension. When a function is declared to
   take transparant union arguments the argument is actually passed as the
   type of its first element. TU64_t types declared below are used to pass
   64 bit integers to low level functions that operate on their parts.
*/
typedef union
{
  uint64_t u;
  struct { uint32_t l, h; };
} __attribute__ ((transparent_union)) TU64_t;

#endif /* _ASM_I386_GCC_H */


/* This file may be included from a number of different headers. It should
   only define FEATURE if NEED_FEATURE is defined but HAVE_FEATURE is not.
*/


#if defined(NEED_mulmod32) && !defined(HAVE_mulmod32)
#define HAVE_mulmod32

static inline uint32_t mulmod32(uint32_t a, uint32_t b, uint32_t p)
{
  /*
    Anand Nair helped adapt this from the MSC code to GCC.
  */

  register uint32_t tmp, ret;

  /* Using seperate asms avoids the need for early clobber constraints on
     intermediate registers otherwise required for (useless) cases like
     mulmod32(a,p,p), while still allowing the optimiser full range in
     useful cases like mulmod(a,a,p).
  */

  asm ("mull %3"
       : "=d" (ret), "=a" (tmp)
       : "1" (a), "rm" (b) );

  asm ("divl %4"
       : "=d" (ret), "=a" (tmp)
       : "0" (ret), "1" (tmp), "rm" (p) );

  return ret;
}
#endif


#if defined(NEED_mulmod64) && !defined(HAVE_mulmod64)
#define HAVE_mulmod64

#define MOD64_GREATEST_PRIME UINT64_C(4611686018427387847)
#define MOD64_SIEVE_LIMIT "2^62"
extern uint16_t mod64_rnd;

/* a*b (mod p), where a,b < p < 2^62.
   Assumes %st(0) contains 1.0/p and FPU is in round-to-zero mode.
*/
static inline uint64_t mulmod64(uint64_t a, uint64_t b, TU64_t p)
{
  uint32_t t1;
  uint32_t t2;
  uint64_t tmp;
  register uint64_t ret;

  asm("fildll  %5"                  "\n\t"
      "fildll  %4"                  "\n\t"
      "mov     %5, %%ecx"           "\n\t"
      "mov     %5, %%eax"           "\n\t"
      "mov     4+%5, %%ebx"         "\n\t"
      "mull    %4"                  "\n\t"
      "fmulp   %%st(0), %%st(1)"    "\n\t"
      "imul    %4, %%ebx"           "\n\t"
      "fmul    %%st(1), %%st(0)"    "\n\t"
      "imul    4+%4, %%ecx"         "\n\t"
      "mov     %%eax, %2"           "\n\t"
      "add     %%ebx, %%ecx"        "\n\t"
      "fistpll %1"                  "\n\t"
      "add     %%ecx, %%edx"        "\n\t"
      "mov     %%edx, %3"           "\n\t"
      "mov     %1, %%eax"           "\n\t"
      "mov     4+%1, %%edx"         "\n\t"
      "mov     %7, %%ebx"           "\n\t"
      "mov     %6, %%ecx"           "\n\t"
      "imul    %%eax, %%ebx"        "\n\t"
      "imul    %%edx, %%ecx"        "\n\t"
      "mull    %6"                  "\n\t"
      "add     %%ebx, %%ecx"        "\n\t"
      "mov     %3, %%ebx"           "\n\t"
      "add     %%ecx,%%edx"         "\n\t"
      "mov     %2, %%ecx"           "\n\t"
      "sub     %%eax, %%ecx"        "\n\t"
      "sbb     %%edx, %%ebx"        "\n\t"
      "mov     %%ecx, %%eax"        "\n\t"
      "mov     %%ebx, %%edx"        "\n\t"
      "sub     %6, %%ecx"           "\n\t"
      "sbb     %7, %%ebx"           "\n\t"
      "jl      0f"                  "\n\t"
      "mov     %%ecx, %%eax"        "\n\t"
      "mov     %%ebx, %%edx"        "\n\t"
      "0:"
      : "=&A" (ret), "=m" (tmp), "=m" (t1), "=m" (t2)
      : "m" (a), "m" (b), "rm" (p.l), "rm" (p.h), "m" (mod64_rnd)
      : "%ebx", "%ecx", "cc" );

  return ret;
}

/* Pushes 1.0/p onto the FPU stack and changes the FPU into round-to-zero
   mode. Saves the old rounding mode in mod64_rnd.
*/
static inline void mod64_init(uint64_t p)
{
  uint16_t tmp;

  asm volatile ("fildll  %2"               "\n\t"
                "fld1"                     "\n\t"
                "fnstcw  %0"               "\n\t"
                "mov     %0, %%ax"         "\n\t"
                "and     $3072, %%ax"      "\n\t"
                "orw     $3072, %0"        "\n\t"  /* Set bits 10,11 */
                "mov     %%ax, %1"         "\n\t"  /* Save old bits */
                "fldcw   %0"               "\n\t"
                "fdivp   %%st(0),%%st(1)"

                : "=m" (tmp), "+m" (mod64_rnd)
                : "m" (p)
                : "%eax", "cc" );
}

/* Same as mod64_init(p) but does not set FPU rounding, for nested use, e.g.
   mod64_init(p1)
   mod64_init0(p2)
   mod64_init0(p3)
   ...
   mod64_fini0()
   mod64_fini0()
   mod64_fini()
*/
static inline void mod64_init0(uint64_t p)
{
  asm volatile ("fildll  %1"               "\n\t"
                "fld1"                     "\n\t"
                "fdivp   %%st(0),%%st(1)"

                : "+m" (mod64_rnd) /* Dummy dependency */
                : "m" (p)
                : "cc" );
}

/* Pops 1.0/p off the FPU stack and restores the previous FPU rounding mode
   from mod64_rnd.
*/
static inline void mod64_fini(void)
{
  uint16_t tmp;

  asm volatile ("fstp    %%st(0)"          "\n\t"
                "fnstcw  %0"               "\n\t"
                "mov     %0, %%ax"         "\n\t"
                "and     $62463, %%ax"     "\n\t"  /* Clear bits 10,11 */
                "or      %1, %%ax"         "\n\t"  /* Restore old bits */
                "mov     %%ax, %0"         "\n\t"
                "fldcw   %0"

                : "=m" (tmp), "+m" (mod64_rnd)
                : 
                : "%eax", "cc" );
}

/* Same as mod64_fini() but does not restore rounding mode.
 */
static inline void mod64_fini0(void)
{
  asm volatile ("fstp    %%st(0)"

                : "+m" (mod64_rnd) /* Dummy dependency */
                : 
                : "cc" );
}

/* a*b (mod p), where a,b < p < 2^62.
   Assumes %st(0) contains b/p and FPU is in round-to-zero mode.
*/
#define PRE2_MULMOD64(a,b,p) ({ \
  uint32_t r1, r2; \
  uint64_t tmp; \
  register uint64_t ret; \
  asm("fildll  %4"                  "\n\t" \
      "mov     %4, %%ebx"           "\n\t" \
      "mov     4+%4, %%ecx"         "\n\t" \
      "imul    %%edx, %%ebx"        "\n\t" \
      "fmul    %%st(1), %%st(0)"    "\n\t" \
      "imul    %%eax, %%ecx"        "\n\t" \
      "fistpll %1"                  "\n\t" \
      "mull    %4"                  "\n\t" \
      "mov     %%eax, %2"           "\n\t" \
      "add     %%ecx, %%ebx"        "\n\t" \
      "add     %%ebx, %%edx"        "\n\t" \
      "mov     %1, %%eax"           "\n\t" \
      "mov     %%edx, %3"           "\n\t" \
      "mov     4+%1, %%edx"         "\n\t" \
      "mov     %7, %%ebx"           "\n\t" \
      "mov     %6, %%ecx"           "\n\t" \
      "imul    %%eax, %%ebx"        "\n\t" \
      "imul    %%edx, %%ecx"        "\n\t" \
      "mull    %6"                  "\n\t" \
      "add     %%ebx, %%ecx"        "\n\t" \
      "mov     %3, %%ebx"           "\n\t" \
      "add     %%ecx,%%edx"         "\n\t" \
      "mov     %2, %%ecx"           "\n\t" \
      "sub     %%eax, %%ecx"        "\n\t" \
      "sbb     %%edx, %%ebx"        "\n\t" \
      "mov     %%ecx, %%eax"        "\n\t" \
      "mov     %%ebx, %%edx"        "\n\t" \
      "sub     %6, %%ecx"           "\n\t" \
      "sbb     %7, %%ebx"           "\n\t" \
      "jl      0f"                  "\n\t" \
      "mov     %%ecx, %%eax"        "\n\t" \
      "mov     %%ebx, %%edx"        "\n"   \
      "0:" \
      : "=&A" (ret), "=m" (tmp), "=m" (r1), "=m" (r2) \
      : "m" ((uint64_t)a), "0" ((uint64_t)b), \
        "rm" (((TU64_t)p).l), "rm" (((TU64_t)p).h), "m" (mod64_rnd) \
      : "%ebx", "%ecx", "cc" ); \
  ret; })

/* Pushes b/p onto the FPU stack. Assumes %st(0) contains 1.0/p.
 */
#define PRE2_MULMOD64_INIT(b) \
  asm volatile ("fildll  %1\n\t" \
                "fmul    %%st(1),%%st(0)" \
                : "+m" (mod64_rnd) \
                : "m" ((uint64_t)b) \
                : "cc" )

/* Pops b/p off the FPU stack.
 */
#define PRE2_MULMOD64_FINI() \
  asm volatile ("fstp    %%st(0)" \
                : "+m" (mod64_rnd) \
                : \
                : "cc" )
#endif


#if defined(NEED_sqrmod64) && !defined(HAVE_sqrmod64)
#define HAVE_sqrmod64
extern uint16_t mod64_rnd;

/* b^2 (mod p), where b < p < 2^62.
   Assumes %st(0) contains 1.0/p and FPU is in round-to-zero mode.
*/
static inline uint64_t sqrmod64(uint64_t b, TU64_t p)
{
  uint32_t r1;
  uint32_t r2;
  uint64_t tmp;
  register uint64_t ret;

  asm("fildll  %4"                  "\n\t"
      "mov     %4, %%ecx"           "\n\t"
      "mov     %4, %%eax"           "\n\t"
      "mov     4+%4, %%ebx"         "\n\t"
      "mul     %%eax"               "\n\t"
      "fmul    %%st(0), %%st(0)"    "\n\t"
      "imul    %%ebx, %%ecx"        "\n\t"
      "fmul    %%st(1), %%st(0)"    "\n\t"
      "mov     %%eax, %2"           "\n\t"
      "fistpll %1"                  "\n\t"
      "lea     (%%edx,%%ecx,2), %%edx\n\t"
      "mov     %%edx, %3"           "\n\t"
      "mov     %1, %%eax"           "\n\t"
      "mov     4+%1, %%edx"         "\n\t"
      "mov     %6, %%ebx"           "\n\t"
      "mov     %5, %%ecx"           "\n\t"
      "imul    %%eax, %%ebx"        "\n\t"
      "imul    %%edx, %%ecx"        "\n\t"
      "mull    %5"                  "\n\t"
      "add     %%ebx, %%ecx"        "\n\t"
      "mov     %3, %%ebx"           "\n\t"
      "add     %%ecx,%%edx"         "\n\t"
      "mov     %2, %%ecx"           "\n\t"
      "sub     %%eax, %%ecx"        "\n\t"
      "sbb     %%edx, %%ebx"        "\n\t"
      "mov     %%ecx, %%eax"        "\n\t"
      "mov     %%ebx, %%edx"        "\n\t"
      "sub     %5, %%ecx"           "\n\t"
      "sbb     %6, %%ebx"           "\n\t"
      "jl      0f"                  "\n\t"
      "mov     %%ecx, %%eax"        "\n\t"
      "mov     %%ebx, %%edx"        "\n\t"
      "0:"
      : "=&A" (ret), "=m" (tmp), "=m" (r1), "=m" (r2)
      : "m" (b), "g" (p.l), "g" (p.h), "m" (mod64_rnd)
      : "%ebx", "%ecx", "cc" );

  return ret;
}
#endif


#if defined(NEED_powmod64) && !defined(HAVE_powmod64)
#define HAVE_powmod64
/*
  b^n (mod p)
*/
uint64_t powmod64(uint64_t b, uint64_t n, uint64_t p) attribute ((pure));
#endif


#if defined(NEED_memset_fast32) && !defined(HAVE_memset_fast32)
#define HAVE_memset_fast32
/* store count copies of x starting at dst.
 */
static inline
void memset_fast32(uint_fast32_t *dst, uint_fast32_t x, uint_fast32_t count)
{
  register uint32_t tmp;
  register uint32_t *ptr;

  asm ("cld\n\t"
       "rep; stosl"
       : "=c" (tmp), "=D" (ptr),
         "=m" (*(struct { uint32_t dummy[count]; } *)dst)
       : "a" (x), "0" (count), "1" (dst)
       : "cc" );
}
#endif
