/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE FILE IO handling
*/

#include <tamtypes.h>
#include <ps2lib_err.h>
#include <kernel.h>
#include <sifrpc.h>
#include <fileio.h>
#include <string.h>

/* TODO: Add versioning support.  */

enum _fio_functions {
	FIO_F_OPEN = 0,
	FIO_F_CLOSE,
	FIO_F_READ,
	FIO_F_WRITE,
	FIO_F_LSEEK,
	FIO_F_IOCTL,
	FIO_F_REMOVE,
	FIO_F_MKDIR,
	FIO_F_RMDIR,
	FIO_F_DOPEN,
	FIO_F_DCLOSE,
	FIO_F_DREAD,
	FIO_F_GETSTAT,
	FIO_F_CHSTAT,
	FIO_F_FORMAT,
	FIO_F_ADDDRV,
	FIO_F_DELDRV
};

/* Shared between _fio_read_intr and fio_read.  The updated modules shipped
   with licensed games changed the size of the buffers from 16 to 64.  */
struct _fio_read_data {
	u32	size1;
	u32	size2;
	void	*dest1;
	void	*dest2;
	u8	buf1[16];
	u8	buf2[16];
};

extern int _iop_reboot_count;
extern SifRpcClientData_t _fio_cd;
extern int _fio_init;
extern int _fio_block_mode;
extern int _fio_completion_sema;
extern int _fio_recv_data[512];
extern int _fio_intr_data[32];

void _fio_read_intr(struct _fio_read_data *);
void _fio_intr();

#ifdef F___fio_internals
SifRpcClientData_t _fio_cd;
int _fio_recv_data[512] __attribute__((aligned(64)));
int _fio_intr_data[32] __attribute__((aligned(64)));
int _fio_init = 0;
int _fio_block_mode;
int _fio_completion_sema;
#endif

#ifdef F_fio_init
int fioInit()
{
	int res;
	ee_sema_t compSema;
	static int _rb_count = 0;
	if(_rb_count != _iop_reboot_count)
	{
	    _rb_count = _iop_reboot_count;
	    _fio_init = 0;
	}

    if (_fio_init)
		return 0;

	SifInitRpc(0);

	while (((res = SifBindRpc(&_fio_cd, 0x80000001, 0)) >= 0) &&
			(_fio_cd.server == NULL))
		nopdelay();

	if (res < 0)
		return res;

	compSema.init_count = 1;
	compSema.max_count = 1;
	compSema.option = 0;
	_fio_completion_sema = CreateSema(&compSema);
	if (_fio_completion_sema < 0)
		return -E_LIB_SEMA_CREATE;

	_fio_init = 1;
	_fio_block_mode = FIO_WAIT;

	return 0;
}
#endif

#ifdef F__fio_intr
void _fio_intr()
{
	iSignalSema(_fio_completion_sema);
}
#endif

#ifdef F_fio_sync
int fioSync(int mode, int *retVal)
{
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	if(_fio_block_mode != FIO_NOWAIT) return -E_LIB_UNSUPPORTED;

	switch(mode)
	{
		case FIO_WAIT:

			WaitSema(_fio_completion_sema);
			SignalSema(_fio_completion_sema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&_fio_recv_data[0]);

			return FIO_COMPLETE;

		case FIO_NOWAIT:

			if(PollSema(_fio_completion_sema) < 0)
				return FIO_INCOMPLETE;

			SignalSema(_fio_completion_sema);

			if(retVal != NULL)
				*retVal = *(int *)UNCACHED_SEG(&_fio_recv_data[0]);

			return FIO_COMPLETE;

		default:
			return -E_LIB_UNSUPPORTED;
	}
}
#endif

#ifdef F_fio_setblockmode
void fioSetBlockMode(int blocking)
{
	_fio_block_mode = blocking;
}

#endif

#ifdef F_fio_exit
void fioExit()
{
	_fio_init = 0;
	memset(&_fio_cd, 0, sizeof _fio_cd);
}
#endif

#ifdef F_fio_open
struct _fio_open_arg {
	int	mode;
	char 	name[FIO_PATH_MAX];
} ALIGNED(16);

