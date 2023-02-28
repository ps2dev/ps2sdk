/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# IOP exception handling.
*/

#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "irx.h"
#include <errno.h>
#include <stdio.h>
#include <ioman.h>

#include "sior.h"
#include "xprintf.h"

#include "irx_imports.h"
#define MAJOR 1
#define MINOR 0
#define MODNAME "SIOTTY"
IRX_ID(MODNAME, MAJOR, MINOR);

#define DPRINTF(x...) sio_printf(MODNAME": "x)//, printf("IOP: "x)
#define PPRINTF(x...) printf(x) // this one must be used BEFORE SIOR RPC is not ready
extern struct irx_export_table _exp_sior;

static struct t_SifRpcClientData cd0;
static union siorCommsData buffer;
static char tbuf[2048];
 
#define DEVNAME "tty"

static int tty_sema = -1;

static int tty_init(iop_device_t *device);
static int tty_deinit(iop_device_t *device);
static int tty_stdout_fd(void);
static int tty_write(iop_file_t *file, void *buf, size_t size);
static int tty_error(void);


/* device ops */
static iop_device_ops_t tty_ops = {
    tty_init,
    tty_deinit,
    (void *)tty_error,
    (void *)tty_stdout_fd,
    (void *)tty_stdout_fd,
#ifndef SUPPORT_READ
    (void *)tty_error,
#else
    tty_read,
#endif
    tty_write,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
    (void *)tty_error,
};
 
/* device descriptor */
static iop_device_t tty_device = {
    DEVNAME,
     IOP_DT_FS, //IOP_DT_CHAR | IOP_DT_CONS,
    1,
    "TTY via EE SIO",
    &tty_ops,
};
 
#ifdef KPRTTY
#define PRNT_IO_BEGIN 0x200
#define PRNT_IO_END   0x201
 
typedef struct _KprArg
{
    int eflag;
    int bsize;
    char *kpbuf;
    int prpos;
    int calls;
} KprArg;
 
static KprArg g_kprarg;
 
#define KPR_BUFFER_SIZE 0x1000
static char kprbuffer[KPR_BUFFER_SIZE];
 
static void PrntFunc(void *context, int chr)
{
    KprArg *kpa = (KprArg *)context;
 
    switch (chr) {
        case 0:
            break;
        case PRNT_IO_BEGIN:
            kpa->calls++;
            break;
        case PRNT_IO_END:
            break;
        case '\n':
            PrntFunc(context, '\r');
        default:
            if (kpa->prpos < kpa->bsize)
                kpa->kpbuf[kpa->prpos++] = chr;
            break;
    }
}
 
static int Kprnt(void *context, const char *format, void *arg)
{
    if (format)
        prnt(PrntFunc, context, format, arg);
 
    return 0;
}
 
static int Kprintf_Handler(void *context, const char *format, va_list ap)
{
    int res;
    KprArg *kpa = (KprArg *)context;
 
    res = CpuInvokeInKmode(Kprnt, kpa, format, ap);
 
    if (QueryIntrContext())
        iSetEventFlag(kpa->eflag, 1);
    else
        SetEventFlag(kpa->eflag, 1);

    return res;
}
 
static void KPRTTY_Thread(void *args)
{
    u32 flags;
    KprArg *kpa = (KprArg *)args;
 
    while (1) {
        WaitEventFlag(kpa->eflag, 1, WEF_AND | WEF_CLEAR, &flags);
 
        if (kpa->prpos) {
            write(1, kpa->kpbuf, kpa->prpos);
            kpa->prpos = 0;
        }
    }
}
 
static void kprtty_init(void)
{
    DPRINTF("%s\n", __func__);
    iop_event_t efp;
    iop_thread_t thp;
    KprArg *kpa;
    int thid, startret;
 
    kpa = &g_kprarg;
 
    efp.attr   = EA_SINGLE;
    efp.option = 0;
    efp.bits   = 0;
 
    thp.attr      = TH_C;
    thp.option    = 0;
    thp.thread    = &KPRTTY_Thread;
    thp.stacksize = 0x800;
    thp.priority  = 8;//8;
 
    kpa->eflag = CreateEventFlag(&efp);
    kpa->bsize = KPR_BUFFER_SIZE;
    kpa->kpbuf = kprbuffer;
    kpa->prpos = 0;
    kpa->calls = 0;
 
    thid = CreateThread(&thp);
    if (thid < 0) DPRINTF("FAILED TO CREATE THREAD FOR KPRTTY\n");
    DPRINTF("kprtty thread ID is %i\n", thid);
    startret = StartThread(thid, (void *)kpa);
    DPRINTF("Started thread (%i)\n", startret);
    KprintfSet(&Kprintf_Handler, kpa);
   // Kprintf("LOL %i", thid);
}
#endif
 

void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode) {
    struct siorInitArgs * i = &buffer.init;
    i->baudrate = baudrate;
    i->lcr_ueps = lcr_ueps;
    i->lcr_upen = lcr_upen;
    i->lcr_usbl = lcr_usbl;
    i->lcr_umode = lcr_umode;
    SifCallRpc(&cd0, SIOR_INIT, 0, &buffer, sizeof(struct siorInitArgs), NULL, 0, NULL, NULL);
}

