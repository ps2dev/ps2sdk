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

#define MODNAME "imgdrv"

IRX_ID(MODNAME, 1, 1);

// Function prototypes
static int imgdrv_read(iop_file_t *f, void *buf, int size);
static int imgdrv_lseek(iop_file_t *f, int offset, int whence);

/*  The IOMAN operations structure is usually longer than this,
    but the structure in the original was this short (probably to save space). */
typedef struct _iop_device_ops_short
{
    int (*init)(iop_device_t *);
    int (*deinit)(iop_device_t *);
    int (*format)(iop_file_t *);
    int (*open)(iop_file_t *, const char *, int);
    int (*close)(iop_file_t *);
    int (*read)(iop_file_t *, void *, int);
    int (*write)(iop_file_t *, void *, int);
    int (*lseek)(iop_file_t *, int, int);
} iop_device_ops_short_t;

static iop_device_ops_short_t imgdrv_ops = {
    DUMMY_IMPLEMENTATION, // init
    DUMMY_IMPLEMENTATION, // deinit
    NOT_SUPPORTED, // format
    NOT_SUPPORTED, // open
    NOT_SUPPORTED, // close
    &imgdrv_read, // read
    NOT_SUPPORTED, // write
    &imgdrv_lseek, // lseek
};

#define MAX_IMAGES 2
// These arrays will be manipulated by the EE-side code before the module is loaded.
void *img[MAX_IMAGES]    = {(void *)0xDEC1DEC1, 0};
int img_size[MAX_IMAGES] = {0xDEC2DEC2, 0};

static iop_device_t img_device = {
    "img",
    IOP_DT_FS,
    1,
    "img",
    (iop_device_ops_t *)&imgdrv_ops,
};

int _start(int argc, char *argv[])
{
    return (AddDrv(&img_device) < 0) ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END;
}

static int imgdrv_read(iop_file_t *f, void *buf, int size)
{
    int i;
    const u32 *img_ptr;
    u32 *img_out;

    img_out = (u32 *)buf;
    img_ptr = (const u32 *)img[f->unit];
    for (i = size; i > 0; i -= 4, img_ptr++, img_out++)
        *img_out = *img_ptr;

    return size;
}

static int imgdrv_lseek(iop_file_t *f, int offset, int whence)
{
    return (whence == SEEK_SET ? 0 : img_size[f->unit]);
}
