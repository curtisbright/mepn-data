/* srsieve.c -- (C) Geoffrey Reynolds, April 2006.

   A sieve for integer sequences in n of the form k*b^n+c.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include "srsieve.h"
#include "version.h"

#define NAME "srsieve"
#define DESC "A sieve for integer sequences in n of the form k*b^n+c"

static void banner(void)
{
  report("%s %d.%d.%d -- %s.", NAME, MAJOR_VER, MINOR_VER, PATCH_VER, DESC);
}

static void help(void)
{
  report("Usage: %s [OPTION ...] <SEQUENCE|FILE ...>", NAME);
  report(" -p --pmin P0      "
         "Look for factors p in P0 <= p <= P1 ...");
  report(" -P --pmax P1");
  report(" -n --nmin N0      "
         "... that divide k*b^n+c with N0 <= n <= N1.");
  report(" -N --nmax N1");
  report(" -o --output FILE  "
         "Write sieve to FILE instead of %s.", DEFAULT_OUTPUT_FILE);
  report(" -r --report X     "
         "Make status reports every X seconds (default %u).",
         DEFAULT_REPORT_PERIOD);
  report(" -s --save X       "
         "Save sieve or write checkpoint every X minutes (default %u).",
         DEFAULT_SAVE_PERIOD / 60);
  report(" -S --stop-rate X  "
         "Stop when it takes X seconds to eliminate a candidate.");
  report(" -m --minimum X    "
         "Do not report factors smaller than X (default %u).",
         DEFAULT_MINIMUM_FACTOR_TO_REPORT);
  report(" -B --babystep X   "
         "Adjust the number of baby steps by a factor of X.");
  report(" -H --hashtable X  "
         "Set maximum hashtable density to X (default %.2f).",
         DEFAULT_HASH_MAX_DENSITY);
  report(" -F --force-base X "
         "Use subsequence base b^X.");
  report(" -L --limit-base X "
         "Limit subsequence base to divisors of b^X.");
  report(" -M --mod M,a,b,.. "
         "Assume all factors p satisfy p=a(mod M) or p=b(mod M) or...");
  report(" -f --factors      "
         "Append reported factors to the file %s.", factors_file_name);
  report(" -g --newpgen      "
         "Write sieve to NewPGen format files t*_b*_k*.%s.",NEWPGEN_EXT);
  report(" -G --prp          "
         "Write sieve to PRP/LLR (sorted by n) files t*_b*.%s.", PRP_EXT);
  report(" -w --pfgw         "
         "Write sieve to pfgw (sorted by n) file sr_b.%s.", PFGW_EXT);
  report(" -a --abcd         "
         "Write sieve to abcd format file sr_b.%s.", ABCD_EXT);
  report(" -c --check        "
         "Check that reported factors really do divide. (Default).");
  report("    --no-check     "
         "Do NOT check that reported factors really do divide.");
  report(" -C --checkpoint   "
         "Write (and resume from) checkpoint file `%s'.", checkpoint_file_name);
  report(" -d --duplicates   "
         "Report duplicate factors (ones that don't eliminate any n).");
  report(" -y --no-filter    "
         "Don't filter sieve primes for special sequences.");
  report(" -z --idle         "
         "Run at idle priority. (Default)");
  report(" -Z --no-idle      "
         "Don't adjust priority.");
  report(" -A --affinity N   "
         "Set affinity to CPU number N.");
  report(" -v --verbose      "
         "Be verbose.");
  report(" -q --quiet        "
         "Be quiet.");
  report(" -h --help         "
         "Print this help.");
  report(" SEQUENCE ...      "
         "Sieve SEQUENCE of the form \"k*b^n+c\", e.g. \"1396*5^n-1\""); 
  report(" FILE ...          "
         "Read sequences or resume sieving from FILE."); 

  exit(0);
}

static const char *short_opts = "p:P:n:N:o:r:s:S:m:B:H:F:L:M:fgGwacCdyzZA:vqh";
static const struct option long_opts[] =
  {
    {"pmin",       required_argument, 0, 'p'},
    {"pmax",       required_argument, 0, 'P'},
    {"nmin",       required_argument, 0, 'n'},
    {"nmax",       required_argument, 0, 'N'},
    {"output",     required_argument, 0, 'o'},
    {"report",     required_argument, 0, 'r'},
    {"save",       required_argument, 0, 's'},
    {"stop-rate",  required_argument, 0, 'S'},
    {"minimum",    required_argument, 0, 'm'},
    {"babystep",   required_argument, 0, 'B'},
    {"hashtable",  required_argument, 0, 'H'},
    {"force-base", required_argument, 0, 'F'},
    {"limit-base", required_argument, 0, 'L'},
    {"mod",        required_argument, 0, 'M'},
    {"factors",    no_argument,       0, 'f'},
    {"newpgen",    no_argument,       0, 'g'},
    {"prp",        no_argument,       0, 'G'},
    {"pfgw",       no_argument,       0, 'w'},
    {"abcd",       no_argument,       0, 'a'},
    {"check",      no_argument,       0, 'c'},
    {"no-check",   no_argument,       0, '1'},
    {"checkpoint", no_argument,       0, 'C'},
    {"duplicates", no_argument,       0, 'd'},
    {"no-filter",  no_argument,       0, 'y'},
    {"idle",       no_argument,       0, 'z'},
    {"no-idle",    no_argument,       0, 'Z'},
    {"affinity",   required_argument, 0, 'A'},
    {"verbose",    no_argument,       0, 'v'},
    {"quiet",      no_argument,       0, 'q'},
    {"help",       no_argument,       0, 'h'},
    {0, 0, 0, 0}
  };


static int opt_ind, opt_c;

static void argerror(const char *str) attribute ((noreturn));
static void argerror(const char *str)
{
  if (long_opts[opt_ind].val == opt_c)
    error("--%s %s: argument %s.", long_opts[opt_ind].name, optarg, str);
  else
    error("-%c %s: argument %s.", opt_c, optarg, str);
}

static uint64_t parse_uint(uint64_t limit)
{
  uint64_t num;
  uint32_t expt;
  char *tail;

  errno = 0;
  num = strtoull(optarg,&tail,0);

  if (errno == 0 && num <= limit)
  {
    switch (*tail)
    {
      case 'e':
      case 'E':
        expt = strtoul(tail+1,&tail,0);
        if (errno != 0)
          goto range_error;
        if (*tail != '\0')
          break;
        while (expt-- > 0)
          if (num > limit/10)
            goto range_error;
          else
            num *= 10;
      case '\0':
        return num;
    }

    argerror("is malformed");
  }

 range_error:
  argerror("is out of range");
}

int main(int argc, char **argv)
{
  int want_help = 0;
  int idle_opt = 1;

  while ((opt_c = getopt_long(argc,argv,short_opts,long_opts,&opt_ind)) != -1)
    switch (opt_c)
    {
      case 'p':
        p_min = parse_uint(UINT64_MAX);
        break;
      case 'P':
        p_max = parse_uint(UINT64_MAX);
        break;
      case 'n':
        n_min = parse_uint(UINT32_MAX);
        break;
      case 'N':
        n_max = parse_uint(UINT32_MAX-1);
        break;
      case 'o':
        output_file = optarg;
        break;
      case 'r':
        report_period = parse_uint(MAX_REPORT_PERIOD) ? : MAX_REPORT_PERIOD;
        break;
      case 's':
        save_period = parse_uint(UINT32_MAX/60) * 60;
        break;
      case 'S':
        factor_stop_rate = parse_uint(UINT32_MAX/1000) * 1000;
        break;
      case 'm':
        min_factor_to_report = parse_uint(UINT64_MAX);
        break;
      case 'B':
        baby_step_factor = strtod(optarg,NULL);
        if (baby_step_factor <= 0.0)
          argerror("must be positive");
        break;
      case 'H':
        hash_max_density = strtod(optarg,NULL);
        if (hash_max_density <= 0.0)
          argerror("must be positive");
        break;
      case 'F':
        subseq_force_base = MAX(1,parse_uint(UINT32_MAX));
        break;
      case 'L':
        subseq_limit_base = MAX(1,parse_uint(UINT32_MAX));
        break;
      case 'M':
        if (!parse_mod_form(optarg))
          argerror("invalid");
        mod_filter_opt = 1;
        break;
      case 'f':
        factors_opt = 1;
        break;
      case 'g':
        newpgen_opt = 1;
        break;
      case 'G':
        prp_opt = 1;
        break;
      case 'w':
        pfgw_opt = 1;
        break;
      case 'a':
        abcd_opt = 1;
        break;
      case 'c':
        check_opt = 1;
        break;
      case '1':
        check_opt = 0;
        break;
      case 'C':
        checkpoint_opt = 1;
        break;
      case 'd':
        duplicates_opt = 1;
        break;
      case 'y':
        no_filter_opt = 1;
        break;
      case 'z':
        idle_opt = 1;
        break;
      case 'Z':
        idle_opt = 0;
        break;
      case 'A':
        set_cpu_affinity(strtol(optarg,NULL,0));
        break;
      case 'v':
        verbose = 2;
        break;
      case 'q':
        verbose = 0;
        break;
      case 'h':
        want_help = 1;
        break;
      default:
        return 1;
    }

  if (verbose)
    banner();

  if (want_help)
    help();

  if (optind == argc)
    error("At least one sequence or input file must be specified.");

  if (idle_opt)
    set_idle_priority();

  for ( ; optind < argc; optind++)
    if (parse_seq_str(argv[optind],"command line") == UINT32_MAX
        && !read_input_file(argv[optind]))
      error("Unrecognised sequence or file `%s'.", argv[optind]);

  if (checkpoint_opt)
    read_checkpoint(p_min, (p_max) ? : UINT64_MAX);

  if (p_max && p_min > p_max)
    error("--pmin %"PRIu64" cannot be greater than --pmax %"PRIu64".",
          p_min, p_max);

  if (n_max && n_min > n_max)
    error("--nmin %"PRIu32" cannot be greater than --nmax %"PRIu32".",
          n_min, n_max);

  /* ensure that the sieve is saved in some form */
  if (output_file == NULL && newpgen_opt == 0 && prp_opt == 0 && pfgw_opt == 0
      && abcd_opt == 0 && (checkpoint_opt == 0 || factors_opt == 0))
    output_file = DEFAULT_OUTPUT_FILE;

  finish_candidate_seqs();
  sieve();

  return 0;
}

