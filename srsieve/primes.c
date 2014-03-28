/* primes.c -- (C) 2006 Mark Rodenkirch, Geoffrey Reynolds.

   init_prime_sieve(p) :- prepare the sieve to generate all primes up to p.
   prime_sieve(p0,p1,fun) :- call fun(p) for each prime p0 <= p <= p1.

   fini_prime_sieve() :- release resources ready for another call to init().

   Original code was supplied by Mark Rodenkirch and hacked about by
   Geoffrey Reynolds for use in srsieve.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "srsieve.h"
#include "bitmap.h"

/* For the sieve to generate all primes up to p, prime_table will need to
   contain all primes up to sqrt(p).
*/
static uint32_t *prime_table, primes_in_prime_table;

/* This table is the same size as prime_table. Its values correspond to the
   smallest composite greater than the low end of the range being sieved.
*/
static uint64_t *composite_table;

/* Candidate odd numbers are sieved in blocks of size RANGE_SIZE. This
   variable should not exceed the size (in bits) of level 2 cache. Some
   machines may benefit if this is made much larger.
*/
#define RANGE_SIZE 120000

/* primes_used_in_range is the current number of primes in prime_table that
   are being used to sieve the current range. This variable is increased in
   steps of size PRIMES_USED_STEP up to a maximum of primes_in_prime_table.
*/
static uint32_t primes_used_in_range;
#define PRIMES_USED_STEP 2000

/* Call check_progress() after every PROGRESS_STEP candidates are processed.
 */
#define PROGRESS_STEP 20000


/* Return an upper bound on the number of primes <= n.
 */
static uint32_t primes_bound(uint32_t n)
{
  return 1.088375*n/log(n)+168;
}

/* The smallest acceptable argument to init_prime_sieve()
 */
#define MINIMUM_PMAX 10

void init_prime_sieve(uint64_t pmax)
{
  uint_fast32_t *sieve;
  uint32_t low_prime_limit, max_low_primes, *low_primes, low_prime_count;
  uint32_t sieve_index, max_prime, max_primes_in_table;
  uint32_t p, minp, i, composite;

  assert(primes_in_prime_table == 0);

  pmax = MAX(MINIMUM_PMAX,pmax);

  max_prime = sqrt(pmax)+1;
  low_prime_limit = sqrt(max_prime)+1;
  max_low_primes = primes_bound(low_prime_limit);
  low_primes = xmalloc(max_low_primes * sizeof(uint32_t));

  low_primes[0] = 3;
  low_prime_count = 1;
  for (p = 5; p < low_prime_limit; p += 2)
    for (minp = 0; minp <= low_prime_count; minp++)
    {
      if (low_primes[minp] * low_primes[minp] > p)
      {
        low_primes[low_prime_count] = p;
        low_prime_count++;
        break;
      }
      if (p % low_primes[minp] == 0)
        break;
    }

  assert(low_prime_count <= max_low_primes);

  /* Divide max_prime by 2 to save memory, also because already know that
     all even numbers in the sieve are composite. */
  sieve = make_bitmap(max_prime/2);
  fill_bits(sieve,1,max_prime/2-1);

  for (i = 0; i < low_prime_count; i++)
  {
    /* Get the current low prime. Start sieving at 3x that prime since 1x
       is prime and 2x is divisible by 2. sieve[1]=3, sieve[2]=5, etc. */
    composite = 3*low_primes[i];
    sieve_index = (composite-1)/2;

    while (composite < max_prime)
    {
      /* composite will always be odd, so add 2*low_primes[i] */
      clear_bit(sieve,sieve_index);
      sieve_index += low_primes[i];
      composite += 2*low_primes[i];
    }
  }

  free(low_primes);
  max_primes_in_table = primes_bound(max_prime);
  prime_table = xmalloc(max_primes_in_table * sizeof(uint32_t));

  for (i = first_bit(sieve); i < max_prime/2; i = next_bit(sieve,i+1))
  {
    /* Convert the value back to an actual prime. */
    prime_table[primes_in_prime_table] = 2*i + 1;
    primes_in_prime_table++;
  }

  assert(primes_in_prime_table <= max_primes_in_table);

  free(sieve);
  composite_table = xmalloc(primes_in_prime_table * sizeof(uint64_t));
}

void fini_prime_sieve(void)
{
  assert(primes_in_prime_table > 0);

  free(prime_table);
  free(composite_table);
  primes_in_prime_table = 0;
}

