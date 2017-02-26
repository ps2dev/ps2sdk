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
 * Additional modload functions only found in newer IOPRP images
 */

#ifndef IOP_XMODLOAD_H
#define IOP_XMODLOAD_H

#include "modload.h"

int GetModuleIdList(int *readbuf, int readbufsize, int *modulecount);
#define I_GetModuleIdList DECLARE_IMPORT(16, GetModuleIdList)

typedef struct {
	char name[56];
	u16 version;
	u16 flags;
	int id;
	u32 entry_addr;
	u32 gp_value;
	u32 text_addr;
	u32 text_size;
	u32 data_size;
	u32 bss_size;
	u32 lreserve[2];
} ModuleStatus;

int ReferModuleStatus(int, ModuleStatus *status);
#define I_ReferModuleStatus DECLARE_IMPORT(17, ReferModuleStatus)

int GetModuleIdListByName(const char *name, int *readbuf, int readbufsize, int *modulecount);
#define I_GetModuleIdListByName DECLARE_IMPORT(18, GetModuleIdListByName)

typedef struct {
	int (*beforeOpen)(void *opt, const char *filename, int flag);
	int (*afterOpen)(void *opt, int fd);
	int (*close)(void *opt, int fd);
	int (*setBufSize)(void *opt, int fd, size_t nbyte);
	int (*beforeRead)(void *opt, int fd, size_t nbyte);
	int (*read)(void *opt, int fd, void *buf, size_t nbyte);
	int (*lseek)(void *opt, int fd, long offset, int whence);
	int (*getfsize)(void *opt, int fd);
} LDfilefunc;

typedef struct {
	char position;
	char access;
	char creserved[2];
	void *distaddr;
	int distoffset;
	LDfilefunc *functable;
	void *funcopt;
	int ireserved[3];
} LMWOoption;

int LoadModuleWithOption(const char *filename, const LMWOoption *option);
#define I_LoadModuleWithOption DECLARE_IMPORT(19, LoadModuleWithOption)

int StopModule(int, int arglen, const char *args, int *result);
#define I_StopModule DECLARE_IMPORT(20, StopModule)

int UnloadModule(int);
#define I_UnloadModule DECLARE_IMPORT(21, UnloadModule)

int SearchModuleByName(const char *name);
#define I_SearchModuleByName DECLARE_IMPORT(22, SearchModuleByName)

int SearchModuleByAddress(const void *addr);
#define I_SearchModuleByAddress DECLARE_IMPORT(23, SearchModuleByAddress)

int SelfStopModule(int arglen, const char *args, int *result);
#define I_SelfStopModule DECLARE_IMPORT(24, SelfStopModule)

void SelfUnloadModule(void);
#define I_SelfUnloadModule DECLARE_IMPORT(25, SelfUnloadModule)

void *AllocLoadMemory(int type, unsigned long size, void *addr);
#define I_AllocLoadMemory DECLARE_IMPORT(26, AllocLoadMemory)

int FreeLoadMemory(void *area);
#define I_FreeLoadMemory DECLARE_IMPORT(27, FreeLoadMemory)

int SetModuleFlags(int, int flag);
#define I_SetModuleFlags DECLARE_IMPORT(28, SetModuleFlags)

#endif /* IOP_XMODLOAD_H */
