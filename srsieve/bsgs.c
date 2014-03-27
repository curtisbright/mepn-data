/* bsgs.c -- (C) Geoffrey Reynolds, April 2006.

   Implementation of a baby step giant step algorithm for finding all
   n in the range nmin <= n <= nmax satisfying b^n=d_i (mod p) where b
   and each d_i are relatively prime to p.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "srsieve.h"
#include "arithmetic.h"
#include "hashtable.h"

#include "filter.c"

double baby_step_factor = DEFAULT_BABY_STEP_FACTOR;

/*
  Giant step baby step algorithm, from Wikipaedea:

  input: A cyclic group G of order n, having a generator a and an element b.

  Output: A value x satisfying a^x = b (mod n).

  1. m <-- ceiling(sqrt(n))
  2. For all j where 0 <= j < m:
     1. Compute a^j mod n and store the pair (j, a^j) in a table.
  3. Compute a^(-m).
  4. g <-- b.
  5. For i = 0 to (m - 1):
     1. Check if g is the second component (a^j) of any pair in the table.
     2. If so, return im + j.
     3. If not, g <-- ga^(-m) mod n.
*/

static uint32_t m;    /* Number of baby steps */
static uint32_t M;    /* Number of giant steps */

static uint32_t sieve_low;
static uint32_t sieve_range;

static uint32_t *C32;   /* List of subsequences that have passed filtering. */
static uint64_t *D64;
#define D32 ((uint32_t *)D64)

static uint64_t *ck64;
static uint64_t *bd64;
#define ck32 ((uint32_t *)ck64)
#define bd32 ((uint32_t *)bd64)


static uint32_t baby_steps32(uint32_t b, uint32_t bj0, uint32_t p)
{
  uint32_t j, bj;

  clear_hashtable();
  for (j = 0, bj = bj0; j < m; j++)
  {
    insert32(j, bj);
    bj = mulmod32(bj,b,p);
    if (bj == bj0)
      return j+1;
  }

  return 0;
}

#define NO_SOLUTION UINT32_MAX

static void bsgs_small(uint64_t p64)
{
  uint32_t i, j, k;
  uint32_t order_of_b_mod_p, solutions = 0, first_solved = NO_SOLUTION;
  uint32_t b, bm, p = p64;

  b = base % p;
  if (b == 0)
    return;

#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
  getMagic(p);
#endif

  for (i = 0; i < seq_count; i++)
    ck32[i] = mulmod32(lmod64_32(-SEQ[i].c,p),invmod32(umod64_32(SEQ[i].k,p),p),p);

  bd32[0] = 1;
  for (i = 1, bm = j = invmod32(b,p); i < subseq_Q; i++)
  {
    bd32[i] = bm;
    bm = mulmod32(bm,j,p);
  }

  for (i = 0; i < subseq_count; i++)
  {
    D32[i] = mulmod32(ck32[SUBSEQ[i].seq],bd32[SUBSEQ[i].d],p);
    C32[i] = NO_SOLUTION;
  }

  /* b <- base^Q (mod p) */
  b = powmod32(b,subseq_Q,p);

  /* Baby steps. */
  order_of_b_mod_p = baby_steps32(b,powmod32(b,sieve_low,p),p);

  if (order_of_b_mod_p > 0)
  {
    for (k = 0; k < subseq_count; k++)
      for (j = lookup32(D32[k]); j < sieve_range; j += order_of_b_mod_p)
        eliminate_term(k,sieve_low+j,p);
  }
  else
  {
    /* First giant step. */
    for (k = 0; k < subseq_count; k++)
      if ((j = lookup32(D32[k])) != HASH_NOT_FOUND)
      {
        solutions++;
        C32[k] = j;
        if (solutions == 1) /* first solution */
          first_solved = k;
      }

    /* Remaining giant steps */
    if (M > 1)
    {
      bm = powmod32(bm,m,p); /* bm <- 1/b^m (mod p) */
      for (i = 1; i < M && solutions <= subseq_count; i++)
        for (k = 0; k < subseq_count; k++)
          if (C32[k] == NO_SOLUTION || first_solved == k)
          {
            D32[k] = mulmod32(D32[k],bm,p);
            if ((j = lookup32(D32[k])) != HASH_NOT_FOUND)
            {
              solutions++;
              if (first_solved == k) /* repeat solution */
              {
                order_of_b_mod_p = i*m+j - C32[k];
                first_solved = NO_SOLUTION;   /* no more repeats needed */
              }
              else
              {
                C32[k] = i*m+j;
                if (solutions == 1) /* first solution */
                  first_solved = k;
              }
            }
          }
    }

    if (order_of_b_mod_p > 0)
      for (k = 0; k < subseq_count; k++)
        for (j = C32[k]; j < sieve_range; j += order_of_b_mod_p)
          eliminate_term(k,sieve_low+j,p);
    else
      for (k = 0; k < subseq_count; k++)
        if (C32[k] != NO_SOLUTION)
          eliminate_term(k,sieve_low+C32[k],p);
  }
}

