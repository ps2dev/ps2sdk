/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Kernel module loader.
*/

#ifndef IOP_MODLOAD_H
#define IOP_MODLOAD_H

#include "types.h"
#include "irx.h"

#define modload_IMPORTS_start DECLARE_IMPORT_TABLE(modload, 1, 1)
#define modload_IMPORTS_end END_IMPORT_TABLE

int ReBootStart(int);
#define I_ReBootStart DECLARE_IMPORT(4, ReBootStart)

int LoadModuleAddress(const char *name, int, int);
#define I_LoadModuleAddress DECLARE_IMPORT(5, LoadModuleAddress)

int LoadModule(const char *name);
#define I_LoadModule DECLARE_IMPORT(6, LoadModule)

int LoadStartModule(const char *name, int arglen, const char *args, int *result);
#define I_LoadStartModule DECLARE_IMPORT(7, LoadStartModule)

int StartModule(int, const char *name, int arg_len, const char *args, int *result);
#define I_StartModule DECLARE_IMPORT(8, StartModule)

int LoadModuleBufferAddress(void *buffer, int, int);
#define I_LoadModuleBufferAddress DECLARE_IMPORT(9, LoadModuleBufferAddress)

int LoadModuleBuffer(void *buffer);
#define I_LoadModuleBuffer DECLARE_IMPORT(10, LoadModuleBuffer)

#endif /* IOP_MODLOAD_H */
