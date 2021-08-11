/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * DEV9 Device Driver definitions and imports.
 */

#ifndef __DEV9_H__
#define __DEV9_H__

#include <types.h>
#include <irx.h>

#include <hdd-ioctl.h>

typedef int (*dev9_intr_cb_t)(int flag);
typedef void (*dev9_shutdown_cb_t)(void);
typedef void (*dev9_dma_cb_t)(int bcr, int dir);

void dev9RegisterIntrCb(int intr, dev9_intr_cb_t cb);

int dev9DmaTransfer(int ctrl, void *buf, int bcr, int dir);

void dev9Shutdown(void);
void dev9IntrEnable(int mask);
void dev9IntrDisable(int mask);

int dev9GetEEPROM(u16 *buf);

void dev9LEDCtl(int ctl);
void dev9LED2Ctl(int ctl);
void dev9ControlPIO3(int ctl);

int dev9RegisterShutdownCb(int idx, dev9_shutdown_cb_t cb);

void dev9RegisterPreDmaCb(int ctrl, dev9_dma_cb_t cb);

void dev9RegisterPostDmaCb(int ctrl, dev9_dma_cb_t cb);

#define dev9_IMPORTS_start DECLARE_IMPORT_TABLE(dev9, 1, 9)
#define dev9_IMPORTS_end   END_IMPORT_TABLE

#define I_dev9RegisterIntrCb     DECLARE_IMPORT(4, dev9RegisterIntrCb)
#define I_dev9DmaTransfer        DECLARE_IMPORT(5, dev9DmaTransfer)
#define I_dev9Shutdown           DECLARE_IMPORT(6, dev9Shutdown)
#define I_dev9IntrEnable         DECLARE_IMPORT(7, dev9IntrEnable)
#define I_dev9IntrDisable        DECLARE_IMPORT(8, dev9IntrDisable)
#define I_dev9GetEEPROM          DECLARE_IMPORT(9, dev9GetEEPROM)
#define I_dev9LEDCtl             DECLARE_IMPORT(10, dev9LEDCtl)
#define I_dev9RegisterShutdownCb DECLARE_IMPORT(11, dev9RegisterShutdownCb)
#define I_dev9RegisterPreDmaCb   DECLARE_IMPORT(12, dev9RegisterPreDmaCb)
#define I_dev9RegisterPostDmaCb  DECLARE_IMPORT(13, dev9RegisterPostDmaCb)
#define I_dev9ControlPIO3        DECLARE_IMPORT(14, dev9ControlPIO3)
#define I_dev9LED2Ctl            DECLARE_IMPORT(15, dev9LED2Ctl)

#endif /* __DEV9_H__ */
