#include <stdio.h>
#include <thbase.h>
#include <thsemap.h>
#include <loadcore.h>
#include <sysmem.h>
#include <intrman.h>
#include <sifcmd.h>
#include <libsd.h>
#include <sysclib.h>

#include <audsrv.h>
#include "audsrv_internal.h"
#include "rpc_client.h"

static SifRpcClientData_t client;
static int rpc_sema;

void initialize_rpc_client(void)
{
	rpc_sema = CreateMutex(IOP_MUTEX_UNLOCKED);

	client.server = NULL;
	while(sceSifBindRpc(&client, AUDSRV_IRX, 0) < 0 || client.server == NULL) DelayThread(500);
}

void deinitialize_rpc_client(void)
{
	memset(&client, 0, sizeof(client));
	DeleteSema(rpc_sema);
}

static void call_end_callback(void *end_param)
{
	iSignalSema(rpc_sema);
}

void call_client_callback(int id)
{
	WaitSema(rpc_sema);
	sceSifCallRpc(&client, id, SIF_RPC_M_NOWAIT, NULL, 0, NULL, 0, &call_end_callback, NULL);
}
