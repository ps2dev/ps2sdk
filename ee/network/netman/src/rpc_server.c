#include <kernel.h>
#include <malloc.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_server.h"

#ifdef F___NETMAN_RpcSvr_threadID
int __NETMAN_RpcSvr_threadID=-1;
#else
extern int __NETMAN_RpcSvr_threadID;
#endif

#ifdef F___NETMAN_Rx_threadID
int __NETMAN_Rx_threadID=-1;
#else
extern int __NETMAN_Rx_threadID;
#endif

#ifdef F___NETMAN_SifHandlerID
int __NETMAN_SifHandlerID=-1;
#else
extern int __NETMAN_SifHandlerID;
#endif

#ifdef F___rpc_server_IsInitialized
unsigned char __rpc_server_IsInitialized=0;
#else
extern unsigned char __rpc_server_IsInitialized;
#endif

#ifdef F___rpc_server_IsProcessingRx
unsigned char __rpc_server_IsProcessingRx;
#else
extern unsigned char __rpc_server_IsProcessingRx;
#endif

#ifdef F___rpc_server_FrameBufferStatus
struct NetManBD *__rpc_server_FrameBufferStatus = NULL;
#else
extern struct NetManBD *__rpc_server_FrameBufferStatus;
#endif

#ifdef F___rpc_server_RxIOPFrameBufferStatus
struct NetManBD *__rpc_server_RxIOPFrameBufferStatus;
#else
extern struct NetManBD *__rpc_server_RxIOPFrameBufferStatus;
#endif

#ifdef F___rpc_server_cb_queue
struct t_SifRpcDataQueue __rpc_server_cb_queue;
#else
extern struct t_SifRpcDataQueue __rpc_server_cb_queue;
#endif

#ifdef F___rpc_server_cb_srv
struct t_SifRpcServerData __rpc_server_cb_srv;
#else
extern struct t_SifRpcServerData __rpc_server_cb_srv;
#endif

extern void *_gp;

#ifdef F___rpc_server_ClearBufferLen
void __rpc_server_ClearBufferLen(int index, void *packet, void *payload)
{
	struct NetManBD *bd;
	SifDmaTransfer_t dmat;

	bd = UNCACHED_SEG(&__rpc_server_FrameBufferStatus[index]);
	bd->length = 0;
	bd->packet = packet;
	bd->payload = payload;

	//Transfer to IOP RAM
	dmat.src = (void*)&__rpc_server_FrameBufferStatus[index];
	dmat.dest = &__rpc_server_RxIOPFrameBufferStatus[index];
	dmat.size = sizeof(struct NetManBD);
	dmat.attr = 0;
	while(SifSetDma(&dmat, 1) == 0){ };
}
#else
extern void __rpc_server_ClearBufferLen(int index, void *packet, void *payload);
#endif

#ifdef F__NetManRPCAllocRxBuffers
int _NetManRPCAllocRxBuffers(void)
{
	int i;

	for(i = 0; i < NETMAN_RPC_BLOCK_SIZE; i++)
	{
		void *packet, *payload;

		if((packet = NetManNetProtStackAllocRxPacket(NETMAN_NETIF_FRAME_SIZE, &payload)) != NULL)
		{
			__rpc_server_ClearBufferLen(i, packet, payload);
		} else {
			printf("NETMAN: error - unable to allocate Rx FIFO buffers.\n");
			return -ENOMEM;
		}
	}

	return 0;
}
#endif

#ifdef F__NetManInitRPCServer
static unsigned char NETMAN_RpcSvr_ThreadStack[0x1000] ALIGNED(16);
static unsigned char NETMAN_Rx_ThreadStack[0x1000] ALIGNED(16);
static unsigned short int RxBufferRdPtr, RxBufferNextRdPtr;

static s32 HandleRxEvent(s32 channel)
{
	struct NetManBD *bd;

	(void)channel;

	bd = UNCACHED_SEG(&__rpc_server_FrameBufferStatus[RxBufferNextRdPtr]);
	if(bd->length > 0)
	{
		iSifSetDChain();

		if(!__rpc_server_IsProcessingRx)
		{
			__rpc_server_IsProcessingRx = 1;
			iWakeupThread(__NETMAN_Rx_threadID);
		}
	}

	ExitHandler();
	//Must allow the other SIF handlers to check their states, as it is possible for this handler to block other handlers until __rpc_server_FrameBufferStatus is cleared.
	return 0;
}

static void NETMAN_RxThread(void *arg);

