/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACSRAM_H
#define _ACSRAM_H

#include <accore.h>

typedef acUint32 acSramAddr;

extern int acSramModuleRestart(int argc, char **argv);
extern int acSramModuleStart(int argc, char **argv);
extern int acSramModuleStatus();
extern int acSramModuleStop();
extern int acSramRead(acSramAddr addr, void *buf, int size);
extern int acSramWrite(acSramAddr addr, void *buf, int size);

#define acsram_IMPORTS_start DECLARE_IMPORT_TABLE(acsram, 1, 1)
#define acsram_IMPORTS_end END_IMPORT_TABLE

#define I_acSramModuleRestart DECLARE_IMPORT(4, acSramModuleRestart)
#define I_acSramModuleStart DECLARE_IMPORT(5, acSramModuleStart)
#define I_acSramModuleStatus DECLARE_IMPORT(6, acSramModuleStatus)
#define I_acSramModuleStop DECLARE_IMPORT(7, acSramModuleStop)
#define I_acSramRead DECLARE_IMPORT(8, acSramRead)
#define I_acSramWrite DECLARE_IMPORT(9, acSramWrite)

#endif
