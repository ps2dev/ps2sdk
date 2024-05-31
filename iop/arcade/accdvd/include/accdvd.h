/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACCDVD_H
#define _ACCDVD_H

#include <accore.h>
#include <cdvdman.h>

typedef enum ac_cdvdsif_id
{
	AC_CDVDSIF_ID_NOP = 0x0,
	AC_CDVDSIF_ID_BREAK = 0x1,
	AC_CDVDSIF_ID_READY = 0x2,
	AC_CDVDSIF_ID_TYPE = 0x3,
	AC_CDVDSIF_ID_ERROR = 0x4,
	AC_CDVDSIF_ID_GETPOS = 0x5,
	AC_CDVDSIF_ID_READTOC = 0x6,
	AC_CDVDSIF_ID_INIT = 0x7,
	AC_CDVDSIF_ID_PAUSE = 0x8,
	AC_CDVDSIF_ID_READ = 0x9,
	AC_CDVDSIF_ID_READI = 0xA,
	AC_CDVDSIF_ID_SYNC = 0xB,
	AC_CDVDSIF_ID_LOOKUP = 0xC,
	AC_CDVDSIF_ID_SEEK = 0xD,
	AC_CDVDSIF_ID_STANDBY = 0xE,
	AC_CDVDSIF_ID_STAT = 0xF,
	AC_CDVDSIF_ID_STOP = 0x10,
	AC_CDVDSIF_ID_TRAY = 0x11,
	AC_CDVDSIF_ID_INITS = 0x12,
	AC_CDVDSIF_ID_READS = 0x13,
	AC_CDVDSIF_ID_SEEKS = 0x14,
	AC_CDVDSIF_ID_STARTS = 0x15,
	AC_CDVDSIF_ID_STATS = 0x16,
	AC_CDVDSIF_ID_STOPS = 0x17,
	AC_CDVDSIF_ID_PAUSES = 0x18,
	AC_CDVDSIF_ID_RESUMES = 0x19,
	AC_CDVDSIF_ID_READRTC = 0x1A,
	AC_CDVDSIF_ID_STREAM = 0x1B,
	AC_CDVDSIF_ID_LAST = 0x1B,
} acCdvdsifId;

enum cdc_xfer_dir
{
	CDC_XFER_SYNC = 0x0,
	CDC_XFER_IN = 0x1,
	CDC_XFER_OUT = 0x2,
};

typedef int (*cdc_xfer_t)(void *dst, void *src, int len, enum cdc_xfer_dir dir);

typedef void (*cdc_done_t)(int eveid);

extern int acCdvdModuleRestart(int argc, char **argv);
extern int acCdvdModuleStart(int argc, char **argv);
extern int acCdvdModuleStatus();
extern int acCdvdModuleStop();
extern int cdc_module_restart(int argc, char **argv);
extern int cdc_module_start(int argc, char **argv);
extern int cdc_module_status();
extern int cdc_module_stop();
extern int cdc_geterrno();
extern int cdc_getfno();
extern int cdc_seterrno(int ret);
extern int cdc_xfer(void *dst, void *src, int len, enum cdc_xfer_dir dir);
extern int cdc_error();
extern int cdc_lookup(sceCdlFILE *fp, const char *name, int namlen, cdc_xfer_t xfer);
extern int cdc_medium();
extern int cdc_readtoc(unsigned char *toc, cdc_xfer_t xfer);
extern int cdc_ready(int nonblocking);
extern int cdc_sync(int nonblocking);
extern int cdc_pause(cdc_done_t done);
extern int cdc_standby(cdc_done_t done);
extern int cdc_stat();
extern int cdc_stop(cdc_done_t done);
extern int cdc_tray(int mode, u32 *traycnt);
extern int cdc_seek(unsigned int lsn, cdc_done_t done);
extern int cdc_read(unsigned int lsn, void *buf, int sectors, const sceCdRMode *mode, cdc_xfer_t xfer, cdc_done_t done);
extern int cdc_getpos();
extern int cdc_inits(void *buf, unsigned int size, unsigned int bsize);
extern int cdc_seeks(unsigned int lsn);
extern int cdc_reads(void *buf, int sectors, int mode, int *errp, cdc_xfer_t xfer);
extern int cdc_starts(unsigned int lsn, const sceCdRMode *mode);
extern int cdc_stats();
extern int cdc_stops();
extern int cdc_pauses();
extern int cdc_resumes();

