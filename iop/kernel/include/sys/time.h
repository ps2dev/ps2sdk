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
 * POSIX declarations for time
 */

#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#ifndef __clock_t_defined
typedef unsigned long long clock_t;
#define __clock_t_defined
#endif

#ifndef __time_t_defined
typedef unsigned long time_t;
#define __time_t_defined
#endif

struct timeval {
  long    tv_sec;         /* seconds */
  long    tv_usec;        /* and microseconds */
};

#endif /* __SYS_TIME_H__ */
