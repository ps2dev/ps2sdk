/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# fileXio RPC Server 
# This module provides an RPC interface to the EE for all the functions 
# of ioman/fileio.
*/

//#define DEBUG


#include "types.h"
#include "stdio.h"
#include "sysclib.h"
#include "thbase.h"
#include "intrman.h"
#include "sysmem.h"
#include "sifman.h"
#include "sifcmd.h"

#include "iomanX.h"
#include "fileXio_iop.h"

#define MODNAME "IOX/File_Manager_Rpc"
IRX_ID(MODNAME, 1, 1);

#define TRUE	1
#define FALSE	0

#define MIN(a, b)	(((a)<(b))?(a):(b))
#define RDOWN_16(a)	(((a) >> 4) << 4)

#define RWSIZE	16384
static void *rwbuf = NULL;
static unsigned int *buffer; // RPC send/receive buffer
struct t_SifRpcDataQueue qd;
struct t_SifRpcServerData sd0;

typedef struct {
	int  ssize;
	int  esize;
	void *sbuf;
	void *ebuf;
	u8 sbuffer[16];
	u8 ebuffer[16];
} rests_pkt; // sizeof = 48
static rests_pkt rests __attribute((aligned(64)));

/* RPC exported functions */
int fileXio_stop(void);
int fileXio_GetDir_RPC(const char* pathname, struct fileXioDirEntry dirEntry[], unsigned int req_entries);


// Functions called by the RPC server
void* fileXioRpc_Stop();
void* fileXioRpc_Getdir(unsigned int* sbuff);


void* fileXio_rpc_server(int fno, void *data, int size);
void fileXio_Thread(void* param);

void DirEntryCopy(struct fileXioDirEntry* dirEntry, iox_dirent_t* internalDirEntry);

int _start( int argc, char **argv)
{
	struct _iop_thread param;
	int th;

	param.attr         = 0x02000000;
	param.thread       = (void*)fileXio_Thread;
	param.priority 	  = 40;
	param.stacksize    = 0x8000;
	param.option      = 0;

	th = CreateThread(&param);

	if (th > 0)
	{
		StartThread(th,0);
		return 0;
	}
	else
		return 1;

	return 0;
}

