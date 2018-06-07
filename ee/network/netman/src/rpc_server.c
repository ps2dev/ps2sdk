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

static int NETMAN_RpcSvr_threadID=-1, NETMAN_Rx_threadID=-1, SifHandlerID=-1;
static unsigned char NETMAN_RpcSvr_ThreadStack[0x1000] ALIGNED(16);
static unsigned char NETMAN_Rx_ThreadStack[0x1000] ALIGNED(16);
static unsigned char IsInitialized=0, IsProcessingRx;

static struct NetManBD *FrameBufferStatus = NULL;
static struct NetManBD *RxIOPFrameBufferStatus;
static unsigned short int RxBufferRdPtr, RxBufferNextRdPtr;
extern void *_gp;

static void NETMAN_RxThread(void *arg);

static void ClearBufferLen(int index, void *packet, void *payload)
{
	struct NetManBD *bd;
	SifDmaTransfer_t dmat;

	bd = UNCACHED_SEG(&FrameBufferStatus[index]);
	bd->length = 0;
	bd->packet = packet;
	bd->payload = payload;

	//Transfer to IOP RAM
	dmat.src = (void*)&FrameBufferStatus[index];
	dmat.dest = &RxIOPFrameBufferStatus[index];
	dmat.size = sizeof(struct NetManBD);
	dmat.attr = 0;
	while(SifSetDma(&dmat, 1) == 0){ };
}

static s32 HandleRxEvent(s32 channel)
{
	struct NetManBD *bd;

	bd = UNCACHED_SEG(&FrameBufferStatus[RxBufferNextRdPtr]);
	if(bd->length > 0)
	{
		iSifSetDChain();

		if(!IsProcessingRx)
		{
			IsProcessingRx = 1;
			iWakeupThread(NETMAN_Rx_threadID);
		}
	}

	ExitHandler();
	return 0;
}

int NetManRPCAllocRxBuffers(void)
{
	int i;
	void *packet, *payload;

	for(i = 0; i < NETMAN_RPC_BLOCK_SIZE; i++)
	{
		if((packet = NetManNetProtStackAllocRxPacket(NETMAN_NETIF_FRAME_SIZE, &payload)) != NULL)
		{
			ClearBufferLen(i, packet, payload);
		} else {
			printf("NETMAN: error - unable to allocate Rx FIFO buffers.\n");
			return -ENOMEM;
		}
	}

	return 0;
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

			//Maintain 64-byte alignment to avoid non-uncached writes to the same cache line from contaminating the line.
			if(FrameBufferStatus == NULL) FrameBufferStatus = memalign(64, sizeof(struct NetManBD) * NETMAN_RPC_BLOCK_SIZE);

			if(FrameBufferStatus != NULL)
			{
				memset(UNCACHED_SEG(FrameBufferStatus), 0, sizeof(struct NetManBD) * NETMAN_RPC_BLOCK_SIZE);
				RxBufferRdPtr = 0;
				RxBufferNextRdPtr = 0;
				IsProcessingRx = 0;

				thread.func=&NETMAN_RxThread;
				thread.stack=NETMAN_Rx_ThreadStack;
				thread.stack_size=sizeof(NETMAN_Rx_ThreadStack);
				thread.gp_reg=&_gp;
				thread.initial_priority=0x59;	/* Should be given a lower priority than the protocol stack, so that the protocol stack can process incoming frames. */
				thread.attr=thread.option=0;

				if((NETMAN_Rx_threadID=CreateThread(&thread)) >= 0)
				{
					StartThread(NETMAN_Rx_threadID, NULL);

					SifHandlerID = AddDmacHandler(DMAC_SIF0, &HandleRxEvent, 0);
					EnableDmac(DMAC_SIF0);
					((struct NetManEEInitResult *)buffer)->result = 0;
					((struct NetManEEInitResult *)buffer)->FrameBufferStatus = FrameBufferStatus;
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
	volatile struct NetManBD *bd;
	void *payload, *payloadNext;
	u32 PacketLength, PacketLengthAligned;
	void *packet, *packetNext;
	u8 run;

	while(1)
	{
		bd = UNCACHED_SEG(&FrameBufferStatus[RxBufferRdPtr]);

		do {
			DI();
			PacketLength = bd->length;
			if (PacketLength > 0)
			{
				run = 1;
			} else {
				run = 0;
				IsProcessingRx = 0;
			}
			EI();

			if(!run)
			{
				SifSetReg(SIF_REG_SMFLAG, NETMAN_SBUS_BITS);	//Acknowledge interrupt
				SleepThread();
			}
		} while(!run);

		payload = bd->payload;
		packet = bd->packet;

		//Must successfully allocate a replacement buffer for the input buffer.
		while((packetNext = NetManNetProtStackAllocRxPacket(NETMAN_NETIF_FRAME_SIZE, &payloadNext)) == NULL){};
		RxBufferNextRdPtr = (RxBufferNextRdPtr + 1) % NETMAN_RPC_BLOCK_SIZE;
		ClearBufferLen(RxBufferRdPtr, packetNext, payloadNext);

		//Increment read pointer by one place.
		RxBufferRdPtr = RxBufferNextRdPtr;

		//Now process the received packet.
		PacketLengthAligned = (PacketLength + 63) & ~63;
		SifWriteBackDCache(payload, PacketLengthAligned);
		NetManNetProtStackReallocRxPacket(packet, PacketLength);
		NetManNetProtStackEnQRxPacket(packet);
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
		RemoveDmacHandler(DMAC_SIF0, SifHandlerID);

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
