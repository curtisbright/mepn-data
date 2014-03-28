/* srfile.c -- (C) Geoffrey Reynolds, April 2006.

   A file utility for srsieve.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "srsieve.h"
#include "version.h"
#include "arithmetic.h"

#define CONGRUENCE 1
#define WEIGHT 0

#define NAME "srfile"
#define DESC "A file utility for srsieve"

static void banner(void)
{
  report("%s %d.%d.%d -- %s.", NAME, MAJOR_VER, MINOR_VER, PATCH_VER, DESC);
}

static void help(void)
{
  report("Usage: %s [OPTION ...] <INFILE ...>", NAME);
  report(" -o --output FILE         "
         "Write sieve to FILE instead of %s.", DEFAULT_OUTPUT_FILE);
  report(" -k --known-factors FILE  "
         "Remove factors in FILE from the sieve.");
  report(" -d --delete SEQ          "
         "Delete sequence SEQ from the sieve e.g. -d \"254*5^n-1\"");
  report(" -g --newpgen             "
         "Write sieve to NewPGen format files t*_b*_k*.%s.",NEWPGEN_EXT);
  report(" -G --prp                 "
         "Write sieve to PRP (sorted by n) files t*_b*.%s.", PRP_EXT);
  report(" -w --pfgw                "
         "Write sieve to pfgw (sorted by n) file sr_b.%s.", PFGW_EXT);
  report(" -a --abcd                "
         "Write sieve to abcd format file sr_b.%s.", ABCD_EXT);
  report(" -Q --subseqs X           "
         "Print base b^Q subsequence stats for all Q dividing X.");
#if CONGRUENCE
  report(" -c --congruence X        "
         "Print congruence (mod X) information for sequences.");
#endif
#if WEIGHT
  report(" -W --weight              "
         "Print sieve weights for each sequence.");
#endif
  report(" -p --pfactor X           "
         "Print Prime95 worktodo.ini entries for P-1 factoring.");
  report("                          "
         "X is number of PRP tests saved by finding a factor.");
  report(" -v --verbose             "
         "Be verbose.");
  report(" -q --quiet               "
         "Be quiet.");
  report(" -h --help                "
         "Print this help.");
  report(" INFILE ...               "
         "Read sieve from INFILE.");

  exit(0);
}

static const char *short_opts = "o:k:d:p:Q:gGwac:Wvqh";
static const struct option long_opts[] =
  {
    {"output",         required_argument, 0, 'o'},
    {"known-factors",  required_argument, 0, 'k'},
    {"delete",         required_argument, 0, 'd'},
    {"pfactor",        required_argument, 0, 'p'},
    {"subseqs",        required_argument, 0, 'Q'},
    {"newpgen",        no_argument,       0, 'g'},
    {"prp",            no_argument,       0, 'G'},
    {"pfgw",           no_argument,       0, 'w'},
    {"abcd",           no_argument,       0, 'a'},
#if CONGRUENCE
    {"congruence",     required_argument, 0, 'c'},
#endif
#if WEIGHT
    {"weight",         no_argument,       0, 'W'},
#endif
    {"verbose",        no_argument,       0, 'v'},
    {"quiet",          no_argument,       0, 'q'},
    {"help",           no_argument,       0, 'h'},
    {0, 0, 0, 0}
  };


static int opt_ind, opt_c;

static void argerror(const char *str)
{
  if (long_opts[opt_ind].val == opt_c)
    error("--%s %s: argument %s", long_opts[opt_ind].name, optarg, str);
  else
    error("-%c %s: argument %s", opt_c, optarg, str);
}

static void remove_factors(const char *file_name);
static void delete_from_file(const char *file_name);
static void delete_sequence(uint64_t k, uint32_t b, int64_t c);
static void print_Q_choices(uint32_t Q);

static int prp_tests_saved = 0;
static void print_pfactor(uintptr_t tf, uint64_t k, int64_t c, uint32_t n)
{
  printf("Pfactor=%"PRIu64",%"PRIu32",%"PRIu32",%"PRId64",%"PRIuPTR",%d\n",
         k, base, n, c, tf, prp_tests_saved);
}

static uintptr_t bit_level(uint64_t p)
{
  uintptr_t shift;

  for (shift = 0; p > 1; shift++)
    p >>= 1;

  return shift;
}

#if CONGRUENCE
#include "arithmetic.h"

static uint32_t seq_q, seq_n, subseq_q;
static uint32_t *subseq_lengths;
static void find_cc(uint32_t seq, uint64_t k, int64_t c, uint32_t n)
{
  if (seq_n < UINT32_MAX)
    seq_q = gcd32(n-seq_n,seq_q);
  seq_n = n;
  subseq_lengths[n%subseq_q]++;
}

static void print_congruence_classes(uint32_t Q)
{
  uint32_t i, j;

  subseq_q = Q;
  subseq_lengths = xmalloc(Q*sizeof(uint32_t));
  for (i = 0; i < seq_count; i++)
  {
    seq_q = 0;
    seq_n = UINT32_MAX;
    memset(subseq_lengths,0,Q*sizeof(uint32_t));
    seq_for_each_term(i, (kcn_app_t)find_cc, NULL);
    report("%s: n = %" PRIu32 "*m+%" PRIu32 ", %" PRIu32 " terms",
           seq_str(i), seq_q, seq_n % seq_q, SEQ[i].ncount);
    for (j = 0; j < Q; j++)
      if (subseq_lengths[j])
        report("  n = %" PRIu32 " (mod %" PRIu32 "): %" PRIu32 " terms",
               j, Q, subseq_lengths[j]);
  }
  free(subseq_lengths);
}
#endif

#if WEIGHT
static void print_sieve_weights(void)
{
  /* TODO */
}
#endif

