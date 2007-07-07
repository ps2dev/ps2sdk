/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
*/

#ifndef __POWEROFF_H__
#define __POWEROFF_H__

#define poweroff_IMPORTS_start DECLARE_IMPORT_TABLE(poweroff, 1, 1)
#define poweroff_IMPORTS_end END_IMPORT_TABLE

typedef void (*pwoffcb)(void*);

void SetPowerButtonHandler(pwoffcb func, void* param);
#define I_SetPowerButtonHandler DECLARE_IMPORT(4, SetPowerButtonHandler)
void AddPowerOffHandler(pwoffcb func, void* param);
#define I_AddPowerOffHandler DECLARE_IMPORT(5, AddPowerOffHandler)
void RemovePowerOffHandler(pwoffcb func);
#define I_RemovePowerOffHandler DECLARE_IMPORT(6, RemovePowerOffHandler)
void PoweroffShutdown();
#define I_PoweroffShutdown DECLARE_IMPORT(7, PoweroffShutdown)

#endif /* __POWEROFF_H__ */
