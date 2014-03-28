/* events.c -- (C) Geoffrey Reynolds, May 2006.

   Interrupts, alarms, and other events that cannot be handled part way
   through a bsgs() iteration.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "srsieve.h"

/* Start with an initial event: initialisation.
 */
volatile int event_happened = 1;

/* Setting an entry in events[] has to be atomic, so we cannot use a bitmap.
 */
static uint_fast8_t events[last_event] = {1};

void notify_event(event_t event)
{
  assert(event < last_event);

  /* The order here is important, see note in process_events() 
   */
  events[event] = 1;
  event_happened = 1;
}

static int clear_event(event_t event)
{
  int ret;

  assert(event < last_event);

  ret = events[event];
  events[event] = 0;

  return ret;
}

static uint64_t prev_prime;
static uint32_t last_report_time;  /* in milliseconds */
static uint32_t next_report_due;   /* in seconds */
static uint32_t next_save_due;     /* in seconds */
static int stop_rate_exceeded;

static void init_progress_report(uint64_t current_prime)
{
  prev_prime = current_prime;
  last_report_time = millisec_clock();
  stop_rate_exceeded = 0;

  if (save_period > 0)
    next_save_due = sec_clock() + save_period;
  else
    next_save_due = UINT32_MAX;

  if (factor_stop_rate || verbose)
    next_report_due = sec_clock() + report_period;
  else
    next_report_due = UINT32_MAX;
}

static uint32_t factor_times[FACTOR_TIMES];
static uint32_t oldest_rec;

void record_factor_time(void)
{
  factor_times[oldest_rec] = millisec_clock();
  oldest_rec = (oldest_rec + 1) % FACTOR_TIMES;
}

static void progress_report(uint64_t p)
{
  uint32_t current_time = millisec_clock();
  uint32_t ms_per_factor;

  if (current_time == last_report_time)
    return;

  if (factor_times[oldest_rec] == 0) /* first cycle of factor_times */
  {
    if (oldest_rec == 0) /* No factors reported since sieve started */
      ms_per_factor = 0;
    else
      ms_per_factor = current_time / oldest_rec;
  }
  else
    ms_per_factor = (current_time-factor_times[oldest_rec]) / FACTOR_TIMES;

  print_status(p, (p - prev_prime) * 1000 / (current_time - last_report_time),
               ms_per_factor);

  if (factor_stop_rate && factor_count >= FACTORS_FOUND_BEFORE_STOPPING)
  {
    if (ms_per_factor > factor_stop_rate)
    {
      if (++stop_rate_exceeded >= TIMES_STOP_RATE_EXCEEDED_BEFORE_STOPPING)
      {
        finish_srsieve("--stop-rate was exceeded", p);
        exit(0);
      }
    }
    else stop_rate_exceeded = 0;
  }

  prev_prime = p;
  last_report_time = current_time;
}

static void handle_signal(int signum)
{
  switch (signum)
  {
    case SIGINT:
      notify_event(received_sigint);
      break;
    case SIGTERM:
      notify_event(received_sigterm);
      break;
  }
}

static void init_signals(void)
{
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
}

/* This function is called (via check_events()) from the top level sieve
   loops (prime_sieve() etc.). It can assume that it is safe to tighten any
   sieving parameters other than p_min and p_max.
*/
void process_events(uint64_t current_prime)
{
  /* event_happened was set last in notify_event(), so clear it first which
     ensures that if some signal arrives while we are in process_events()
     it might have to wait until the next sieve iteration to get processed,
     but it won't be lost.
  */
  event_happened = 0;

  if (clear_event(initialise_events))
  {
    init_signals();
    init_progress_report(current_prime);
  }

  if (clear_event(subsequence_eliminated))
    remove_eliminated_subsequences(current_prime);

  if (clear_event(sieve_parameters_changed))
    init_progress_report(current_prime);

  if (clear_event(received_sigterm))
  {
    finish_srsieve("SIGTERM was received",current_prime);
    signal(SIGTERM,SIG_DFL);
    raise(SIGTERM);
  }

  if (clear_event(received_sigint))
  {
    finish_srsieve("SIGINT was received",current_prime);
    signal(SIGINT,SIG_DFL);
    raise(SIGINT);
  }

  if (clear_event(report_due))
    progress_report(current_prime);

  if (clear_event(save_due))
    write_save_file(current_prime);
}

void check_progress(void)
{
  uint32_t current_time = sec_clock();

  if (current_time >= next_save_due)
  {
    next_save_due += save_period;
    notify_event(save_due);
  }

  if (current_time >= next_report_due)
  {
    next_report_due += report_period;
    notify_event(report_due);
  }
}
