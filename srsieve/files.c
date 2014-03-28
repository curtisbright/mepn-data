/* files.c -- (C) Geoffrey Reynolds, May 2006.

   Routines for reading and writing sieve and sequence files.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "srsieve.h"

/* Set n_min and n_max from the current candidate sequences.
 */
void calculate_n_range(void)
{
  uint32_t i, nmin = UINT32_MAX, nmax = 0;

  for (i = 0; i < seq_count; i++)
  {
    nmin = MIN(nmin, SEQ[i].N[0]);
    nmax = MAX(nmax, SEQ[i].N[SEQ[i].ncount-1]);
  }

  n_min = nmin;
  n_max = nmax;
}

static int line_counter = 0;
void line_error(const char *msg, const char *fmt, const char *file_name)
{
  error("Line %d: %s in %s format file `%s'.",line_counter,msg,fmt,file_name);
}

/* Reads a line from file, ignoring blank lines and comments. Returns a
   pointer to the first non-whitespace character in the line.
*/
#define MAX_LINE_LENGTH 256
#define COMMENT_CHAR '#'
const char *read_line(FILE *file)
{
  static char buf[MAX_LINE_LENGTH];
  const char *ptr;

  while ((ptr = fgets(buf,MAX_LINE_LENGTH,file)) != NULL)
  {
    line_counter++;
    while (isspace(*ptr))
      ptr++;
    if (*ptr != COMMENT_CHAR && *ptr != '\0')
      break;
  }

  return ptr;
}

FILE *xfopen(const char *fn, const char *mode, void (*fun)(const char *,...))
{
  FILE *ret;

  ret = fopen(fn,mode);
  if (ret == NULL && fun != NULL)
    fun("Failed to open %sput file `%s'.", (*mode == 'r') ? "in" : "out", fn);

  line_counter = 0;
  return ret;
}

void xfclose(FILE *file, const char *fn)
{
  if (file != NULL && fclose(file))
    warning("Problem closing file `%s'.", fn);
}

/* Check that b is a valid base and that it matches the base of previously
   read sequences, or throw an error. If base has not yet been determined
   (i.e. this is the first sequence read) then set it to b.
*/
static void check_and_set_base(uint32_t b, const char *file_name)
{
  static const char *base_file_name = NULL;

  if (b < 2)
    error("Invalid base %"PRIu32" in `%s'.", b, file_name);

  if (base == 0)
  {
    base = b;
    base_file_name = file_name;
  }
  else if (b != base)
  {
    error("Base %"PRIu32" in `%s' does not match base %"PRIu32" from `%s'.",
          b, file_name, base, base_file_name);
  }
}

static void set_p_min(uint64_t p, const char *file_name)
{
  static const char *p_min_file = NULL;

  if (file_name == checkpoint_file_name && p > p_min)
  {
    /* allow checkpoint pmin to override a smaller pmin. */
  }
  else if (p_min_file == NULL && p_min && p_min != p)
  {
    /* set from command line */
    warning("--pmin %"PRIu64" overrides pmin=%"PRIu64" from `%s'.",
            p_min,p,file_name);
    return;
  }
  else if (p_min && p >= p_min && file_name)
  {
    return;
  }

  p_min = p;
  p_min_file = file_name;
}

/* Return the index of the candidate sequence k*b^n+c, or create a new one.
   This should really be in sequences.c. Hash table search would be better.
 */
#define SEQ_GROW_SIZE 32
static uint32_t get_seq(uint64_t k, int64_t c)
{
  static uint32_t seq_alloc = 0, last_seq = UINT32_MAX;
  uint32_t i, j, first_seq;

  if (last_seq != UINT32_MAX && SEQ[last_seq].k <= k && SEQ[last_seq].c <= c)
    first_seq = last_seq;
  else
    first_seq = 0;

  for (i = first_seq; i < seq_count; i++)
    if (SEQ[i].k >= k)
    {
      if (SEQ[i].k == k && SEQ[i].c == c)
        return i;
      if (SEQ[i].k > k || SEQ[i].c > c)
        break;
    }

  if (seq_count == seq_alloc)
  {
    seq_alloc += SEQ_GROW_SIZE;
    SEQ = xrealloc(SEQ, seq_alloc*sizeof(seq_t));
  }

  for (j = seq_count; j > i; j--)
    memcpy(&SEQ[j],&SEQ[j-1],sizeof(seq_t));

  SEQ[i].k = k;
  SEQ[i].c = c;
  SEQ[i].type = seq_none;
  SEQ[i].N = NULL;
  SEQ[i].nsize = 0;
  SEQ[i].ncount = 0;
  seq_count++;

  last_seq = i;
  return i;
}

