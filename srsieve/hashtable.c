/* hashtable.c -- (C) Geoffrey Reynolds, April 2006.

   Hash table implementation of the baby steps giant steps lookup table.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include "srsieve.h"
#include "hashtable.h"

HASH_T *htable;
HASH_T *olist;
uint32_t hsize;
uint32_t hsize_minus1;
uint64_t *BJ64;
#if !CONST_EMPTY_SLOT
HASH_T empty_slot;
#endif

#if COLLECT_HASHTABLE_STATS
uint64_t hash_insertions, hash_lookups;
static uint64_t insert_collisions, lookup_collisions;
#endif

void init_hashtable(uint32_t M)
{
  assert(hsize == 0);
  assert(M <= HASH_MAX_ELTS);

  M = MAX(M,HASH_MINIMUM_ELTS);
  for (hsize = UINT32_C(1)<<HASH_MINIMUM_SHIFT; hsize < M/hash_max_density; )
    hsize <<= 1;
  hsize_minus1 = hsize - 1;

  htable = xmalloc(hsize*sizeof(HASH_T));
  olist = xmalloc(M*sizeof(HASH_T));

#if CONST_EMPTY_SLOT
  BJ64 = xmalloc(M*sizeof(uint64_t));
#else
  /* The j values are all in the range 0 <= j < M, so we can use M as an
     empty slot marker as long as we fill BJ[M] with a value that will never
     match a real b^j value. Since b^j is always in the range 0 <= b^j < p
     for some prime p, any value larger than all 32/64 bit primes will do.
  */
  empty_slot = M;
  BJ64 = xmalloc((M+1)*sizeof(uint64_t));
  BJ32[empty_slot] = UINT32_MAX;
  BJ64[empty_slot] = UINT64_MAX;
#endif

  debug("Using a hashtable of %"PRIu32" %s elements (maximum density %.2f)",
        hsize, HASH_DESC, (double)M/hsize);

#if COLLECT_HASHTABLE_STATS
  insert_collisions = lookup_collisions = hash_insertions = hash_lookups = 0;
#endif
}

void fini_hashtable(void)
{
  assert (hsize > 0);

  free(BJ64);
  free(olist);
  free(htable);
  hsize = 0;

#if COLLECT_HASHTABLE_STATS
  debug("Hashtable stats: %"PRIu64" insertions (%"PRIu64"=%.2f%% collisions)",
        hash_insertions, insert_collisions,
        100.0 * insert_collisions / hash_insertions);
  debug("Hashtable stats: %"PRIu64" lookups (%"PRIu64"=%.2f%% collisions)",
        hash_lookups, lookup_collisions,
        100.0 * lookup_collisions / hash_lookups);
#endif
}
