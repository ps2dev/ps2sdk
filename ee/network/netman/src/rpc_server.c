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

static int PS2IP_threadID=-1;
static unsigned char PS2IP_ThreadStack[0x1000] ALIGNED(128);

static unsigned char FrameTagBuffer[(sizeof(struct PacketReqs)+0x3F)&~0x3F];
static unsigned char IsInitialized=0;

extern void *_gp;

/* Main EE RPC thread. */
static void *PS2IP_EE_RPC_Thread(int fnum, void *buffer, int NumBytes){
	unsigned int PacketNum;
	void *data, *result;
	unsigned short int PacketLength;
	struct NetManPacketBuffer *pbuf;

	switch(fnum){
		case NETMAN_EE_RPC_FUNC_INIT:
			((struct NetManInitResult *)buffer)->FrameTagBuffer=FrameTagBuffer;
			((struct NetManInitResult *)buffer)->result=0;
			result=buffer;
			break;
		case NETMAN_EE_RPC_FUNC_DEINIT:
			result=NULL;
			break;
		case NETMAN_EE_RPC_FUNC_HANDLE_PACKETS:

			for(PacketNum=0; PacketNum<((struct PacketReqs*)UNCACHED_SEG(FrameTagBuffer))->NumPackets; PacketNum++){
				data=&((unsigned char*)buffer)[((struct PacketReqs*)UNCACHED_SEG(FrameTagBuffer))->tags[PacketNum].offset];
				PacketLength=((struct PacketReqs*)UNCACHED_SEG(FrameTagBuffer))->tags[PacketNum].length;

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
			result=buffer;
			break;
		case NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE:
			NetManToggleGlobalNetIFLinkState(*(unsigned int*)buffer);
			result=NULL;
			break;
		default:
			printf("[EE] PS2IP: Unrecognized command: 0x%x\n", fnum);
			*(int *)buffer=EINVAL;
			result=buffer;
	}

	return result;
}

static void PS2IP_RPC_Thread(void *arg){
	static struct t_SifRpcDataQueue cb_queue __attribute__((aligned(64)));
	static struct t_SifRpcServerData cb_srv __attribute__((aligned(64)));
	static unsigned char cb_rpc_buffer[MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE] __attribute__((aligned(64)));

	SifSetRpcQueue(&cb_queue, GetThreadId());
	SifRegisterRpc(&cb_srv, NETMAN_RPC_NUMBER, &PS2IP_EE_RPC_Thread, cb_rpc_buffer, NULL, NULL, &cb_queue);
	SifRpcLoop(&cb_queue);
}

int NetManInitRPCServer(void){
	int result;
	ee_thread_t ThreadData;

	if(!IsInitialized){
		ThreadData.func=&PS2IP_RPC_Thread;
		ThreadData.stack=PS2IP_ThreadStack;
		ThreadData.stack_size=sizeof(PS2IP_ThreadStack);
		ThreadData.gp_reg=&_gp;
		ThreadData.initial_priority=0x59;	/* The RPC server thread should be given lower priority than the protocol stack, as frame transmission should be given priority. */
		ThreadData.attr=ThreadData.option=0;

		if((PS2IP_threadID=CreateThread(&ThreadData))>=0){
			StartThread(PS2IP_threadID, NULL);
			IsInitialized=1;
			result=0;
		}
		else result=PS2IP_threadID;
	}
	else result=0;

	return result;
}

void NetManDeinitRPCServer(void){
	if(IsInitialized){
		if(PS2IP_threadID>=0){
			TerminateThread(PS2IP_threadID);
			DeleteThread(PS2IP_threadID);
		}

		IsInitialized=0;
	}
}
