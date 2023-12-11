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
#define MODNAME "DVRAV"

#ifdef DEBUG
#define DPRINTF(x...) printf(MODNAME ": " x)
#else
#define DPRINTF(x...)
#endif

extern int module_start();
extern int module_stop();
extern int dvrav_df_init(iomanX_iop_device_t *dev);
extern int dvrav_df_exit(iomanX_iop_device_t *dev);
extern int dvrav_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param);
extern int dvrav_df_devctl(iomanX_iop_file_t *a1, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrav_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrav_df_null();
extern s64 dvrav_df_null_long();
extern int avioctl2_select_position(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_position(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_position_up(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_position_down(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_set_d_audio_sel(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_tun_offset(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_tun_offset_up(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_tun_offset_down(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_tun_scan_ch(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_bs_gain(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_set_preset_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_change_sound(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_set_d_video_sel(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_av_src(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_preset_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_set_position_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_position_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_tun_scan_mode(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_f_select_position(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_select_rec_src(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_rec_src(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_tun_scan_mode_euro(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_tun_scan_ch_euro(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_curfreq_info(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_teletext_ver_no(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_set_tv_guide_page(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_tv_guide_page(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_change_mode_tv_to_dvd(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_vps_data(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_pdc_data(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_format1_data(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_get_header_time_data(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int avioctl2_set_acs_position_euro(iomanX_iop_file_t *a1, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

struct DevctlCmdTbl_t
{
    u16 cmd;
    int (*fn)(iomanX_iop_file_t *, int, void *, unsigned int, void *, unsigned int);
} DevctlCmdTbl[33] =
    {
        {0x5616, &avioctl2_select_position},
        {0x5619, &avioctl2_get_position},
        {0x5617, &avioctl2_position_up},
        {0x5618, &avioctl2_position_down},
        {0x5612, &avioctl2_set_d_audio_sel},
        {0x560B, &avioctl2_get_tun_offset},
        {0x560C, &avioctl2_tun_offset_up},
        {0x560D, &avioctl2_tun_offset_down},
        {0x560E, &avioctl2_tun_scan_ch},
        {0x560F, &avioctl2_get_bs_gain},
        {0x5610, &avioctl2_set_preset_info},
        {0x5611, &avioctl2_change_sound},
        {0x5613, &avioctl2_set_d_video_sel},
        {0x5614, &avioctl2_get_av_src},
        {0x5615, &avioctl2_get_preset_info},
        {0x561A, &avioctl2_set_position_info},
        {0x561B, &avioctl2_get_position_info},
        {0x561C, &avioctl2_tun_scan_mode},
        {0x561D, &avioctl2_f_select_position},
        {0x561E, &avioctl2_select_rec_src},
        {0x561F, &avioctl2_get_rec_src},
        {0x5618, &avioctl2_tun_scan_mode_euro},
        {0x5619, &avioctl2_tun_scan_ch_euro},
        {0x561A, &avioctl2_get_curfreq_info},
        {0x561B, &avioctl2_get_teletext_ver_no},
        {0x561C, &avioctl2_set_tv_guide_page},
        {0x561D, &avioctl2_get_tv_guide_page},
        {0x561E, &avioctl2_change_mode_tv_to_dvd},
        {0x561F, &avioctl2_get_vps_data},
        {0x5620, &avioctl2_get_pdc_data},
        {0x5621, &avioctl2_get_format1_data},
        {0x5622, &avioctl2_get_header_time_data},
        {0x5623, &avioctl2_set_acs_position_euro},
};

static iomanX_iop_device_ops_t DvrFuncTbl =
    {
        &dvrav_df_init,
        &dvrav_df_exit,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        &dvrav_df_ioctl,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null_long,
        &dvrav_df_devctl,
        (void *)&dvrav_df_null,
        (void *)&dvrav_df_null,
        &dvrav_df_ioctl2,
    };
static iomanX_iop_device_t DVRAV = {
    .name = "dvr_av",
    .desc = "Digital Video Recorder AV task",
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
        if (((*((vu32 *)0xB0004230)) & 8) != 0)
            break;
        DelayThread(1000);
    }
    if (i == 30000) {
        DPRINTF("AV task of DVRP is not running...\n");
        return MODULE_NO_RESIDENT_END;
    } else {
        if (iomanX_AddDrv(&DVRAV) != 0)
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
    if (iomanX_DelDrv(DVRAV.name) != 0)
        return MODULE_REMOVABLE_END;
    return MODULE_NO_RESIDENT_END;
}

int dvrav_df_init(iomanX_iop_device_t *dev)
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

int dvrav_df_exit(iomanX_iop_device_t *dev)
{
    (void)dev;

    if (DeleteSema(sema_id) != 0)
        return -1;
    return 0;
}

int dvrav_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param)
{
    (void)f;
    (void)cmd;
    (void)param;

    WaitSema(sema_id);
    SignalSema(sema_id);
    return -22;
}

int dvrav_df_devctl(
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

int dvrav_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
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

int dvrav_df_null()
{
    return -48;
}

s64 dvrav_df_null_long()
{
    return -48LL;
}

int avioctl2_get_tun_offset(
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

    cmdack.command = 0x3102;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_get_tun_offset -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("avioctl2_get_tun_offset -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u16 *)buf + 1) = cmdack.output_word[1];
    }
    return 0;
}

int avioctl2_tun_offset_up(
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

    cmdack.command = 0x3103;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_tun_offset_up -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("avioctl2_tun_offset_up -> Status error!\n");
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

int avioctl2_tun_offset_down(
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

    cmdack.command = 0x3104;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_tun_offset_down -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("avioctl2_tun_offset_down -> Status error!\n");
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

int avioctl2_tun_scan_ch(
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
    (void)buflen;

    cmdack.command = 0x3105;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word[1] = *((u16 *)arg + 1);
    cmdack.input_word[2] = *((u16 *)arg + 2);
    cmdack.input_word_count = 3;
    cmdack.timeout = 35000000;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("avioctl2_tun_scan_ch -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("avioctl2_tun_scan_ch -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.return_result_word[0];
        *((u16 *)buf + 1) = cmdack.return_result_word[1];
        *((u16 *)buf + 2) = cmdack.return_result_word[2];
        *((u16 *)buf + 3) = cmdack.return_result_word[3];
        *((u16 *)buf + 4) = cmdack.return_result_word[4];
        *((u16 *)buf + 5) = cmdack.return_result_word[5];
    }
    return 0;
}

int avioctl2_get_bs_gain(
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

    cmdack.command = 0x3106;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_get_bs_gain -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_get_bs_gain -> Status error!\n");
            return -68;
        } else {
            *(u16 *)buf = cmdack.output_word[0];
        }
    }
    return 0;
}

int avioctl2_set_preset_info(
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

    cmdack.command = 0x3107;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word[1] = *((u16 *)arg + 1);
    cmdack.input_word_count = 2;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_set_preset_info -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_set_preset_info -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int avioctl2_change_sound(
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

    cmdack.command = 0x3108;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_change_sound -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_change_sound -> Status error!\n");
            return -68;
        } else {
            *(u16 *)buf = cmdack.output_word[0];
        }
    }
    return 0;
}

int avioctl2_set_d_audio_sel(
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

    cmdack.command = 0x3109;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word[1] = *((u16 *)arg + 1);
    cmdack.input_word[2] = *((u16 *)arg + 2);
    cmdack.input_word[3] = *((u16 *)arg + 3);
    cmdack.input_word_count = 4;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_set_d_audio_sel -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_set_d_audio_sel -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int avioctl2_set_d_video_sel(
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

    cmdack.command = 0x310A;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_set_d_video_sel -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_set_d_video_sel -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int avioctl2_get_av_src(
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

    cmdack.command = 0x310B;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_get_av_src -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("avioctl2_get_av_src -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        *((u16 *)buf + 1) = cmdack.output_word[1];
    }
    return 0;
}

int avioctl2_get_preset_info(
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
    (void)buflen;

    cmdack.command = 0x310C;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_get_preset_info -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_get_preset_info -> Status error!\n");
            return -68;
        } else {
            *(u16 *)buf = cmdack.output_word[0];
        }
    }
    return 0;
}

int avioctl2_select_position(
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

    cmdack.command = 0x310E;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_select_position -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_select_position -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int avioctl2_position_up(
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

    cmdack.command = 0x310F;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_position_up -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_position_up -> Status error!\n");
            return -68;
        } else {
            *(u16 *)buf = cmdack.output_word[0];
        }
    }
    return 0;
}

int avioctl2_position_down(
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

    cmdack.command = 0x3110;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_position_down -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_position_down -> Status error!\n");
            return -68;
        } else {
            *(u16 *)buf = cmdack.output_word[0];
        }
    }
    return 0;
}

int avioctl2_get_position(
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

    cmdack.command = 0x3111;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_get_position -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("avioctl2_get_position -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = cmdack.output_word[0];
        DPRINTF("Now position = %d\n", cmdack.output_word[0]);
        return 0;
    }
}

int avioctl2_set_position_info(
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

    cmdack.command = 0x3112;
    cmdack.input_word[0] = *(u16 *)arg;
    cmdack.input_word[1] = *((u16 *)arg + 1);
    cmdack.input_word[2] = *((u16 *)arg + 2);
    cmdack.input_word[3] = *((u16 *)arg + 3);
    cmdack.input_word[4] = *((u16 *)arg + 4);
    cmdack.input_word_count = 5;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_set_position_info -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_set_position_info -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int avioctl2_get_position_info(
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
    (void)buflen;

    cmdack.command = 0x3113;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_get_position_info -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack) {
        DPRINTF("avioctl2_get_position_info -> Status error!\n");
        return -68;
    } else {
        *(u16 *)buf = *(u16 *)arg;
        *((u16 *)buf + 1) = cmdack.output_word[0];
        *((u16 *)buf + 2) = cmdack.output_word[1];
        *((u16 *)buf + 3) = cmdack.output_word[2];
        *((u16 *)buf + 4) = cmdack.output_word[3];
    }
    return 0;
}

int avioctl2_tun_scan_mode(
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

    cmdack.command = 0x3114;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_tun_scan_mode -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_tun_scan_mode -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int avioctl2_f_select_position(
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

    cmdack.command = 0x3115;
    cmdack.input_word_count = 1;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_f_select_position -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_f_select_position -> Status error!\n");
            return -68;
        }
    }
    return 0;
}

int avioctl2_select_rec_src(
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

    cmdack.command = 0x3116;
    cmdack.input_word_count = 1;
    cmdack.timeout = 10000000;
    cmdack.input_word[0] = *(u16 *)arg;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF("avioctl2_select_rec_src -> Handshake error!\n");
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF("avioctl2_select_rec_src -> Status error!\n");
        return -68;
    }
    return 0;
}

int avioctl2_get_rec_src(
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

    cmdack.command = 0x3117;
    cmdack.input_word_count = 0;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF("avioctl2_get_rec_src -> Handshake error!\n");
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF("avioctl2_get_rec_src -> Status error!\n");
            return -68;
        } else {
            *(u16 *)buf = cmdack.output_word[0];
        }
    }
    return 0;
}

int avioctl2_cmd_ack(
    const char *a1,
    u32 a2,
    iomanX_iop_file_t *a3,
    u8 cmd,
    void *arg,
    unsigned int arglen,
    void *buf)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a2;
    (void)a3;

    cmdack.command = cmd | 0x3100;
    if (arglen >> 1) {
        u16 *input_word_tmp;
        unsigned int input_word_copied;
        input_word_tmp = (u16 *)&cmdack.input_word[0];
        input_word_copied = 0;
        do {
            *input_word_tmp = *(u16 *)arg;
            arg = (char *)arg + 2;
            input_word_copied += 1;
            input_word_tmp += 1;
        } while (input_word_copied < arglen >> 1);
    }
    cmdack.input_word_count = arglen >> 1;
    if (DvrdrvExecCmdAck(&cmdack)) {
        DPRINTF(" %s  -> Handshake error!\n", a1);
        return -5;
    } else {
        if (cmdack.ack_status_ack) {
            DPRINTF(" %s  -> Status error!\n", a1);
            return -68;
        } else {
            u16 *input_word;
            char *buf_tmp;
            int out_count;
            input_word = cmdack.output_word;
            buf_tmp = (char *)buf;
            out_count = 1;
            do {
                *(u16 *)buf_tmp = *input_word;
                out_count += 1;
                input_word += 1;
                buf_tmp += 2;
            } while (out_count < 16);
            return 0;
        }
    }
}

int avioctl2_cmd_ack_comp(
    const char *a1,
    u32 a2,
    iomanX_iop_file_t *a3,
    u8 cmd,
    void *arg,
    unsigned int arglen,
    void *buf)
{
    drvdrv_exec_cmd_ack cmdack;

    (void)a1;
    (void)a2;
    (void)a3;

    cmdack.command = cmd | 0x3100;
    if (arglen >> 1) {
        u16 *input_word_tmp;
        unsigned int input_word_copied;
        input_word_tmp = (u16 *)&cmdack.input_word[0];
        input_word_copied = 0;
        do {
            *input_word_tmp = *(u16 *)arg;
            input_word_tmp += 1;
            arg = (char *)arg + 2;
            input_word_copied += 1;
        } while (input_word_copied < arglen >> 1);
    }
    cmdack.input_word_count = arglen >> 1;
    cmdack.timeout = a2;
    if (DvrdrvExecCmdAckComp(&cmdack)) {
        DPRINTF(" %s  -> Handshake error!\n", a1);
        return -5;
    } else if (cmdack.ack_status_ack || cmdack.comp_status) {
        DPRINTF(" %s  -> Status error!\n", a1);
        return -68;
    } else {
        int out_count;
        u16 *input_word;
        u16 *buf_tmp;
        out_count = 1;
        input_word = cmdack.return_result_word;
        buf_tmp = (u16 *)buf;
        do {
            *buf_tmp = *input_word;
            out_count += 1;
            input_word += 1;
            buf_tmp += 1;
        } while (out_count < 16);
        return 0;
    }
}

int avioctl2_tun_scan_mode_euro(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack("avioctl2_tun_scan_mode_euro", 0x8000, a1, cmd, arg, arglen, buf);
}

int avioctl2_tun_scan_ch_euro(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack_comp("avioctl2_tun_scan_ch_euro", 0x8000, a1, cmd, arg, arglen, buf);
}

int avioctl2_get_curfreq_info(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack("avioctl2_get_curfreq_info", 0x8000, a1, cmd, arg, arglen, buf);
}

int avioctl2_get_teletext_ver_no(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack_comp("avioctl2_get_teletext_ver_no", 0x8000, a1, cmd, arg, arglen, buf);
}

int avioctl2_set_tv_guide_page(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack("avioctl2_set_tv_guide_page", 0x8000, a1, cmd, arg, arglen, buf);
}

int avioctl2_get_tv_guide_page(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack("avioctl2_get_tv_guide_page", 0x8000, a1, cmd, arg, arglen, buf);
}

int avioctl2_change_mode_tv_to_dvd(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack("avioctl2_change_mode_tv_to_dvd", 0x8000, a1, cmd, arg, arglen, buf);
}

int avioctl2_get_vps_data(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack_comp("avioctl2_get_vps_data", 0x6B6C0, a1, cmd, arg, arglen, buf);
}

int avioctl2_get_pdc_data(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack_comp("avioctl2_get_pdc_data", 0x2DC6C0, a1, cmd, arg, arglen, buf);
}

int avioctl2_get_format1_data(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack_comp("avioctl2_get_format1_data", 0x2DC6C0, a1, cmd, arg, arglen, buf);
}

int avioctl2_get_header_time_data(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack_comp("avioctl2_get_header_time_data", 0x2DC6C0, a1, cmd, arg, arglen, buf);
}

int avioctl2_set_acs_position_euro(
    iomanX_iop_file_t *a1,
    int cmd,
    void *arg,
    unsigned int arglen,
    void *buf,
    unsigned int buflen)
{
    (void)buflen;

    return avioctl2_cmd_ack("avioctl2_set_acs_position_euro", 0x8000, a1, cmd, arg, arglen, buf);
}
