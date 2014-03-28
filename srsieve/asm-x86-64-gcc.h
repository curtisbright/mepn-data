/* asm-x86-64-gcc.h -- (C) Geoffrey Reynolds, July 2006.

   Inline x86-64 assember routines for GCC.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

/* This file may be included from a number of different headers. It should
   only define FEATURE if NEED_FEATURE is defined but HAVE_FEATURE is not.
*/

#if defined(NEED_mulmod32) && !defined(HAVE_mulmod32)
#define HAVE_mulmod32
static inline uint32_t mulmod32(uint32_t a, uint32_t b, uint32_t p)
{
  register uint32_t tmp, ret;

  asm ("mull %3"
       : "=d" (ret), "=a" (tmp)
       : "1" (a), "rm" (b) );

  asm ("divl %4"
       : "=d" (ret), "=a" (tmp)
       : "0" (ret), "1" (tmp), "rm" (p) );

  return ret;
}
#endif

#if USE_FPU_MULMOD
#if defined(NEED_mulmod64) && !defined(HAVE_mulmod64)
#define HAVE_mulmod64

#define MOD64_GREATEST_PRIME UINT64_C(4611686018427387847)
#define MOD64_SIEVE_LIMIT "2^62"

/* Saved copy of FPU precision and rounding mode (bits 8,9,10,11). */
extern uint16_t mod64_rnd;

/* a*b (mod p), where a,b < p < 2^62.
   Assumes %st(0) contains 1.0/p and FPU is in round-to-zero mode.
*/
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t p)
{
  register uint64_t ret, r1;
  uint64_t tmp;

  asm ("fildll  %3"                 "\n\t"
       "fildll  %4"                 "\n\t"
       "imul    %1, %0"             "\n\t"
       "fmulp   %%st(0), %%st(1)"   "\n\t"
       "fmul    %%st(1), %%st(0)"   "\n\t"
       "fistpll %2"                 "\n\t"
       "mov     %2, %1"             "\n\t"
       "imul    %5, %1"             "\n\t"
       "sub     %1, %0"             "\n\t"
       "mov     %0, %1"             "\n\t"
       "sub     %5, %0"             "\n\t"
       "cmovl   %1, %0"

       : "=r" (ret), "=r" (r1), "=m" (tmp)
       : "m" (a), "m" (b), "rm" (p), "0" (a), "1" (b), "m" (mod64_rnd)
       : "cc" );

  return ret;
}

/* Changes the FPU into extended precision round-to-zero mode and pushes
   1.0/p onto the FPU stack. Saves the old FPU mode in mod64_rnd.
*/
static inline void mod64_init(uint64_t p)
{
  uint16_t tmp;

  asm volatile ("fildll  %2"               "\n\t"
                "fld1"                     "\n\t"
                "fnstcw  %0"               "\n\t"
                "mov     %0, %%ax"         "\n\t"
                "and     $0x0F00, %%ax"    "\n\t"
                "orw     $0x0F00, %0"      "\n\t"  /* Set bits 8,9,10,11 */
                "mov     %%ax, %1"         "\n\t"  /* Save old bits */
                "fldcw   %0"               "\n\t"
                "fdivp   %%st(0),%%st(1)"

                : "=m" (tmp), "+m" (mod64_rnd)
                : "m" (p)
                : "%rax", "cc" );
}

