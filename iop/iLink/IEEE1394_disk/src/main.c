#include <errno.h>
#include <ioman.h>
#include <irx.h>
#include <loadcore.h>
#include <stdio.h>
#include <thbase.h>
#include <sysmem.h>

#include "sbp2_disk.h"
#include "fs_driver.h"
#include "fat_driver.h"

#define MODNAME "IEEE1394_disk"
IRX_ID(MODNAME, 0x01, 0x06);

static int nulldev(){
	return -EIO;
};

/* Device driver I/O functions */
static iop_device_ops_t device_functarray={
	&fs_init,		/* INIT */
	&fs_deinit,		/* DEINIT */
	(void *)&nulldev,	/* FORMAT */
	&fs_open,		/* OPEN */
	&fs_close,		/* CLOSE */
	&fs_read,		/* READ */
	&fs_write,		/* WRITE */
	&fs_lseek,		/* LSEEK */
	&fs_ioctl,		/* IOCTL */
	&fs_remove,		/* REMOVE */
	&fs_mkdir,		/* MKDIR */
	&fs_rmdir,		/* RMDIR */
	&fs_dopen,		/* DOPEN */
	&fs_dclose,		/* DCLOSE */
	&fs_dread,		/* DREAD */
	&fs_getstat,		/* GETSTAT */
	(void *)&nulldev,	/* CHSTAT */
};

static const char dev_name[]="sd";

static iop_device_t device_driver={
	dev_name,		/* Device name */
	IOP_DT_FS,		/* Device type flag */
	1,			/* Version */
	"IEEE1394_disk",	/* Description. */
	&device_functarray	/* Device driver function pointer array */
};

int _start(int argc, char** argv)
{
  if(InitFAT() != 0)
  {
	printf("Error initializing FAT driver!\n");
	return MODULE_NO_RESIDENT_END;
  }

  init_ieee1394DiskDriver();

  DelDrv(dev_name);
  AddDrv(&device_driver);

#if 0
   iop_sys_clock_t lTime;
   u32 lSecStart, lUSecStart;
   u32 lSecEnd,   lUSecEnd, nbytes;
   int fd, size, bytesToRead, block_size;

   printf("trying to open file...");
   while((fd=open("sd0:PS2ESDL/SLPM_55052_00.pdi", O_RDONLY))<0){DelayThread(2000);};
   nbytes=size=lseek(fd, 0, SEEK_END)/4;
   block_size=2048*512;
   lseek(fd, 0, SEEK_SET);

   printf("Read test start\n" );

   GetSystemTime ( &lTime );
   SysClock2USec ( &lTime, &lSecStart, &lUSecStart );

	void *buffer;
	if((buffer=malloc(block_size))==NULL) printf("Unable to allocate memory. :(\n");
	printf("Read test: %p.\n", buffer);
	while(size>0){
		bytesToRead=(size>(block_size))?(block_size):size;
		read(fd, buffer, bytesToRead);
		size-=bytesToRead;
	}
	free(buffer);
	printf("Completed.\n");

   GetSystemTime ( &lTime );
   SysClock2USec ( &lTime, &lSecEnd, &lUSecEnd );
   close(fd);

   printf("Done: %lu %lu/%lu %lu\n", lSecStart, lUSecStart, lSecEnd, lUSecEnd );
   printf("KB: %ld, time: %ld, Approximate KB/s: %ld.\n", (nbytes/1024), (lSecEnd -lSecStart), (nbytes/1024)/(lSecEnd -lSecStart));
#endif

  return MODULE_RESIDENT_END;
}