int scan_seq(const char *str, uint64_t *k, uint32_t *b, int64_t *c)
{
  switch(sscanf(str, "%"SCNu64"*%"SCNu32"^%*[^-+]%"SCNd64, k, b, c))
  {
    case 1:
      if (sscanf(str, "%"SCNu32"^%*[^-+]%"SCNd64, b, c) != 2)
        break;
      *k = 1;
      /* fall through */
    case 3:
      return 1;
  }

  return 0;
}

static char seq_buf[64];
const char *kbc_str(uint64_t k, uint32_t b, int64_t c)
{
  if (k == 1)
    sprintf(seq_buf, "%"PRIu32"^n%+"PRId64, b, c);
  else
    sprintf(seq_buf, "%"PRIu64"*%"PRIu32"^n%+"PRId64, k, b, c);

  return seq_buf;
}

const char *kbnc_str(uint64_t k, uint32_t b, uint32_t n, int64_t c)
{
  if (k == 1)
    sprintf(seq_buf, "%"PRIu32"^%"PRIu32"%+"PRId64, b, n, c);
  else
    sprintf(seq_buf, "%"PRIu64"*%"PRIu32"^%"PRIu32"%+"PRId64, k, b, n, c);

  return seq_buf;
}

const char *seq_str(uint32_t seq)
{
  assert (seq < seq_count);

  return kbc_str(SEQ[seq].k, base, SEQ[seq].c);
}

uint32_t parse_seq_str(const char *str, const char *file_name)
{
  uint64_t k;
  int64_t c;
  uint32_t b;

  if (scan_seq(str,&k,&b,&c))
  {
    check_and_set_base(b,file_name);
    return get_seq(k,c);
  }

  return UINT32_MAX;
}

#define SEQ_N_GROW_SIZE 1024
static void add_seq_n(uint32_t seq, uint32_t n)
{
  uint32_t count;

  assert(seq < seq_count);
  assert(SEQ[seq].type == seq_none || SEQ[seq].type == seq_list);

  count = SEQ[seq].ncount;
  if (count > 0 && SEQ[seq].N[count-1] >= n)
  {
    /* File is not in order of strictly increasing n. */

    uint64_t k = SEQ[seq].k;
    int64_t c = SEQ[seq].c;

    if (SEQ[seq].N[count-1] == n)
      error("Duplicate term %s in input file.", kbnc_str(k,base,n,c));
    else
      error("Smaller term %s follows larger term in input file.",
            kbnc_str(k,base,n,c));
  }

  if (count == SEQ[seq].nsize)
  {
    SEQ[seq].nsize += SEQ_N_GROW_SIZE;
    SEQ[seq].N = xrealloc(SEQ[seq].N, SEQ[seq].nsize*sizeof(uint32_t));
    SEQ[seq].type = seq_list;
  }

  SEQ[seq].N[count] = n;
  SEQ[seq].ncount++;
}

static void report_read(uint32_t terms, uint32_t seqs, const char *format,
                        const char *file_name)
{
  report("Read %"PRIu32" term%s for %"PRIu32" sequence%s from %s format file"
         " `%s'.", terms,plural(terms),seqs,plural(seqs),format,file_name);
}

static void report_wrote(uint32_t terms, uint32_t seqs, const char *format,
                        const char *file_name)
{
  report("Wrote %"PRIu32" term%s for %"PRIu32" sequence%s to %s format file"
         " `%s'.",terms,plural(terms),seqs,plural(seqs),format,file_name);
}

