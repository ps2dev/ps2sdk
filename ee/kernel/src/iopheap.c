/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2002, Vzzrzzn
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "tamtypes.h"
#include "ps2lib_err.h"
#include "kernel.h"
#include "sifrpc.h"
#include "sifcmd.h"
#include "string.h"
#include <iopcontrol.h>
#include "iopheap-common.h"

#include "iopheap.h"

extern SifRpcClientData_t _ih_cd;

#ifdef F_SifInitIopHeap
SifRpcClientData_t _ih_cd;

int SifInitIopHeap()
{
    int res;
    if (HasIopRebootedSinceLastCall())
        SifExitIopHeap();

    if (_ih_cd.server)
        return 0;

    sceSifInitRpc(0);

    while ((res = sceSifBindRpc(&_ih_cd, 0x80000003, 0)) >= 0 && !_ih_cd.server)
        nopdelay();

    if (res < 0)
        return -E_SIF_RPC_BIND;

    return 0;
}
#endif

#ifdef F_SifExitIopHeap
void SifExitIopHeap()
{
    memset(&_ih_cd, 0, sizeof _ih_cd);
}
#endif

#ifdef F_SifAllocIopHeap
void *SifAllocIopHeap(int size)
{
    union
    {
        int size;
        u32 addr;
    } arg;

    if (SifInitIopHeap() < 0)
        return NULL;

    arg.size = size;

    if (sceSifCallRpc(&_ih_cd, 1, 0, &arg, 4, &arg, 4, NULL, NULL) < 0)
        return NULL;

    return (void *)arg.addr;
}
#endif

#ifdef F_SifFreeIopHeap
int SifFreeIopHeap(void *addr)
{
    union
    {
        void *addr;
        int result;
    } arg;

    if (SifInitIopHeap() < 0)
        return -E_LIB_API_INIT;

    arg.addr = addr;

    if (sceSifCallRpc(&_ih_cd, 2, 0, &arg, 4, &arg, 4, NULL, NULL) < 0)
        return -E_SIF_RPC_CALL;

    return arg.result;
}
#endif

#ifdef F_SifLoadIopHeap
int SifLoadIopHeap(const char *path, void *addr)
{
    struct _iop_load_heap_arg arg;

    if (SifInitIopHeap() < 0)
        return -E_LIB_API_INIT;

    arg.p.addr = addr;
    strncpy(arg.path, path, LIH_PATH_MAX - 1);
    arg.path[LIH_PATH_MAX - 1] = 0;

    if (sceSifCallRpc(&_ih_cd, 3, 0, &arg, sizeof arg, &arg, 4, NULL, NULL) < 0)
        return -E_SIF_RPC_CALL;

    return arg.p.result;
}
#endif
