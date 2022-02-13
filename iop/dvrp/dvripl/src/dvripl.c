/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2021-2021, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdbool.h>
#include "iomanX.h"
#include "pvrdrv.h"
#include "stdio.h"
#include "thbase.h"
#include "thsemap.h"
#include "speedregs.h"
#include "errno.h"
#include "timrman.h"
#include "sysmem.h"

extern int module_start();
extern int module_stop();
extern int dvripl_df_init(iop_device_t *dev);
extern int dvripl_df_exit(iop_device_t *dev);
extern int dvripl_df_ioctl(iop_file_t *f, int cmd, void *param);
extern int dvripl_df_devctl(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvripl_df_ioctl2(iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvripl_df_null();
extern s64 dvripl_df_null_long();
extern int iplioctl2_update(iop_file_t *a1, int cmd, void *arg);
extern void dvr_ready(int a1, void *a2);

struct _iop_device_ops DvrFuncTbl =
    {
        &dvripl_df_init,
        &dvripl_df_exit,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_ioctl,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_null_long,
        &dvripl_df_devctl,
        &dvripl_df_null,
        &dvripl_df_null,
        &dvripl_df_ioctl2};
s32 dvr_ready_flag;
iop_device_t DVRMAN;
s32 sema_id;
char SBUF[32768];

// Based off of DESR / PSX DVR system software version 1.31.
#define MODNAME "DVRIPL"
IRX_ID(MODNAME, 1, 1);

int _start(int a1, char **argv)
{
    if (a1 >= 0)
        return module_start();
    else
        return module_stop();
}

int module_start()
{
    bool v0;

    DVRMAN.name = "dvr_ipl";
    DVRMAN.desc = "Digital Video Recorder";
    DVRMAN.ops = &DvrFuncTbl;
    DVRMAN.type = 0x10000010;
    v0 = AddDrv(&DVRMAN) == 0;
    if (!v0)
        return 1;
#if 0
    return 2;
#else
    return 0;
#endif
}

int module_stop()
{
    bool v0;

    v0 = DelDrv("dvr_ipl") == 0;
    if (!v0)
        return 2;
    return 1;
}

int dvripl_df_init(iop_device_t *dev)
{
    int v1;
    iop_sema_t v3;

    printf("dvripl_df_init\n");
    v3.attr = 0;
    v3.initial = 1;
    v3.max = 1;
    v3.option = 0;
    v1 = CreateSema(&v3);
    if (v1 < 0)
        return -1;
    sema_id = v1;
    return 0;
}

int dvripl_df_exit(iop_device_t *dev)
{
    bool v1;

    printf("dvripl_df_exit\n");
    v1 = DeleteSema(sema_id) == 0;
    if (!v1)
        return -1;
    return 0;
}

int dvripl_df_ioctl(iop_file_t *f, int cmd, void *param)
{
    printf("dvripl_df_ioctl\n");
    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvripl_df_devctl(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    bool v10;
    int v11;

    printf("dvripl_df_devctl\n");
    WaitSema(sema_id);
    v10 = cmd != 0x5602;
    v11 = -22;
    if (!v10)
        v11 = iplioctl2_update(a1, 0x5602, arg);
    SignalSema(sema_id);
    return v11;
}

int dvripl_df_ioctl2(iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    printf("dvripl_df_ioctl2\n");
    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvripl_df_null()
{
    return -48;
}

s64 dvripl_df_null_long()
{
    return -48LL;
}

int iplioctl2_update(iop_file_t *a1, int cmd, void *arg)
{
    int total_size;
    int retval;
    int csum;
    int cmdackerr1;
    int cmdackerr2;
    int cmdackerr3;
    int update_fd;
#if 0
    int hard_timer;
    u32 system_clock;
#endif
    int cmdackerr4;
    drvdrv_exec_cmd_ack cmdack;

    total_size = 0;
    retval = 0;
    csum = 0;
    printf("iplioctl2_update\n");
    printf("NOP\n");
    cmdack.command = 0x101;
    cmdack.input_word_count = 0;
    cmdackerr1 = DvrdrvExecCmdAck(&cmdack);
    printf("dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
    if (cmdackerr1)
        goto LABEL_2;
    if (cmdack.ack_status_ack) {
        printf("NOP -> Status error!\n");
        return -5;
    }
    printf("VERSION\n");
    cmdack.command = 0x102;
    cmdack.input_word_count = 0;
    cmdackerr2 = DvrdrvExecCmdAck(&cmdack);
    printf("dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
    if (cmdackerr2) {
    LABEL_2:
        printf("NOP -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack) {
        printf("NOP -> Status error!\n");
        return -5;
    }
    printf("major : %04x\n", cmdack.output_word[0]);
    printf("minor : %04x\n", cmdack.output_word[1]);
    printf("CONFIG\n");
    cmdack.command = 0x106;
    cmdack.input_word[0] = 1;
    cmdack.input_word[1] = 6;
    cmdack.input_word[2] = 0x1000;
    cmdack.input_word[3] = 0x8968;
    cmdack.input_word[4] = 0x115A;
    cmdack.input_word[5] = 0x6048;
    cmdack.input_word[6] = 0xF;
    cmdack.input_word[7] = 0x5353;
    cmdack.input_word_count = 8;
    cmdackerr3 = DvrdrvExecCmdAck(&cmdack);
    printf("dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
    if (cmdackerr3) {
        printf("CONFIG -> Handshake error!(%d)\n", cmdackerr3);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        printf("CONFIG -> Status error!\n");
        return -5;
    }
    update_fd = open((const char *)arg, 1, 0x124);
    if (update_fd >= 0) {
        int chunk_offset;
        chunk_offset = 0x10000000;
        printf("Opened \"%s\"\n", (const char *)arg);
        printf("Downloading  \"%s\"\n", (const char *)arg);
#if 0
        hard_timer = AllocHardTimer(1, 32, 1);
        SetupHardTimer(hard_timer, 1, 0, 1);
        StartHardTimer(hard_timer);
#endif
        while (1) {
            int read_buf_offs;
            int *read_buf;
            s32 chunk_size;
            int read_size;
            printf("%08X\n", chunk_offset);
            read_size = read(update_fd, SBUF, 0x8000);
            chunk_size = read_size;
            if (read_size < 0) {
                retval = -5;
                printf("Cannot read \"%s\"\n", (const char *)arg);
                goto LABEL_30;
            }
            read_buf = (int *)SBUF;
            if (!read_size)
                break;
            read_buf_offs = 0;
            while (read_buf_offs++ < chunk_size / 4) {
                unsigned int read_buf_tmp;
                read_buf_tmp = *read_buf++;
                csum += (read_buf_tmp << 24) + ((read_buf_tmp & 0xFF00) << 8) + ((read_buf_tmp & 0xFF0000) >> 8) + ((read_buf_tmp & 0xff000000) >> 24);
            }
            cmdack.command = 0x103;
            cmdack.input_word[0] = chunk_offset >> 16;
            cmdack.input_word[1] = chunk_offset;
            cmdack.input_word_count = 2;
            cmdack.input_buffer = SBUF;
            cmdack.input_buffer_length = chunk_size;
            if (DvrdrvExecCmdAckDmaSendComp(&cmdack)) {
                retval = -5;
                printf("Handshake error! (phase:%d)\n", cmdack.phase);
                goto LABEL_30;
            }
            chunk_offset += chunk_size;
            if (cmdack.ack_status_ack)
                goto LABEL_29;
            total_size += chunk_size;
        }
#if 0
        system_clock = GetTimerCounter(hard_timer);
        printf("System Clock : %ld\n", system_clock);
        StopHardTimer(hard_timer);
        FreeHardTimer(hard_timer);
#endif
        printf("CHECK SUM\n");
        printf("total_size:%d\n", total_size);
        printf("csum : %x\n", csum);
        cmdack.command = 0x105;
        cmdack.input_word[0] = 0x1000;
        cmdack.input_word[2] = total_size >> 16;
        cmdack.input_word[4] = csum >> 16;
        cmdack.input_word[1] = 0;
        cmdack.input_word[3] = total_size;
        cmdack.input_word[5] = csum;
        cmdack.input_word_count = 6;
        cmdackerr4 = DvrdrvExecCmdAck(&cmdack);
        printf("result: %d\n", cmdackerr4);
        printf("dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
        printf("dvrcmd.ack_p[1]:%x\n", cmdack.output_word[0]);
        printf("dvrcmd.ack_p[2]:%x\n", cmdack.output_word[1]);
        if (cmdackerr4) {
            retval = -5;
            goto LABEL_30;
        }
        if (cmdack.ack_status_ack)
        LABEL_29:
            retval = -5;
    } else {
        retval = -89;
    }
LABEL_30:
    close(update_fd);
    DvrdrvUnregisterIntrHandler(dvr_ready);
    printf("done.\n");
    return retval;
}

void dvr_ready(int a1, void *a2)
{
    Kprintf("DVRRDY INTERRUPT\n");
    dvr_ready_flag = 1;
    iWakeupThread(*(u32 *)a2);
}