/* Returns 1 if successful, or 0 if file_name is not a srsieve format file
   (so that another file type can be tried). The srsieve file format is:

    pmin=<uint64_t>
    k*b^n+c
    n
    n
    ...
    k*b^n+c
    n
    n
    ...
*/
static int read_srsieve_file(const char *file_name)
{
  FILE *file;
  const char *line;
  uint64_t p_start;
  uint32_t seq, n, n_count, k_count;
  char ch;

  /* try to read header */
  file = xfopen(file_name, "r", error);
  line = read_line(file);
  if (line == NULL || sscanf(line, "pmin=%" SCNu64, &p_start) != 1)
  {
    fclose(file);
    return 0;
  }
  set_p_min(p_start,file_name);

  /* read sequences */
  k_count = n_count = seq = 0;
  while ((line = read_line(file)) != NULL)
  {
    if (sscanf(line, "%" SCNu32 " %c", &n, &ch) == 1)
    {
      if (k_count == 0)
        line_error("Missing sequence header","srsieve",file_name);
      add_seq_n(seq,n);
      n_count++;
    }
    else if ((seq = parse_seq_str(line,file_name)) < UINT32_MAX)
      k_count++;
    else
      line_error("Malformed line","srsieve",file_name);
  }

  if (ferror(file))
    line_error("Read error","srsieve",file_name);
  fclose(file);
  report_read(n_count,k_count,"srsieve",file_name);

  return 1;
}

static int read_newpgen_file(const char *file_name)
{
  FILE *file;
  const char *line;
  char mode_char;
  int mode_bits, type;
  uint32_t b, n_count, n, old_count, seq;
  uint64_t k, p_start;

  /* Try to read header */
  file = xfopen(file_name, "r", error);
  line = read_line(file);
  /* headers are of the form <p_start>:<mode_char>:<int>:<base>:<mode_bits>
   */
  if (sscanf(line, "%" SCNu64 ":%c:%*d:%" SCNu32 ":%d",
             &p_start, &mode_char, &b, &mode_bits) != 4)
  {
    fclose(file);
    return 0;
  }

  check_and_set_base(b,file_name);
  set_p_min(p_start,file_name);

  if (mode_char == 'P' && mode_bits == 257)
    type = 16;
  else if (mode_char == 'M' && mode_bits == 258)
    type = 17;
  else if (mode_char == 'P' && mode_bits == 32769)
    type = 19;
  else if (mode_char == 'M' && mode_bits == 32770)
    type = 20;
  else
    line_error("Unrecognised sieve type","NewPGen",file_name);

  old_count = seq_count;
  for (n_count = 0; (line = read_line(file)) != NULL; n_count++)
  {
    if (sscanf(line, "%" SCNu64 " %" SCNu32, &k, &n) != 2)
      line_error("Malformed line","NewPGen",file_name);
    switch (type)
    {
      case 16: seq = get_seq(k,1); break;
      case 17: seq = get_seq(k,-1); break;
      case 19: seq = get_seq(1,k); break;
      case 20: default: seq = get_seq(1,-k); break;
    }
    add_seq_n(seq,n);
  }

  if (ferror(file))
    line_error("Read error","NewPGen",file_name);
  fclose(file);
  report_read(n_count,seq_count-old_count,"NewPGen",file_name);

  return 1;
}

static int read_pfgw_file(const char *file_name)
{
  FILE *file;
  const char *line;
  uint64_t p, k;
  int64_t c;
  uint32_t b, n_count, n, old_count;
  char c1, c2, c3;

  /*
    ABC $a*<base>^$b$c (or ABC $a*<base>^$b+$c)
    k n c
    k n c
  */

  file = xfopen(file_name, "r", error);
  line = read_line(file);

  /* The "%*[+$]" allows the "+" to be ignored if present, for compatibility
     with versions prior to 0.6.13.
  */
  switch (sscanf(line,"ABC $%c*%"SCNu32"^$%c%*[+$]%c // Sieved to %"SCNu64,
                 &c1,&b,&c2,&c3,&p))
  {
    case 5:
      set_p_min(p,file_name);
      /* fall through */
    case 4:
      if (c1 != 'a' || c2 != 'b' || c3 != 'c')
        line_error("Unrecognised header","abc",file_name);
      check_and_set_base(b,file_name);
      break;
    default:
      fclose(file);
      return 0;
  }

  old_count = seq_count;
  n_count = 0;
  while ((line = read_line(file)) != NULL)
  {
    if (sscanf(line, "%"SCNu64" %"SCNu32" %"SCNd64, &k, &n, &c) != 3)
      line_error("Malformed line","abc",file_name);
    add_seq_n(get_seq(k,c), n);
    n_count++;
  }

  if (ferror(file))
    line_error("Read error","abc",file_name);
  fclose(file);
  report_read(n_count,seq_count-old_count,"abc",file_name);

  return 1;
}

