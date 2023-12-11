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
#include "sysclib.h"
#include "thbase.h"
#include "thsemap.h"
#include "speedregs.h"
#include "errno.h"

#define MODNAME "DVR"
#ifdef DEBUG
#define DPRINTF(x...) printf(MODNAME ": " x)
#else
#define DPRINTF(x...)
#endif

extern int module_start();
extern int module_stop();
extern int dvr_df_init(iomanX_iop_device_t *dev);
extern int dvr_df_exit(iomanX_iop_device_t *dev);
extern int dvr_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param);
extern int dvr_df_devctl(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvr_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvr_df_null();
extern s64 dvr_df_null_long();
extern int dvrioctl2_rec_start(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_rec_pause(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_rec_stop(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_set_rec_end_time(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_rec_info(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_rec_time(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_read_resfile(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_clear_resfile_flag(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_rec_prohibit(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_status_register(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_ifo_time_entry(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_ifo_vobu_entry(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_epg_test(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_send_timer_event(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_epg_cancel(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_clr(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_send(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_trans_dvrp(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_start_hdd_test(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_stop_hdd_test(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_hdd_test_stat(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_pre_update_a(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_pre_update_b(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_rec_vro_pckn(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_enc_dec_test(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_recv_first(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_recv_next(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_finish_auto_process(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_make_menu(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_re_enc_start(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_rec_pictclip(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrpAuthEnc(u16);

// The following has been excluded.
unsigned char dvrpAuth_tbl[256] = {0x00};

struct DevctlCmdTbl_t
{
    u16 cmd;
    int (*fn)(iomanX_iop_file_t *, const char *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[31] =
    {
        {0x5663, &dvrioctl2_get_status_register},
        {0x5638, &dvrioctl2_get_ifo_time_entry},
        {0x5639, &dvrioctl2_get_ifo_vobu_entry},
        {0x5632, &dvrioctl2_rec_start},
        {0x5633, &dvrioctl2_rec_pause},
        {0x5634, &dvrioctl2_rec_stop},
        {0x5635, &dvrioctl2_set_rec_end_time},
        {0x5636, &dvrioctl2_get_rec_info},
        {0x5637, &dvrioctl2_get_rec_time},
        {0x563A, &dvrioctl2_read_resfile},
        {0x563B, &dvrioctl2_clear_resfile_flag},
        {0x563F, &dvrioctl2_rec_prohibit},
        {0x5640, &dvrioctl2_epg_test},
        {0x5641, &dvrioctl2_send_timer_event},
        {0x5642, &dvrioctl2_epg_cancel},
        {0x565A, &dvrioctl2_tevent_buf_clr},
        {0x565B, &dvrioctl2_tevent_buf_send},
        {0x565C, &dvrioctl2_tevent_buf_trans_dvrp},
        {0x5643, &dvrioctl2_start_hdd_test},
        {0x5644, &dvrioctl2_stop_hdd_test},
        {0x5645, &dvrioctl2_get_hdd_test_stat},
        {0x5646, &dvrioctl2_pre_update_a},
        {0x5647, &dvrioctl2_pre_update_b},
        {0x5648, &dvrioctl2_get_rec_vro_pckn},
        {0x5649, &dvrioctl2_enc_dec_test},
        {0x565D, &dvrioctl2_tevent_buf_recv_first},
        {0x565E, &dvrioctl2_tevent_buf_recv_next},
        {0x5651, &dvrioctl2_finish_auto_process},
        {0x564A, &dvrioctl2_make_menu},
        {0x564B, &dvrioctl2_re_enc_start},
        {0x5650, &dvrioctl2_rec_pictclip},
};

static iomanX_iop_device_ops_t DvrFuncTbl =
    {
        &dvr_df_init,
        &dvr_df_exit,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        &dvr_df_ioctl,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null_long,
        &dvr_df_devctl,
        (void *)&dvr_df_null,
        (void *)&dvr_df_null,
        &dvr_df_ioctl2,
    };
char TEVENT_BUF[6144];
char *tevent_p;
int tevent_data_sz;
static iomanX_iop_device_t DVR = {
    .name = "dvr",
    .desc = "Digital Video Recorder DVR task",
    .type = (IOP_DT_FS | IOP_DT_FSEXT),
    .ops = &DvrFuncTbl,
};
s32 sema_id;

// Based off of DESR / PSX DVR system software version 1.31.
// Added additional functions from DESR / PSX DVR system software version 2.11.
IRX_ID(MODNAME, 1, 1);

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
        if (((*((vu32 *)0xB0004230)) & 0x10) != 0)
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

int dvr_df_init(iomanX_iop_device_t *dev)
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

int dvr_df_exit(iomanX_iop_device_t *dev)
{
    (void)dev;

    if (DeleteSema(sema_id) != 0)
        return -1;
    return 0;
}

int dvr_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param)
{
    (void)f;
    (void)cmd;
    (void)param;

    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvr_df_devctl(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v11;
    unsigned int v12;
    unsigned int v13;

    v11 = 0;
    v12 = 0;
    WaitSema(sema_id);
    v13 = 0;
    while (DevctlCmdTbl[v13].cmd != cmd) {
        v13 = ++v12;
        if (v12 >= sizeof(DevctlCmdTbl) / sizeof(DevctlCmdTbl[0]))
            goto LABEL_5;
    }
    v11 = DevctlCmdTbl[v13].fn(a1, name, cmd, arg, arglen, buf, buflen);
LABEL_5:
    if (v12 == sizeof(DevctlCmdTbl) / sizeof(DevctlCmdTbl[0]))
        v11 = -22;
    SignalSema(sema_id);
    return v11;
}

int dvr_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
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

int dvr_df_null()
{
    return -48;
}

s64 dvr_df_null_long()
{
    return -48LL;
}

int dvrioctl2_rec_start(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int busywait;
    int cmdackerr;
#ifdef DEBUG
    int err;
#endif
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2101;
    cmdack.input_word[0] = *((u16 *)arg + 1);
    cmdack.input_word[1] = *(u16 *)arg;
    cmdack.input_word[2] = *((u16 *)arg + 3);
    cmdack.input_word[3] = *((u16 *)arg + 2);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word[5] = *((u16 *)arg + 5);
    cmdack.input_word[6] = *((u16 *)arg + 6);
    busywait = 0x4;
    while (busywait-- >= 0)
        ;
    cmdack.input_word_count = 7;
    cmdack.timeout = 5000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_rec_start -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
#ifdef DEBUG
    err = cmdack.ack_status_ack;
#endif
    if (!cmdack.ack_status_ack) {
        if (!cmdack.comp_status)
            return 0;
#ifdef DEBUG
        err = cmdack.comp_status;
#endif
    }
    DPRINTF("dvrioctl2_rec_start -> Status error!,%04X\n", err);
    return -68;
}

int dvrioctl2_rec_pause(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    drvdrv_exec_cmd_ack *p_cmdack;
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    v7 = 0;
    p_cmdack = &cmdack;
    cmdack.command = 0x2102;
    do {
        ++v7;
        p_cmdack->input_word[0] = *(u16 *)arg;
        arg = (char *)arg + 2;
        p_cmdack = (drvdrv_exec_cmd_ack *)((char *)p_cmdack + 2);
    } while (v7 <= 0);
    cmdack.input_word_count = 1;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_rec_pause -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_rec_pause -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_rec_stop(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2103;
    cmdack.input_word_count = 0;
    cmdack.timeout = 5000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_rec_stop -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_rec_stop -> Status error!,%04X\n", cmdack.ack_status_ack);
    } else {
        if (!cmdack.comp_status)
            return 0;
        DPRINTF("dvrioctl2_rec_start -> Status error!,%04X\n", cmdack.comp_status);
    }
    return -68;
}

int dvrioctl2_set_rec_end_time(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2104;
    cmdack.input_word[0] = (*(u32 *)arg) >> 16;
    cmdack.input_word_count = 2;
    cmdack.input_word[1] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_rec_end_time -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_rec_end_time -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_get_rec_info(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;

    if (buflen >= 0x15) {
        int cmdackerr;
        drvdrv_exec_cmd_ack cmdack;
        cmdack.command = 0x2105;
        cmdack.input_word_count = 0;
        cmdackerr = DvrdrvExecCmdAck(&cmdack);
        if (cmdackerr) {
            DPRINTF("dvrioctl2_get_rec_info -> Handshake error!,%d\n", cmdackerr);
            return -5;
        } else {
            int v9;
            v9 = 0;
            if (cmdack.ack_status_ack) {
                DPRINTF("dvrioctl2_get_rec_info -> Status error!,%04X\n", cmdack.ack_status_ack);
                return -68;
            } else {
                int v10;
                *(u16 *)buf = 0;
                v10 = 0;
                do {
                    u16 *v11;
                    v9 += 1;
                    v11 = &cmdack.output_word[v9];
                    *((u8 *)buf + v10 + 3) = (*v11 & 0xFF00) >> 8;
                    *((u8 *)buf + v10 + 2) = (*v11 & 0x00FF);
                    v10 = 2 * v9;
                } while (v9 < 10);
                *((u8 *)buf + 22) = 0;
                return 0;
            }
        }
    } else {
        DPRINTF("dvrioctl2_get_rec_info -> buflen is smaller than 21.\n");
        return -22;
    }
}

int dvrioctl2_get_rec_time(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x2106;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_get_rec_time -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_get_rec_time -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        } else {
            *(u16 *)buf = 0;
            *((u32 *)buf + 1) = (cmdack.output_word[0] << 16) + cmdack.output_word[1];
        }
    }
    return 0;
}

int dvrioctl2_get_ifo_time_entry(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x2107;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_get_ifo_time_entry -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_get_ifo_time_entry -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = 0;
        *((u16 *)buf + 1) = cmdack.output_word[0];
        *((u16 *)buf + 2) = cmdack.output_word[1];
        *((u16 *)buf + 3) = cmdack.output_word[2];
        *((u32 *)buf + 2) = (cmdack.output_word[3] << 16) + cmdack.output_word[4];
    }
    return 0;
}

int dvrioctl2_get_ifo_vobu_entry(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x2108;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_get_ifo_vobu_entry -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_get_ifo_vobu_entry -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = 0;
        *((u16 *)buf + 1) = cmdack.output_word[0];
        *((u16 *)buf + 2) = cmdack.output_word[1];
        *((u16 *)buf + 3) = cmdack.output_word[2];
    }
    return 0;
}

int dvrioctl2_read_resfile(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2109;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_read_resfile -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_read_resfile -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_clear_resfile_flag(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x210A;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_clear_resfile_flag -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_clear_resfile_flag -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_rec_prohibit(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x210E;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_rec_prohibit -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_rec_prohibit -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_epg_test(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x210F;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_epg_test -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_epg_test -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_send_timer_event(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2110;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = arg;
    cmdack.timeout = 10000000;
    cmdack.input_buffer_length = arglen;
    if (DvrdrvExecCmdAckDmaSendComp(&cmdack)) {
        DPRINTF("dvrioctl2_send_timer_event -> IO error (phase %d)\n", cmdack.phase);
        return -5;
    } else {
        if (cmdack.comp_status) {
            DPRINTF(
                "dvrioctl2_send_timer_event -> Complete parameter (%04X) error (phase %d)\n",
                cmdack.comp_status,
                cmdack.phase);
            return -5;
        }
    }
    return 0;
}

int dvrioctl2_epg_cancel(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2111;
    cmdack.input_word_count = 0;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_epg_cancel -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_epg_cancel -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_get_status_register(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    *(u16 *)buf = (*((vu32 *)0xB0004230));
    *((u16 *)buf + 1) = (*((vu32 *)0xB0004234));
    *((u16 *)buf + 2) = (*((vu32 *)0xB0004238));
    *((u16 *)buf + 3) = (*((vu32 *)0xB000423C));
    return 0;
}

int dvrioctl2_tevent_buf_clr(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    char *v8;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    v7 = 6143;
    v8 = &TEVENT_BUF[6143];
    do {
        *v8 = 0;
        --v7;
        --v8;
    } while (v7 >= 0);
    tevent_p = TEVENT_BUF;
    tevent_data_sz = 0;
    return 0;
}

int dvrioctl2_tevent_buf_send(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)a1;
    (void)name;
    (void)cmd;
    (void)buf;
    (void)buflen;

    if ((int)(tevent_data_sz + arglen) < 0x1801) {
        signed int i;
        for (i = 0; i < (int)arglen; arg = (char *)arg + 1) {
            ++i;
            *tevent_p = *(u8 *)arg;
            tevent_p += 1;
        }
        tevent_data_sz += arglen;
    }
    return 0;
}

int dvrioctl2_tevent_buf_trans_dvrp(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2110;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = TEVENT_BUF;
    cmdack.timeout = 10000000;
    cmdack.input_buffer_length = tevent_data_sz;
    if (DvrdrvExecCmdAckDmaSendComp(&cmdack)) {
        DPRINTF("dvrioctl2_tevent_buf_trans_dvrp -> IO error (phase %d)\n", cmdack.phase);
        return -5;
    } else {
        if (cmdack.comp_status) {
            DPRINTF(
                "dvrioctl2_tevent_buf_trans_dvrp -> Complete parameter (%04X) error (phase %d)\n",
                cmdack.comp_status,
                cmdack.phase);
            return -5;
        }
    }
    return 0;
}

int dvrioctl2_start_hdd_test(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
#ifdef DEBUG
    int ack_status_ack;
#endif
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2112;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word[1] = *((u16 *)arg + 3);
    cmdack.input_word[2] = *((u16 *)arg + 2);
    cmdack.input_word[3] = *((u16 *)arg + 5);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word[5] = *((u16 *)arg + 6);
    cmdack.input_word[6] = *((u16 *)arg + 7);
    cmdack.input_word[7] = *((u16 *)arg + 8);
    cmdack.input_word_count = 8;
    cmdack.timeout = 1000000 * (*((u16 *)arg + 8) + 30);
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_start_hdd_test -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
#ifdef DEBUG
    ack_status_ack = cmdack.ack_status_ack;
#endif
    if (!cmdack.ack_status_ack) {
        if (!cmdack.comp_status)
            return 0;
#ifdef DEBUG
        ack_status_ack = cmdack.comp_status;
#endif
    }
    DPRINTF("dvrioctl2_start_hdd_test -> Status error!,%04X\n", ack_status_ack);
    return -68;
}

int dvrioctl2_stop_hdd_test(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
#ifdef DEBUG
    int ack_status_ack;
#endif
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2113;
    cmdack.input_word_count = 0;
    cmdack.timeout = 30000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_stop_hdd_test -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
#ifdef DEBUG
    ack_status_ack = cmdack.ack_status_ack;
#endif
    if (!cmdack.ack_status_ack) {
        if (!cmdack.comp_status)
            return 0;
#ifdef DEBUG
        ack_status_ack = cmdack.comp_status;
#endif
    }
#ifdef DEBUG
    DPRINTF("dvrioctl2_stop_hdd_test -> Status error!,%04X\n", ack_status_ack);
#endif
    return -68;
}

int dvrioctl2_get_hdd_test_stat(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x2114;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_get_hdd_test_stat -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_get_hdd_test_stat -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u32 *)buf + 1) = (cmdack.output_word[1] << 16) + cmdack.output_word[2];
        *((u32 *)buf + 2) = (cmdack.output_word[3] << 16) + cmdack.output_word[4];
        *((u16 *)buf + 6) = cmdack.output_word[5];
        *((u16 *)buf + 7) = cmdack.output_word[6];
        *((u16 *)buf + 8) = cmdack.output_word[7];
    }
    return 0;
}

int dvrioctl2_pre_update_a(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x2115;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_pre_update_a -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_pre_update_a -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = dvrpAuthEnc(cmdack.output_word[0]);
        return 0;
    }
}

int dvrioctl2_pre_update_b(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
#ifdef DEBUG
    int ack_status_ack;
#endif
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2116;
    cmdack.input_word_count = 1;
    cmdack.timeout = 10000000;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_pre_update_b -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
#ifdef DEBUG
    ack_status_ack = cmdack.ack_status_ack;
#endif
    if (!cmdack.ack_status_ack) {
        if (!cmdack.comp_status)
            return 0;
#ifdef DEBUG
        ack_status_ack = cmdack.comp_status;
#endif
    }
#ifdef DEBUG
    DPRINTF("dvrioctl2_pre_update_b -> Status error!,%04X\n", ack_status_ack);
#endif
    return -68;
}

int dvrioctl2_get_rec_vro_pckn(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x2117;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_get_rec_vro_pckn -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack == 0xFFFE) {
        DPRINTF("dvrioctl2_get_rec_vro_pckn -> Mode error!,%04X\n", 0xFFFE);
    } else {
        if (!cmdack.ack_status_ack) {
            *(u32 *)buf = (cmdack.output_word[0] << 16) + cmdack.output_word[1];
            return 0;
        }
        DPRINTF("dvrioctl2_get_rec_vro_pckn -> Status error!,%04X\n", cmdack.ack_status_ack);
    }
    return -68;
}

int dvrioctl2_enc_dec_test(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x2118;
    cmdack.input_word_count = 1;
    cmdack.timeout = 10000000;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_enc_dec_test -> ACK Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_enc_dec_test -> ACK Status error!,%04X\n", cmdack.ack_status_ack);
    } else {
        if (!cmdack.comp_status)
            return 0;
        DPRINTF("dvrioctl2_enc_dec_test -> COMP Status error!,%04X\n", cmdack.comp_status);
    }
    return -68;
}

int dvrioctl2_make_menu(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    int v8;
    int v9;
    u16 *v11;
    int cmdackerr;
    int v16;
    int v17;
    u16 *input_word;
    int v19;
    int busywait;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x2119;
    cmdack.input_word[0] = *((u16 *)arg + 1);
    cmdack.input_word[1] = *(u16 *)arg;
    cmdack.input_word[2] = *((u16 *)arg + 3);
    cmdack.input_word[3] = *((u16 *)arg + 2);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word[5] = 0;
    v7 = 6;
    v8 = 1;
    v9 = 0;
    v11 = &cmdack.input_word[5];
    do {
        char *v12;
        char *v13;
        v12 = (char *)arg + v8;
        v8 += 2;
        v13 = (char *)arg + v9;
        v9 += 2;
        ++v7;
        v11[1] = v13[10] + ((u8)v12[10] << 8);
        ++v11;
    } while (v7 < 16);
    cmdack.input_word_count = 16;
    cmdack.timeout = 30000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    *(u16 *)buf = cmdack.comp_status;
    if (cmdackerr) {
        DPRINTF("dvrioctl2_make_menu -> ACK Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_make_menu -> ACK Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    }
    v16 = 1;
    if (cmdack.comp_status) {
        DPRINTF("dvrioctl2_make_menu -> COMP Status error!,%04X\n", cmdack.comp_status);
        return -68;
    }
    v17 = 1;
    input_word = cmdack.return_result_word;
    v19 = 0;
    do {
        char *v20;
        char *v21;
        v20 = (char *)buf + v17;
        v21 = (char *)buf + v19;
        v17 += 2;
        v19 += 2;
        v16 += 1;
        v21[2] = (*input_word & 0x00FF);
        v20[2] = (*input_word & 0xFF00) >> 8;
        input_word += 1;
    } while (v16 < 11);
    *((u8 *)buf + 22) = 0;
    busywait = 0x14;
    while (busywait-- >= 0)
        ;
    return 0;
}

int dvrioctl2_re_enc_start(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    int v8;
    int v9;
    u16 *v10;
    int v13;
    int v14;
    int v15;
    u16 *v16;
    int busywait;
    int cmdackerr;
    int v24;
    int v25;
    u16 *v26;
    int v27;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x211A;
    cmdack.input_word[0] = *((u16 *)arg + 1);
    cmdack.input_word[1] = *(u16 *)arg;
    cmdack.input_word[2] = *((u16 *)arg + 3);
    cmdack.input_word[3] = *((u16 *)arg + 2);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word[5] = *((u16 *)arg + 5);
    cmdack.input_word[6] = *((u16 *)arg + 6);
    v7 = 7;
    v8 = 1;
    v9 = 0;
    v10 = &cmdack.input_word[6];
    do {
        u8 *v11;
        u8 *v12;
        v11 = (u8 *)arg + v8;
        v12 = (u8 *)arg + v9;
        v10[1] = v12[14] + ((u8)v11[14] << 8);
        v8 += 2;
        v9 += 2;
        v7 += 1;
        v10 += 1;
    } while (v7 < 17);
    cmdack.input_word[17] = 0;
    v13 = 18;
    v14 = 1;
    v15 = 0;
    v16 = &cmdack.input_word[17];
    do {
        u8 *v17;
        u8 *v18;
        v17 = (u8 *)arg + v14;
        v18 = (u8 *)arg + v15;
        v16[1] = v18[35] + ((u8)v17[35] << 8);
        v14 += 2;
        v15 += 2;
        v13 += 1;
        v16 += 1;
    } while (v13 < 28);
    cmdack.input_word[26] = 0;
    cmdack.input_word[27] = 0;
    cmdack.input_word[28] = *((u16 *)arg + 28);
    cmdack.input_word[29] = *((u16 *)arg + 29);
    cmdack.input_word[30] = *((u16 *)arg + 30);
    cmdack.input_word[31] = *((u16 *)arg + 31);
    cmdack.input_word[32] = 0;
    busywait = 0x1f;
    while (busywait-- >= 0)
        ;
    cmdack.input_word_count = 33;
    cmdack.timeout = 30000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    *(u16 *)buf = cmdack.comp_status;
    if (cmdackerr) {
        DPRINTF("dvrioctl2_re_enc_start -> ACK Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_re_enc_start -> ACK Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    }
    v24 = 2;
    if (cmdack.comp_status) {
        DPRINTF("dvrioctl2_re_enc_start -> COMP Status error!,%04X\n", cmdack.comp_status);
        return -68;
    }
    v25 = 1;
    v26 = &cmdack.input_word[1];
    v27 = 0;
    do {
        u8 *v28;
        u8 *v29;
        v28 = (u8 *)buf + v25;
        v29 = (u8 *)buf + v27;
        v29[2] = *((u8 *)v26 + 274);
        v28[2] = v26[137] >> 8;
        v25 += 2;
        v27 += 2;
        v24 += 1;
        v26 += 1;
    } while (v24 < 12);
    *((u8 *)buf + 22) = 0;
    return 0;
}

int dvrpAuthEnc(u16 a1)
{
    return (u8)dvrpAuth_tbl[(u8)a1] | ((u8)dvrpAuth_tbl[a1 >> 8] << 8);
}

int dvr_recv_dma(iomanX_iop_file_t *a1, u8 *buf, int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    DPRINTF("------------------- 2 ------------------ dvr_recv_dma(io=%p, buf=%p, buflen=%d)\n", a1, buf, buflen);
    if (((u32)buf & 3) != 0) {
        DPRINTF("dvr_recv_dma : Address is not a multiple of 4.\n");
        return -14;
    } else {
        u8 *buf_tmp;
        int buflen_tmp;
        int ack_status_ack2;
        int err;
        buf_tmp = buf;
        if ((buflen & 0x7F) != 0)
            DPRINTF("buflen is not a multiple of 128.\n");
        buflen_tmp = buflen;
        while (buflen_tmp > 0) {
            cmdack.command = 0x211B;
            cmdack.input_word[0] = 0;
            cmdack.input_word[1] = 0;
            cmdack.input_word[2] = 0;
            cmdack.input_word[3] = 6144;
            cmdack.input_word_count = 4;
            cmdack.output_buffer = buf_tmp;
            cmdack.timeout = 0x10000;
            if (DvrdrvExecCmdAckDmaRecvComp(&cmdack)) {
                err = -5;
                DPRINTF("dvr_recv_dma : IO error (phase %d)\n", cmdack.phase);
                goto finish;
            }
            if (cmdack.comp_status) {
                err = -5;
                DPRINTF("dvr_recv_dma : Complete parameter error (phase %d), %04X\n", cmdack.phase, cmdack.comp_status);
                goto finish;
            }
            ack_status_ack2 = cmdack.ack_status_ack2;
            DPRINTF("rsize : %d / %d \n", cmdack.ack_status_ack2, 6144);
            if (ack_status_ack2 <= 0)
                break;
            buflen_tmp -= ack_status_ack2;
            if (ack_status_ack2 < 0x1801)
                break;
            buf_tmp += ack_status_ack2;
        }
        err = cmdack.ack_status_ack2;
    finish:
        DPRINTF("dvr_recv_dma: ret = %d\n", err);
        return err;
    }
}

u8 TELTEXT_BUF[0x1800];
unsigned int TELTEXT_ACTLEN;
unsigned int TELTEXT_BUFOFFSET;

int dvrioctl2_tevent_buf_recv_first(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)name;
    (void)arg;
    (void)arglen;

    TELTEXT_BUFOFFSET = 0;
    TELTEXT_ACTLEN = 0;
    DPRINTF("dvr_tevent_buf_recv_first(io=%p, cmd=%d  buf=%p, nbyte=%u)\n", a1, cmd, buf, buflen);
    TELTEXT_ACTLEN = dvr_recv_dma(a1, TELTEXT_BUF, sizeof(TELTEXT_BUF));
    if ((TELTEXT_ACTLEN & 0x80000000) == 0) {
        if (buflen == 1024) {
            memcpy(buf, (u8 *)TELTEXT_BUF + TELTEXT_BUFOFFSET, 1024);
            TELTEXT_BUFOFFSET += 1024;
            if (TELTEXT_ACTLEN <= 0x400) {
                DPRINTF("tevent_buf_recv_first: actlen:%u request-buflen:%d return 0 < DMA EOT >\n", TELTEXT_ACTLEN, 1024);
                return 0;
            } else {
                DPRINTF("tevent_buf_recv_first: return <DMA_REMAIND>\n");
                return 0xFFE5;
            }
        } else {
            DPRINTF("tevent_buf_recv_first: bufflen error! return 0 < EOT > .\n");
            return 0;
        }
    } else {
        DPRINTF("tevent_buf_recv_first: dma error!  return 0 < EOT >.\n");
        return 0;
    }
}

int dvrioctl2_tevent_buf_recv_next(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)name;
    (void)arg;
    (void)arglen;

    DPRINTF("dvr_tevent_buf_recv_next(io=%p, cmd=%d buf=%p, nbyte=%u)\n", a1, cmd, buf, buflen);
    if (buflen == 1024) {
        if (TELTEXT_BUFOFFSET + 1024 < sizeof(TELTEXT_BUF)) {
            memcpy(buf, TELTEXT_BUF + TELTEXT_BUFOFFSET, 1024);
            TELTEXT_BUFOFFSET += 1024;
            if ((int)TELTEXT_BUFOFFSET < (int)TELTEXT_ACTLEN) {
                DPRINTF("tevent_buf_recv_next: return <DMA_REMAIND>\n");
                return 65509;
            }
            DPRINTF("tevent_buf_recv_next: return 0 < DMA EOT >!\n");
        } else {
            DPRINTF("tevent_buf_recv_next: bufflen error!\n");
        }
    } else {
        DPRINTF("tevent_buf_recv_next: bufflen error!\n");
    }
    return 0;
}

int dvrioctl2_finish_auto_process(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x211C;
    cmdack.input_word_count = 1;
    cmdack.timeout = 5000000;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_finish_auto_process -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack == 0xFFFE) {
        DPRINTF("dvrioctl2_finish_auto_process -> Mode error!,%04X\n", 0xFFFE);
    } else {
        if (!cmdack.ack_status_ack) {
            *(u32 *)buf = (cmdack.output_word[0] << 16) + cmdack.output_word[1];
            return 0;
        }
        DPRINTF("dvrioctl2_finish_auto_process -> Status error!,%04X\n", cmdack.ack_status_ack);
    }
    return -68;
}

int dvrioctl2_rec_pictclip(
    iomanX_iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int busywait;
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)name;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x211D;
    cmdack.input_word[0] = *((u16 *)arg + 1);
    cmdack.input_word[1] = *((u16 *)arg + 0);
    cmdack.input_word[2] = *((u16 *)arg + 3);
    cmdack.input_word[3] = *((u16 *)arg + 2);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word[5] = *((u16 *)arg + 5);
    cmdack.input_word[6] = *((u16 *)arg + 6);
    busywait = 4;
    while (busywait-- >= 0)
        ;
    cmdack.input_word_count = 7;
    cmdack.timeout = 30000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        DPRINTF("dvrioctl2_re_enc_start -> ACK Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_re_enc_start -> ACK Status error!,%04X\n", cmdack.ack_status_ack);
    } else {
        if (!cmdack.comp_status)
            return 0;
        DPRINTF("dvrioctl2_re_enc_start -> COMP Status error!,%04X\n", cmdack.comp_status);
    }
    return -68;
}
