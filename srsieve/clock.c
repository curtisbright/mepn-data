/* clock.c -- (C) Geoffrey Reynolds, May 2006.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include "srsieve.h"
#include "config.h"


/* Return the CPU time used by this process to date, in milliseconds. Using
   32 bits for millisecond times gives 49 days before wrap-around.
*/
#if HAVE_GETRUSAGE
#include <sys/resource.h>
uint32_t millisec_clock(void)
{
  int ret;
  struct rusage r;

  ret = getrusage(RUSAGE_SELF, &r);

  assert(ret == 0);

  return (r.ru_utime.tv_sec + r.ru_stime.tv_sec) * 1000
    + (r.ru_utime.tv_usec + r.ru_stime.tv_usec) / 1000;
}

uint32_t sec_clock(void)
{
  int ret;
  struct rusage r;

  ret = getrusage(RUSAGE_SELF, &r);

  assert(ret == 0);

  return (r.ru_utime.tv_sec + r.ru_stime.tv_sec);
}
#else
/* If clock_t is an unsigned 32 bit type and CLOCKS_PER_SEC is 1 million
   (typical case) then clock() will wrap around every 72 minutes. But as
   long as this function is called at least once every 72 minutes, and not
   called from a signal handler, it will work correctly. We can ensure this
   happens by setting the maximum reporting period to 1 hour in srsieve.h.
*/
#include <time.h>

static uint64_t clocks(void)
{
  static clock_t last_clock = 0;
  static uint64_t accumulated_clocks = 0;
  clock_t this_clock = clock();

  assert(this_clock != (clock_t)-1);

  accumulated_clocks += (this_clock - last_clock);
  last_clock = this_clock;

  return accumulated_clocks;
}

uint32_t millisec_clock(void)
{
  return clocks() * 1000 / CLOCKS_PER_SEC;
}

uint32_t sec_clock(void)
{
  return clocks() / CLOCKS_PER_SEC;
}
#endif /* HAVE_RUSAGE */

/* Return a pointer to a string representing a millisecond time in seconds.
*/
const char *millisec_str(uint32_t time)
{
  static char str[12];
  int prec;

  if (time < 10000)
    prec = 3;
  else if (time < 100000)
    prec = 2;
  else if (time < 1000000)
    prec = 1;
  else
    prec = 0;

  sprintf(str, "%.*f", prec, time/1000.0);

  return str;
}
