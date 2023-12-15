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
#include <stdlib.h>
#include <stdarg.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <ps2sdkapi.h>

#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <errno.h>

extern int _iop_reboot_count;

/* Extern symbols to help fileXio override weak methods for fileXio_ps2sdk */
/* More info here: https://stackoverflow.com/a/22178564 */
extern uint8_t __ps2sdk_fileXio_close;
extern uint8_t __ps2sdk_fileXio_open;
extern uint8_t __ps2sdk_fileXio_read;
extern uint8_t __ps2sdk_fileXio_lseek;
extern uint8_t __ps2sdk_fileXio_lseek64;
extern uint8_t __ps2sdk_fileXio_write;
extern uint8_t __ps2sdk_fileXio_ioctl;
extern uint8_t __ps2sdk_fileXio_remove;
extern uint8_t __ps2sdk_fileXio_rename;
extern uint8_t __ps2sdk_fileXio_mkdir;
extern uint8_t __ps2sdk_fileXio_rmdir;
extern uint8_t __ps2sdk_fileXio_stat;
extern uint8_t __ps2sdk_fileXio_readlink;
extern uint8_t __ps2sdk_fileXio_symlink;
extern uint8_t __ps2sdk_fileXio_dopen;
extern uint8_t __ps2sdk_fileXio_dread;
extern uint8_t __ps2sdk_fileXio_dclose;

#ifdef F___cd0
SifRpcClientData_t __cd0;
#else
extern SifRpcClientData_t __cd0;
#endif

#ifdef F___sbuff
unsigned int __sbuff[0x1300] __attribute__((aligned (64)));
#else
extern unsigned int __sbuff[0x1300] __attribute__((aligned (64)));
#endif

#ifdef F___intr_data
int __intr_data[0xC00] __attribute__((aligned(64)));
#else
extern int __intr_data[0xC00] __attribute__((aligned(64)));
#endif

#ifdef F___fileXioInited
int __fileXioInited = 0;
#else
extern int __fileXioInited;
#endif

#ifdef F___fileXioBlockMode
int __fileXioBlockMode;
#else
extern int __fileXioBlockMode;
#endif

#ifdef F___fileXioCompletionSema
int __fileXioCompletionSema = -1;
#else
extern int __fileXioCompletionSema;
#endif

#ifdef F___lock_sema_id
int __lock_sema_id = -1;
#else
extern int __lock_sema_id;
#endif

static inline void _fxio_intr(void)
{
	iSignalSema(__fileXioCompletionSema);
}

static inline int _lock(void)
{
	return(WaitSema(__lock_sema_id));
}

static inline int _unlock(void)
{
	return(SignalSema(__lock_sema_id));
}

#ifdef F_fileXioInit
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

	if(__fileXioInited)
	{
		return 0;
	}

	sp.init_count = 1;
	sp.max_count = 1;
	sp.option = 0;
	__lock_sema_id = CreateSema(&sp);

	while(((res = SifBindRpc(&__cd0, FILEXIO_IRX, 0)) >= 0) && (__cd0.server == NULL))
		nopdelay();

	if(res < 0)
		return res;

	sp.init_count = 1;
	sp.max_count = 1;
	sp.option = 0;
	__fileXioCompletionSema = CreateSema(&sp);
	if (__fileXioCompletionSema < 0)
		return -1;

	__fileXioInited = 1;
	__fileXioBlockMode = FXIO_WAIT;

	return 0;
}
#endif

#ifdef F_fileXioExit
void fileXioExit(void)
{
	if(__fileXioInited)
	{
		if(__lock_sema_id >= 0) DeleteSema(__lock_sema_id);
		if(__fileXioCompletionSema >= 0) DeleteSema(__fileXioCompletionSema);

		memset(&__cd0, 0, sizeof(__cd0));

		__fileXioInited = 0;
	}
}
#endif

#ifdef F_fileXioStop
void fileXioStop(void)
{
	if(fileXioInit() < 0)
		return;

	SifCallRpc(&__cd0, FILEXIO_STOP, 0, __sbuff, 0, __sbuff, 0, 0, 0);

	return;
}
#endif

