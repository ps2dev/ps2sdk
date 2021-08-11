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
extern int dvrmisc_df_init(iop_device_t *dev);
extern int dvrmisc_df_exit(iop_device_t *dev);
extern int dvrmisc_df_ioctl(iop_file_t *f, int cmd, void *param);
extern int dvrmisc_df_devctl(iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrmisc_df_ioctl2(iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrmisc_df_null();
extern s64 dvrmisc_df_null_long();
extern int dvrioctl2_nop(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_version(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_led_hdd_rec(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_led_dvd_rec(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_sircs(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_time(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_set_timezone(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_save_preset_info(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_load_preset_info(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_dev_rst(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_sdram_chk(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_mpe_chk(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_mpd_chk(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_test_vdec_chk(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_partition_free(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_buzzer(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_clr_preset_info(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_get_vbi_err_rate(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_update_dvrp_firmware(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrioctl2_flash_write_status(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

struct DevctlCmdTbl_t
{
    u16 cmd;
    int (*fn)(iop_file_t *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[20] =
    {
        {22120u, &dvrioctl2_get_sircs},
        {22118u, &dvrioctl2_led_hdd_rec},
        {22119u, &dvrioctl2_led_dvd_rec},
        {22121u, &dvrioctl2_get_time},
        {22122u, &dvrioctl2_set_timezone},
        {22123u, &dvrioctl2_save_preset_info},
        {22124u, &dvrioctl2_load_preset_info},
        {22116u, &dvrioctl2_nop},
        {22117u, &dvrioctl2_version},
        {22125u, &dvrioctl2_test_dev_rst},
        {22126u, &dvrioctl2_test_sdram_chk},
        {22127u, &dvrioctl2_test_mpe_chk},
        {22128u, &dvrioctl2_test_mpd_chk},
        {22129u, &dvrioctl2_test_vdec_chk},
        {22130u, &dvrioctl2_partition_free},
        {22131u, &dvrioctl2_buzzer},
        {22132u, &dvrioctl2_clr_preset_info},
        {22133u, &dvrioctl2_get_vbi_err_rate},
        {22134u, &dvrioctl2_update_dvrp_firmware},
        {22135u, &dvrioctl2_flash_write_status}};
struct _iop_device_ops DvrFuncTbl =
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
        &dvrmisc_df_ioctl2};
iop_device_t DVRMISC;
s32 sema_id;
char SBUF[16384];

// Based off of DESR / PSX DVR system software version 1.31.
#define MODNAME "DVRMISC"
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
        if (((*((vu32 *)0xB0004230)) & 4) != 0)
            break;
        DelayThread(1000);
    }
    if (i == 30000) {
        printf("MISC task of DVRP is not running...\n");
        return 1;
    } else {
        DVRMISC.name = "dvr_misc";
        DVRMISC.desc = "Digital Video Recorder MISC task";
        DVRMISC.type = 0x10000010;
        DVRMISC.ops = &DvrFuncTbl;
        v1 = AddDrv(&DVRMISC) == 0;
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

    v0 = DelDrv("dvr_misc") == 0;
    result = 1;
    if (!v0)
        return 2;
    return result;
}

int dvrmisc_df_init(iop_device_t *dev)
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

int dvrmisc_df_exit(iop_device_t *dev)
{
    bool v1;
    int result;

    v1 = DeleteSema(sema_id) == 0;
    result = 0;
    if (!v1)
        return -1;
    return result;
}

int dvrmisc_df_ioctl(iop_file_t *f, int cmd, void *param)
{
    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvrmisc_df_devctl(
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
        if (v11 >= 20)
            goto LABEL_5;
    }
    v10 = DevctlCmdTbl[v12].fn(a1, cmd, arg, arglen, buf, buflen);
LABEL_5:
    if (v11 == 20)
        v10 = -22;
    SignalSema(sema_id);
    return v10;
}

int dvrmisc_df_ioctl2(
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

int dvrmisc_df_null()
{
    return -48;
}

s64 dvrmisc_df_null_long()
{
    return -48LL;
}

int dvrioctl2_nop(iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5101;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_nop -> Handshake error!\n");
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_nop -> Status error!\n");
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_version(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5102;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_version -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_version -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        retval = 0;
        *((u16 *)buf + 1) = cmdack.output_word[1];
    }
    return retval;
}

int dvrioctl2_led_hdd_rec(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v6;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5104;
    v6 = *(u16 *)arg;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = v6;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_led_hdd_rec -> Handshake error!\n");
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_led_hdd_rec -> Status error!\n");
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_led_dvd_rec(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v6;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5106;
    v6 = *(u16 *)arg;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = v6;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_led_dvd_rec -> Handshake error!\n");
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_led_dvd_rec -> Status error!\n");
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_get_sircs(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5107;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_get_sircs -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_get_sircs -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u16 *)buf + 1) = cmdack.output_word[1];
        *((u16 *)buf + 2) = cmdack.output_word[2];
        *((u16 *)buf + 3) = cmdack.output_word[3];
        retval = 0;
        *((u16 *)buf + 4) = cmdack.output_word[4];
    }
    return retval;
}

int dvrioctl2_get_time(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5108;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_get_time -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_get_time -> Status error!\n");
        return -68;
    } else {
        *(u8 *)buf = cmdack.output_word[0];
        *((u8 *)buf + 1) = cmdack.output_word[1];
        *((u8 *)buf + 2) = cmdack.output_word[2];
        *((u8 *)buf + 3) = cmdack.output_word[3];
        *((u8 *)buf + 5) = cmdack.output_word[5];
        *((u8 *)buf + 6) = cmdack.output_word[6];
        retval = 0;
        *((u8 *)buf + 7) = cmdack.output_word[7];
    }
    return retval;
}

int dvrioctl2_set_timezone(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    u16 v6;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5109;
    cmdack.input_word[0] = *(u16 *)arg;
    v6 = *((u16 *)arg + 1);
    cmdack.input_word_count = 2;
    cmdack.input_word[1] = v6;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_set_timezone -> Handshake error!\n");
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_set_timezone -> Status error!\n");
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_save_preset_info(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x510A;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_save_preset_info -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || (retval = 0, cmdack.comp_status)) {
        printf("dvrioctl2_save_preset_info -> Status error!\n");
        return -68;
    }
    return retval;
}

int dvrioctl2_load_preset_info(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x510B;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_load_preset_info -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || (retval = 0, cmdack.comp_status)) {
        printf("dvrioctl2_load_preset_info -> Status error!\n");
        return -68;
    }
    return retval;
}

int dvrioctl2_test_dev_rst(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x510C;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_test_dev_rst -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || (retval = 0, cmdack.comp_status)) {
        printf("dvrioctl2_test_dev_rst -> Status error!\n");
        return -68;
    }
    return retval;
}

int dvrioctl2_test_sdram_chk(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x510D;
    cmdack.input_word_count = 0;
    cmdack.timeout = 20000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_test_sdram_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || (retval = 0, cmdack.comp_status)) {
        printf("dvrioctl2_test_sdram_chk -> Status error!\n");
        return -68;
    } else {
        *(u32 *)buf = (cmdack.return_result_word[0] << 16) + cmdack.return_result_word[1];
        *((u32 *)buf + 1) = (cmdack.return_result_word[2] << 16) + cmdack.return_result_word[3];
        *((u32 *)buf + 2) = (cmdack.return_result_word[4] << 16) + cmdack.return_result_word[5];
    }
    return retval;
}

int dvrioctl2_test_mpe_chk(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x510E;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_test_mpe_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || (retval = 0, cmdack.comp_status)) {
        printf("dvrioctl2_test_mpe_chk -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.return_result_word[0];
    }
    return retval;
}

int dvrioctl2_test_mpd_chk(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x510F;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_test_mpd_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || (retval = 0, cmdack.comp_status)) {
        printf("dvrioctl2_test_mpd_chk -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.return_result_word[0];
    }
    return retval;
}

int dvrioctl2_test_vdec_chk(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5110;
    cmdack.input_word_count = 0;
    cmdack.timeout = 15000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_test_vdec_chk -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || (retval = 0, cmdack.comp_status)) {
        printf("dvrioctl2_test_vdec_chk -> Status error!\n");
        return -68;
    }
    return retval;
}

int dvrioctl2_partition_free(
    iop_file_t *a1,
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

    v7 = devctl((const char *)arg, 0x5002, 0, 0, 0, 0);
    v8 = v7;
    if (v7 < 0) {
        printf("dvrioctl2_partition_free : Cannot execute PDIOC_ZONEFREE.,%d\n", v7);
        return -5;
    }
    v9 = devctl((const char *)arg, 0x5001, 0, 0, 0, 0);
    v10 = v8 * (s64)v9;
    if (v9 < 0) {
        printf("dvrioctl2_partition_free : Cannot execute PDIOC_ZONESZ.,%d\n", v9);
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
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5111;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_buzzer -> Handshake error!\n");
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_buzzer -> Status error!\n");
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_clr_preset_info(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5112;
    cmdack.input_word_count = 0;
    cmdack.timeout = 20000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        printf("dvrioctl2_clr_preset_info -> Handshake error!\n");
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_clr_preset_info -> Status error!\n");
            return -68;
        }
    }
    return retval;
}

int dvrioctl2_get_vbi_err_rate(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5113;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_get_vbi_err_rate -> Handshake error!\n");
        return -5;
    } else {
        retval = 0;
        if (cmdack.ack_status_ack) {
            printf("dvrioctl2_get_vbi_err_rate -> Status error!\n");
            return -68;
        } else {
            *(u32 *)buf = (cmdack.output_word[0] << 16) + cmdack.output_word[1];
            *((u32 *)buf + 1) = (cmdack.output_word[2] << 16) + cmdack.output_word[3];
        }
    }
    return retval;
}

int dvrioctl2_update_dvrp_firmware(
    iop_file_t *a1,
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
    const char *fmterr;
    int i;
    char *v13;
    int read_size;
    int j;
    int read_tmp;
    const char *fmterr_;
    drvdrv_exec_cmd_ack cmdack;

    read_offset = 0;
    checksum = 0;
    retval = -1;
    update_fd = open((const char *)arg, 1, 0x49, arglen);
    if (update_fd < 0) {
        retval = -2;
        goto LABEL_38;
    }
    update_size = lseek(update_fd, 0, 2);
    if (lseek(update_fd, 0, 0) < 0)
        goto LABEL_38;
    cmdack.command = 0x5114;
    cmdack.input_word[0] = update_size >> 16;
    cmdack.input_word[1] = update_size;
    cmdack.input_word_count = 2;
    cmdack.timeout = 10000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        fmterr = "FLASH_DATA_TOTALSIZE -> Handshake error!\n";
    LABEL_37:
        retval = -5;
        printf(fmterr);
        goto LABEL_38;
    }
    if (cmdack.ack_status_ack) {
        fmterr = "FLASH_DATA_TOTALSIZE -> Status error!\n";
        goto LABEL_37;
    }
    if (cmdack.comp_status) {
        fmterr = "FLASH_DATA_TOTALSIZE -> Status error!\n";
        goto LABEL_37;
    }
    if (update_size != (cmdack.return_result_word[0] << 16) + cmdack.return_result_word[1])
        printf("Size of firmware is not equal to Size of buffer on DVRP memory.\n");
    for (i = 0x3FFF;; i = 0x3FFF) {
        v13 = &SBUF[0x3FFF];
        do {
            *v13 = 0;
            --i;
            --v13;
        } while (i >= 0);
        read_size = read(update_fd, SBUF, 0x4000);
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
            fmterr = "MISCCMD_FLASH_DATA_DOWNLOAD -> Handshake error!\n";
            goto LABEL_37;
        }
        if (cmdack.ack_status_ack || (read_offset += read_size, cmdack.comp_status)) {
            fmterr = "MISCCMD_FLASH_DATA_DOWNLOAD -> Status error!\n";
            goto LABEL_37;
        }
        for (j = 0; j < read_size; ++j) {
            read_tmp = (u8)SBUF[j];
            checksum += read_tmp;
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
        fmterr = "MISCCMD_FLASH_DATA_CHECKSUM -> Handshake error!\n";
        goto LABEL_37;
    }
    if (cmdack.ack_status_ack) {
        fmterr_ = "MISCCMD_FLASH_DATA_CHECKSUM -> ACK Status error!\n";
    } else {
        if (cmdack.comp_status) {
            retval = -68;
            printf("MISCCMD_FLASH_DATA_CHECKSUM -> COMP Status error!\n");
            printf(
                "Check sum error! IOP:%08lX,DVRP:%08lX\n",
                checksum,
                (cmdack.return_result_word[0] << 16) | cmdack.return_result_word[1]);
            goto LABEL_38;
        }
        cmdack.command = 0x5117;
        cmdack.input_word_count = 0;
        cmdack.timeout = 10000000;
        if (DvrdrvExecCmdAckComp(&cmdack)) {
            fmterr = "MISCCMD_FLASH_DATA_WRITE -> Handshake error!\n";
            goto LABEL_37;
        }
        if (cmdack.ack_status_ack) {
            fmterr_ = "MISCCMD_FLASH_DATA_WRITE -> ACK Status error!\n";
        } else {
            retval = 0;
            if (!cmdack.comp_status)
                goto LABEL_38;
            fmterr_ = "MISCCMD_FLASH_DATA_WRITE -> COMP Status error!\n";
        }
    }
    retval = -68;
    printf(fmterr_);
LABEL_38:
    close(update_fd);
    return retval;
}

int dvrioctl2_flash_write_status(
    iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    cmdack.command = 0x5118;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        printf("dvrioctl2_flash_write_status -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        printf("dvrioctl2_flash_write_status -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        retval = 0;
        *((u16 *)buf + 1) = cmdack.output_word[1];
    }
    return retval;
}
