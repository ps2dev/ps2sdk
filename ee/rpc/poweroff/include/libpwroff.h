/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Power-off library.
 */

#ifndef __LIBPWROFF_H__
#define __LIBPWROFF_H__

#define POWEROFF_THREAD_PRIORITY	0x70

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*poweroff_callback)(void *arg);

/** Initializes the poweroff library.
 * A service thread with a default priority of 0x70 will be created.
 */
int poweroffInit(void);
void poweroffSetCallback(poweroff_callback cb, void *arg);
void poweroffShutdown(void);
void poweroffChangeThreadPriority(int priority);

#ifdef __cplusplus
}
#endif

#endif /* __LIBPWROFF_H__ */
