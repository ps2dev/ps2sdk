#include <intrman.h>
#include <loadcore.h>
#include <sifcmd.h>
#include <sifman.h>
#include <stdio.h>
#include <thbase.h>
#include <thevent.h>
#include <vblank.h>
#ifdef BUILDING_RMMAN2
#ifdef BUILDING_RMMANX
#include <iomanX.h>
#else
#include <cdvdman.h>
#endif
#else
#include <rsio2man.h>
#endif
#include <irx.h>

#include "rmman.h"

#ifdef BUILDING_RMMAN2
#ifdef BUILDING_RMMANX
IRX_ID("rmmanx", 2, 4);
// Based off the module from XOSD 2.14
#else
IRX_ID("rmman2", 2, 4);
// Based off the module from DVD Player 3.10
#endif
#else
IRX_ID("rmman", 1, 16);
#endif

#ifdef DEBUG
#define DPRINTF(x...) printf("RMMAN: "x)
#else
#define DPRINTF(x...)
#endif

#define RM_EF_EXIT_THREAD		1
#define RM_EF_EXIT_THREAD_DONE		2
#define RM_EF_CLOSE_PORT		4
#define RM_EF_CLOSE_PORT_DONE		8

#ifdef BUILDING_RMMAN2
#define RM_MAX_PORT 1
#define RM_MAX_SLOT 1
#define RM_EE_DATA_TYPE struct rmEEData2
#define RM_RPCFUNC_END RMMAN2_RPCFUNC_END
#define RM_RPCFUNC_INIT RMMAN2_RPCFUNC_INIT
#define RM_RPCFUNC_CLOSE RMMAN2_RPCFUNC_CLOSE
#define RM_RPCFUNC_OPEN RMMAN2_RPCFUNC_OPEN
#define RM_RPCFUNC_VERSION RMMAN2_RPCFUNC_VERSION
#define RM_RPCFUNC_REMOTE2_6 RMMAN2_RPCFUNC_REMOTE2_6
#ifdef BUILDING_RMMANX
#define RM_RPC_ID RMMANX_RPC_ID
#else
#define RM_RPC_ID RMMAN2_RPC_ID
#endif
#define RM_PACKET_CMD(packet) (packet->cmd.u.cmd2)
#else
#define RM_MAX_PORT 2
#define RM_MAX_SLOT 4
#define RM_EE_DATA_TYPE struct rmEEData
#define RM_RPCFUNC_END RMMAN_RPCFUNC_END
#define RM_RPCFUNC_INIT RMMAN_RPCFUNC_INIT
#define RM_RPCFUNC_CLOSE RMMAN_RPCFUNC_CLOSE
#define RM_RPCFUNC_OPEN RMMAN_RPCFUNC_OPEN
#define RM_RPCFUNC_VERSION RMMAN_RPCFUNC_VERSION
#define RM_RPC_ID RMMAN_RPC_ID
#define RM_PACKET_CMD(packet) (packet->cmd.u.cmd1)
#endif

enum RM_TASK {
	RM_TASK_QUERY = 0,
	RM_TASK_INIT,
	RM_TASK_POLL
};

struct RmData {
	RM_EE_DATA_TYPE eeData;
#ifndef BUILDING_RMMAN2
	u32 unused1;
	u8 inBuffer[32];
#endif
	u8 outBuffer[32];
	u32 state;
	u32 reqState;
	u32 frame;
	RM_EE_DATA_TYPE *eeBuffer;
#ifndef BUILDING_RMMAN2
	s32 port;
	s32 slot;
	u32 currentTask;
	u32 counter;
	u32 powerMode;
	u32 closed;
#endif
	u32 connected;
#ifndef BUILDING_RMMAN2
	u32 eventFlagID;
	u32 unused2;
	sio2_transfer_data_t sio2Data;
#endif
};

#ifdef BUILDING_RMMAN2
#ifdef BUILDING_RMMANX
extern struct irx_export_table _exp_rmmanx;
#else
extern struct irx_export_table _exp_rmman2;
#endif
#else
extern struct irx_export_table _exp_rmman;
#endif

static struct RmData RmData[RM_MAX_PORT][RM_MAX_SLOT];
static int portStatus[RM_MAX_PORT];
static int eventFlagID;
static int MainThreadID;
static int IsInitialized;
static int RpcThreadID;
static SifRpcDataQueue_t RpcDataQueue;
static SifRpcServerData_t RpcServerData;
static struct rmRpcPacket RpcDataBuffer __attribute__((__aligned__(4)));

