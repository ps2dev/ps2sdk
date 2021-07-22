/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2021-2021, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx.h"
#include "tamtypes.h"
#include "stdbool.h"
#include "dev9.h"
#include "sysmem.h"
#include "intrman.h"
#include "loadcore.h"
#include "stdio.h"
#include "thbase.h"
#include "thsemap.h"
#include "speedregs.h"

#include "pvrdrv.h"

// Based off of DESR / PSX DVR system software version 1.31.
#define MODNAME "DVR_Basic_driver"
IRX_ID(MODNAME, 1, 1);

typedef struct struct_itr_sid_tbl_
{
    u16 command;
    u8 gap2[2];
    int sema;
    u16 error;
    u8 timed_out;
    u8 byteB;
} struct_itr_sid_tbl;

typedef struct __attribute__((aligned(4))) struct_itr_sema_
{
    int sema;
    u8 used;
} struct_itr_sema;

typedef struct struct_dvrdrv_
{
    int arg1;
    u16 arg2;
} struct_dvrdrv;

int _start(int a1);
int module_start();
int module_stop();
int DvrdrvInit();
int DvrdrvSendCmdAck(struct_itr_sema *itrsema, u16 command, u16 *input_word, s32 input_word_count, u16 *status_4220, u16 *ack_status, u32 *status_4228);
int DvrdrvSetDmaDirection(u32 arg);
int DvrdrvTransferDma(u8 *output_buffer, int a2);
void DvrPreDmaHandler(int bcr, int dir);
void DvrPostDmaHandler(int bcr, int dir);
int DvrdrvPrepareWaitDmaEnd(struct_itr_sema *itrsema, u16 command);
int DvrdrvCancelWaitDmaEnd(u16 command);
int DvrdrvWaitDmaEnd(struct_itr_sema *itrsema, u16 command);
int DvrdrvPrepareWaitCmdComp(struct_itr_sema *itrsema, u16 command, u32 timeout);
int DvrdrvCancelWaitCmdComp(u16 command);
int DvrdrvWaitCmdComp(struct_itr_sema *itrsema, u16 command, u16 *status_4220, u16 *comp_status, u32 *status_4228);
s32 DvrdrvBlockPhase();
s32 DvrdrvUnblockPhase();
int DvrdrvEnd();
int DVR_INTR_HANDLER(int flag);
int INTR_DVRRDY_HANDLER(int a1, struct_dvrdrv *a2);
int INTR_DVRRDY_TO_HANDLER(void *a1);
void INTR_CMD_ACK_HANDLER(int, void *);
int INTR_CMD_ACK_TO_HANDLER(struct_itr_sid_tbl *a1);
void INTR_CMD_COMP_HANDLER(int, void *);
unsigned int INTR_CMD_COMP_TO_HANDLER(struct_itr_sid_tbl *a1);
void INTR_DMAACK_HANDLER(int, void *);
unsigned int INTR_DMAACK_TO_HANDLER(struct_itr_sid_tbl *a1);
void INTR_DMAEND_HANDLER(int, void *);
int BlockAPI();
int UnblockAPI();
struct_itr_sid_tbl *SetItrSidTbl(int itrsid_index, u16 command, int sema);
struct_itr_sid_tbl *GetItrSidTbl(int itrsid_index, u16 command);
int ClearItrSidTbl(struct_itr_sid_tbl *a1);
struct_itr_sema *AllocItrSema();
int ReleaseItrSema(struct_itr_sema *itrsema);

extern struct irx_export_table _exp_pvrdrv;
struct_dvrdrv DVRDRV;
s32 api_sema_id;
s32 phase_sema_id;
void (*intrhandler_callbacks[32])(int, void *);
int intrhandler_intrnum[32];
void *intrhandler_callbacksarg[32];
struct_itr_sid_tbl itrsid_table[3][32];
struct_itr_sema itr_sema_table[32];

int _start(int a1)
{
    int result;

    if (a1 >= 0)
        result = module_start();
    else
        result = module_stop();
    return result;
}

int module_start()
{
    int result = 1;

    if (DvrdrvInit() != -1) {
        if (RegisterLibraryEntries(&_exp_pvrdrv) == 0) {
            result = 2;
        }
    }
    return result;
}

int module_stop()
{
    bool v0;
    int result;

    ReleaseLibraryEntries(&_exp_pvrdrv);
    v0 = DvrdrvEnd() == 0;
    result = 1;
    if (!v0)
        result = 2;
    return result;
}

