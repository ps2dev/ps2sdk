#include <kernel.h>
#include <malloc.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <stdio.h>
#include <string.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_server.h"

static int NETMAN_RpcSvr_threadID=-1, NETMAN_Rx_threadID=-1;
static unsigned char NETMAN_RpcSvr_ThreadStack[0x1000] ALIGNED(16);
static unsigned char NETMAN_Rx_ThreadStack[0x1000] ALIGNED(16);
static unsigned char IsInitialized=0, IsProcessingRx;

static u8 *FrameBuffer = NULL;
static u16 *FrameBufferStatus = NULL;
static u8 *RxIOPFrameBufferStatus;
static unsigned short int RxBufferRdPtr;
extern void *_gp;

static void NETMAN_RxThread(void *arg);

static void ClearBufferLen(int index)
{
	static u32 zero[4] ALIGNED(16) = {0, 0, 0, 0};
	SifDmaTransfer_t dmat;

	FrameBufferStatus[index] = 0;

	//Transfer to IOP RAM
	dmat.src = (void*)zero;
	dmat.dest = &RxIOPFrameBufferStatus[index * 16];
	dmat.size = sizeof(zero);
	dmat.attr = 0;
	while(SifSetDma(&dmat, 1) == 0){ };
}

static void HandleRxEvent(void *packet, void *common)
{
	u16 id = ((SifCmdHeader_t*)packet)->opt & 0xFFFF;
	u16 len = (((SifCmdHeader_t*)packet)->opt >> 16) & 0xFFFF;

	FrameBufferStatus[id] = len;

	if(!IsProcessingRx)
	{
		IsProcessingRx = 1;
		iWakeupThread(NETMAN_Rx_threadID);
	}
}

/* Main EE RPC thread. */
static void *NETMAN_EE_RPC_Handler(int fnum, void *buffer, int NumBytes)
{
	ee_thread_t thread;
	void *result;

	switch(fnum)
	{
		case NETMAN_EE_RPC_FUNC_INIT:
			RxIOPFrameBufferStatus = *(void**)buffer;

			if(FrameBuffer == NULL) FrameBuffer = memalign(64, NETMAN_MAX_FRAME_SIZE * NETMAN_RPC_BLOCK_SIZE);
			if(FrameBufferStatus == NULL) FrameBufferStatus = malloc(sizeof(u16) * NETMAN_RPC_BLOCK_SIZE);

			if(FrameBuffer != NULL && FrameBufferStatus != NULL)
			{
				memset(UNCACHED_SEG(FrameBuffer), 0, NETMAN_MAX_FRAME_SIZE * NETMAN_RPC_BLOCK_SIZE);
				memset(FrameBufferStatus, 0, sizeof(u16) * NETMAN_RPC_BLOCK_SIZE);
				RxBufferRdPtr = 0;
				IsProcessingRx = 0;

				((struct NetManEEInitResult *)buffer)->FrameBuffer = FrameBuffer;

				thread.func=&NETMAN_RxThread;
				thread.stack=NETMAN_Rx_ThreadStack;
				thread.stack_size=sizeof(NETMAN_Rx_ThreadStack);
				thread.gp_reg=&_gp;
				thread.initial_priority=0x57;	/* Should be given a higher priority than the protocol stack, so that it can dump frames in the EE and return. */
				thread.attr=thread.option=0;

				if((NETMAN_Rx_threadID=CreateThread(&thread)) >= 0)
				{
					StartThread(NETMAN_Rx_threadID, NULL);

					SifAddCmdHandler(NETMAN_SIFCMD_ID, &HandleRxEvent, NULL);
					((struct NetManEEInitResult *)buffer)->result = 0;
				}
				else{
					((struct NetManEEInitResult *)buffer)->result = NETMAN_Rx_threadID;
				}
			}else{
				((struct NetManEEInitResult *)buffer)->result = -ENOMEM;
			}
			result=buffer;
			break;
		case NETMAN_EE_RPC_FUNC_DEINIT:
			if(FrameBuffer != NULL)
			{
				free(FrameBuffer);
				FrameBuffer = NULL;
			}
			if(FrameBufferStatus != NULL)
			{
				free(FrameBufferStatus);
				FrameBufferStatus = NULL;
			}
			result=NULL;
			break;
		case NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE:
			NetManToggleGlobalNetIFLinkState(*(u32*)buffer);
			result=NULL;
			break;
		default:
			printf("NETMAN [EE]: Unrecognized command: 0x%x\n", fnum);
			*(int *)buffer=EINVAL;
			result=buffer;
	}

	return result;
}

