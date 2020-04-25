/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * fileXio RPC client
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <ps2sdkapi.h>

#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <errno.h>

extern int _iop_reboot_count;
static SifRpcClientData_t cd0;
static unsigned int sbuff[0x1300] __attribute__((aligned (64)));
static int _intr_data[0xC00] __attribute__((aligned(64)));
static int fileXioInited = 0;
static int fileXioBlockMode;
static int fileXioCompletionSema = -1;

static void _fxio_intr(void)
{
	iSignalSema(fileXioCompletionSema);
}

static int _lock_sema_id = -1;
static inline int _lock(void)
{
	return(WaitSema(_lock_sema_id));
}

static inline int _unlock(void)
{
	return(SignalSema(_lock_sema_id));
}

static time_t io_to_posix_time(const unsigned char *ps2time)
{
        struct tm tim;
        tim.tm_sec  = ps2time[1];
        tim.tm_min  = ps2time[2];
        tim.tm_hour = ps2time[3];
        tim.tm_mday = ps2time[4];
        tim.tm_mon  = ps2time[5] - 1;
        tim.tm_year = ((u16)ps2time[6] | ((u16)ps2time[7] << 8)) - 1900;
        return mktime(&tim);
}

static mode_t iox_to_posix_mode(unsigned int ps2mode)
{
        mode_t posixmode = 0;
        if (ps2mode & FIO_S_IFREG) posixmode |= S_IFREG;
        if (ps2mode & FIO_S_IFDIR) posixmode |= S_IFDIR;
        if (ps2mode & FIO_S_IRUSR) posixmode |= S_IRUSR;
        if (ps2mode & FIO_S_IWUSR) posixmode |= S_IWUSR;
        if (ps2mode & FIO_S_IXUSR) posixmode |= S_IXUSR;
        if (ps2mode & FIO_S_IRGRP) posixmode |= S_IRGRP;
        if (ps2mode & FIO_S_IWGRP) posixmode |= S_IWGRP;
        if (ps2mode & FIO_S_IXGRP) posixmode |= S_IXGRP;
        if (ps2mode & FIO_S_IROTH) posixmode |= S_IROTH;
        if (ps2mode & FIO_S_IWOTH) posixmode |= S_IWOTH;
        if (ps2mode & FIO_S_IXOTH) posixmode |= S_IXOTH;
        return posixmode;
}

static int fileXioGetstatHelper(const char *path, struct stat *buf) {
        iox_stat_t fiostat;

        if (fileXioGetStat(path, &fiostat) < 0) {
                // FIXME: set errno
                return -1;
        }

        buf->st_dev = 0;
        buf->st_ino = 0;
        buf->st_mode = iox_to_posix_mode(fiostat.mode);
        buf->st_nlink = 0;
        buf->st_uid = 0;
        buf->st_gid = 0;
        buf->st_rdev = 0;
        buf->st_size = ((off_t)fiostat.hisize << 32) | (off_t)fiostat.size;
        buf->st_atime = io_to_posix_time(fiostat.atime);
        buf->st_mtime = io_to_posix_time(fiostat.mtime);
        buf->st_ctime = io_to_posix_time(fiostat.ctime);
        buf->st_blksize = 16*1024;
        buf->st_blocks = buf->st_size / 512;

        return 0;
}

static DIR *fileXioOpendirHelper(const char *path)
{
	int dd;
	DIR *dir;

	dd = fileXioDopen(path);
	if (dd < 0) {
		// FIXME: set errno
		//printf("%s: ERROR: fileXioDopen\n", __FUNCTION__);
		return NULL;
	}

	dir = malloc(sizeof(DIR));
        dir->dd_fd = dd;
        dir->dd_loc = 0;
        dir->dd_size = 0;
        dir->dd_buf = malloc(sizeof(struct dirent) + 255);
        dir->dd_len = 0;
        dir->dd_seek = 0;

	return dir;
}

static struct dirent *fileXioReaddirHelper(DIR *dir)
{
	int rv;
        struct dirent *de;
        iox_dirent_t fiode;

	if(dir == NULL) {
		// FIXME: set errno
		return NULL;
	}

        de = (struct dirent *)dir->dd_buf;
        rv = fileXioDread(dir->dd_fd, &fiode);
	if (rv <= 0) {
		// FIXME: set errno
		return NULL;
	}

