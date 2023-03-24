#include <loadcore.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <irx.h>
#include <secrman.h>
#include <sifman.h>
#include <sifcmd.h>

#include "secrsif.h"

#define MODNAME "secrsif"
IRX_ID(MODNAME, 1, 2);

#ifdef DEBUG
#define DPRINTF(x...) printf("SECRSIF: " x)
#else
#define DPRINTF(x...)
#endif

extern struct irx_export_table _exp_secrsif;

static SifRpcDataQueue_t SifSecrDownloadHeaderQD;
static SifRpcDataQueue_t SifSecrDownloadBlockQD;
static SifRpcDataQueue_t SifSecrDownloadGetKbitQD;
static SifRpcDataQueue_t SifSecrDownloadGetKcQD;
static SifRpcDataQueue_t SifSecrDownloadGetICVPS2QD;
static SifRpcDataQueue_t SifSecrDiskBootHeaderQD;
static SifRpcDataQueue_t SifSecrDiskBootBlockQD;

static SifRpcServerData_t SifSecrDownloadHeaderData;
static SifRpcServerData_t SifSecrDownloadBlockData;
static SifRpcServerData_t SifSecrDownloadGetKbitData;
static SifRpcServerData_t SifSecrDownloadGetKcData;
static SifRpcServerData_t SifSecrDownloadGetICVPS2Data;
static SifRpcServerData_t SifSecrDiskBootHeaderData;
static SifRpcServerData_t SifSecrDiskBootBlockData;

static int SifSecrDownloadHeaderThreadID;
static int SifSecrDownloadBlockThreadID;
static int SifSecrDownloadGetKbitThreadID;
static int SifSecrDownloadGetKcThreadID;
static int SifSecrDownloadGetICVPS2ThreadID;
static int SifSecrDiskBootHeaderThreadID;
static int SifSecrDiskBootBlockThreadID;

static unsigned char SifServerBuffer[0x1000];

static void *SifSecrDownloadHeader(int function, void *buffer, int nbytes)
{
    (void)function;
    (void)nbytes;

    ((struct SecrSifDownloadHeaderParams *)buffer)->result = SecrDownloadHeader(((struct SecrSifDownloadHeaderParams *)buffer)->port, ((struct SecrSifDownloadHeaderParams *)buffer)->slot, ((struct SecrSifDownloadHeaderParams *)buffer)->buffer, &((struct SecrSifDownloadHeaderParams *)buffer)->BitTable, &((struct SecrSifDownloadHeaderParams *)buffer)->size);
    return buffer;
}

static void *SifSecrDownloadBlockHandler(int function, void *buffer, int nbytes)
{
    (void)function;
    (void)nbytes;

    ((struct SecrSifDownloadBlockParams *)buffer)->result = SecrDownloadBlock(((struct SecrSifDownloadBlockParams *)buffer)->buffer, ((struct SecrSifDownloadBlockParams *)buffer)->size);
    return buffer;
}

static void *SifSecrDownloadGetKbitHandler(int function, void *buffer, int nbytes)
{
    (void)function;
    (void)nbytes;

    ((struct SecrSifDownloadGetKbitParams *)buffer)->result = SecrDownloadGetKbit(((struct SecrSifDownloadGetKbitParams *)buffer)->port, ((struct SecrSifDownloadGetKbitParams *)buffer)->slot, ((struct SecrSifDownloadGetKbitParams *)buffer)->kbit);
    return buffer;
}

static void *SifSecrDownloadGetKcHandler(int function, void *buffer, int nbytes)
{
    (void)function;
    (void)nbytes;
    ((struct SecrSifDownloadGetKcParams *)buffer)->result = SecrDownloadGetKc(((struct SecrSifDownloadGetKcParams *)buffer)->port, ((struct SecrSifDownloadGetKcParams *)buffer)->slot, ((struct SecrSifDownloadGetKcParams *)buffer)->kc);
    return buffer;
}

static void *SifSecrDownloadGetICVPS2Handler(int function, void *buffer, int nbytes)
{
    (void)function;
    (void)nbytes;

    ((struct SecrSifDownloadGetIcvps2Params *)buffer)->result = SecrDownloadGetICVPS2(((struct SecrSifDownloadGetIcvps2Params *)buffer)->icvps2);
    return buffer;
}