static void setup_sieve(uint64_t low_end_of_range)
{
  uint64_t max_prime, last_composite;
  uint32_t i, save_used_in_range;

  if (primes_used_in_range >= primes_in_prime_table)
    return;

  save_used_in_range = primes_used_in_range;

  if (primes_used_in_range == 0)
    primes_used_in_range = PRIMES_USED_STEP;

  while (primes_used_in_range < primes_in_prime_table)
  {
    max_prime = prime_table[primes_used_in_range - 1];
    max_prime *= max_prime;
    /* The sieve range does not include even numbers, so it contains twice
       as many candidates. */
    if (max_prime > low_end_of_range + 2*RANGE_SIZE)
      break;
    primes_used_in_range += PRIMES_USED_STEP;
  }

  if (primes_used_in_range > primes_in_prime_table)
    primes_used_in_range = primes_in_prime_table;

  /* Find the largest composite greater than low_end_of_range. */ 
  for (i = save_used_in_range; i < primes_used_in_range; i++)
  {
    max_prime = prime_table[i];
    last_composite = (low_end_of_range / max_prime) * max_prime;
    composite_table[i] = last_composite + max_prime;

    /* We only care about odd composites since the sieve range only refers
       to odd values. */
    if (!(composite_table[i] & 1))
      composite_table[i] += max_prime;
  }
}

void prime_sieve(uint64_t low_prime, uint64_t high_prime, void(*fun)(uint64_t),
                 uint32_t mod, const uint_fast32_t *map)
{
  uint64_t composite, prime, low_end_of_range, candidate;
  uint_fast32_t *sieve;
  uint32_t sieve_index, i, j, k;

  assert(primes_in_prime_table > 0);

  if (low_prime <= prime_table[primes_in_prime_table-1])
  {
    /* Skip ahead to low_prime. A binary search would be faster. */
    for (i = 0; prime_table[i] < low_prime; i++)
      ;
    while (i < primes_in_prime_table)
    {
      if (mod == 0) /* Null filter */
        for (j = MIN(i+PROGRESS_STEP/3,primes_in_prime_table); i < j; i++)
        {
          if (prime_table[i] > high_prime)
            return;
          check_events(prime_table[i]);
          fun(prime_table[i]);
        }
      else if (map == NULL) /* Global GFN filter */
        for (j = MIN(i+PROGRESS_STEP/3,primes_in_prime_table); i < j; i++)
        {
          if (prime_table[i] > high_prime)
            return;
          if (((uint_fast32_t)prime_table[i] & mod) == 0)
          {
            check_events(prime_table[i]);
            fun(prime_table[i]);
          }
        }
      else /* Global QR filter */
        for (j = MIN(i+PROGRESS_STEP/3,primes_in_prime_table); i < j; i++)
        {
          if (prime_table[i] > high_prime)
            return;
          if (test_bit(map,prime_table[i]%mod))
          {
            check_events(prime_table[i]);
            fun(prime_table[i]);
          }
        }
      check_progress();
    }
    low_end_of_range = prime_table[primes_in_prime_table-1]+1;
  }
  else /* Set low_end_of_range to the greatest even number <= low_prime */
    low_end_of_range = (low_prime | 1) - 1;

  sieve = make_bitmap(RANGE_SIZE);
  primes_used_in_range = 0;

  while (low_end_of_range <= high_prime)
  {
    setup_sieve(low_end_of_range);
    fill_bits(sieve,0,RANGE_SIZE-1);

    for (i = 0; i < primes_used_in_range; i++)
    {
      prime = prime_table[i];
      composite = composite_table[i];
      sieve_index = (composite - low_end_of_range)/2;

      while (composite < low_end_of_range + 2*RANGE_SIZE)
      {
        clear_bit(sieve,sieve_index);
        sieve_index += prime;
        composite += 2*prime;
      }
      composite_table[i] = composite;
    }

    k = MIN((high_prime-low_end_of_range+1)/2,RANGE_SIZE);
    i = first_bit(sieve);
    while (i < k)
    {
      if (mod == 0) /* Null filter */
        for (j = MIN(i+PROGRESS_STEP,k); i < j; i = next_bit(sieve,i+1))
        {
          candidate = low_end_of_range + 2*i + 1;
          check_events(candidate);
          fun(candidate);
        }
      else if (map == NULL) /* Global GFN filter */
        for (j = MIN(i+PROGRESS_STEP,k); i < j; i = next_bit(sieve,i+1))
        {
          candidate = low_end_of_range + 2*i + 1;
          if (((uint_fast32_t)candidate & mod) == 0)
          {
            check_events(candidate);
            fun(candidate);
          }
        }
      else /* Global QR filter */
        for (j = MIN(i+PROGRESS_STEP,k); i < j; i = next_bit(sieve,i+1))
        {
          candidate = low_end_of_range + 2*i + 1;
          if (test_bit(map,candidate%mod))
          {
            check_events(candidate);
            fun(candidate);
          }
        }
      check_progress();
    }
    low_end_of_range += 2*RANGE_SIZE;
  }

  free(sieve);
}