int DvrdrvInit()
{
    int v0;
    int v1;
    iop_sema_t v12;
    iop_sema_t v13;

    v12.attr = 0;
    v12.initial = 1;
    v12.max = 1;
    v12.option = 0;
    v0 = CreateSema(&v12);
    if (v0 < 0)
        return -1;
    api_sema_id = v0;
    v13.attr = 0;
    v13.initial = 1;
    v13.max = 1;
    v13.option = 0;
    v1 = CreateSema(&v13);
    if (v1 < 0)
        return -1;
    phase_sema_id = v1;
    v13.attr = 0;
    v13.initial = 0;
    v13.max = 1;
    v13.option = 0;
    for (int i = 0; i < 32; i += 1) {
        itr_sema_table[i].sema = CreateSema(&v13);
        itr_sema_table[i].used = 0;
    }
    for (int i = 0; i < 32; i += 1) {
        intrhandler_callbacks[i] = 0;
        intrhandler_intrnum[i] = 0;
    }
    for (int i = 0; i < 3; i += 1) {
        for (int j = 0; j < 32; j += 1) {
            ClearItrSidTbl(&itrsid_table[i][j]);
        }
    }
    dev9RegisterIntrCb(9, DVR_INTR_HANDLER);
    DvrdrvRegisterIntrHandler(1, &DVRDRV, (void (*)(int, void *))INTR_DVRRDY_HANDLER);
    DvrdrvRegisterIntrHandler(2, &DVRDRV, INTR_CMD_ACK_HANDLER);
    DvrdrvRegisterIntrHandler(4, &DVRDRV, INTR_CMD_COMP_HANDLER);
    DvrdrvRegisterIntrHandler(8, &DVRDRV, INTR_DMAACK_HANDLER);
    DvrdrvRegisterIntrHandler(16, &DVRDRV, INTR_DMAEND_HANDLER);
    DvrdrvEnableIntr(0x1Fu);
    dev9RegisterPreDmaCb(2, DvrPreDmaHandler);
    dev9RegisterPostDmaCb(2, DvrPostDmaHandler);
    return 0;
}

int DvrdrvResetSystem()
{
    s32 v0;
    bool v1;
    int v2;
    iop_sys_clock_t v4;
    USE_SPD_REGS;

    BlockAPI();
    SPD_REG16(0x4008) &= 0xFEu;
    DelayThread(100000);
    SPD_REG16(0x4008) |= 1u;
    DVRDRV.arg1 = GetThreadId();
    USec2SysClock(0x1C9C380u, &v4);
    SetAlarm(&v4, (unsigned int (*)(void *))INTR_DVRRDY_TO_HANDLER, &DVRDRV);
    v0 = SleepThread();
    CancelAlarm((unsigned int (*)(void *))INTR_DVRRDY_TO_HANDLER, &DVRDRV);
    v1 = v0 != 0;
    v2 = -2;
    if (!v1) {
        v2 = -1;
        if ((DVRDRV.arg2 & 1) != 0)
            v2 = 0;
        DVRDRV.arg2 = 0;
    }
    DvrdrvDisableIntr(1);
    UnblockAPI();
    return v2;
}

int DvrdrvSendCmdAck(struct_itr_sema *itrsema, u16 command, u16 *input_word, s32 input_word_count, u16 *status_4220, u16 *ack_status, u32 *status_4228)
{
    int v11;
    int v12;
    struct_itr_sid_tbl *v14;
    s32 v15;
    u16 *v16;
    s16 v17;
    int v18;
    unsigned int (*v19)(void *);
    int v20;
    int v21;
    unsigned int (*v22)(void *);
    s16 v23;
    unsigned int v24;
    int v25;
    s32 i;
    iop_sys_clock_t v28;
    USE_SPD_REGS;

    v11 = -1;
    v12 = 0;
    do {
        if ((SPD_REG16(0x4230) & 2) != 0)
            break;
        DelayThread(1000);
        ++v12;
    } while (v12 < 100);
    if (v12 == 100) {
        printf("DvrdrvSendCmdAck -> Command is running... (Time out)\n");
        return -2;
    }
    BlockAPI();
    v14 = SetItrSidTbl(0, command, itrsema->sema);
    if (!v14) {
        v11 = -1;
        printf("DvrdrvSendCmdAck() -> SetItrSidTbl Error\n");
        goto LABEL_44;
    }
    SPD_REG16(0x4218) |= 0x80u;
    while ((SPD_REG16(0x4218) & 0x80) != 0)
        ;
    v15 = 0;
    if (input_word_count > 0) {
        v16 = input_word;
        do {
            v17 = *v16++;
            ++v15;
            SPD_REG16(0x4214) = v17;
        } while (v15 < input_word_count);
    }
    USec2SysClock(0x1C9C380u, &v28);
    v18 = (u16)(command & 0xF00) >> 8;
    if (v18 == 1) {
        v19 = (unsigned int (*)(void *))INTR_CMD_ACK_TO_HANDLER;
    LABEL_18:
        SetAlarm(&v28, v19, v14);
        goto LABEL_19;
    }
    if (v18 && (u16)(command & 0xF00) >> 8 < 4u) {
        v19 = (unsigned int (*)(void *))INTR_DMAACK_TO_HANDLER;
        goto LABEL_18;
    }
LABEL_19:
    v20 = itrsema->sema;
    SPD_REG16(0x4210) = command;
    WaitSema(v20);
    v21 = (u16)(command & 0xF00) >> 8;
    if (v21 == 1) {
        v22 = (unsigned int (*)(void *))INTR_CMD_ACK_TO_HANDLER;
    } else {
        if (!v21 || (u16)(command & 0xF00) >> 8 >= 4u) {
            printf("DvrdrvSendCmdAck() -> Error!\n");
            goto LABEL_26;
        }
        v22 = (unsigned int (*)(void *))INTR_DMAACK_TO_HANDLER;
    }
    CancelAlarm(v22, v14);
LABEL_26:
    if (!v14->timed_out || v14->error) {
        v23 = v14->error;
        v24 = command & 0xF00;
        if ((v23 & 0x8000) == 0) {
            if (v24 >> 8 == 1) {
                v11 = 0;
                if ((v23 & 2) == 0) {
                    v11 = -1;
                    printf("DvrdrvSendCmdAck -> Interrupt Flag Error!,%04X\n", (u16)v14->error);
                    goto LABEL_44;
                }
            } else if (v24 >> 8) {
                if (v24 >> 8 < 4) {
                    v11 = 0;
                    if ((v14->error & 8) == 0) {
                        v11 = -1;
                        goto LABEL_44;
                    }
                }
            }
            v25 = 64;
            if ((SPD_REG16(0x4228) & 1) == 0)
                v25 = (u8)(SPD_REG16(0x4228) & 0xFC) >> 2;
            *status_4228 = v25;
            for (i = 0; i < *status_4228; ++ack_status) {
                ++i;
                *ack_status = SPD_REG16(0x4224);
            }
            *status_4220 = SPD_REG16(0x4220);
            goto LABEL_44;
        }
        v11 = -3;
    } else {
        v11 = -2;
        printf("DvrdrvSendCmdAck() -> TO!\n");
    }
LABEL_44:
    ClearItrSidTbl(v14);
    UnblockAPI();
    return v11;
}