int fileXio_CopyFile_RPC(const char *src, const char *dest, int mode)
{
  iox_stat_t stat;
  int infd, outfd, size, remain, i, retval = 0;

  if ((infd = open(src, O_RDONLY, 0666)) < 0) {
    return infd;
  }
  if ((outfd = open(dest, O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0) {
    close(infd);
    return outfd;
  }
  size = lseek(infd, 0, SEEK_END);
  lseek(infd, 0, SEEK_SET);
  if (!size)
    return retval;

  remain = size % RWSIZE;
  for (i = 0; i < (size / RWSIZE); i++) {
    read(infd, rwbuf, RWSIZE);
    write(outfd, rwbuf, RWSIZE);
  }
  read(infd, rwbuf, remain);
  write(outfd, rwbuf, remain);
  close(infd);
  close(outfd);

  stat.mode = mode;
  chstat(dest, &stat, 1);

  return size;
}
int fileXio_Read_RPC(int infd, char *read_buf, int read_size, void *intr_data)
{
     int srest;			
     int erest;			
     int asize;			
     int abuffer;			
     int rlen;				
     int total;			
     int readlen;			
     int status;			
     struct t_SifDmaTransfer dmaStruct;
     void *buffer;
     void *aebuffer;				
     int intStatus;	// interrupt status - for dis/en-abling interrupts
     int read_buf2 = (int)read_buf;

	total = 0;

	if(read_size < 16)
	{
		srest = read_size;
		erest = asize = abuffer = 0;
		buffer = read_buf;
		aebuffer = 0;
	}else{
		if ((read_buf2 & 0xF) == 0)	
			srest=0;						
		else				
			srest=((read_buf2 >> 4) << 4) - read_buf2 + 16;
		buffer = read_buf;
		abuffer = read_buf2 + srest;			
		aebuffer=(void *)(((read_buf2 + read_size) >> 4) << 4);	
		asize = (int)aebuffer - (int)abuffer;
		erest = (read_buf2 + read_size) - (int)aebuffer;			
	}
	if (srest>0)
	{
		if (srest!=(rlen=read(infd, rests.sbuffer, srest)))
		{
			total += srest = (rlen>0 ? rlen:0);
			goto EXIT;
		}
		else
			total += srest;
	}

	status=0;
	while (asize>0)
	{
		readlen=MIN(RWSIZE, asize);

		while(SifDmaStat(status)>=0);		

		rlen=read(infd, rwbuf, readlen);		
		if (readlen!=rlen){
			if (rlen<=0)goto EXIT;
			dmaStruct.dest=(void *)abuffer;
			dmaStruct.size=rlen;
			dmaStruct.attr=0;
			dmaStruct.src =rwbuf;
			CpuSuspendIntr(&intStatus);			
			SifSetDma(&dmaStruct, 1);			
			CpuResumeIntr(intStatus);			
			total	+=rlen;
			goto EXIT;
		}else{			//read ok
			total   += rlen;
			asize   -=rlen;
			dmaStruct.dest=(void *)abuffer;
			abuffer +=rlen;
			dmaStruct.size=rlen;
			dmaStruct.attr=0;
			dmaStruct.src =rwbuf;
			CpuSuspendIntr(&intStatus);			
			status=SifSetDma(&dmaStruct, 1);		
			CpuResumeIntr(intStatus);
		}
	}
	if (erest>0)
	{
		rlen = read(infd, rests.ebuffer, erest);	
		total += (rlen>0 ? rlen : 0);
	}
EXIT:
	rests.ssize=srest;           
	rests.esize=erest;           
	rests.sbuf =buffer;          
	rests.ebuf =aebuffer;        
      dmaStruct.src =&rests;
	dmaStruct.size=sizeof(rests_pkt);
	dmaStruct.attr=0;
	dmaStruct.dest=intr_data;
	CpuSuspendIntr(&intStatus);					
	SifSetDma(&dmaStruct, 1);					
	CpuResumeIntr(intStatus);					
	return (total);
}

int fileXio_Write_RPC(int outfd, const char *write_buf, int write_size, int mis,char *misbuf)
{
     SifRpcReceiveData_t rdata;
     int left;			
     int wlen;			
     int writelen;		
     int pos;
     int total;

	left  = write_size;
	total = 0;
	if (mis > 0)
      {           
		wlen=write(outfd, misbuf, mis);	
		if (wlen != mis)
            {
			if (wlen > 0)
				total += wlen;
			return (total);
		}
		total += wlen;
	}

	left-=mis;
	pos=(int)write_buf+mis;
	while(left){
		writelen = MIN(RWSIZE, left);
		SifRpcGetOtherData(&rdata, (void *)pos, rwbuf, writelen, 0);
		wlen=write(outfd, rwbuf, writelen);	
		if (wlen != writelen){
			if (wlen>0)
				total+=wlen;
			return (total);
		}
		left -=writelen;
		pos  +=writelen;
		total+=writelen;
	}
	return (total);
}


// This is the getdir for use by the EE RPC client
// It DMA's entries to the specified buffer in EE memory
int fileXio_GetDir_RPC(const char* pathname, struct fileXioDirEntry dirEntry[], unsigned int req_entries)
{
	int matched_entries;
      int fd, res;
  	iox_dirent_t dirbuf;
	struct fileXioDirEntry localDirEntry;
	int intStatus;	// interrupt status - for dis/en-abling interrupts
	struct t_SifDmaTransfer dmaStruct;
	int dmaID;

	dmaID = 0;

	#ifdef DEBUG
		printf("RPC GetDir Request\n");
		printf("dirname: %s\n",pathname);
	#endif

	matched_entries = 0;

      fd = dopen(pathname);
      if (fd <= 0) 
      {
        return fd;
      }
	{

        res = 1;
        while (res > 0)
        {
          memset(&dirbuf, 0, sizeof(dirbuf));
          res = dread(fd, &dirbuf);
          if (res > 0)
          {
		// check for too many entries
		if (matched_entries == req_entries)
		{
			close(fd);
			return (matched_entries);
		}
		// wait for any previous DMA to complete
	      // before over-writing localDirEntry
	      while(SifDmaStat(dmaID)>=0);
            DirEntryCopy(&localDirEntry, &dirbuf);
	      // DMA localDirEntry to the address specified by dirEntry[matched_entries]	
	      // setup the dma struct
	      dmaStruct.src = &localDirEntry;
	      dmaStruct.dest = &dirEntry[matched_entries];
	      dmaStruct.size = sizeof(struct fileXioDirEntry);
	      dmaStruct.attr = 0;
	      // Do the DMA transfer
	      CpuSuspendIntr(&intStatus);
	      dmaID = SifSetDma(&dmaStruct, 1);
	      CpuResumeIntr(intStatus);
  	      matched_entries++;
          } // if res
        } // while

      } // if dirs and files

      // cleanup and return # of entries
      close(fd);
	return (matched_entries);
}

// This is the mount for use by the EE RPC client
int fileXio_Mount_RPC(const char* mountstring, const char* mountpoint, int flag)
{
	int res=0;
	#ifdef DEBUG
		printf("RPC Mount Request\n");
		printf("mountpoint: %s - %s - %d\n",mountpoint,mountstring,flag);
	#endif
	res = mount(mountpoint, mountstring, flag, NULL, 0);
	return(res);
}

int fileXio_chstat_RPC(char *filename, void* eeptr, int mask)
{
      int res=0;
	iox_stat_t localStat;
      SifRpcReceiveData_t rdata;

	SifRpcGetOtherData(&rdata, (void *)eeptr, &localStat, 64, 0);

	res = chstat(filename, &localStat, mask);
      return(res);
}

int fileXio_getstat_RPC(char *filename, void* eeptr)
{
	iox_stat_t localStat;
	int res = 0;
	struct t_SifDmaTransfer dmaStruct;
	int intStatus;	// interrupt status - for dis/en-abling interrupts

	res = getstat(filename, &localStat);

	if(res >= 0)
	{
		// DMA localStat to the address specified by eeptr	
		// setup the dma struct
		dmaStruct.src = &localStat;
		dmaStruct.dest = eeptr;
		dmaStruct.size = sizeof(iox_stat_t);
		dmaStruct.attr = 0;
		// Do the DMA transfer
		CpuSuspendIntr(&intStatus);
		SifSetDma(&dmaStruct, 1);
		CpuResumeIntr(intStatus);
	}

	return(res);
}

int fileXio_dread_RPC(int fd, void* eeptr)
{
      int res=0;
	iox_dirent_t localDir;
      struct t_SifDmaTransfer dmaStruct;
      int intStatus;	// interrupt status - for dis/en-abling interrupts

	res = dread(fd, &localDir);
      if (res > 0)
      {
	  // DMA localStat to the address specified by eeptr	
	  // setup the dma struct
	  dmaStruct.src = &localDir;
	  dmaStruct.dest = eeptr;
	  dmaStruct.size = 64+256;
	  dmaStruct.attr = 0;
	  // Do the DMA transfer
	  CpuSuspendIntr(&intStatus);
	  SifSetDma(&dmaStruct, 1);
	  CpuResumeIntr(intStatus);
      }

      return(res);
}

void* fileXioRpc_Stop()
{
	#ifdef DEBUG
		printf("RPC Stop Request\n");
	#endif
	return NULL;
}

// Send:   Offset 0 = filename string (512 bytes)
// Send:   Offset 512 = pointer to array of DirEntry structures in EE mem
// Send:   Offset 516 = requested number of entries
// Return: Offset 0 = ret val (number of matched entries). Size = int
void* fileXioRpc_Getdir(unsigned int* sbuff)
{
	int ret;

	ret=fileXio_GetDir_RPC(
		(char*)&sbuff[0/4],	
		(struct fileXioDirEntry*) sbuff[512/4],	
		sbuff[516/4]		
		);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = mount format string (512 bytes)
// Send:   Offset 512 = mount point (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_Mount(unsigned int* sbuff)
{
	int ret;

	#ifdef DEBUG
		printf("RPC Mount Request\n");
	#endif
	ret=fileXio_Mount_RPC(
		(char*)&sbuff[0/4],		// mount format string
		(char*)&sbuff[512/4],		// mount point
		sbuff[1024/4]			// flag (normal,readonly,robust)
		);

	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = mount format string (512 bytes)
// Send:   Offset 512 = mount point (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_uMount(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC uMount Request\n");
	#endif
	ret=umount((char*)&sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = source filename (512 bytes)
// Send:   Offset 512 = destination filename (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_CopyFile(unsigned int* sbuff)
{
	int ret;

	ret=fileXio_CopyFile_RPC(
		(char*)&sbuff[0/4],		// source filename
		(char*)&sbuff[512/4],		// dest filename
		sbuff[1024/4]		// mode
		);

	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = filenames (512 bytes)
// Send:   Offset 512 = mode (int)
// Return: Offset 0 = return status (int)
void* fileXioRpc_MkDir(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC MkDir Request\n");
	#endif
	ret=mkdir((char*)&sbuff[0/4],sbuff[512/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = filenames (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_RmDir(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC RmDir Request\n");
	#endif
	ret=rmdir((char*)&sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = filenames (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_Remove(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Remove Request\n");
	#endif
	ret=remove((char*)&sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = source filename (512 bytes)
// Send:   Offset 512 = destination filename (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_Rename(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Rename Request\n");
	#endif
	ret=rename((char*)&sbuff[0/4],(char*)&sbuff[512/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = source filename (512 bytes)
// Send:   Offset 512 = destination filename (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_SymLink(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC SymLink Request\n");
	#endif
	ret=symlink((char*)&sbuff[0/4],(char*)&sbuff[512/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = source filename (512 bytes)
// Send:   Offset 512 = buffer (512 bytes)
// Send:   Offset 1024 = buflen (int)
// Return: Offset 0 = return status (int)
void* fileXioRpc_ReadLink(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC ReadLink Request\n");
	#endif
	ret=readlink((char*)&sbuff[0/4],(char*)&sbuff[512/4],sbuff[1024/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = filename (512 bytes)
// Return: Offset 0 = return status (int)
void* fileXioRpc_ChDir(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC ChDir Request\n");
	#endif
	ret=chdir((char*)&sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = source filename (512 bytes)
// Send:   Offset 512 = flags (int)
// Send:   Offset 516 = modes (int)
// Return: Offset 0 = return status (int) / fd
void* fileXioRpc_Open(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Open Request\n");
	#endif
	ret=open((char*)&sbuff[0/4],sbuff[512/4],sbuff[516/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = fd (int)
// Return: Offset 0 = return status (int)
void* fileXioRpc_Close(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Close Request\n");
	#endif
	ret=close(sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = fd (int)
// Send:   Offset 4 = pointer to buffer in EE mem
// Send:   Offset 8 = buffer size (int)
// Send:   Offset 12 = pointer to intr_data in EE mem
void* fileXioRpc_Read(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Read Request\n");
	#endif
	ret=fileXio_Read_RPC(sbuff[0/4],(void *)sbuff[4/4],sbuff[8/4],(void *)sbuff[12/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = fd (int)
// Send:   Offset 4 = pointer to buffer in EE mem
// Send:   Offset 8 = buffer size (int)
// Send:   Offset 12 = misaligned buffer size (int)
// Send:   Offset 16 = misaligned buffer (16)
void* fileXioRpc_Write(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Write Request\n");
	#endif
	ret=fileXio_Write_RPC(sbuff[0/4],(void *)sbuff[4/4],sbuff[8/4],
                            sbuff[12/4],(char *)&sbuff[16/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = fd (int)
// Send:   Offset 4 = offset (long)
// Send:   Offset 8 = whence (int)
// Return: Offset 0 = return status (int)
void* fileXioRpc_Lseek(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Lseek Request\n");
	#endif
	ret=lseek(sbuff[0/4],(long)sbuff[4/4],sbuff[8/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = fd (int)
// Send:   Offset 4 = offset (long)
// Send:   Offset 12 = whence (int)
// Return: Offset 0 = return status (int)
void* fileXioRpc_Lseek64(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Lseek64 Request\n");
	#endif
	ret=lseek64(sbuff[0/4],(long)sbuff[4/4],sbuff[12/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = filename (int)
// Send:   Offset 512 = pointer to EE mem
// Send:   Offset 516 = mask (int)
// Return: Offset 0 = return status (int)
void* fileXioRpc_ChStat(unsigned int* sbuff)
{
	int ret=-1;
	#ifdef DEBUG
		printf("RPC ChStat Request\n");
	#endif
      ret=fileXio_chstat_RPC((char *)&sbuff[0/4],(void *)sbuff[512/4],sbuff[516/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = filename 
// Send:   Offset 512 = pointer to EE mem
// Return: Offset 0 = return status (int)
void* fileXioRpc_GetStat(unsigned int* sbuff)
{
	int ret=-1;
	#ifdef DEBUG
		printf("RPC GetStat Request\n");
	#endif
	ret=fileXio_getstat_RPC((char *)&sbuff[0/4],(void *)sbuff[512/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = device
// Send:   Offset 128 = blockdevice
// Send:   Offset 640 = args
// Send:   Offset 1152 = arglen (int)
// Return: Offset 0 = return status 
void* fileXioRpc_Format(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Format Request\n");
	#endif
	ret=format((char *)&sbuff[0/4],(char *)&sbuff[128/4],(char *)&sbuff[640/4],sbuff[1152/4]);
	sbuff[0] = ret;
	return sbuff;
}

//int io_AddDrv(iop_device_t *device);
void* fileXioRpc_AddDrv(unsigned int* sbuff)
{
	int ret=-1;
	#ifdef DEBUG
		printf("RPC AddDrv Request\n");
	#endif
	//	BODY
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = fsname 
// Return: Offset 0 = return status (int)
void* fileXioRpc_DelDrv(unsigned int* sbuff)
{
	int ret=-1;
	#ifdef DEBUG
		printf("RPC DelDrv Request\n");
	#endif
//	ret=deldrv((char *)&sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

// Send:   Offset 0 = devname 
// Send:   Offset 512 = flag (int)
// Return: Offset 0 = return status (int)
void* fileXioRpc_Sync(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Sync Request\n");
	#endif
	ret=sync((char *)&sbuff[0/4],sbuff[512/4]);
	sbuff[0] = ret;
	return sbuff;
}

//int io_devctl(const char *name, int cmd, void *arg, unsigned int arglen, void *bufp,
//		unsigned int buflen);
void* fileXioRpc_Devctl(unsigned int* sbuff)
{
	struct devctl_packet *packet = (struct devctl_packet *)sbuff;
	struct fxio_ctl_return_pkt *ret_buf = (struct fxio_ctl_return_pkt *)rwbuf;
	SifDmaTransfer_t dmatrans;
	int intStatus;
	int ret;

	#ifdef DEBUG
		printf("RPC Devctl Request\n");
	#endif

	ret = devctl(packet->name, packet->cmd, packet->arg, packet->arglen, ret_buf->buf, packet->buflen);

	// Transfer buffer back to EE
	if(packet->buflen != 0) 
	{
		dmatrans.src = ret_buf;
		dmatrans.dest = packet->intr_data;
		dmatrans.attr = 0;
		dmatrans.size = sizeof(struct fxio_ctl_return_pkt);

		ret_buf->dest = packet->buf;

		// EE is expecting data.. on error, simply use size of 0 so no data is copied.
		if(ret >= 0)
			ret_buf->len = packet->buflen;
		else
			ret_buf->len = 0;

		CpuSuspendIntr(&intStatus);			
		SifSetDma(&dmatrans, 1);			
		CpuResumeIntr(intStatus);	
	}
	
	sbuff[0] = ret;
	return sbuff;
}

//int io_ioctl(int fd, int cmd, void *arg);
void* fileXioRpc_Ioctl(unsigned int* sbuff)
{
	int ret=-1;
	#ifdef DEBUG
		printf("RPC Ioctl Request\n");
	#endif
	//	BODY
	sbuff[0] = ret;
	return sbuff;
}
//int io_ioctl2(int fd, int cmd, void *arg, unsigned int arglen, void *bufp,
//		unsigned int buflen);
void* fileXioRpc_Ioctl2(unsigned int* sbuff)
{
	struct ioctl2_packet *packet = (struct ioctl2_packet *)sbuff;
	struct fxio_ctl_return_pkt *ret_buf = (struct fxio_ctl_return_pkt *)rwbuf;
	SifDmaTransfer_t dmatrans;
	int intStatus;
	int ret;

	#ifdef DEBUG
		printf("RPC ioctl2 Request\n");
	#endif

	ret = ioctl2(packet->fd, packet->cmd, packet->arg, packet->arglen, ret_buf->buf, packet->buflen);

	// Transfer buffer back to EE
	if(packet->buflen != 0) 
	{
		dmatrans.src = ret_buf;
		dmatrans.dest = packet->intr_data;
		dmatrans.attr = 0;
		dmatrans.size = sizeof(struct fxio_ctl_return_pkt);

		ret_buf->dest = packet->buf;

		// EE is expecting data.. on error, simply use size of 0 so no data is copied.
		if(ret >= 0)
			ret_buf->len = packet->buflen;
		else
			ret_buf->len = 0;

		CpuSuspendIntr(&intStatus);			
		SifSetDma(&dmatrans, 1);			
		CpuResumeIntr(intStatus);	
	}
	
	sbuff[0] = ret;
	return sbuff;
}

void* fileXioRpc_Dopen(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Dopen Request\n");
	#endif
	ret=dopen((char *)&sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

void* fileXioRpc_Dread(unsigned int* sbuff)
{
	int ret=-1;
	#ifdef DEBUG
		printf("RPC Dread Request\n");
	#endif
	ret=fileXio_dread_RPC(sbuff[0/4],(void*)sbuff[4/4]);
	sbuff[0] = ret;
	return sbuff;
}

void* fileXioRpc_Dclose(unsigned int* sbuff)
{
	int ret;
	#ifdef DEBUG
		printf("RPC Dclose Request\n");
	#endif
	ret=dclose(sbuff[0/4]);
	sbuff[0] = ret;
	return sbuff;
}

/*************************************************
* The functions below are for internal use only, *
* and are not to be exported                     *
*************************************************/

void fileXio_Thread(void* param)
{
	printf("fileXio: fileXio RPC Server v1.00\nCopyright (c) 2003 adresd\n");
	#ifdef DEBUG
		printf("fileXio: RPC Initialize\n");
	#endif

	SifInitRpc(0);

	// 0x4800 bytes for DirEntry structures 
	// 0x400 bytes for the filename string
	buffer = AllocSysMemory(0, 0x4C00, NULL);
	if(buffer == NULL)
	{
		#ifdef DEBUG
  			printf("Failed to allocate memory for RPC buffer!\n");
		#endif

		SleepThread();
	}

	rwbuf = AllocSysMemory(0, RWSIZE, NULL);
	if (rwbuf == NULL)
	{
		#ifdef DEBUG
  			printf("Failed to allocate memory for RW buffer!\n");
		#endif

		SleepThread();
	}

	SifSetRpcQueue(&qd, GetThreadId());
	SifRegisterRpc(&sd0, FILEXIO_IRX,fileXio_rpc_server,(void *)buffer,0,0,&qd);
	SifRpcLoop(&qd);
}

void* fileXio_rpc_server(int fno, void *data, int size)
{

	switch(fno) {
		case FILEXIO_DOPEN:
			return fileXioRpc_Dopen((unsigned*)data);
		case FILEXIO_DREAD:
			return fileXioRpc_Dread((unsigned*)data);
		case FILEXIO_DCLOSE:
			return fileXioRpc_Dclose((unsigned*)data);
		case FILEXIO_MOUNT:
			return fileXioRpc_Mount((unsigned*)data);
		case FILEXIO_UMOUNT:
			return fileXioRpc_uMount((unsigned*)data);
		case FILEXIO_GETDIR:
			return fileXioRpc_Getdir((unsigned*)data);
		case FILEXIO_COPYFILE:
			return fileXioRpc_CopyFile((unsigned*)data);
		case FILEXIO_STOP:
			return fileXioRpc_Stop();
		case FILEXIO_CHDIR:
			return fileXioRpc_ChDir((unsigned*)data);
		case FILEXIO_RENAME:
			return fileXioRpc_Rename((unsigned*)data);
		case FILEXIO_MKDIR:
			return fileXioRpc_MkDir((unsigned*)data);
		case FILEXIO_RMDIR:
			return fileXioRpc_RmDir((unsigned*)data);
		case FILEXIO_REMOVE:
			return fileXioRpc_Remove((unsigned*)data);
		case FILEXIO_OPEN:
			return fileXioRpc_Open((unsigned*)data);
		case FILEXIO_CLOSE:
			return fileXioRpc_Close((unsigned*)data);
		case FILEXIO_READ:
			return fileXioRpc_Read((unsigned*)data);
		case FILEXIO_WRITE:
			return fileXioRpc_Write((unsigned*)data);
		case FILEXIO_LSEEK:
			return fileXioRpc_Lseek((unsigned*)data);
		case FILEXIO_IOCTL:
			return fileXioRpc_Ioctl((unsigned*)data);
		case FILEXIO_GETSTAT:
			return fileXioRpc_GetStat((unsigned*)data);
		case FILEXIO_CHSTAT:
			return fileXioRpc_ChStat((unsigned*)data);
		case FILEXIO_FORMAT:
			return fileXioRpc_Format((unsigned*)data);
		case FILEXIO_ADDDRV:
			return fileXioRpc_AddDrv((unsigned*)data);
		case FILEXIO_DELDRV:
			return fileXioRpc_DelDrv((unsigned*)data);
		case FILEXIO_SYNC:
			return fileXioRpc_Sync((unsigned*)data);
		case FILEXIO_DEVCTL:
			return fileXioRpc_Devctl((unsigned*)data);
		case FILEXIO_SYMLINK:
			return fileXioRpc_SymLink((unsigned*)data);
		case FILEXIO_READLINK:
			return fileXioRpc_ReadLink((unsigned*)data);
		case FILEXIO_IOCTL2:
			return fileXioRpc_Ioctl2((unsigned*)data);
		case FILEXIO_LSEEK64:
			return fileXioRpc_Lseek64((unsigned*)data);
	}
	return NULL;
}


// Copy a DIR Entry from the native format to our format
void DirEntryCopy(struct fileXioDirEntry* dirEntry, iox_dirent_t* internalDirEntry)
{
	dirEntry->fileSize = internalDirEntry->stat.size;
	dirEntry->fileProperties = internalDirEntry->stat.attr;
	strncpy(dirEntry->filename,internalDirEntry->name,127);
	dirEntry->filename[127] = '\0';
}
