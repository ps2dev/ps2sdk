/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2021-2021, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Personal video recorder driver.
 * Interface to the DVRP through the DEV9 interface.
 */

#ifndef __PVRDRV_H__
#define __PVRDRV_H__

#include <types.h>
#include <irx.h>

typedef struct __attribute__((aligned(4))) drvdrv_exec_cmd_ack_
{
    u16 command;
    u16 input_word[64];
    u32 input_word_count;
    u16 status_4220_ack;
    u16 ack_status_ack;
    u16 output_word[64];
    u32 status_4228_ack;
    u16 status_4220_comp;
    u16 comp_status;
    u16 return_result_word[64];
    u32 status_4228_comp;
    u32 timeout;
    void *input_buffer;
    u32 input_buffer_length;
    void *output_buffer;
    int ack_status_ack2;
    int phase;
} drvdrv_exec_cmd_ack;

extern int DvrdrvResetSystem();
extern int DvrdrvEnableIntr(u16 a1);
extern int DvrdrvDisableIntr(s16 a1);
extern int DvrdrvRegisterIntrHandler(int a1, void *arg, void (*a3)(int, void *));
extern int DvrdrvUnregisterIntrHandler(void (*a1)(int, void *));
extern int DvrdrvExecCmdAck(drvdrv_exec_cmd_ack *a1);
extern int DvrdrvExecCmdAckComp(drvdrv_exec_cmd_ack *a1);
extern int DvrdrvExecCmdAckDmaSendComp(drvdrv_exec_cmd_ack *a1);
extern int DvrdrvExecCmdAckDmaRecvComp(drvdrv_exec_cmd_ack *a1);
extern int DvrdrvExecCmdAckDma2Comp(drvdrv_exec_cmd_ack *a1);

#define pvrdrv_IMPORTS_start DECLARE_IMPORT_TABLE(pvrdrv, 1, 1)
#define pvrdrv_IMPORTS_end END_IMPORT_TABLE

#define I_DvrdrvResetSystem DECLARE_IMPORT(4, DvrdrvResetSystem)
#define I_DvrdrvEnableIntr DECLARE_IMPORT(5, DvrdrvEnableIntr)
#define I_DvrdrvDisableIntr DECLARE_IMPORT(6, DvrdrvDisableIntr)
#define I_DvrdrvRegisterIntrHandler DECLARE_IMPORT(7, DvrdrvRegisterIntrHandler)
#define I_DvrdrvUnregisterIntrHandler DECLARE_IMPORT(8, DvrdrvUnregisterIntrHandler)
#define I_DvrdrvExecCmdAck DECLARE_IMPORT(9, DvrdrvExecCmdAck)
#define I_DvrdrvExecCmdAckComp DECLARE_IMPORT(10, DvrdrvExecCmdAckComp)
#define I_DvrdrvExecCmdAckDmaSendComp DECLARE_IMPORT(11, DvrdrvExecCmdAckDmaSendComp)
#define I_DvrdrvExecCmdAckDmaRecvComp DECLARE_IMPORT(12, DvrdrvExecCmdAckDmaRecvComp)
#define I_DvrdrvExecCmdAckDma2Comp DECLARE_IMPORT(13, DvrdrvExecCmdAckDma2Comp)

#endif