/* This function cannot parse a general ABCD format file, only the specific
   type (ABCD k*b^n+c with fixed k,b,c and variable n) written by srsieve.
*/
static int read_abcd_file(const char *file_name)
{
  FILE *file;
  const char *line;
  uint64_t k, p_start;
  int64_t c;
  uint32_t seq, n, delta, b, n_count, s_count;

  file = xfopen(file_name, "r", error);
  n_count = s_count = seq = n = 0;
  while ((line = read_line(file)) != NULL)
  {
    if (sscanf(line, "%" SCNu32, &delta) == 1)
    {
      if (s_count == 0)
      {
        fclose(file);
        return 0;
      }
      n += delta;
      add_seq_n(seq,n);
      n_count++;
    }
    else
      switch (sscanf(line, "ABCD %" SCNu64 "*%" SCNu32 "^$a%" SCNd64
                     " [%" SCNu32 "] // Sieved to %" SCNu64,
                     &k, &b, &c, &n, &p_start))
      {
        case 5:
          set_p_min(p_start,file_name);
          /* fall through */

        case 4:
          check_and_set_base(b,file_name);
          seq = get_seq(k,c);
          s_count++;
          add_seq_n(seq,n);
          n_count++;
          break;

        default:
          if (s_count == 0)
          {
            fclose(file);
            return 0;
          }
          else
            line_error("Malformed line","ABCD",file_name);
      }
  }

  if (ferror(file))
    line_error("Read error","ABCD",file_name);
  fclose(file);
  report_read(n_count,s_count,"ABCD",file_name);

  return 1;
}

static int read_candidate_seq_file(const char *file_name)
{
  FILE *file;
  const char *line;
  uint32_t k_count;

  file = xfopen(file_name, "r", error);
  for (k_count = 0; (line = read_line(file)) != NULL; k_count++)
    if (parse_seq_str(line,file_name) == UINT32_MAX)
    {
      if (k_count == 0)
      {
        fclose(file);
        return 0;
      }
      else
        line_error("Malformed line","candidate sequence",file_name);
    }

  if (ferror(file))
    line_error("Read error","candidate sequence",file_name);
  fclose(file);
  report("Read %"PRIu32" sequence%s from input file `%s'.",
         k_count, plural(k_count), file_name);

  return 1;
}

int read_input_file(const char *file_name)
{
  if (read_candidate_seq_file(file_name))
    return 1;

  if (read_newpgen_file(file_name))
    return 1;

  if (read_pfgw_file(file_name))
    return 1;

  if (read_srsieve_file(file_name))
    return 1;

  if (read_abcd_file(file_name))
    return 1;

  return 0;
}

static int npg_format_index(int64_t c) attribute ((const));

static void write_n(FILE *file, uint64_t k, int64_t c, uint32_t n)
{
  fprintf(file,"%"PRIu32"\n",n);
}

static void write_kn(FILE *file, uint64_t k, int64_t c, uint32_t n)
{
  fprintf(file,"%"PRIu64" %"PRIu32"\n",k,n);
}

static void write_cn(FILE *file, uint64_t k, int64_t c, uint32_t n)
{
  fprintf(file,"%"PRId64" %"PRIu32"\n",c,n);
}

static void write_mcn(FILE *file, uint64_t k, int64_t c, uint32_t n)
{
  fprintf(file,"%"PRId64" %"PRIu32"\n",-c,n);
}

static void write_knc(FILE *file, uint64_t k, int64_t c, uint32_t n)
{
  fprintf(file,"%"PRIu64" %"PRIu32" %+"PRId64"\n",k,n,c);
}

static void write_srsieve_file(const char *file_name, uint64_t p)
{
  FILE *file;
  uint32_t seq, count;

  if ((file = xfopen(file_name, "w", warning)) == NULL)
    return;

  fprintf(file, "pmin=%"PRIu64"\n", p);
  for (count = 0, seq = 0; seq < seq_count; seq++)
  {
    fprintf(file, "%s\n", seq_str(seq));
    count += seq_for_each_term(seq,(kcn_app_t)write_n,file);
  }

  report_wrote(count, seq_count, "srsieve", file_name);
  xfclose(file,file_name);
}

typedef struct
{
  int sieve_type;
  char mode_char;
  int mode_bits;
  kcn_app_t write_fun;
} npg_format_t;

#define NUM_NPG_FORMATS 4

