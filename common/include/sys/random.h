/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2SDK_SYS_RANDOM_H__
#define __PS2SDK_SYS_RANDOM_H__

#include <unistd.h>

/* getrandom flags */
#define GRND_NONBLOCK 1
#define GRND_RANDOM 2

#ifdef __cplusplus
extern "C" {
#endif

ssize_t getrandom (void *buf, size_t buflen, unsigned int flags);

#ifdef __cplusplus
}
#endif

#endif
