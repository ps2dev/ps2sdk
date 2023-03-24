/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2021-2021, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "iomanX.h"
#include "loadcore.h"
#include "pvrdrv.h"
#include "stdio.h"
#include "sysclib.h"
#include "thbase.h"
#include "thsemap.h"
#include "speedregs.h"
#include "errno.h"

#define MODNAME "DVRFILE"
#ifdef DEBUG
#define DPRINTF(x...) printf(MODNAME ": " x)
#else
#define DPRINTF(x...)
#endif

extern int module_start(int argc, char *argv[]);
extern int module_stop(int argc, char *argv[]);
extern int dvrf_df_init(iomanX_iop_device_t *f);
extern int dvrf_df_exit(iomanX_iop_device_t *f);
extern int dvrf_df_chdir(iomanX_iop_file_t *f, const char *name);
extern int dvrf_df_chstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat, unsigned int statmask);
extern int dvrf_df_close(iomanX_iop_file_t *f);
extern int dvrf_df_dclose(iomanX_iop_file_t *f);
extern int dvrf_df_devctl(iomanX_iop_file_t *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrf_df_dopen(iomanX_iop_file_t *f, const char *path);
extern int dvrf_df_dread(iomanX_iop_file_t *f, iox_dirent_t *buf);
extern int dvrf_df_format(iomanX_iop_file_t *f, const char *dev, const char *blockdev, void *arg, int arglen);
extern int dvrf_df_getstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat);
extern int dvrf_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param);
extern int dvrf_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
extern int dvrf_df_lseek(iomanX_iop_file_t *f, int offset, int mode);
extern s64 dvrf_df_lseek64(iomanX_iop_file_t *f, s64 offset, int whence);
extern int dvrf_df_mkdir(iomanX_iop_file_t *f, const char *path, int mode);
extern int dvrf_df_mount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen);
extern int dvrf_df_open(iomanX_iop_file_t *f, const char *name, int flags, int mode);
extern int dvrf_df_read(iomanX_iop_file_t *f, void *ptr, int size);
extern int dvrf_df_readlink(iomanX_iop_file_t *f, const char *path, char *buf, unsigned int buflen);
extern int dvrf_df_remove(iomanX_iop_file_t *f, const char *name);
extern int dvrf_df_rename(iomanX_iop_file_t *f, const char *old, const char *new_1);
extern int dvrf_df_rmdir(iomanX_iop_file_t *f, const char *path);
extern int dvrf_df_symlink(iomanX_iop_file_t *f, const char *old, const char *new_1);
extern int dvrf_df_sync(iomanX_iop_file_t *f, const char *dev, int flag);
extern int dvrf_df_umount(iomanX_iop_file_t *f, const char *fsname);
extern int dvrf_df_write(iomanX_iop_file_t *f, void *ptr, int size);
extern void CopySceStat(iox_stat_t *stat, u8 *dvrp_stat);

static inline u32 bswap32(u32 val)
{
#if 0
    return __builtin_bswap32(val);
#else
    return (val << 24) + ((val & 0xFF00) << 8) + ((val >> 8) & 0xFF00) + ((val >> 24) & 0xFF);
#endif
}

static int dvrf_translator_df_chdir(iomanX_iop_file_t *f, const char *name)
{
    char translated_name[1040];

    sprintf(translated_name, "%s%d:%s", f->device->name, f->unit, name);
    return dvrf_df_chdir(f, translated_name);
}

static int dvrf_translator_df_chstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat, unsigned int statmask)
{
    char translated_name[1040];

    sprintf(translated_name, "%s%d:%s", f->device->name, f->unit, name);
    return dvrf_df_chstat(f, translated_name, stat, statmask);
}

static int dvrf_translator_df_devctl(iomanX_iop_file_t *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    char translated_name[1040];

    sprintf(translated_name, "%s%d:%s", f->device->name, f->unit, name);
    return dvrf_df_devctl(f, translated_name, cmd, arg, arglen, buf, buflen);
}