#if USE_32BIT_SIEVE
static uint32_t (*setup32_flt)(uint32_t);
static void (*setup32_noflt)(uint32_t);
static uint32_t bQ32;

static uint32_t setup32_flt_bigkc(uint32_t p)
{
  uint32_t inv_b, bm;
  uint32_t i, j, seq;

  bd32[0] = 1;
  for (i = 1, bm = inv_b = invmod32(base,p); i < subseq_Q; i++)
  {
    bd32[i] = bm;
    bm = mulmod32(bm,inv_b,p);
  }
  bQ32 = bm;

  do_filters32(p);
  for (i = 0; i < seq_count; i++)
    ck32[i] = 0;

  /* Compute -c/(k*b^d) (mod p) for each subsequence. */
  for (i = j = 0; i < subseq_count; i++)
    if (filter_p[SUBSEQ[i].filter])
    {
      seq = SUBSEQ[i].seq;
      if (ck32[seq] == 0)
        ck32[seq] = mulmod32(lmod64_32(-SEQ[seq].c,p),
                             invmod32(umod64_32(SEQ[seq].k,p),p),p);
      D32[j] = mulmod32(ck32[seq],bd32[SUBSEQ[i].d],p);
      C32[j] = i;
      j++;
    }

  return j;
}

static void setup32_noflt_bigkc(uint32_t p)
{
  uint32_t inv_b, bm;
  uint32_t i;

  bd32[0] = 1;
  for (i = 1, bm = inv_b = invmod32(base,p); i < subseq_Q; i++)
  {
    bd32[i] = bm;
    bm = mulmod32(bm,inv_b,p);
  }
  bQ32 = bm;

  for (i = 0; i < seq_count; i++)
    ck32[i] = mulmod32(lmod64_32(-SEQ[i].c,p),invmod32(umod64_32(SEQ[i].k,p),p),p);

  /* Compute -c/(k*b^d) (mod p) for each subsequence. */
  for (i = 0; i < subseq_count; i++)
    D32[i] = mulmod32(ck32[SUBSEQ[i].seq],bd32[SUBSEQ[i].d],p);
}

/* Used when all sequences k*b^n+c satisfy k,|c| < p.
 */
static uint32_t setup32_flt_smallkc(uint32_t p)
{
  uint32_t inv_b, bm;
  uint32_t i, j, seq;

  bd32[0] = 1;
  for (i = 1, bm = inv_b = invmod32(base,p); i < subseq_Q; i++)
  {
    bd32[i] = bm;
    bm = mulmod32(bm,inv_b,p);
  }
  bQ32 = bm;

  do_filters32(p);
  for (i = 0; i < seq_count; i++)
    ck32[i] = 0;

  /* Compute -c/(k*b^d) (mod p) for each subsequence. */
  for (i = j = 0; i < subseq_count; i++)
    if (filter_p[SUBSEQ[i].filter])
    {
      seq = SUBSEQ[i].seq;
      if (ck32[seq] == 0)
        ck32[seq] = mulmod32(mod32(-SEQ[seq].c,p),invmod32(SEQ[seq].k,p),p);
      D32[j] = mulmod32(ck32[seq],bd32[SUBSEQ[i].d],p);
      C32[j] = i;
      j++;
    }

  return j;
}

