/* filter.c -- (C) Geoffrey Reynolds, September 2006.

   Global filters: Filter sieve primes before calling to bsgs().
   Per-sequence filters: Select a subset of sequences for bsgs.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <math.h>
#include "arithmetic.h"
#include "bitmap.h"
#include "srsieve.h"

/* Set this flag to disable the filtering code. This is for debugging, if
   there are no bugs then it is not needed.
 */
int no_filter_opt = 0;

/* Set this flag to indicate that a command-line global filter has been
   installed.
*/
int mod_filter_opt = 0;

uint32_t mod;
const uint_fast32_t *map;

typedef struct
{
  int type;       /* 0=even,neg 1=odd,neg 2=even,pos 3=odd,pos */
  int k_sym;      /* (k/p) = symbol[k_sym] */
  int c_sym;      /* (|c|/p) = symbol[c_sym] */
} qr_filter_t;

static qr_filter_t *qr = NULL;
static int qr_len = 0, qr_alloc = 0;

static uint32_t *core32_list = NULL;
static int core32_len = 0, core32_alloc = 0;
static uint64_t *core64_list = NULL;
static int core64_len = 0, core64_alloc = 0;

static int32_t *symbol;
static uint_fast8_t *filter_p;

#define CORE_GROW 32;

static int get_core32(uint32_t core)
{
  int i;

  for (i = 0; i < core32_len; i++)
    if (core32_list[i] == core)
      break;
  if (i == core32_len)
  {
    if (core32_alloc <= core32_len)
    {
      core32_alloc += CORE_GROW;
      core32_list = xrealloc(core32_list,core32_alloc*sizeof(uint32_t));
    }
    core32_list[i] = core;
    core32_len++;
  }

  return i;
}

static int get_core64(uint64_t core)
{
  int i;

  for (i = 0; i < core64_len; i++)
    if (core64_list[i] == core)
      break;
  if (i == core64_len)
  {
    if (core64_alloc <= core64_len)
    {
      core64_alloc += CORE_GROW;
      core64_list = xrealloc(core64_list,core64_alloc*sizeof(uint64_t));
    }
    core64_list[i] = core;
    core64_len++;
  }

  return i;
}

static uint32_t make_qr_filter(int type, uint64_t k_core, uint64_t c_core)
{
  uint32_t i;
  int k_sym, c_sym;

  if (k_core <= UINT32_MAX)
    k_sym = get_core32(k_core);
  else
    k_sym = -get_core64(k_core)-1;

  if (c_core <= UINT32_MAX)
    c_sym = get_core32(c_core);
  else
    c_sym = -get_core64(c_core)-1;

  for (i = 0; i < qr_len; i++)
    if (qr[i].type == type && qr[i].k_sym == k_sym && qr[i].c_sym == c_sym)
      break;
  if (i == qr_len)
  {
    if (qr_alloc <= qr_len)
    {
      qr_alloc += CORE_GROW;
      qr = xrealloc(qr,qr_alloc*sizeof(qr_filter_t));
    }
    qr[i].type = type;
    qr[i].k_sym = k_sym;
    qr[i].c_sym = c_sym;
    qr_len++;
  }

  return i;
}

static void finish_qr_filters(void)
{
  uint32_t i;

  for (i = 0; i < qr_len; i++)
  {
    if (qr[i].k_sym < 0)
      qr[i].k_sym = core32_len-qr[i].k_sym-1;
    if (qr[i].c_sym < 0)
      qr[i].c_sym = core32_len-qr[i].c_sym-1;
  }

  filter_p = xmalloc((6+qr_len)*sizeof(uint_fast8_t));
  filter_p[0] = 1; /* Null filter. */
  if ((core32_len+core64_len) > 0)
    symbol = xmalloc((core32_len+core64_len)*sizeof(int32_t));
}

/* Return the greatest value y < 6 such that x = A^(2^y).
 */
static uint32_t squares(uint64_t x)
{
  uint64_t r;
  uint32_t y;

  for (y = 0; y < 5; y++)
  {
    r = sqrtl(x);
    if (r*r != x)
      break;
    x = r;
  }

  return y;
}

/* Return the greatest value y < 6 such that x = M*(2^y).
 */
static uint32_t twos(uint32_t x)
{
  uint32_t y;

  for (y = 0; y < 5; y++)
  {
    if (x % 2)
      break;
    x /= 2;
  }

  return y;
}

