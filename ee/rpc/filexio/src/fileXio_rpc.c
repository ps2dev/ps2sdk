/*
 * fileXioiop.c - fileXio RPC client 
 *
 * (c) 2003 adresd <adresd_ps2dev@yahoo.com>
 * (c) 2003 Nicholas Van Veen <nickvv@xtra.co.nz>
 *
 * This module provides an RPC interface on the EE to the functions 
 * of ioman/fileio.
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdarg.h>
#include <string.h>
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "fileXio_rpc.h"

typedef struct {
	int  ssize;
	int  esize;
	void *sbuf;
	void *ebuf;
	u8 sbuffer[16];
	u8 ebuffer[16];
} rests_pkt; // sizeof = 48

static SifRpcClientData_t cd0;
static unsigned sbuff[0x1300] __attribute__((aligned (64)));
static int _intr_data[0xC00] __attribute__((aligned(64)));
static int fileXioInited = 0;
static int fileXioBlockMode;
static int fileXioCompletionSema;

void _fxio_intr()
{
	iSignalSema(fileXioCompletionSema);
}

int fileXioInit()
{
	int res;
	ee_sema_t compSema;

	if(fileXioInited)
		return 0;

	while(((res = SifBindRpc(&cd0, FILEXIO_IRX, 0)) >= 0) && (cd0.server == NULL))
		nopdelay();

	if(res < 0)
		return res;

	compSema.init_count = 1;
	compSema.max_count = 1;
	compSema.option = 0;
	fileXioCompletionSema = CreateSema(&compSema);
	if (fileXioCompletionSema < 0)
		return -1;

	fileXioInited = 1;
	fileXioBlockMode = FXIO_WAIT;

	return 0;
}


void fileXioStop()
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return;

	SifCallRpc(&cd0, FILEXIO_STOP, 0, sbuff, 0, sbuff, 0, 0, 0);

	return;
}

int fileXioGetdir(const char* pathname, struct fileXioDirEntry dirEntry[], unsigned int req_entries)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	// copy the requested pathname to the rpc buffer
	strncpy((char*)sbuff, pathname, 512);

	SifWriteBackDCache(dirEntry, (sizeof(struct fileXioDirEntry) * req_entries));

	sbuff[512/4] = (int)dirEntry;
	sbuff[516/4] = req_entries;

	// This will get the directory contents, and fill dirEntry via DMA
	SifCallRpc(&cd0, FILEXIO_GETDIR, fileXioBlockMode, sbuff, 512+4+4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioMount(const char* mountpoint, const char* mountstring, int flag)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, mountstring, 512);
	strncpy((char*)&sbuff[512/4], mountpoint, 512);
	sbuff[1024/4] = (int)flag;

	SifCallRpc(&cd0, FILEXIO_MOUNT, fileXioBlockMode, sbuff, 1024+4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioUmount(const char* mountpoint)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, mountpoint, 512);

	SifCallRpc(&cd0, FILEXIO_UMOUNT, fileXioBlockMode, sbuff, 512, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioCopyfile(const char* source, const char* dest, int mode)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, source, 512);
	strncpy((char*)&sbuff[512/4], dest, 512);
	sbuff[1024/4] = (int)mode;

	SifCallRpc(&cd0, FILEXIO_COPYFILE, fileXioBlockMode, sbuff, 1024+4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioMkdir(const char* pathname, int mode)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, pathname, 512);
	sbuff[512/4] = mode;

	SifCallRpc(&cd0, FILEXIO_MKDIR, fileXioBlockMode, sbuff, 516, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioRmdir(const char* pathname)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, pathname, 512);

	SifCallRpc(&cd0, FILEXIO_RMDIR, fileXioBlockMode, sbuff, 512, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioRemove(const char* pathname)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, pathname, 512);

	SifCallRpc(&cd0, FILEXIO_REMOVE, fileXioBlockMode, sbuff, 512, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioRename(const char* source,const char* dest)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, source, 512);
	strncpy((char*)&sbuff[512/4], dest, 512);

	SifCallRpc(&cd0, FILEXIO_RENAME, fileXioBlockMode, sbuff, 1024, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioSymlink(const char* source,const char* dest)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, source, 512);
	strncpy((char*)&sbuff[512/4], dest, 512);

	SifCallRpc(&cd0, FILEXIO_SYMLINK, fileXioBlockMode, sbuff, 1024, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioReadlink(const char* source, char* buf, int buflen)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	if( !IS_UNCACHED_SEG(buf))
  	  SifWriteBackDCache(buf, buflen);

	strncpy((char*)sbuff, source, 512);
	sbuff[512/4] = (int)buf;
	sbuff[516/4] = buflen;

	SifCallRpc(&cd0, FILEXIO_READLINK, fileXioBlockMode, sbuff, 512+4+4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioChdir(const char* pathname)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, pathname, 512);

	SifCallRpc(&cd0, FILEXIO_CHDIR, fileXioBlockMode, sbuff, 512, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioOpen(const char* source, int flags, int modes)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, source, 512);
	sbuff[512/4] = flags;
	sbuff[516/4] = modes;

	SifCallRpc(&cd0, FILEXIO_OPEN, fileXioBlockMode, sbuff, 512+4+4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioClose(int fd)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	sbuff[0/4] = fd;

	SifCallRpc(&cd0, FILEXIO_CLOSE, fileXioBlockMode, sbuff, 4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

static void recv_intr(void *data_raw)
{
	rests_pkt *rests = UNCACHED_SEG(data_raw);
	int i;
      char *tmp;

	tmp = (char*)rests->sbuf;
	if(rests->ssize)
		for(i = 0; i < rests->ssize; i++)
			tmp[i] = rests->sbuffer[i];

	tmp = (char*)rests->ebuf;
	if(rests->esize)
		for(i = 0; i < rests->esize; i++)
			tmp[i] = rests->ebuffer[i];

	iSignalSema(fileXioCompletionSema);
}

int fileXioRead(int fd, unsigned char *buf, int size)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	sbuff[0/4] = fd;
	sbuff[4/4] = (int)buf;
	sbuff[8/4] = size;
    sbuff[12/4] = (int)_intr_data;

	if (!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, size);
	SifWriteBackDCache(_intr_data, 128);
	SifWriteBackDCache(sbuff, 16);

	SifCallRpc(&cd0, FILEXIO_READ, fileXioBlockMode, sbuff, 16, sbuff, 4, recv_intr, _intr_data);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioWrite(int fd, unsigned char *buf, int size)
{
	unsigned int miss;

	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	if((u32)buf & 0xf)
	{
		miss = 16 - ((u32)buf & 0xf);
		if(miss > size) miss = size;
	} else {
		miss = 0;
	}

	sbuff[0/4] = fd;
	sbuff[4/4] = (int)buf;
	sbuff[8/4] = size;
	sbuff[12/4] = miss;

	memcpy((void *)&sbuff[16/4], UNCACHED_SEG(buf), miss);

	if(!IS_UNCACHED_SEG(buf))
		SifWriteBackDCache(buf, size);

	SifCallRpc(&cd0, FILEXIO_WRITE, fileXioBlockMode, sbuff, 32,sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioLseek(int fd,long offset,int whence)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	sbuff[0/4] = fd;
	sbuff[4/4] = offset;
	sbuff[8/4] = whence;

	SifCallRpc(&cd0, FILEXIO_LSEEK, fileXioBlockMode, sbuff, 12, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

//
// NOTE: needs to be fixed! 64-bit
//
int fileXioLseek64(int fd, long long offset, int whence)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	sbuff[0/4] = fd;
	sbuff[4/4] = offset;
	sbuff[8/4] = whence;

	SifCallRpc(&cd0, FILEXIO_LSEEK64, fileXioBlockMode, sbuff, 12, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioChStat(const char *name, iox_stat_t *stat, int mask)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char *)sbuff, name, 512);
	sbuff[512/4] = (int)stat;
	sbuff[516/4] = mask;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(iox_stat_t));

	SifCallRpc(&cd0, FILEXIO_CHSTAT, fileXioBlockMode, sbuff, 12, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioGetStat(const char *name, iox_stat_t *stat)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char *)sbuff, name, 512);
	sbuff[512/4] = (int)stat;

	if(!IS_UNCACHED_SEG(stat))
		SifWriteBackDCache(stat, sizeof(iox_stat_t));

	SifCallRpc(&cd0, FILEXIO_GETSTAT, fileXioBlockMode, sbuff, 516, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioFormat(const char *dev, const char *blockdev, const char *args, int arglen)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff,dev,128);
	if(blockdev)
		strncpy((char*)&sbuff[128/4],blockdev,512);

	if(arglen > 512) arglen = 512;
	memcpy(&sbuff[640/4], args, arglen);
	sbuff[(1152)/4] = (int)arglen;

	SifCallRpc(&cd0, FILEXIO_FORMAT, fileXioBlockMode,  sbuff, 1024+128+4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioSync(const char *devname, int flag)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, devname, 512);
	sbuff[512/4] = flag;

	SifCallRpc(&cd0, FILEXIO_SYNC, fileXioBlockMode, sbuff, 516, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioDopen(const char *name)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	strncpy((char*)sbuff, name, 512);
	SifCallRpc(&cd0, FILEXIO_DOPEN, fileXioBlockMode, sbuff, 512, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioDclose(int fd)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	sbuff[0/4] = fd;
	SifCallRpc(&cd0, FILEXIO_DCLOSE, fileXioBlockMode, sbuff, 4, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioDread(int fd, iox_dirent_t *dirent)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

	WaitSema(fileXioCompletionSema);

	sbuff[0/4] = fd;
	sbuff[4/4] = (int)dirent;

	if (!IS_UNCACHED_SEG(dirent))
		SifWriteBackDCache(dirent, sizeof(iox_dirent_t));

	SifCallRpc(&cd0, FILEXIO_DREAD, fileXioBlockMode, sbuff, 8, sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

static void fxio_ctl_intr(void *data_raw)
{
	struct fxio_ctl_return_pkt *pkt = UNCACHED_SEG(data_raw);
	int i;
	u8 *dest = pkt->dest;

	for(i = 0; i < pkt->len; i++)
		dest[i] = pkt->buf[i];

	iSignalSema(fileXioCompletionSema);
}

int fileXioDevctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct devctl_packet *packet = (struct devctl_packet *)sbuff;

	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

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

	SifWriteBackDCache(_intr_data, sizeof(struct fxio_ctl_return_pkt));
	SifWriteBackDCache(buf, buflen);

	if(buflen)
		SifCallRpc(&cd0, FILEXIO_DEVCTL, fileXioBlockMode, packet, sizeof(struct devctl_packet), sbuff, 4, fxio_ctl_intr, _intr_data);
	else
		SifCallRpc(&cd0, FILEXIO_DEVCTL, fileXioBlockMode, packet, sizeof(struct devctl_packet), sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioIoctl2(int fd, int command, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	struct ioctl2_packet *packet = (struct ioctl2_packet *)sbuff;

	if(!fileXioInited && ((fileXioInit()) < 0))
		return -ENOPKG;

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

	SifWriteBackDCache(_intr_data, sizeof(struct fxio_ctl_return_pkt));
	SifWriteBackDCache(buf, buflen);

	if(buflen)
		SifCallRpc(&cd0, FILEXIO_IOCTL2, fileXioBlockMode, packet, sizeof(struct devctl_packet), sbuff, 4, fxio_ctl_intr, _intr_data);
	else
		SifCallRpc(&cd0, FILEXIO_IOCTL2, fileXioBlockMode, packet, sizeof(struct devctl_packet), sbuff, 4, (void *)_fxio_intr, 0);

	if(fileXioBlockMode == FXIO_NOWAIT)
		return 0;
	else
		return sbuff[0];
}

int fileXioWaitAsync(int mode, int *retVal)
{
	if(!fileXioInited && ((fileXioInit()) < 0))
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
