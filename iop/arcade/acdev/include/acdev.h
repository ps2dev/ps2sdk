/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACDEV_H
#define _ACDEV_H

#include <irx.h>
#include <tamtypes.h>

extern int SetAcMemDelayReg(unsigned int value);
extern int GetAcMemDelayReg(void);
extern int SetAcIoDelayReg(unsigned int value);
extern int GetAcIoDelayReg(void);

#define acdev_IMPORTS_start DECLARE_IMPORT_TABLE(acdev, 1, 1)
#define acdev_IMPORTS_end END_IMPORT_TABLE

#define I_SetAcMemDelayReg DECLARE_IMPORT(4, SetAcMemDelayReg)
#define I_GetAcMemDelayReg DECLARE_IMPORT(5, GetAcMemDelayReg)
#define I_SetAcIoDelayReg DECLARE_IMPORT(6, SetAcIoDelayReg)
#define I_GetAcIoDelayReg DECLARE_IMPORT(7, GetAcIoDelayReg)

#endif
