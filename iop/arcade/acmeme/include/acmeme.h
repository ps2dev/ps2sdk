/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACMEME_H
#define _ACMEME_H

#include <accore.h>

extern int acMemeModuleRestart(int argc, char **argv);
extern int acMemeModuleStart(int argc, char **argv);
extern int acMemeModuleStatus();
extern int acMemeModuleStop();

#define acmeme_IMPORTS_start DECLARE_IMPORT_TABLE(acmeme, 1, 1)
#define acmeme_IMPORTS_end END_IMPORT_TABLE

#define I_acMemeModuleRestart DECLARE_IMPORT(4, acMemeModuleRestart)
#define I_acMemeModuleStart DECLARE_IMPORT(5, acMemeModuleStart)
#define I_acMemeModuleStatus DECLARE_IMPORT(6, acMemeModuleStatus)
#define I_acMemeModuleStop DECLARE_IMPORT(7, acMemeModuleStop)

#endif
