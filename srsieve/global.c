/* global.c -- (C) Geoffrey Reynolds, April 2006.

   Global variables.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <stdlib.h>
#include "srsieve.h"

seq_t      *SEQ = NULL;          /* global array of candidate sequences */
uint32_t    seq_count = 0;       /* current number of sequences */
uint32_t    remaining_terms = 0; /* number of candidate n remaining */
uint32_t    factor_count = 0;    /* candidate n eliminated so far */

uint64_t    p_min = 0;
uint64_t    p_max = 0;
uint32_t    n_min = 0;
uint32_t    n_max = 0;
uint32_t    base = 0;


double      hash_max_density = DEFAULT_HASH_MAX_DENSITY;
uint64_t    min_factor_to_report = DEFAULT_MINIMUM_FACTOR_TO_REPORT;
const char *output_file = NULL;
const char *checkpoint_file_name = CHECKPOINT_FILE_NAME;
int         factors_opt = 0;
int         checkpoint_opt = 0;
int         newpgen_opt = 0;
int         prp_opt = 0;
int         pfgw_opt = 0;
int         abcd_opt = 0;
int         check_opt = 1;
int         duplicates_opt = 0;
int         verbose = 1;
uint32_t    save_period = DEFAULT_SAVE_PERIOD;
uint32_t    report_period = DEFAULT_REPORT_PERIOD;
uint32_t    factor_stop_rate = 0;