int main(int argc, char **argv)
{
  uint64_t k;
  int64_t c;
  const char *factors_file = NULL, *delete_file = NULL;
  uint32_t i, b, want_help = 0, pfactor_opt = 0, del_opt = 0, Q = 0;

#if CONGRUENCE
  uint32_t congruence_opt = 0;
#endif
#if WEIGHT
  int weight_opt = 0;
#endif

  while ((opt_c = getopt_long(argc,argv,short_opts,long_opts,&opt_ind)) != -1)
    switch (opt_c)
    {
      case 'k':
        factors_file = optarg;
        break;
      case 'd':
        if (strchr(optarg, '^'))
        {
           if (!scan_seq(optarg, &k, &b, &c))
             argerror("malformed");
        }
        else
           delete_file = optarg;
        del_opt = 1;
        break;
      case 'o':
        output_file = optarg;
        break;
      case 'p':
        pfactor_opt = 1;
        prp_tests_saved = strtol(optarg,NULL,0);
        verbose = 0;
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
      case 'Q':
        Q = strtoul(optarg,NULL,0);
        break;
#if CONGRUENCE
      case 'c':
        congruence_opt = strtoul(optarg,NULL,0);
        break;
#endif
#if WEIGHT
      case 'W':
        weight_opt = 1;
        break;
#endif
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
    error("at least one input file must be specified");

  for ( ; optind < argc; optind++)
    if (!read_input_file(argv[optind]))
      error("unrecognised file `%s'", argv[optind]);

  if (Q > 0)
  {
    calculate_n_range();
    print_Q_choices(Q);
    exit(0);
  }

  if (pfactor_opt)
    for_each_term((kcn_app_t)print_pfactor,(void *)bit_level(p_min));


#if CONGRUENCE
  if (congruence_opt)
    print_congruence_classes(congruence_opt);
#endif

#if WEIGHT
  if (weight_opt)
    print_sieve_weights();
#endif

#if CONGRUENCE
  if (congruence_opt)
    exit(0);
#endif

#if WEIGHT
  if (weight_opt)
    exit(0);
#endif

  if (pfactor_opt)
    exit(0);


  if (factors_file != NULL)
    remove_factors(factors_file);

  if (del_opt)
  {
    if (!delete_file)
      delete_sequence(k,b,c);
    else
      delete_from_file(delete_file);
  }

  for (i = 0; i < seq_count; i++)
    remaining_terms += SEQ[i].ncount;

  /* ensure that the sieve is saved to at least one of these */
  if (output_file == NULL && newpgen_opt == 0 && prp_opt == 0
      && pfgw_opt == 0 && abcd_opt == 0)
    output_file = DEFAULT_OUTPUT_FILE;

  write_output_files(p_min);

  return 0;
}

/* Returns: 0 if p is not a factor of k*b^n+c;
            1 if p is a factor of k*b^n+c;
           -1 if p is too large to check.
*/
static int check_factor(uint64_t k, int64_t c, uint32_t n, uint64_t p)
{
  int ret;

  if (p == 0)
    return 0;

  if (p == UINT64_MAX || p > MOD64_GREATEST_PRIME)
    return -1;

#if USE_ASM && (defined(__ppc64__) || defined(__powerpc64__))
  getMagic(p);
#endif

  mod64_init(p);
  ret = (addmod64(mulmod64(powmod64(umod64(base,p),n,p),umod64(k,p),p),
                  lmod64(c,p),p) == 0);
  mod64_fini();

  return ret;
}

static int read_factor(const char *s, uint64_t *k, uint32_t *b, uint32_t *n,
                    int64_t *c, uint64_t *p)
{
  if (sscanf(s,"%"SCNu64" | %"SCNu64"*%"SCNu32"^%"SCNu32"%"SCNd64,
             p, k, b, n, c) == 5)
    return 1;

  if (sscanf(s,"%"SCNu64" | %"SCNu32"^%"SCNu32"%"SCNd64, p, b, n, c) == 4)
  {
    *k = 1;
    return 1;
  }

  return 0;
}

static int read_p95(const char *s, uint64_t *k, uint32_t *b, uint32_t *n,
                    int64_t *c, uint64_t *p)
{
  /* Prime95 factors may be too large to fit in a uint64_t, but sscanf
     should indicate a successful assignment regardless.
  */
  if (sscanf(s, "%"SCNu64"*%"SCNu32"^%"SCNu32"%"SCNd64" has a factor: %"SCNu64,
             k, b, n, c, p) == 5)
    return 1;

  if (sscanf(s, "%"SCNu32"^%"SCNu32"%"SCNd64" has a factor: %"SCNu64,
             b, n, c, p) == 4)
  {
    *k = 1;
    return 1;
  }

  return 0;
}

static int read_ecm(const char *s, uint64_t *k, uint32_t *b, uint32_t *n,
                    int64_t *c, uint64_t *p)
{
  if (sscanf(s, "Probable prime cofactor (%"SCNu64"*%"SCNu32"^%"SCNu32"%"SCNd64
             ")/%"SCNu64, k, b, n, c, p) == 5)
    return 1;

  if (sscanf(s, "Composite cofactor (%"SCNu64"*%"SCNu32"^%"SCNu32"%"SCNd64
             ")/%"SCNu64, k, b, n, c, p) == 5)
    return 1;

  if (sscanf(s, "Probable prime cofactor (%"SCNu32"^%"SCNu32"%"SCNd64
             ")/%"SCNu64, b, n, c, p) == 4)
  {
    *k = 1;
    return 1;
  }

  if (sscanf(s, "Composite cofactor (%"SCNu32"^%"SCNu32"%"SCNd64
             ")/%"SCNu64, b, n, c, p) == 4)
  {
    *k = 1;
    return 1;
  }

  return 0;
}

static void remove_factors(const char *file_name)
{
  FILE *file;
  const char *line;
  uint64_t k, p;
  int64_t c;
  uint32_t b, n, i, j, seq, accept_count;
  int check;

  file = xfopen(file_name, "r", error);
  accept_count = 0;
  while ((line = read_line(file)) != NULL)
  {
    if (read_factor(line,&k,&b,&n,&c,&p) == 0
        && read_p95(line,&k,&b,&n,&c,&p) == 0
        && read_ecm(line,&k,&b,&n,&c,&p) == 0)
      continue;

    if (b != base)
      continue;

    check = check_factor(k,c,n,p);
    if (check == 0)
      error("factor %"PRIu64" from file `%s' DOES NOT DIVIDE %s",
            p, file_name, kbnc_str(k,b,n,c));

    for (seq = 0; seq < seq_count; seq++)
      if (SEQ[seq].k == k && SEQ[seq].c == c)
        break;

    if (seq == seq_count || SEQ[seq].ncount == 0)
      continue;

    if (n >= SEQ[seq].N[0] && n <= SEQ[seq].N[SEQ[seq].ncount-1])
    {
      for (i = 0; i < SEQ[seq].ncount && SEQ[seq].N[i] < n; i++)
        ;

      if (i < SEQ[seq].ncount && SEQ[seq].N[i] == n)
      {
        if (check == -1)
          debug("Accepting unchecked factor for term %s.", kbnc_str(k,b,n,c));
        else
          debug("Accepting factor %"PRIu64" for term %s.",
                p,kbnc_str(k,b,n,c));
        accept_count++;
        for (j = i; j < SEQ[seq].ncount - 1; j++)
          SEQ[seq].N[j] = SEQ[seq].N[j+1];
        SEQ[seq].ncount--;
      }
    }
  }

  if (ferror(file))
    line_error("read error","factors",file_name);
  report("Removed %"PRIu32" term%s with new factors in `%s'.",
         accept_count, plural(accept_count), file_name);
  fclose(file);
}

static void delete_from_file(const char *file_name)
{
  FILE *file;
  const char *line;
  uint64_t k;
  int64_t c;
  uint32_t b, delete_count, not_found;

  file = xfopen(file_name, "r", error);
  delete_count = not_found = 0;
  while ((line = read_line(file)) != NULL)
  {
     if (scan_seq(line, &k, &b, &c))
     {
        fflush(stdout);

        uint32_t i, j;

        for (i = 0; i < seq_count; i++)
          if (SEQ[i].k == k && SEQ[i].c == c)
            break;

        if (i == seq_count || b != base)
        {
           report("Sequence %s was not found", kbc_str(k,b,c));
           not_found++;
        }
        else
        {
           report("Deleted %" PRIu32 " term%s for sequence %s.",
                  SEQ[i].ncount, plural(SEQ[i].ncount), kbc_str(k,b,c));

           if (SEQ[i].N != NULL)
             free(SEQ[i].N);

           seq_count--;
           for (j = i; j < seq_count; j++)
             memcpy(&SEQ[j],&SEQ[j+1],sizeof(seq_t));

          delete_count++;
        }
     }
  }

  if (ferror(file))
    line_error("read error","sequences",file_name);
  report("Removed %"PRIu32" sequence%s with from `%s'.",
         delete_count, plural(delete_count), file_name);
  fclose(file);
}

static void delete_sequence(uint64_t k, uint32_t b, int64_t c)
{
  uint32_t i, j;

  for (i = 0; i < seq_count; i++)
    if (SEQ[i].k == k && SEQ[i].c == c)
      break;

  if (i == seq_count || b != base)
    error("unknown sequence %s", kbc_str(k,b,c));

  report("Deleted all %" PRIu32 " term%s for sequence %s.",
         SEQ[i].ncount, plural(SEQ[i].ncount), kbc_str(k,b,c));

  if (SEQ[i].N != NULL)
    free(SEQ[i].N);

  seq_count--;
  for (j = i; j < seq_count; j++)
    memcpy(&SEQ[j],&SEQ[j+1],sizeof(seq_t));
}

#include "choose.c"

static int compare_choice(const void *X, const void *Y)
{
  uint32_t s1, s2;

  s1 = ((const struct choice *)X)->work;
  s2 = ((const struct choice *)Y)->work;

  return (s1 < s2) ? -1 : s1 != s2;
}

static void print_Q_choices(uint32_t Q)
{
  uint32_t i, j, k;

  k = for_each_divisor(Q,NULL);

  R = xmalloc(Q*sizeof(uint_fast8_t));
  S = xmalloc(k*sizeof(struct choice));

  for (i = 0; i < k; i++)
    S[i].subseqs = 0;

  for (i = 0; i < seq_count; i++)
  {
    for (j = 0; j < Q; j++)
      R[j] = 0;
    for (j = 0; j < SEQ[i].ncount; j++)
      R[SEQ[i].N[j]%Q] = 1;
    for_each_divisor(Q,count_subsequences);
  }

  free(R);

  /* Delete any divisors which are clearly not optimal. */
  for (i = 0; i < k; i++)
    S[i].work = rate_Q(S[i].div,S[i].subseqs);
  for (i = 0; i < k; i++)
    for (j = 0; j < k; j++)
      if (i != j && S[i].div < S[j].div &&
          ((double)S[i].subseqs/S[i].div) <= ((double)S[j].subseqs/S[j].div))
        S[j].work = UINT32_MAX;

  qsort(S,k,sizeof(struct choice),compare_choice);

  for (i = 0; i < k && S[i].work < UINT32_MAX; i++)
    printf("Base %"PRIu32"^%"PRIu32": %"PRIu32" subsequences (%.1f%%),"
           " work=%"PRIu32".\n", base, S[i].div, S[i].subseqs,
           100.0*S[i].subseqs/(S[i].div*seq_count), S[i].work);

  free(S);
}

/* This replaces the function of the same name defined in sequences.c.
 */
uint32_t for_each_term(kcn_app_t app, void *arg)
{
  uint32_t i, seq, next[seq_count], ret = 0;

  if (seq_count == 0)
    return 0;

  for (i = 0, seq = 0; i < seq_count; i++)
  {
    assert(SEQ[i].type == seq_list);
    next[i] = 0;
    if (SEQ[i].N[0] < SEQ[seq].N[0])
      seq = i;
  }

  while (next[seq] < UINT32_MAX)
  {
    app(arg, SEQ[seq].k, SEQ[seq].c, SEQ[seq].N[next[seq]]);
    ret++;
    if (++next[seq] >= SEQ[seq].ncount)
      next[seq] = UINT32_MAX;
    for (i = seq_count; i-- > 0; )
      if (next[i] < UINT32_MAX)
        if (next[seq]==UINT32_MAX || SEQ[i].N[next[i]]<=SEQ[seq].N[next[seq]])
          seq = i;
  }

  return ret;
}

/* This replaces the function of the same name defined in sequences.c.
 */
uint32_t seq_for_each_term(uint32_t seq, kcn_app_t app, void *arg)
{
  uint32_t i, ncount = SEQ[seq].ncount;
  const uint32_t *N = SEQ[seq].N;
  uint64_t k = SEQ[seq].k;
  int64_t c = SEQ[seq].c;

  for (i = 0; i < ncount; i++)
    app(arg,k,c,N[i]);

  return i;
}