static const npg_format_t npg_formats[NUM_NPG_FORMATS] =
{
  {16, 'P',   257, (kcn_app_t)write_kn},  /* k*b^n+1 fixed k */
  {17, 'M',   258, (kcn_app_t)write_kn},  /* k*b^n-1 fixed k */
  {19, 'P', 32769, (kcn_app_t)write_cn},  /* b^n+k fixed k */
  {20, 'M', 32770, (kcn_app_t)write_mcn}  /* b^n-k fixed k */
};

static int npg_format_index(int64_t c)
{
  if (c == 1)
    return 0;
  else if (c == -1)
    return 1;
  else if (c > 1)
    return 2;
  else /* if (c < -1) */
    return 3;
}

static void write_newpgen_files(uint64_t p)
{
  FILE *file;
  char file_name[FILENAME_MAX+1];
  int t;
  uint32_t i, count, total;

  for (total = 0, i = 0; i < seq_count; i++)
  {
    assert(ABS(SEQ[i].c) == 1 || SEQ[i].k == 1);

    t = npg_format_index(SEQ[i].c);
    if (t == 0 || t == 1)
      snprintf(file_name, FILENAME_MAX, "t%d_b%" PRIu32 "_k%" PRIu64 ".%s",
               npg_formats[t].sieve_type, base, SEQ[i].k, NEWPGEN_EXT);
    else
      snprintf(file_name, FILENAME_MAX, "t%d_b%" PRIu32 "_k%" PRIu64 ".%s",
               npg_formats[t].sieve_type, base, ABS(SEQ[i].c), NEWPGEN_EXT);
    file_name[FILENAME_MAX] = '\0';

    if ((file = xfopen(file_name, "w", warning)) == NULL)
      continue;

    fprintf(file, "%"PRIu64":%c:1:%"PRIu32":%d\n",
            p, npg_formats[t].mode_char, base, npg_formats[t].mode_bits);
    count = seq_for_each_term(i,npg_formats[t].write_fun,file);

    if (seq_count == 1 || verbose > 1)
      report("Wrote %"PRIu32" term%s for sequence %s to NewPGen format file"
             " `%s'.", count, plural(count), seq_str(i), file_name);
    xfclose(file,file_name);
    total += count;
  }

  if (seq_count > 1 && verbose == 1) /* don't report twice */
    report("Wrote %"PRIu32" term%s for %"PRIu32" sequence%s to NewPGen format"
           " files t*_b%"PRIu32"_k*.%s.", total, plural(total),
           seq_count, plural(seq_count), base, NEWPGEN_EXT);
}

static uint32_t kn4_count[NUM_NPG_FORMATS];
static void write_kn4(FILE *files[], uint64_t k, int64_t c, uint32_t n)
{
  int t;

  t = npg_format_index(c);
  if (files[t] != NULL)
  {
    if (t == 2)
      k = c;
    else if (t == 3)
      k = -c;
    fprintf(files[t],"%"PRIu64" %"PRIu32"\n",k,n);
    kn4_count[t]++;
  }
}

/* A 'prp' file is just a series of NewPGen files all of the same type
   merged together and sorted in order of increasing n. This is convenient
   when testing with PRP or LLR, although NewPGen 2.82 itself does not read
   these files. Note that PRP version 24.14 does not correctly read type 19
   and 20 files. LLR version 3.7.0 reads all types 16, 17, 19, and 20.
*/
static void write_prp_files(uint64_t p)
{
  FILE *file[NUM_NPG_FORMATS];
  char file_name[NUM_NPG_FORMATS][FILENAME_MAX+1];
  int t;
  uint32_t i, seqs[NUM_NPG_FORMATS];

  for (t = 0; t < NUM_NPG_FORMATS; t++)
  {
    seqs[t] = 0;
    kn4_count[t] = 0;
  }

  for (i = 0; i < seq_count; i++)
  {
    assert(ABS(SEQ[i].c) == 1 || SEQ[i].k == 1);
    t = npg_format_index(SEQ[i].c);
    seqs[t]++;
  }

  for (t = 0; t < NUM_NPG_FORMATS; t++)
    if (seqs[t])
    {
      snprintf(file_name[t], FILENAME_MAX, "t%d_b%"PRIu32".%s",
               npg_formats[t].sieve_type, base, PRP_EXT);
      file_name[t][FILENAME_MAX] = '\0';
      file[t] = xfopen(file_name[t], "w", warning);
      if (file[t] != NULL)
        fprintf(file[t], "%"PRIu64":%c:1:%"PRIu32":%d\n",
                p, npg_formats[t].mode_char, base, npg_formats[t].mode_bits);
    }
    else
      file[t] = NULL;

  for_each_term((kcn_app_t)write_kn4,file);

  for (i = 0; i < NUM_NPG_FORMATS; i++)
    if (file[i] != NULL)
    {
      report_wrote(kn4_count[i], seqs[i], "PRP", file_name[i]);
      xfclose(file[i],file_name[i]);
    }
}