#define RES_MAX 7
static const uint_fast32_t m1 = 1<<1;
static const uint_fast32_t p2 = 1<<1 | 1<<7;
static const uint_fast32_t m2 = 1<<1 | 1<<3;
static const uint_fast32_t p3 = 1<<1 | 1<<11;
static const uint_fast32_t m3 = 1<<1;
static const uint_fast32_t p5 = 1<<1 | 1<<9;
static const uint_fast32_t m5 = 1<<1 | 1<<3 | 1<<7  | 1<<9;
static const uint_fast32_t p6 = 1<<1 | 1<<5 | 1<<19 | 1<<23;
static const uint_fast32_t m6 = 1<<1 | 1<<5 | 1<<7  | 1<<9;
static const uint_fast32_t p7 = 1<<1 | 1<<3 | 1<<9  | 1<<19 | 1<<25 | 1<<27;
static const uint_fast32_t m7 = 1<<1 | 1<<9 | 1<<11;

/* Set (mod,map) so that for all odd primes p, test_bit(map,p%mod) is true
   if and only if res is a quadratic residue with respect to p.
*/ 
static uint32_t make_residue_map(int32_t res, const uint_fast32_t **map)
{
  switch (res)
  {
    case -1:
      *map = &m1;
      return 4;
    case  2:
      *map = &p2;
      return 8;
    case -2:
      *map = &m2;
      return 8;
    case  3:
      *map = &p3;
      return 12;
    case -3:
      *map = &m3;
      return 6;
    case  5:
      *map = &p5;
      return 10;
    case -5:
      *map = &m5;
      return 20;
    case  6:
      *map = &p6;
      return 24;
    case -6:
      *map = &m6;
      return 24;
    case  7:
      *map = &p7;
      return 28;
    case -7:
      *map = &m7;
      return 14;

    default:
      *map = NULL;
      return 0;
  }
}

/* Find the greatest value y < 6 such that every term of subseq is
   of the form A^(2^y)+B^(2^y).
*/
static uint32_t gen_fermat_y(uint32_t subseq)
{
  uint32_t y;

  /* k*b^n+c must satisfy: */

  /* 1.  n = M*2^y.  */
  y = squares(base)+MIN(twos(SUBSEQ[subseq].a),twos(SUBSEQ[subseq].b));

  /* 2.  c = B^(2^y) */
  y = (SEQ[SUBSEQ[subseq].seq].c < 0) ?
      0 : MIN(y,squares(SEQ[SUBSEQ[subseq].seq].c));

  /* 3.  k = A^(2^y) */
  y = MIN(y,squares(SEQ[SUBSEQ[subseq].seq].k));

  return y;
}