int DvrdrvSetDmaDirection(u32 arg)
{
    USE_SPD_REGS;
    if (arg < 2 && (SPD_REG16(0x4100) & 2) != 0) {
        while ((SPD_REG16(0x4100) & 2) != 0)
            ;
    }
    if ((SPD_REG16(0x4004) & 7) == arg)
        return 0;
    SPD_REG16(0x4004) = arg | (SPD_REG16(0x4004) & 0xF8);
    SPD_REG16(0x4100) |= 1u;
    while ((SPD_REG16(0x4100) & 1) != 0)
        ;
    return 0;
}

int DvrdrvTransferDma(u8 *output_buffer, int a2)
{
    int v4;
    int result;
    int v6;
    int v7;
    u32 *v8;
    USE_SPD_REGS;

    v4 = 0;
    if (((u32)output_buffer & 3) != 0)
        return -1;
    BlockAPI();
    v6 = a2 / 128;
    v7 = a2 % 128;
    SPD_REG16(0x4108) = a2 / 128;
    SPD_REG16(0x410C) = 32;
    v8 = (u32 *)&output_buffer[128 * (u16)(a2 / 128)];
    switch (SPD_REG16(0x4004) & 7) {
        case 0:
        case 2:
        case 7:
            v4 = 0;
            goto LABEL_7;
        case 1:
        case 3:
            v4 = 1;
            goto LABEL_7;
        case 4:
        case 5:
        case 6:
            UnblockAPI();
            return -1;
        default:
        LABEL_7:
            dev9DmaTransfer(2, output_buffer, (v6 << 16) | 0x20, v4);
            if (v7 > 0) {
                // TODO: verify this 16-bit copy
                do {
                    v7 -= 4;
                    SPD_REG16(4120) = ((*((u32 *)v8)) & 0x0000FFFF);
                    SPD_REG16(4122) = ((*((u32 *)v8)) & 0xFFFF0000) >> 16;
                    v8 += 1;
                } while (v7 > 0);
            }
            UnblockAPI();
            result = 0;
            break;
    }
    return result;
}

void DvrPreDmaHandler(int bcr, int dir)
{
    USE_SPD_REGS;
    SPD_REG16(0x4100) |= 2u;
}

void DvrPostDmaHandler(int bcr, int dir)
{
    USE_SPD_REGS;
    while ((SPD_REG16(0x4100) & 2) != 0)
        ;
}

int DvrdrvPrepareWaitDmaEnd(struct_itr_sema *itrsema, u16 command)
{
    int v4;

    BlockAPI();
    if (itrsema) {
        v4 = 0;
        SetItrSidTbl(1, command, itrsema->sema);
    } else {
        v4 = -1;
        printf("DvrdrvPrepareWaitDmaEnd : itrsema==NULL\n");
    }
    UnblockAPI();
    return v4;
}

int DvrdrvCancelWaitDmaEnd(u16 command)
{
    struct_itr_sid_tbl *v2;
    int v3;

    BlockAPI();
    v2 = GetItrSidTbl(1, command);
    if (v2) {
        ClearItrSidTbl(v2);
        v3 = 0;
    } else {
        v3 = -1;
        printf("DvrdrvCancelWaitDmaEnd -> Cannot found Sid!!\n");
    }
    UnblockAPI();
    return v3;
}

int DvrdrvWaitDmaEnd(struct_itr_sema *itrsema, u16 command)
{
    struct_itr_sid_tbl *v3;
    s16 v4;
    int v5;
    u8 v6;
    int v7;
    char *v8;
    char v10;
    USE_SPD_REGS;

    WaitSema(itrsema->sema);
    v3 = GetItrSidTbl(1, command);
    v4 = v3->error;
    v5 = -3;
    if ((v4 & 0x8000) == 0) {
        if ((v4 & 0x10) != 0) {
            v5 = 0;
            if ((SPD_REG16(0x4228) & 1) != 0)
                v6 = 64;
            else
                v6 = (u8)(SPD_REG16(0x4228) & 0xFC) >> 2;
            v7 = 0;
            if (v6) {
                v8 = &v10;
                do {
                    ++v7;
                    *(u16 *)v8 = SPD_REG16(0x4224);
                    v8 += 2;
                } while (v7 < v6);
            }
        } else {
            v5 = -1;
            printf("DvrdrvWaitDmaEnd -> Interrupt Flag Error!,%04X\n", (u16)v3->error);
        }
    }
    ClearItrSidTbl(v3);
    return v5;
}

