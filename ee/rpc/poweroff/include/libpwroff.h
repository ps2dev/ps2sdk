/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: $
*/

#ifndef _LIBPWROFF_H
#define _LIBPWROFF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*poweroff_callback)(void *arg);

int poweroffInit();
void poweroffSetCallback(poweroff_callback cb, void *arg);
void poweroffShutdown();

#ifdef __cplusplus
}
#endif

#endif /* _LIBPWROFF_H */