/* Return the proportion of primes expected to pass the filters.
*/
static double make_filters(uint32_t *mod, const uint_fast32_t **map)
{
  uint64_t *k_core, *c_core;
  int i, type;

  if (subseq_Q == 1 || no_filter_opt || mod_filter_opt)
  {
    for (i = 0; i < subseq_count; i++)
      SUBSEQ[i].filter = 0;
    finish_qr_filters();
    if (!mod_filter_opt)
      *mod = 0;
    return 1.0;
  }

  k_core = xmalloc(seq_count*sizeof(uint64_t));
  c_core = xmalloc(seq_count*sizeof(uint64_t));

  for (i = 0; i < seq_count; i++)
  {
    k_core[i] = core64(SEQ[i].k);
#ifndef NDEBUG
    report("core64(%llu)=%llu",SEQ[i].k,k_core[i]);
#endif
    if (SEQ[i].c < 0)
      c_core[i] = core64(-SEQ[i].c);
    else
      c_core[i] = core64(SEQ[i].c);
  }

  for (i = 0; i < subseq_count; i++)
  {
    if (squares(base) > 0)
      type = 0;
    else if (SUBSEQ[i].a%2 == 0) /* subsequence has odd XOR even n */
      type = SUBSEQ[i].b%2;
    else
      type = -1;

    if (type == -1)
    {
      /* subsequence has odd AND even n. This can only happen when
         Q is odd. */
      SUBSEQ[i].filter = 0; /* Null filter */
    }
    else if (type == 0 && k_core[SUBSEQ[i].seq]==1 && c_core[SUBSEQ[i].seq]==1)
    {
      if (SEQ[SUBSEQ[i].seq].c < 0)
        warning("%s has algebraic factors.",seq_str(SUBSEQ[i].seq));
      SUBSEQ[i].filter = gen_fermat_y(i); /* GFN or Null filter. */
    }
    else
    {
      if (SEQ[SUBSEQ[i].seq].c > 0)
        type += 2;
      SUBSEQ[i].filter = /* QR filter */
        6+make_qr_filter(type,k_core[SUBSEQ[i].seq],c_core[SUBSEQ[i].seq]);
    }
  }

  finish_qr_filters();

  uint64_t k_core0 = k_core[0];
  uint64_t c_core0 = c_core[0];
  uint32_t fmax, fmin;
  double ret;

  free(k_core);
  free(c_core);

  for (i = 1, fmin = fmax = SUBSEQ[0].filter; i < subseq_count; i++)
  {
    fmin = MIN(fmin,SUBSEQ[i].filter);
    fmax = MAX(fmax,SUBSEQ[i].filter);
  }
  if (fmax < 6) /* All subsequences have GFN or Null filters. */
  {
    *map = NULL;
    *mod = (2 << fmin)-2; /* Best global GFN filter. */
    for (ret = 0.0, i = 0; i < subseq_count; i++)
    {
      ret += 1.0/(1<<(SUBSEQ[i].filter-fmin));
      if (SUBSEQ[i].filter == fmin)
        SUBSEQ[i].filter = 0;
    }
    return ret/subseq_count;
  }

  if (fmin == fmax && k_core0 <= RES_MAX && c_core0 <= RES_MAX
      && (squares(base) > 0 || base <= RES_MAX))
  {
    uint32_t k = k_core0;
    uint32_t c = c_core0;
    uint32_t b = core64(base);

    /* Unique QR filter. */
    *mod = 0;
    switch(qr[fmin-6].type)
    {
      case 0: /* even n, negative c. */
        if (c == 1)
          *mod = make_residue_map(k,map);
        else if (k == 1)
          *mod = make_residue_map(-c,map);
        break;
      case 1: /* odd n, negative c. */
        if (c == 1)
          *mod = make_residue_map(k*b,map);
        else if (k*b == 1)
          *mod = make_residue_map(-c,map);
        break;
      case 2: /* even n, positive c. */
        if (c == 1)
          *mod = make_residue_map(-k,map);
        else if (k == 1)
          *mod = make_residue_map(c,map);
        break;
      case 3: /* odd n, positive c. */
      default:
        if (c == 1)
          *mod = make_residue_map(-k*b,map);
        else if (k*b == 1)
          *mod = make_residue_map(c,map);
        break;
    }

    /* TODO: Install a general QR filter if no fixed filter is available. */

    if (*mod > 0)
      for (i = 0; i < subseq_count; i++)
        SUBSEQ[i].filter = 0;
  }

  if (*mod > 0)
    return 1.0;

  for (ret = 0.0, i = 0; i < subseq_count; i++)
    if (SUBSEQ[i].filter < 6)
      ret += 1.0/(1<<(SUBSEQ[i].filter));
    else
      ret += 0.5;

  return ret/subseq_count;
}

static inline void do_fermat_filters(uint_fast32_t p)
{
  filter_p[1] = !(p & 2); /* p=4k+1 */
  filter_p[2] = !(p & 6); /* p=8k+1 */
  filter_p[3] = !(p & 14); /* p=16k+1 */
  filter_p[4] = !(p & 30); /* p=32k+1 */
  filter_p[5] = !(p & 62); /* p=64k+1 */
}