static void write_pfgw_file(uint64_t p)
{
  FILE *file;
  char file_name[FILENAME_MAX+1];
  uint32_t count;

  snprintf(file_name, FILENAME_MAX, "sr_%"PRIu32".%s", base, PFGW_EXT);
  file_name[FILENAME_MAX] = '\0';
  if ((file = xfopen(file_name,"w",warning)) == NULL)
    return;

  fprintf(file,"ABC $a*%"PRIu32"^$b$c // Sieved to %"PRIu64" with srsieve\n",
          base,p);
  count = for_each_term((kcn_app_t)write_knc,file);

  report_wrote(count,seq_count,"abc",file_name);
  xfclose(file,file_name);
}

static uint32_t abcd_n;
static uint64_t abcd_p;
static void write_abcd_delta(FILE *file, uint64_t k, int64_t c, uint32_t n)
{
  if (abcd_n != UINT32_MAX)
    fprintf(file,"%" PRIu32 "\n",n-abcd_n);
  else if (abcd_p == 0)
    fprintf(file," [%" PRIu32 "]\n",n);
  else
  {
    fprintf(file," [%" PRIu32 "] // Sieved to %" PRIu64 " with srsieve\n",
            n,abcd_p);
    abcd_p = 0;
  }
  abcd_n = n;
}

static void write_abcd_file(uint64_t p)
{
  FILE *file;
  uint32_t seq, count;
  char file_name[FILENAME_MAX+1];

  snprintf(file_name, FILENAME_MAX, "sr_%"PRIu32".%s", base, ABCD_EXT);
  file_name[FILENAME_MAX] = '\0';
  if ((file = xfopen(file_name, "w", warning)) == NULL)
    return;

  abcd_p = p;
  for (count = 0, seq = 0; seq < seq_count; seq++)
  {
    fprintf(file, "ABCD %" PRIu64 "*%" PRIu32 "^$a%+" PRId64,
            SEQ[seq].k, base, SEQ[seq].c);
    abcd_n = UINT32_MAX;
    count += seq_for_each_term(seq,(kcn_app_t)write_abcd_delta,file);
  }

  report_wrote(count,seq_count,"ABCD",file_name);
  xfclose(file,file_name);
}

void read_checkpoint(uint64_t pmin, uint64_t pmax)
{
  FILE *file;
  uint64_t p;

  if ((file = fopen(checkpoint_file_name,"r")) == NULL)
    return;

  if (fscanf(file,"pmin=%" SCNu64, &p) == 1)
  {
    if (p > pmin && p < pmax)
    {
      report("Resuming from checkpoint pmin=%" PRIu64 " in `%s'.",
             p, checkpoint_file_name);
      set_p_min(p,checkpoint_file_name);
    }
  }
  else
    error("Cannot read checkpoint from `%s'.", checkpoint_file_name);

  xfclose(file,checkpoint_file_name);
}

static void write_checkpoint(uint64_t p)
{
  FILE *file;

  if ((file = xfopen(checkpoint_file_name,"w",warning)) != NULL)
  {
    fprintf(file,"pmin=%" PRIu64 "\n",p);
    xfclose(file,checkpoint_file_name);
  }
}

void write_output_files(uint64_t p)
{
  if (output_file != NULL)
    write_srsieve_file(output_file,p);
  if (newpgen_opt)
    write_newpgen_files(p);
  if (prp_opt)
    write_prp_files(p);
  if (pfgw_opt)
    write_pfgw_file(p);
  if (abcd_opt)
    write_abcd_file(p);

  if (checkpoint_opt)
    remove(checkpoint_file_name);
}

void write_save_file(uint64_t p)
{
  if (checkpoint_opt)
    write_checkpoint(p);
  if (!(checkpoint_opt && factors_opt))
    write_output_files(p);
}
