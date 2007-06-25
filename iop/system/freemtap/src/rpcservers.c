/*
 * freemtap - IOP multitap driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "irx.h"
#include "stdio.h"
#include "freemtap.h"
#include "sifman.h"
#include "sifcmd.h"
#include "thbase.h"

#define MTAPSERV_PORT_OPEN			0x80000901
#define MTAPSERV_PORT_CLOSE			0x80000902
#define MTAPSERV_GET_CONNECTION		0x80000903
#define MTAPSERV_GET_SLOT_NUMBER	0x800009FE
#define MTAPSERV_CHANGE_SLOT		0x800009FF


static u32 sb[4]; // Server buffer

static s32 threadid_rpc1;
static SifRpcDataQueue_t qd1;
static SifRpcServerData_t sd1;

static s32 threadid_rpc2;
static SifRpcDataQueue_t qd2;
static SifRpcServerData_t sd2;

static s32 threadid_rpc3;
static SifRpcDataQueue_t qd3;
static SifRpcServerData_t sd3;

static s32 threadid_rpc4;
static SifRpcDataQueue_t qd4;
static SifRpcServerData_t sd4;

static s32 threadid_rpc5;
static SifRpcDataQueue_t qd5;
static SifRpcServerData_t sd5;


void* rpc_server_change_slot(s32 fno, u32 *data, s32 size)
{
	data[1] = mtapChangeSlot( data[0], data[1] );

	return data;
}


void rpc_thread_change_slot()
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();	
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd5, GetThreadId());	
	sceSifRegisterRpc(&sd5, MTAPSERV_CHANGE_SLOT, (void*)rpc_server_change_slot, sb, 0, 0, &qd5);
	sceSifRpcLoop(&qd5);
}


void* rpc_server_get_slot_number(s32 fno, u32 *data, s32 size)
{
	data[1] = mtapGetSlotNumber( data[0] );

	return data;
}


void rpc_thread_get_slot_number()
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();	
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd4, GetThreadId());	
	sceSifRegisterRpc(&sd4, MTAPSERV_GET_SLOT_NUMBER, (void*)rpc_server_get_slot_number, sb, 0, 0, &qd4);
	sceSifRpcLoop(&qd4);
}

void* RpcServerGetConnection(s32 fno, u32 *data, s32 size)
{
	data[1] = mtapGetConnection( data[0] );

	return data;
}

void RpcThreadGetConnection()
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();	
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd3, GetThreadId());	
	sceSifRegisterRpc(&sd3, MTAPSERV_GET_CONNECTION, (void*)RpcServerGetConnection, sb, 0, 0, &qd3);
	sceSifRpcLoop(&qd3);
}

void* RpcServerPortClose(s32 fno, u32 *data, s32 size)
{
	data[1] = mtapPortClose( data[0] );

	return data;
}

void RpcThreadPortClose()
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();	
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd2, GetThreadId());	
	sceSifRegisterRpc(&sd2, MTAPSERV_PORT_CLOSE, (void*)RpcServerPortClose, sb, 0, 0, &qd2);
	sceSifRpcLoop(&qd2);
}

void* RpcServerPortOpen(s32 fno, u32 *data, s32 size)
{

	data[1] = mtapPortOpen( data[0] );

	return data;
}

void RpcThreadPortOpen()
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();	
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd1, GetThreadId());	
	sceSifRegisterRpc(&sd1, MTAPSERV_PORT_OPEN, (void*)RpcServerPortOpen, sb, 0, 0, &qd1);
	sceSifRpcLoop(&qd1);

}





// You should *not* setup 1 RPC server pr. function. Unfortunatly, this
// is what XMTAPMAN does, so we have to do the same to be compatible.
s32 InitRpcServers()
{
	iop_thread_t rpc_thread;
	
	// mtapPortOpen RPC Server
	rpc_thread.attr = TH_C;
	rpc_thread.thread = RpcThreadPortOpen;
	rpc_thread.stacksize = 0x2000;
	rpc_thread.priority = 32;

	threadid_rpc1 = CreateThread(&rpc_thread);
	
	if(threadid_rpc1 == 0) return 0;

	StartThread(threadid_rpc1, 0);

	// mtapPortClose RPC Server
	rpc_thread.thread = RpcThreadPortClose;
	
	threadid_rpc2 = CreateThread(&rpc_thread);

	if(threadid_rpc2 == 0) return 0;
	
	StartThread(threadid_rpc2, 0);

	// mtapGetConnection RPC Server
	rpc_thread.thread = RpcThreadGetConnection;
	
	threadid_rpc3 = CreateThread(&rpc_thread);

	if(threadid_rpc3 == 0) return 0;
	
	StartThread(threadid_rpc3, 0);

	// mtapGetSlotNumber RPC Server
	rpc_thread.thread = rpc_thread_get_slot_number;
	
	threadid_rpc4 = CreateThread(&rpc_thread);

	if(threadid_rpc4 == 0) return 0;
	
	StartThread(threadid_rpc4, 0);

	// mtapChangeSlot RPC Server
	rpc_thread.thread = rpc_thread_change_slot;
	
	threadid_rpc5 = CreateThread(&rpc_thread);

	if(threadid_rpc5 == 0) return 0;
	
	StartThread(threadid_rpc5, 0);

	return 1;
}