        de->d_ino = 0;
        de->d_off = 0;
        de->d_reclen = 0;
	strncpy(de->d_name, fiode.name, 255);
	de->d_name[255] = 0;

	return de;
}

static void fileXioRewinddirHelper(DIR *dir)
{
	printf("rewinddir not implemented\n");
}

static int fileXioClosedirHelper(DIR *dir)
{
	int rv;

	if(dir == NULL) {
		// FIXME: set errno
		return -1;
	}

	rv = fileXioDclose(dir->dd_fd); // Check return value?
	free(dir->dd_buf);
	free(dir);
	return 0;
}

int fileXioInit(void)
{
	int res;
	ee_sema_t sp;
	static int _rb_count = 0;

	if(_rb_count != _iop_reboot_count)
	{
		_rb_count = _iop_reboot_count;

		fileXioExit();
	}

	if(fileXioInited)
	{
		return 0;
	}

	sp.init_count = 1;
	sp.max_count = 1;
	sp.option = 0;
	_lock_sema_id = CreateSema(&sp);

	while(((res = SifBindRpc(&cd0, FILEXIO_IRX, 0)) >= 0) && (cd0.server == NULL))
		nopdelay();

	if(res < 0)
		return res;

	sp.init_count = 1;
	sp.max_count = 1;
	sp.option = 0;
	fileXioCompletionSema = CreateSema(&sp);
	if (fileXioCompletionSema < 0)
		return -1;

	fileXioInited = 1;
	fileXioBlockMode = FXIO_WAIT;

	_ps2sdk_close = fileXioClose;
	_ps2sdk_open = fileXioOpen;
	_ps2sdk_read = fileXioRead;
	_ps2sdk_lseek = fileXioLseek;
	_ps2sdk_write = fileXioWrite;
	_ps2sdk_ioctl = fileXioIoctl;
	_ps2sdk_remove= fileXioRemove;
	_ps2sdk_rename= fileXioRename;
	_ps2sdk_mkdir = fileXioMkdir;
	_ps2sdk_rmdir = fileXioRmdir;

	_ps2sdk_stat = fileXioGetstatHelper;

	_ps2sdk_opendir = fileXioOpendirHelper;
	_ps2sdk_readdir = fileXioReaddirHelper;
	_ps2sdk_rewinddir = fileXioRewinddirHelper;
	_ps2sdk_closedir = fileXioClosedirHelper;

	return 0;
}

void fileXioExit(void)
{
	if(fileXioInited)
	{
		if(_lock_sema_id >= 0) DeleteSema(_lock_sema_id);
		if(fileXioCompletionSema >= 0) DeleteSema(fileXioCompletionSema);

		memset(&cd0, 0, sizeof(cd0));

		fileXioInited = 0;
	}
}

void fileXioStop(void)
{
	if(fileXioInit() < 0)
		return;

	SifCallRpc(&cd0, FILEXIO_STOP, 0, sbuff, 0, sbuff, 0, 0, 0);

	return;
}