static int CreateMainThread(void);
static void MainThread(void *arg);
static int HandleTasks(struct RmData *RmData);
#ifndef BUILDING_RMMAN2
static int FindRemote(struct RmData *RmData);
static int InitFindRmCmd(struct RmData *RmData);
#endif
static int PollRemote(struct RmData *RmData);
#ifndef BUILDING_RMMAN2
static int InitPollRmCmd(struct RmData *RmData);
static int RmExecute(struct RmData *RmData);
#endif
static int DmaSendEE(struct RmData *RmData);
#ifndef BUILDING_RMMAN2
static int HandleRmTaskFailed(struct RmData *RmData);
static int InitRemote(struct RmData *RmData);
static int InitInitRmCmd(struct RmData *RmData);
#endif

int _start(int argc, char *argv[])
{
	int result;
	struct irx_export_table *export_table;

	(void)argc;
	(void)argv;

#ifdef BUILDING_RMMAN2
#ifdef BUILDING_RMMANX
	export_table = &_exp_rmmanx;
#else
	export_table = &_exp_rmman2;
#endif
#else
	export_table = &_exp_rmman;
#endif

	if(RegisterLibraryEntries(export_table) == 0)
	{
		result = CreateMainThread() <= 0 ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END;
	}
	else result = MODULE_NO_RESIDENT_END;

	return result;
}

int rmmanInit(void)
{
	iop_event_t EventFlagData;
	iop_thread_t ThreadData;
	int result;
	int i;

	for (i = 0; i < RM_MAX_PORT; i += 1)
	{
		portStatus[i] = 0;
	}
	IsInitialized = 1;
	EventFlagData.attr = 2;
	EventFlagData.bits = 0;
	if((eventFlagID = CreateEventFlag(&EventFlagData)) != 0)
	{
		ThreadData.attr = TH_C;
		ThreadData.thread = &MainThread;
		ThreadData.priority = 0x2E;
		ThreadData.stacksize = 0x800;
		if((MainThreadID = CreateThread(&ThreadData)) != 0)
		{
			StartThread(MainThreadID, NULL);
			result=MainThreadID;
		}
		else result=0;
	}
	else result=0;

	return result;
}

int rmmanOpen(int port, int slot, void *buffer)
{
	int result;

	if ((port >= RM_MAX_PORT) || (slot >= RM_MAX_SLOT))
	{
		return 0;
	}

	if(!((portStatus[port] >> slot) & 1))
	{
		struct RmData *pRmData;
#ifndef BUILDING_RMMAN2
		iop_event_t evf;
#endif

		pRmData = &RmData[port][slot];
#ifndef BUILDING_RMMAN2
		pRmData->port = port;
		pRmData->slot = slot;
#endif
		pRmData->state = RM_STATE_EXECCMD;
		pRmData->reqState = RM_RSTATE_COMPLETE;
		pRmData->eeBuffer = buffer;
#ifndef BUILDING_RMMAN2
		pRmData->currentTask = RM_TASK_QUERY;
		pRmData->counter = 0;
#endif

#ifndef BUILDING_RMMAN2
		evf.attr = EA_MULTI;
		evf.bits = 0;

		if((pRmData->eventFlagID = CreateEventFlag(&evf)) != 0)
		{
			pRmData->powerMode = 0;
			portStatus[port] |= (1 << slot);
			result = 1;
		} else
			result = 0;
#else
		result = 1;
#endif
	} else
		result = 0;

	return result;
}

int rmmanClose(int port, int slot)
{
	int result;

	if ((port >= RM_MAX_PORT) || (slot >= RM_MAX_SLOT))
	{
		return 0;
	}

	if((portStatus[port] >> slot) & 1)
	{
#ifndef BUILDING_RMMAN2
		struct RmData *pRmData;
		u32 bits;

		pRmData = &RmData[port][slot];

		if(pRmData->state != RM_STATE_FINDRM)
		{
			SetEventFlag(pRmData->eventFlagID, RM_EF_EXIT_THREAD);
			WaitEventFlag(pRmData->eventFlagID, RM_EF_EXIT_THREAD_DONE, WEF_AND|WEF_CLEAR, &bits);

			if(pRmData->closed != 0)
			{	//Remote Control port was closed successfully.
				portStatus[port] ^= (1 << slot);
				result = 1;
			} else {
				result = 0;
			}
		} else {
			portStatus[port] ^= (1 << slot);
			result = 1;
		}
#else
		portStatus[port] ^= (1 << slot);
		result = 1;
#endif
	} else
		result = 0;

	return result;
}