static void *SifSecrDiskBootHeaderHandler(int function, void *buffer, int nbytes)
{
    (void)function;
    (void)nbytes;

    ((struct SecrSifDiskBootHeaderParams *)buffer)->result = SecrDiskBootHeader(((struct SecrSifDiskBootHeaderParams *)buffer)->buffer, &((struct SecrSifDiskBootHeaderParams *)buffer)->BitTable, &((struct SecrSifDiskBootHeaderParams *)buffer)->size);
    return buffer;
}

static void *SifSecrDiskBootBlockHandler(int function, void *buffer, int nbytes)
{
    (void)function;
    (void)nbytes;

    ((struct SecrSifDiskBootBlockParams *)buffer)->result = SecrDiskBootBlock(((struct SecrSifDiskBootBlockParams *)buffer)->source, ((struct SecrSifDiskBootBlockParams *)buffer)->destination, ((struct SecrSifDiskBootBlockParams *)buffer)->size);
    DPRINTF("sif_disk_boot_block %p %ld\n", ((struct SecrSifDiskBootBlockParams *)buffer)->source, ((struct SecrSifDiskBootBlockParams *)buffer)->result);

    return buffer;
}

static void SifSecrDownloadHeaderThread(void *parameters)
{
    (void)parameters;

    if (!sceSifCheckInit()) {
        DPRINTF("yet sif hasn't been init\n");
        sceSifInit();
    }

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&SifSecrDownloadHeaderQD, GetThreadId());
    sceSifRegisterRpc(&SifSecrDownloadHeaderData, SECRSIF_DOWNLOAD_HEADER, &SifSecrDownloadHeader, SifServerBuffer, NULL, NULL, &SifSecrDownloadHeaderQD);
    sceSifRpcLoop(&SifSecrDownloadHeaderQD);
}

static void SifSecrDownloadBlockThread(void *parameters)
{
    (void)parameters;

    if (!sceSifCheckInit()) {
        DPRINTF("yet sif hasn't been init\n");
        sceSifInit();
    }

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&SifSecrDownloadBlockQD, GetThreadId());
    sceSifRegisterRpc(&SifSecrDownloadBlockData, SECRSIF_DOWNLOAD_BLOCK, &SifSecrDownloadBlockHandler, SifServerBuffer, NULL, NULL, &SifSecrDownloadBlockQD);
    sceSifRpcLoop(&SifSecrDownloadBlockQD);
}

static void SifSecrDownloadGetKbitThread(void *parameters)
{
    (void)parameters;

    if (!sceSifCheckInit()) {
        DPRINTF("yet sif hasn't been init\n");
        sceSifInit();
    }

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&SifSecrDownloadGetKbitQD, GetThreadId());
    sceSifRegisterRpc(&SifSecrDownloadGetKbitData, SECRSIF_DOWNLOAD_GET_KBIT, &SifSecrDownloadGetKbitHandler, SifServerBuffer, NULL, NULL, &SifSecrDownloadGetKbitQD);
    sceSifRpcLoop(&SifSecrDownloadGetKbitQD);
}

static void SifSecrDownloadGetKcThread(void *parameters)
{
    (void)parameters;

    if (!sceSifCheckInit()) {
        DPRINTF("yet sif hasn't been init\n");
        sceSifInit();
    }

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&SifSecrDownloadGetKcQD, GetThreadId());
    sceSifRegisterRpc(&SifSecrDownloadGetKcData, SECRSIF_DOWNLOAD_GET_KC, &SifSecrDownloadGetKcHandler, SifServerBuffer, NULL, NULL, &SifSecrDownloadGetKcQD);
    sceSifRpcLoop(&SifSecrDownloadGetKcQD);
}

