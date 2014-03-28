/* util.c -- (C) Geoffrey Reynolds, April 2006.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "srsieve.h"

void error(const char *fmt, ...)
{
  va_list args;

  fprintf(stderr,"ERROR: ");
  va_start(args,fmt);
  vfprintf(stderr,fmt,args);
  putc('\n',stderr);
  va_end(args);

  exit(1);
}

void warning(const char *fmt, ...)
{
  va_list args;

  fprintf(stderr,"WARNING: ");
  va_start(args,fmt);
  vfprintf(stderr,fmt,args);
  putc('\n',stderr);
  va_end(args);
}

void report1(const char *fmt, ...)
{
  va_list args;

  if (verbose)
  {
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
  }
}

void report(const char *fmt, ...)
{
  va_list args;

  if (verbose)
  {
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
    putchar('\n');
  }
}

void debug(const char *fmt, ...)
{
  va_list args;

  if (verbose > 1)
  {
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
    putchar('\n');
  }
}

void logger(const char *fmt, ...)
{
  va_list args;
  FILE *log_file;
  time_t tm;
  char tm_buf[32];
  struct tm *tm_struct;

  if ((log_file = fopen(LOG_FILE_NAME,"a")) != NULL)
  {
    time(&tm);
    if ((tm_struct = localtime(&tm)) != NULL)
      if (strftime(tm_buf,sizeof(tm_buf),LOG_STRFTIME_FORMAT,tm_struct) != 0)
        fprintf(log_file,tm_buf);
    va_start(args,fmt);
    vfprintf(log_file,fmt,args);
    va_end(args);
    putc('\n',log_file);
    xfclose(log_file,LOG_FILE_NAME);
  }
  else
    warning("could not open log file `%s'", LOG_FILE_NAME);

  if (verbose)
  {
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
    putchar('\n');
  }
}

void *xmalloc(uint32_t sz)
{
  void *ret;

  if ((ret = malloc(sz)) == NULL)
    error("malloc failed.");

  return ret;
}

void *xrealloc(void *d, uint32_t sz)
{
  void *ret;

  if ((ret = realloc(d,sz)) == NULL)
    error("realloc failed.");

  return ret;
}