int DvrdrvPrepareWaitCmdComp(struct_itr_sema *itrsema, u16 command, u32 timeout)
{
    int v6;
    int v7;
    struct_itr_sid_tbl *v8;
    iop_sys_clock_t v10;

    BlockAPI();
    if (itrsema) {
        v7 = itrsema->sema;
        v6 = 0;
        v8 = SetItrSidTbl(2, command, v7);
        USec2SysClock(timeout, &v10);
        SetAlarm(&v10, (unsigned int (*)(void *))INTR_CMD_COMP_TO_HANDLER, v8);
    } else {
        v6 = -1;
        printf("DvrdrvPrepareWaitCmdComp : itrsema==NULL\n");
    }
    UnblockAPI();
    return v6;
}

int DvrdrvCancelWaitCmdComp(u16 command)
{
    struct_itr_sid_tbl *v2;
    struct_itr_sid_tbl *v3;
    int v4;

    BlockAPI();
    v2 = GetItrSidTbl(2, command);
    v3 = v2;
    if (v2) {
        CancelAlarm((unsigned int (*)(void *))INTR_CMD_COMP_TO_HANDLER, v2);
        ClearItrSidTbl(v3);
        v4 = 0;
    } else {
        v4 = -1;
        printf("DvrdrvCancelWaitCmdComp -> Cannot found Sid!!\n");
    }
    UnblockAPI();
    return v4;
}

int DvrdrvWaitCmdComp(struct_itr_sema *itrsema, u16 command, u16 *status_4220, u16 *comp_status, u32 *status_4228)
{
    struct_itr_sid_tbl *v8;
    int v9;
    s16 v10;
    int v11;
    s32 v12;
    u16 *v13;
    USE_SPD_REGS;

    WaitSema(itrsema->sema);
    v8 = GetItrSidTbl(2, command);
    CancelAlarm((unsigned int (*)(void *))INTR_CMD_COMP_TO_HANDLER, v8);
    v9 = -2;
    if (!v8->timed_out) {
        v10 = v8->error;
        v9 = -3;
        if ((v10 & 0x8000) == 0) {
            if ((v10 & 4) != 0) {
                v9 = 0;
                if ((SPD_REG16(0x4228) & 1) != 0)
                    v11 = 64;
                else
                    v11 = (u8)(SPD_REG16(0x4228) & 0xFC) >> 2;
                *status_4228 = v11;
                v12 = 0;
                if (*status_4228 > 0) {
                    v13 = comp_status;
                    do {
                        ++v12;
                        *v13++ = SPD_REG16(0x4224);
                    } while (v12 < *status_4228);
                }
                *status_4220 = SPD_REG16(0x4220);
            } else {
                v9 = -1;
                printf("DvrdrvWaitCmdComp -> Interrupt Flag Error!,%04X\n", (u16)v8->error);
            }
        }
    }
    ClearItrSidTbl(v8);
    return v9;
}

s32 DvrdrvBlockPhase()
{
    return WaitSema(phase_sema_id);
}

s32 DvrdrvUnblockPhase()
{
    return SignalSema(phase_sema_id);
}

int DvrdrvEnableIntr(u16 a1)
{
    int state;
    USE_SPD_REGS;

    CpuSuspendIntr(&state);
    SPD_REG16(0x4208) |= a1;
    dev9IntrEnable(0x200);
    CpuResumeIntr(state);
    return 0;
}

int DvrdrvDisableIntr(s16 a1)
{
    int state;
    USE_SPD_REGS;

    CpuSuspendIntr(&state);
    SPD_REG16(0x4208) &= ~a1;
    SPD_REG16(0x4204) = a1;
    if (!SPD_REG16(0x4208))
        dev9IntrDisable(0x200);
    CpuResumeIntr(state);
    return 0;
}

int DvrdrvRegisterIntrHandler(int a1, void *arg, void (*a3)(int, void *))
{
    int v6;
    void (**v7)(int, void *);

    BlockAPI();
    v6 = 0;
    v7 = intrhandler_callbacks;
    do {
        if (!*v7) {
            *v7 = a3;
            intrhandler_intrnum[v6] = a1;
            intrhandler_callbacksarg[v6] = arg;
            UnblockAPI();
            return 0;
        }
        ++v6;
        ++v7;
    } while (v6 < 32);
    UnblockAPI();
    return -1;
}

int DvrdrvUnregisterIntrHandler(void (*a1)(int, void *))
{
    int v2;
    void (**v3)(int, void *);

    BlockAPI();
    v2 = 0;
    v3 = intrhandler_callbacks;
    do {
        if (*v3 == a1) {
            *v3 = 0;
            intrhandler_intrnum[v2] = 0;
            UnblockAPI();
            return 0;
        }
        ++v2;
        ++v3;
    } while (v2 < 32);
    UnblockAPI();
    return -1;
}

