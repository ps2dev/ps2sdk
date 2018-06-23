/*

IOP->EE TTY over SBUS by Herben

This file contains the file system driver and glue needed to create a TTY file file system and redirect it to EE over SBUS.

Additionally it installs a Kprintf handler so you can even see the things you don't normally see like the error messages WaitSema gives
for those naughty ps2ip modules. ;)

Of course this requires that the EE-side code accept this command and output the string to screen/sio...

*/

#include <tamtypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <sysclib.h>
#include <sysmem.h>
#include <sys/stat.h>
#include <excepman.h>
#include <intrman.h>
#include <ioman.h>
#include <ps2_debug.h>
#include <ps2_sbus.h>

#include "iopdebug.h"

extern void sbus_tty_puts(const char *str);

static int ttyfs_error() { return -1; }

static int ttyfs_init()
{
    //DBG_puts("SIOTTY: FS Init()\n");
	return 0;
}

static int ttyfs_deinit()
{
    //DBG_puts("SIOTTY: FS Deinit()\n");
	return 0;
}

static int ttyfs_open(iop_file_t *file, const char *name, int flags, int mode)
{
    //DBG_puts("SIOTTY: FS Open()\n");
	return 0;
}

static int ttyfs_close(iop_file_t *file, int fd)
{
    //DBG_puts("SIOTTY: FS Close()\n");
    return(0);
}

static int ttyfs_read(iop_file_t *file, void *ptr, size_t size) {
    //DBG_puts("SIOTTY: FS Read()\n");
    return(-1);
}

static int ttyfs_write(iop_file_t *file, u8 *ptr, size_t size) {
    char temp[65];
    int bCount = 0, toWrite;

    //DBG_puts("SIOTTY: FS Write()\n");

    while(bCount < size)
    {
        toWrite = (size - bCount);
        if(toWrite > 64)
            toWrite = 64;

        memcpy(temp, &ptr[bCount], toWrite);
        temp[toWrite] = '\0';
        sbus_tty_puts(temp);

        bCount += toWrite;
    }

    return(bCount);
}

static int ttyfs_lseek(iop_file_t *file, int offset, int mode) {
    //DBG_puts("SIOTTY: FS Lseek()\n");

    return(-1);
}

static int ttyfs_format(iop_file_t *file, const char *dev, const char *blockdev, void *arg, size_t arglen) {
    //DBG_puts("SIOTTY: FS Format()\n");

    return(-1);
}

static int ttyfs_remove(iop_file_t *file, const char *name) {
    //DBG_puts("SIOTTY: FS Remove()\n");

    return(-1);
}

static int ttyfs_dread(iop_file_t *file, iox_dirent_t *dirent) {
    //DBG_puts("SIOTTY: FS Dread()\n");

    return(-1);
}

static int ttyfs_getstat(iop_file_t *file, const char *name, iox_stat_t *stat) {
    //DBG_puts("SIOTTY: FS GetStat()\n");

    return(-1);
}

static int ttyfs_devctl(iop_file_t *file, const char *name, int cmd, void *arg, size_t arglen, void *buf, size_t buflen) {
    //DBG_puts("SIOTTY: FS DEVCTL()\n");

    return(-1);
}

static int ttyfs_ioctl2(iop_file_t *file, int cmd, void *arg, size_t arglen, void *buf, size_t buflen) {
    //DBG_puts("SIOTTY: FS IOCTL2()\n");

    return(-1);
}


static void *fsd_ops[] = { ttyfs_init, ttyfs_deinit, ttyfs_format, ttyfs_open, ttyfs_close,
	ttyfs_read, ttyfs_write, ttyfs_lseek, ttyfs_error, ttyfs_remove, ttyfs_error, ttyfs_error,
	ttyfs_open, ttyfs_close, ttyfs_dread, ttyfs_getstat, ttyfs_error, ttyfs_error, ttyfs_error,
	ttyfs_error, ttyfs_error, ttyfs_error, ttyfs_error, ttyfs_devctl, ttyfs_error, ttyfs_error,
	ttyfs_ioctl2
};

iop_device_t tty_fsd =
{
	"tty",
	0x10,
	1,
	"TTY via EE SIO",
	(iop_device_ops_t *) &fsd_ops
};

void sprintf_putchar(void *context, int c)
{
    char **string = (char **)context;

    if(c < 0x100) { ((*string)++)[0] = c; }
    else { (*string)[0] = 0; }
}

extern int _vsprintf(char * str, const char * format, va_list ap);

static char kprint_buffer[1024];

int _kPrintf(void *context, const char * format, va_list ap)
{
    int r = prnt(&sprintf_putchar, &format, format, ap);
    sbus_tty_puts(kprint_buffer);
    return r;
}

int sbus_tty_init(void)
{
    close(0);
    close(1);
	DelDrv("tty");
	DelDrv("dummytty");

	if(AddDrv(&tty_fsd) != 0) { return(-1); }

    open("tty:", O_RDONLY);
    open("tty:", O_WRONLY);

    KprintfSet(&_kPrintf, NULL);

    return(0);
}
