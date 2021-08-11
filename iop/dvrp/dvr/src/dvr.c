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
extern int dvr_df_init(iop_device_t *dev);
extern int dvr_df_exit(iop_device_t *dev);
extern int dvr_df_ioctl(iop_file_t *f, int cmd, void *param);
extern int dvr_df_devctl(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvr_df_ioctl2(iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvr_df_null();
extern s64 dvr_df_null_long();
extern int dvrioctl2_rec_start(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_rec_pause(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_rec_stop(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_set_rec_end_time(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_rec_info(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_rec_time(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_read_resfile(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_clear_resfile_flag(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_rec_prohibit(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_status_register(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_ifo_time_entry(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_ifo_vobu_entry(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_epg_test(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_send_timer_event(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_epg_cancel(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_clr(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_send(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_tevent_buf_trans_dvrp(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_start_hdd_test(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_stop_hdd_test(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_hdd_test_stat(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_pre_update_a(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_pre_update_b(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_rec_vro_pckn(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_enc_dec_test(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_make_menu(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_re_enc_start(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrpAuthEnc(u16);

// The following has been excluded.
unsigned char dvrpAuth_tbl[256] = {0x00};

struct DevctlCmdTbl_t
{
    u16 cmd;
    int (*fn)(iop_file_t *, const char *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[27] =
    {
        {22115u, &dvrioctl2_get_status_register},
        {22072u, &dvrioctl2_get_ifo_time_entry},
        {22073u, &dvrioctl2_get_ifo_vobu_entry},
        {22066u, &dvrioctl2_rec_start},
        {22067u, &dvrioctl2_rec_pause},
        {22068u, &dvrioctl2_rec_stop},
        {22069u, &dvrioctl2_set_rec_end_time},
        {22070u, &dvrioctl2_get_rec_info},
        {22071u, &dvrioctl2_get_rec_time},
        {22074u, &dvrioctl2_read_resfile},
        {22075u, &dvrioctl2_clear_resfile_flag},
        {22079u, &dvrioctl2_rec_prohibit},
        {22080u, &dvrioctl2_epg_test},
        {22081u, &dvrioctl2_send_timer_event},
        {22082u, &dvrioctl2_epg_cancel},
        {22106u, &dvrioctl2_tevent_buf_clr},
        {22107u, &dvrioctl2_tevent_buf_send},
        {22108u, &dvrioctl2_tevent_buf_trans_dvrp},
        {22083u, &dvrioctl2_start_hdd_test},
        {22084u, &dvrioctl2_stop_hdd_test},
        {22085u, &dvrioctl2_get_hdd_test_stat},
        {22086u, &dvrioctl2_pre_update_a},
        {22087u, &dvrioctl2_pre_update_b},
        {22088u, &dvrioctl2_get_rec_vro_pckn},
        {22089u, &dvrioctl2_enc_dec_test},
        {22090u, &dvrioctl2_make_menu},
        {22091u, &dvrioctl2_re_enc_start}};

struct _iop_device_ops DvrFuncTbl =
    {
        &dvr_df_init,
        &dvr_df_exit,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_ioctl,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_null_long,
        &dvr_df_devctl,
        &dvr_df_null,
        &dvr_df_null,
        &dvr_df_ioctl2};
char TEVENT_BUF[6144];
char *tevent_p;
int tevent_data_sz;
iop_device_t DVR;
s32 sema_id;

// Based off of DESR / PSX DVR system software version 1.31.
#define MODNAME "DVR"
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
    int i;
    bool v1;
    int result;

    for (i = 0; i < 30000; ++i) {
        if (((*((vu32 *)0xB0004230)) & 0x10) != 0)
            break;
        DelayThread(1000);
    }
    if (i == 30000) {
        printf("DVR task of DVRP is not running...\n");
        return 1;
    } else {
        DVR.name = "dvr";
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

    v0 = DelDrv("dvr") == 0;
    result = 1;
    if (!v0)
        return 2;
    return result;
}

int dvr_df_init(iop_device_t *dev)
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

int dvr_df_exit(iop_device_t *dev)
{
    bool v1;
    int result;

    v1 = DeleteSema(sema_id) == 0;
    result = 0;
    if (!v1)
        return -1;
    return result;
}

int dvr_df_ioctl(iop_file_t *f, int cmd, void *param)
{
    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvr_df_devctl(
    iop_file_t *a1,
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
        if (v12 >= 27)
            goto LABEL_5;
    }
    v11 = DevctlCmdTbl[v13].fn(a1, name, cmd, arg, arglen, buf, buflen);
LABEL_5:
    if (v12 == 27)
        v11 = -22;
    SignalSema(sema_id);
    return v11;
}

int dvr_df_ioctl2(iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
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
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    int cmdackerr;
    int retval;
    int err;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2101;
    cmdack.input_word[0] = *((u16 *)arg + 1);
    cmdack.input_word[1] = *(u16 *)arg;
    cmdack.input_word[2] = *((u16 *)arg + 3);
    cmdack.input_word[3] = *((u16 *)arg + 2);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word[5] = *((u16 *)arg + 5);
    cmdack.input_word[6] = *((u16 *)arg + 6);
    v7 = 0x4;
    while (v7-- >= 0)
        ;
    cmdack.input_word_count = 7;
    cmdack.timeout = 5000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_rec_start -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    err = cmdack.ack_status_ack;
    if (!cmdack.ack_status_ack) {
        retval = 0;
        if (!cmdack.comp_status)
            return retval;
        err = cmdack.comp_status;
    }
    printf("dvrioctl2_rec_start -> Status error!,%04X\n", err);
    return -68;
}

int dvrioctl2_rec_pause(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    drvdrv_exec_cmd_ack *p_cmdack;
    u16 v9;
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    v7 = 0;
    p_cmdack = &cmdack;
    cmdack.command = 0x2102;
    do {
        v9 = *(u16 *)arg;
        arg = (char *)arg + 2;
        ++v7;
        p_cmdack->input_word[0] = v9;
        p_cmdack = (drvdrv_exec_cmd_ack *)((char *)p_cmdack + 2);
    } while (v7 <= 0);
    cmdack.input_word_count = 1;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_rec_pause -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_rec_pause -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_rec_stop(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2103;
    cmdack.input_word_count = 0;
    cmdack.timeout = 5000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_rec_stop -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        printf("dvrioctl2_rec_stop -> Status error!,%04X\n", cmdack.ack_status_ack);
    } else {
        retval = 0;
        if (!cmdack.comp_status)
            return retval;
        printf("dvrioctl2_rec_start -> Status error!,%04X\n", cmdack.comp_status);
    }
    return -68;
}

int dvrioctl2_set_rec_end_time(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2104;
    cmdack.input_word[0] = (*(u32 *)arg) >> 16;
    cmdack.input_word_count = 2;
    cmdack.input_word[1] = *(u16 *)arg;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_rec_end_time -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_rec_end_time -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_get_rec_info(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int v9;
    int v10;
    u16 *v11;
    drvdrv_exec_cmd_ack cmdack;

    if (buflen >= 0x15) {
        cmdack.command = 0x2105;
        cmdack.input_word_count = 0;
        cmdackerr = DvrdrvExecCmdAck(&cmdack);
        if (cmdackerr) {
            printf("dvrioctl2_get_rec_info -> Handshake error!,%d\n", cmdackerr);
            return -5;
        } else {
            v9 = 0;
            if (cmdack.ack_status_ack) {
                printf("dvrioctl2_get_rec_info -> Status error!,%04X\n", cmdack.ack_status_ack);
                return -68;
            } else {
                *(u16 *)buf = 0;
                v10 = 0;
                do {
                    ++v9;
                    v11 = &cmdack.command + v9;
                    *((u8 *)buf + v10 + 3) = v11[69] >> 8;
                    *((u8 *)buf + v10 + 2) = *((u8 *)v11 + 138);
                    v10 = 2 * v9;
                } while (v9 < 10);
                *((u8 *)buf + 22) = 0;
                return 0;
            }
        }
    } else {
        printf("dvrioctl2_get_rec_info -> buflen is smaller than 21.\n");
        return -22;
    }
}

int dvrioctl2_get_rec_time(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2106;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_get_rec_time -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_get_rec_time -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        } else {
            *(u16 *)buf = 0;
            *((u32 *)buf + 1) = (cmdack.output_word[0] << 16) + cmdack.output_word[1];
        }
    }
    return retval;
}

int dvrioctl2_get_ifo_time_entry(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2107;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_get_ifo_time_entry -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_get_ifo_time_entry -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = 0;
        *((u16 *)buf + 1) = cmdack.output_word[0];
        *((u16 *)buf + 2) = cmdack.output_word[1];
        retval = 0;
        *((u16 *)buf + 3) = cmdack.output_word[2];
        *((u32 *)buf + 2) = (cmdack.output_word[3] << 16) + cmdack.output_word[4];
    }
    return retval;
}

int dvrioctl2_get_ifo_vobu_entry(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2108;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_get_ifo_vobu_entry -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_get_ifo_vobu_entry -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = 0;
        *((u16 *)buf + 1) = cmdack.output_word[0];
        *((u16 *)buf + 2) = cmdack.output_word[1];
        retval = 0;
        *((u16 *)buf + 3) = cmdack.output_word[2];
    }
    return retval;
}

int dvrioctl2_read_resfile(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2109;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_read_resfile -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_read_resfile -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_clear_resfile_flag(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x210A;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_clear_resfile_flag -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_clear_resfile_flag -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_rec_prohibit(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v7;
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x210E;
    v7 = *(u16 *)arg;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = v7;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_rec_prohibit -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_rec_prohibit -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_epg_test(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v7;
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x210F;
    v7 = *(u16 *)arg;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = v7;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_epg_test -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_epg_test -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_send_timer_event(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    drvdrv_exec_cmd_ack cmdack;

    v7 = 0;
    cmdack.command = 0x2110;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = arg;
    cmdack.timeout = 10000000;
    cmdack.input_buffer_length = arglen;
    if (DvrdrvExecCmdAckDmaSendComp(&cmdack)) {
        printf("dvrioctl2_send_timer_event -> IO error (phase %d)\n", cmdack.phase);
        return -5;
    } else {
        if (cmdack.comp_status) {
            v7 = -5;
            printf(
                "dvrioctl2_send_timer_event -> Complete parameter (%04X) error (phase %d)\n",
                cmdack.comp_status,
                cmdack.phase);
        }
        return v7;
    }
}

int dvrioctl2_epg_cancel(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v7;
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2111;
    v7 = *(u16 *)arg;
    cmdack.input_word_count = 0;
    cmdack.input_word[0] = v7;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_epg_cancel -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_epg_cancel -> Status error!,%04X\n", cmdack.ack_status_ack);
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_get_status_register(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    s16 v7;
    s16 v8;

    *(u16 *)buf = (*((vu32 *)0xB0004230));
    v7 = (*((vu32 *)0xB0004238));
    v8 = (*((vu32 *)0xB000423C));
    *((u16 *)buf + 1) = (*((vu32 *)0xB0004234));
    *((u16 *)buf + 2) = v7;
    *((u16 *)buf + 3) = v8;
    return 0;
}

int dvrioctl2_tevent_buf_clr(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    char *v8;

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
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    signed int i;
    char *v8;

    if ((int)(tevent_data_sz + arglen) < 0x1801) {
        for (i = 0; i < (int)arglen; arg = (char *)arg + 1) {
            v8 = tevent_p;
            ++i;
            *tevent_p = *(u8 *)arg;
            tevent_p = v8 + 1;
        }
        tevent_data_sz += arglen;
    }
    return 0;
}

int dvrioctl2_tevent_buf_trans_dvrp(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    drvdrv_exec_cmd_ack cmdack;

    v7 = 0;
    cmdack.command = 0x2110;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = TEVENT_BUF;
    cmdack.timeout = 10000000;
    cmdack.input_buffer_length = tevent_data_sz;
    if (DvrdrvExecCmdAckDmaSendComp(&cmdack)) {
        printf("dvrioctl2_tevent_buf_trans_dvrp -> IO error (phase %d)\n", cmdack.phase);
        return -5;
    } else {
        if (cmdack.comp_status) {
            v7 = -5;
            printf(
                "dvrioctl2_tevent_buf_trans_dvrp -> Complete parameter (%04X) error (phase %d)\n",
                cmdack.comp_status,
                cmdack.phase);
        }
        return v7;
    }
}

int dvrioctl2_start_hdd_test(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v7;
    int cmdackerr;
    int retval;
    int ack_status_ack;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2112;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word[1] = *((u16 *)arg + 3);
    cmdack.input_word[2] = *((u16 *)arg + 2);
    cmdack.input_word[3] = *((u16 *)arg + 5);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word[5] = *((u16 *)arg + 6);
    cmdack.input_word[6] = *((u16 *)arg + 7);
    v7 = *((u16 *)arg + 8);
    cmdack.input_word_count = 8;
    cmdack.input_word[7] = v7;
    cmdack.timeout = 1000000 * (*((u16 *)arg + 8) + 30);
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_start_hdd_test -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    ack_status_ack = cmdack.ack_status_ack;
    if (!cmdack.ack_status_ack) {
        retval = 0;
        if (!cmdack.comp_status)
            return retval;
        ack_status_ack = cmdack.comp_status;
    }
    printf("dvrioctl2_start_hdd_test -> Status error!,%04X\n", ack_status_ack);
    return -68;
}

int dvrioctl2_stop_hdd_test(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    int ack_status_ack;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2113;
    cmdack.input_word_count = 0;
    cmdack.timeout = 30000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_stop_hdd_test -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    ack_status_ack = cmdack.ack_status_ack;
    if (!cmdack.ack_status_ack) {
        retval = 0;
        if (!cmdack.comp_status)
            return retval;
        ack_status_ack = cmdack.comp_status;
    }
    printf("dvrioctl2_stop_hdd_test -> Status error!,%04X\n", ack_status_ack);
    return -68;
}

int dvrioctl2_get_hdd_test_stat(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2114;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_get_hdd_test_stat -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_get_hdd_test_stat -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u32 *)buf + 1) = (cmdack.output_word[1] << 16) + cmdack.output_word[2];
        *((u32 *)buf + 2) = (cmdack.output_word[3] << 16) + cmdack.output_word[4];
        *((u16 *)buf + 6) = cmdack.output_word[5];
        *((u16 *)buf + 7) = cmdack.output_word[6];
        retval = 0;
        *((u16 *)buf + 8) = cmdack.output_word[7];
    }
    return retval;
}

int dvrioctl2_pre_update_a(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2115;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_pre_update_a -> Handshake error!,%d\n", cmdackerr);
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_pre_update_a -> Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    } else {
        *(u16 *)buf = dvrpAuthEnc(cmdack.output_word[0]);
        return 0;
    }
}

int dvrioctl2_pre_update_b(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v7;
    int cmdackerr;
    int retval;
    int ack_status_ack;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2116;
    v7 = *(u16 *)arg;
    cmdack.input_word_count = 1;
    cmdack.timeout = 10000000;
    cmdack.input_word[0] = v7;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_pre_update_b -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    ack_status_ack = cmdack.ack_status_ack;
    if (!cmdack.ack_status_ack) {
        retval = 0;
        if (!cmdack.comp_status)
            return retval;
        ack_status_ack = cmdack.comp_status;
    }
    printf("dvrioctl2_pre_update_b -> Status error!,%04X\n", ack_status_ack);
    return -68;
}

int dvrioctl2_get_rec_vro_pckn(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2117;
    cmdack.input_word_count = 0;
    cmdackerr = DvrdrvExecCmdAck(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_get_rec_vro_pckn -> Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack == 0xFFFE) {
        printf("dvrioctl2_get_rec_vro_pckn -> Mode error!,%04X\n", 0xFFFE);
    } else {
        retval = 0;
        if (!cmdack.ack_status_ack) {
            *(u32 *)buf = (cmdack.output_word[0] << 16) + cmdack.output_word[1];
            return retval;
        }
        printf("dvrioctl2_get_rec_vro_pckn -> Status error!,%04X\n", cmdack.ack_status_ack);
    }
    return -68;
}

int dvrioctl2_enc_dec_test(
    iop_file_t *a1,
    const char *name,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v7;
    int cmdackerr;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2118;
    v7 = *(u16 *)arg;
    cmdack.input_word_count = 1;
    cmdack.timeout = 10000000;
    cmdack.input_word[0] = v7;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    if (cmdackerr) {
        printf("dvrioctl2_enc_dec_test -> ACK Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        printf("dvrioctl2_enc_dec_test -> ACK Status error!,%04X\n", cmdack.ack_status_ack);
    } else {
        retval = 0;
        if (!cmdack.comp_status)
            return retval;
        printf("dvrioctl2_enc_dec_test -> COMP Status error!,%04X\n", cmdack.comp_status);
    }
    return -68;
}

int dvrioctl2_make_menu(
    iop_file_t *a1,
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
    u16 v10;
    u16 *v11;
    char *v12;
    char *v13;
    int cmdackerr;
    int v16;
    int v17;
    u16 *input_word;
    int v19;
    char *v20;
    char *v21;
    int v22;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x2119;
    cmdack.input_word[0] = *((u16 *)arg + 1);
    v7 = 6;
    cmdack.input_word[1] = *(u16 *)arg;
    v8 = 1;
    cmdack.input_word[2] = *((u16 *)arg + 3);
    v9 = 0;
    cmdack.input_word[3] = *((u16 *)arg + 2);
    v10 = *((u16 *)arg + 4);
    v11 = &cmdack.input_word[5];
    cmdack.input_word[5] = 0;
    cmdack.input_word[4] = v10;
    do {
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
        printf("dvrioctl2_make_menu -> ACK Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        printf("dvrioctl2_make_menu -> ACK Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    }
    v16 = 1;
    if (cmdack.comp_status) {
        printf("dvrioctl2_make_menu -> COMP Status error!,%04X\n", cmdack.comp_status);
        return -68;
    }
    v17 = 1;
    input_word = cmdack.input_word;
    v19 = 0;
    do {
        v20 = (char *)buf + v17;
        v17 += 2;
        v21 = (char *)buf + v19;
        v19 += 2;
        ++v16;
        v21[2] = *((u8 *)input_word + 274);
        v20[2] = input_word[137] >> 8;
        ++input_word;
    } while (v16 < 11);
    *((u8 *)buf + 22) = 0;
    v22 = 0x14;
    while (v22-- >= 0)
        ;
    return 0;
}

int dvrioctl2_re_enc_start(
    iop_file_t *a1,
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
    char *v11;
    char *v12;
    int v13;
    int v14;
    int v15;
    u16 *v16;
    char *v17;
    char *v18;
    u16 v19;
    int v20;
    int cmdackerr;
    int v24;
    int v25;
    u16 *v26;
    int v27;
    char *v28;
    char *v29;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x211A;
    cmdack.input_word[0] = *((u16 *)arg + 1);
    cmdack.input_word[1] = *(u16 *)arg;
    cmdack.input_word[2] = *((u16 *)arg + 3);
    v7 = 7;
    cmdack.input_word[3] = *((u16 *)arg + 2);
    v8 = 1;
    cmdack.input_word[4] = *((u16 *)arg + 4);
    v9 = 0;
    cmdack.input_word[5] = *((u16 *)arg + 5);
    v10 = &cmdack.input_word[6];
    cmdack.input_word[6] = *((u16 *)arg + 6);
    do {
        v11 = (char *)arg + v8;
        v8 += 2;
        v12 = (char *)arg + v9;
        v9 += 2;
        ++v7;
        v10[1] = v12[14] + ((u8)v11[14] << 8);
        ++v10;
    } while (v7 < 17);
    cmdack.input_word[17] = 0;
    v13 = 18;
    v14 = 1;
    v15 = 0;
    v16 = &cmdack.input_word[17];
    do {
        v17 = (char *)arg + v14;
        v14 += 2;
        v18 = (char *)arg + v15;
        v15 += 2;
        ++v13;
        v16[1] = v18[35] + ((u8)v17[35] << 8);
        ++v16;
    } while (v13 < 28);
    cmdack.input_word[28] = *((u16 *)arg + 28);
    cmdack.input_word[29] = *((u16 *)arg + 29);
    cmdack.input_word[30] = *((u16 *)arg + 30);
    v19 = *((u16 *)arg + 31);
    cmdack.input_word[32] = 0;
    cmdack.input_word[26] = 0;
    cmdack.input_word[27] = 0;
    cmdack.input_word[31] = v19;
    v20 = 0x1f;
    while (v20-- >= 0)
        ;
    cmdack.input_word_count = 33;
    cmdack.timeout = 30000000;
    cmdackerr = DvrdrvExecCmdAckComp(&cmdack);
    *(u16 *)buf = cmdack.comp_status;
    if (cmdackerr) {
        printf("dvrioctl2_re_enc_start -> ACK Handshake error!,%d\n", cmdackerr);
        return -5;
    }
    if (cmdack.ack_status_ack) {
        printf("dvrioctl2_re_enc_start -> ACK Status error!,%04X\n", cmdack.ack_status_ack);
        return -68;
    }
    v24 = 2;
    if (cmdack.comp_status) {
        printf("dvrioctl2_re_enc_start -> COMP Status error!,%04X\n", cmdack.comp_status);
        return -68;
    }
    v25 = 1;
    v26 = &cmdack.input_word[1];
    v27 = 0;
    do {
        v28 = (char *)buf + v25;
        v25 += 2;
        v29 = (char *)buf + v27;
        v27 += 2;
        ++v24;
        v29[2] = *((u8 *)v26 + 274);
        v28[2] = v26[137] >> 8;
        ++v26;
    } while (v24 < 12);
    *((u8 *)buf + 22) = 0;
    return 0;
}

int dvrpAuthEnc(u16 a1)
{
    return (u8)dvrpAuth_tbl[(u8)a1] | ((u8)dvrpAuth_tbl[a1 >> 8] << 8);
}
