#include <intrman.h>
#include <loadcore.h>
#include <sifcmd.h>
#include <sifman.h>
#include <stdio.h>
#include <thbase.h>
#include <thevent.h>
#include <vblank.h>
#include <rsio2man.h>
#include <irx.h>

#include "rmman.h"

IRX_ID("rmman", 1, 16);

#ifdef DEBUG
#define DPRINTF(x...) printf("RMMAN: "x)
#else
#define DPRINTF(x...)
#endif

#define RM_EF_EXIT_THREAD		1
#define RM_EF_EXIT_THREAD_DONE		2
#define RM_EF_CLOSE_PORT		4
#define RM_EF_CLOSE_PORT_DONE		8

enum RM_TASK {
	RM_TASK_QUERY = 0,
	RM_TASK_INIT,
	RM_TASK_POLL
};

struct RmData{				//size = 316
	struct rmEEData eeData;		//0x000
	u32 unused1;			//0x030
	u8 inBuffer[32];		//0x034
	u8 outBuffer[32];		//0x054
	u32 state;			//0x074
	u32 reqState;			//0x078
	u32 frame;			//0x07C
	struct rmEEData *eeBuffer;	//0x080
	s32 port;			//0x084
	s32 slot;			//0x088
	u32 currentTask;		//0x08C
	u32 counter;			//0x090
	u32 powerMode;			//0x094
	u32 closed;			//0x098
	u32 connected;			//0x09C
	u32 eventFlagID;		//0x0a0
	u32 unused2;			//0x0a4
	sio2_transfer_data_t sio2Data;	//0x0a8
};

extern struct irx_export_table _exp_rmman;

static struct RmData RmData[2][4];
static int portStatus[2];
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
static int FindRemote(struct RmData *RmData);
static int InitFindRmCmd(struct RmData *RmData);
static int PollRemote(struct RmData *RmData);
static int InitPollRmCmd(struct RmData *RmData);
static int RmExecute(struct RmData *RmData);
static int DmaSendEE(struct RmData *RmData);
static int HandleRmTaskFailed(struct RmData *RmData);
static int InitRemote(struct RmData *RmData);
static int InitInitRmCmd(struct RmData *RmData);

int _start(int argc, char *argv[])
{
	int result;

	(void)argc;
	(void)argv;

	if(RegisterLibraryEntries(&_exp_rmman) == 0)
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

	portStatus[0] = 0;
	portStatus[1] = 0;
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
	iop_event_t evf;
	int result;

	if(!((portStatus[port] >> slot) & 1))
	{
		struct RmData *pRmData;

		pRmData = &RmData[port][slot];
		pRmData->port = port;
		pRmData->slot = slot;
		pRmData->state = RM_STATE_EXECCMD;
		pRmData->reqState = RM_RSTATE_COMPLETE;
		pRmData->eeBuffer = buffer;
		pRmData->currentTask = RM_TASK_QUERY;
		pRmData->counter = 0;

		evf.attr = EA_MULTI;
		evf.bits = 0;

		if((pRmData->eventFlagID = CreateEventFlag(&evf)) != 0)
		{
			pRmData->powerMode = 0;
			portStatus[port] |= (1 << slot);
			result = 1;
		} else
			result = 0;
	} else
		result = 0;

	return result;
}

int rmmanClose(int port, int slot)
{
	int result;
	u32 bits;

	if((portStatus[port] >> slot) & 1)
	{
		struct RmData *pRmData;

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
		for(port = 0; port < 2; port++)
		{
			int slot;
			for(slot = 0; slot < 4; slot++)
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

		for(port = 0; port < 2; port++)
		{
			for(slot = 0; slot < 4; slot++)
			{
				if((portStatus[port] >> slot) & 1)
				{
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
				}
			}
		}
	}
}

static int HandleTasks(struct RmData *RmData)
{
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

	DmaSendEE(RmData);

	return 1;
}

static int FindRemote(struct RmData *RmData)
{
	int result;

	InitFindRmCmd(RmData);
	if(RmExecute(RmData) != 0)
	{
		if(RmData->port < 2)
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

static int PollRemote(struct RmData *RmData)
{
	InitPollRmCmd(RmData);
	return(RmExecute(RmData) > 0);
}

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

static int rmmanVersion(void)
{
	return _irx_id.v;
}

static void *RmmanRpc_init(struct rmRpcPacket *packet)
{
	packet->cmd.result = rmmanInit();
	return packet;
}

static void *RmmanRpc_version(struct rmRpcPacket *packet)
{
	packet->cmd.result = rmmanVersion();
	return packet;
}

static void *RmmanRpc_open(struct rmRpcPacket *packet)
{
	packet->cmd.result = rmmanOpen(packet->cmd.port, packet->cmd.slot, packet->cmd.data);
	return packet;
}

static void *RmmanRpc_end(struct rmRpcPacket *packet)
{
	packet->cmd.result = rmmanEnd();
	return packet;
}

static void *RmmanRpc_close(struct rmRpcPacket *packet)
{
	packet->cmd.result = rmmanClose(packet->cmd.port, packet->cmd.slot);
	return packet;
}

static void *RpcHandler(int fno, void *buffer, int len)
{
	void *retBuff;

	(void)fno;
	(void)len;

	switch(((struct rmRpcPacket *)buffer)->cmd.command)
	{
		case RMMAN_RPCFUNC_END:
			retBuff = RmmanRpc_end((struct rmRpcPacket *)buffer);
			break;
		case RMMAN_RPCFUNC_INIT:
			retBuff = RmmanRpc_init((struct rmRpcPacket *)buffer);
			break;
		case RMMAN_RPCFUNC_CLOSE:
			retBuff = RmmanRpc_close((struct rmRpcPacket *)buffer);
			break;
		case RMMAN_RPCFUNC_OPEN:
			retBuff = RmmanRpc_open((struct rmRpcPacket *)buffer);
			break;
		case RMMAN_RPCFUNC_VERSION:
			retBuff = RmmanRpc_version((struct rmRpcPacket *)buffer);
			break;
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
	sceSifRegisterRpc(&RpcServerData, RMMAN_RPC_ID, &RpcHandler, &RpcDataBuffer, NULL, NULL, &RpcDataQueue);
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