#ifdef I_sceCdSearchFile
#undef I_sceCdSearchFile
#endif
#ifdef I_sceCdRead
#undef I_sceCdRead
#endif
#ifdef I_sceCdCallback
#undef I_sceCdCallback
#endif
#ifdef I_sceCdTrayReq
#undef I_sceCdTrayReq
#endif
#ifdef I_sceCdGetReadPos
#undef I_sceCdGetReadPos
#endif
#ifdef I_sceCdSync
#undef I_sceCdSync
#endif
#ifdef I_sceCdInit
#undef I_sceCdInit
#endif
#ifdef I_sceCdDiskReady
#undef I_sceCdDiskReady
#endif
#ifdef I_sceCdGetError
#undef I_sceCdGetError
#endif
#ifdef I_sceCdGetDiskType
#undef I_sceCdGetDiskType
#endif
#ifdef I_sceCdStatus
#undef I_sceCdStatus
#endif
#ifdef I_sceCdGetToc
#undef I_sceCdGetToc
#endif
#ifdef I_sceCdSeek
#undef I_sceCdSeek
#endif
#ifdef I_sceCdStandby
#undef I_sceCdStandby
#endif
#ifdef I_sceCdStop
#undef I_sceCdStop
#endif
#ifdef I_sceCdPause
#undef I_sceCdPause
#endif
#ifdef I_sceCdStInit
#undef I_sceCdStInit
#endif
#ifdef I_sceCdStStart
#undef I_sceCdStStart
#endif
#ifdef I_sceCdStSeek
#undef I_sceCdStSeek
#endif
#ifdef I_sceCdStStop
#undef I_sceCdStStop
#endif
#ifdef I_sceCdStRead
#undef I_sceCdStRead
#endif
#ifdef I_sceCdStStat
#undef I_sceCdStStat
#endif
#ifdef I_sceCdStPause
#undef I_sceCdStPause
#endif
#ifdef I_sceCdStResume
#undef I_sceCdStResume
#endif
#ifdef I_sceCdMmode
#undef I_sceCdMmode
#endif

#define accdvd_IMPORTS_start DECLARE_IMPORT_TABLE(accdvd, 1, 1)
#define accdvd_IMPORTS_end END_IMPORT_TABLE

