/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACJV_H
#define _ACJV_H

#include <accore.h>

typedef acUint32 acJvAddr;

extern int acJvModuleRestart(int argc, char **argv);
extern int acJvModuleStart(int argc, char **argv);
extern int acJvModuleStatus();
extern int acJvModuleStop();
extern int acJvRead(acJvAddr addr, void *buf, int size);
extern int acJvWrite(acJvAddr addr, void *buf, int size);
extern int acJvGet(acJvAddr addr);
extern int acJvPut(acJvAddr addr, int value);

#define acjv_IMPORTS_start DECLARE_IMPORT_TABLE(acjv, 1, 1)
#define acjv_IMPORTS_end END_IMPORT_TABLE

#define I_acJvModuleRestart DECLARE_IMPORT(4, acJvModuleRestart)
#define I_acJvModuleStart DECLARE_IMPORT(5, acJvModuleStart)
#define I_acJvModuleStatus DECLARE_IMPORT(6, acJvModuleStatus)
#define I_acJvModuleStop DECLARE_IMPORT(7, acJvModuleStop)
#define I_acJvRead DECLARE_IMPORT(8, acJvRead)
#define I_acJvWrite DECLARE_IMPORT(9, acJvWrite)
#define I_acJvGet DECLARE_IMPORT(10, acJvGet)
#define I_acJvPut DECLARE_IMPORT(11, acJvPut)

#endif