static int dvrf_translator_df_dopen(iomanX_iop_file_t *f, const char *path)
{
    char translated_path[1040];

    sprintf(translated_path, "%s%d:%s", f->device->name, f->unit, path);
    return dvrf_df_dopen(f, translated_path);
}

static int dvrf_translator_df_format(iomanX_iop_file_t *f, const char *dev, const char *blockdev, void *arg, int arglen)
{
    char translated_dev[1040];

    if (strcmp(f->device->name, "dvr_hdd") != 0) {
        sprintf(translated_dev, "%s:%s", f->device->name, dev);
        *(u32 *)arg = bswap32(*(u32 *)arg);
    } else {
        sprintf(translated_dev, "%s%d:%s", f->device->name, f->unit, dev);
    }
    return dvrf_df_format(f, translated_dev, blockdev, arg, arglen);
}

static int dvrf_translator_df_getstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat)
{
    char translated_name[1040];

    sprintf(translated_name, "%s%d:%s", f->device->name, f->unit, name);
    return dvrf_df_getstat(f, translated_name, stat);
}

static int dvrf_translator_df_mkdir(iomanX_iop_file_t *f, const char *path, int mode)
{
    char translated_path[1040];

    sprintf(translated_path, "%s%d:%s", f->device->name, f->unit, path);
    return dvrf_df_mkdir(f, translated_path, mode);
}

static int dvrf_translator_df_mount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
    char translated_fsname[1040];

    sprintf(translated_fsname, "%s%d:%s", f->device->name, f->unit, fsname);
    return dvrf_df_mount(f, translated_fsname, devname, flag, arg, arglen);
}

static int dvrf_translator_df_open(iomanX_iop_file_t *f, const char *name, int flags, int mode)
{
    char translated_name[1040];

    sprintf(translated_name, "%s%d:%s", f->device->name, f->unit, name);
    return dvrf_df_open(f, translated_name, flags, mode);
}

static int dvrf_translator_df_readlink(iomanX_iop_file_t *f, const char *path, char *buf, unsigned int buflen)
{
    char translated_path[1040];

    sprintf(translated_path, "%s%d:%s", f->device->name, f->unit, path);
    return dvrf_df_readlink(f, translated_path, buf, buflen);
}

static int dvrf_translator_df_remove(iomanX_iop_file_t *f, const char *name)
{
    char translated_name[1040];

    sprintf(translated_name, "%s%d:%s", f->device->name, f->unit, name);
    return dvrf_df_remove(f, translated_name);
}

static int dvrf_translator_df_rename(iomanX_iop_file_t *f, const char *old, const char *new_1)
{
    char translated_old[1040];
    char translated_new[1040];

    sprintf(translated_old, "%s%d:%s", f->device->name, f->unit, old);
    sprintf(translated_new, "%s%d:%s", f->device->name, f->unit, new_1);
    return dvrf_df_rename(f, translated_old, translated_new);
}

static int dvrf_translator_df_rmdir(iomanX_iop_file_t *f, const char *path)
{
    char translated_path[1040];

    sprintf(translated_path, "%s%d:%s", f->device->name, f->unit, path);
    return dvrf_df_rmdir(f, translated_path);
}

static int dvrf_translator_df_symlink(iomanX_iop_file_t *f, const char *old, const char *new_1)
{
    char translated_old[1040];
    char translated_new[1040];

    sprintf(translated_old, "%s%d:%s", f->device->name, f->unit, old);
    sprintf(translated_new, "%s%d:%s", f->device->name, f->unit, new_1);
    return dvrf_df_symlink(f, translated_old, translated_new);
}

static int dvrf_translator_df_sync(iomanX_iop_file_t *f, const char *dev, int flag)
{
    char translated_dev[1040];

    sprintf(translated_dev, "%s%d:%s", f->device->name, f->unit, dev);
    return dvrf_df_sync(f, translated_dev, flag);
}

static int dvrf_translator_df_umount(iomanX_iop_file_t *f, const char *fsname)
{
    char translated_fsname[1040];

    sprintf(translated_fsname, "%s%d:%s", f->device->name, f->unit, fsname);
    return dvrf_df_umount(f, translated_fsname);
}

