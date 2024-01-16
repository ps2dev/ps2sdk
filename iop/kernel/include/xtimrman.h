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
 * Additional timrman functions only found in newer IOPRP images
 */

#ifndef __XTIMRMAN_H__
#define __XTIMRMAN_H__

#include <timrman.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TC_SYSCLOCK 1
#define TC_PIXEL    2
#define TC_HLINE    4

int SetTimerHandler(int timid, unsigned long comparevalue, unsigned int (*timeuphandler)(void *), void *common);
int SetOverflowHandler(int timid, unsigned int (*handler)(void *), void *common);
int SetupHardTimer(int timid, int source, int mode, int prescale);
int StartHardTimer(int timid);
int StopHardTimer(int timid);

#define xtimrman_IMPORTS_start DECLARE_IMPORT_TABLE(timrman, 1, 1)
#define xtimrman_IMPORTS_end   END_IMPORT_TABLE

#define I_SetTimerHandler    DECLARE_IMPORT(20, SetTimerHandler)
#define I_SetOverflowHandler DECLARE_IMPORT(21, SetOverflowHandler)
#define I_SetupHardTimer     DECLARE_IMPORT(22, SetupHardTimer)
#define I_StartHardTimer     DECLARE_IMPORT(23, StartHardTimer)
#define I_StopHardTimer      DECLARE_IMPORT(24, StopHardTimer)

#ifdef __cplusplus
}
#endif

#endif /* __XTIMRMAN_H__ */
