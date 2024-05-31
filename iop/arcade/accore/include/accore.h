/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACCORE_H
#define _ACCORE_H

#include <irx.h>
#include <tamtypes.h>

typedef s8 acInt8;
typedef u8 acUint8;
typedef s16 acInt16;
typedef u16 acUint16;
typedef s32 acInt32;
typedef u32 acUint32;
typedef s64 acInt64;
typedef u64 acUint64;

typedef int acSpl;

typedef enum ac_dma_state
{
	AC_DMA_STATE_FREE = 0x0,
	AC_DMA_STATE_QUEUE = 0x1,
	AC_DMA_STATE_READY = 0x2,
	AC_DMA_STATE_XFER = 0x3,
} acDmaState;

typedef struct ac_dma acDmaData;
typedef acDmaData *acDmaT;

typedef int (*acDmaOp)(acDmaT dma, void *ioptr, void *buf, int count);

typedef struct ac_dma_ops
{
	int (*do_xfer)(acDmaT dma, int intr, acDmaOp op);
	void (*do_done)(acDmaT dma);
	void (*do_error)(acDmaT dma, int intr, acDmaState state, int result);
} acDmaOpsData;
typedef acDmaOpsData *acDmaOpsT;

typedef struct ac_queue acQueueData;
typedef acQueueData *acQueueT;

struct ac_queue
{
	acQueueT q_prev;
	acQueueT q_next;
};

typedef acQueueData acQueueChainData;
typedef acQueueData acQueueHeadData;

struct ac_dma
{
	acQueueChainData d_chain;
	acDmaOpsT d_ops;
	acUint16 d_slice;
	acUint8 d_attr;
	acUint8 d_state;
};

typedef enum ac_intr_num
{
	AC_INTR_NUM_ATA = 0x0,
	AC_INTR_NUM_JV = 0x1,
	AC_INTR_NUM_UART = 0x2,
	AC_INTR_NUM_LAST = 0x2,
} acIntrNum;

typedef int (*acIntrHandler)(void *arg);

typedef volatile acUint16 *acC448Reg;

extern int acDmaModuleRestart(int argc, char **argv);
extern int acDmaModuleStart(int argc, char **argv);
extern int acDmaModuleStatus();
extern int acDmaModuleStop();
extern acDmaT acDmaSetup(acDmaData *dma, acDmaOpsData *ops, int priority, int slice, int output);
extern int acDmaRequest(acDmaT dma);
extern int acDmaRequestI(acDmaT dma);
extern int acDmaCancel(acDmaT dma, int result);
extern int acDmaCancelI(acDmaT dma, int result);
extern int acIntrModuleRestart(int argc, char **argv);
extern int acIntrModuleStart(int argc, char **argv);
extern int acIntrModuleStatus();
extern int acIntrModuleStop();
extern int acIntrRegister(acIntrNum inum, acIntrHandler func, void *arg);
extern int acIntrRelease(acIntrNum inum);
extern int acIntrEnable(acIntrNum inum);
extern int acIntrDisable(acIntrNum inum);
extern int acIntrClear(acIntrNum inum);

#define accore_IMPORTS_start DECLARE_IMPORT_TABLE(accore, 1, 1)
#define accore_IMPORTS_end END_IMPORT_TABLE

#define I_acDmaModuleRestart DECLARE_IMPORT(4, acDmaModuleRestart)
#define I_acDmaModuleStart DECLARE_IMPORT(5, acDmaModuleStart)
#define I_acDmaModuleStatus DECLARE_IMPORT(6, acDmaModuleStatus)
#define I_acDmaModuleStop DECLARE_IMPORT(7, acDmaModuleStop)
#define I_acDmaSetup DECLARE_IMPORT(8, acDmaSetup)
#define I_acDmaRequest DECLARE_IMPORT(9, acDmaRequest)
#define I_acDmaRequestI DECLARE_IMPORT(10, acDmaRequestI)
#define I_acDmaCancel DECLARE_IMPORT(11, acDmaCancel)
#define I_acDmaCancelI DECLARE_IMPORT(12, acDmaCancelI)
#define I_acIntrModuleRestart DECLARE_IMPORT(13, acIntrModuleRestart)
#define I_acIntrModuleStart DECLARE_IMPORT(14, acIntrModuleStart)
#define I_acIntrModuleStatus DECLARE_IMPORT(15, acIntrModuleStatus)
#define I_acIntrModuleStop DECLARE_IMPORT(16, acIntrModuleStop)
#define I_acIntrRegister DECLARE_IMPORT(17, acIntrRegister)
#define I_acIntrRelease DECLARE_IMPORT(18, acIntrRelease)
#define I_acIntrEnable DECLARE_IMPORT(19, acIntrEnable)
#define I_acIntrDisable DECLARE_IMPORT(20, acIntrDisable)
#define I_acIntrClear DECLARE_IMPORT(21, acIntrClear)

#endif