static iomanX_iop_device_ops_t dvrf_translator_functbl = {
    dvrf_df_init,
    dvrf_df_exit,
    dvrf_translator_df_format,
    dvrf_translator_df_open,
    dvrf_df_close,
    dvrf_df_read,
    dvrf_df_write,
    dvrf_df_lseek,
    dvrf_df_ioctl,
    dvrf_translator_df_remove,
    dvrf_translator_df_mkdir,
    dvrf_translator_df_rmdir,
    dvrf_translator_df_dopen,
    dvrf_df_dclose,
    dvrf_df_dread,
    dvrf_translator_df_getstat,
    dvrf_translator_df_chstat,
    dvrf_translator_df_rename,
    dvrf_translator_df_chdir,
    dvrf_translator_df_sync,
    dvrf_translator_df_mount,
    dvrf_translator_df_umount,
    dvrf_df_lseek64,
    dvrf_translator_df_devctl,
    dvrf_translator_df_symlink,
    dvrf_translator_df_readlink,
    dvrf_df_ioctl2,
};

#define GEN_TRANSLATION_FUNCS(basefuncname, basedevname, shouldbswapformatarg, drvname) \
    static iomanX_iop_device_t basefuncname##_drv = {                                          \
        basedevname,                                                                    \
        (IOP_DT_FS | IOP_DT_FSEXT),                                                     \
        1,                                                                              \
        drvname,                                                                        \
        &dvrf_translator_functbl,                                                       \
    };

GEN_TRANSLATION_FUNCS(dvrpfs, "dvr_pfs", 1, "PFS Driver for DVR");
GEN_TRANSLATION_FUNCS(dvrhdd, "dvr_hdd", 0, "HDD Driver for DVR");
GEN_TRANSLATION_FUNCS(dvrhdck, "dvr_hdck", 1, "HDCK Driver for DVR");
GEN_TRANSLATION_FUNCS(dvrfssk, "dvr_fssk", 1, "FSSK Driver for DVR");
GEN_TRANSLATION_FUNCS(dvrfsck, "dvr_fsck", 1, "FSCK Driver for DVR");

s32 sema_id;
int current_chunk_size;
int RBUF[32768];
int SBUF[32768];

// Based off of DESR / PSX DVR system software version 1.31.
IRX_ID(MODNAME, 1, 1);

int _start(int argc, char *argv[])
{
    if (argc >= 0)
        return module_start(argc, argv);
    else
        return module_stop(argc, argv);
}

int module_start(int argc, char *argv[])
{
    int i;
    USE_SPD_REGS;

    for (i = 0; i < 30000; i += 1) {
        if ((SPD_REG16(0x4230) & 0x20) != 0) {
            break;
        }
        DelayThread(1000);
    }

    if (i == 30000) {
        DPRINTF("IOMAN task of DVRP is not running...\n");
        return MODULE_NO_RESIDENT_END;
    }
    sema_id = -1;
    current_chunk_size = 0x4000;
    if (iomanX_AddDrv(&dvrpfs_drv) || iomanX_AddDrv(&dvrhdd_drv)) {
        goto fail;
    }
    for (i = 0; i < argc; i += 1) {
        if (!strcmp(argv[i], "fschk"))
            goto setup_fschk;
    }
#if 0
    return MODULE_REMOVABLE_END;
#else
    return MODULE_RESIDENT_END;
#endif
setup_fschk:
    DPRINTF("dvrfile.irx : FILE SYSTEM CHECK MODE\n");

    if (iomanX_AddDrv(&dvrhdck_drv)) {
        DPRINTF("hdck\n");
        goto fail;
    }
    if (iomanX_AddDrv(&dvrfssk_drv)) {
        DPRINTF("fssk\n");
        goto fail;
    }
    if (iomanX_AddDrv(&dvrfsck_drv)) {
        DPRINTF("fsck\n");
        goto fail;
    }
#if 0
    return MODULE_REMOVABLE_END;
#else
    return MODULE_RESIDENT_END;
#endif
fail:
    iomanX_DelDrv(dvrpfs_drv.name);
    iomanX_DelDrv(dvrhdd_drv.name);
    iomanX_DelDrv(dvrhdck_drv.name);
    iomanX_DelDrv(dvrfssk_drv.name);
    iomanX_DelDrv(dvrfsck_drv.name);
    return MODULE_NO_RESIDENT_END;
}

