/*      
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# IOP executable file loader.
*/

#ifndef LOADFILE_H
#define LOADFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <tamtypes.h>

#define LF_PATH_MAX	252
#define LF_ARG_MAX	252

enum _lf_val_types {
	LF_VAL_BYTE  = 0,
	LF_VAL_SHORT,
	LF_VAL_LONG
};

typedef struct 
{
	u32 epc;
	u32 gp;
	u32 sp;
	u32 dummy;  
} t_ExecData;

int SifLoadFileInit(void);
void SifLoadFileExit(void);

int SifLoadModule(const char *path, int arg_len, const char *args);
int SifLoadStartModule(const char *path, int arg_len, const char *args, int *mod_res);

int SifLoadModuleEncrypted(const char *path, int arg_len, const char *args);

int SifLoadModuleBuffer(void *ptr, int arg_len, const char *args);
int SifLoadStartModuleBuffer(void *ptr, int arg_len, const char *args, int *mod_res);

int SifStopModule(int id, int arg_len, const char *args, int *mod_res);
int SifUnloadModule(int id);
int SifSearchModuleByName(const char *name);
int SifSearchModuleByAddress(const void *ptr);

int SifLoadElfPart(const char *path, const char *secname, t_ExecData *data);
int SifLoadElf(const char *path, t_ExecData *data);
int SifLoadElfEncrypted(const char *path, t_ExecData *data);

int SifIopSetVal(u32 iop_addr, int val, int type);
int SifIopGetVal(u32 iop_addr, void *val, int type);

/* Load a module via a buffer stored in EE RAM.  */
int SifExecModuleBuffer(void *ptr, u32 size, u32 arg_len, const char *args, int *mod_res);
/* Load a module via a buffer read in from a file.  */
int SifExecModuleFile(const char *path, u32 arg_len, const char *args, int *mod_res);

#ifdef __cplusplus
}
#endif

#endif /* LOADFILE_H */
