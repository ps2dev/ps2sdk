/*
 * vblank.h - Vertical blank interrupt routines.
 *
 * (C)2002, David Ryan (Oobles@hotmail.com)
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 * 
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef IOP_VBLANK_H
#define IOP_VBLANK_H

#include "irx.h"

#define vblank_IMPORTS_start DECLARE_IMPORT_TABLE(vblank, 1, 1)
#define vblank_IMPORTS_end END_IMPORT_TABLE

void WaitVblankStart();
#define I_WaitVblankStart DECLARE_IMPORT(4, WaitVblankStart)
void WaitVblankEnd();
#define I_WaitVblankEnd DECLARE_IMPORT(5, WaitVblankEnd)
void WaitVblank();
#define I_WaitVblank DECLARE_IMPORT(6, WaitVblank)
void WaitNonVblank();
#define I_WaitNonVblank DECLARE_IMPORT(7, WaitNonVblank)

int RegisterVblankHandler(int startend, int priority, int (*handler)(void *),
		void *arg);
#define I_RegisterVblankHandler DECLARE_IMPORT(8, RegisterVblankHandler)
int ReleaseVblankHandler(int startend, int (*handler)(void *));
#define I_ReleaseVblankHandler DECLARE_IMPORT(9, ReleaseVblankHandler)

#endif /* IOP_VBLANK_H */
