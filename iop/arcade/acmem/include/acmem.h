/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACMEM_H
#define _ACMEM_H

#include <accore.h>

typedef struct ac_mem
{
	void *m_buf;
	acUint32 m_size;
	acUint32 m_id;
	acUint32 m_attr;
} acMemData;

typedef acMemData *acMemT;

typedef acUint32 acMemEEaddr;

extern int acMemModuleRestart(int argc, char **argv);
extern int acMemModuleStart(int argc, char **argv);
extern int acMemModuleStatus();
extern int acMemModuleStop();
extern acMemT acMemSetup(acMemData *mem, void *buf, int size);
extern int acMemSend(acMemT mem, acMemEEaddr addr, int size, int retry);
extern int acMemReceive(acMemT mem, acMemEEaddr addr, int size);
extern int acMemWait(acMemT mem, int threshold, int retry);

#define acmem_IMPORTS_start DECLARE_IMPORT_TABLE(acmem, 1, 1)
#define acmem_IMPORTS_end END_IMPORT_TABLE

#define I_acMemModuleRestart DECLARE_IMPORT(4, acMemModuleRestart)
#define I_acMemModuleStart DECLARE_IMPORT(5, acMemModuleStart)
#define I_acMemModuleStatus DECLARE_IMPORT(6, acMemModuleStatus)
#define I_acMemModuleStop DECLARE_IMPORT(7, acMemModuleStop)
#define I_acMemSetup DECLARE_IMPORT(8, acMemSetup)
#define I_acMemSend DECLARE_IMPORT(9, acMemSend)
#define I_acMemReceive DECLARE_IMPORT(10, acMemReceive)
#define I_acMemWait DECLARE_IMPORT(11, acMemWait)

#endif
