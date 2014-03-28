/* asm-ppc64.h -- (C) 2006 Mark Rodenkirch.

   PPC assember mulmod, powmod routines.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/


#if defined(NEED_mulmod64) && !defined(HAVE_mulmod64)
#define HAVE_mulmod64

#define MOD64_GREATEST_PRIME UINT64_C(9223372036854775783)
#define MOD64_SIEVE_LIMIT "2^63"

extern uint64_t  pMagic;
extern uint64_t  pShift;

extern void getMagic(uint64_t d);

#if USE_INLINE_MULMOD
static inline uint64_t mulmod64(uint64_t a, uint64_t b, uint64_t p)
{
  register uint64_t ret, tmp;

  asm ("li      %0, 64"          "\n\t"
       "sub     %0, %0, %6"      "\n\t"
       "mulld   r7, %1, %3"      "\n\t"
       "mulhdu  r8, %1, %3"      "\n\t"
       "mulhdu  %1, r7, %5"      "\n\t"
       "mulld   r9, r8, %5"      "\n\t"
       "mulhdu  r10, r8, %5"     "\n\t"
       "addc    r9, %1, r9"      "\n\t"
       "addze   r10, r10"        "\n\t"
       "srd     r9, r9, %6"      "\n\t"
       "sld     r10, r10, %0"    "\n\t"
       "or      r9, r9, r10"     "\n\t"
       "mulld   r9, r9, %4"      "\n\t"
       "sub     %0, r7, r9"      "\n\t"
       "cmpdi   cr6, %0, 0"      "\n\t"
       "bge+    cr6, 0f"         "\n\t"
       "add     %0, %0, %4"      "\n"
       "0:"
       : "=&r" (ret), "=&r" (tmp)
       : "1" (a), "r" (b), "r" (p), "r" (pMagic), "r" (pShift)
       : "r7", "r8", "r9", "r10", "cr6" );

  return ret;
}
#else
/* Use the code in mulmod-ppc64.S
 */
extern uint64_t mulmod(uint64_t a, uint64_t b, uint64_t p, uint64_t magic,
                       uint64_t shift) attribute ((const));

#define mulmod64(a, b, p) mulmod(a, b, p, pMagic, pShift)
#endif /* USE_INLINE_MULMOD */

static inline void mod64_init(uint64_t p) { }
static inline void mod64_fini(void) { }
#define PRE2_MULMOD64(a,b,p) mulmod64(a,b,p)
#define PRE2_MULMOD64_INIT(b) { }
#define PRE2_MULMOD64_FINI() { }
#endif


#if defined(NEED_powmod64) && !defined(HAVE_powmod64)
#define HAVE_powmod64

extern uint64_t  pMagic;
extern uint64_t  pShift;

extern void getMagic(uint64_t d);

extern uint64_t expmod(uint64_t a, uint64_t b, uint64_t p, uint64_t magic,
                       uint64_t shift) attribute ((const));

#define powmod64(b, e, p) expmod(b, e, p, pMagic, pShift)
#endif
