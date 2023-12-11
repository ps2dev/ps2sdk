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
#include "loadcore.h"
#include "pvrdrv.h"
#include "stdio.h"
#include "thbase.h"
#include "thsemap.h"
#include "speedregs.h"
#include "errno.h"

#define MODNAME "DVRDV"
#ifdef DEBUG
#define DPRINTF(x...) printf(MODNAME ": " x)
#else
#define DPRINTF(x...)
#endif

extern int module_start();
extern int module_stop();
extern int dvrdv_df_init(iomanX_iop_device_t *dev);
extern int dvrdv_df_exit(iomanX_iop_device_t *dev);
extern int dvrdv_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param);
extern int dvrdv_df_devctl(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrdv_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrdv_df_null();
extern s64 dvrdv_df_null_long();
extern int dvrioctl2_dv_dubb_start(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_dv_dubb_stop(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_dv_dubb_rec_start(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_dv_dubb_rec_stop(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_dvcam_info(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_dvcam_name(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

struct DevctlCmdTbl_t
{
    u16 cmd;
    int (*fn)(iomanX_iop_file_t *, const char *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[6] =
    {
        {0x5601, &dvrioctl2_dv_dubb_start},
        {0x5602, &dvrioctl2_dv_dubb_stop},
        {0x5603, &dvrioctl2_dv_dubb_rec_start},
        {0x5604, &dvrioctl2_dv_dubb_rec_stop},
        {0x5607, &dvrioctl2_get_dvcam_info},
        {0x5608, &dvrioctl2_get_dvcam_name},
};
static iomanX_iop_device_ops_t DvrFuncTbl =
    {
        &dvrdv_df_init,
        &dvrdv_df_exit,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        &dvrdv_df_ioctl,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null_long,
        &dvrdv_df_devctl,
        (void *)&dvrdv_df_null,
        (void *)&dvrdv_df_null,
        &dvrdv_df_ioctl2,
    };
static iomanX_iop_device_t DVR = {
    .name = "dvr_dv",
    .desc = "Digital Video Recorder DVR task",
    .type = (IOP_DT_FS | IOP_DT_FSEXT),
    .ops = &DvrFuncTbl,
};
s32 sema_id;

// Based off of DESR / PSX DVR system software version 2.11.
IRX_ID(MODNAME, 1, 0);

int _start(int argc, char *argv[])
{
    (void)argv;

    if (argc >= 0)
        return module_start();
    else
        return module_stop();
}

int module_start()
{
    int i;

    for (i = 0; i < 30000; ++i) {
        if (((*((vu32 *)0xB0004230)) & 4) != 0)
            break;
        DelayThread(1000);
    }
    if (i == 30000) {
        DPRINTF("DVR task of DVRP is not running...\n");
        return MODULE_NO_RESIDENT_END;
    } else {
        if (iomanX_AddDrv(&DVR) != 0)
            return MODULE_NO_RESIDENT_END;
    }
#if 0
    return MODULE_REMOVABLE_END;
#else
    return MODULE_RESIDENT_END;
#endif
}

int module_stop()
{
    if (iomanX_DelDrv(DVR.name) != 0)
        return MODULE_REMOVABLE_END;
    return MODULE_NO_RESIDENT_END;
}

int dvrdv_df_init(iomanX_iop_device_t *dev)
{
    int v1;
    iop_sema_t v3;

    (void)dev;

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

int dvrdv_df_exit(iomanX_iop_device_t *dev)
{
    (void)dev;

    if (DeleteSema(sema_id) != 0)
        return -1;
    return 0;
}

int dvrdv_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param)
{
    (void)f;
    (void)cmd;
    (void)param;

    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvrdv_df_devctl(
    iomanX_iop_file_t *a1,
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
    iomanX_iop_file_t *f,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{

    (void)f;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

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
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    DPRINTF("------------------------------ dv dubb start!!!\n");
    cmdack.command = 0x4101;
    cmdack.input_word_count = 0;
    cmdack_err = DvrdrvExecCmdAck(&cmdack);
    if (cmdack_err) {
        DPRINTF("dvrioctl2_dv_dubb_start -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_dv_dubb_start -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_dv_dubb_stop(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    DPRINTF("------------------------------ dv dubb stop!!!\n");
    cmdack.command = 0x4102;
    cmdack.input_word_count = 0;
    cmdack_err = DvrdrvExecCmdAck(&cmdack);
    if (cmdack_err) {
        DPRINTF("dvrioctl2_dv_dubb_stop -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_dv_dubb_stop -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_dv_dubb_rec_start(
    iomanX_iop_file_t *a1,
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

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x4103;
    DPRINTF("------------------------------ dv dubb rec start!!!\n");
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
        DPRINTF("dvrioctl2_dv_dubb_rec_start -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_dv_dubb_rec_start -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_dv_dubb_rec_stop(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    DPRINTF("------------------------------ dv dubb rec stop!!!\n");
    cmdack.command = 0x4104;
    cmdack.input_word_count = 0;
    cmdack.timeout = 5000000;
    cmdack_err = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdack_err) {
        DPRINTF("phase %d\n", cmdack.phase);
        DPRINTF("dvrioctl2_dv_dubb_rec_stop -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("phase %d\n", cmdack.phase);
            DPRINTF("dvrioctl2_dv_dubb_rec_stop -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_get_dvcam_info(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x4107;
    cmdack.input_word_count = 0;
    cmdack_err = DvrdrvExecCmdAck(&cmdack);
    if (cmdack_err) {
        DPRINTF("phase %d\n", cmdack.phase);
        DPRINTF("dvrioctl2_get_dvcam_info -> Handshake error!,%d\n", cmdack_err);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_get_dvcam_info -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        } else {
            u16 *v11;
            int cpy_cnt;
            v11 = &cmdack.output_word[0];
            cpy_cnt = 0;
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
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)name;
    (void)arg;
    (void)arglen;

    DPRINTF("dvrioctl2_get_dvcam_name(io=%p, cmd=%d  buf=%p, nbyte=%u)\n", a1, cmd, buf, buflen);
    cmdack.command = 0x4108;
    cmdack.input_word[0] = 0;
    cmdack.input_word[1] = 0;
    cmdack.input_word[2] = (buflen & 0xFFFF0000) >> 16;
    cmdack.input_word[3] = buflen;
    cmdack.input_word_count = 4;
    cmdack.output_buffer = buf;
    cmdack.timeout = 5000000;
    if (DvrdrvExecCmdAckDmaRecvComp(&cmdack)) {
        DPRINTF("dvrioctl2_get_dvcam_name : IO error (phase %d)\n", cmdack.phase);
        return -5;
    } else {
        if (cmdack.comp_status) {
            DPRINTF("dvrioctl2_get_dvcam_name : Complete parameter error (phase %d), %04X\n", cmdack.phase, cmdack.comp_status);
            return -5;
        }
    }
    return 0;
}