/* Main EE RPC thread. */
static void *NETMAN_EE_RPC_Handler(int fnum, void *buffer, int NumBytes)
{
	ee_thread_t thread;
	void *result;

	(void)NumBytes;

	switch(fnum)
	{
		case NETMAN_EE_RPC_FUNC_INIT:
			__rpc_server_RxIOPFrameBufferStatus = *(void**)buffer;

			//Maintain 64-byte alignment to avoid non-uncached writes to the same cache line from contaminating the line.
			if(__rpc_server_FrameBufferStatus == NULL) __rpc_server_FrameBufferStatus = memalign(64, sizeof(struct NetManBD) * NETMAN_RPC_BLOCK_SIZE);

			if(__rpc_server_FrameBufferStatus != NULL)
			{
				memset(UNCACHED_SEG(__rpc_server_FrameBufferStatus), 0, sizeof(struct NetManBD) * NETMAN_RPC_BLOCK_SIZE);
				RxBufferRdPtr = 0;
				RxBufferNextRdPtr = 0;
				__rpc_server_IsProcessingRx = 0;

				thread.func=&NETMAN_RxThread;
				thread.stack=NETMAN_Rx_ThreadStack;
				thread.stack_size=sizeof(NETMAN_Rx_ThreadStack);
				thread.gp_reg=&_gp;
				thread.initial_priority=0x59;	/* Should be given a lower priority than the protocol stack, so that the protocol stack can process incoming frames. */
				thread.attr=thread.option=0;

				if((__NETMAN_Rx_threadID=CreateThread(&thread)) >= 0)
				{
					StartThread(__NETMAN_Rx_threadID, NULL);

					__NETMAN_SifHandlerID = AddDmacHandler(DMAC_SIF0, &HandleRxEvent, 0);
					EnableDmac(DMAC_SIF0);
					((struct NetManEEInitResult *)buffer)->result = 0;
					((struct NetManEEInitResult *)buffer)->FrameBufferStatus = __rpc_server_FrameBufferStatus;
				}
				else{
					((struct NetManEEInitResult *)buffer)->result = __NETMAN_Rx_threadID;
				}
			}else{
				((struct NetManEEInitResult *)buffer)->result = -ENOMEM;
			}
			result=buffer;
			break;
		case NETMAN_EE_RPC_FUNC_DEINIT:
			if(__rpc_server_FrameBufferStatus != NULL)
			{
				free(__rpc_server_FrameBufferStatus);
				__rpc_server_FrameBufferStatus = NULL;
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

static void NETMAN_RPC_Thread(void *arg)
{
	static unsigned char cb_rpc_buffer[64] __attribute__((__aligned__(64)));

	(void)arg;

	SifSetRpcQueue(&__rpc_server_cb_queue, __NETMAN_RpcSvr_threadID);
	SifRegisterRpc(&__rpc_server_cb_srv, NETMAN_RPC_NUMBER, &NETMAN_EE_RPC_Handler, cb_rpc_buffer, NULL, NULL, &__rpc_server_cb_queue);
	SifRpcLoop(&__rpc_server_cb_queue);
}

static void NETMAN_RxThread(void *arg)
{
	volatile struct NetManBD *bd;
	u32 PacketLength, PacketLengthAligned;
	u8 run;

	(void)arg;

	while(1)
	{
		void *payload, *payloadNext;
		void *packet, *packetNext;

		bd = UNCACHED_SEG(&__rpc_server_FrameBufferStatus[RxBufferRdPtr]);

		do {
			DI();
			PacketLength = bd->length;
			if (PacketLength > 0)
			{
				run = 1;
			} else {
				run = 0;
				__rpc_server_IsProcessingRx = 0;
			}
			EI();

			if(!run)
				SleepThread();
		} while(!run);

		payload = bd->payload;
		packet = bd->packet;

		//Must successfully allocate a replacement buffer for the input buffer.
		while((packetNext = NetManNetProtStackAllocRxPacket(NETMAN_NETIF_FRAME_SIZE, &payloadNext)) == NULL){};
		RxBufferNextRdPtr = (RxBufferNextRdPtr + 1) % NETMAN_RPC_BLOCK_SIZE;
		__rpc_server_ClearBufferLen(RxBufferRdPtr, packetNext, payloadNext);

		//Increment read pointer by one place.
		RxBufferRdPtr = RxBufferNextRdPtr;

		//Now process the received packet.
		PacketLengthAligned = (PacketLength + 63) & ~63;
		SifWriteBackDCache(payload, PacketLengthAligned);
		NetManNetProtStackReallocRxPacket(packet, PacketLength);
		NetManNetProtStackEnQRxPacket(packet);
	}
}

int _NetManInitRPCServer(void)
{
	int result;
	ee_thread_t ThreadData;

	if(!__rpc_server_IsInitialized)
	{
		ThreadData.func=&NETMAN_RPC_Thread;
		ThreadData.stack=NETMAN_RpcSvr_ThreadStack;
		ThreadData.stack_size=sizeof(NETMAN_RpcSvr_ThreadStack);
		ThreadData.gp_reg=&_gp;
		ThreadData.initial_priority=0x57;	/* The RPC server thread should be given a higher priority than the protocol stack, so that it can issue commants to the EE and return. */
		ThreadData.attr=ThreadData.option=0;

		if((__NETMAN_RpcSvr_threadID=CreateThread(&ThreadData))>=0)
		{
			StartThread(__NETMAN_RpcSvr_threadID, NULL);
			__rpc_server_IsInitialized=1;
			result=0;
		}
		else result=__NETMAN_RpcSvr_threadID;
	}
	else result=0;

	return result;
}
#endif

#ifdef F__NetManDeinitRPCServer
void _NetManDeinitRPCServer(void)
{
	if(__rpc_server_IsInitialized)
	{
		SifRemoveRpc(&__rpc_server_cb_srv, &__rpc_server_cb_queue);
		SifRemoveRpcQueue(&__rpc_server_cb_queue);
		RemoveDmacHandler(DMAC_SIF0, __NETMAN_SifHandlerID);

		if(__NETMAN_RpcSvr_threadID>=0)
		{
			TerminateThread(__NETMAN_RpcSvr_threadID);
			DeleteThread(__NETMAN_RpcSvr_threadID);
			__NETMAN_RpcSvr_threadID = -1;
		}

		if(__NETMAN_Rx_threadID>=0)
		{
			TerminateThread(__NETMAN_Rx_threadID);
			DeleteThread(__NETMAN_Rx_threadID);
			__NETMAN_Rx_threadID = -1;
		}

		__rpc_server_IsInitialized=0;
	}
}
#endif
