/* subseq.c -- (C) Geoffrey Reynolds, June 2006.

   Base b sequences in n=qm+r where n0 < n < n1 are represented as a number
   of base b^Q subsequences in m where n0/Q < m < n1/Q.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "srsieve.h"
#include "bitmap.h"
#include "arithmetic.h"

subseq_t *SUBSEQ = NULL;
uint32_t subseq_count = 0;
uint32_t subseq_limit_base = DEFAULT_LIMIT_BASE;
uint32_t subseq_force_base = 0;
uint32_t subseq_Q = 0;

static uint32_t m_low;
static uint32_t m_high;

static uint_fast32_t subseq_test_m(uint32_t subseq, uint32_t m)
{
  assert(subseq < subseq_count);

  if (m < m_low || m > m_high)
    return 0;

  return test_bit(SUBSEQ[subseq].M,m-m_low);
}

static void subseq_clear_m(uint32_t subseq, uint32_t m)
{
  assert(subseq < subseq_count);
  assert(m >= m_low);
  assert(m <= m_high);

  clear_bit(SUBSEQ[subseq].M,m-m_low);
}

uint32_t make_new_subseqs(void)
{
  uint32_t i, t_count;

  assert(subseq_count == 0);

  subseq_Q = 1;
  m_low = n_min;
  m_high = n_max;
  subseq_count = seq_count;
  SUBSEQ = xmalloc(subseq_count*sizeof(subseq_t));

  for (i = 0, t_count = 0; i < seq_count; i++)
  {
    assert(SEQ[i].type == seq_none);

    SUBSEQ[i].seq = i;
    SUBSEQ[i].d = 0;
    SUBSEQ[i].filter = 0;
    SUBSEQ[i].mcount = n_max-n_min+1;
    SUBSEQ[i].M = make_bitmap(SUBSEQ[i].mcount);
    SUBSEQ[i].a = 1;
    SUBSEQ[i].b = 0;
    fill_bits(SUBSEQ[i].M,0,SUBSEQ[i].mcount-1);

    SEQ[i].type = seq_bitmap;
    t_count += SUBSEQ[i].mcount;
  }

  return t_count;
}

static void report_prime(uint32_t seq, uint32_t n)
{
  report("%s is prime.",kbnc_str(SEQ[seq].k,base,n,SEQ[seq].c));
}

static void report_factor(uint32_t subseq, uint32_t n, uint64_t p)
{
  uint64_t k;
  int64_t c;

  k = SEQ[SUBSEQ[subseq].seq].k;
  c = SEQ[SUBSEQ[subseq].seq].c;

  if (check_opt && !is_factor(k,c,n,p))
    error("%"PRIu64" DOES NOT DIVIDE %s.",p,kbnc_str(k,base,n,c));

  factor_count++;
  remaining_terms--;
  SUBSEQ[subseq].mcount--;
  if (SUBSEQ[subseq].mcount == 0)
    notify_event(subsequence_eliminated);

  if (p < min_factor_to_report)
    return;

  record_factor_time();
  save_factor(k,c,n,p);

  report("%"PRIu64" | %s",p,kbnc_str(k,base,n,c));
}

static void report_duplicate_factor(uint32_t seq, uint32_t n, uint64_t p)
{
  uint64_t k;
  int64_t c;

  k = SEQ[seq].k;
  c = SEQ[seq].c;

  if (check_opt && !is_factor(k,c,n,p))
    error("%"PRIu64" DOES NOT DIVIDE %s (duplicate).",p,kbnc_str(k,base,n,c));

  if (p >= min_factor_to_report && n <= n_max)
    report("%"PRIu64" | %s (duplicate)",p,kbnc_str(k,base,n,c));
}

/* Return 1 iff p == k*b^n+c. Assumes p - c < 2^64.
 */
static inline int is_equal(uint64_t k, uint32_t n, int64_t c, uint64_t p)
{
  assert(c >= 0 || p < UINT64_MAX + c);

  if (n > 64 || (c > 0 && p <= c))
    return 0;
  p -= c;

  if (p % k)
    return 0;
  p /= k;

  for ( ; p % base == 0; p /= base)
    n--;

  return (n == 0 && p == 1);
}


/* eliminate a single term n=Qm+d for this subsequence.
*/
void eliminate_term(uint32_t subseq, uint32_t m, uint64_t p)
{
  assert(subseq < subseq_count);

  uint32_t n = m*subseq_Q + SUBSEQ[subseq].d;
  uint32_t seq = SUBSEQ[subseq].seq;

  if (subseq_test_m(subseq,m))
  {
    if (is_equal(SEQ[seq].k,n,SEQ[seq].c,p)) /* k*b^n+c == p */
      report_prime(seq,n);
    else
    {
      subseq_clear_m(subseq,m);
      report_factor(subseq,n,p);
    }
  }
  else if (duplicates_opt)
    report_duplicate_factor(seq,n,p);
}

