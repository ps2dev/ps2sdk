/*
 * freepad - IOP pad driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
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

#define PAD_RPCCMD_OPEN         0x01
// 0x2 undefined
#define PAD_RPCCMD_INFO_ACT		0x03
#define PAD_RPCCMD_INFO_COMB	0x04
#define PAD_RPCCMD_INFO_MODE	0x05
#define PAD_RPCCMD_SET_MMODE    0x06
#define PAD_RPCCMD_SET_ACTDIR   0x07
#define PAD_RPCCMD_SET_ACTALIGN 0x08
#define PAD_RPCCMD_GET_BTNMASK  0x09
#define PAD_RPCCMD_SET_BTNINFO  0x0A
#define PAD_RPCCMD_SET_VREF     0x0B
#define PAD_RPCCMD_GET_PORTMAX  0x0C
#define PAD_RPCCMD_GET_SLOTMAX  0x0D
#define PAD_RPCCMD_CLOSE        0x0E
#define PAD_RPCCMD_END          0x0F
#define PAD_RPCCMD_INIT         0x10
// 0x11 undefined
#define PAD_RPCCMD_GET_MODVER   0x12

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

void* RpcPadOpen(u32 *data)
{

	data[3] = padPortOpen(data[1], data[2], data[4], &data[5]);

	return data;
}

void* RpcPadSetMainMode(u32 *data)
{
	data[5] = padSetMainMode(data[1], data[2], data[3], data[4]);
	
	return data;
}

void* RpcPadInfoAct(u32 *data)
{

	data[5] = padInfoAct(data[1], data[2], data[3], data[4]);

	return data;
}

void* RpcPadInfoComb(u32 *data)
{
	data[5] = padInfoComb(data[1], data[2], data[3], data[4]);

	return data;
}

void* RpcPadInfoMode(u32 *data)
{
	data[5] = padInfoMode(data[1], data[2], data[3], data[4]);

	return data;
}

void* RpcPadSetActDirect(u32 *data)
{
	data[5] = padSetActDirect(data[1], data[2], (u8*)&data[3]);

	return data;
}

void* RpcPadSetActAlign(u32 *data)
{

	data[5] = padSetActAlign(data[1], data[2], (u8*)&data[3]);

	return data;
}

void* RpcPadGetButtonMask(u32 *data)
{

	data[3] = padGetButtonMask(data[1], data[2]);

	return data;
}

void* RpcPadSetButtonInfo(u32 *data)
{
	data[4] = padSetButtonInfo(data[1], data[2], data[3]);

	return data;
}

void* RpcPadSetVrefParam(u32 *data)
{
	data[7] = padSetVrefParam(data[1], data[2], (u8*)&data[3]);

	return data;
}

void* RpcPadGetPortMax(u32 *data)
{
	data[3] = padGetPortMax();
	
	return data;
}

void* RpcPadGetSlotMax(u32 *data)
{
	data[3] = padGetSlotMax(data[1]);
	
	return data;
}

void* RpcPadClose(u32 *data)
{
	data[3] = padPortClose(data[1], data[2], data[4]);

	return data;
}

void* RpcPadEnd(u32 *data)
{
	data[3] = padEnd();

	return data;
}

void* RpcPadInit(u32 *data)
{
	data[3] = padInit((void*)data[4]);

	return data;
}

void* RpcGetModVersion(u32 *data)
{
	data[3] = padGetModVersion();
	
	return data;
}

void* RpcServer(s32 fno, u32 *data, s32 size)
{
	switch(data[0])
	{
		case PAD_RPCCMD_OPEN:			return RpcPadOpen(data);
		case PAD_RPCCMD_SET_MMODE:		return RpcPadSetMainMode(data);
		case PAD_RPCCMD_INFO_ACT:		return RpcPadInfoAct(data);
		case PAD_RPCCMD_INFO_COMB:		return RpcPadInfoComb(data);
		case PAD_RPCCMD_INFO_MODE:		return RpcPadInfoMode(data);
		case PAD_RPCCMD_SET_ACTDIR:		return RpcPadSetActDirect(data);
		case PAD_RPCCMD_SET_ACTALIGN:	return RpcPadSetActAlign(data);
		case PAD_RPCCMD_GET_BTNMASK:	return RpcPadGetButtonMask(data);
		case PAD_RPCCMD_SET_BTNINFO:	return RpcPadSetButtonInfo(data);
		case PAD_RPCCMD_SET_VREF:		return RpcPadSetVrefParam(data);
		case PAD_RPCCMD_GET_PORTMAX:	return RpcPadGetPortMax(data);
		case PAD_RPCCMD_GET_SLOTMAX:	return RpcPadGetSlotMax(data);
		case PAD_RPCCMD_CLOSE:			return RpcPadClose(data);	
		case PAD_RPCCMD_END:			return RpcPadEnd(data);
		case PAD_RPCCMD_INIT:			return RpcPadInit(data);
		case PAD_RPCCMD_GET_MODVER:		return RpcGetModVersion(data);	
		
		default:
			M_PRINTF("RpcServer, invalid function code (%i).\n", (int)fno);
		break;
	}

	return data;
}

void* RpcServerExt(s32 fno, u32 *data, s32 size)
{
	M_PRINTF("Extend Service: This serviced is not supported.\n");

	return data;
}


void RpcThread()
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();	
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qd, GetThreadId());	
	sceSifRegisterRpc(&sd, PAD_BIND_RPC_ID1, (void*)RpcServer, sb, 0, 0, &qd);
	sceSifRpcLoop(&qd);
}

void RpcThreadExt()
{
	if( sceSifCheckInit() == 0)
	{
		M_PRINTF("Sif not initialized.\n");
		sceSifInit();	
	}

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&qdext, GetThreadId());	
	sceSifRegisterRpc(&sdext, PAD_BIND_RPC_ID2, (void*)RpcServerExt, sbext, 0, 0, &qdext);
	sceSifRpcLoop(&qdext);
}


u32 InitRpcServers()
{
	iop_thread_t rpc_thread;
	iop_thread_t rpc_threadext;

	// RPC Server
	rpc_thread.attr = TH_C;
	rpc_thread.option = 0;
	rpc_thread.thread = RpcThread;
	rpc_thread.stacksize = 0x800;
	rpc_thread.priority = 20;

	ThreadIdRpcServer = CreateThread(&rpc_thread);
	
	if(ThreadIdRpcServer == 0) return 1;

	StartThread(ThreadIdRpcServer, NULL);

	// RPC Server Extended
	rpc_threadext.attr = TH_C;
	rpc_threadext.option = 0;
	rpc_threadext.thread = RpcThreadExt;
	rpc_threadext.stacksize = 0x800;
	rpc_threadext.priority = 20;

	ThreadIdRpcServerExt = CreateThread(&rpc_threadext);

	if(ThreadIdRpcServerExt == 0) return 1;

	StartThread(ThreadIdRpcServerExt, NULL);
	
	return 0;
}
