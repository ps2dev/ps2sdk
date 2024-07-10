/*

IOP->PPC TTY

*/

#include <tamtypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <sysclib.h>
#include <sysmem.h>
#include <excepman.h>
#include <intrman.h>
#include <ioman.h>
#include <thsemap.h>
#include "tty.h"

#ifdef DEBUG
#define DPRINTF(fmt, x...) printf(MODNAME ": " fmt, ##x)
#else
#define DPRINTF(x...)
#endif

static int tty_sema = -1;

extern void tty_puts(const char *str);

static int ttyfs_error() { return -1; }

static int ttyfs_init()
{
    DPRINTF("FS Init()\n");
    if ((tty_sema = CreateMutex(IOP_MUTEX_UNLOCKED)) < 0)
    {
        DPRINTF("Failed to create mutex\n");
        return -1;
    }
	return 0;
}

static int ttyfs_deinit()
{
    DPRINTF("FS Deinit()\n");
    DeleteSema(tty_sema);
	return 0;
}

static int ttyfs_open(iop_file_t *file, const char *name, int flags)
{
    (void)file;
    (void)name;
    (void)flags;

    DPRINTF("FS Open()\n");
	return 0;
}

static int ttyfs_dopen(iop_file_t *file, const char *name)
{
    (void)file;
    (void)name;

    DPRINTF("FS Dopen()\n");
    return 0;
}

static int ttyfs_close(iop_file_t *file)
{
    (void)file;

    DPRINTF("FS Close()\n");
    return(0);
}

static int ttyfs_write(iop_file_t *file, void *ptr, int size) {
    char temp[65];
    int bCount = 0;

    (void)file;

    DPRINTF("FS Write()\n");

    WaitSema(tty_sema);
    while(bCount < size)
    {
        int toWrite;

        toWrite = (size - bCount);
        if(toWrite > 64)
            toWrite = 64;

        memcpy(temp, &(((u8 *)ptr)[bCount]), toWrite);
        temp[toWrite] = '\0';
        tty_puts(temp);

        bCount += toWrite;
    }
    SignalSema(tty_sema);

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
	IOP_DT_CHAR | IOP_DT_CONS,
	1,
	"TTY via PPC SIO",
	&fsd_ops,
};

#ifdef KPRINTF
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
    tty_puts(kprint_buffer);
    return r;
}
#endif

int tty_init(void)
{
	DelDrv(tty_fsd.name);
	DelDrv("dummytty");

	if(AddDrv(&tty_fsd) != 0) { return(-1); }

    // open stdin
    close(0);
    open("tty:", O_RDONLY);
    // open stdout
    close(1);
    open("tty:", O_WRONLY);
#ifdef KPRINTF
    printf("PPCTTY: KprintfSet\n");
    KprintfSet(&_kPrintf, NULL);
#endif
    return(0);
}