int rmmanEnd(void)
{
	u32 bits;
	int result;

	if(IsInitialized != 0)
	{
		int port;
		for(port = 0; port < RM_MAX_PORT; port++)
		{
			int slot;
			for(slot = 0; slot < RM_MAX_SLOT; slot++)
			{	//If port,slot is opened, close it.
				if((portStatus[port] >> slot) & 1)
					rmmanClose(port, slot);
			}
		}

		SetEventFlag(eventFlagID, RM_EF_EXIT_THREAD);
		WaitEventFlag(eventFlagID, RM_EF_EXIT_THREAD_DONE, WEF_AND|WEF_CLEAR, &bits);
		DeleteEventFlag(eventFlagID);
		eventFlagID = 0;
		if(DeleteThread(eventFlagID) == 0)
		{
			MainThreadID = 0;
			IsInitialized = 0;

			result = 1;
		} else
			result = 0;
	} else
		result = 1;

	return result;
}

static void MainThread(void *arg)
{
	iop_event_info_t evfInfo;
	int port, slot;

	(void)arg;

	while(1)
	{
		WaitVblankStart();

		ReferEventFlagStatus(eventFlagID, &evfInfo);

		if(evfInfo.currBits == RM_EF_EXIT_THREAD)
		{
			SetEventFlag(eventFlagID, RM_EF_EXIT_THREAD_DONE);
			ExitThread();
		}

		for(port = 0; port < RM_MAX_PORT; port++)
		{
			for(slot = 0; slot < RM_MAX_SLOT; slot++)
			{
				if((portStatus[port] >> slot) & 1)
				{
#ifndef BUILDING_RMMAN2
					ReferEventFlagStatus(RmData[port][slot].eventFlagID, &evfInfo);

					if(evfInfo.currBits == RM_EF_CLOSE_PORT)
					{
						RmData[port][slot].powerMode = 3;
						if(InitRemote(&RmData[port][slot]) == 0)
							RmData[port][slot].closed = 0;
						else
							RmData[port][slot].closed = 1;

						SetEventFlag(RmData[port][slot].eventFlagID, RM_EF_CLOSE_PORT_DONE);
					} else {
						HandleTasks(&RmData[port][slot]);
					}
#else
					HandleTasks(&RmData[port][slot]);
#endif
				}
			}
		}
	}
}

static int HandleTasks(struct RmData *RmData)
{
#ifndef BUILDING_RMMAN2
	switch(RmData->currentTask)
	{
		case RM_TASK_QUERY:
			if(RmData->counter == 0 && FindRemote(RmData) != 0)
			{
				RmData->counter = 0;
				RmData->state = RM_STATE_EXECCMD;
				RmData->currentTask++;
			 } else {
				 if(RmData->counter < 10)
				 {
					 RmData->counter++;
					 RmData->state = RM_STATE_FINDRM;
				 } else {
					 RmData->counter = 0;
				 }
			 }
			break;
		case RM_TASK_INIT:
			if(InitRemote(RmData) != 0)
			{
				RmData->counter = 0;
				RmData->currentTask++;
			} else {
				HandleRmTaskFailed(RmData);
			}
			break;
		case RM_TASK_POLL:
			RmData->state = RM_STATE_STABLE;
			if(PollRemote(RmData) != 0)
			{
				RmData->counter = 0;

				if(RmData->reqState == RM_RSTATE_BUSY)
					RmData->reqState = RM_RSTATE_COMPLETE;
			} else {
				if(HandleRmTaskFailed(RmData) == 0 && RmData->reqState == RM_RSTATE_BUSY)
					RmData->reqState = RM_RSTATE_FAILED;
			}
			break;
	}
#else
	RmData->connected = PollRemote(RmData);
#endif

	DmaSendEE(RmData);

	return 1;
}

#ifndef BUILDING_RMMAN2
static int FindRemote(struct RmData *RmData)
{
	int result;

	InitFindRmCmd(RmData);
	if(RmExecute(RmData) != 0)
	{
		if(RmData->port < RM_MAX_PORT)
		{
			switch(RmData->outBuffer[1])
			{
				case 0x12:
					result = 1;
					break;
				case 0x1F:
					InitRemote(RmData);
					result = 1;
					break;
				default:
					RmData->connected = 0;
					result = 0;
			}
		} else
			result = 1;
	} else
		result = 0;

	return result;
}

