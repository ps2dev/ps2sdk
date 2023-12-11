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

#define MODNAME "DVRMISC"
#ifdef DEBUG
#define DPRINTF(x...) printf(MODNAME ": " x)
#else
#define DPRINTF(x...)
#endif

extern int module_start();
extern int module_stop();
extern int dvrmisc_df_init(iomanX_iop_device_t *dev);
extern int dvrmisc_df_exit(iomanX_iop_device_t *dev);
extern int dvrmisc_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param);
extern int dvrmisc_df_devctl(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrmisc_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrmisc_df_null();
extern s64 dvrmisc_df_null_long();
extern int dvrioctl2_nop(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_version(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_led_hdd_rec(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_led_dvd_rec(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_sircs(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_time(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_set_timezone(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_save_preset_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_load_preset_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_dev_rst(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_sdram_chk(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_mpe_chk(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_mpd_chk(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_vdec_chk(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_partition_free(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_buzzer(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_clr_preset_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_vbi_err_rate(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_update_dvrp_firmware(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_flash_write_status(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_set_device_key(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_device_key(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_set_dv_nodeid(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_dv_nodeid(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_diag_test(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

struct DevctlCmdTbl_t
{
    u16 cmd;
    int (*fn)(iomanX_iop_file_t *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[25] =
    {
        {0x5668, &dvrioctl2_get_sircs},
        {0x5666, &dvrioctl2_led_hdd_rec},
        {0x5667, &dvrioctl2_led_dvd_rec},
        {0x5669, &dvrioctl2_get_time},
        {0x566A, &dvrioctl2_set_timezone},
        {0x566B, &dvrioctl2_save_preset_info},
        {0x566C, &dvrioctl2_load_preset_info},
        {0x5664, &dvrioctl2_nop},
        {0x5665, &dvrioctl2_version},
        {0x566D, &dvrioctl2_test_dev_rst},
        {0x566E, &dvrioctl2_test_sdram_chk},
        {0x566F, &dvrioctl2_test_mpe_chk},
        {0x5670, &dvrioctl2_test_mpd_chk},
        {0x5671, &dvrioctl2_test_vdec_chk},
        {0x5672, &dvrioctl2_partition_free},
        {0x5673, &dvrioctl2_buzzer},
        {0x5674, &dvrioctl2_clr_preset_info},
        {0x5675, &dvrioctl2_get_vbi_err_rate},
        {0x5676, &dvrioctl2_update_dvrp_firmware},
        {0x5677, &dvrioctl2_flash_write_status},
        {0x5678, &dvrioctl2_set_device_key},
        {0x5679, &dvrioctl2_get_device_key},
        {0x567A, &dvrioctl2_set_dv_nodeid},
        {0x567B, &dvrioctl2_get_dv_nodeid},
        {0x5682, &dvrioctl2_diag_test},
};
static iomanX_iop_device_ops_t DvrFuncTbl =
    {
        &dvrmisc_df_init,
        &dvrmisc_df_exit,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_ioctl,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_null_long,
        &dvrmisc_df_devctl,
        &dvrmisc_df_null,
        &dvrmisc_df_null,
        &dvrmisc_df_ioctl2,
    };
static iomanX_iop_device_t DVRMISC = {
    .name = "dvr_misc",
    .desc = "Digital Video Recorder MISC task",
    .type = (IOP_DT_FS | IOP_DT_FSEXT),
    .ops = &DvrFuncTbl,
};
s32 sema_id;
char SBUF[16384];

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
        if (((*((vu32 *)0xB0004230)) & 4) != 0)
            break;
        DelayThread(1000);
    }
    if (i == 30000) {
        DPRINTF("MISC task of DVRP is not running...\n");
        return MODULE_NO_RESIDENT_END;
    } else {
        if (iomanX_AddDrv(&DVRMISC) != 0)
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
    if (iomanX_DelDrv(DVRMISC.name) != 0)
        return MODULE_REMOVABLE_END;
    return MODULE_NO_RESIDENT_END;
}

int dvrmisc_df_init(iomanX_iop_device_t *dev)
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

int dvrmisc_df_exit(iomanX_iop_device_t *dev)
{
    (void)dev;

    if (DeleteSema(sema_id) != 0)
        return -1;
    return 0;
}

int dvrmisc_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param)
{
    (void)f;
    (void)cmd;
    (void)param;

    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvrmisc_df_devctl(
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

    (void)name;

    v10 = 0;
    v11 = 0;
    WaitSema(sema_id);
    v12 = 0;
    while (DevctlCmdTbl[v12].cmd != cmd) {
        v12 = ++v11;
        if (v11 >= sizeof(DevctlCmdTbl) / sizeof(DevctlCmdTbl[0]))
            goto LABEL_5;
    }
    v10 = DevctlCmdTbl[v12].fn(a1, cmd, arg, arglen, buf, buflen);
LABEL_5:
    if (v11 == sizeof(DevctlCmdTbl) / sizeof(DevctlCmdTbl[0]))
        v10 = -22;
    SignalSema(sema_id);
    return v10;
}

int dvrmisc_df_ioctl2(
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

int dvrmisc_df_null()
{
    return -48;
}

s64 dvrmisc_df_null_long()
{
    return -48LL;
}

int dvrioctl2_nop(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x5101;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_nop -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_nop -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_version(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x5102;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_version -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_version -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u16 *)buf + 1) = cmdack.output_word[1];
    }
    return 0;
}

int dvrioctl2_led_hdd_rec(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x5104;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_led_hdd_rec -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_led_hdd_rec -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_led_dvd_rec(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x5106;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_led_dvd_rec -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_led_dvd_rec -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_get_sircs(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x5107;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_get_sircs -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_get_sircs -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u16 *)buf + 1) = cmdack.output_word[1];
        *((u16 *)buf + 2) = cmdack.output_word[2];
        *((u16 *)buf + 3) = cmdack.output_word[3];
        *((u16 *)buf + 4) = cmdack.output_word[4];
    }
    return 0;
}

int dvrioctl2_get_time(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x5108;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_get_time -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_get_time -> Status error!\n");
        return -68;
    } else {
        *(u8 *)buf = cmdack.output_word[0];
        *((u8 *)buf + 1) = cmdack.output_word[1];
        *((u8 *)buf + 2) = cmdack.output_word[2];
        *((u8 *)buf + 3) = cmdack.output_word[3];
        *((u8 *)buf + 5) = cmdack.output_word[5];
        *((u8 *)buf + 6) = cmdack.output_word[6];
        *((u8 *)buf + 7) = cmdack.output_word[7];
    }
    return 0;
}

int dvrioctl2_set_timezone(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x5109;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word_count = 2;
    cmdack.input_word[1] = *((u16 *)arg + 1);
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_set_timezone -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_set_timezone -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_save_preset_info(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x510A;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_save_preset_info -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("dvrioctl2_save_preset_info -> Status error!\n");
        return -68;
    }
    return 0;
}

int dvrioctl2_load_preset_info(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x510B;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_load_preset_info -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("dvrioctl2_load_preset_info -> Status error!\n");
        return -68;
    }
    return 0;
}

int dvrioctl2_test_dev_rst(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x510C;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_test_dev_rst -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("dvrioctl2_test_dev_rst -> Status error!\n");
        return -68;
    }
    return 0;
}

int dvrioctl2_test_sdram_chk(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x510D;
    cmdack.input_word_count = 0;
    cmdack.timeout = 20000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_test_sdram_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("dvrioctl2_test_sdram_chk -> Status error!\n");
        return -68;
    } else {
        *(u32 *)buf = (cmdack.return_result_word[0] << 16) + cmdack.return_result_word[1];
        *((u32 *)buf + 1) = (cmdack.return_result_word[2] << 16) + cmdack.return_result_word[3];
        *((u32 *)buf + 2) = (cmdack.return_result_word[4] << 16) + cmdack.return_result_word[5];
    }
    return 0;
}

int dvrioctl2_test_mpe_chk(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x510E;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_test_mpe_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("dvrioctl2_test_mpe_chk -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.return_result_word[0];
    }
    return 0;
}

int dvrioctl2_test_mpd_chk(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x510F;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_test_mpd_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("dvrioctl2_test_mpd_chk -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.return_result_word[0];
    }
    return 0;
}

int dvrioctl2_test_vdec_chk(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x5110;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_test_vdec_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("dvrioctl2_test_vdec_chk -> Status error!\n");
        return -68;
    }
    return 0;
}

int dvrioctl2_partition_free(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v7;
    int v8;
    int v9;
    s64 v10;

    (void)a1;
    (void)cmd;
    (void)arglen;
    (void)buflen;

    v7 = iomanX_devctl((const char *)arg, 0x5002, 0, 0, 0, 0);
    v8 = v7;
    if (v7 < 0) {
        DPRINTF("dvrioctl2_partition_free : Cannot execute PDIOC_ZONEFREE.,%d\n", v7);
        return -5;
    }
    v9 = iomanX_devctl((const char *)arg, 0x5001, 0, 0, 0, 0);
    v10 = v8 * (s64)v9;
    if (v9 < 0) {
        DPRINTF("dvrioctl2_partition_free : Cannot execute PDIOC_ZONESZ.,%d\n", v9);
        return -5;
    }
    *(u64 *)buf = v10;
    if (*(s64 *)buf <= 0x7FFFFFF) {
        *(u32 *)buf = 0;
        *((u32 *)buf + 1) = 0;
    } else {
        *(u32 *)buf = v10 - 0x8000000;
        *((u32 *)buf + 1) = (v10 >> 32) - 1 + ((unsigned int)(v10 - 0x8000000) < 0xF8000000);
    }
    return 0;
}

int dvrioctl2_buzzer(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x5111;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_buzzer -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_buzzer -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_clr_preset_info(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buf;
    (void)buflen;

    cmdack.command = 0x5112;
    cmdack.input_word_count = 0;
    cmdack.timeout = 20000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("dvrioctl2_clr_preset_info -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_clr_preset_info -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int dvrioctl2_get_vbi_err_rate(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x5113;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_get_vbi_err_rate -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("dvrioctl2_get_vbi_err_rate -> Status error!\n");
            return -68;
        } else {
            *(u32 *)buf = (cmdack.output_word[0] << 16) + cmdack.output_word[1];
            *((u32 *)buf + 1) = (cmdack.output_word[2] << 16) + cmdack.output_word[3];
        }
    }
    return 0;
}

int dvrioctl2_update_dvrp_firmware(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int read_offset;
    int checksum;
    int retval;
    int update_fd;
    int update_size;
    int i;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)buf;
    (void)buflen;

    read_offset = 0;
    checksum = 0;
    retval = -1;
    update_fd = iomanX_open((const char *)arg, 1, 0x49, arglen);
    if (update_fd < 0) {
        retval = -2;
        goto LABEL_38;
    }
    update_size = iomanX_lseek(update_fd, 0, 2);
    if (iomanX_lseek(update_fd, 0, 0) < 0)
        goto LABEL_38;
    cmdack.command = 0x5114;
    cmdack.input_word[0] = update_size >> 16;
    cmdack.input_word[1] = update_size;
    cmdack.input_word_count = 2;
    cmdack.timeout = 10000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("FLASH_DATA_TOTALSIZE -> Handshake error!\n");
    LABEL_37:
        retval = -5;
        goto LABEL_38;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("FLASH_DATA_TOTALSIZE -> Status error!\n");
        goto LABEL_37;
    }
    if (cmdack.comp_status) {
        DPRINTF("FLASH_DATA_TOTALSIZE -> Status error!\n");
        goto LABEL_37;
    }
    if (update_size != (cmdack.return_result_word[0] << 16) + cmdack.return_result_word[1])
        DPRINTF("Size of firmware is not equal to Size of buffer on DVRP memory.\n");
    for (i = 0x3FFF;; i = 0x3FFF) {
        char *v13;
        int read_size;
        int j;
        v13 = &SBUF[0x3FFF];
        do {
            *v13 = 0;
            --i;
            --v13;
        } while (i >= 0);
        read_size = iomanX_read(update_fd, SBUF, 0x4000);
        if (read_size <= 0)
            break;
        cmdack.command = 0x5115;
        cmdack.input_word[0] = read_offset >> 16;
        cmdack.input_word[1] = read_offset;
        cmdack.input_word_count = 2;
        cmdack.input_buffer = SBUF;
        cmdack.input_buffer_length = read_size;
        cmdack.timeout = 10000000;
        if (DvrdrvExecCmdAckDmaSendComp(&cmdack)) {
            DPRINTF("MISCCMD_FLASH_DATA_DOWNLOAD -> Handshake error!\n");
            goto LABEL_37;
        }
        if (cmdack.ack_status_ack || (read_offset += read_size, cmdack.comp_status)) {
            DPRINTF("MISCCMD_FLASH_DATA_DOWNLOAD -> Status error!\n");
            goto LABEL_37;
        }
        for (j = 0; j < read_size; ++j) {
            checksum += (u8)SBUF[j];
        }
    }
    cmdack.command = 0x5116;
    cmdack.input_word[0] = update_size >> 16;
    cmdack.input_word[2] = checksum >> 16;
    cmdack.input_word[1] = update_size;
    cmdack.input_word[3] = checksum;
    cmdack.input_word[4] = 0;
    cmdack.input_word[5] = 0;
    cmdack.input_word_count = 6;
    cmdack.timeout = 10000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("MISCCMD_FLASH_DATA_CHECKSUM -> Handshake error!\n");
        goto LABEL_37;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("MISCCMD_FLASH_DATA_CHECKSUM -> ACK Status error!\n");
    } else {
        if (cmdack.comp_status) {
            retval = -68;
            DPRINTF("MISCCMD_FLASH_DATA_CHECKSUM -> COMP Status error!\n");
            DPRINTF(
                "Check sum error! IOP:%08X,DVRP:%08X\n",
                checksum,
                (cmdack.return_result_word[0] << 16) | cmdack.return_result_word[1]);
            goto LABEL_38;
        }
        cmdack.command = 0x5117;
        cmdack.input_word_count = 0;
        cmdack.timeout = 10000000;
        if (DvrdrvExecCmdAckComp(&cmdack)) {
            DPRINTF("MISCCMD_FLASH_DATA_WRITE -> Handshake error!\n");
            goto LABEL_37;
        }
        if (cmdack.ack_status_ack) {
            DPRINTF("MISCCMD_FLASH_DATA_WRITE -> ACK Status error!\n");
        } else {
            retval = 0;
            if (!cmdack.comp_status)
                goto LABEL_38;
            DPRINTF("MISCCMD_FLASH_DATA_WRITE -> COMP Status error!\n");
        }
    }
    retval = -68;
LABEL_38:
    iomanX_close(update_fd);
    return retval;
}

int dvrioctl2_flash_write_status(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x5118;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("dvrioctl2_flash_write_status -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_flash_write_status -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u16 *)buf + 1) = cmdack.output_word[1];
    }
    return 0;
}

int dvrioctl2_set_device_key(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
    int bsize;
    int *byteswap_tmp2;
    int *byteswap_tmp;
    int *byteswap_tmp_end;
    int cmdack_err3;
    int cmdack_err2;
    drvdrv_exec_cmd_ack cmdack;

    DPRINTF(
        "dvrioctl2_set_device_key (io=%p,cmd=%08X,argp=%p,arglen=%u,bufp=%p,buflen=%u)\n",
        a1,
        cmd,
        arg,
        arglen,
        buf,
        buflen);
    cmdack.command = 0x511B;
    cmdack.input_word[0] = 0;
    cmdack.input_word[1] = 456;
    cmdack.input_word_count = 2;
    cmdack.timeout = 10000000;
    DPRINTF("dvrcmd.cmd_p[0]:%x\n", 0);
    DPRINTF("dvrcmd.cmd_p[1]:%x\n", 456);
    cmdack_err = DvrdrvExecCmdAckComp(&cmdack);
    DPRINTF("dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
    if (cmdack_err) {
        DPRINTF("DEVKEY_TOTALSIZE -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("DEVKEY_TOTALSIZE -> Status error!\n");
        return -5;
    }
    bsize = (cmdack.return_result_word[0] << 16) + cmdack.return_result_word[1];
    if (bsize != 456)
        DPRINTF("Size of firmware is not equal to Size of buffer on DVRP memory.\n");
    DPRINTF("FSIZE:%08X\n", 456);
    DPRINTF("BSIZE:%08X\n", bsize);
    cmdack.command = 0x511C;
    ((u32 *)SBUF)[0] = *(u32 *)"XESD";
    byteswap_tmp2 = (int *)&SBUF[4];
    byteswap_tmp = (int *)arg;
    byteswap_tmp_end = (int *)((char *)arg + 448);
    do {
        byteswap_tmp2[0] = byteswap_tmp[0];
        byteswap_tmp2[1] = byteswap_tmp[1];
        byteswap_tmp2[2] = byteswap_tmp[2];
        byteswap_tmp2[3] = byteswap_tmp[3];
        byteswap_tmp += 4;
        byteswap_tmp2 += 4;
    } while (byteswap_tmp != byteswap_tmp_end);
    *byteswap_tmp2 = *byteswap_tmp;
    cmdack.input_word_count = 2;
    cmdack.input_word[0] = 0;
    cmdack.input_word[1] = 0;
    cmdack.input_buffer = &SBUF;
    cmdack.input_buffer_length = 456;
    cmdack.timeout = 10000000;
    cmdack_err3 = DvrdrvExecCmdAckDmaSendComp(&cmdack);
    DPRINTF("dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
    if (cmdack_err3) {
        DPRINTF("MISCCMD_DEVKEY_DOWNLOAD -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("MISCCMD_DEVKEY_DOWNLOAD -> Status error!\n");
        return -5;
    }
    if (cmdack.comp_status) {
        DPRINTF("MISCCMD_DEVKEY_DOWNLOAD -> Status error!\n");
        return -5;
    }
    cmdack.command = 0x5119;
    cmdack.input_word_count = 0;
    cmdack.timeout = 10000000;
    cmdack_err2 = DvrdrvExecCmdAckComp(&cmdack);
    DPRINTF("dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
    if (cmdack_err2) {
        DPRINTF("MISCCMD_SAVE_DEVKEY_INFO -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("MISCCMD_SAVE_DEVKEY_INFO -> Status error!\n");
        return -68;
    }
    if (cmdack.comp_status) {
        DPRINTF("MISCCMD_SAVE_DEVKEY_INFO -> Status error!\n");
        return -5;
    }
    return 0;
}

int dvrioctl2_get_device_key(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int v6;
    u16 *return_result_word;
    int busywait;
    drvdrv_exec_cmd_ack cmdack;
    u8 v27[8];

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x511A;
    cmdack.input_word[0] = 0;
    cmdack.input_word[1] = 4;
    cmdack.input_word_count = 2;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("MISCCMD_GET_DEVKEY_INFO -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("MISCCMD_GET_DEVKEY_INFO -> Status error!\n");
        return -68;
    }
    if (cmdack.comp_status) {
        DPRINTF("MISCCMD_GET_DEVKEY_INFO -> Status error!\n");
        return -5;
    }
    v6 = 0;
    return_result_word = cmdack.return_result_word;
    do {
        u8 *v8;
        v8 = &v27[v6];
        v6 += 2;
        v8[0] = (*return_result_word & 0xFF00) >> 8;
        v8[1] = (*return_result_word & 0x00FF);
        return_result_word += 1;
    } while (v6 < 4);
    busywait = 2;
    do {
    } while (busywait-- >= 0);
    if (memcmp(v27, "XESD", 4) == 0) {
        int v13;
        u16 *in_word_tmp;
        u16 v17;
        int v18;
        int v19;
        unsigned int v20;
        v13 = 0;
        in_word_tmp = &cmdack.return_result_word[2];
        do {
            u8 *v15;
            v15 = (u8 *)buf + v13;
            v13 += 2;
            v15[0] = (*in_word_tmp & 0xFF00) >> 8;
            v15[1] = (*in_word_tmp & 0x00FF);
            in_word_tmp += 1;
        } while (v13 < 4);
        v17 = 4;
        v18 = 224;
        v19 = 0;
        v20 = 224;
        while (1) {
            int v21;
            int v22;
            v21 = v18;
            if (v20 >= 0x11)
                v21 = 16;
            cmdack.command = 0x511A;
            cmdack.input_word[0] = v17;
            cmdack.input_word[1] = v21;
            cmdack.input_word_count = 2;
            cmdack.timeout = 30000000;
            if (DvrdrvExecCmdAckComp(&cmdack)) {
                DPRINTF("MISCCMD_GET_DEVKEY_INFO -> Handshake error!\n");
                return -5;
            }
            if (cmdack.ack_status_ack) {
                DPRINTF("MISCCMD_GET_DEVKEY_INFO -> Status error!\n");
                return -68;
            }
            v22 = 1;
            if (cmdack.comp_status) {
                DPRINTF("MISCCMD_GET_DEVKEY_INFO -> Status error!\n");
                return -5;
            }
            if ((u16)v21 + 1 > 1) {
                u16 *in_word_tmp2;
                in_word_tmp2 = cmdack.return_result_word;
                do {
                    u8 *v24;
                    v24 = (u8 *)buf + v19;
                    v19 += 2;
                    v22 += 1;
                    v24[4] = (*in_word_tmp2 & 0xFF00) >> 8;
                    v24[5] = (*in_word_tmp2 & 0x00FF);
                    in_word_tmp2 += 1;
                } while (v22 < (u16)v21 + 1);
            }
            v18 -= v21;
            v20 = (u16)v18;
            v17 += v21;
            if (!(u16)v18)
                return 0;
        }
    }
    return -68;
}

int dvrioctl2_set_dv_nodeid(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    unsigned int argwalked;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)buf;
    (void)buflen;

    argwalked = 0;
    cmdack.command = 0x511D;
    if (arglen) {
        u8 *inword_tmp;
        inword_tmp = ((u8 *)&cmdack.input_word[0]);
        do {
            u8 *inword_tmp2;
            inword_tmp2 = ((u8 *)arg + argwalked);
            inword_tmp[0] = inword_tmp2[1];
            inword_tmp[1] = inword_tmp2[0];
            inword_tmp += 2;
            argwalked += 2;
        } while (argwalked < arglen);
    }
    cmdack.input_word_count = 4;
    cmdack.timeout = 10000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("MISCCMD_SAVE_DV_NODEID -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("MISCCMD_SAVE_DV_NODEID -> Status error!\n");
        return -68;
    }
    if (cmdack.comp_status) {
        DPRINTF("MISCCMD_SAVE_DV_NODEID -> Status error!\n");
        return -5;
    }
    return 0;
}

int dvrioctl2_get_dv_nodeid(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    unsigned int bufwalked;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arg;
    (void)arglen;

    cmdack.command = 0x511E;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("MISCCMD_GET_DV_NODEID -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("MISCCMD_GET_DV_NODEID -> Status error!\n");
        return -68;
    }
    if (cmdack.comp_status) {
        DPRINTF("MISCCMD_GET_DV_NODEID -> Status error!\n");
        return -5;
    }
    bufwalked = 0;
    if (buflen) {
        u16 *return_result_word;
        int bufbusyloop;

        return_result_word = cmdack.return_result_word;
        do {
            char *buftmp;
            buftmp = (char *)buf + bufwalked;
            bufwalked += 2;
            buftmp[0] = (*return_result_word & 0xFF00) >> 8;
            buftmp[1] = (*return_result_word & 0x00FF);
            return_result_word += 1;
        } while (bufwalked < buflen);

        bufbusyloop = 0;
        while ((unsigned int)(bufbusyloop++) < buflen)
            ;
    }
    return 0;
}
#ifdef DEBUG
int test_count = 0;
#endif
int dvrioctl2_diag_test(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int cmdack_err;
#ifdef DEBUG
    int testcnt_tmp;
#endif
    int outbuf_cnt;
    u16 *outbuf_tmp;
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)cmd;
    (void)arglen;
    (void)buflen;

    cmdack.command = 0x511F;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word[1] = *((u16 *)arg + 1);
    cmdack.input_word[2] = *((u16 *)arg + 2);
    cmdack.input_word[3] = *((u16 *)arg + 3);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word_count = 5;
    cmdack.timeout = 120000000;
    if (cmdack.input_word[2]) {
        DPRINTF("------------------- > SetTO:%d msec\n", 120000);
        cmdack.timeout = 10000000 * cmdack.input_word[2];
    }
    DPRINTF("arg :  %4x", cmdack.input_word[0]);
    DPRINTF(" %4x", cmdack.input_word[1]);
    DPRINTF(" %4x ", cmdack.input_word[2]);
    DPRINTF(" SetTimeOutTo : %d usec\n", cmdack.timeout);
    cmdack_err = DvrdrvExecCmdAckComp(&cmdack);
#ifdef DEBUG
    testcnt_tmp = test_count++;
#endif
    DPRINTF(
        "cmd ID : %d -------------------- TEST VERSION - diag test -------------------- r %d -- c %d\n",
        31,
        cmdack_err,
        testcnt_tmp);
    DPRINTF("diag_test dvrcmd.ack_p[0]:%x\n", cmdack.ack_status_ack);
    DPRINTF("diag_test dvrcmd.phase:%x\n", cmdack.phase);
    if (cmdack_err) {
        DPRINTF("dvrioctl2_diag_test -> Handshake error!\n");
        return -5;
    }
    if (cmdack.ack_status_ack) {
        DPRINTF("dvrioctl2_diag_test -> Status error in ACK! param:%04x\n", cmdack.ack_status_ack);
        return -68;
    }
    if (cmdack.comp_status) {
        DPRINTF("dvrioctl2_diag_test -> Status error in COMP! param:%04x\n", cmdack.comp_status);
        return -68;
    }
    outbuf_cnt = 0;
    DPRINTF("---------------------------- return buffer\n");
    outbuf_tmp = (u16 *)&cmdack.return_result_word[0];
    do {
        outbuf_cnt += 1;
        DPRINTF(" %4x", *outbuf_tmp);
        *(u16 *)buf = *outbuf_tmp;
        outbuf_tmp += 1;
        buf = (char *)buf + 2;
    } while (outbuf_cnt < 16);
    DPRINTF("\n");
    return 0;
}
