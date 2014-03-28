/* priority.c -- (C) Geoffrey Reynolds, February 2007.

   Reduce process priority to idle.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifdef __GNUC__
#define _GNU_SOURCE 1
#endif

#include "srsieve.h"
#include "config.h"


#if HAVE_SETPRIORITY
#include <sys/resource.h>
# ifndef PRIO_MAX
# define PRIO_MAX 10
# endif
#elif defined(_WIN32)
#include <windows.h>
#endif

void set_idle_priority(void)
{
#if HAVE_SETPRIORITY
  /* This seems backwards, but PRIO_MAX refers to maximum niceness. */
  setpriority(PRIO_PROCESS,0,PRIO_MAX);
#elif defined(_WIN32)
  SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
#else
  /* Do nothing. */
#endif
}


#if HAVE_SETAFFINITY
#include <sched.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

void set_cpu_affinity(int cpu_number)
{
#if HAVE_SETAFFINITY
  if (cpu_number < CPU_SETSIZE)
  {
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(cpu_number,&set);

    sched_setaffinity(0,sizeof(cpu_set_t),&set);
  }
#elif defined(_WIN32)
  if (cpu_number < 32)
  {
    // SetProcessAffinityMask(GetCurrentProcess(), 1<<cpu_number);
    SetThreadAffinityMask(GetCurrentThread(), 1<<cpu_number);
  }
#else
  /* Do nothing */
#endif
}