#define I_acCdvdModuleRestart DECLARE_IMPORT(4, acCdvdModuleRestart)
#define I_acCdvdModuleStart DECLARE_IMPORT(5, acCdvdModuleStart)
#define I_acCdvdModuleStatus DECLARE_IMPORT(6, acCdvdModuleStatus)
#define I_acCdvdModuleStop DECLARE_IMPORT(7, acCdvdModuleStop)
#define I_sceCdSearchFile DECLARE_IMPORT(10, sceCdSearchFile)
#define I_sceCdRead DECLARE_IMPORT(11, sceCdRead)
#define I_sceCdCallback DECLARE_IMPORT(13, sceCdCallback)
#define I_sceCdTrayReq DECLARE_IMPORT(14, sceCdTrayReq)
#define I_sceCdGetReadPos DECLARE_IMPORT(15, sceCdGetReadPos)
#define I_sceCdSync DECLARE_IMPORT(16, sceCdSync)
#define I_sceCdInit DECLARE_IMPORT(17, sceCdInit)
#define I_sceCdDiskReady DECLARE_IMPORT(19, sceCdDiskReady)
#define I_sceCdGetError DECLARE_IMPORT(20, sceCdGetError)
#define I_sceCdGetDiskType DECLARE_IMPORT(21, sceCdGetDiskType)
#define I_sceCdStatus DECLARE_IMPORT(22, sceCdStatus)
#define I_sceCdGetToc DECLARE_IMPORT(24, sceCdGetToc)
#define I_sceCdSeek DECLARE_IMPORT(25, sceCdSeek)
#define I_sceCdStandby DECLARE_IMPORT(26, sceCdStandby)
#define I_sceCdStop DECLARE_IMPORT(27, sceCdStop)
#define I_sceCdPause DECLARE_IMPORT(28, sceCdPause)
#define I_sceCdStInit DECLARE_IMPORT(29, sceCdStInit)
#define I_sceCdStStart DECLARE_IMPORT(30, sceCdStStart)
#define I_sceCdStSeek DECLARE_IMPORT(31, sceCdStSeek)
#define I_sceCdStStop DECLARE_IMPORT(32, sceCdStStop)
#define I_sceCdStRead DECLARE_IMPORT(33, sceCdStRead)
#define I_sceCdStStat DECLARE_IMPORT(34, sceCdStStat)
#define I_sceCdStPause DECLARE_IMPORT(35, sceCdStPause)
#define I_sceCdStResume DECLARE_IMPORT(36, sceCdStResume)
#define I_sceCdMmode DECLARE_IMPORT(37, sceCdMmode)
#define I_cdc_module_restart DECLARE_IMPORT(41, cdc_module_restart)
#define I_cdc_module_start DECLARE_IMPORT(42, cdc_module_start)
#define I_cdc_module_status DECLARE_IMPORT(43, cdc_module_status)
#define I_cdc_module_stop DECLARE_IMPORT(44, cdc_module_stop)
#define I_cdc_geterrno DECLARE_IMPORT(45, cdc_geterrno)
#define I_cdc_getfno DECLARE_IMPORT(46, cdc_getfno)
#define I_cdc_seterrno DECLARE_IMPORT(47, cdc_seterrno)
#define I_cdc_xfer DECLARE_IMPORT(48, cdc_xfer)
#define I_cdc_error DECLARE_IMPORT(49, cdc_error)
#define I_cdc_lookup DECLARE_IMPORT(50, cdc_lookup)
#define I_cdc_medium DECLARE_IMPORT(51, cdc_medium)
#define I_cdc_readtoc DECLARE_IMPORT(52, cdc_readtoc)
#define I_cdc_ready DECLARE_IMPORT(53, cdc_ready)
#define I_cdc_sync DECLARE_IMPORT(54, cdc_sync)
#define I_cdc_pause DECLARE_IMPORT(55, cdc_pause)
#define I_cdc_standby DECLARE_IMPORT(56, cdc_standby)
#define I_cdc_stat DECLARE_IMPORT(57, cdc_stat)
#define I_cdc_stop DECLARE_IMPORT(58, cdc_stop)
#define I_cdc_tray DECLARE_IMPORT(59, cdc_tray)
#define I_cdc_seek DECLARE_IMPORT(60, cdc_seek)
#define I_cdc_read DECLARE_IMPORT(61, cdc_read)
#define I_cdc_getpos DECLARE_IMPORT(62, cdc_getpos)
#define I_cdc_inits DECLARE_IMPORT(63, cdc_inits)
#define I_cdc_seeks DECLARE_IMPORT(64, cdc_seeks)
#define I_cdc_reads DECLARE_IMPORT(65, cdc_reads)
#define I_cdc_starts DECLARE_IMPORT(66, cdc_starts)
#define I_cdc_stats DECLARE_IMPORT(67, cdc_stats)
#define I_cdc_stops DECLARE_IMPORT(68, cdc_stops)
#define I_cdc_pauses DECLARE_IMPORT(69, cdc_pauses)
#define I_cdc_resumes DECLARE_IMPORT(70, cdc_resumes)

#endif
