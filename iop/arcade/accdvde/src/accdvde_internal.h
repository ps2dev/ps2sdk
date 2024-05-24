/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACCDVDE_INTERNAL_H
#define _ACCDVDE_INTERNAL_H

#include <irx_imports.h>

struct cde_softc
{
	acUint8 cal[16];
	acUint8 rpl[48];
	acInt32 thid;
	acCdvdsifId fno;
	acUint32 padding;
	acInt32 st_index;
	acMemData st_mem[2];
	acMemData rd_mem;
};

typedef int (*cde_ops_t)(void *arg);

struct ac_cdvdsif_reply
{
	acInt32 error;
	acInt32 result;
	acUint32 padding[2];
};

struct ac_cdvdsif_ready
{
	acInt32 mode;
	acUint32 padding[3];
};

struct ac_cdvdsif_init
{
	acInt32 mode;
	acUint32 padding[3];
};

struct ac_cdvdsif_read
{
	acUint32 lsn;
	acUint32 sectors;
	void *buf;
	sceCdRMode rmode;
};

struct ac_cdvdsif_readtoc
{
	acUint8 *toc;
	acUint32 size;
	acUint32 padding[2];
};

struct ac_cdvdsif_sync
{
	acInt32 mode;
	acUint32 padding[3];
};

struct ac_cdvdsif_sync_rpl
{
	acInt32 error;
	acInt32 result;
	acCdvdsifId fno;
	acUint32 rpos;
};

struct ac_cdvdsif_lookup
{
	acInt8 *name;
	acInt32 namlen;
	acUint32 padding[2];
};

struct ac_cdvdsif_lookup_rpl
{
	acInt32 error;
	acInt32 result;
	acUint32 padding[2];
	sceCdlFILE file;
};

struct ac_cdvdsif_seek
{
	acUint32 lsn;
	acUint32 padding[3];
};

struct ac_cdvdsif_tray
{
	acInt32 mode;
	acUint32 padding;
	acUint32 *traycnt;
	acUint32 padding2;
};

struct ac_cdvdsif_tray_rpl
{
	acInt32 error;
	acInt32 result;
	acUint32 *traycnt;
	acUint32 status;
};

struct ac_cdvdsif_inits
{
	acUint32 size;
	acUint32 bsize;
	acUint8 *buf;
	acUint32 padding;
};

struct ac_cdvdsif_reads
{
	acUint32 mode;
	acUint32 sectors;
	acUint32 *buf;
	acUint32 *err;
};

struct ac_cdvdsif_reads_rpl
{
	acInt32 error;
	acUint32 result;
	acUint32 *buf;
	acUint32 *err;
};

struct ac_cdvdsif_seeks
{
	acUint32 lsn;
	acUint32 padding[3];
};

struct ac_cdvdsif_starts
{
	acUint32 lsn;
	acUint32 padding[2];
	sceCdRMode rmode;
};

struct ac_cdvdsif_readrtc_rpl
{
	acInt32 error;
	acUint32 result;
	sceCdCLOCK rtc;
};

extern int acCdvdeModuleStart(int argc, char **argv);

#endif
