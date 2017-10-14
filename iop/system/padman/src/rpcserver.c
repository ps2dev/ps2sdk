/*
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

/**
 * @file
 * IOP pad driver
 */

#include "types.h"
#include "sifcmd.h"
#include "thbase.h"
#include "stdio.h"
#include "sifman.h"
#include "sifcmd.h"
#include "freepad.h"

#define PAD_BIND_RPC_ID1 0x80000100
#define PAD_BIND_RPC_ID2 0x80000101

enum PAD_RPCCMD {
	PAD_RPCCMD_OPEN	= 0x01,
	// 0x2 undefined
	PAD_RPCCMD_INFO_ACT	= 0x03,
	PAD_RPCCMD_INFO_COMB,
	PAD_RPCCMD_INFO_MODE,
	PAD_RPCCMD_SET_MMODE,
	PAD_RPCCMD_SET_ACTDIR,
	PAD_RPCCMD_SET_ACTALIGN,
	PAD_RPCCMD_GET_BTNMASK,
	PAD_RPCCMD_SET_BTNINFO,
	PAD_RPCCMD_SET_VREF,
	PAD_RPCCMD_GET_PORTMAX,
	PAD_RPCCMD_GET_SLOTMAX,
	PAD_RPCCMD_CLOSE,
	PAD_RPCCMD_END,
	PAD_RPCCMD_INIT,
	// 0x11 undefined
	PAD_RPCCMD_GET_MODVER	= 0x12,
	PAD_RPCCMD_13
};

// RPC Server
static s32 ThreadIdRpcServer;
static SifRpcDataQueue_t qd;
static SifRpcServerData_t sd;
static u32 sb[32]; // server buffer

// RPC Server Extended
static s32 ThreadIdRpcServerExt;
static SifRpcDataQueue_t qdext;
static SifRpcServerData_t sdext;
static u32 sbext[32]; // server buffer

static void* RpcPadOpen(u32 *data)
{

	data[3] = padPortOpen(data[1], data[2], data[4], &data[5]);

	return data;
}

static void* RpcPadSetMainMode(u32 *data)
{
	data[5] = padSetMainMode(data[1], data[2], data[3], data[4]);

	return data;
}

static void* RpcPadInfoAct(u32 *data)
{

	data[5] = padInfoAct(data[1], data[2], data[3], data[4]);

	return data;
}

static void* RpcPadInfoComb(u32 *data)
{
	data[5] = padInfoComb(data[1], data[2], data[3], data[4]);

	return data;
}

static void* RpcPadInfoMode(u32 *data)
{
	data[5] = padInfoMode(data[1], data[2], data[3], data[4]);

	return data;
}

static void* RpcPadSetActDirect(u32 *data)
{
	data[5] = padSetActDirect(data[1], data[2], (u8*)&data[3]);

	return data;
}

static void* RpcPadSetActAlign(u32 *data)
{

	data[5] = padSetActAlign(data[1], data[2], (u8*)&data[3]);

	return data;
}

static void* RpcPadGetButtonMask(u32 *data)
{

	data[3] = padGetButtonMask(data[1], data[2]);

	return data;
}

static void* RpcPadSetButtonInfo(u32 *data)
{
	data[4] = padSetButtonInfo(data[1], data[2], data[3]);

	return data;
}

static void* RpcPadSetVrefParam(u32 *data)
{
	data[7] = padSetVrefParam(data[1], data[2], (u8*)&data[3]);

	return data;
}

static void* RpcPadGetPortMax(u32 *data)
{
	data[3] = padGetPortMax();

	return data;
}

static void* RpcPadGetSlotMax(u32 *data)
{
	data[3] = padGetSlotMax(data[1]);

	return data;
}

static void* RpcPadClose(u32 *data)
{
	data[3] = padPortClose(data[1], data[2], data[4]);

	return data;
}

static void* RpcPadEnd(u32 *data)
{
	data[3] = padEnd();

	return data;
}

static void* RpcPadInit(u32 *data)
{
	data[3] = padInit((void*)data[4]);

	return data;
}

static void* RpcGetModVersion(u32 *data)
{
	data[3] = padGetModVersion();

	return data;
}

static void* RpcServer(int fno, void *buffer, int length)
{
	u32 *data = (u32*)buffer;

	switch(data[0])
	{
		case PAD_RPCCMD_INIT:			return RpcPadInit(data);
		case PAD_RPCCMD_END:			return RpcPadEnd(data);
		case PAD_RPCCMD_GET_MODVER:		return RpcGetModVersion(data);
		case PAD_RPCCMD_OPEN:			return RpcPadOpen(data);
		case PAD_RPCCMD_CLOSE:			return RpcPadClose(data);
		case PAD_RPCCMD_INFO_ACT:		return RpcPadInfoAct(data);
		case PAD_RPCCMD_INFO_COMB:		return RpcPadInfoComb(data);
		case PAD_RPCCMD_INFO_MODE:		return RpcPadInfoMode(data);
		case PAD_RPCCMD_SET_MMODE:		return RpcPadSetMainMode(data);
		case PAD_RPCCMD_SET_ACTDIR:		return RpcPadSetActDirect(data);
		case PAD_RPCCMD_SET_ACTALIGN:	return RpcPadSetActAlign(data);
		case PAD_RPCCMD_GET_BTNMASK:	return RpcPadGetButtonMask(data);
		case PAD_RPCCMD_SET_BTNINFO:	return RpcPadSetButtonInfo(data);
		case PAD_RPCCMD_SET_VREF:		return RpcPadSetVrefParam(data);
		case PAD_RPCCMD_GET_PORTMAX:	return RpcPadGetPortMax(data);
		case PAD_RPCCMD_GET_SLOTMAX:	return RpcPadGetSlotMax(data);

		default:
			M_PRINTF("invalid function code (%03x)\n", (int)data[0]);
		break;
	}

	return buffer;
}

static void* RpcServerExt(int fno, void *buffer, int length)
{
	M_PRINTF("Extend Service: This service is not supported.\n");

	return buffer;
}


static void RpcThread(void *arg)
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd, GetThreadId());
	sceSifRegisterRpc(&sd, PAD_BIND_RPC_ID1, &RpcServer, sb, NULL, NULL, &qd);
	sceSifRpcLoop(&qd);
}

static void RpcThreadExt(void *arg)
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qdext, GetThreadId());
	sceSifRegisterRpc(&sdext, PAD_BIND_RPC_ID2, &RpcServerExt, sbext, NULL, NULL, &qdext);
	sceSifRpcLoop(&qdext);
}

int InitRpcServers(int prio)
{
	iop_thread_t rpc_thread;

	if(prio == 0)
		prio = PADMAN_THPRI_LO;

	// RPC Server
	rpc_thread.attr = TH_C;
	rpc_thread.thread = &RpcThread;
	rpc_thread.stacksize = 0x800;
	rpc_thread.priority = prio;

	ThreadIdRpcServer = CreateThread(&rpc_thread);

	if(ThreadIdRpcServer == 0) return 0;

	StartThread(ThreadIdRpcServer, NULL);

	// RPC Server Extended
	rpc_thread.attr = TH_C;
	rpc_thread.thread = &RpcThreadExt;
	rpc_thread.stacksize = 0x800;
	rpc_thread.priority = prio;

	ThreadIdRpcServerExt = CreateThread(&rpc_thread);

	if(ThreadIdRpcServerExt == 0) return 0;

	StartThread(ThreadIdRpcServerExt, NULL);

	return 1;
}
