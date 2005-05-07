/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Struct and function declarations for dealing with time
*/

#ifndef _TIME_H
#define _TIME_H

#ifndef __clock_t_defined
typedef unsigned long clock_t;
#define __clock_t_defined
#endif

#ifndef __time_t_defined
typedef unsigned long time_t;
#define __time_t_defined
#endif

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

clock_t clock();
time_t time(time_t *t);

#endif  // TIME_H