static uint32_t first_n(uint32_t subseq)
{
  return subseq_Q*(m_low+first_bit(SUBSEQ[subseq].M))+SUBSEQ[subseq].d;
}

/* different to next_bit, returns the next > n, not >=.
 */
static uint32_t next_n(uint32_t subseq, uint32_t n)
{
  uint32_t m;

  assert(n % subseq_Q == SUBSEQ[subseq].d);

  m = n / subseq_Q;

  assert(m >= m_low);
  assert(m <= m_high);

  m = next_bit(SUBSEQ[subseq].M,m-m_low+1)+m_low;

  if (m > m_high)
    return UINT32_MAX;

  return m*subseq_Q + SUBSEQ[subseq].d;
}

static uint32_t for_each(uint32_t first,uint32_t count,kcn_app_t app,void *arg)
{
  uint32_t i, j, *next, ret = 0;

  if (count == 0)
    return 0;

  if (count == 1)
  {
    for (i = first_n(first); i <= n_max; i = next_n(first,i))
    {
      app(arg,SEQ[SUBSEQ[first].seq].k,SEQ[SUBSEQ[first].seq].c,i);
      ret++;
    }
    return ret;
  }

  next = xmalloc(count*sizeof(uint32_t));

  for (i = 0, j = 0; i < count; i++)
  {
    next[i] = first_n(first+i);
    if (next[i] < next[j])
      j = i;
  }

  while (next[j] <= n_max)
  {
    app(arg,SEQ[SUBSEQ[first+j].seq].k,SEQ[SUBSEQ[first+j].seq].c,next[j]);
    ret++;
    next[j] = next_n(first+j,next[j]);
    for (i = count; i-- > 0; )
      if (next[i] <= next[j])
        j = i;
  }

  free(next);

  return ret;
}

/* For each remaining term k*b^n+c, calls app(arg,k,c,n) in order of
   increasing n.
 */
uint32_t for_each_term(kcn_app_t app, void *arg)
{
  return for_each(0,subseq_count,app,arg);
}

/* For each remaining term in seq, calls app(arg,k,c,n) in order of
   increasing n.
*/
uint32_t seq_for_each_term(uint32_t seq, kcn_app_t app, void *arg)
{
  uint32_t first, count;

  for (first = 0; first < subseq_count; first++)
    if (SUBSEQ[first].seq == seq)
      break;

  for (count = 0; first+count < subseq_count; count++)
    if (SUBSEQ[first+count].seq != seq)
      break;

  return for_each(first,count,app,arg);
}

void remove_eliminated_subsequences(uint64_t p)
{
  uint32_t i, j, deletion = 1;

  /* This is more complicated than it should be because of the need to
     compact the sequences to fill any holes left by deletions, which means
     we have to update the subsequence .seq field to refer to the new
     sequence index. Using pointers instead of indices would make this
     simpler, but we use the indices in bsgs.c to index the lookup tables of
     per-sequence values.
  */

  while (deletion)
    for (i = 0, deletion = 0; i < subseq_count; i++)
      if (SUBSEQ[i].mcount == 0)
      {
        deletion = 1;
        if ((i == 0 || SUBSEQ[i].seq != SUBSEQ[i-1].seq)
            && (i == subseq_count-1 || SUBSEQ[i].seq != SUBSEQ[i+1].seq))
        {
          SEQ[SUBSEQ[i].seq].type = seq_delete;
          for (j = i+1; j < subseq_count; j++)
            SUBSEQ[j].seq--;
          remove_eliminated_sequences(p);
        }
        if (SUBSEQ[i].M != NULL)
          free(SUBSEQ[i].M);
        for (j = i; j < subseq_count-1; j++)
          memcpy(&SUBSEQ[j],&SUBSEQ[j+1],sizeof(subseq_t));
        subseq_count--;
        break;
      }
}

/* ********************************************************************* */

#include "choose.c"

static uint32_t find_best_Q(uint32_t Q, int list, uint32_t *s, int force)
{
  uint32_t i, j, k;

  if (force)
    k = 1;
  else
    k = for_each_divisor(Q,NULL);

  R = xmalloc(Q*sizeof(uint_fast8_t));
  S = xmalloc(k*sizeof(struct choice));

  for (i = 0; i < k; i++)
    S[i].subseqs = 0;

  if (list)
    for (i = 0; i < seq_count; i++)
    {
      for (j = 0; j < Q; j++)
        R[j] = 0;
      for (j = 0; j < SEQ[i].ncount; j++)
        R[SEQ[i].N[j]%Q] = 1;
      if (force)
        count_subsequences(0,Q,Q);
      else
        for_each_divisor(Q,count_subsequences);
    }
  else /* bitmap */
    for (i = 0; i < subseq_count; i++)
    {
      for (j = 0; j < Q; j++)
        R[j] = 0;
      for (j = first_n(i); j <= n_max; j = next_n(i,j))
        R[j%Q] = 1;
      if (force)
        count_subsequences(0,Q,Q);
      else
        for_each_divisor(Q,count_subsequences);
    }

  free(R);

  for (i = 0; i < k; i++)
    S[i].work = rate_Q(S[i].div,S[i].subseqs);

  for (i = 0, j = 0; i < k; i++)
    if (S[i].work < S[j].work)
      j = i;

  i = S[j].div;
  *s = S[j].subseqs;
  free(S);
  return i;
}

