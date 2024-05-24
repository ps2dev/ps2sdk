/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __AIFDEV9_H__
#define __AIFDEV9_H__

#include <types.h>
#include <irx.h>

#include <dev9.h>

#include <aifregs.h>

// AIF management functions
typedef int (*aif_intr_cb_t)(void);

extern int aifIsDetected(void);

extern void aifIntrEnable(int mask);
extern void aifIntrDisable(int mask);

extern void aifRegisterIntrCb(int intr, aif_intr_cb_t cb);
extern int aifRegisterShutdownCb(int idx, dev9_shutdown_cb_t cb);

extern unsigned char aifRTCReadData(unsigned short int address);
extern void aifRTCWriteData(unsigned char data, unsigned short int address);

#define aifdev9_IMPORTS_start DECLARE_IMPORT_TABLE(aifdev9, 1, 1)
#define aifdev9_IMPORTS_end   END_IMPORT_TABLE

#define I_aifIsDetected         DECLARE_IMPORT(4, aifIsDetected)
#define I_aifIntrEnable         DECLARE_IMPORT(5, aifIntrEnable)
#define I_aifIntrDisable        DECLARE_IMPORT(6, aifIntrDisable)
#define I_aifRegisterIntrCb     DECLARE_IMPORT(7, aifRegisterIntrCb)
#define I_aifRegisterShutdownCb DECLARE_IMPORT(8, aifRegisterShutdownCb)
#define I_aifRTCReadData        DECLARE_IMPORT(9, aifRTCReadData)
#define I_aifRTCWriteData       DECLARE_IMPORT(10, aifRTCWriteData)

#endif