static void do_qr_filters(int base_sym, int neg_sym)
{
  int i;

  if (base_sym) /* (b/p) == 1 */
  {
    if (neg_sym) /* so (-1/p) == 1 */
    {
      for (i = 0; i < qr_len; i++) /* (-cbk/p) == (|c|k/p) */
        filter_p[i+6] = (symbol[qr[i].k_sym] == symbol[qr[i].c_sym]);
    }
    else /* p % 4 == 3, so (-1/p) == -1 */
    {
      for (i = 0; i < qr_len; i++) /* (-cbk/p) == -(ck/p) */
      {
        if (qr[i].type >= 2) /* c > 0, so -(ck/p) == -(|c|k/p) */
          filter_p[i+6] = (symbol[qr[i].k_sym] != symbol[qr[i].c_sym]);
        else /* c < 0, so -(ck/p) == -(-|c|k/p) == (|c|k/p) */
          filter_p[i+6] = (symbol[qr[i].k_sym] == symbol[qr[i].c_sym]);
      }
    }
  }
  else /* (b/p) == -1 */
  {
    if (neg_sym) /* so (-1/p) == 1 */
    {
      for (i = 0; i < qr_len; i++) /* (-cbk/p) == (|c|bk/p) */
      {
        if (qr[i].type % 2 == 0) /* n even, so (|c|bk/p) == (|c|k/p) */
          filter_p[i+6] = (symbol[qr[i].k_sym] == symbol[qr[i].c_sym]);
        else /* n odd, so (|c|bk/p) == -(|c|k/p) */
          filter_p[i+6] = (symbol[qr[i].k_sym] != symbol[qr[i].c_sym]);
      }
    }
    else /* p % 4 == 3, so (-1/p) == -1 */
    {
      for (i = 0; i < qr_len; i++)
      {
        if (qr[i].type % 2 == 0) /* n even */
        {
          if (qr[i].type >= 2) /* c > 0, so (-ck/p) == -(|c|k/p) */
            filter_p[i+6] = (symbol[qr[i].k_sym] != symbol[qr[i].c_sym]);
          else /* c < 0, so (-ck/p) == -(-|c|k/p) == (|c|k/p) */
            filter_p[i+6] = (symbol[qr[i].k_sym] == symbol[qr[i].c_sym]);
        }
        else /* n odd */
        {
          if (qr[i].type >= 2) /* c > 0, so (-cbk/p) == (|c|k/p) */
            filter_p[i+6] = (symbol[qr[i].k_sym] == symbol[qr[i].c_sym]);
          else /* c < 0, so (-cbk/p) == -(-|c|bk/p) == -(|c|k/p) */
            filter_p[i+6] = (symbol[qr[i].k_sym] != symbol[qr[i].c_sym]);
        }
      }
    }
  }
}

#if USE_32BIT_SIEVE
static void do_filters32(uint32_t p)
{
  /* Do GFN filters. */
  do_fermat_filters(p);

  /* Do QR filters. */
  if (qr_len > 0)
  {
    int i;

    /* Build table of Legendre symbols. */
    for (i = 0; i < core32_len; i++)
      symbol[i] = legendre32(core32_list[i],p);
    for (i = 0; i < core64_len; i++)
      symbol[core32_len+i] = legendre64(core64_list[i],p);

    do_qr_filters(legendre32(base,p)==1,!(p&2));
  }
}
#endif

static void do_filters64(uint64_t p)
{
  /* Do GFN filters. */
  do_fermat_filters(p);

  /* Do QR filters. */
  if (qr_len > 0)
  {
    int i;

    /* Build table of Legendre symbols. */
    for (i = 0; i < core32_len; i++)
      symbol[i] = legendre32_64(core32_list[i],p);
    for (i = 0; i < core64_len; i++)
      symbol[core32_len+i] = legendre64(core64_list[i],p);

    do_qr_filters(legendre32_64(base,p)==1,!(p&2));
  }
}

/* Set up a filter for the command line argument --mod M,a,b,...
 */
int parse_mod_form(const char *str)
{
  uint_fast32_t *bitmap;
  uint32_t i;
  int32_t m, n;
  char *tail;

  m = strtol(str,&tail,0);
  if (m < 2 || m == INT32_MAX)
    return 0;

  bitmap = make_bitmap(m);
  for (i = 0; *tail == ','; i++)
  {
    n = strtol(tail+1,&tail,0);
    if (n <= -m || n >= m)
      return 0;
    if (n < 0)
      n += m;
    set_bit(bitmap,n);
  }

  if (i == 0 || *tail != '\0')
    return 0;

  mod = m;
  map = bitmap;

  return 1;
}

#include <stdio.h>
void describe_global_filter(uint32_t mod, const uint_fast32_t *map)
{
  if (verbose > 1 && mod > 0)
  {
    printf("Filtering for primes of the form p=");
    if (map == NULL)
    {
      printf("1 (mod %"PRIu32").\n",mod+2);
    }
    else
    {
      uint32_t i = first_bit(map);
      assert(i < mod);
      printf("%"PRIu32,i);
      for (i = next_bit(map,i+1); i < mod; i = next_bit(map,i+1))
        printf(",%"PRIu32,i);
      printf(" (mod %"PRIu32").\n",mod);
    }
  }
}
