/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACFLASH_H
#define _ACFLASH_H

#include <accore.h>

typedef acUint32 acFlashAddr;

typedef struct ac_flash_info
{
	acUint32 fi_blocks;
	acUint32 fi_bsize;
} acFlashInfoData;

typedef acFlashInfoData *acFlashInfoT;

extern int acFlashModuleRestart(int argc, char **argv);
extern int acFlashModuleStart(int argc, char **argv);
extern int acFlashModuleStatus();
extern int acFlashModuleStop();
extern int acFlashStart();
extern int acFlashStatus(acFlashAddr addr);
extern int acFlashStop();
extern int acFlashInfo(acFlashInfoData *info);
extern int acFlashErase(acFlashAddr addr);
extern int acFlashProgram(acFlashAddr addr, void *buf, int count);
extern int acFlashRead(acFlashAddr addr, void *buf, int count);
extern int acFlashVerify(acFlashAddr addr, void *buf, int count);

#define acflash_IMPORTS_start DECLARE_IMPORT_TABLE(acflash, 1, 1)
#define acflash_IMPORTS_end END_IMPORT_TABLE

#define I_acFlashModuleRestart DECLARE_IMPORT(4, acFlashModuleRestart)
#define I_acFlashModuleStart DECLARE_IMPORT(5, acFlashModuleStart)
#define I_acFlashModuleStatus DECLARE_IMPORT(6, acFlashModuleStatus)
#define I_acFlashModuleStop DECLARE_IMPORT(7, acFlashModuleStop)
#define I_acFlashStart DECLARE_IMPORT(8, acFlashStart)
#define I_acFlashStatus DECLARE_IMPORT(9, acFlashStatus)
#define I_acFlashStop DECLARE_IMPORT(10, acFlashStop)
#define I_acFlashInfo DECLARE_IMPORT(11, acFlashInfo)
#define I_acFlashErase DECLARE_IMPORT(12, acFlashErase)
#define I_acFlashProgram DECLARE_IMPORT(13, acFlashProgram)
#define I_acFlashRead DECLARE_IMPORT(14, acFlashRead)
#define I_acFlashVerify DECLARE_IMPORT(15, acFlashVerify)

#endif