static void setup32_noflt_smallkc(uint32_t p)
{
  uint32_t inv_b, bm;
  uint32_t i;

  bd32[0] = 1;
  for (i = 1, bm = inv_b = invmod32(base,p); i < subseq_Q; i++)
  {
    bd32[i] = bm;
    bm = mulmod32(bm,inv_b,p);
  }
  bQ32 = bm;

  for (i = 0; i < seq_count; i++)
    ck32[i] = mulmod32(mod32(-SEQ[i].c,p),invmod32(SEQ[i].k,p),p);

  /* Compute -c/(k*b^d) (mod p) for each subsequence. */
  for (i = 0; i < subseq_count; i++)
    D32[i] = mulmod32(ck32[SUBSEQ[i].seq],bd32[SUBSEQ[i].d],p);
}

static void (*bsgs32)(uint64_t);

static void bsgs32_flt(uint64_t p64)
{
  uint32_t i, j, k, cc;
  uint32_t b, p = p64;

#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
  getMagic(p);
#endif

  if ((cc = setup32_flt(p)) == 0)
    return;

  /* b <- base^Q (mod p) */
  b = powmod32(base,subseq_Q,p);

  /* Baby steps. */
  if ((i = baby_steps32(b,powmod32(b,sieve_low,p),p)) > 0)
  {
    for (k = 0; k < cc; k++)
      for (j = lookup32(D32[k]); j < sieve_range; j += i)
        eliminate_term(C32[k],sieve_low+j,p);
  }
  else
  {
    /* First giant step. */
    for (k = 0; k < cc; k++)
      if ((j = lookup32(D32[k])) != HASH_NOT_FOUND)
        eliminate_term(C32[k],sieve_low+j,p);

    /* Remaining giant steps */
    if (M > 1)
    {
      b = powmod32(bQ32,m,p); /* b <- 1/b^Qm (mod p) */
      for (i = 1; i < M; i++)
        for (k = 0; k < cc; k++)
        {
          D32[k] = mulmod32(D32[k],b,p);
          if ((j = lookup32(D32[k])) != HASH_NOT_FOUND)
            eliminate_term(C32[k],sieve_low+i*m+j,p);
        }
    }
  }
}

static void bsgs32_noflt(uint64_t p64)
{
  uint32_t i, j, k;
  uint32_t b, p = p64;

#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
  getMagic(p);
#endif

  setup32_noflt(p);

  /* b <- base^Q (mod p) */
  b = powmod32(base,subseq_Q,p);

  /* Baby steps. */
  if ((i = baby_steps32(b,powmod32(b,sieve_low,p),p)) > 0)
  {
    for (k = 0; k < subseq_count; k++)
      for (j = lookup32(D32[k]); j < sieve_range; j += i)
        eliminate_term(k,sieve_low+j,p);
  }
  else
  {
    /* First giant step. */
    for (k = 0; k < subseq_count; k++)
      if ((j = lookup32(D32[k])) != HASH_NOT_FOUND)
        eliminate_term(k,sieve_low+j,p);

    /* Remaining giant steps */
    if (M > 1)
    {
      b = powmod32(bQ32,m,p); /* b <- 1/b^Qm (mod p) */
      for (i = 1; i < M; i++)
        for (k = 0; k < subseq_count; k++)
        {
          D32[k] = mulmod32(D32[k],b,p);
          if ((j = lookup32(D32[k])) != HASH_NOT_FOUND)
            eliminate_term(k,sieve_low+i*m+j,p);
        }
    }
  }
}
#endif /* USE_32BIT_SIEVE */

static uint32_t baby_steps64(uint64_t b, uint64_t bj0, uint64_t p)
{
  uint64_t bj;
  uint32_t j;

  clear_hashtable();
  PRE2_MULMOD64_INIT(b);
  for (j = 0, bj = bj0; j < m; j++)
  {
    insert64(j, bj);
    bj = PRE2_MULMOD64(bj,b,p);
    if (bj == bj0)
    {
      PRE2_MULMOD64_FINI();
      return j+1;
    }
  }

  PRE2_MULMOD64_FINI();
  return 0;
}

static uint32_t (*setup64_flt)(uint64_t);
static void (*setup64_noflt)(uint64_t);
static uint64_t bQ64;