int DvrdrvEnd()
{
    int result;

    result = -1;
    if (api_sema_id < 0)
        return result;
    result = -1;
    if (phase_sema_id < 0)
        return result;
    DvrdrvDisableIntr(0x801F);
    DvrdrvUnregisterIntrHandler((void *)INTR_DVRRDY_HANDLER);
    DvrdrvUnregisterIntrHandler((void *)INTR_CMD_ACK_HANDLER);
    DvrdrvUnregisterIntrHandler((void *)INTR_CMD_COMP_HANDLER);
    DvrdrvUnregisterIntrHandler((void *)INTR_DMAACK_HANDLER);
    DvrdrvUnregisterIntrHandler((void *)INTR_DMAEND_HANDLER);
    DeleteSema(api_sema_id);
    DeleteSema(phase_sema_id);
    api_sema_id = -1;
    phase_sema_id = -1;
    for (int i = 0; i < 32; i += 1) {
        DeleteSema(itr_sema_table[i].sema);
    }
    return 0;
}

int DVR_INTR_HANDLER(int flag)
{
    s16 v1;
    int v2;
    int *v3;
    int v4;
    void (*v5)(int, void *);
    USE_SPD_REGS;

    v1 = 0;
    v2 = 0;
    v3 = intrhandler_intrnum;
    v4 = SPD_REG16(0x4200);
    do {
        if ((*v3 & v4) != 0) {
            v5 = intrhandler_callbacks[v2];
            if (v5) {
                v5(v4, intrhandler_callbacksarg[v2]);
                v1 |= *(u16 *)v3;
            }
        }
        ++v2;
        ++v3;
    } while (v2 < 32);
    SPD_REG16(0x4204) = v1;
    return 1;
}

int INTR_DVRRDY_HANDLER(int a1, struct_dvrdrv *a2)
{
    int v2;
    bool v3;

    v2 = a2->arg1;
    v3 = a2->arg1 < 0;
    a2->arg2 |= 1u;
    if (!v3)
        iWakeupThread(v2);
    return 0;
}

int INTR_DVRRDY_TO_HANDLER(void *a1)
{
    s32 v1;

    v1 = *(u32 *)a1;
    if (v1 >= 0)
        iReleaseWaitThread(v1);
    return 0;
}

void INTR_CMD_ACK_HANDLER(int a1, void *a2)
{
    struct_itr_sid_tbl *v2;
    int v3;
    int v4;
    char *v5;
    s32 v6;
    char v7;
    USE_SPD_REGS;

    v2 = GetItrSidTbl(0, SPD_REG16(0x4220));
    if (v2) {
        v6 = v2->sema;
        v2->error |= 2u;
        if (v6 < 0)
            Kprintf("Illegal Sema ID : %d\n", v6);
        else
            iSignalSema(v6);
    } else {
        Kprintf("ACK:GetItrSidTbl(%04Xh) error\n", SPD_REG16(0x4220));
        Kprintf("Clear \"Reply FIFO\"\n");
        v3 = 64;
        if ((SPD_REG16(0x4228) & 1) == 0)
            v3 = (u8)(SPD_REG16(0x4228) & 0xFC) >> 2;
        v4 = 0;
        if (v3) {
            v5 = &v7;
            do {
                ++v4;
                *(u16 *)v5 = SPD_REG16(0x4224);
                v5 += 2;
            } while (v4 < v3);
        }
    }
}

int INTR_CMD_ACK_TO_HANDLER(struct_itr_sid_tbl *a1)
{
    unsigned int v1;
    s32 v3;
    iop_sys_clock_t clock;

    GetSystemTime(&clock);
    v1 = clock.lo;
    Kprintf("CMDACK_TO:[%u]\n", v1);
    v3 = a1->sema;
    a1->timed_out = 1;
    iSignalSema(v3);
    return 0;
}

void INTR_CMD_COMP_HANDLER(int a1, void *a2)
{
    struct_itr_sid_tbl *v2;
    int v3;
    int v4;
    char *v5;
    s32 v6;
    char v7;
    USE_SPD_REGS;

    v2 = GetItrSidTbl(2, SPD_REG16(0x4220));
    if (v2) {
        v6 = v2->sema;
        v2->error |= 4u;
        if (v6 < 0)
            Kprintf("Illegal thread ID : %d\n", v6);
        else
            iSignalSema(v6);
    } else {
        Kprintf("COMP:GetItrSidTbl(%04Xh) error\n", SPD_REG16(0x4220));
        Kprintf("Clear \"Reply FIFO\"\n");
        v3 = 64;
        if ((SPD_REG16(0x4228) & 1) == 0)
            v3 = (u8)(SPD_REG16(0x4228) & 0xFC) >> 2;
        v4 = 0;
        if (v3) {
            v5 = &v7;
            do {
                ++v4;
                *(u16 *)v5 = SPD_REG16(0x4224);
                v5 += 2;
            } while (v4 < v3);
        }
    }
}

unsigned int INTR_CMD_COMP_TO_HANDLER(struct_itr_sid_tbl *a1)
{
    unsigned int v1;
    s32 v3;
    iop_sys_clock_t clock;

    GetSystemTime(&clock);
    v1 = clock.lo;
    Kprintf("COMP TO:[%u]\n", v1);
    v3 = a1->sema;
    a1->timed_out = 1;
    iSignalSema(v3);
    return 0;
}