void print_status(uint64_t p, uint32_t p_per_sec, uint32_t ms_per_n)
{
  report1("p=%"PRIu64", %"PRIu32" p/sec", p, p_per_sec);

  if (ms_per_n > 0)
    report1(", %s sec/n", millisec_str(ms_per_n));

  report(", %"PRIu32" term%s eliminated, %"PRIu32" remain%s", factor_count,
         plural(factor_count), remaining_terms, (remaining_terms==1)? "s":"");
}

#if !defined(NDEBUG) && USE_ASM && (__i386__ || __x86_64__) && __GNUC__
#include <fenv.h>
static int old_mode;
#endif

void start_srsieve(void)
{
  logger("%s started: %"PRIu32" <= n <= %"PRIu32", %"PRIu64" <= p <= %"PRIu64,
         NAME, n_min, n_max, p_min, p_max);

#if !defined(NDEBUG) && USE_ASM && (__i386__ || __x86_64__) && __GNUC__
  old_mode = fegetround();
#endif
}

void finish_srsieve(const char *reason, uint64_t p)
{
#if !defined(NDEBUG) && USE_ASM && (__i386__ || __x86_64__) && __GNUC__
  int new_mode = fegetround();
  if (old_mode != new_mode)
    warning("FPU rounding mode was not restored: old=%d, new=%d.",
            old_mode, new_mode);
#endif

  report("Sieving %"PRIu64" <= p <= %"PRIu64" eliminated %"PRIu32" term%s,"
         " %"PRIu32" remain%s.",
         p_min, p, factor_count, plural(factor_count), remaining_terms,
         (remaining_terms==1)? "s":"");

  if (p >= p_max) /* Sieving job completed */
    write_output_files(p);
  else
    write_save_file(p);
  logger("%s stopped: at p=%"PRIu64" because %s.", NAME, p, reason);
  close_factors_file();
}