static uint32_t setup64_flt_bigkc(uint64_t p)
{
  uint64_t inv_b, bm;
  uint32_t i, j, seq;

  /* Precompute 1/b^d (mod p) for 0 <= d <= Q.
   */
  bd64[0] = 1;
  inv_b = invmod32_64(base,p);
  PRE2_MULMOD64_INIT(inv_b);
  for (i = 1, bm = inv_b; i < subseq_Q; i++)
  {
    bd64[i] = bm;
    bm = PRE2_MULMOD64(bm,inv_b,p);
  }
  PRE2_MULMOD64_FINI();
  bQ64 = bm;

  do_filters64(p);
  for (i = 0; i < seq_count; i++)
    ck64[i] = 0;

  /* Compute -c/(k*b^d) (mod p) for each subsequence. */
  for (i = j = 0; i < subseq_count; i++)
    if (filter_p[SUBSEQ[i].filter])
    {
      seq = SUBSEQ[i].seq;
      if (ck64[seq] == 0)
        ck64[seq] =
          mulmod64(lmod64(-SEQ[seq].c,p),invmod64(umod64(SEQ[seq].k,p),p),p);
      D64[j] = mulmod64(ck64[seq],bd64[SUBSEQ[i].d],p);
      C32[j] = i;
      j++;
    }

  return j;
}

static void setup64_noflt_bigkc(uint64_t p)
{
  uint64_t inv_b, bm;
  uint32_t i;

  /* Precompute 1/b^d (mod p) for 0 <= d <= Q.
   */
  bd64[0] = 1;
  inv_b = invmod32_64(base,p);
  PRE2_MULMOD64_INIT(inv_b);
  for (i = 1, bm = inv_b; i < subseq_Q; i++)
  {
    bd64[i] = bm;
    bm = PRE2_MULMOD64(bm,inv_b,p);
  }
  PRE2_MULMOD64_FINI();
  bQ64 = bm;

  for (i = 0; i < seq_count; i++)
    ck64[i] = mulmod64(lmod64(-SEQ[i].c,p),invmod64(umod64(SEQ[i].k,p),p),p);

  /* Compute -c/(k*b^d) (mod p) for each subsequence. */
  for (i = 0; i < subseq_count; i++)
    D64[i] = mulmod64(ck64[SUBSEQ[i].seq],bd64[SUBSEQ[i].d],p);
}

/* Used when all sequences k*b^n+c satisfy k,|c| < 2^32.
 */
static uint32_t setup64_flt_smallkc(uint64_t p)
{
  uint64_t inv_b, bm;
  uint32_t i, j, seq;

  bd64[0] = 1;
  inv_b = invmod32_64(base,p);
  PRE2_MULMOD64_INIT(inv_b);
  for (i = 1, bm = inv_b; i < subseq_Q; i++)
  {
    bd64[i] = bm;
    bm = PRE2_MULMOD64(bm,inv_b,p);
  }
  PRE2_MULMOD64_FINI();
  bQ64 = bm;

  do_filters64(p);
  for (i = 0; i < seq_count; i++)
    ck64[i] = 0;

  for (i = j = 0; i < subseq_count; i++)
    if (filter_p[SUBSEQ[i].filter])
    {
      seq = SUBSEQ[i].seq;
      if (ck64[seq] == 0)
        ck64[seq] = mulmod64(mod64(-SEQ[seq].c,p),invmod32_64(SEQ[seq].k,p),p);
      D64[j] = mulmod64(ck64[seq],bd64[SUBSEQ[i].d],p);
      C32[j] = i;
      j++;
    }

  return j;
}

static void setup64_noflt_smallkc(uint64_t p)
{
  uint64_t inv_b, bm;
  uint32_t i;

  /* Precompute 1/b^d (mod p) for 0 <= d <= Q.
   */
  bd64[0] = 1;
  inv_b = invmod32_64(base,p);
  PRE2_MULMOD64_INIT(inv_b);
  for (i = 1, bm = inv_b; i < subseq_Q; i++)
  {
    bd64[i] = bm;
    bm = PRE2_MULMOD64(bm,inv_b,p);
  }
  PRE2_MULMOD64_FINI();
  bQ64 = bm;

  for (i = 0; i < seq_count; i++)
    ck64[i] = mulmod64(mod64(-SEQ[i].c,p),invmod32_64(SEQ[i].k,p),p);

  /* Compute -c/(k*b^d) (mod p) for each subsequence. */
  for (i = 0; i < subseq_count; i++)
    D64[i] = mulmod64(ck64[SUBSEQ[i].seq],bd64[SUBSEQ[i].d],p);
}

static void (*bsgs64)(uint64_t);

