/*
 * nprintf - IOP naplink RPC server v1.0
 *
 * Copyright (c) 2003 Tord Lindstrom <pukko@home.se>
 * Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
 *
 * This installs a naplink compatible RPC handler, to service the nprintf functions
 * given via naplink for printing from ee.
 *
 * Licensed under the AFL v2.0. See the file LICENSE included with this
 * distribution for licensing terms.
 */
#include "types.h"
#include "stdio.h"
#include "sysclib.h"
#include "thbase.h"
#include "intrman.h"
#include "sysmem.h"
#include "sifman.h"
#include "sifcmd.h"

////////////////////////////////////////////////////////////////////////
#define NPM_PUTS     0x01
#define RPC_NPM_USER 0x014d704e

/*! \brief RPC handler function.
 *  \ingroup fakehost 
 *
 *  \param  cmd    Command.
 *  \param  buffer Pointer to buffer.
 *  \param  size   Size of buffer.
 */
static void *naplinkRpcHandler(int cmd, void *buffer, int size)
{
    return buffer;
}

////////////////////////////////////////////////////////////////////////
static SifRpcServerData_t server __attribute((aligned(16)));
static SifRpcDataQueue_t  queue __attribute((aligned(16)));
static unsigned char rpc_buffer[512] __attribute((aligned(16)));

/*! \brief naplink compatbile RPC handler thread.
 *  \ingroup fakehost 
 *
 *  \param  arg Startup parameters.
 */
static void napThread(void *arg)
{
    int pid;

    SifInitRpc(0);
    pid = GetThreadId();
    SifSetRpcQueue(&queue, pid);
    SifRegisterRpc(&server, RPC_NPM_USER, naplinkRpcHandler,
                   rpc_buffer, 0, 0, &queue);
    SifRpcLoop(&queue);  // Never exits
    ExitDeleteThread();
}

/*! \brief Setup naplink compatible RPC handler.
 *  \ingroup fakehost 
 *
 *  \return Status.
 *
 *  return values:
 *    0 on success.
 *    -1 on error.
 */
int naplinkRpcInit(void)
{
    iop_thread_t th_attr;
    int ret;
    int pid;

    th_attr.attr = 0x02000000;
    th_attr.option = 0;
    th_attr.thread = napThread;
    th_attr.stacksize = 0x800;
    th_attr.priority = 0x4f;

    pid = CreateThread(&th_attr);
    if (pid < 0) {
        printf("IOP: napRpc createThread failed %d\n", pid);
        return -1;
    }

    ret = StartThread(pid, 0);
    if (ret < 0) {
        printf("IOP: napRpc startThread failed %d\n", ret);
        DeleteThread(pid);
        return -1;
    }
    return 0;
}