void INTR_DMAACK_HANDLER(int a1, void *a2)
{
    struct_itr_sid_tbl *v2;
    s32 v3;
    USE_SPD_REGS;

    v2 = GetItrSidTbl(0, SPD_REG16(0x4220));
    if (v2) {
        v3 = v2->sema;
        v2->error |= 8u;
        if (v3 < 0)
            Kprintf("Illegal thread ID : %d\n", v3);
        else
            iSignalSema(v3);
    } else {
        Kprintf("GetItrSidTbl(%04Xh) error\n", SPD_REG16(0x4220));
    }
}

unsigned int INTR_DMAACK_TO_HANDLER(struct_itr_sid_tbl *a1)
{
    unsigned int v1;
    iop_sys_clock_t clock;

    GetSystemTime(&clock);
    v1 = clock.lo;
    Kprintf("DMAACK_TO:[%u]\n", v1);
    GetItrSidTbl(0, a1->command)->timed_out = 1;
    iSignalSema(a1->sema);
    return 0;
}

void INTR_DMAEND_HANDLER(int a1, void *a2)
{
    struct_itr_sid_tbl *v2;
    s32 v3;
    USE_SPD_REGS;

    v2 = GetItrSidTbl(1, SPD_REG16(0x4220));
    if (v2) {
        v3 = v2->sema;
        v2->error |= 0x10u;
        if (v3 < 0)
            Kprintf("Illegal thread ID : %d\n", v3);
        else
            iSignalSema(v3);
    } else {
        Kprintf("GetItrSidTbl(%04Xh) error\n", SPD_REG16(0x4220));
    }
}

int BlockAPI()
{
    WaitSema(api_sema_id);
    return 0;
}

int UnblockAPI()
{
    SignalSema(api_sema_id);
    return 0;
}

struct_itr_sid_tbl *SetItrSidTbl(int itrsid_index, u16 command, int sema)
{
    int state;
    int i = 0;

    CpuSuspendIntr(&state);
    for (i = 0; i < 32; i += 1) {
        if (itrsid_table[itrsid_index][i].sema == -1 || itrsid_table[itrsid_index][i].command == command) {
            itrsid_table[itrsid_index][i].command = command;
            itrsid_table[itrsid_index][i].sema = sema;
            itrsid_table[itrsid_index][i].error = 0;
            break;
        }
    }
    CpuResumeIntr(state);
    if (i != 32)
        return &itrsid_table[itrsid_index][i];
    Kprintf("SetItrSidTbl : Error!\n");
    return 0;
}

struct_itr_sid_tbl *GetItrSidTbl(int itrsid_index, u16 command)
{
    int state;
    int i = 0;

    CpuSuspendIntr(&state);
    for (i = 0; i < 32; i += 1) {
        if (itrsid_table[itrsid_index][i].command == command) {
            break;
        }
    }
    CpuResumeIntr(state);
    if (i != 32)
        return &itrsid_table[itrsid_index][i];
    Kprintf("GetItrSidTbl : Error!\n");
    return 0;
}

int ClearItrSidTbl(struct_itr_sid_tbl *a1)
{
    int v2;
    int state;

    CpuSuspendIntr(&state);
    a1->timed_out = 0;
    v2 = state;
    a1->command = 0;
    a1->sema = -1;
    a1->error = 0;
    return CpuResumeIntr(v2);
}

struct_itr_sema *AllocItrSema()
{
    struct_itr_sema *v0;
    int v1;
    u8 *v2;
    struct_itr_sema *v3;
    struct_itr_sema *result;
    int state;

    v0 = 0;
    CpuSuspendIntr(&state);
    v1 = 0;
    v2 = &itr_sema_table[0].used;
    v3 = itr_sema_table;
    while (1) {
        ++v1;
        if (!*v2)
            break;
        v2 += 8;
        ++v3;
        if (v1 >= 32)
            goto LABEL_5;
    }
    v0 = v3;
    *v2 = 1;
LABEL_5:
    CpuResumeIntr(state);
    result = v0;
    if (v0)
        return result;
    printf("AllocItrSema : empty\n");
    return 0;
}

int ReleaseItrSema(struct_itr_sema *itrsema)
{
    int v2;
    struct_itr_sema *v3;
    int result;
    int state;

    CpuSuspendIntr(&state);
    v2 = 0;
    v3 = itr_sema_table;
    while (v3 != itrsema) {
        ++v2;
        ++v3;
        if (v2 >= 32)
            goto LABEL_4;
    }
    itr_sema_table[v2].used = 0;
LABEL_4:
    CpuResumeIntr(state);
    result = 0;
    if (v2 != 32)
        return result;
    printf("ReleaseItrSema : full\n");
    return -1;
}

int DvrdrvExecCmdAck(drvdrv_exec_cmd_ack *a1)
{
    struct_itr_sema *v2;
    u16 v3;
    int v4;
    int result;

    v2 = AllocItrSema();
    if (v2) {
        DvrdrvBlockPhase();
        v3 = a1->command;
        a1->phase = 1;
        v4 = DvrdrvSendCmdAck(
            v2,
            v3,
            a1->input_word,
            a1->input_word_count,
            &a1->status_4220_ack,
            &a1->ack_status_ack,
            &a1->status_4228_ack);
        DvrdrvUnblockPhase();
        ReleaseItrSema(v2);
        result = v4;
    } else {
        printf("DvrdrvExecCmdAck : Cannot alloc sema\n");
        result = -1;
    }
    return result;
}