static struct t_SifRpcDataQueue cb_queue;
static struct t_SifRpcServerData cb_srv;

static void NETMAN_RPC_Thread(void *arg)
{
	static unsigned char cb_rpc_buffer[64] __attribute__((aligned(64)));

	SifSetRpcQueue(&cb_queue, NETMAN_RpcSvr_threadID);
	SifRegisterRpc(&cb_srv, NETMAN_RPC_NUMBER, &NETMAN_EE_RPC_Handler, cb_rpc_buffer, NULL, NULL, &cb_queue);
	SifRpcLoop(&cb_queue);
}

static void NETMAN_RxThread(void *arg)
{
	void *data, *payload;
	u32 PacketLength;
	void *packet;
	u8 run;

	while(1)
	{
		do {
			DI();
			PacketLength = FrameBufferStatus[RxBufferRdPtr];
			if (PacketLength > 0)
			{
				run = 1;
			} else {
				run = 0;
				IsProcessingRx = 0;
			}
			EI();

			if(!run)
				SleepThread();
		} while(!run);

		data = UNCACHED_SEG(&FrameBuffer[RxBufferRdPtr * NETMAN_MAX_FRAME_SIZE]);

		if((packet = NetManNetProtStackAllocRxPacket(PacketLength, &payload)) != NULL)
		{
			memcpy(payload, data, PacketLength);
			NetManNetProtStackEnQRxPacket(packet);
		}

		ClearBufferLen(RxBufferRdPtr);
		//Increment read pointer by one place.
		RxBufferRdPtr = (RxBufferRdPtr + 1) % NETMAN_RPC_BLOCK_SIZE;
	}
}

int NetManInitRPCServer(void)
{
	int result;
	ee_thread_t ThreadData;

	if(!IsInitialized)
	{
		ThreadData.func=&NETMAN_RPC_Thread;
		ThreadData.stack=NETMAN_RpcSvr_ThreadStack;
		ThreadData.stack_size=sizeof(NETMAN_RpcSvr_ThreadStack);
		ThreadData.gp_reg=&_gp;
		ThreadData.initial_priority=0x57;	/* The RPC server thread should be given a higher priority than the protocol stack, so that it can issue commants to the EE and return. */
		ThreadData.attr=ThreadData.option=0;

		if((NETMAN_RpcSvr_threadID=CreateThread(&ThreadData))>=0)
		{
			StartThread(NETMAN_RpcSvr_threadID, NULL);
			IsInitialized=1;
			result=0;
		}
		else result=NETMAN_RpcSvr_threadID;
	}
	else result=0;

	return result;
}

void NetManDeinitRPCServer(void)
{
	if(IsInitialized)
	{
		SifRemoveRpc(&cb_srv, &cb_queue);
		SifRemoveRpcQueue(&cb_queue);
		SifRemoveCmdHandler(NETMAN_SIFCMD_ID);

		if(NETMAN_RpcSvr_threadID>=0)
		{
			TerminateThread(NETMAN_RpcSvr_threadID);
			DeleteThread(NETMAN_RpcSvr_threadID);
			NETMAN_RpcSvr_threadID = -1;
		}

		if(NETMAN_Rx_threadID>=0)
		{
			TerminateThread(NETMAN_Rx_threadID);
			DeleteThread(NETMAN_Rx_threadID);
			NETMAN_Rx_threadID = -1;
		}

		IsInitialized=0;
	}
}