static void SifSecrDownloadGetICVPS2Thread(void *parameters)
{
    (void)parameters;

    if (!sceSifCheckInit()) {
        DPRINTF("yet sif hasn't been init\n");
        sceSifInit();
    }

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&SifSecrDownloadGetICVPS2QD, GetThreadId());
    sceSifRegisterRpc(&SifSecrDownloadGetICVPS2Data, SECRSIF_DOWNLOAD_GET_ICVPS2, &SifSecrDownloadGetICVPS2Handler, SifServerBuffer, NULL, NULL, &SifSecrDownloadGetICVPS2QD);
    sceSifRpcLoop(&SifSecrDownloadGetICVPS2QD);
}

static void SifSecrDiskBootHeaderThread(void *parameters)
{
    (void)parameters;

    if (!sceSifCheckInit()) {
        DPRINTF("yet sif hasn't been init\n");
        sceSifInit();
    }

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&SifSecrDiskBootHeaderQD, GetThreadId());
    sceSifRegisterRpc(&SifSecrDiskBootHeaderData, SECRSIF_DISK_BOOT_HEADER, &SifSecrDiskBootHeaderHandler, SifServerBuffer, NULL, NULL, &SifSecrDiskBootHeaderQD);
    sceSifRpcLoop(&SifSecrDiskBootHeaderQD);
}

static void SifSecrDiskBootBlockThread(void *parameters)
{
    (void)parameters;

    if (!sceSifCheckInit()) {
        DPRINTF("yet sif hasn't been init\n");
        sceSifInit();
    }

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&SifSecrDiskBootBlockQD, GetThreadId());
    sceSifRegisterRpc(&SifSecrDiskBootBlockData, SECRSIF_DISK_BOOT_BLOCK, &SifSecrDiskBootBlockHandler, SifServerBuffer, NULL, NULL, &SifSecrDiskBootBlockQD);
    sceSifRpcLoop(&SifSecrDiskBootBlockQD);
}

int _start(int argc, char *argv[])
{
    iop_thread_t thread;

    (void)argc;
    (void)argv;

    if (RegisterLibraryEntries(&_exp_secrsif) != 0) {
        return MODULE_NO_RESIDENT_END;
    }

    thread.attr      = TH_C;
    thread.priority  = 0x28;
    thread.stacksize = 0x800;

    thread.thread = &SifSecrDownloadHeaderThread;
    if ((SifSecrDownloadHeaderThreadID = CreateThread(&thread)) == 0) {
        return MODULE_NO_RESIDENT_END;
    }

    StartThread(SifSecrDownloadHeaderThreadID, NULL);

    thread.thread = &SifSecrDownloadBlockThread;
    if ((SifSecrDownloadBlockThreadID = CreateThread(&thread)) == 0) {
        return MODULE_NO_RESIDENT_END;
    }

    StartThread(SifSecrDownloadBlockThreadID, NULL);

    thread.thread = &SifSecrDownloadGetKbitThread;
    if ((SifSecrDownloadGetKbitThreadID = CreateThread(&thread)) == 0) {
        return MODULE_NO_RESIDENT_END;
    }

    StartThread(SifSecrDownloadGetKbitThreadID, NULL);

    thread.thread = &SifSecrDownloadGetKcThread;
    if ((SifSecrDownloadGetKcThreadID = CreateThread(&thread)) == 0) {
        return MODULE_NO_RESIDENT_END;
    }

    StartThread(SifSecrDownloadGetKcThreadID, NULL);

    thread.thread = &SifSecrDownloadGetICVPS2Thread;
    if ((SifSecrDownloadGetICVPS2ThreadID = CreateThread(&thread)) == 0) {
        return MODULE_NO_RESIDENT_END;
    }

    StartThread(SifSecrDownloadGetICVPS2ThreadID, NULL);

    thread.thread = &SifSecrDiskBootHeaderThread;
    if ((SifSecrDiskBootHeaderThreadID = CreateThread(&thread)) == 0) {
        return MODULE_NO_RESIDENT_END;
    }

    StartThread(SifSecrDiskBootHeaderThreadID, NULL);

    thread.thread = &SifSecrDiskBootBlockThread;
    if ((SifSecrDiskBootBlockThreadID = CreateThread(&thread)) == 0) {
        return MODULE_NO_RESIDENT_END;
    }

    StartThread(SifSecrDiskBootBlockThreadID, NULL);

    return MODULE_RESIDENT_END;
}