/* Same as mod64_init(p) but does not set FPU rounding.
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

/* Pops 1.0/p off the FPU stack and restores the previous FPU precision and
   rounding rounding mode from mod64_rnd.
*/
static inline void mod64_fini(void)
{
  uint16_t tmp;

  asm volatile ("fstp    %%st(0)"          "\n\t"
                "fnstcw  %0"               "\n\t"
                "mov     %0, %%ax"         "\n\t"
                "and     $0xF0FF, %%ax"    "\n\t"  /* Clear bits 8,9,10,11 */
                "or      %1, %%ax"         "\n\t"  /* Restore old bits */
                "mov     %%ax, %0"         "\n\t"
                "fldcw   %0"

                : "=m" (tmp), "+m" (mod64_rnd)
                : 
                : "%rax", "cc" );
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
  register uint64_t ret, r1; \
  uint64_t tmp; \
  asm ("fildll  %3"                 "\n\t" \
       "imul    %1, %0"             "\n\t" \
       "fmul    %%st(1), %%st(0)"   "\n\t" \
       "fistpll %2"                 "\n\t" \
       "mov     %2, %1"             "\n\t" \
       "imul    %5, %1"             "\n\t" \
       "sub     %1, %0"             "\n\t" \
       "mov     %0, %1"             "\n\t" \
       "sub     %5, %0"             "\n\t" \
       "cmovl   %1, %0" \
       : "=r" (ret), "=r" (r1), "=m" (tmp) \
       : "m" (a), "0" (b), "rm" (p), "1" (a), "m" (mod64_rnd) \
       : "cc" ); \
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

#endif /* defined(NEED_mulmod64) && !defined(HAVE_mulmod64) */


#if defined(NEED_sqrmod64) && !defined(HAVE_sqrmod64)
#define HAVE_sqrmod64
extern uint16_t mod64_rnd;

/* b^2 (mod p), where b < p < 2^62.
   Assumes %st(0) contains 1.0/p and FPU is in round-to-zero mode.
*/
static inline uint64_t sqrmod64(uint64_t a, uint64_t p)
{
  register uint64_t ret, r1;
  uint64_t tmp;

  asm ("fildll  %3"                 "\n\t"
       //       "mov     %3, %0"             "\n\t"
       "imul    %0, %0"             "\n\t"
       "fmul    %%st(0), %%st(0)"   "\n\t"
       "fmul    %%st(1), %%st(0)"   "\n\t"
       "fistpll %2"                 "\n\t"
       "mov     %2, %1"             "\n\t"
       "imul    %4, %1"             "\n\t"
       "sub     %1, %0"             "\n\t"
       "mov     %0, %1"             "\n\t"
       "sub     %4, %0"             "\n\t"
       "cmovl   %1, %0"

       : "=r" (ret), "=&r" (r1), "=m" (tmp)
       : "m" (a), "rm" (p), "0" (a), "m" (mod64_rnd)
       : "cc" );

  return ret;
}
#endif /* defined(NEED_sqrmod64) && !defined(HAVE_sqrmod64) */

#else /* USE_FPU_MULMOD */

#if defined(NEED_mulmod64) && !defined(HAVE_mulmod64)
#define HAVE_mulmod64

#define MOD64_GREATEST_PRIME UINT64_C(2251799813685119)
#define MOD64_SIEVE_LIMIT "2^51"

/* Saved copy of SSE rounding mode (bits 13,14). */
extern uint16_t mod64_rnd;

extern double one_over_p attribute ((aligned(16)));
extern double b_over_p attribute ((aligned(16)));

/* a*b (mod p), where a,b < p < 2^51. Assumes SSE operations round to zero.
*/
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t p)
{
  register uint64_t ret, tmp;
  register double x, y;

  asm ("cvtsi2sdq %0, %2"    "\n\t"
       "cvtsi2sdq %5, %3"    "\n\t"
       "imul      %5, %0"    "\n\t"
       "mulsd     %7, %2"    "\n\t"
       "mulsd     %2, %3"    "\n\t"
       "cvtsd2si  %3, %1"    "\n\t"
       "imul      %6, %1"    "\n\t"
       "sub       %1, %0"    "\n\t"
       "mov       %0, %1"    "\n\t"
       "sub       %6, %0"    "\n\t"
       "cmovl     %1, %0"

       : "=r" (ret), "=&r" (tmp), "=&x" (x), "=&x" (y)
       : "0" (a), "rm" (b), "rm" (p), "xm" (one_over_p), "m" (mod64_rnd)
       : "cc" );

  return ret;
}

/* Set one_over_p to 1.0/p and SSE rounding mode to round-to-zero.
   Saves the old rounding mode in mod64_rnd.
*/
static inline void mod64_init(uint64_t p)
{
  uint32_t tmp;
  register double x;

  asm volatile ("stmxcsr   %0"              "\n\t"
                "mov       %0, %%ax"        "\n\t"
                "and       $24576, %%ax"    "\n\t"
                "orw       $24576, %0"      "\n\t"  /* Set bits 13,14 */
                "mov       %%ax, %1"        "\n\t"  /* Save old bits */
                "ldmxcsr   %0"              "\n\t"
                "mov       $1, %%eax"       "\n\t"
                "cvtsi2sdq %4, %3"          "\n\t"
                "cvtsi2sd  %%eax, %2"       "\n\t"
                "divsd     %3, %2"

                : "=m" (tmp), "+m" (mod64_rnd), "=x" (one_over_p), "=x" (x)
                : "rm" (p)
                : "%rax", "cc" );
}