static void bsgs64_flt(uint64_t p)
{
  uint32_t i, j, k, cc;
  uint64_t b;

#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
  getMagic(p);
#endif

  mod64_init(p);

  if ((cc = setup64_flt(p)) == 0)
  {
    mod64_fini();
    return;
  }

  /* b <- base^Q (mod p) */
  b = powmod64(base,subseq_Q,p);

  /* Baby steps. */
  if ((i = baby_steps64(b,powmod64(b,sieve_low,p),p)) > 0)
  {
    /* i is the order of b (mod p). This is all the information we need to
       determine every solution for this p, no giant steps are needed.
    */
    for (k = 0; k < cc; k++)
      for (j = lookup64(D64[k]); j < sieve_range; j += i)
        eliminate_term(C32[k],sieve_low+j,p);
  }
  else
  {
    /* First giant step. */
    for (k = 0; k < cc; k++)
      if ((j = lookup64(D64[k])) != HASH_NOT_FOUND)
        eliminate_term(C32[k],sieve_low+j,p);

    /* Remaining giant steps. */
    if (M > 1)
    {
      b = powmod64(bQ64,m,p); /* b <- 1/b^m (mod p) */
      PRE2_MULMOD64_INIT(b);
      for (i = 1; i < M; i++)
        for (k = 0; k < cc; k++)
        {
          D64[k] = PRE2_MULMOD64(D64[k],b,p);
          if ((j = lookup64(D64[k])) != HASH_NOT_FOUND)
            eliminate_term(C32[k],sieve_low+i*m+j,p);
        }
      PRE2_MULMOD64_FINI();
    }
  }

  mod64_fini();
}

static void bsgs64_noflt(uint64_t p)
{
  uint64_t b;
  uint32_t i, j, k;

#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
  getMagic(p);
#endif

  mod64_init(p);

  setup64_noflt(p);

  /* b <- base^Q (mod p) */
  b = powmod64(base,subseq_Q,p);

  /* Baby steps. */
  if ((i = baby_steps64(b,powmod64(b,sieve_low,p),p)) > 0)
  {
    /* i is the order of b (mod p). This is all the information we need to
       determine every solution for this p, no giant steps are needed.
    */
    for (k = 0; k < subseq_count; k++)
      for (j = lookup64(D64[k]); j < sieve_range; j += i)
        eliminate_term(k,sieve_low+j,p);
  }
  else
  {
    /* First giant step. */
    for (k = 0; k < subseq_count; k++)
      if ((j = lookup64(D64[k])) != HASH_NOT_FOUND)
        eliminate_term(k,sieve_low+j,p);

    /* Remaining giant steps. */
    if (M > 1)
    {
      b = powmod64(bQ64,m,p); /* b <- 1/b^m (mod p) */
      PRE2_MULMOD64_INIT(b);
      for (i = 1; i < M; i++)
        for (k = 0; k < subseq_count; k++)
        {
          D64[k] = PRE2_MULMOD64(D64[k],b,p);
          if ((j = lookup64(D64[k])) != HASH_NOT_FOUND)
            eliminate_term(k,sieve_low+i*m+j,p);
        }
      PRE2_MULMOD64_FINI();
    }
  }

  mod64_fini();
}

static uint32_t small_sieve_limit(void)
{
  uint32_t limit;

  /* Calculate the limit below which bsgs_small must be used. */
  limit = base;

  /* In order for the subsequence code to be effective, the small sieve
     should at least remove all very small factors.
  */
  limit = MAX(limit,257);

  /* If all values of k,|c| are quite small, then there is an advantage in
     setting limit >= k,|c|.
  */
  if (k_max < 65536 && abs_c_max < 65536)
  {
    limit = MAX(limit,k_max);
    limit = MAX(limit,abs_c_max);
  }

  return limit;
}