int DvrdrvExecCmdAckComp(drvdrv_exec_cmd_ack *a1)
{
    struct_itr_sema *v2;
    struct_itr_sema *v3;
    int result;
    u16 v5;
    int v6;
    u16 v7;

    v2 = AllocItrSema();
    v3 = v2;
    if (v2) {
        DvrdrvPrepareWaitCmdComp(v2, a1->command, a1->timeout);
        DvrdrvBlockPhase();
        v5 = a1->command;
        a1->phase = 1;
        v6 = DvrdrvSendCmdAck(
            v3,
            v5,
            a1->input_word,
            a1->input_word_count,
            &a1->status_4220_ack,
            &a1->ack_status_ack,
            &a1->status_4228_ack);
        if (v6) {
            DvrdrvCancelWaitCmdComp(a1->command);
            DvrdrvUnblockPhase();
        } else {
            DvrdrvUnblockPhase();
            v7 = a1->command;
            a1->phase = 4;
            v6 = DvrdrvWaitCmdComp(v3, v7, &a1->status_4220_comp, &a1->comp_status, &a1->status_4228_comp);
        }
        ReleaseItrSema(v3);
        result = v6;
    } else {
        printf("DvrdrvExecCmdAck : Cannot alloc sema\n");
        result = -1;
    }
    return result;
}

int DvrdrvExecCmdAckDmaSendComp(drvdrv_exec_cmd_ack *a1)
{
    struct_itr_sema *v2;
    struct_itr_sema *v3;
    int v5;
    int v6;
    u16 v7;
    int v8;
    s16 v9;
    struct_itr_sema *v10;
    u16 v11;
    u16 v12;
    u16 v13[65];
    s32 input_word_count;
    u16 v15;
    u16 ack_status[65];
    u32 v17[37];
    int v18;

    v2 = AllocItrSema();
    v3 = v2;
    if (!v2) {
        printf("DvrdrvExecCmdAck : Cannot alloc sema\n");
        return -1;
    }
    DvrdrvPrepareWaitCmdComp(v2, a1->command, a1->timeout);
    a1->phase = 1;
    DvrdrvBlockPhase();
    v5 = DvrdrvSendCmdAck(
        v3,
        a1->command,
        a1->input_word,
        a1->input_word_count,
        &a1->status_4220_ack,
        &a1->ack_status_ack,
        &a1->status_4228_ack);
    v6 = v5;
    if (v5) {
        printf("Phase1 error -> %d\n", v5);
        v7 = a1->command;
    } else {
        DvrdrvUnblockPhase();
        a1->phase = 2;
        DvrdrvBlockPhase();
        DvrdrvSetDmaDirection(1u);
        v8 = a1->input_buffer_length;
        if ((v8 & 0x7F) != 0)
            v18 = (v8 / 128 + 1) << 7;
        else
            v18 = a1->input_buffer_length;
        v9 = a1->command;
        input_word_count = 2;
        v12 = (v9 & 0xF0FF) | 0x200;
        v13[0] = (v18 & 0xFFFF0000) >> 16;
        v13[1] = v18;
        v10 = AllocItrSema();
        DvrdrvPrepareWaitDmaEnd(v10, v12);
        v6 = DvrdrvSendCmdAck(v3, v12, v13, input_word_count, &v15, ack_status, v17);
        if (v6) {
            v7 = a1->command;
        } else {
            v6 = -1;
            if (!ack_status[0]) {
                if (v18 <= 0) {
                    DvrdrvCancelWaitDmaEnd(v12);
                } else {
                    DvrdrvTransferDma(a1->input_buffer, v18);
                    DvrdrvWaitDmaEnd(v10, v12);
                }
                ReleaseItrSema(v10);
                DvrdrvUnblockPhase();
                v11 = a1->command;
                a1->phase = 4;
                v6 = DvrdrvWaitCmdComp(v3, v11, &a1->status_4220_comp, &a1->comp_status, &a1->status_4228_comp);
                goto LABEL_17;
            }
            v7 = a1->command;
        }
    }
    DvrdrvCancelWaitCmdComp(v7);
    DvrdrvUnblockPhase();
LABEL_17:
    ReleaseItrSema(v3);
    return v6;
}

