/* srsieve.h -- (C) Geoffrey Reynolds, April 2006.

   A sieve for generalised Sierpinski/Riesel type problems.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _SRSIEVE_H
#define _SRSIEVE_H

#include <limits.h>
#include <stdint.h>
#include "config.h"

/* Also check config.h for compiler settings.
 */

/* Use a 32-bit arithmetic for primes below 2^32. Faster for 32-bit
   machines, maybe not for 64-bit machines.

   TODO: Check that setting this to zero doesn't break anything, e.g. code
   which assumes that p > 2^32 if the 64-bit arithmetic is being used.
*/
#define USE_32BIT_SIEVE 1

/* If --limit-base is not specified use DEFAULT_LIMIT_BASE.
 */
#define DEFAULT_LIMIT_BASE 2880

/* If --pmax is not specified use pmax = pmin + DEFAULT_P_RANGE.
 */
#define DEFAULT_P_RANGE UINT64_C(4000000000000);

/* Default for the --minimum command line switch
 */
#define DEFAULT_MINIMUM_FACTOR_TO_REPORT 100000

/* Default output file if none of --output, --newpgen, --prp are specified.
 */
#define DEFAULT_OUTPUT_FILE "srsieve.out"

/* Checkpoint file written if --checkpoint specified.
 */
#define CHECKPOINT_FILE_NAME "checkpoint.txt"

/* Extension to use for files when --newpgen is specified.
 */
#define NEWPGEN_EXT "npg"

/* Extension to use for files when --prp is specified.
 */
#define PRP_EXT "prp"

/* Extension to use for files when --pfgw is specified.
 */
#define PFGW_EXT "pfgw"

/* Extension to use for files when --abcd is specified.
 */
#define ABCD_EXT "abcd"

/* File to write log reports to.
 */
#define LOG_FILE_NAME "srsieve.log"

/* See note in clock.c before increasing this on systems lacking
   getrusage().
*/
#define MAX_REPORT_PERIOD 3600

/* Report period if --report is not specified.
 */
#define DEFAULT_REPORT_PERIOD 60

/* Save period if --save is not specified
 */
#define DEFAULT_SAVE_PERIOD 3600

/* Default if --babystep is not specified
 */
#define DEFAULT_BABY_STEP_FACTOR 1.0

/* Default value of hash_max_density if --hashtable is not specified.
   For a hash table expected to hold M elements, use a main table of at
   least M/hash_max_density elements.
*/
#define DEFAULT_HASH_MAX_DENSITY 0.65

/* Date format used for log file entries (see date --help)
 */
#define LOG_STRFTIME_FORMAT "%c "

/* Calculate the rate at which factors are being found as an average over
   the last FACTOR_TIMES factors
*/
#define FACTOR_TIMES 32

/* If --stop-rate is specified, don't actually stop until the rate is
   exceeded in this many consecutive reports.
*/
#define TIMES_STOP_RATE_EXCEEDED_BEFORE_STOPPING 3

/* If --stop-rate is specified, don't actually stop unless at least this
   many factors have been found.
*/
#define FACTORS_FOUND_BEFORE_STOPPING 3


/* Nothing below here should normally need adjustment. */


#define MAX_UINT32_PRIME UINT32_C(4294967291)

#ifdef __GNUC__
/* macros that evaluate their arguments only once. From the GCC manual.
 */
#define MIN(a,b) \
   ({ typeof (a) _a = (a); \
      typeof (b) _b = (b); \
      _a < _b ? _a : _b; })
#define MAX(a,b) \
   ({ typeof (a) _a = (a); \
      typeof (b) _b = (b); \
      _a > _b ? _a : _b; })
#define ABS(a) \
   ({ typeof (a) _a = (a); \
      _a < 0 ? -_a : _a; })
#else
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#endif


/* bsgs.c */

extern double baby_step_factor;

void sieve(void);


/* clock.c */

uint32_t millisec_clock(void);
uint32_t sec_clock(void);
const char *millisec_str(uint32_t time);


/* sequences.c */

typedef void (*kcn_app_t)(void *arg, uint64_t k, int64_t c, uint32_t n);
typedef enum { seq_delete, seq_none, seq_list, seq_bitmap } seq_type_t;
typedef struct
{
  uint64_t k;             /* k in k*b^n+c */
  int64_t c;              /* c in k*b^n+c */
  seq_type_t type;
  uint32_t *N;            /* List of remaining n */
  uint32_t nsize;         /* N has room for nsize n */
  uint32_t ncount;        /* Number of remaining n */
} seq_t;

extern uint64_t k_max, abs_c_max;

void remove_eliminated_sequences(uint64_t p);
void finish_candidate_seqs(void);


/* subseq.c */

