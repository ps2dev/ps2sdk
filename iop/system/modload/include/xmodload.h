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

#ifndef __XMODLOAD_H__
#define __XMODLOAD_H__

#include <modload.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int GetModuleIdList(int *readbuf, int readbufsize, int *modulecount);

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

extern int ReferModuleStatus(int modid, ModuleStatus *status);
extern int GetModuleIdListByName(const char *name, int *readbuf, int readbufsize, int *modulecount);

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

extern int LoadModuleWithOption(const char *filename, const LMWOoption *option);
extern int StopModule(int modid, int arglen, const char *args, int *result);
extern int UnloadModule(int modid);
extern int SearchModuleByName(const char *name);
extern int SearchModuleByAddress(const void *addr);
extern int SelfStopModule(int arglen, const char *args, int *result);
extern void SelfUnloadModule(void);
extern void *AllocLoadMemory(int type, unsigned int size, void *addr);
extern int FreeLoadMemory(void *area);
extern int SetModuleFlags(int modid, int flag);

#define xmodload_IMPORTS_start DECLARE_IMPORT_TABLE(modload, 1, 7)
#define xmodload_IMPORTS_end END_IMPORT_TABLE

#define I_GetModuleIdList DECLARE_IMPORT(16, GetModuleIdList)
#define I_ReferModuleStatus DECLARE_IMPORT(17, ReferModuleStatus)
#define I_GetModuleIdListByName DECLARE_IMPORT(18, GetModuleIdListByName)
#define I_LoadModuleWithOption DECLARE_IMPORT(19, LoadModuleWithOption)
#define I_StopModule DECLARE_IMPORT(20, StopModule)
#define I_UnloadModule DECLARE_IMPORT(21, UnloadModule)
#define I_SearchModuleByName DECLARE_IMPORT(22, SearchModuleByName)
#define I_SearchModuleByAddress DECLARE_IMPORT(23, SearchModuleByAddress)
#define I_SelfStopModule DECLARE_IMPORT(26, SelfStopModule)
#define I_SelfUnloadModule DECLARE_IMPORT(27, SelfUnloadModule)
#define I_AllocLoadMemory DECLARE_IMPORT(28, AllocLoadMemory)
#define I_FreeLoadMemory DECLARE_IMPORT(29, FreeLoadMemory)
#define I_SetModuleFlags DECLARE_IMPORT(30, SetModuleFlags)

#ifdef __cplusplus
}
#endif

#endif /* __XMODLOAD_H__ */