static void init_bsgs(void)
{
  uint32_t r = n_max/subseq_Q-n_min/subseq_Q+1, s = subseq_count;

  /*
    In the worst case we will do do one table insertion and one mulmod
    for m baby steps, then s table lookups and s mulmods for M giant
    steps. The average case depends on how many solutions are found
    and how early in the loop they are found, which I don't know how
    to analyse. However for the worst case we just want to minimise
    m + s*M subject to m*M >= r, which is when m = sqrt(s*r).
  */
  double f = make_filters(&mod,&map);
  M = MAX(1,sqrt((double)r/s/f/baby_step_factor));
  m = MIN(r,ceil((double)r/M));

  if (m > HASH_MAX_ELTS)
  {
#if SHORT_HASHTABLE
      debug("NOTE: Srsieve was compiled with SHORT_HASHTABLE=1. It may be");
      debug("NOTE: worthwhile compiling with SHORT_HASHTABLE=0 for this job.");
#endif
    M = ceil((double)r/HASH_MAX_ELTS);
    m = ceil((double)r/M);
  }

  sieve_low = n_min / subseq_Q;
  sieve_range = m*M;

  describe_global_filter(mod,map);
  debug("Sieve range is %"PRIu32" to %"PRIu32
        ", using %"PRIu32" baby steps, %"PRIu32" giant steps.",
        sieve_low*subseq_Q, (sieve_low+sieve_range)*subseq_Q-1, m, M);

  assert(sieve_low <= n_min/subseq_Q);
  assert(n_max/subseq_Q < sieve_low+sieve_range);

  uint32_t i;
  for (i = 0; i < subseq_count; i++)
    if (SUBSEQ[i].filter != 0)
      break;

  if (i < subseq_count) /* At least one non-null filter installed. */
  {
    if (k_max <= small_sieve_limit() && abs_c_max <= small_sieve_limit())
#if USE_32BIT_SIEVE
      setup32_flt = setup32_flt_smallkc;
    else
      setup32_flt = setup32_flt_bigkc;

    bsgs32 = bsgs32_flt;

    if (k_max < UINT32_MAX && abs_c_max < INT32_MAX)
#endif
      setup64_flt = setup64_flt_smallkc;
    else
      setup64_flt = setup64_flt_bigkc;

    bsgs64 = bsgs64_flt;
  }
  else
  {
    if (k_max <= small_sieve_limit() && abs_c_max <= small_sieve_limit())
#if USE_32BIT_SIEVE
      setup32_noflt = setup32_noflt_smallkc;
    else
      setup32_noflt = setup32_noflt_bigkc;

    bsgs32 = bsgs32_noflt;

    if (k_max < UINT32_MAX && abs_c_max < INT32_MAX)
#endif
      setup64_noflt = setup64_noflt_smallkc;
    else
      setup64_noflt = setup64_noflt_bigkc;

    bsgs64 = bsgs64_noflt;
  }

  C32 = xmalloc(subseq_count*sizeof(uint32_t));
  D64 = xmalloc(subseq_count*sizeof(uint64_t));

  ck64 = xmalloc(subseq_count*sizeof(uint64_t));
  bd64 = xmalloc(subseq_Q*sizeof(uint64_t));

  init_hashtable(m);
}

static void fini_bsgs(void)
{
  fini_hashtable();
  free(bd64);
  free(ck64);
  free(D64);
  free(C32);
}

void sieve(void)
{
  uint64_t low_p, high_p;

  init_bsgs();

  p_min = MAX(3,p_min);
  if (p_max == 0)
    p_max = p_min + DEFAULT_P_RANGE;
  if (p_max > MOD64_GREATEST_PRIME)
    error("64 bit sieve is limited to primes below %s.", MOD64_SIEVE_LIMIT);

  init_prime_sieve(p_max);
  start_srsieve();

  low_p = p_min;
  high_p = MIN(p_max,small_sieve_limit());
  if (low_p <= high_p)
  {
    debug("Beginning small sieve at p=%"PRIu64".", low_p);
    prime_sieve(low_p,high_p,bsgs_small,mod,map);
    low_p = high_p + 1;
    if (subseq_Q == 1 && low_p < p_max)
    {
      fini_bsgs();
      make_subseqs(0);
      init_bsgs();
    }
  }

#if USE_32BIT_SIEVE
  high_p = MIN(p_max,MAX_UINT32_PRIME);
  if (low_p <= high_p)
  {
    debug("Beginning 32 bit sieve at p=%"PRIu64".", low_p);
    prime_sieve(low_p,high_p,bsgs32,mod,map);
    low_p = high_p + 1;
  }
#endif

  if (low_p <= p_max)
  {
    debug("Beginning 64 bit sieve at p=%"PRIu64".", low_p);
    prime_sieve(low_p,p_max,bsgs64,mod,map);
  }

  fini_prime_sieve();
  fini_bsgs();

  finish_srsieve("--pmax was reached",p_max);
}
