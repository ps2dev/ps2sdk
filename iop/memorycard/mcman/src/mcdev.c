/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <mcman.h>
#include "mcman-internal.h"

// mc driver vars
static int mcman_mc_port = 0;
static int mcman_mc_slot = 0;
static int mcman_io_sema = 0;

// mc driver ops functions prototypes
int mc_init(MC_IO_DEV_T *iop_dev);
int mc_deinit(MC_IO_DEV_T *dev);
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
int mc_open(MC_IO_FIL_T *f, const char *filename, int flags, int mode);
#else
int mc_open(MC_IO_FIL_T *f, const char *filename, int flags);
#endif
int mc_close(MC_IO_FIL_T *f);
int mc_lseek(MC_IO_FIL_T *f, int pos, int where);
int mc_read(MC_IO_FIL_T *f, void *buf, int size);
int mc_write(MC_IO_FIL_T *f, void *buf, int size);
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
int mc_format(MC_IO_FIL_T *f, const char *dev, const char *blockdev, void *arg, int arglen);
#else
int mc_format(MC_IO_FIL_T *f);
#endif
int mc_remove(MC_IO_FIL_T *f, const char *filename);
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
int mc_mkdir(MC_IO_FIL_T *f, const char *dirname, int mode);
#else
int mc_mkdir(MC_IO_FIL_T *f, const char *dirname);
#endif
int mc_rmdir(MC_IO_FIL_T *f, const char *dirname);
int mc_dopen(MC_IO_FIL_T *f, const char *dirname);
int mc_dclose(MC_IO_FIL_T *f);
int mc_dread(MC_IO_FIL_T *f, MC_IO_DRE_T *dirent);
int mc_getstat(MC_IO_FIL_T *f, const char *filename, MC_IO_STA_T *stat);
int mc_chstat(MC_IO_FIL_T *f, const char *filename, MC_IO_STA_T *stat, unsigned int statmask);
int mc_ioctl(MC_IO_FIL_T *f, int cmd, void* param);

