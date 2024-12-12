/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _SYS_UTIME_H
#define _SYS_UTIME_H

#include <time.h>

__BEGIN_DECLS

/*
 *  POSIX 1003.1b 5.6.6 Set File Access and Modification Times
 */

struct utimbuf 
{
	time_t actime; /* Access time */
	time_t modtime; /* Modification time */
};

extern int utime(const char *pathname, const struct utimbuf *times);

__END_DECLS

#endif /* _SYS_UTIME_H */
