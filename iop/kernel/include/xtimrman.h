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

#ifndef IOP_XTIMRMAN_H
#define IOP_XTIMRMAN_H

#include "timrman.h"

int SetTimerHandler(int timid, unsigned long comparevalue, unsigned int (*timeuphandler)(void*), void *common);
#define I_SetTimerHandler DECLARE_IMPORT(20, SetTimerHandler)

int SetOverflowHandler(int timid, unsigned int (*handler)(void*), void *common);
#define I_SetOverflowHandler DECLARE_IMPORT(21, SetOverflowHandler)

int SetupHardTimer(int timid, int source, int mode, int prescale);
#define I_SetupHardTimer DECLARE_IMPORT(22, SetupHardTimer)

int StartHardTimer(int timid);
#define I_StartHardTimer DECLARE_IMPORT(23, StartHardTimer)

int StopHardTimer(int timid);
#define I_StopHardTimer DECLARE_IMPORT(24, StopHardTimer)

#endif /* IOP_XTIMRMAN_H */