int module_stop(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (iomanX_DelDrv(dvrpfs_drv.name) || iomanX_DelDrv(dvrhdd_drv.name)) {
#if 0
        return MODULE_REMOVABLE_END;
#else
        return MODULE_RESIDENT_END;
#endif
    }
    return MODULE_NO_RESIDENT_END;
}

static int check_cmdack_err(int (*func)(drvdrv_exec_cmd_ack *cmdack), drvdrv_exec_cmd_ack *cmdack, int *retval, const char *funcname)
{
    if (func(cmdack)) {
        *retval = -EIO;
        DPRINTF("%s -> IO error (phase %d)\n", funcname, cmdack->phase);
        return 1;
    }
    if (cmdack->comp_status) {
        *retval = -EIO;
        DPRINTF("%s -> Complete parameter error (phase %d), %04X\n", funcname, cmdack->phase, cmdack->comp_status);
        return 1;
    }
    *retval = (cmdack->return_result_word[0] << 16) + cmdack->return_result_word[1];
    return 0;
}

int dvrf_df_init(iomanX_iop_device_t *f)
{
    int this_sema_id;
    iop_sema_t sema_struct;

    (void)f;

    if (sema_id >= 0) {
        return 0;
    }
    sema_struct.attr = 0;
    sema_struct.initial = 1;
    sema_struct.max = 1;
    sema_struct.option = 0;
    this_sema_id = CreateSema(&sema_struct);
    if (this_sema_id < 0) {
        return -1;
    }
    sema_id = this_sema_id;
    return 0;
}

int dvrf_df_exit(iomanX_iop_device_t *f)
{
    (void)f;

    if (sema_id < 0) {
        return 0;
    }
    if (DeleteSema(sema_id)) {
        return -1;
    }
    sema_id = -1;
    return 0;
}

