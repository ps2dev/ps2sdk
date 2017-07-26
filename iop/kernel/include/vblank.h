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
 * Vertical blank interrupt routines.
 */

#ifndef __VBLANK_H__
#define __VBLANK_H__

#include <irx.h>

void WaitVblankStart();
void WaitVblankEnd();
void WaitVblank();
void WaitNonVblank();

int RegisterVblankHandler(int startend, int priority, int (*handler)(void *),
		void *arg);
int ReleaseVblankHandler(int startend, int (*handler)(void *));

#define vblank_IMPORTS_start DECLARE_IMPORT_TABLE(vblank, 1, 1)
#define vblank_IMPORTS_end END_IMPORT_TABLE

#define I_WaitVblankStart DECLARE_IMPORT(4, WaitVblankStart)
#define I_WaitVblankEnd DECLARE_IMPORT(5, WaitVblankEnd)
#define I_WaitVblank DECLARE_IMPORT(6, WaitVblank)
#define I_WaitNonVblank DECLARE_IMPORT(7, WaitNonVblank)
#define I_RegisterVblankHandler DECLARE_IMPORT(8, RegisterVblankHandler)
#define I_ReleaseVblankHandler DECLARE_IMPORT(9, ReleaseVblankHandler)

#endif /* __VBLANK_H__ */
