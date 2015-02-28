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
static unsigned char NETMAN_RpcSvr_ThreadStack[0x1000] ALIGNED(128);

static unsigned char *FrameBuffer;
static unsigned char IsInitialized=0;

extern void *_gp;

/* Main EE RPC thread. */
static void *NETMAN_EE_RPC_Handler(int fnum, void *buffer, int NumBytes){
	unsigned int PacketNum;
	void *data, *result;
	unsigned short int PacketLength;
	struct NetManPacketBuffer *pbuf;

	switch(fnum){
		case NETMAN_EE_RPC_FUNC_INIT:
			if(FrameBuffer == NULL) FrameBuffer = memalign(64, (MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE+0x3F)&~0x3F);

			if(FrameBuffer != NULL){
				((struct NetManEEInitResult *)buffer)->FrameBuffer = FrameBuffer;
				((struct NetManEEInitResult *)buffer)->result = 0;
			}else{
				((struct NetManEEInitResult *)buffer)->result = -ENOMEM;
			}
			result=buffer;
			break;
		case NETMAN_EE_RPC_FUNC_DEINIT:
			if(FrameBuffer != NULL){
				free(FrameBuffer);
				FrameBuffer=NULL;
			}
			result=NULL;
			break;
		case NETMAN_EE_RPC_FUNC_HANDLE_PACKETS:
			if(FrameBuffer != NULL){
				for(PacketNum=0; PacketNum<((struct PacketReqs*)buffer)->NumPackets; PacketNum++){
					data=UNCACHED_SEG(&((unsigned char*)FrameBuffer)[((struct PacketReqs*)buffer)->tags[PacketNum].offset]);
					PacketLength=((struct PacketReqs*)buffer)->tags[PacketNum].length;

					/* Need too be careful here. With some packets in the waiting queue (Occupying packet buffers), packet buffer exhaustion can occur. */
					if((pbuf=NetManNetProtStackAllocRxPacket(PacketLength))==NULL){
						NetManNetProtStackFlushInputQueue();
						if((pbuf=NetManNetProtStackAllocRxPacket(PacketLength))==NULL) break;	// Can't continue.
					}

					memcpy(pbuf->payload, data, PacketLength);
					NetManNetProtStackEnQRxPacket(pbuf);
				}
				NetManNetProtStackFlushInputQueue();

				*(int *)buffer=0;
			}else *(int *)buffer=-1;
			result=buffer;

			break;
		case NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE:
			NetManToggleGlobalNetIFLinkState(*(unsigned int*)buffer);
			result=NULL;
			break;
		default:
			printf("NETMAN [EE]: Unrecognized command: 0x%x\n", fnum);
			*(int *)buffer=EINVAL;
			result=buffer;
	}

	return result;
}

static void NETMAN_RPC_Thread(void *arg){
	static struct t_SifRpcDataQueue cb_queue __attribute__((aligned(64)));
	static struct t_SifRpcServerData cb_srv __attribute__((aligned(64)));
	static unsigned char cb_rpc_buffer[(sizeof(struct PacketReqs)+0x3F)&~0x3F] __attribute__((aligned(64)));

	SifSetRpcQueue(&cb_queue, GetThreadId());
	SifRegisterRpc(&cb_srv, NETMAN_RPC_NUMBER, &NETMAN_EE_RPC_Handler, cb_rpc_buffer, NULL, NULL, &cb_queue);
	SifRpcLoop(&cb_queue);
}

int NetManInitRPCServer(void){
	int result;
	ee_thread_t ThreadData;

	if(!IsInitialized){
		ThreadData.func=&NETMAN_RPC_Thread;
		ThreadData.stack=NETMAN_RpcSvr_ThreadStack;
		ThreadData.stack_size=sizeof(NETMAN_RpcSvr_ThreadStack);
		ThreadData.gp_reg=&_gp;
		ThreadData.initial_priority=0x59;	/* The RPC server thread should be given lower priority than the protocol stack, as frame transmission should be given priority. */
		ThreadData.attr=ThreadData.option=0;

		if((NETMAN_RpcSvr_threadID=CreateThread(&ThreadData))>=0){
			StartThread(NETMAN_RpcSvr_threadID, NULL);
			IsInitialized=1;
			result=0;
		}
		else result=NETMAN_RpcSvr_threadID;
	}
	else result=0;

	return result;
}

void NetManDeinitRPCServer(void){
	if(IsInitialized){
		if(NETMAN_RpcSvr_threadID>=0){
			TerminateThread(NETMAN_RpcSvr_threadID);
			DeleteThread(NETMAN_RpcSvr_threadID);
		}

		IsInitialized=0;
	}
}
