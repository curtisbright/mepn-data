/* factors.c -- (C) Geoffrey Reynolds, May 2006.

   Factors file routines and misc factoring related functions.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "srsieve.h"
#include "arithmetic.h"

/* Close and re-open Factors file between writes once p exceeds this
   threshold.
*/
#define FACTORS_FILE_CLOSE_THRESHOLD 1000000000

const char *factors_file_name = "srfactors.txt";

int is_factor(uint64_t k, int64_t c, uint32_t n, uint64_t p)
{
  assert(p > 0);

  if (p < UINT32_MAX)
  {
    uint32_t res;

    res = addmod32(mulmod32(powmod32(umod64_32(base,p),n,p),
                             umod64_32(k,p),p), lmod64_32(c,p),p);

    return res == 0;
  }
  else
  {
    uint64_t res;

#if (USE_ASM && (__i386__ || (__x86_64__ && USE_FPU_MULMOD)) && __GNUC__)
    /* We don't know whether 1/p or b/p is on the top of the FPU stack, so
       reinitialise without changing rounding mode (which is already set).
    */
    mod64_init0(p);
#endif

    res = addmod64(mulmod64(powmod64(umod64(base,p),n,p),umod64(k,p),p),
                   lmod64(c,p),p);

#if (USE_ASM && (__i386__ || (__x86_64__ && USE_FPU_MULMOD)) && __GNUC__)
    mod64_fini0();
#endif

    return (res == UINT64_C(0));
  }

}

static FILE *factors_file = NULL;
static void open_factors_file(void)
{
  factors_file = xfopen(factors_file_name, "a", error);
}

void close_factors_file(void)
{
  xfclose(factors_file,factors_file_name);
  factors_file = NULL;
}

void save_factor(uint64_t k, int64_t c, uint32_t n, uint64_t p)
{
  if (!factors_opt)
    return;

  if (factors_file == NULL)
    open_factors_file();

  if (fprintf(factors_file,"%"PRIu64" | %s\n",p,kbnc_str(k,base,n,c)) < 0)
    warning("Could not write to factors file `%s'.", factors_file_name);

  if (p > FACTORS_FILE_CLOSE_THRESHOLD)
    close_factors_file();
}
