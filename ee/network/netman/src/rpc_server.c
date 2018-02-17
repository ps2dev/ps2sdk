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

static int NETMAN_RpcSvr_threadID=-1;
static unsigned char NETMAN_RpcSvr_ThreadStack[0x1000] ALIGNED(16);
static unsigned char IsInitialized=0;

static u8 *FrameBuffer = NULL;
static u8 *RxIOPFrameBuffer;
static unsigned short int RxBufferRdPtr;
extern void *_gp;

static void ClearBufferLen(int index)
{
	static u32 zero ALIGNED(16) = 0;

	SifDmaTransfer_t dmat;

	//Transfer to IOP RAM
	dmat.src = (void*)&zero;
	dmat.dest = &RxIOPFrameBuffer[index * NETMAN_MAX_FRAME_SIZE];
	dmat.size = 16;
	dmat.attr = 0;
	while(SifSetDma(&dmat, 1)==0){ };
}

/* Main EE RPC thread. */
static void *NETMAN_EE_RPC_Handler(int fnum, void *buffer, int NumBytes)
{
	void *data, *result, *payload;
	vu32 *pPacketLength;
	u32 PacketLength;
	void *packet;

	switch(fnum)
	{
		case NETMAN_EE_RPC_FUNC_INIT:
			RxIOPFrameBuffer = *(void**)buffer;

			if(FrameBuffer == NULL) FrameBuffer = memalign(64, NETMAN_MAX_FRAME_SIZE * NETMAN_RPC_BLOCK_SIZE);

			if(FrameBuffer != NULL)
			{
				memset(UNCACHED_SEG(FrameBuffer), 0, NETMAN_MAX_FRAME_SIZE * NETMAN_RPC_BLOCK_SIZE);
				RxBufferRdPtr = 0;
				((struct NetManEEInitResult *)buffer)->FrameBuffer = FrameBuffer;
				((struct NetManEEInitResult *)buffer)->result = 0;
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
			result=NULL;
			break;
		case NETMAN_EE_RPC_FUNC_HANDLE_PACKETS:
			if(FrameBuffer != NULL)
			{
				while(1)
				{
					pPacketLength = (vu32*)UNCACHED_SEG(&FrameBuffer[RxBufferRdPtr * NETMAN_MAX_FRAME_SIZE]);
					if((PacketLength = *pPacketLength) < 1)
						break;

					data = UNCACHED_SEG(&FrameBuffer[RxBufferRdPtr * NETMAN_MAX_FRAME_SIZE + 16]);

					if((packet = NetManNetProtStackAllocRxPacket(PacketLength, &payload)) != NULL)
					{
						memcpy(payload, data, PacketLength);
						NetManNetProtStackEnQRxPacket(packet);
					}

					*pPacketLength = 0;
					ClearBufferLen(RxBufferRdPtr);
					//Increment read pointer by one place.
					RxBufferRdPtr = (RxBufferRdPtr + 1) % NETMAN_RPC_BLOCK_SIZE;
				}
			}

			*(u32 *)buffer=0;
			result=buffer;
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
		ThreadData.initial_priority=0x57;	/* The RPC server thread should be given a higher priority than the protocol stack, so that it can dump frames in the EE and return. */
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

		if(NETMAN_RpcSvr_threadID>=0)
		{
			TerminateThread(NETMAN_RpcSvr_threadID);
			DeleteThread(NETMAN_RpcSvr_threadID);
			NETMAN_RpcSvr_threadID = -1;
		}

		IsInitialized=0;
	}
}