static int InitFindRmCmd(struct RmData *RmData)
{
	int i;

	//The original was setting these, byte by byte.
	RmData->sio2Data.port_ctrl1[RmData->port] = 0xFFC0050F;
	RmData->sio2Data.port_ctrl2[RmData->port] = 0x00060014;

	RmData->sio2Data.port_ctrl2[RmData->port] = (RmData->sio2Data.port_ctrl2[RmData->port] & ~0x03000000) | 0x01000000;

	RmData->sio2Data.in = RmData->inBuffer;
	RmData->sio2Data.out = RmData->outBuffer;
	RmData->inBuffer[0] = 0x61;
	RmData->sio2Data.in_size = 7;
	RmData->sio2Data.out_size = 7;
	RmData->sio2Data.regdata[1] = 0;
	RmData->inBuffer[1] = 0x0F;

	RmData->sio2Data.regdata[0] = (RmData->sio2Data.regdata[0] & ~3) | (RmData->port & 3);
	RmData->sio2Data.regdata[0] = (RmData->sio2Data.regdata[0] & ~0x000000C0) | 0x40;
	RmData->sio2Data.regdata[0] = (RmData->sio2Data.regdata[0] & ~0x0001FF00) | 0x00700;
	RmData->sio2Data.regdata[0] = (RmData->sio2Data.regdata[0] & ~0x07FC0000) | 0x001C0000;
	
	for(i = 2; i < 7; i++)
		RmData->inBuffer[i] = 0;

	return 1;
}
#endif

static int PollRemote(struct RmData *RmData)
{
#ifndef BUILDING_RMMAN2
	InitPollRmCmd(RmData);
	return(RmExecute(RmData) > 0);
#else
	int i;
	char rmbuf[16];

#ifdef BUILDING_RMMANX
	if (iomanX_devctl("dvr_misc:", 0x5668, 0, 0, rmbuf, 0xA) < 0)
#else
	if (sceCdApplySCmd(0x1E, 0, 0, rmbuf) && (rmbuf[0] & 0x80) != 0)
#endif
	{
		RmData->state = RM_STATE_DISCONN;
		return 0;
	}
#ifdef BUILDING_RMMANX
	RmData->outBuffer[0] = rmbuf[0];
	for (i = 1; i < 4; i += 1)
	{
		RmData->outBuffer[i] = rmbuf[(i - 1) * 2];
	}
	RmData->outBuffer[4] = rmbuf[8];
#else
	for (i = 1; i < 4; i += 1)
	{
		RmData->outBuffer[(i - 1)] = rmbuf[i];
	}
#endif
	RmData->state = RM_STATE_STABLE;
	return 1;
#endif
}

#ifndef BUILDING_RMMAN2
static int InitPollRmCmd(struct RmData *RmData)
{
	int i;

	RmData->inBuffer[0] = 0x61;
	RmData->inBuffer[1] = 0x04;
	for(i = 2; i < 7; i++)
		RmData->inBuffer[i] = 0;

	return 1;
}

static int RmExecute(struct RmData *RmData)
{
	int result;

	sio2_rm_transfer_init();
	sio2_transfer(&RmData->sio2Data);
	sio2_transfer_reset();

	if(((RmData->sio2Data.stat6c >> 14) & 3) == 0)
	{
		RmData->connected = 1;
		result = 1;
	} else {
		RmData->connected = 0;
		result = 0;
	}

	return result;
}

static int HandleRmTaskFailed(struct RmData *RmData)
{
	if(RmData->counter + 1 < 10)
	{
		RmData->counter++;
		return RmData->counter;
	} else {
		RmData->currentTask = RM_TASK_QUERY;
		return 0;
	}
}
#endif

static int DmaSendEE(struct RmData *RmData)
{
	SifDmaTransfer_t dmat;
	int i, OldState, dmatID;

	RmData->eeData.frame = RmData->frame;
	RmData->frame++;
	for(i = 0; (unsigned int)i < sizeof(RmData->eeData.data); i++)
		RmData->eeData.data[i] = RmData->outBuffer[i];

	RmData->eeData.state = RmData->state;
	RmData->eeData.connected = RmData->connected;

	dmat.src = &RmData->eeData;
	dmat.dest = ((RmData->frame & 1) == 0) ? (u8*)RmData->eeBuffer : (u8*)RmData->eeBuffer + 128;
	dmat.size = 128;
	dmat.attr = 0;

	CpuSuspendIntr(&OldState);
	dmatID = sceSifSetDma(&dmat, 1);
	CpuResumeIntr(OldState);

	return(dmatID > 0);
}

#ifndef BUILDING_RMMAN2
static int InitRemote(struct RmData *RmData)
{
	InitInitRmCmd(RmData);
	return(RmExecute(RmData) > 0);
}

static int InitInitRmCmd(struct RmData *RmData)
{
	int i;

	RmData->inBuffer[0] = 0x61;
	RmData->inBuffer[1] = 0x06;
	RmData->inBuffer[2] = (u8)RmData->powerMode;

	for(i = 3; i < 7; i++)
		RmData->inBuffer[i] = 0;

	return 1;
}
#endif

