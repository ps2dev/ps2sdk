/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACRAM_H
#define _ACRAM_H

#include <accore.h>

typedef acUint32 acRamAddr;

typedef struct ac_ram acRamData;
typedef acRamData *acRamT;
typedef void (*acRamDone)(acRamT ram, void *arg, int ret);

struct ac_ram
{
	acQueueChainData r_chain;
	acRamDone r_done;
	void *r_arg;
	void *r_buf;
	acUint32 r_count;
	acRamAddr r_addr;
	acInt32 r_tmout;
};

extern int acRamModuleRestart(int argc, char **argv);
extern int acRamModuleStart(int argc, char **argv);
extern int acRamModuleStatus();
extern int acRamModuleStop();
extern acRamT acRamSetup(acRamData *ram, acRamDone done, void *arg, int tmout);
extern int acRamRead(acRamT ram, acRamAddr addr, void *buf, int count);
extern int acRamReadI(acRamT ram, acRamAddr addr, void *buf, int count);
extern int acRamWrite(acRamT ram, acRamAddr addr, void *buf, int count);
extern int acRamWriteI(acRamT ram, acRamAddr addr, void *buf, int count);

#define acram_IMPORTS_start DECLARE_IMPORT_TABLE(acram, 1, 1)
#define acram_IMPORTS_end END_IMPORT_TABLE

#define I_acRamModuleRestart DECLARE_IMPORT(4, acRamModuleRestart)
#define I_acRamModuleStart DECLARE_IMPORT(5, acRamModuleStart)
#define I_acRamModuleStatus DECLARE_IMPORT(6, acRamModuleStatus)
#define I_acRamModuleStop DECLARE_IMPORT(7, acRamModuleStop)
#define I_acRamSetup DECLARE_IMPORT(8, acRamSetup)
#define I_acRamRead DECLARE_IMPORT(9, acRamRead)
#define I_acRamReadI DECLARE_IMPORT(10, acRamReadI)
#define I_acRamWrite DECLARE_IMPORT(11, acRamWrite)
#define I_acRamWriteI DECLARE_IMPORT(12, acRamWriteI)

#endif