#if MCMAN_ENABLE_EXTENDED_DEV_OPS
int mc_rename(MC_IO_FIL_T *f, const char *old, const char *new);
int mc_chdir(MC_IO_FIL_T *f, const char *name);
int mc_sync(MC_IO_FIL_T *f, const char *dev, int flag);
int mc_mount(MC_IO_FIL_T *f, const char *fsname, const char *devname, int flag, void *arg, int arglen);
int mc_umount(MC_IO_FIL_T *f, const char *fsname);
s64 mc_lseek64(MC_IO_FIL_T *f, s64 pos, int where);
int mc_devctl(MC_IO_FIL_T *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
int mc_symlink(MC_IO_FIL_T *f, const char *old, const char *new);
int mc_readlink(MC_IO_FIL_T *f, const char *path, char *buf, unsigned int buflen);
int mc_ioctl2(MC_IO_FIL_T *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
#endif

// driver ops func tab
static MC_IO_OPS_T mcman_mcops = {
	&mc_init,
	&mc_deinit,
	&mc_format,
	&mc_open,
	&mc_close,
	&mc_read,
	&mc_write,
	&mc_lseek,
	&mc_ioctl,
	&mc_remove,
	&mc_mkdir,
	&mc_rmdir,
	&mc_dopen,
	&mc_dclose,
	&mc_dread,
	&mc_getstat,
	&mc_chstat,
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
	&mc_rename,
	&mc_chdir,
	&mc_sync,
	&mc_mount,
	&mc_umount,
	&mc_lseek64,
	&mc_devctl,
	&mc_symlink,
	&mc_readlink,
	&mc_ioctl2,
#endif
};

// driver descriptor
static MC_IO_DEV_T mcman_mcdev = {
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
	"mc",
#elif defined(BUILDING_VMCMAN)
	"vmc",
#elif defined(BUILDING_XFROMMAN)
	"xfrom",
#endif
	IOP_DT_FS
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
	| IOP_DT_FSEXT
#endif
	,
	1,
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
	"Memory Card",
#elif defined(BUILDING_VMCMAN)
	"Virtual Memory Card",
#elif defined(BUILDING_XFROMMAN)
	"External flash rom",
#endif
	&mcman_mcops,
};

//--------------------------------------------------------------
int mc_init(MC_IO_DEV_T *dev)
{
	(void)dev;

	return 0;
}

//--------------------------------------------------------------
int mcman_ioerrcode(int errcode)
{
	register int r = errcode;

	if (r < 0) {
		switch (r + 8) {
			case 0:
				r = -EIO;
				break;
			case 1:
				r = -EMFILE;
				break;
			case 2:
				r = -EEXIST;
				break;
			case 3:
				r = -EACCES;
				break;
			case 4:
				r = -ENOENT;
				break;
			case 5:
				r = -ENOSPC;
				break;
			case 6:
				r = -EFORMAT;
				break;
			default:
				r = -ENXIO;
				break;
		}
	}
	return r;
}

//--------------------------------------------------------------
int mcman_modloadcb(const char *filename, int *port, int *slot)
{
	register const char *path = filename;
	register int   upos;

	if (*path == 0x20) {
		path++;
		while (*path == 0x20)
			path++;
	}

	if (((u8)path[0] | 0x20) != mcman_mcdev.name[0])
		return 0;
	if (((u8)path[1] | 0x20) != mcman_mcdev.name[1])
		return 0;

	if ((u32)strlen(path) < 2) return 2;

	upos = mcman_chrpos(path, ':');

	if (upos < 0)
		upos = strlen(path);

	if (port) {
		upos --;
		if (((u8)path[upos] - 0x30) < 10)
			*port = (u8)path[upos] - 0x30;
		else *port = 0;
	}

	upos--;
	if (port) {
		*port += 2;
	}

	if (slot) {
		register int m, v;

		*slot = 0;
		for (m = 1; ((u8)path[upos --] - 0x30) < 10; m = v << 1) {
			v = m << 2;
			*slot += m * (path[upos] - 0x30);
			v += m;
		}
	}

	return 1;
}

//--------------------------------------------------------------
void mcman_unit2card(u32 unit)
{
#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
	mcman_mc_port = unit & 1;
	mcman_mc_slot = (unit >> 1) & (MCMAN_MAXSLOT - 1);
#elif defined(BUILDING_VMCMAN)
	mcman_mc_port = 0;
	mcman_mc_slot = unit & (MCMAN_MAXSLOT - 1);
#elif defined(BUILDING_XFROMMAN)
	(void)unit;
	mcman_mc_port = 0;
	mcman_mc_slot = 0;
#endif

	// original mcman/xmcman code below is silly and I doubt it
	// can support more than 2 units anyway...
	/*
	register u32 mask = 0xF0000000;

	while (!(unit & mask)) {
		mask >>= 4;
		if (mask < 0x10)
			break;
	}

 	mcman_mc_port = unit & 0xf;
 	mcman_mc_slot = 0;

 	if (mask < 0xf) {
		while (mask) {
			mcman_mc_slot = ((u32)(unit & mask)) / ((u32)(mask & (mask >> 0x3))) \
				+ (((mcman_mc_slot << 2) + mcman_mc_slot) << 1);
			mask >>= 4;
		}
	}
	*/
}

//--------------------------------------------------------------
int mcman_initdev(void)
{
	iop_sema_t smp;

#if !defined(BUILDING_XFROMMAN) && !defined(BUILDING_VMCMAN)
	SetCheckKelfPathCallback(mcman_modloadcb);
#endif

#if MCMAN_ENABLE_EXTENDED_DEV_OPS
	iomanX_DelDrv(mcman_mcdev.name);
	if (iomanX_AddDrv(&mcman_mcdev)) {
		McCloseAll();
		return 1;
	}
#else
	DelDrv(mcman_mcdev.name);
	if (AddDrv(&mcman_mcdev)) {
		McCloseAll();
		return 1;
	}
#endif

	smp.attr = 1;
	smp.initial = 1;
	smp.max = 1;
	smp.option = 0;
	mcman_io_sema = CreateSema(&smp);

	return 0;
}

//--------------------------------------------------------------
int mc_deinit(MC_IO_DEV_T *dev)
{
	(void)dev;

	DeleteSema(mcman_io_sema);
	McCloseAll();

	return 0;
}

//--------------------------------------------------------------
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
int mc_open(MC_IO_FIL_T *f, const char *filename, int flags, int mode)
#else
int mc_open(MC_IO_FIL_T *f, const char *filename, int flags)
#endif
{
	register int r;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		r = McOpen(mcman_mc_port, mcman_mc_slot, filename, flags);
		if (r >= 0)
			f->privdata = (void*)(uiptr)(int)r;
	}
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_close(MC_IO_FIL_T *f)
{
	register int r;

	WaitSema(mcman_io_sema);
	r = McClose((int)(uiptr)f->privdata);
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_lseek(MC_IO_FIL_T *f, int pos, int where)
{
	register int r;

	WaitSema(mcman_io_sema);
	r = McSeek((int)(uiptr)f->privdata, pos, where);
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_read(MC_IO_FIL_T *f, void *buf, int size)
{
	register int r;

	WaitSema(mcman_io_sema);
	r = McRead((int)(uiptr)f->privdata, buf, size);
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_write(MC_IO_FIL_T *f, void *buf, int size)
{
	register int r;

	WaitSema(mcman_io_sema);
	r = McWrite((int)(uiptr)f->privdata, buf, size);
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
int mc_format(MC_IO_FIL_T *f, const char *dev, const char *blockdev, void *arg, int arglen)
#else
int mc_format(MC_IO_FIL_T *f)
#endif
{
	register int r;

#if MCMAN_ENABLE_EXTENDED_DEV_OPS
	(void)dev;
	(void)blockdev;
	(void)arg;
	(void)arglen;
#endif

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -2) {
		r = McFormat(mcman_mc_port, mcman_mc_slot);
	}
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_remove(MC_IO_FIL_T *f, const char *filename)
{
	register int r;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		r = McDelete(mcman_mc_port, mcman_mc_slot, filename, 0);
	}
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
#if MCMAN_ENABLE_EXTENDED_DEV_OPS
int mc_mkdir(MC_IO_FIL_T *f, const char *dirname, int mode)
#else
int mc_mkdir(MC_IO_FIL_T *f, const char *dirname)
#endif
{
	register int r;

#if MCMAN_ENABLE_EXTENDED_DEV_OPS
	(void)mode;
#endif

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		r = McOpen(mcman_mc_port, mcman_mc_slot, dirname, 0x40);
	}
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_rmdir(MC_IO_FIL_T *f, const char *dirname)
{
	register int r;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		r = McDelete(mcman_mc_port, mcman_mc_slot, dirname, 0);
	}
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_dopen(MC_IO_FIL_T *f, const char *dirname)
{
	register int r;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		r = McOpen(mcman_mc_port, mcman_mc_slot, dirname, 0);
		if (r >= 0)
			f->privdata = (void*)(uiptr)(int)r;
	}

	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_dclose(MC_IO_FIL_T *f)
{
	register int r;

	WaitSema(mcman_io_sema);
	r = McClose((int)(uiptr)f->privdata);
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_dread(MC_IO_FIL_T *f, MC_IO_DRE_T *dirent)
{
	register int r;

	WaitSema(mcman_io_sema);
	r = mcman_dread((int)(uiptr)f->privdata, dirent);
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_getstat(MC_IO_FIL_T *f, const char *filename, MC_IO_STA_T *stat)
{
	register int r;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		r = mcman_getstat(mcman_mc_port, mcman_mc_slot, filename, stat);
	}

	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_chstat(MC_IO_FIL_T *f, const char *filename, MC_IO_STA_T *stat, unsigned int statmask)
{
	register int r;
	sceMcTblGetDir mctbl;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		register int flags;

		flags = 0x000;

		if (statmask & MC_IO_CST_ATTR) {
			flags |= 0x008;
			mctbl.Reserve2 = stat->attr;
		}

		if (statmask & MC_IO_CST_MODE) {
			flags |= 0x200;
			mctbl.AttrFile = 0;
			if (stat->mode & MC_IO_S_RD) mctbl.AttrFile |= sceMcFileAttrReadable;
			if (stat->mode & MC_IO_S_WR) mctbl.AttrFile |= sceMcFileAttrWriteable;
			if (stat->mode & MC_IO_S_EX) mctbl.AttrFile |= sceMcFileAttrExecutable;
#if !MCMAN_ENABLE_EXTENDED_DEV_OPS
			if (stat->mode & SCE_STM_C) mctbl.AttrFile |= sceMcFileAttrDupProhibit;
			if (stat->mode & sceMcFileAttrPS1) mctbl.AttrFile |= sceMcFileAttrPS1;
			if (stat->mode & sceMcFileAttrPDAExec) mctbl.AttrFile |= sceMcFileAttrPDAExec;
#endif
		}

		if (statmask & MC_IO_CST_CT) {
			flags |= 0x001;
			memcpy(&mctbl._Create, stat->ctime, sizeof(sceMcStDateTime));
		}

		if (statmask & MC_IO_CST_MT) {
			flags |= 0x002;
			memcpy(&mctbl._Modify, stat->mtime, sizeof(sceMcStDateTime));
		}

		r = McSetFileInfo(mcman_mc_port, mcman_mc_slot, filename, &mctbl, flags);
	}
	SignalSema(mcman_io_sema);

	return mcman_ioerrcode(r);
}

//--------------------------------------------------------------
int mc_ioctl(MC_IO_FIL_T *f, int cmd, void* param)
{
	(void)f;
	(void)cmd;
	(void)param;

	WaitSema(mcman_io_sema);
	SignalSema(mcman_io_sema);
	return 0;
}

#if MCMAN_ENABLE_EXTENDED_DEV_OPS
//--------------------------------------------------------------
int mc_rename(MC_IO_FIL_T *f, const char *old, const char *new)
{
	(void)f;
	(void)old;
	(void)new;

	register int r;
	sceMcTblGetDir mctbl;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);

	r = McDetectCard2(mcman_mc_port, mcman_mc_slot);
	if (r >= -1) {
		int l;

		l = strlen(new);
		if (l < 32) {
			memcpy((void *)mctbl.EntryName, new, l);
			mctbl.EntryName[l] = 0;
			r = McSetFileInfo(mcman_mc_port, mcman_mc_slot, old, &mctbl, sceMcFileAttrFile);
			r = mcman_ioerrcode(r);
		}
		else {
			r = -ENAMETOOLONG;
		}
	}
	else {
		r = mcman_ioerrcode(r);
	}
	SignalSema(mcman_io_sema);

	return r;
}

//--------------------------------------------------------------
int mc_chdir(MC_IO_FIL_T *f, const char *name)
{
	(void)f;
	(void)name;

	WaitSema(mcman_io_sema);
	SignalSema(mcman_io_sema);
	return 0;
}

//--------------------------------------------------------------
int mc_sync(MC_IO_FIL_T *f, const char *dev, int flag)
{
	(void)f;
	(void)dev;
	(void)flag;

	WaitSema(mcman_io_sema);
	SignalSema(mcman_io_sema);
	return 0;
}

//--------------------------------------------------------------
int mc_mount(MC_IO_FIL_T *f, const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
	int r;

	(void)fsname;
	(void)flag;
	(void)arg;
	(void)arglen;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);
#if defined(BUILDING_VMCMAN)
	r = mcman_iomanx_backing_mount(mcman_mc_port, mcman_mc_slot, devname);
#else
	r = 0;
#endif
	SignalSema(mcman_io_sema);
	return r;
}

//--------------------------------------------------------------
int mc_umount(MC_IO_FIL_T *f, const char *fsname)
{
	int r;

	(void)fsname;

	WaitSema(mcman_io_sema);
	mcman_unit2card(f->unit);
#if defined(BUILDING_VMCMAN)
	r = mcman_iomanx_backing_umount(mcman_mc_port, mcman_mc_slot);
#else
	r = 0;
#endif
	SignalSema(mcman_io_sema);
	return r;
}

//--------------------------------------------------------------
s64 mc_lseek64(MC_IO_FIL_T *f, s64 pos, int where)
{
	return mc_lseek(f, pos, where);
}

//--------------------------------------------------------------
int mc_devctl(MC_IO_FIL_T *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)name;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	WaitSema(mcman_io_sema);
	SignalSema(mcman_io_sema);
	return 0;
}

//--------------------------------------------------------------
int mc_symlink(MC_IO_FIL_T *f, const char *old, const char *new)
{
	(void)f;
	(void)old;
	(void)new;

	WaitSema(mcman_io_sema);
	SignalSema(mcman_io_sema);
	return 0;
}

//--------------------------------------------------------------
int mc_readlink(MC_IO_FIL_T *f, const char *path, char *buf, unsigned int buflen)
{
	(void)f;
	(void)path;
	(void)buf;
	(void)buflen;

	WaitSema(mcman_io_sema);
	SignalSema(mcman_io_sema);
	return 0;
}

//--------------------------------------------------------------
int mc_ioctl2(MC_IO_FIL_T *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	WaitSema(mcman_io_sema);
	SignalSema(mcman_io_sema);
	return 0;
}
#endif