uint32_t make_subseqs(int list)
{
  uint32_t i, j, r, n, Q, s, s_count, t_count, tmp_low, tmp_high;
  uint32_t *g, *nmin, *subseq;
  subseq_t *tmp;

#ifndef NDEBUG
  uint32_t old_count = 0;
#endif

  if (subseq_force_base)
    Q = find_best_Q(subseq_force_base,list,&s_count,1);
  else
    Q = find_best_Q(subseq_limit_base,list,&s_count,0);

  if (list == 0 && Q == 1) /* Nothing to do */
    return remaining_terms;

  tmp = xmalloc(s_count*sizeof(subseq_t));
  tmp_low = n_min/Q;
  tmp_high = n_max/Q;
  g = xmalloc(Q*sizeof(uint32_t));
  nmin = xmalloc(Q*sizeof(uint32_t));
  subseq = xmalloc(Q*sizeof(uint32_t));

  for (i = 0, s = 0; i < seq_count; i++)
  {
    for (j = 0; j < Q; j++)
      g[j] = 0, nmin[j] = UINT32_MAX;

    if (list)
      for (j = 0; j < SEQ[i].ncount; j++)
      {
        n = SEQ[i].N[j];
        r = n % Q;
        nmin[r] = MIN(nmin[r],n);
        g[r] = gcd32(g[r],n-nmin[r]);
      }
    else
      for (n = first_n(i); n <= n_max; n = next_n(i,n))
      {
        r = n % Q;
        nmin[r] = MIN(nmin[r],n);
        g[r] = gcd32(g[r],n-nmin[r]);
      }

    for (r = 0; r < Q; r++)
      if (nmin[r] != UINT32_MAX)
      {
        tmp[s].seq = i;
        tmp[s].d = r;
        tmp[s].filter = 0;
        tmp[s].M = make_bitmap(tmp_high-tmp_low+1);
        tmp[s].mcount = 0;
        tmp[s].a = MAX(1,g[r]);
        tmp[s].b = nmin[r]%tmp[s].a;
        subseq[r] = s++;
      }

    assert (s <= s_count);

    if (list)
      for (j = 0; j < SEQ[i].ncount; j++)
      {
        n = SEQ[i].N[j];
        r = n % Q;
        set_bit(tmp[subseq[r]].M,n/Q-tmp_low);
        tmp[subseq[r]].mcount++;
      }
    else
      for (n = first_n(i); n <= n_max; n = next_n(i,n))
      {
        r = n % Q;
        set_bit(tmp[subseq[r]].M,n/Q-tmp_low);
        tmp[subseq[r]].mcount++;
      }

    if (list)
    {
      free(SEQ[i].N);
      SEQ[i].N = NULL;
      SEQ[i].nsize = 0;
      SEQ[i].type = seq_bitmap;
#ifndef NDEBUG
      old_count += SEQ[i].ncount;
#endif
    }
    else
    {
      free(SUBSEQ[i].M);
      SUBSEQ[i].M = NULL;
#ifndef NDEBUG
      old_count += SUBSEQ[i].mcount;
#endif
    }
  }

  assert (s == s_count);

  free(subseq);
  free(nmin);
  free(g);

  if (Q > 1)
    report("Split %"PRIu32" base %"PRIu32" sequence%s into %"
           PRIu32" base %"PRIu32"^%"PRIu32" subsequence%s.", seq_count,
           base, plural(seq_count), s_count, base, Q, plural(s_count));

  subseq_Q = Q;
  m_low = tmp_low;
  m_high = tmp_high;

  if (!list)
    free(SUBSEQ);
  SUBSEQ = tmp;
  subseq_count = s_count;

  for (i = 0, t_count = 0; i < subseq_count; i++)
    t_count += SUBSEQ[i].mcount;

  assert(t_count == old_count);

  return t_count;
}

#ifndef NDEBUG
#include <stdio.h>
const char *subseq_str(uint32_t sub)
{
  static char buf[64];
  uint32_t seq = SUBSEQ[sub].seq;

  snprintf(buf,64,"%"PRIu64"*%"PRIu32"^(%"PRIu32"*m+%"PRIu32")%+"PRId64,
          SEQ[seq].k,base,SUBSEQ[sub].a,SUBSEQ[sub].b,SEQ[seq].c);

  return buf;
}
#endif