int DvrdrvExecCmdAckDmaRecvComp(drvdrv_exec_cmd_ack *a1)
{
    struct_itr_sema *v2;
    struct_itr_sema *v3;
    int v5;
    u16 v6;
    s16 v7;
    struct_itr_sema *v8;
    int v9;
    u16 v10;
    u16 v11;
    u16 v12[65];
    s32 input_word_count;
    u16 v14;
    u16 ack_status[65];
    u32 v16[41];

    v2 = AllocItrSema();
    v3 = v2;
    if (!v2) {
        printf("DvrdrvExecCmdAck : Cannot alloc sema\n");
        return -1;
    }
    DvrdrvPrepareWaitCmdComp(v2, a1->command, a1->timeout);
    a1->phase = 1;
    DvrdrvBlockPhase();
    v5 = DvrdrvSendCmdAck(
        v3,
        a1->command,
        a1->input_word,
        a1->input_word_count,
        &a1->status_4220_ack,
        &a1->ack_status_ack,
        &a1->status_4228_ack);
    if (v5) {
        v6 = a1->command;
    } else {
        DvrdrvUnblockPhase();
        a1->phase = 3;
        DvrdrvBlockPhase();
        DvrdrvSetDmaDirection(0);
        v7 = a1->command;
        input_word_count = 0;
        v11 = (v7 & 0xF0FF) | 0x300;
        v8 = AllocItrSema();
        DvrdrvPrepareWaitDmaEnd(v8, v11);
        v5 = DvrdrvSendCmdAck(v3, v11, v12, input_word_count, &v14, ack_status, v16);
        if (v5) {
            v6 = a1->command;
        } else {
            v5 = -1;
            if (!ack_status[0]) {
                v9 = (ack_status[1] << 16) + ack_status[2];
                a1->ack_status_ack2 = v9;
                if (v9 <= 0) {
                    DvrdrvCancelWaitDmaEnd(v11);
                } else {
                    DvrdrvTransferDma((u8 *)a1->output_buffer, v9);
                    DvrdrvWaitDmaEnd(v8, v11);
                }
                ReleaseItrSema(v8);
                DvrdrvUnblockPhase();
                v10 = a1->command;
                a1->phase = 4;
                v5 = DvrdrvWaitCmdComp(v3, v10, &a1->status_4220_comp, &a1->comp_status, &a1->status_4228_comp);
                goto LABEL_14;
            }
            v6 = a1->command;
        }
    }
    DvrdrvCancelWaitCmdComp(v6);
    DvrdrvUnblockPhase();
LABEL_14:
    ReleaseItrSema(v3);
    return v5;
}

int DvrdrvExecCmdAckDma2Comp(drvdrv_exec_cmd_ack *a1)
{
    struct_itr_sema *v2;
    struct_itr_sema *v3;
    int v5;
    int v6;
    s16 v7;
    struct_itr_sema *v8;
    u16 v9;
    s16 v10;
    struct_itr_sema *v11;
    int v12;
    u16 v13;
    u16 v14;
    u16 v15[65];
    s32 input_word_count;
    u16 v17;
    u16 ack_status[65];
    u32 v19[37];
    int v20;
    u16 v21;
    u16 v22[65];
    s32 v23;
    u16 v24;
    u16 v25[65];
    u32 v26[41];

    v2 = AllocItrSema();
    v3 = v2;
    if (!v2) {
        printf("DvrdrvExecCmdAck : Cannot alloc sema\n");
        return -1;
    }
    DvrdrvPrepareWaitCmdComp(v2, a1->command, a1->timeout);
    a1->phase = 1;
    DvrdrvBlockPhase();
    v5 = DvrdrvSendCmdAck(
        v3,
        a1->command,
        a1->input_word,
        a1->input_word_count,
        &a1->status_4220_ack,
        &a1->ack_status_ack,
        &a1->status_4228_ack);
    if (v5)
        goto LABEL_15;
    DvrdrvUnblockPhase();
    a1->phase = 2;
    DvrdrvBlockPhase();
    DvrdrvSetDmaDirection(1u);
    v6 = a1->input_buffer_length;
    v20 = (v6 & 0x7F) != 0 ? (v6 / 128 + 1) << 7 : a1->input_buffer_length;
    v7 = a1->command;
    input_word_count = 2;
    v14 = (v7 & 0xF0FF) | 0x200;
    v15[0] = (v20 & 0xFFFF0000) >> 16;
    v15[1] = v20;
    v8 = AllocItrSema();
    DvrdrvPrepareWaitDmaEnd(v8, v14);
    v5 = DvrdrvSendCmdAck(v3, v14, v15, input_word_count, &v17, ack_status, v19);
    if (v5) {
    LABEL_15:
        v9 = a1->command;
        goto LABEL_16;
    }
    v5 = -1;
    if (!ack_status[0]) {
        if (v20 <= 0) {
            DvrdrvCancelWaitDmaEnd(v14);
        } else {
            DvrdrvTransferDma(a1->input_buffer, v20);
            DvrdrvWaitDmaEnd(v8, v14);
        }
        ReleaseItrSema(v8);
        DvrdrvUnblockPhase();
        a1->phase = 3;
        DvrdrvBlockPhase();
        DvrdrvSetDmaDirection(0);
        v10 = a1->command;
        v23 = 0;
        v21 = (v10 & 0xF0FF) | 0x300;
        v11 = AllocItrSema();
        DvrdrvPrepareWaitDmaEnd(v11, v21);
        v5 = DvrdrvSendCmdAck(v3, v21, v22, v23, &v24, v25, v26);
        if (!v5 && !v25[0]) {
            v12 = (v25[1] << 16) + v25[2];
            a1->ack_status_ack2 = v12;
            if (v12 <= 0) {
                DvrdrvCancelWaitDmaEnd(v21);
            } else {
                DvrdrvTransferDma((u8 *)a1->output_buffer, v12);
                DvrdrvWaitDmaEnd(v11, v21);
            }
            ReleaseItrSema(v11);
            DvrdrvUnblockPhase();
            v13 = a1->command;
            a1->phase = 4;
            v5 = DvrdrvWaitCmdComp(v3, v13, &a1->status_4220_comp, &a1->comp_status, &a1->status_4228_comp);
            goto LABEL_21;
        }
        goto LABEL_15;
    }
    v9 = a1->command;
LABEL_16:
    DvrdrvCancelWaitCmdComp(v9);
    DvrdrvUnblockPhase();
LABEL_21:
    ReleaseItrSema(v3);
    return v5;
}
