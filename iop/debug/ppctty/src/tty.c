/*

IOP->PPC TTY

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
#include <thsemap.h>
#include "tty.h"

#ifdef DEBUG
#define DPRINTF(fmt, x...) printf(MODNAME ": " fmt, ##x)
#else
#define DPRINTF(x...)
#endif

static int tty_sema = -1;

extern void tty_puts(const char *str);

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

IOMAN_RETURN_VALUE_IMPL(0);
IOMAN_RETURN_VALUE_IMPL(EPERM);

static iop_device_ops_t fsd_ops =
{
    &ttyfs_init, // init
    &ttyfs_deinit, // deinit
    IOMAN_RETURN_VALUE(EPERM), // format
    IOMAN_RETURN_VALUE(0), // open
    IOMAN_RETURN_VALUE(0), // close
    IOMAN_RETURN_VALUE(EPERM), // read
    &ttyfs_write, // write
    IOMAN_RETURN_VALUE(EPERM), // lseek
    IOMAN_RETURN_VALUE(EPERM), // ioctl
    IOMAN_RETURN_VALUE(EPERM), // remove
    IOMAN_RETURN_VALUE(EPERM), // mkdir
    IOMAN_RETURN_VALUE(EPERM), // rmdir
    IOMAN_RETURN_VALUE(0), // dopen
    IOMAN_RETURN_VALUE(0), // dclose
    IOMAN_RETURN_VALUE(EPERM), // dread
    IOMAN_RETURN_VALUE(EPERM), // getstat
    IOMAN_RETURN_VALUE(EPERM), // chstat
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