int dvrf_df_chdir(iomanX_iop_file_t *f, const char *name)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    strcpy((char *)SBUF, name);
    cmdack.command = 0x1101;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(name) + 1;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_chstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat, unsigned int statmask)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    SBUF[0] = statmask;
    strcpy((char *)&SBUF[1], name);
    cmdack.command = 0x1102;
    cmdack.input_buffer = SBUF;
    cmdack.input_word_count = 2;
    cmdack.input_buffer_length = strlen(name) + 5;
    cmdack.output_buffer = RBUF;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDma2Comp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    CopySceStat(stat, (u8 *)RBUF);
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_close(iomanX_iop_file_t *f)
{
    int retval;
    int dvrp_fd;
    drvdrv_exec_cmd_ack cmdack;

    retval = 0;
    WaitSema(sema_id);
    cmdack.command = 0x1103;
    dvrp_fd = (int)f->privdata;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word_count = 2;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    retval = 0;
    f->privdata = NULL;
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_dclose(iomanX_iop_file_t *f)
{
    int retval;
    int dvrp_fd;
    drvdrv_exec_cmd_ack cmdack;

    retval = 0;
    WaitSema(sema_id);
    cmdack.command = 0x1104;
    dvrp_fd = (int)f->privdata;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word_count = 2;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    retval = 0;
    f->privdata = NULL;
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_devctl(iomanX_iop_file_t *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int retval;
    u32 argoffset;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    retval = 0;
    WaitSema(sema_id);
    if (cmd == 0x5065) {
        if ((*(u32 *)arg & 0x7F) != 0) {
            retval = -EINVAL;
        } else if (*(int *)arg <= 0x20000) {
            current_chunk_size = *(u32 *)arg;
        } else {
            retval = -EDOM;
        }
        goto finish;
    }
    argoffset = arglen + 16;
    SBUF[0] = bswap32(argoffset);
    SBUF[1] = bswap32((u32)cmd);
    SBUF[2] = bswap32(buflen);
    SBUF[3] = bswap32(arglen);
    memcpy(&SBUF[4], arg, arglen);
    strcpy((char *)SBUF + argoffset, name);
    cmdack.input_buffer_length = argoffset + strlen(name) + 1;
    cmdack.command = 0x1105;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.output_buffer = RBUF;
    cmdack.timeout = 30000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDma2Comp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    if (!retval && buf) {
        memcpy(buf, cmdack.output_buffer, buflen);
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_dopen(iomanX_iop_file_t *f, const char *path)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    WaitSema(sema_id);
    strcpy((char *)SBUF, path);
    cmdack.command = 0x1106;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(path) + 1;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    if (retval < 0) {
        DPRINTF("%s -> fd error (fd=%d)\n", __func__, retval);
        goto finish;
    }
    f->privdata = (void *)retval;
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_dread(iomanX_iop_file_t *f, iox_dirent_t *buf)
{
    int dvrp_fd;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    WaitSema(sema_id);
    cmdack.command = 0x1107;
    dvrp_fd = (int)f->privdata;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word_count = 2;
    cmdack.output_buffer = RBUF;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaRecvComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    memcpy(buf, RBUF, sizeof(*buf));
    buf->stat.mode = bswap32(buf->stat.mode);
    buf->stat.attr = bswap32(buf->stat.attr);
    buf->stat.size = bswap32(buf->stat.size);
    buf->stat.hisize = bswap32(buf->stat.hisize);
    buf->stat.private_0 = bswap32(buf->stat.private_0);
    buf->stat.private_1 = bswap32(buf->stat.private_1);
    buf->stat.private_2 = bswap32(buf->stat.private_2);
    buf->stat.private_3 = bswap32(buf->stat.private_3);
    buf->stat.private_4 = bswap32(buf->stat.private_4);
    buf->stat.private_5 = bswap32(buf->stat.private_5);
    u8 tmp;
    tmp = buf->stat.ctime[6];
    buf->stat.ctime[6] = buf->stat.ctime[7];
    buf->stat.ctime[7] = tmp;
    tmp = buf->stat.atime[6];
    buf->stat.atime[6] = buf->stat.atime[7];
    buf->stat.atime[7] = tmp;
    tmp = buf->stat.mtime[6];
    buf->stat.mtime[6] = buf->stat.mtime[7];
    buf->stat.mtime[7] = tmp;
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_format(iomanX_iop_file_t *f, const char *dev, const char *blockdev, void *arg, int arglen)
{
    size_t blockdev_len;
    u32 dev_len;
    const char *dev_;
    u32 arg_offset;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    dev_len = strlen(dev) + 13;
    blockdev_len = strlen(blockdev);
    dev_ = dev;
    arg_offset = dev_len + blockdev_len + 1;
    SBUF[1] = bswap32(arg_offset);
    SBUF[0] = bswap32(dev_len);
    SBUF[2] = bswap32(arglen);
    strcpy((char *)&SBUF[3], dev_);
    strcpy((char *)SBUF + dev_len, blockdev);
    memcpy((char *)SBUF + arg_offset, arg, arglen);
    cmdack.command = 0x1108;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = arg_offset + arglen;
    cmdack.timeout = 3600000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_getstat(iomanX_iop_file_t *f, const char *name, iox_stat_t *stat)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    strcpy((char *)SBUF, name);
    cmdack.command = 0x1109;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(name) + 1;
    cmdack.output_buffer = RBUF;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDma2Comp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    CopySceStat(stat, (u8 *)RBUF);
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_ioctl(iomanX_iop_file_t *f, int cmd, void *param)
{
    int dvrp_fd;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    WaitSema(sema_id);
    dvrp_fd = (int)f->privdata;
    cmdack.command = 0x110A;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word[2] = (cmd >> 16) & 0xFFFF;
    cmdack.input_word[3] = cmd;
    cmdack.input_word[4] = ((u32)param >> 16) & 0xFFFF;
    cmdack.input_word[5] = (u32)param & 0xFFFF;
    cmdack.input_word_count = 6;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_ioctl2(iomanX_iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
    int dvrp_fd;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    WaitSema(sema_id);
    dvrp_fd = (int)f->privdata;
    SBUF[1] = bswap32(cmd);
    SBUF[2] = bswap32(buflen);
    SBUF[0] = bswap32(dvrp_fd);
    SBUF[3] = bswap32(arglen);
    memcpy(((u8 *)SBUF) + 0x10, arg, arglen);
    cmdack.command = 0x110B;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = arglen + 16;
    cmdack.input_word_count = 0;
    cmdack.output_buffer = RBUF;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDma2Comp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    memcpy(buf, RBUF, buflen);
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_lseek(iomanX_iop_file_t *f, int offset, int mode)
{
    int dvrp_fd;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    WaitSema(sema_id);
    dvrp_fd = (int)f->privdata;
    cmdack.command = 0x110C;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word[2] = (offset >> 16) & 0xFFFF;
    cmdack.input_word[3] = offset;
    cmdack.input_word[4] = (mode >> 16) & 0xFFFF;
    cmdack.input_word[5] = mode;
    cmdack.input_word_count = 6;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

s64 dvrf_df_lseek64(iomanX_iop_file_t *f, s64 offset, int whence)
{
    int dvrp_fd;
    s64 retval;
    int rretval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    dvrp_fd = (int)f->privdata;
    cmdack.command = 0x110D;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word[2] = (offset >> 48) & 0xFFFF;
    cmdack.input_word[3] = (offset >> 32) & 0xFFFF;
    cmdack.input_word[4] = (offset >> 16) & 0xFFFF;
    cmdack.input_word[5] = offset;
    cmdack.input_word[6] = (whence >> 16) & 0xFFFF;
    cmdack.input_word[7] = whence;
    cmdack.input_word_count = 8;
    cmdack.timeout = 10000000;
    rretval = 0;
    if (check_cmdack_err(&DvrdrvExecCmdAckComp, &cmdack, &rretval, __func__)) {
        retval = rretval;
        goto finish;
    }
    retval = ((s64)cmdack.return_result_word[0] << 48) | ((s64)cmdack.return_result_word[1] << 32) | ((s64)cmdack.return_result_word[2] << 16) | (s64)cmdack.return_result_word[3];
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_mkdir(iomanX_iop_file_t *f, const char *path, int mode)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    SBUF[0] = bswap32(mode);
    strcpy((char *)&SBUF[1], path);
    cmdack.command = 0x110E;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(path) + 5;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_mount(iomanX_iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
    size_t devname_len;
    u32 fsname_len;
    const char *fsname_;
    unsigned int arg_offs;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    SBUF[0] = bswap32(flag);
    fsname_len = strlen(fsname) + 17;
    devname_len = strlen(devname);
    fsname_ = fsname;
    arg_offs = fsname_len + devname_len + 1;
    SBUF[2] = bswap32(arg_offs);
    SBUF[1] = bswap32(fsname_len);
    SBUF[3] = bswap32(arglen);
    strcpy((char *)&SBUF[4], fsname_);
    strcpy((char *)SBUF + fsname_len, devname);
    memcpy(((u8 *)SBUF) + arg_offs, arg, arglen);

    cmdack.command = 0x110F;
    cmdack.input_buffer = SBUF;
    cmdack.input_word_count = 0;
    cmdack.input_buffer_length = arg_offs + arglen;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_open(iomanX_iop_file_t *f, const char *name, int flags, int mode)
{
    u16 mode_;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    mode_ = mode;
    WaitSema(sema_id);
    SBUF[0] = bswap32(flags);
    mode_ = (mode_ << 8) + (mode_ >> 8);
    memcpy(&SBUF[1], &mode_, sizeof(mode_));
    strcpy((char *)&SBUF[1] + 2, name);
    cmdack.command = 0x1110;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(name) + 7;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    if (retval < 0) {
        DPRINTF("%s -> fd error (fd=%d)\n", __func__, retval);
        goto finish;
    }
    f->privdata = (void *)retval;
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_read(iomanX_iop_file_t *f, void *ptr, int size)
{
    size_t total_read;
    char *out_buf;
    int retval;
    int dvrp_fd;
    int remain_size;
    int chunk_size;
    int unaligned_size;
    drvdrv_exec_cmd_ack cmdack;

    total_read = 0;
    unaligned_size = 0;
    if (((u32)ptr & 3) != 0) {
        unaligned_size = 4 - ((u32)ptr & 3);
    }
    WaitSema(sema_id);
    out_buf = (char *)ptr;
    dvrp_fd = (int)f->privdata;
    remain_size = size;
    cmdack.command = 0x1111;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word_count = 4;
    cmdack.timeout = 10000000;
    while (1) {
        int read_size;
        if (remain_size <= 0) {
            break;
        }
        chunk_size = current_chunk_size;
        if (remain_size < current_chunk_size) {
            chunk_size = remain_size;
        }
        if (unaligned_size != 0) {
            chunk_size = unaligned_size;
        }
        cmdack.input_word[2] = (chunk_size >> 16) & 0xFFFF;
        cmdack.input_word[3] = chunk_size;
        if ((chunk_size & 0x7F) != 0 || unaligned_size != 0) {
            cmdack.output_buffer = (char *)RBUF;
        } else {
            cmdack.output_buffer = out_buf;
        }
        if (check_cmdack_err(&DvrdrvExecCmdAckDmaRecvComp, &cmdack, &read_size, __func__)) {
            retval = read_size;
            goto finish;
        }
        if ((chunk_size & 0x7F) != 0 || unaligned_size != 0) {
            memcpy(out_buf, RBUF, chunk_size);
            unaligned_size = 0;
        }
        if (read_size <= 0) {
            break;
        }
        remain_size -= read_size;
        out_buf += read_size;
        total_read += read_size;
    }
    retval = total_read;
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_readlink(iomanX_iop_file_t *f, const char *path, char *buf, unsigned int buflen)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    SBUF[0] = bswap32(buflen);
    strcpy((char *)&SBUF[1], path);
    cmdack.input_buffer_length = strlen(path) + 5;
    cmdack.command = 0x1112;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.output_buffer = RBUF;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDma2Comp, &cmdack, &retval, __func__)) {
        goto finish;
    }
    memcpy(buf, RBUF, buflen);
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_remove(iomanX_iop_file_t *f, const char *name)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    strcpy((char *)SBUF, name);
    cmdack.command = 0x1113;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(name) + 1;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_rename(iomanX_iop_file_t *f, const char *old, const char *new_1)
{
    size_t old_strlen;
    const char *old_;
    size_t new_offs;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    old_strlen = strlen(old);
    old_ = old;
    new_offs = old_strlen + 5;
    SBUF[0] = bswap32(new_offs);
    strcpy((char *)&SBUF[1], old_);
    strcpy((char *)SBUF + new_offs, new_1);
    cmdack.command = 0x1114;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = new_offs + strlen(new_1) + 1;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_rmdir(iomanX_iop_file_t *f, const char *path)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    strcpy((char *)SBUF, path);
    cmdack.command = 0x1115;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(path) + 1;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_symlink(iomanX_iop_file_t *f, const char *old, const char *new_1)
{
    size_t old_len;
    const char *old_;
    size_t new_offs;
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    old_len = strlen(old);
    old_ = old;
    new_offs = old_len + 5;
    SBUF[0] = bswap32(new_offs);
    strcpy((char *)&SBUF[1], old_);
    strcpy((char *)SBUF + new_offs, new_1);
    cmdack.command = 0x1116;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = new_offs + strlen(new_1) + 1;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_sync(iomanX_iop_file_t *f, const char *dev, int flag)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    SBUF[0] = bswap32(flag);
    strcpy((char *)&SBUF[1], dev);
    cmdack.command = 0x1117;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(dev) + 5;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_umount(iomanX_iop_file_t *f, const char *fsname)
{
    int retval;
    drvdrv_exec_cmd_ack cmdack;

    (void)f;

    WaitSema(sema_id);
    strcpy((char *)SBUF, fsname);
    cmdack.command = 0x1118;
    cmdack.input_word_count = 0;
    cmdack.input_buffer = SBUF;
    cmdack.input_buffer_length = strlen(fsname) + 1;
    cmdack.timeout = 10000000;
    if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
        goto finish;
    }
finish:
    SignalSema(sema_id);
    return retval;
}

int dvrf_df_write(iomanX_iop_file_t *f, void *ptr, int size)
{
    int total_write;
    char *in_buffer;
    int retval;
    int dvrp_fd;
    int remain_size;
    int unaligned_size;
    drvdrv_exec_cmd_ack cmdack;

    total_write = 0;
    in_buffer = (char *)ptr;
    unaligned_size = 0;
    if (((u32)ptr & 3) != 0) {
        unaligned_size = 4 - ((u32)ptr & 3);
        memcpy(RBUF, ptr, unaligned_size);
    }
    WaitSema(sema_id);
    dvrp_fd = (int)f->privdata;
    remain_size = size;
    cmdack.command = 0x1119;
    cmdack.input_word[0] = (dvrp_fd >> 16) & 0xFFFF;
    cmdack.input_word[1] = dvrp_fd;
    cmdack.input_word_count = 4;
    cmdack.timeout = 10000000;
    while (remain_size > 0) {
        u32 chunk_size;
        chunk_size = current_chunk_size;
        if (remain_size < current_chunk_size) {
            chunk_size = remain_size;
        }
        if (unaligned_size != 0) {
            chunk_size = unaligned_size;
        }
        cmdack.input_word[2] = (chunk_size >> 16) & 0xFFFF;
        cmdack.input_word[3] = chunk_size;
        if (unaligned_size != 0) {
            cmdack.input_buffer = (char *)RBUF;
            unaligned_size = 0;
        } else {
            cmdack.input_buffer = in_buffer;
        }
        cmdack.input_buffer_length = chunk_size;
        if (check_cmdack_err(&DvrdrvExecCmdAckDmaSendComp, &cmdack, &retval, __func__)) {
            goto finish;
        }
        if (retval <= 0) {
            goto finish;
        }
        remain_size -= retval;
        in_buffer += retval;
        total_write += retval;
    }
    retval = total_write;
finish:
    SignalSema(sema_id);
    return retval;
}

void CopySceStat(iox_stat_t *stat, u8 *dvrp_stat)
{
    stat->mode = bswap32(((u32 *)dvrp_stat)[0]);
    stat->attr = bswap32(((u32 *)dvrp_stat)[1]);
    stat->size = bswap32(((u32 *)dvrp_stat)[2]);
    memcpy(stat->ctime, &((u32 *)dvrp_stat)[3], 6);
    stat->ctime[6] = ((u8 *)dvrp_stat)[19];
    stat->ctime[7] = ((u8 *)dvrp_stat)[18];
    memcpy(stat->atime, &((u32 *)dvrp_stat)[5], 6);
    stat->atime[6] = ((u8 *)dvrp_stat)[27];
    stat->atime[7] = ((u8 *)dvrp_stat)[26];
    memcpy(stat->mtime, &((u32 *)dvrp_stat)[7], 6);
    stat->mtime[6] = ((u8 *)dvrp_stat)[35];
    stat->mtime[7] = ((u8 *)dvrp_stat)[34];
    stat->hisize = bswap32(((u32 *)dvrp_stat)[9]);
    stat->private_0 = bswap32(((u32 *)dvrp_stat)[10]);
    stat->private_1 = bswap32(((u32 *)dvrp_stat)[11]);
    stat->private_2 = bswap32(((u32 *)dvrp_stat)[12]);
    stat->private_3 = bswap32(((u32 *)dvrp_stat)[13]);
    stat->private_4 = bswap32(((u32 *)dvrp_stat)[14]);
    stat->private_5 = bswap32(((u32 *)dvrp_stat)[15]);
}
