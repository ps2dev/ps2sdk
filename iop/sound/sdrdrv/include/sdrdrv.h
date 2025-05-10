/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _SDRDRV_H
#define _SDRDRV_H

typedef int (*sceSdrUserCommandFunction)(unsigned int command, void *data, int size);

extern int sceSdrChangeThreadPriority(int priority_main, int priority_cb);
extern sceSdrUserCommandFunction sceSdrSetUserCommandFunction(int command, sceSdrUserCommandFunction func);

#define sdrdrv_IMPORTS_start DECLARE_IMPORT_TABLE(sdrdrv, 1, 1)
#define sdrdrv_IMPORTS_end END_IMPORT_TABLE

#define I_sceSdrChangeThreadPriority DECLARE_IMPORT(4, sceSdrChangeThreadPriority)
#define I_sceSdrSetUserCommandFunction DECLARE_IMPORT(5, sceSdrSetUserCommandFunction)

#endif