#ifdef F_fileXioGetDeviceList
int fileXioGetDeviceList(struct fileXioDevice deviceEntry[], unsigned int req_entries)
{
	int rv;
	struct fxio_devlist_packet *packet=(struct fxio_devlist_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->deviceEntry = deviceEntry;
	packet->reqEntries = req_entries;

	// This will get the directory contents, and fill dirEntry via DMA
	if((rv = SifCallRpc(&__cd0, FILEXIO_GETDEVICELIST, __fileXioBlockMode, __sbuff, sizeof(struct fxio_devlist_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioGetdir
int fileXioGetdir(const char* pathname, struct fileXioDirEntry dirEntry[], unsigned int req_entries)
{
	int rv;
	struct fxio_getdir_packet *packet=(struct fxio_getdir_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	// copy the requested pathname to the rpc buffer
	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	SifWriteBackDCache(dirEntry, (sizeof(struct fileXioDirEntry) * req_entries));

	packet->dirEntry = dirEntry;
	packet->reqEntries = req_entries;

	// This will get the directory contents, and fill dirEntry via DMA
	if((rv = SifCallRpc(&__cd0, FILEXIO_GETDIR, __fileXioBlockMode, __sbuff, sizeof(struct fxio_getdir_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioMount
int fileXioMount(const char* mountpoint, const char* mountstring, int flag)
{
	int rv;
	struct fxio_mount_packet *packet=(struct fxio_mount_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->blockdevice, mountstring, sizeof(packet->blockdevice));
	strncpy(packet->mountpoint, mountpoint, sizeof(packet->mountpoint));
	packet->flags = flag;

	if((rv = SifCallRpc(&__cd0, FILEXIO_MOUNT, __fileXioBlockMode, __sbuff, sizeof(struct fxio_mount_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioUmount
int fileXioUmount(const char* mountpoint)
{
	int rv;
	struct fxio_unmount_packet *packet=(struct fxio_unmount_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->mountpoint, mountpoint, sizeof(packet->mountpoint));

	if((rv = SifCallRpc(&__cd0, FILEXIO_UMOUNT, __fileXioBlockMode, __sbuff, sizeof(struct fxio_unmount_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioCopyfile
int fileXioCopyfile(const char* source, const char* dest, int mode)
{
	int rv;
	struct fxio_copyfile_packet *packet=(struct fxio_copyfile_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));
	packet->mode = mode;

	if((rv = SifCallRpc(&__cd0, FILEXIO_COPYFILE, __fileXioBlockMode, __sbuff, sizeof(struct fxio_copyfile_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioMkdir
int fileXioMkdir(const char* pathname, int mode)
{
	int rv;
	struct fxio_mkdir_packet *packet=(struct fxio_mkdir_packet*)__sbuff;

	__ps2sdk_fileXio_mkdir = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));
	packet->mode = mode;

	if((rv = SifCallRpc(&__cd0, FILEXIO_MKDIR, __fileXioBlockMode, __sbuff, sizeof(struct fxio_mkdir_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioRmdir
int fileXioRmdir(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)__sbuff;

	__ps2sdk_fileXio_rmdir = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&__cd0, FILEXIO_RMDIR, __fileXioBlockMode, __sbuff, sizeof(struct fxio_pathsel_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioRemove
int fileXioRemove(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)__sbuff;

	__ps2sdk_fileXio_remove = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&__cd0, FILEXIO_REMOVE, __fileXioBlockMode, __sbuff, sizeof(struct fxio_pathsel_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioRename
int fileXioRename(const char* source, const char* dest)
{
	int rv;
	struct fxio_rename_packet *packet=(struct fxio_rename_packet*)__sbuff;

	__ps2sdk_fileXio_rename = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));

	if((rv = SifCallRpc(&__cd0, FILEXIO_RENAME, __fileXioBlockMode, __sbuff, sizeof(struct fxio_rename_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioSymlink
int fileXioSymlink(const char* source, const char* dest)
{
	int rv;
	struct fxio_rename_packet *packet=(struct fxio_rename_packet*)__sbuff;

	__ps2sdk_fileXio_symlink = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->source, source, sizeof(packet->source));
	strncpy(packet->dest, dest, sizeof(packet->dest));

	if((rv = SifCallRpc(&__cd0, FILEXIO_SYMLINK, __fileXioBlockMode, __sbuff, sizeof(struct fxio_rename_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioReadlink
int fileXioReadlink(const char* source, char* buf, unsigned int buflen)
{
	int rv;
	struct fxio_readlink_packet *packet=(struct fxio_readlink_packet*)__sbuff;

	__ps2sdk_fileXio_readlink = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	if( !IS_UNCACHED_SEG(buf))
  	  SifWriteBackDCache(buf, buflen);

	strncpy(packet->source, source, sizeof(packet->source));
	packet->buffer = buf;
	packet->buflen = buflen;

	if((rv = SifCallRpc(&__cd0, FILEXIO_READLINK, __fileXioBlockMode, __sbuff, sizeof(struct fxio_readlink_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioChdir
int fileXioChdir(const char* pathname)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, pathname, sizeof(packet->pathname));

	if((rv = SifCallRpc(&__cd0, FILEXIO_CHDIR, __fileXioBlockMode, __sbuff, sizeof(struct fxio_pathsel_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioOpen
int fileXioOpen(const char* source, int flags, ...)
{
	int rv, mode;
	struct fxio_open_packet *packet=(struct fxio_open_packet*)__sbuff;
	va_list alist;

	va_start(alist, flags);
	mode = va_arg(alist, int);	//Retrieve the mode argument, regardless of whether it is expected or not.
	va_end(alist);

	__ps2sdk_fileXio_open = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, source, sizeof(packet->pathname));
	packet->flags = flags;
	packet->mode = mode;
	if((rv = SifCallRpc(&__cd0, FILEXIO_OPEN, __fileXioBlockMode, __sbuff, sizeof(struct fxio_open_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioClose
int fileXioClose(int fd)
{
	int rv;
	struct fxio_close_packet *packet=(struct fxio_close_packet*)__sbuff;

	__ps2sdk_fileXio_close = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->fd = fd;

	if((rv = SifCallRpc(&__cd0, FILEXIO_CLOSE, __fileXioBlockMode, __sbuff, sizeof(struct fxio_close_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioRead
static inline void recv_intr(void *data_raw)
{
	rests_pkt *rests = UNCACHED_SEG(data_raw);

	if(rests->ssize) memcpy(rests->sbuf, rests->sbuffer, rests->ssize);
	if(rests->esize) memcpy(rests->ebuf, rests->ebuffer, rests->esize);

	iSignalSema(__fileXioCompletionSema);
}

int fileXioRead(int fd, void *buf, int size)
{
	int rv;
	struct fxio_read_packet *packet=(struct fxio_read_packet*)__sbuff;

	__ps2sdk_fileXio_read = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->fd = fd;
	packet->buffer = buf;
	packet->size = size;
	packet->intrData = __intr_data;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, size);

	if((rv = SifCallRpc(&__cd0, FILEXIO_READ, __fileXioBlockMode, __sbuff, sizeof(struct fxio_read_packet), __sbuff, 4, &recv_intr, __intr_data)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioWrite
int fileXioWrite(int fd, const void *buf, int size)
{
	unsigned int miss;
	int rv;
	struct fxio_write_packet *packet=(struct fxio_write_packet*)__sbuff;

	__ps2sdk_fileXio_write = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	if((unsigned int)buf & 0x3F)
	{
		miss = 64 - ((unsigned int)buf & 0x3F);
		if(miss > (unsigned int)size) miss = size;
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

	if((rv = SifCallRpc(&__cd0, FILEXIO_WRITE, __fileXioBlockMode, __sbuff, sizeof(struct fxio_write_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioLseek
int fileXioLseek(int fd, int offset, int whence)
{
	int rv;
	struct fxio_lseek_packet *packet=(struct fxio_lseek_packet*)__sbuff;

	__ps2sdk_fileXio_lseek = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->fd = fd;
	packet->offset = (u32)offset;
	packet->whence = whence;

	if((rv = SifCallRpc(&__cd0, FILEXIO_LSEEK, __fileXioBlockMode, __sbuff, sizeof(struct fxio_lseek_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioLseek64
//
// NOTE: 64-bit
//
s64 fileXioLseek64(int fd, s64 offset, int whence)
{
	s64 rv;
	struct fxio_lseek64_packet *packet=(struct fxio_lseek64_packet*)__sbuff;
	struct fxio_lseek64_return_pkt *ret_packet=(struct fxio_lseek64_return_pkt*)__sbuff;

	__ps2sdk_fileXio_lseek64 = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->fd = fd;
	packet->offset_lo = (u32)(offset & 0xffffffff);
	packet->offset_hi = (u32)((offset >> 32) & 0xffffffff);
	packet->whence = whence;

	if((rv = SifCallRpc(&__cd0, FILEXIO_LSEEK64, __fileXioBlockMode, __sbuff, sizeof(struct fxio_lseek64_packet), __sbuff, 8, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else {
			s64 rvHI = ret_packet->pos_hi;
			rvHI = rvHI << 32;
			rv = rvHI | ret_packet->pos_lo;
		}
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();

	return(rv);
}
#endif

#ifdef F_fileXioChStat
int fileXioChStat(const char *name, iox_stat_t *stat, int mask)
{
	int rv;
	struct fxio_chstat_packet *packet=(struct fxio_chstat_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	packet->stat = stat;
	packet->mask = mask;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(iox_stat_t));

	if((rv = SifCallRpc(&__cd0, FILEXIO_CHSTAT, __fileXioBlockMode, __sbuff, sizeof(struct fxio_chstat_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioGetStat
int fileXioGetStat(const char *name, iox_stat_t *stat)
{
	int rv;
	struct fxio_getstat_packet *packet=(struct fxio_getstat_packet*)__sbuff;

	__ps2sdk_fileXio_stat = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	packet->stat = stat;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(iox_stat_t));

	if((rv = SifCallRpc(&__cd0, FILEXIO_GETSTAT, __fileXioBlockMode, __sbuff, sizeof(struct fxio_getstat_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioFormat
int fileXioFormat(const char *dev, const char *blockdev, const void *args, int arglen)
{
	int rv;
	struct fxio_format_packet *packet=(struct fxio_format_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->device, dev, sizeof(packet->device));
	if(blockdev)
		strncpy(packet->blockDevice, blockdev, sizeof(packet->blockDevice));

	if((unsigned int)arglen > sizeof(packet->args)) arglen = sizeof(packet->args);
	memcpy(packet->args, args, arglen);
	packet->arglen = arglen;

	if((rv = SifCallRpc(&__cd0, FILEXIO_FORMAT, __fileXioBlockMode,  __sbuff, sizeof(struct fxio_format_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioSync
int fileXioSync(const char *devname, int flag)
{
	int rv;
	struct fxio_sync_packet *packet=(struct fxio_sync_packet*)__sbuff;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->device, devname, sizeof(packet->device));
	packet->flags = flag;

	if((rv = SifCallRpc(&__cd0, FILEXIO_SYNC, __fileXioBlockMode, __sbuff, sizeof(struct fxio_sync_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioDopen
int fileXioDopen(const char *name)
{
	int rv;
	struct fxio_pathsel_packet *packet=(struct fxio_pathsel_packet*)__sbuff;

	__ps2sdk_fileXio_dopen = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	strncpy(packet->pathname, name, sizeof(packet->pathname));
	if((rv = SifCallRpc(&__cd0, FILEXIO_DOPEN, __fileXioBlockMode, __sbuff, sizeof(struct fxio_pathsel_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioDclose
int fileXioDclose(int fd)
{
	int rv;
	struct fxio_close_packet *packet=(struct fxio_close_packet*)__sbuff;

	__ps2sdk_fileXio_dclose = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->fd = fd;
	if((rv = SifCallRpc(&__cd0, FILEXIO_DCLOSE, __fileXioBlockMode, __sbuff, sizeof(struct fxio_close_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioDread
int fileXioDread(int fd, iox_dirent_t *dirent)
{
	int rv;
	struct fxio_dread_packet *packet=(struct fxio_dread_packet*)__sbuff;

	__ps2sdk_fileXio_dread = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->fd = fd;
	packet->dirent = dirent;

	if (!IS_UNCACHED_SEG(dirent))
		SifWriteBackDCache(dirent, sizeof(iox_dirent_t));

	if((rv = SifCallRpc(&__cd0, FILEXIO_DREAD, __fileXioBlockMode, __sbuff, sizeof(struct fxio_dread_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

static inline void fxio_ctl_intr(void *data_raw)
{
	struct fxio_ctl_return_pkt *pkt = UNCACHED_SEG(data_raw);

	memcpy(pkt->dest, pkt->buf, pkt->len);

	iSignalSema(__fileXioCompletionSema);
}

#ifdef F_fileXioDevctl
int fileXioDevctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct fxio_devctl_packet *packet = (struct fxio_devctl_packet *)__sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	if(arglen > CTL_BUF_SIZE) arglen = CTL_BUF_SIZE;
	if(buflen > CTL_BUF_SIZE) buflen = CTL_BUF_SIZE;
	strncpy(packet->name, name, CTL_BUF_SIZE);
	packet->name[CTL_BUF_SIZE-1] = '\0';
	memcpy(packet->arg, arg, arglen);

	packet->cmd = cmd;
	packet->arglen = arglen;
	packet->buf = buf;
	packet->buflen = buflen;
	packet->intr_data = __intr_data;

	SifWriteBackDCache(buf, buflen);

	if(buflen)
		rv = SifCallRpc(&__cd0, FILEXIO_DEVCTL, __fileXioBlockMode, packet, sizeof(struct fxio_devctl_packet), __sbuff, 4, &fxio_ctl_intr, __intr_data);
	else
		rv = SifCallRpc(&__cd0, FILEXIO_DEVCTL, __fileXioBlockMode, packet, sizeof(struct fxio_devctl_packet), __sbuff, 4, (void *)&_fxio_intr, NULL);

	if(rv >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioIoctl
int fileXioIoctl(int fd, int cmd, void *arg){
	struct fxio_ioctl_packet *packet = (struct fxio_ioctl_packet *)__sbuff;
	int rv;

	__ps2sdk_fileXio_ioctl = 1;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	memcpy(packet->arg, arg, IOCTL_BUF_SIZE);

	packet->fd = fd;
	packet->cmd = cmd;

	if((rv = SifCallRpc(&__cd0, FILEXIO_IOCTL, __fileXioBlockMode, packet, sizeof(struct fxio_ioctl_packet), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioIoctl2
int fileXioIoctl2(int fd, int command, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct fxio_ioctl2_packet *packet = (struct fxio_ioctl2_packet *)__sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	if(arglen > CTL_BUF_SIZE) arglen = CTL_BUF_SIZE;
	if(buflen > CTL_BUF_SIZE) buflen = CTL_BUF_SIZE;
	memcpy(packet->arg, arg, arglen);

	packet->fd = fd;
	packet->cmd = command;
	packet->arglen = arglen;
	packet->buf = buf;
	packet->buflen = buflen;
	packet->intr_data = __intr_data;

	SifWriteBackDCache(buf, buflen);

	if(buflen)
		rv = SifCallRpc(&__cd0, FILEXIO_IOCTL2, __fileXioBlockMode, packet, sizeof(struct fxio_ioctl2_packet), __sbuff, 4, &fxio_ctl_intr, __intr_data);
	else
		rv = SifCallRpc(&__cd0, FILEXIO_IOCTL2, __fileXioBlockMode, packet, sizeof(struct fxio_ioctl2_packet), __sbuff, 4, (void *)&_fxio_intr, NULL);

	if(rv >= 0)
	{
		if(__fileXioBlockMode == FXIO_NOWAIT) { rv = 0; }
		else { rv = __sbuff[0]; }
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif

#ifdef F_fileXioWaitAsync
int fileXioWaitAsync(int mode, int *retVal)
{
	if(fileXioInit() < 0)
		return -ENOPKG;

	if(__fileXioBlockMode != FXIO_NOWAIT) return 0;

	switch(mode)
	{
		case FXIO_WAIT:

			WaitSema(__fileXioCompletionSema);
			SignalSema(__fileXioCompletionSema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&__sbuff[0]);

			return FXIO_COMPLETE;

		case FXIO_NOWAIT:

			if(PollSema(__fileXioCompletionSema) < 0)
				return FXIO_INCOMPLETE;

			SignalSema(__fileXioCompletionSema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&__sbuff[0]);

			return FXIO_COMPLETE;

		default:
			return -1;
	}
}
#endif

#ifdef F_fileXioSetBlockMode
void fileXioSetBlockMode(int blocking)
{
	__fileXioBlockMode = blocking;
}
#endif

#ifdef F_fileXioSetRWBufferSize
int fileXioSetRWBufferSize(int size){
	struct fxio_rwbuff *packet = (struct fxio_rwbuff *)__sbuff;
	int rv;

	if(fileXioInit() < 0)
		return -ENOPKG;

	_lock();
	WaitSema(__fileXioCompletionSema);

	packet->size = size;

	if((rv = SifCallRpc(&__cd0, FILEXIO_SETRWBUFFSIZE, 0, packet, sizeof(struct fxio_rwbuff), __sbuff, 4, (void *)&_fxio_intr, NULL)) >= 0)
	{
		rv = __sbuff[0];
	}
	else
		SignalSema(__fileXioCompletionSema);

	_unlock();
	return(rv);
}
#endif
