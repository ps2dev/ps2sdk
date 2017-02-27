/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Struct and function declarations for dealing with time
 */

#ifndef _TIME_H
#define _TIME_H

#include <stddef.h>
#include <sys/time.h>

#define CLOCKS_PER_SEC (147456000 / 256)    // ie kBUSCLKBY256  from <timer.h>

struct tm
{
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
  int	tm_yday;
  int	tm_isdst;
};

#ifdef __cplusplus
extern "C" {
#endif

clock_t clock(void);
time_t time(time_t *t);

// to be implemented...
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm *timeptr);
char *asctime(const struct tm *timeptr);
char *ctime(const time_t *timep);
struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);

#ifdef __cplusplus
}
#endif

#endif  // TIME_H