int fioOpen(const char *name, int mode)
{
	struct _fio_open_arg arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.mode = mode;
	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	/* TODO: All of these can be cleaned up (get rid of res), and an
	   appropiate error from errno.h be used instead.  */
	if ((res = SifCallRpc(&_fio_cd, FIO_F_OPEN, _fio_block_mode, &arg, sizeof arg,
					_fio_recv_data, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	if(_fio_block_mode == FIO_NOWAIT)
		return 0;
	else
		return _fio_recv_data[0];
}
#endif

#ifdef F_fio_close
int fioClose(int fd)
{
	union { int fd; int result; } arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd = fd;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_CLOSE, 0, &arg, 4, &arg, 4,
					(void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.result;
}
#endif

#ifdef F__fio_read_intr
void _fio_read_intr(struct _fio_read_data *data)
{
	struct _fio_read_data *uncached = UNCACHED_SEG(data);

	if (uncached->size1 && uncached->dest1) {
		memcpy(uncached->dest1, uncached->buf1, uncached->size1);
	}

	if (uncached->size2 && uncached->dest2) {
		memcpy(uncached->dest2, uncached->buf2, uncached->size2);
	}

	iSignalSema(_fio_completion_sema);
}
#endif

#ifdef F_fio_read
struct _fio_read_arg {
	int	fd;
	void	*ptr;
	int	size;
	struct _fio_read_data *read_data;
} ALIGNED(16);

int fioRead(int fd, void *ptr, int size)
{
	struct _fio_read_arg arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd      = fd;
	arg.ptr       = ptr;
	arg.size      = size;
	arg.read_data = (struct _fio_read_data *)_fio_intr_data;

	if (!IS_UNCACHED_SEG(ptr))
		SifWriteBackDCache(ptr, size);
	SifWriteBackDCache(_fio_intr_data, 128);
	SifWriteBackDCache(&arg, sizeof(arg));

	if ((res = SifCallRpc(&_fio_cd, FIO_F_READ, _fio_block_mode, &arg, sizeof arg,
					_fio_recv_data, 4, (void *)_fio_read_intr, _fio_intr_data)) < 0)
		return res;

	if(_fio_block_mode == FIO_NOWAIT)
		return 0;
	else
		return _fio_recv_data[0];
}
#endif

#ifdef F_fio_write
struct _fio_write_arg {
	int	fd;
	void	*ptr;
	u32	size;
	u32	mis;
	u8	aligned[16];
} ALIGNED(16);

int fioWrite(int fd, const void *ptr, int size)
{
	struct _fio_write_arg arg;
	int mis, res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd = fd;
	arg.ptr  = ptr;
	arg.size = size;

	/* Copy the unaligned (16-byte) portion into the argument */
	mis = 0;
	if ((u32)ptr & 0xf) {
		mis = 16 - ((u32)ptr & 0xf);
		if (mis > size)
			mis = size;
	}
	arg.mis = mis;


	if (mis)
		memcpy(arg.aligned, ptr, mis);

	if (!IS_UNCACHED_SEG(ptr))
		SifWriteBackDCache(ptr, size);

	if ((res = SifCallRpc(&_fio_cd, FIO_F_WRITE, _fio_block_mode, &arg, sizeof arg,
					_fio_recv_data, 4, (void *)_fio_intr, NULL)) < 0)
		return res;


	if(_fio_block_mode == FIO_NOWAIT)
		return 0;
	else
		return _fio_recv_data[0];
}
#endif

#ifdef F_fio_lseek
struct _fio_lseek_arg {
	union {
		int	fd;
		int	result;
	} p;
	int	offset;
	int	whence;
} ALIGNED(16);

int fioLseek(int fd, int offset, int whence)
{
	struct _fio_lseek_arg arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.fd   = fd;
	arg.offset = offset;
	arg.whence = whence;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_LSEEK, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.p.result;
}
#endif

#ifdef F_fio_ioctl
struct _fio_ioctl_arg {
	union {
		int fd;
		int result;
	} p;
	int request;
	u8 data[1024];		// Will this be ok ?
} ALIGNED(16);

int fioIoctl(int fd, int request, void *data)
{
	struct _fio_ioctl_arg arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.fd = fd;
	arg.request = request;
	memcpy(arg.data, data, 1024);

	if ((res = SifCallRpc(&_fio_cd, FIO_F_IOCTL, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.p.result;
}
#endif

#ifdef F_fio_remove
int fioRemove(const char *name)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, name, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_REMOVE, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.result;
}
#endif

#ifdef F_fio_mkdir
int fioMkdir(const char* path)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res;

 	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, path, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_MKDIR, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.result;
}
#endif

#ifdef F_fio_rmdir
int fioRmdir(const char* dirname)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, dirname, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_RMDIR, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.result;
}
#endif

#ifdef F_fio_putc
int fioPutc(int fd,int c)
{
	return fioWrite(fd,&c,1);
}
#endif

#ifdef F_fio_getc
int fioGetc(int fd)
{
	int c;

	fioRead(fd,&c,1);
	return c;
}
#endif

#ifdef F_fio_gets
int fioGets(int fd, char* buff, int n)
{
	// Rather than doing a slow byte-at-a-time read
	// read upto the max langth, then re-position file afterwards
	int read;
	int i;

	read = fioRead(fd, buff, n);

    for (i=0; i<(read-1); i++)
	{
		switch (buff[i])
		{
		case '\n':
			fioLseek(fd, (i + 1) - read, SEEK_CUR);
			buff[i]=0;	// terminate after newline
			return i;

		case 0:
			fioLseek(fd, i-read, SEEK_CUR);
            return i;
		}
	}

	// if we reached here, then we havent found the end of a string
	return i;
}
#endif

#ifdef F_fio_dopen
int fioDopen(const char *name)
{
	union {
		char name[FIO_PATH_MAX];
		int	result;
	} arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_DOPEN, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.result;
}
#endif

#ifdef F_fio_dclose
int fioDclose(int fd)
{
	union {
		int fd;
		int result;
	} arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.fd = fd;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_DCLOSE, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.result;
}
#endif

#ifdef F_fio_dread
struct _fio_dread_arg {
	union {
		int fd;
		int result;
	} p;
	fio_dirent_t *buf;
} ALIGNED(16);

int fioDread(int fd, fio_dirent_t *buf)
{
	struct _fio_dread_arg arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.fd = fd;
	arg.buf = buf;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, sizeof(fio_dirent_t));

	if ((res = SifCallRpc(&_fio_cd, FIO_F_DREAD, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.p.result;
}
#endif

#ifdef F_fio_getstat
struct _fio_getstat_arg {
	union {
		fio_stat_t *buf;
		int result;
	} p;
	char name[FIO_PATH_MAX];
} ALIGNED(16);

int fioGetstat(const char *name, fio_stat_t *buf)
{
	struct _fio_getstat_arg arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.buf = buf;
	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, sizeof(fio_stat_t));

	if ((res = SifCallRpc(&_fio_cd, FIO_F_GETSTAT, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.p.result;
}
#endif

#ifdef F_fio_chstat
struct _fio_chstat_arg {
	union {
		int cbit;
		int result;
	} p;
	fio_stat_t stat;
	char name[FIO_PATH_MAX];
};

int fioChstat(const char *name, fio_stat_t *buf, u32 cbit)
{
	struct _fio_chstat_arg arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	arg.p.cbit = cbit;
	memcpy(&arg.stat, buf, sizeof(fio_stat_t));
	strncpy(arg.name, name, FIO_PATH_MAX - 1);
	arg.name[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_CHSTAT, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.p.result;
}
#endif

#ifdef F_fio_format
int fioFormat(const char *name)
{
	union {
		char path[FIO_PATH_MAX];
		int	result;
	} arg;
	int res;

	if (!_fio_init && ((res = fioInit()) < 0))
		return res;

	WaitSema(_fio_completion_sema);

	strncpy(arg.path, name, FIO_PATH_MAX - 1);
	arg.path[FIO_PATH_MAX - 1] = 0;

	if ((res = SifCallRpc(&_fio_cd, FIO_F_FORMAT, 0, &arg, sizeof arg,
					&arg, 4, (void *)_fio_intr, NULL)) < 0)
		return res;

	return arg.result;
}
#endif
