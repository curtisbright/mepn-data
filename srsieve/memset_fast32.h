/* memset_fast32.h -- (C) Geoffrey Reynolds, September 2006.

   memset_fast32() and memset32_8 functions.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _MEMSET_FAST32_H
#define _MEMSET_FAST32_H

#if USE_ASM
# define NEED_memset_fast32
# if (__i386__ && __GNUC__)
#  include "asm-i386-gcc.h"
# elif defined(__ppc64__) || defined(__powerpc64__)
#  include "asm-ppc64.h"
# elif (__x86_64__ && __GNUC__)
#  include "asm-x86-64-gcc.h"
# endif
#endif

#ifndef HAVE_memset_fast32
#define HAVE_memset_fast32
/* store count copies of x starting at dst.
 */
static inline
void memset_fast32(uint_fast32_t *dst, uint_fast32_t x, uint_fast32_t count)
{
  while (count > 0)
    dst[--count] = x;
}
#endif

#endif /* _MEMSET_FAST32_H */