/* Restore the previous SSE rounding mode from mod64_rnd.
*/
static inline void mod64_fini(void)
{
  uint32_t tmp;

  asm volatile ("stmxcsr   %0"              "\n\t"
                "mov       %0, %%ax"        "\n\t"
                "and       $40959, %%ax"    "\n\t"  /* Clear bits 13,14 */
                "or        %1, %%ax"        "\n\t"  /* Restore old bits */
                "mov       %%ax, %0"        "\n\t"
                "ldmxcsr   %0"

                : "=m" (tmp), "+m" (mod64_rnd)
                : 
                : "%rax", "cc" );
}

/* a*b (mod p), where a,b < p < 2^51. Assumes SSE operations round to zero.
*/
#define PRE2_MULMOD64(a,b,p) ({ \
  register uint64_t ret, tmp; \
  register double x; \
  asm ("cvtsi2sd  %0, %2"    "\n\t" \
       "imul      %4, %0"    "\n\t" \
       "mulsd     %6, %2"    "\n\t" \
       "cvtsd2si  %2, %1"    "\n\t" \
       "imul      %5, %1"    "\n\t" \
       "sub       %1, %0"    "\n\t" \
       "mov       %0, %1"    "\n\t" \
       "sub       %5, %0"    "\n\t" \
       "cmovl     %1, %0" \
       : "=r" (ret), "=&r" (tmp), "=&x" (x) \
       : "0" (a), "r" (b), "r" (p), "x" (b_over_p), "m" (mod64_rnd) \
       : "cc" ); \
  ret; })

/* Set b_over_p to b/p. Assumes one_over_p contains 1.0/p.
 */
#define PRE2_MULMOD64_INIT(b) \
  asm ("cvtsi2sdq %2, %0"    "\n\t" \
       "mulsd     %3, %0" \
       : "=&x" (b_over_p), "+m" (mod64_rnd) \
       : "rm" ((uint64_t)b), "xm" (one_over_p) \
       : "cc" )

#define PRE2_MULMOD64_FINI() {}

#endif /* defined(NEED_mulmod64) && !defined(HAVE_mulmod64) */


#if defined(NEED_sqrmod64) && !defined(HAVE_sqrmod64)
#define HAVE_sqrmod64
extern uint16_t mod64_rnd;

/* b^2 (mod p), where b < p < 2^51. Assumes SSE operations round to zero.
*/
static inline uint64_t sqrmod64(uint64_t b, uint64_t p)
{
  register uint64_t ret, tmp;
  register double x;

  asm ("cvtsi2sd  %0, %2"    "\n\t"
       "imul      %0, %0"    "\n\t"
       "mulsd     %2, %2"    "\n\t"
       "mulsd     %5, %2"    "\n\t"
       "cvtsd2si  %2, %1"    "\n\t"
       "imul      %4, %1"    "\n\t"
       "sub       %1, %0"    "\n\t"
       "mov       %0, %1"    "\n\t"
       "sub       %4, %0"    "\n\t"
       "cmovl     %1, %0"

       : "=r" (ret), "=&r" (tmp), "=&x" (x)
       : "0" (b), "rm" (p), "xm" (one_over_p), "m" (mod64_rnd)
       : "cc" );

  return ret;
}
#endif /* defined(NEED_sqrmod64) && !defined(HAVE_sqrmod64) */

#endif /* USE_FPU_MULMOD */


#if defined(NEED_powmod64) && !defined(HAVE_powmod64)
#define HAVE_powmod64
/*
  b^n (mod p)
*/
#if USE_FPU_MULMOD
uint64_t powmod64_k8_fpu(uint64_t b, uint64_t n, uint64_t p)
     attribute ((pure));
#define powmod64(b,n,p) powmod64_k8_fpu(b,n,p)
#else
uint64_t powmod64_k8(uint64_t b, uint64_t n, uint64_t p, double invp)
     attribute ((const));
#define powmod64(b,n,p) powmod64_k8(b,n,p,one_over_p)
#endif
#endif


#if defined(NEED_memset_fast32) && !defined(HAVE_memset_fast32)
#define HAVE_memset_fast32
/* store count copies of x starting at dst.
 */
static inline
void memset_fast32(uint_fast32_t *dst, uint_fast32_t x, uint_fast32_t count)
{
  register uint_fast32_t tmp;
  register uint_fast32_t *ptr;

  asm ("rep"    "\n\t"
#if (UINT_FAST32_MAX==UINT64_MAX)
       "stosq"
#else
       "stosl"
#endif
       : "=c" (tmp), "=D" (ptr),
         "=m" (*(struct { uint_fast32_t dummy[count]; } *)dst)
       : "a" (x), "0" (count), "1" (dst)
       : "cc" );
}
#endif
