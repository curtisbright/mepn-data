/* sequences.c -- (C) Geoffrey Reynolds, April 2006.

   Routines for creating and manipulating candidates sequences.

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
#include "arithmetic.h"

uint64_t k_max, abs_c_max; /* greatest values of k and |c| in k*b^n+c */

void remove_eliminated_sequences(uint64_t p)
{
  uint32_t i, j;

  for (i = 0, j = 0; i < seq_count; i++)
  {
    if (SEQ[i].type == seq_delete)
    {
      logger("removed candidate sequence %s from the sieve", seq_str(i));
      if (SEQ[i].N != NULL)
        free(SEQ[i].N);
    }
    else
    {
      if (j < i)
        memcpy(&SEQ[j],&SEQ[i],sizeof(seq_t));
      j++;
    }
  }

  seq_count = j;
  if (seq_count == 0)
  {
    finish_srsieve("all candidate sequences were eliminated",p);
    exit(0);
  }
}

void finish_candidate_seqs(void)
{
  uint64_t k, abs_c;
  uint32_t i, new_sieve = 0, old_sieve = 0;

  for (i = 0; i < seq_count; i++)
  {
    k = SEQ[i].k;
    abs_c = ABS(SEQ[i].c);

    if (SEQ[i].type == seq_none)
      new_sieve = 1;
    else
      old_sieve = 1;
    if (new_sieve && old_sieve)
      error("cannot accept new candidate sequences in to an existing sieve.");

    if (abs_c != 1 && k != 1 && (newpgen_opt || prp_opt))
      error("%s: Either k or |c| in k*b^n+c must be 1 for --newpgen or --prp.",
            seq_str(i));
    if (gcd64(base,abs_c) != 1)
      error("%s: b and c in k*b^n+c must be relatively prime.", seq_str(i));
    if (gcd64(k,abs_c) != 1)
      error("%s: k and c in k*b^n+c must be relatively prime.", seq_str(i));
    if (k < 1)
      error("%s: k in k*b^n+c must be positive.", seq_str(i));
    /*if (base % 2 == 1 && k % 2 == 1 && abs_c % 2 == 1)
      error("%s: Every term is divisible by 2.", seq_str(i));*/

    k_max = MAX(k_max,k);
    abs_c_max = MAX(abs_c_max,abs_c);
  }

  if (new_sieve)
  {
    if (n_max == 0)
      error("--nmax is a required argument when starting a new sieve.");
    remaining_terms = make_new_subseqs();
  }
  else /* if (old_sieve) */
  {
    calculate_n_range();
    remaining_terms = make_subseqs(1);
  }
}
