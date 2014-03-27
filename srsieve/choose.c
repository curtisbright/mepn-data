/* choose.c -- (C) Geoffrey Reynolds, August 2006.

   Choose the best Q for sieving in base b^Q.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <math.h>
#include "arithmetic.h"

#define BABY_WORK    1.0    /* 1 mulmod, 1 insert */
#define GIANT_WORK   1.0    /* 1 mulmod, 1 lookup */
#define EXP_WORK     0.7    /* 1 mulmod */
#define SUBSEQ_WORK  1.4    /* 1 mulmod, 1 lookup (giant step 0), plus some
                               unknown overhead (array copy if filtering). */

static
void choose_steps(uint32_t *baby, uint32_t *giant, uint32_t Q, uint32_t s)
{
  uint32_t r = n_max/Q-n_min/Q+1;

  *giant = MAX(1,sqrt((double)r/s));
  *baby = MIN(r,ceil((double)r/(*giant)));
}

static uint32_t rate_Q(uint32_t Q, uint32_t s)
{
  uint32_t baby, giant, work;

  choose_steps(&baby,&giant,Q,s);
  work = baby*BABY_WORK + s*(giant-1)*GIANT_WORK + Q*EXP_WORK + s*SUBSEQ_WORK;

  return work;
}

/* Express n as n = p_0^m_0 * p_1^m_1 * ... * p_(k-1)^m_(k-1) for prime p_i
   and positive m_j. Store p_i in P[i], m_j in M[j], and return k. Since n
   is a uint32_t, k cannot be greater than 9.
*/
static uint32_t find_multiplicities(uint32_t n, uint32_t *P, uint32_t *M)
{
  uint32_t i, m, q;

  for (i = 0, q = 2; n > 1; q = (q + 1) | 1)
  {
    for (m = 0; n % q == 0; m++)
      n /= q;
    if (m > 0)
      P[i] = q, M[i] = m, i++;
  }

  return i;
}

/* Call fun(i,d,Q) once for each divisor d_i of Q, i=0,1,...,t. Return t.
   If fun is NULL then just count.
 */
static
uint32_t for_each_divisor(uint32_t Q, void (*fun)(uint32_t,uint32_t,uint32_t))
{
  uint32_t A[9], P[9], M[9], d, i, j, k, t;

  k = find_multiplicities(Q,P,M);

  for (i = 0, t = 1; i < k; i++)
  {
    A[i] = 0;
    t *= (M[i]+1);
  }

  if (fun != NULL)
    for (i = 0; i < t; i++)
    {
      for (j = 0; j < k; A[j++] = 0)
        if (++A[j] <= M[j])
          break;
      for (j = 0, d = 1; j < k; j++)
        d *= pow32(P[j],A[j]);

      fun(i,d,Q);
    }

  return t;
}

static uint_fast8_t *R;
static struct choice
{
  uint32_t div;
  uint32_t subseqs;
  uint32_t work;
} *S;

static uint32_t count_residue_classes(uint32_t d, uint32_t Q)
{
  uint32_t i, count;
  uint_fast8_t *R0;

  R0 = xmalloc(d*sizeof(uint_fast8_t));

  for (i = 0; i < d; i++)
    R0[i] = 0;

  for (i = 0; i < Q; i++)
    if (R[i])
      R0[i%d] = 1;

  for (i = 0, count = 0; i < d; i++)
    if (R0[i])
      count++;

  free(R0);

  return count;
}

static void count_subsequences(uint32_t i, uint32_t d, uint32_t Q)
{
  S[i].div = d;
  S[i].subseqs += count_residue_classes(d,Q);
}
