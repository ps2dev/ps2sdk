/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACTIMER_H
#define _ACTIMER_H

#include <accore.h>

typedef acUint64 acTime;
typedef struct ac_timer acTimerData;
typedef acTimerData *acTimerT;
typedef void (*acTimerDone)(acTimerT timer, void *t_arg);

struct ac_timer
{
	acQueueChainData t_chain;
	acTime t_deadline;
	acTimerDone t_done;
	void *t_arg;
};

extern int acTimerModuleStart(int argc, char **argv);
extern int acTimerModuleRestart(int argc, char **argv);
extern int acTimerModuleStop();
extern int acTimerModuleStatus();
extern int acTimerAdd(acTimerT timer, acTimerDone done, void *arg, unsigned int us);
extern int acTimerRemove(acTimerT timer);

#define actimer_IMPORTS_start DECLARE_IMPORT_TABLE(actimer, 1, 1)
#define actimer_IMPORTS_end END_IMPORT_TABLE

#define I_acTimerModuleStart DECLARE_IMPORT(4, acTimerModuleStart)
#define I_acTimerModuleRestart DECLARE_IMPORT(5, acTimerModuleRestart)
#define I_acTimerModuleStop DECLARE_IMPORT(6, acTimerModuleStop)
#define I_acTimerModuleStatus DECLARE_IMPORT(7, acTimerModuleStatus)
#define I_acTimerAdd DECLARE_IMPORT(8, acTimerAdd)
#define I_acTimerRemove DECLARE_IMPORT(9, acTimerRemove)

#endif
