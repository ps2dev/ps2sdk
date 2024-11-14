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
#include <errno.h>
#include <sysclib.h>
#include <sysmem.h>
#include <excepman.h>
#include <intrman.h>
#include <ioman.h>
#include <ps2_debug.h>
#include <ps2_sbus.h>

#include "iopdebug.h"

extern void sbus_tty_puts(const char *str);

static int ttyfs_error() { return -EPERM; }

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

static int ttyfs_open(iop_file_t *file, const char *name, int flags)
{
    (void)file;
    (void)name;
    (void)flags;

    //DBG_puts("SIOTTY: FS Open()\n");
	return 0;
}

static int ttyfs_dopen(iop_file_t *file, const char *name)
{
    (void)file;
    (void)name;

    //DBG_puts("SIOTTY: FS Dopen()\n");
    return 0;
}

static int ttyfs_close(iop_file_t *file)
{
    (void)file;

    //DBG_puts("SIOTTY: FS Close()\n");
    return(0);
}

static int ttyfs_write(iop_file_t *file, void *ptr, int size) {
    char temp[65];
    int bCount = 0;

    (void)file;

    //DBG_puts("SIOTTY: FS Write()\n");

    while(bCount < size)
    {
        int toWrite;

        toWrite = (size - bCount);
        if(toWrite > 64)
            toWrite = 64;

        memcpy(temp, &(((u8 *)ptr)[bCount]), toWrite);
        temp[toWrite] = '\0';
        sbus_tty_puts(temp);

        bCount += toWrite;
    }

    return(bCount);
}

static iop_device_ops_t fsd_ops =
{
    &ttyfs_init,
    &ttyfs_deinit,
    (void *)&ttyfs_error,
    &ttyfs_open,
    &ttyfs_close,
	(void *)&ttyfs_error,
    &ttyfs_write,
    (void *)&ttyfs_error,
    (void *)&ttyfs_error,
    (void *)&ttyfs_error,
    (void *)&ttyfs_error,
    (void *)&ttyfs_error,
	&ttyfs_dopen,
    &ttyfs_close,
    (void *)&ttyfs_error,
    (void *)&ttyfs_error,
    (void *)&ttyfs_error,
};

static iop_device_t tty_fsd =
{
	"tty",
	IOP_DT_FS,
	1,
	"TTY via EE SIO",
	&fsd_ops,
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
    (void)context;

    int r = prnt(&sprintf_putchar, &format, format, ap);
    sbus_tty_puts(kprint_buffer);
    return r;
}

int sbus_tty_init(void)
{
    close(0);
    close(1);
	DelDrv(tty_fsd.name);
	DelDrv("dummytty");

	if(AddDrv(&tty_fsd) != 0) { return(-1); }

    open("tty:", O_RDONLY);
    open("tty:", O_WRONLY);

    KprintfSet(&_kPrintf, NULL);

    return(0);
}