int fileXioGetDeviceList(struct fileXioDevice deviceEntry[], unsigned int req_entries)
{
	int rv;
	struct fxio_devlist_packet *packet=(struct fxio_devlist_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->deviceEntry = deviceEntry;
	packet->reqEntries = req_entries;

	// This will get the directory contents, and fill dirEntry via DMA
	if((rv = SifCallRpc(&cd0, FILEXIO_GETDEVICELIST, fileXioBlockMode, sbuff, sizeof(struct fxio_devlist_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioGetdir(const char* pathname, struct fileXioDirEntry dirEntry[], unsigned int req_entries)
{
	int rv;
	struct fxio_getdir_packet *packet=(struct fxio_getdir_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	// copy the requested pathname to the rpc buffer
	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	SifWriteBackDCache(dirEntry, (sizeof(struct fileXioDirEntry) * req_entries));

	packet->dirEntry = dirEntry;
	packet->reqEntries = req_entries;

	// This will get the directory contents, and fill dirEntry via DMA
	if((rv = SifCallRpc(&cd0, FILEXIO_GETDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_getdir_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioMount(const char* mountpoint, const char* mountstring, int flag)
{
	int rv;
	struct fxio_mount_packet *packet=(struct fxio_mount_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->blockdevice, mountstring, sizeof(packet->blockdevice));
	strncpy(packet->mountpoint, mountpoint, sizeof(packet->mountpoint));
	packet->flags = flag;

	if((rv = SifCallRpc(&cd0, FILEXIO_MOUNT, fileXioBlockMode, sbuff, sizeof(struct fxio_mount_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioUmount(const char* mountpoint)
{
	int rv;
	struct fxio_unmount_packet *packet=(struct fxio_unmount_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->mountpoint, mountpoint, sizeof(packet->mountpoint));

	if((rv = SifCallRpc(&cd0, FILEXIO_UMOUNT, fileXioBlockMode, sbuff, sizeof(struct fxio_unmount_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioCopyfile(const char* source, const char* dest, int mode)
{
	int rv;
	struct fxio_copyfile_packet *packet=(struct fxio_copyfile_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));
	packet->mode = mode;

	if((rv = SifCallRpc(&cd0, FILEXIO_COPYFILE, fileXioBlockMode, sbuff, sizeof(struct fxio_copyfile_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioMkdir(const char* pathname, int mode)
{
	int rv;
	struct fxio_mkdir_packet *packet=(struct fxio_mkdir_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));
	packet->mode = mode;

	if((rv = SifCallRpc(&cd0, FILEXIO_MKDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_mkdir_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioRmdir(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&cd0, FILEXIO_RMDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioRemove(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&cd0, FILEXIO_REMOVE, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioRename(const char* source, const char* dest)
{
	int rv;
	struct fxio_rename_packet *packet=(struct fxio_rename_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));

	if((rv = SifCallRpc(&cd0, FILEXIO_RENAME, fileXioBlockMode, sbuff, sizeof(struct fxio_rename_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioSymlink(const char* source, const char* dest)
{
	int rv;
	struct fxio_rename_packet *packet=(struct fxio_rename_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));

	if((rv = SifCallRpc(&cd0, FILEXIO_SYMLINK, fileXioBlockMode, sbuff, sizeof(struct fxio_rename_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioReadlink(const char* source, char* buf, int buflen)
{
	int rv;
	struct fxio_readlink_packet *packet=(struct fxio_readlink_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if( !IS_UNCACHED_SEG(buf))
  	  SifWriteBackDCache(buf, buflen);

	strncpy(packet->source, source, sizeof(packet->source));
	packet->buffer = buf;
	packet->buflen = buflen;

	if((rv = SifCallRpc(&cd0, FILEXIO_READLINK, fileXioBlockMode, sbuff, sizeof(struct fxio_readlink_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioChdir(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&cd0, FILEXIO_CHDIR, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioOpen(const char* source, int flags, ...)
{
	int rv, mode;
	struct fxio_open_packet *packet=(struct fxio_open_packet*)sbuff;
	va_list alist;

	va_start(alist, flags);
	mode = va_arg(alist, int);	//Retrieve the mode argument, regardless of whether it is expected or not.
	va_end(alist);

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, source, sizeof(packet->pathname));
	packet->flags = flags;
	packet->mode = mode;
	if((rv = SifCallRpc(&cd0, FILEXIO_OPEN, fileXioBlockMode, sbuff, sizeof(struct fxio_open_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioClose(int fd)
{
	int rv;
	struct fxio_close_packet *packet=(struct fxio_close_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;

	if((rv = SifCallRpc(&cd0, FILEXIO_CLOSE, fileXioBlockMode, sbuff, sizeof(struct fxio_close_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

static void recv_intr(void *data_raw)
{
	rests_pkt *rests = UNCACHED_SEG(data_raw);

	if(rests->ssize) memcpy(rests->sbuf, rests->sbuffer, rests->ssize);
	if(rests->esize) memcpy(rests->ebuf, rests->ebuffer, rests->esize);

	iSignalSema(fileXioCompletionSema);
}

int fileXioRead(int fd, void *buf, int size)
{
	int rv;
	struct fxio_read_packet *packet=(struct fxio_read_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->buffer = buf;
	packet->size = size;
	packet->intrData = _intr_data;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, size);

	if((rv = SifCallRpc(&cd0, FILEXIO_READ, fileXioBlockMode, sbuff, sizeof(struct fxio_read_packet), sbuff, 4, &recv_intr, _intr_data)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioWrite(int fd, const void *buf, int size)
{
	unsigned int miss;
	int rv;
	struct fxio_write_packet *packet=(struct fxio_write_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if((unsigned int)buf & 0x3F)
	{
		miss = 64 - ((unsigned int)buf & 0x3F);
		if(miss > size) miss = size;
	} else {
		miss = 0;
	}

	packet->fd = fd;
	packet->buffer = buf;
	packet->size = size;
	packet->unalignedDataLen = miss;

	memcpy(packet->unalignedData, buf, miss);

	if(!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache((void*)buf, size);

	if((rv = SifCallRpc(&cd0, FILEXIO_WRITE, fileXioBlockMode, sbuff, sizeof(struct fxio_write_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioLseek(int fd, int offset, int whence)
{
	int rv;
	struct fxio_lseek_packet *packet=(struct fxio_lseek_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->offset = (u32)offset;
	packet->whence = whence;

	if((rv = SifCallRpc(&cd0, FILEXIO_LSEEK, fileXioBlockMode, sbuff, sizeof(struct fxio_lseek_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

//
// NOTE: 64-bit
//
s64 fileXioLseek64(int fd, s64 offset, int whence)
{
	s64 rv;
	struct fxio_lseek64_packet *packet=(struct fxio_lseek64_packet*)sbuff;
	struct fxio_lseek64_return_pkt *ret_packet=(struct fxio_lseek64_return_pkt*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->offset_lo = (u32)(offset & 0xffffffff);
	packet->offset_hi = (u32)((offset >> 32) & 0xffffffff);
	packet->whence = whence;

	if((rv = SifCallRpc(&cd0, FILEXIO_LSEEK64, fileXioBlockMode, sbuff, sizeof(struct fxio_lseek64_packet), sbuff, 8, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else {
			s64 rvHI = ret_packet->pos_hi;
			rvHI = rvHI << 32;
			rv = rvHI | ret_packet->pos_lo;
		}
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();

	return(rv);
}

int fileXioChStat(const char *name, iox_stat_t *stat, int mask)
{
	int rv;
	struct fxio_chstat_packet *packet=(struct fxio_chstat_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	packet->stat = stat;
	packet->mask = mask;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(iox_stat_t));

	if((rv = SifCallRpc(&cd0, FILEXIO_CHSTAT, fileXioBlockMode, sbuff, sizeof(struct fxio_chstat_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioGetStat(const char *name, iox_stat_t *stat)
{
	int rv;
	struct fxio_getstat_packet *packet=(struct fxio_getstat_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	packet->stat = stat;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(iox_stat_t));

	if((rv = SifCallRpc(&cd0, FILEXIO_GETSTAT, fileXioBlockMode, sbuff, sizeof(struct fxio_getstat_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioFormat(const char *dev, const char *blockdev, const void *args, int arglen)
{
	int rv;
	struct fxio_format_packet *packet=(struct fxio_format_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->device, dev, sizeof(packet->device));
	if(blockdev)
		strncpy(packet->blockDevice, blockdev, sizeof(packet->blockDevice));

	if(arglen > sizeof(packet->args)) arglen = sizeof(packet->args);
	memcpy(packet->args, args, arglen);
	packet->arglen = arglen;

	if((rv = SifCallRpc(&cd0, FILEXIO_FORMAT, fileXioBlockMode,  sbuff, sizeof(struct fxio_format_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioSync(const char *devname, int flag)
{
	int rv;
	struct fxio_sync_packet *packet=(struct fxio_sync_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->device, devname, sizeof(packet->device));
	packet->flags = flag;

	if((rv = SifCallRpc(&cd0, FILEXIO_SYNC, fileXioBlockMode, sbuff, sizeof(struct fxio_sync_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioDopen(const char *name)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	if((rv = SifCallRpc(&cd0, FILEXIO_DOPEN, fileXioBlockMode, sbuff, sizeof(struct fxio_pathsel_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioDclose(int fd)
{
	int rv;
	struct fxio_close_packet *packet=(struct fxio_close_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	if((rv = SifCallRpc(&cd0, FILEXIO_DCLOSE, fileXioBlockMode, sbuff, sizeof(struct fxio_close_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioDread(int fd, iox_dirent_t *dirent)
{
	int rv;
	struct fxio_dread_packet *packet=(struct fxio_dread_packet*)sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->fd = fd;
	packet->dirent = dirent;

	if (!IS_UNCACHED_SEG(dirent))
		SifWriteBackDCache(dirent, sizeof(iox_dirent_t));

	if((rv = SifCallRpc(&cd0, FILEXIO_DREAD, fileXioBlockMode, sbuff, sizeof(struct fxio_dread_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

static void fxio_ctl_intr(void *data_raw)
{
	struct fxio_ctl_return_pkt *pkt = UNCACHED_SEG(data_raw);

	memcpy(pkt->dest, pkt->buf, pkt->len);

	iSignalSema(fileXioCompletionSema);
}

int fileXioDevctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct fxio_devctl_packet *packet = (struct fxio_devctl_packet *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if(arglen > CTL_BUF_SIZE) arglen = CTL_BUF_SIZE;
	if(buflen > CTL_BUF_SIZE) buflen = CTL_BUF_SIZE;
	strncpy(packet->name, name, CTL_BUF_SIZE);
	packet->name[CTL_BUF_SIZE-1] = '\0';
	memcpy(packet->arg, arg, arglen);

	packet->cmd = cmd;
	packet->arglen = arglen;
	packet->buf = buf;
	packet->buflen = buflen;
	packet->intr_data = _intr_data;

	SifWriteBackDCache(buf, buflen);

	if(buflen)
		rv = SifCallRpc(&cd0, FILEXIO_DEVCTL, fileXioBlockMode, packet, sizeof(struct fxio_devctl_packet), sbuff, 4, &fxio_ctl_intr, _intr_data);
	else
		rv = SifCallRpc(&cd0, FILEXIO_DEVCTL, fileXioBlockMode, packet, sizeof(struct fxio_devctl_packet), sbuff, 4, (void *)&_fxio_intr, NULL);

	if(rv >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioIoctl(int fd, int cmd, void *arg){
	struct fxio_ioctl_packet *packet = (struct fxio_ioctl_packet *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	memcpy(packet->arg, arg, IOCTL_BUF_SIZE);

	packet->fd = fd;
	packet->cmd = cmd;

	if((rv = SifCallRpc(&cd0, FILEXIO_IOCTL, fileXioBlockMode, packet, sizeof(struct fxio_ioctl_packet), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioIoctl2(int fd, int command, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct fxio_ioctl2_packet *packet = (struct fxio_ioctl2_packet *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	if(arglen > CTL_BUF_SIZE) arglen = CTL_BUF_SIZE;
	if(buflen > CTL_BUF_SIZE) buflen = CTL_BUF_SIZE;
	memcpy(packet->arg, arg, arglen);

	packet->fd = fd;
	packet->cmd = command;
	packet->arglen = arglen;
	packet->buf = buf;
	packet->buflen = buflen;
	packet->intr_data = _intr_data;

	SifWriteBackDCache(buf, buflen);

	if(buflen)
		rv = SifCallRpc(&cd0, FILEXIO_IOCTL2, fileXioBlockMode, packet, sizeof(struct fxio_ioctl2_packet), sbuff, 4, &fxio_ctl_intr, _intr_data);
	else
		rv = SifCallRpc(&cd0, FILEXIO_IOCTL2, fileXioBlockMode, packet, sizeof(struct fxio_ioctl2_packet), sbuff, 4, (void *)&_fxio_intr, NULL);

	if(rv >= 0)
	{
		if(fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = sbuff[0]; }
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

int fileXioWaitAsync(int mode, int *retVal)
{
	if(fileXioInit() < 0)
		return -ENOPKG;

	if(fileXioBlockMode != FXIO_NOWAIT) return 0;

	switch(mode)
	{
		case FXIO_WAIT:

			WaitSema(fileXioCompletionSema);
			SignalSema(fileXioCompletionSema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&sbuff[0]);

			return FXIO_COMPLETE;

		case FXIO_NOWAIT:

			if(PollSema(fileXioCompletionSema) < 0)
				return FXIO_INCOMPLETE;

			SignalSema(fileXioCompletionSema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&sbuff[0]);

			return FXIO_COMPLETE;

		default:
			return -1;
	}
}

void fileXioSetBlockMode(int blocking)
{
	fileXioBlockMode = blocking;
}

int fileXioSetRWBufferSize(int size){
	struct fxio_rwbuff *packet = (struct fxio_rwbuff *)sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(fileXioCompletionSema);

	packet->size = size;

	if((rv = SifCallRpc(&cd0, FILEXIO_SETRWBUFFSIZE, 0, packet, sizeof(struct fxio_rwbuff), sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		rv = sbuff[0];
	}
	else
		SignalSema(fileXioCompletionSema);

	_unlock();
	return(rv);
}

