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

extern int module_start();
extern int module_stop();
extern int dvrdv_df_init(iop_device_t *dev);
extern int dvrdv_df_exit(iop_device_t *dev);
extern int dvrdv_df_ioctl(iop_file_t *f, int cmd, void *param);
extern int dvrdv_df_devctl(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrdv_df_ioctl2(iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrdv_df_null();
extern s64 dvrdv_df_null_long();
extern int dvrioctl2_dv_dubb_start(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_dv_dubb_stop(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_dv_dubb_rec_start(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_dv_dubb_rec_stop(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_dvcam_info(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_dvcam_name(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

struct DevctlCmdTbl_t
{
    u16 cmd;
    int (*fn)(iop_file_t *, const char *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[6] =
    {
        {0x5601, &dvrioctl2_dv_dubb_start},
        {0x5602, &dvrioctl2_dv_dubb_stop},
        {0x5603, &dvrioctl2_dv_dubb_rec_start},
        {0x5604, &dvrioctl2_dv_dubb_rec_stop},
        {0x5607, &dvrioctl2_get_dvcam_info},
        {0x5608, &dvrioctl2_get_dvcam_name},
};
struct _iop_device_ops DvrFuncTbl =
    {
        &dvrdv_df_init,
        &dvrdv_df_exit,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_ioctl,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_null_long,
        &dvrdv_df_devctl,
        &dvrdv_df_null,
        &dvrdv_df_null,
        &dvrdv_df_ioctl2};
iop_device_t DVR;
s32 sema_id;

// Based off of DESR / PSX DVR system software version 2.11.
#define MODNAME "DVRDV"
IRX_ID(MODNAME, 1, 0);

int _start(int a1, char **argv)
{
    if (a1 >= 0)
        return module_start();
    else
        return module_stop();
}

int module_start()
{
    int i;
    bool v1;
    int result;

    for (i = 0; i < 30000; ++i) {
        if (((*((vu32 *)0xB0004230)) & 4) != 0)
            break;
        DelayThread(1000);
    }
    if (i == 30000) {
        printf("DVR task of DVRP is not running...\n");
        return 1;
    } else {
        DVR.name = "dvr_dv";
        DVR.desc = "Digital Video Recorder DVR task";
        DVR.type = 0x10000010;
        DVR.ops = &DvrFuncTbl;
        v1 = AddDrv(&DVR) == 0;
#if 0
        result = 2;
#else
        result = 0;
#endif
        if (!v1)
            return 1;
    }
    return result;
}

int module_stop()
{
    bool v0;
    int result;

    v0 = DelDrv("dvr_dv") == 0;
    result = 1;
    if (!v0)
        return 2;
    return result;
}

int dvrdv_df_init(iop_device_t *dev)
{
    int v1;
    iop_sema_t v3;

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

int dvrdv_df_exit(iop_device_t *dev)
{
    bool v1;
    int result;

    v1 = DeleteSema(sema_id) == 0;
    result = 0;
    if (!v1)
        return -1;
    return result;
}

int dvrdv_df_ioctl(iop_file_t *f, int cmd, void *param)
{
    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvrdv_df_devctl(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v10;
    unsigned int v11;
    unsigned int v12;

    v10 = 0;
    v11 = 0;
    WaitSema(sema_id);
    v12 = 0;
    while (DevctlCmdTbl[v12].cmd != cmd) {
        v12 = ++v11;
        if (v11 >= sizeof(DevctlCmdTbl) / sizeof(DevctlCmdTbl[0]))
            goto LABEL_5;
    }
    v10 = DevctlCmdTbl[v12].fn(a1, name, cmd, arg, arglen, buf, buflen);
LABEL_5:
    if (v11 == sizeof(DevctlCmdTbl) / sizeof(DevctlCmdTbl[0]))
        v10 = -22;
    SignalSema(sema_id);
    return v10;
}

int dvrdv_df_ioctl2(
    iop_file_t *f,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvrdv_df_null()
{
    return -48;
}

s64 dvrdv_df_null_long()
{
    return -48LL;
}

int dvrioctl2_dv_dubb_start(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    printf("------------------------------ dv dubb start!!!\n");
    cmdack.command = 0x4101;
    cmdack.input_word_count = 0;
    cmdack_err = DvrdrvExecCmdAck(&cmdack);
    if (cmdack_err) {
        printf("dvrioctl2_dv_dubb_start -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_dv_dubb_start -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_dv_dubb_stop(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    printf("------------------------------ dv dubb stop!!!\n");
    cmdack.command = 0x4102;
    cmdack.input_word_count = 0;
    cmdack_err = DvrdrvExecCmdAck(&cmdack);
    if (cmdack_err) {
        printf("dvrioctl2_dv_dubb_stop -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_dv_dubb_stop -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_dv_dubb_rec_start(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v8;
    u16 *v9;
    u16 *v10;
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x4103;
    printf("------------------------------ dv dubb rec start!!!\n");
    v8 = 4;
    v9 = (u16 *)((u8 *)arg + 8);
    v10 = &cmdack.input_word[4];
    do {
        *v10 = *v9;
        v9 += 1;
        v8 += 1;
        v10 += 1;
    } while (v8 < 7);
    cmdack.input_word[0] = *((u16 *)arg + 1);
    cmdack.input_word[1] = *(u16 *)arg;
    cmdack.input_word[2] = *((u16 *)arg + 3);
    cmdack.input_word[3] = *((u16 *)arg + 2);
    cmdack.input_word_count = 7;
    cmdack_err = DvrdrvExecCmdAck(&cmdack);
    if (cmdack_err) {
        printf("dvrioctl2_dv_dubb_rec_start -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_dv_dubb_rec_start -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_dv_dubb_rec_stop(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    printf("------------------------------ dv dubb rec stop!!!\n");
    cmdack.command = 0x4104;
    cmdack.input_word_count = 0;
    cmdack.timeout = 5000000;
    cmdack_err = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdack_err) {
        printf("phase %d\n", cmdack.phase);
        printf("dvrioctl2_dv_dubb_rec_stop -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            printf("phase %d\n", cmdack.phase);
            printf("dvrioctl2_dv_dubb_rec_stop -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_get_dvcam_info(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    int cpy_cnt;
    u16 *v11;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x4107;
    cmdack.input_word_count = 0;
    cmdack_err = DvrdrvExecCmdAck(&cmdack);
    if (cmdack_err) {
        printf("phase %d\n", cmdack.phase);
        printf("dvrioctl2_get_dvcam_info -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        cpy_cnt = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_get_dvcam_info -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        } else {
            v11 = &cmdack.output_word[0];
            do {
                cpy_cnt += 1;
                *(u16 *)buf = *v11;
                buf = (char *)buf + 2;
                v11 += 1;
            } while (cpy_cnt < 4);
            return 0;
        }
    }
}

int dvrioctl2_get_dvcam_name(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    printf("dvrioctl2_get_dvcam_name(io=%p, cmd=%d  buf=%p, nbyte=%u)\n", a1, cmd, buf, buflen);
    cmdack.command = 0x4108;
    cmdack.input_word[0] = 0;
    cmdack.input_word[1] = 0;
    cmdack.input_word[2] = (buflen & 0xFFFF0000) >> 16;
    cmdack.input_word[3] = buflen;
    cmdack.input_word_count = 4;
    cmdack.output_buffer = buf;
    cmdack.timeout = 5000000;
    if (DvrdrvExecCmdAckDmaRecvComp(&cmdack)) {
        printf("dvrioctl2_get_dvcam_name : IO error (phase %d)\n", cmdack.phase);
        return -5;
    } else {
        if (cmdack.comp_status) {
            printf("dvrioctl2_get_dvcam_name : Complete parameter error (phase %d), %04X\n", cmdack.phase, cmdack.comp_status);
            return -5;
        }
    }
    return 0;
}