typedef struct
{
  uint32_t seq;
  uint32_t d;
  uint32_t filter;
  uint_fast32_t *M;
  uint32_t mcount;
  uint32_t a,b;
} subseq_t;

extern subseq_t *SUBSEQ;
extern uint32_t subseq_count;
extern uint32_t subseq_limit_base;
extern uint32_t subseq_force_base;
extern uint32_t subseq_Q;

uint32_t make_new_subseqs(void);
void eliminate_term(uint32_t subseq, uint32_t m, uint64_t p);
uint32_t for_each_term(kcn_app_t app, void *arg);
uint32_t seq_for_each_term(uint32_t seq, kcn_app_t app, void *arg);
void remove_eliminated_subsequences(uint64_t p);
uint32_t make_subseqs(int list);
#ifndef NDEBUG
const char *subseq_str(uint32_t sub);
#endif

/* events.c */

typedef enum
{
  initialise_events,
  received_sigterm,
  received_sigint,
  report_due,
  save_due,

  subsequence_eliminated,
  sieve_parameters_changed,

  /* add more events here */

  last_event
} event_t;

static inline void check_events(uint64_t current_prime)
{
  extern volatile int event_happened;
  extern void process_events(uint64_t);

  if (event_happened)
    process_events(current_prime);
}
void notify_event(event_t event);
void record_factor_time(void);
void check_progress(void);


/* factors.c */

extern const char *factors_file_name;

uint32_t greatest_odd_prime_factor(uint64_t k);
int is_factor(uint64_t k, int64_t c, uint32_t n, uint64_t p);
void save_factor(uint64_t k, int64_t c, uint32_t n, uint64_t p);
void close_factors_file(void);


/* files.c */

void calculate_n_range(void);
void line_error(const char *,const char *,const char *) attribute ((noreturn));
#ifdef EOF
const char *read_line(FILE *file);
FILE *xfopen(const char *fn, const char *mode, void (*fun)(const char *,...));
void xfclose(FILE *f, const char *fn);
#endif
int read_input_file(const char *fn);
int scan_seq(const char *str, uint64_t *k, uint32_t *b, int64_t *c);
const char *kbc_str(uint64_t k, uint32_t b, int64_t c);
const char *kbnc_str(uint64_t k, uint32_t b, uint32_t n, int64_t c);
const char *seq_str(uint32_t seq);
uint32_t parse_seq_str(const char *str, const char *file_name);
void read_checkpoint(uint64_t pmin, uint64_t pmax);
void write_output_files(uint64_t p);
void write_save_file(uint64_t p);


/* filter.c */

extern int no_filter_opt;
extern int mod_filter_opt;

int parse_mod_form(const char *str);
void describe_global_filter(uint32_t mod, const uint_fast32_t *map);


/* global.c */

extern seq_t      *SEQ;
extern uint32_t    seq_count;
extern uint32_t    remaining_terms;
extern uint32_t    factor_count;

extern uint64_t    p_min;
extern uint64_t    p_max;
extern uint32_t    n_min;
extern uint32_t    n_max;
extern uint32_t    base;

extern double      hash_max_density;
extern uint64_t    min_factor_to_report;
extern const char *output_file;
extern const char *checkpoint_file_name;
extern int         factors_opt;
extern int         checkpoint_opt;
extern int         newpgen_opt;
extern int         prp_opt;
extern int         pfgw_opt;
extern int         abcd_opt;
extern int         check_opt;
extern int         duplicates_opt;
extern int         verbose;
extern uint32_t    save_period;
extern uint32_t    report_period;
extern uint32_t    factor_stop_rate;


/* primes.c */

void init_prime_sieve(uint64_t pmax);
void fini_prime_sieve(void);
void prime_sieve(uint64_t pmin, uint64_t pmax, void (*fun)(uint64_t),
                 uint32_t mod, const uint_fast32_t *map);


/* priority.c */

void set_idle_priority(void);
void set_cpu_affinity(int cpu_number);


/* srsieve.c */

void print_status(uint64_t p, uint32_t p_per_sec, uint32_t ms_per_n);
void start_srsieve(void);
void finish_srsieve(const char *reason, uint64_t p);


/* util.c */

void error(const char *fmt, ...) attribute ((noreturn,format(printf,1,2)));
void warning(const char *fmt, ...) attribute ((format(printf,1,2)));
void report(const char *fmt, ...) attribute ((format(printf,1,2)));
void report1(const char *fmt, ...) attribute ((format(printf,1,2)));
void debug(const char *fmt, ...) attribute ((format(printf,1,2)));
void logger(const char *fmt, ...) attribute ((format(printf,1,2)));
void *xmalloc(uint32_t sz) attribute ((malloc));
void *xrealloc(void *d, uint32_t sz);
static inline const char *plural(int n)
{
  return (n == 1) ? "" : "s";
}

#endif /* _SRSIEVE_H */
