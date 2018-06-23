/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Kernel module loader.
 */

#ifndef __MODLOAD_H__
#define __MODLOAD_H__

#include <types.h>
#include <irx.h>

void *GetModloadInternalData(void **pInternalData);

int ReBootStart(const char *command, unsigned int flags);
int LoadModuleAddress(const char *name, int, int);
int LoadModule(const char *name);
int LoadStartModule(const char *name, int arglen, const char *args, int *result);
int StartModule(int, const char *name, int arglen, const char *args, int *result);
int LoadModuleBufferAddress(void *buffer, int, int);
int LoadModuleBuffer(void *buffer);
void SetSecrmanCallbacks(void *SecrCardBootFile_fnc, void *SecrDiskBootFile_fnc, void *SetLoadfileCallbacks_fnc);
void SetCheckKelfPathCallback(void *CheckKelfPath_fnc);

#define modload_IMPORTS_start DECLARE_IMPORT_TABLE(modload, 1, 1)
#define modload_IMPORTS_end END_IMPORT_TABLE

#define I_GetModloadInternalData DECLARE_IMPORT(3, GetModloadInternalData);
#define I_ReBootStart DECLARE_IMPORT(4, ReBootStart)
#define I_LoadModuleAddress DECLARE_IMPORT(5, LoadModuleAddress)
#define I_LoadModule DECLARE_IMPORT(6, LoadModule)
#define I_LoadStartModule DECLARE_IMPORT(7, LoadStartModule)
#define I_StartModule DECLARE_IMPORT(8, StartModule)
#define I_LoadModuleBufferAddress DECLARE_IMPORT(9, LoadModuleBufferAddress)
#define I_LoadModuleBuffer DECLARE_IMPORT(10, LoadModuleBuffer)
#define I_SetSecrmanCallbacks DECLARE_IMPORT(12, SetSecrmanCallbacks)
#define I_SetCheckKelfPathCallback DECLARE_IMPORT(13, SetCheckKelfPathCallback)

#endif /* __MODLOAD_H__ */
