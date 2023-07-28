/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __MISC_HDSK_H__
#define __MISC_HDSK_H__

extern int HdskUnlockHdd(int unit);
extern int HdskCreateEventFlag(void);
extern int HdskCreateThread(void (*function)(void *arg), int StackSize);

#endif
