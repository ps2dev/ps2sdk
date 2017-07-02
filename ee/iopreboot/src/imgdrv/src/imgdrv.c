/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Image device driver (IMGDRV)
 * Used by the special IOP reboot sequence, to reboot the IOP with a buffered IOPRP image.
 */

#include <ioman.h>
#include <irx.h>
#include <loadcore.h>

//Function prototypes
static int imgdrv_dummy(void);
static int imgdrv_read(iop_file_t *f, void *buf, int size);
static int imgdrv_lseek(iop_file_t *f, int offset, int whence);

static iop_device_ops_t imgdrv_ops = {
	(void*)imgdrv_dummy,	//init
	(void*)imgdrv_dummy,	//deinit
	NULL,			//format
	(void*)imgdrv_dummy,	//open
	(void*)imgdrv_dummy,	//close
	&imgdrv_read,		//read
	NULL,			//write
	&imgdrv_lseek,		//lseek
};

#define MAX_IMAGES	2
//These arrays will be manipulated by the EE-side code before the module is loaded.
const void *img[MAX_IMAGES] = {0};
int img_size[MAX_IMAGES] = {0};

static iop_device_t img_device = {
	"img",
	IOP_DT_FS,
	1,
	"img",
	&imgdrv_ops
};

int _start(int argc, char *argv[])
{
	return AddDrv(&img_device) < 0;
}

static int imgdrv_dummy(void)
{
	return 0;
}

static int imgdrv_read(iop_file_t *f, void *buf, int size)
{
	int i;
	const u32 *img_ptr;
	u32 *img_out;

	img_out = (u32*)buf;
	img_ptr = (const u32*)img[f->unit];
	for(i = size; i > 0; i-=4,img_ptr++,img_out++)
		*img_out = *img_ptr;

	return size;
}

static int imgdrv_lseek(iop_file_t *f, int offset, int whence)
{
	return(whence == SEEK_SET ? 0 : img_size[f->unit]);
}
