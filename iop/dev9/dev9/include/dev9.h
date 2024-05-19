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

void SpdRegisterIntrHandler(int intr, dev9_intr_cb_t cb);

int SpdDmaTransfer(int ctrl, void *buf, int bcr, int dir);

void Dev9CardStop(void);
void SpdIntrEnable(int mask);
void SpdIntrDisable(int mask);

int SpdGetEthernetID(u16 *buf);

void SpdSetLED(int ctl);
void dev9LED2Ctl(int ctl);
void dev9ControlPIO3(int ctl);

int Dev9RegisterPowerOffHandler(int idx, dev9_shutdown_cb_t cb);

void dev9RegisterPreDmaCb(int ctrl, dev9_dma_cb_t cb);

void dev9RegisterPostDmaCb(int ctrl, dev9_dma_cb_t cb);

#define dev9_IMPORTS_start DECLARE_IMPORT_TABLE(dev9, 1, 9)
#define dev9_IMPORTS_end   END_IMPORT_TABLE

#define I_SpdRegisterIntrHandler      DECLARE_IMPORT(4, SpdRegisterIntrHandler)
#define I_SpdDmaTransfer              DECLARE_IMPORT(5, SpdDmaTransfer)
#define I_Dev9CardStop                DECLARE_IMPORT(6, Dev9CardStop)
#define I_SpdIntrEnable               DECLARE_IMPORT(7, SpdIntrEnable)
#define I_SpdIntrDisable              DECLARE_IMPORT(8, SpdIntrDisable)
#define I_SpdGetEthernetID            DECLARE_IMPORT(9, SpdGetEthernetID)
#define I_SpdSetLED                   DECLARE_IMPORT(10, SpdSetLED)
#define I_Dev9RegisterPowerOffHandler DECLARE_IMPORT(11, Dev9RegisterPowerOffHandler)
#define I_dev9RegisterPreDmaCb        DECLARE_IMPORT(12, dev9RegisterPreDmaCb)
#define I_dev9RegisterPostDmaCb       DECLARE_IMPORT(13, dev9RegisterPostDmaCb)
#define I_dev9ControlPIO3             DECLARE_IMPORT(14, dev9ControlPIO3)
#define I_dev9LED2Ctl                 DECLARE_IMPORT(15, dev9LED2Ctl)

// Backwards-compatibility defines

#define dev9RegisterIntrCb SpdRegisterIntrHandler
#define dev9DmaTransfer SpdDmaTransfer
#define dev9Shutdown Dev9CardStop
#define dev9IntrEnable SpdIntrEnable
#define dev9IntrDisable SpdIntrDisable
#define dev9GetEEPROM SpdGetEthernetID
#define dev9LEDCtl SpdSetLED
#define dev9RegisterShutdownCb Dev9RegisterPowerOffHandler

#define I_dev9RegisterIntrCb I_SpdRegisterIntrHandler
#define I_dev9DmaTransfer I_SpdDmaTransfer
#define I_dev9Shutdown I_Dev9CardStop
#define I_dev9IntrEnable I_SpdIntrEnable
#define I_dev9IntrDisable I_SpdIntrDisable
#define I_dev9GetEEPROM I_SpdGetEthernetID
#define I_dev9LEDCtl I_SpdSetLED
#define I_dev9RegisterShutdownCb I_Dev9RegisterPowerOffHandler

#endif /* __DEV9_H__ */