static int rmmanVersion(void)
{
	return _irx_id.v;
}

#ifdef BUILDING_RMMAN2
static int rmmanRemote2_6(u8 *res)
{
#ifdef BUILDING_RMMANX
	*res = 1;
	return 1;
#else
	char scmdbuf[16];

	if (sceCdApplySCmd(0x20, 0, 0, scmdbuf) != 0)
	{
		if ((scmdbuf[0] & 0x80) == 0)
		{
			*res = scmdbuf[1];
			return 1;
		}
	}
	return 0;
#endif
}
#endif

static void *RmmanRpc_init(struct rmRpcPacket *packet)
{
	RM_PACKET_CMD(packet).result = rmmanInit();
	return packet;
}

static void *RmmanRpc_version(struct rmRpcPacket *packet)
{
	RM_PACKET_CMD(packet).result = rmmanVersion();
	return packet;
}

static void *RmmanRpc_open(struct rmRpcPacket *packet)
{
#ifndef BUILDING_RMMAN2
	RM_PACKET_CMD(packet).result = rmmanOpen(RM_PACKET_CMD(packet).port, RM_PACKET_CMD(packet).slot, RM_PACKET_CMD(packet).data);
#else
	RM_PACKET_CMD(packet).result = rmmanOpen(0, 0, RM_PACKET_CMD(packet).data);
#endif
	return packet;
}

static void *RmmanRpc_end(struct rmRpcPacket *packet)
{
	RM_PACKET_CMD(packet).result = rmmanEnd();
	return packet;
}

static void *RmmanRpc_close(struct rmRpcPacket *packet)
{
#ifndef BUILDING_RMMAN2
	RM_PACKET_CMD(packet).result = rmmanClose(RM_PACKET_CMD(packet).port, RM_PACKET_CMD(packet).slot);
#else
	RM_PACKET_CMD(packet).result = rmmanClose(0, 0);
#endif
	return packet;
}

#ifdef BUILDING_RMMAN2
static void *RmmanRpc_remote2_6(struct rmRpcPacket *packet)
{
	RM_PACKET_CMD(packet).result = rmmanRemote2_6((u8 *)&(RM_PACKET_CMD(packet).data));
	return packet;
}
#endif

static void *RpcHandler(int fno, void *buffer, int len)
{
	void *retBuff;

	(void)fno;
	(void)len;

	switch(((struct rmRpcPacket *)buffer)->cmd.command)
	{
		case RM_RPCFUNC_END:
			retBuff = RmmanRpc_end((struct rmRpcPacket *)buffer);
			break;
		case RM_RPCFUNC_INIT:
			retBuff = RmmanRpc_init((struct rmRpcPacket *)buffer);
			break;
		case RM_RPCFUNC_CLOSE:
			retBuff = RmmanRpc_close((struct rmRpcPacket *)buffer);
			break;
		case RM_RPCFUNC_OPEN:
			retBuff = RmmanRpc_open((struct rmRpcPacket *)buffer);
			break;
		case RM_RPCFUNC_VERSION:
			retBuff = RmmanRpc_version((struct rmRpcPacket *)buffer);
			break;
#ifdef BUILDING_RMMAN2
		case RM_RPCFUNC_REMOTE2_6:
			retBuff = RmmanRpc_remote2_6((struct rmRpcPacket *)buffer);
			break;
#endif
		default:
			DPRINTF("invalid function code (%03x)\n", (unsigned int)((struct rmRpcPacket *)buffer)->cmd.command);
			retBuff = buffer;
	}

	return retBuff;
}

static void RpcThread(void *arg)
{
	(void)arg;

	if(!sceSifCheckInit())
	{
		DPRINTF("yet sif hasn't been init\n");
		sceSifInit();
	}

	sceSifInitRpc(0);

	sceSifSetRpcQueue(&RpcDataQueue, GetThreadId());
	sceSifRegisterRpc(&RpcServerData, RM_RPC_ID, &RpcHandler, &RpcDataBuffer, NULL, NULL, &RpcDataQueue);
	sceSifRpcLoop(&RpcDataQueue);
}

static int CreateMainThread(void)
{
	iop_thread_t ThreadData;
	int result;

	ThreadData.attr=TH_C;
	ThreadData.thread=&RpcThread;
	ThreadData.priority=0x2E;
	ThreadData.stacksize=0x800;
	if((RpcThreadID=CreateThread(&ThreadData))!=0)
	{
		StartThread(RpcThreadID, NULL);
		result = 1;
	}
	else result = 0;

	return result;
}