int sio_putc(int c) {
    buffer.result = c;
    SifCallRpc(&cd0, SIOR_PUTC, 0, &buffer, sizeof(int), &buffer, sizeof(int), NULL, NULL);
    return buffer.result;
}

int sio_getc(void) {
    SifCallRpc(&cd0, SIOR_GETC, 0, 0, 0, &buffer, sizeof(int), NULL, NULL);
    return buffer.result;
}

int sio_getc_block(void) {
    SifCallRpc(&cd0, SIOR_GETCBLOCK, 0, 0, 0, &buffer, sizeof(int), NULL, NULL);
    return buffer.result;
}

size_t sio_write(const char *buf, size_t size) {
    buffer.write.buf = buf;
    buffer.write.len = size;
    SifCallRpc(&cd0, SIOR_WRITE, 0, &buffer, 8, &buffer, 4, NULL, NULL);
    return buffer.result;
}

size_t sio_read(char *buf, size_t size) {
    buffer.read.buf = buf;
    buffer.read.len = size;
    SifCallRpc(&cd0, SIOR_READ, 0, &buffer, 8, &buffer, 4, NULL, NULL);
    return buffer.result;
}

int sio_puts(const char * str) {
    buffer.cstr = str;
    SifCallRpc(&cd0, SIOR_PUTS, 0, &buffer, 4, &buffer, 4, NULL, NULL);
    return buffer.result;
}

int sio_putsn(const char * str) {
    buffer.cstr = str;
    SifCallRpc(&cd0, SIOR_PUTSN, 0, &buffer, 4, &buffer, 4, NULL, NULL);
    return buffer.result;
}

char *sio_gets(char * str) {
    buffer.str = str;
    SifCallRpc(&cd0, SIOR_GETS, 0, &buffer, 4, &buffer, 4, NULL, NULL);
    return buffer.str;
}

void sio_flush(void) {
    SifCallRpc(&cd0, SIOR_FLUSH, 0, NULL, 0, NULL, 0, NULL, NULL);
}

int sio_vprintf(const char * str, va_list args) {
    int res;

    res = vsnprintf(tbuf, sizeof(tbuf), str, args);
    //printf("%s", tbuf);
    sio_putsn(tbuf);

    return res;
}

int sio_printf(const char * str, ...) {
    int res;
    va_list args;

    va_start(args, str);
    res = sio_vprintf(str, args);
    va_end(args);

    return res;
}

int _start(int argc, char *argv[])
{
    int retries;

    (void)argc;
    (void)argv;

    memset(&cd0, 0, sizeof(cd0));

    for (retries = 0; retries < 15; retries++) {
	PPRINTF("Binding to SIOR RPC.\n");
	if (SifBindRpc(&cd0, SIOR_IRX, 0) < 0) {
	    PPRINTF("\tFailed!\n");
	    return MODULE_NO_RESIDENT_END;
	}
	if (cd0.server != 0) {
	    PPRINTF("Registering library.\n");
	    if (RegisterLibraryEntries(&_exp_sior)) {
		PPRINTF("\ttFailed\n");
		return MODULE_NO_RESIDENT_END;
	    }
        DPRINTF("RPC Binding success!\n");
	    goto SIOR_BIND_SUCCESS;
	}
	PPRINTF("\tnot available. retrying...\n");
	DelayThread(1000);
    }
    PPRINTF("Giving up.\n");
    return MODULE_NO_RESIDENT_END;
    SIOR_BIND_SUCCESS:
    DPRINTF("\n\t%s By El_isra\nBased on SIOR.IRX and UDPTTY.IRX\n", MODNAME);
    close(0);
    close(1);
    close(2);
    DelDrv(tty_device.name);
 
    if (AddDrv(&tty_device) < 0)
    {
        DPRINTF("failed to AddDrv()\n");
        return MODULE_NO_RESIDENT_END;
    }
    retries = open(DEVNAME":", O_WRONLY /*0x1000 | O_RDWR*/);
    retries = open(DEVNAME":", O_WRONLY);
    retries = open(DEVNAME":", O_WRONLY);
 
    DPRINTF("%s v%i.%s loaded and ready!\n", MODNAME, MAJOR, MINOR);
 
#ifdef KPRTTY
    kprtty_init();
    DPRINTF("KPRTTY enabled!\n");
#endif

 return MODULE_RESIDENT_END;
}


/* TTY driver.  */
 
static int tty_init(iop_device_t *device)
{
    (void)device;
 
    if ((tty_sema = CreateMutex(IOP_MUTEX_UNLOCKED)) < 0)
    {
        DPRINTF("Could NOT CreateMutex(IOP_MUTEX_UNLOCKED)\n");
        return -1;
    } else DPRINTF("%s sucess\n", __func__);
    return 0;
}
 
static int tty_deinit(iop_device_t *device)
{
    DPRINTF("%s Deinit\n", device->name);
    DeleteSema(tty_sema);
    return 0;
}
 
static int tty_stdout_fd(void)
{
    return 1;
}
 
static int tty_write(iop_file_t *file, void *buf, int size)
{
    (void)file;

    WaitSema(tty_sema);
    sio_write(buf, size);
    SignalSema(tty_sema);

    return 0;
}
 
static int tty_error(void)
{
    return -EIO;
}

#ifdef SUPPORT_READ
static int tty_read (iop_file_t *, void buf*, int size)
{
    (void)file;
    sio_read(buf, size);
    return 0;
}
#endif