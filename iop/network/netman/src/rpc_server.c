#include <errno.h>
#include <stdio.h>
#include <sysclib.h>
#include <sifcmd.h>
#include <sifman.h>
#include <loadcore.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#include <intrman.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_server.h"
#include "rpc_client.h"

/* Data used for registering the RPC server. */
static SifRpcServerData_t rpc_sdata;
static unsigned char rpc_buffer[MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE];
static SifRpcDataQueue_t rpc_qdata;

static unsigned char SifRpcTxBuffer[80];

static struct AlignmentData *EEAlignmentDataStructure;

/* Packet transmission buffer. The EE will DMA transfer the packet to be transmitted directly into this buffer before invoking FUNC_SEND_PACKET. */
static unsigned char TxPacketTagBuffer[(sizeof(struct PacketReqs)+0x3F)&~0x3F];

static int RpcThreadID;
static unsigned char IsInitialized=0;

static void LinkStateUp(void){
	NetManRpcToggleGlobalNetIFLinkState(1);
}

static void LinkStateDown(void){
	NetManRpcToggleGlobalNetIFLinkState(0);
}

static struct NetManPacketBuffer *AllocRxPacket(unsigned int length){
	return NetManRpcNetProtStackAllocRxPacket(length);
}

static void FreeRxPacket(struct NetManPacketBuffer *packet){
	NetManRpcNetProtStackFreeRxPacket(packet);
}

static int EnQRxPacket(struct NetManPacketBuffer *packet){
	return NetManRpcProtStackEnQRxPacket(packet);
}

static int FlushInputQueue(void){
	return NetmanRpcFlushInputQueue();
}

static struct NetManNetProtStack RpcStack={
	&LinkStateUp,
	&LinkStateDown,
	&AllocRxPacket,
	&FreeRxPacket,
	&EnQRxPacket,
	&FlushInputQueue
};

static void *NETMAN_rpc_handler(int fno, void *buffer, int size){
	static int ResultValue;
	void *result;
	unsigned int i;

	switch(fno){
		case NETMAN_IOP_RPC_FUNC_INIT:
			EEAlignmentDataStructure=((struct NetManInit*)buffer)->AlignmentData;

			result=SifRpcTxBuffer;
			((struct NetManInitResult *)SifRpcTxBuffer)->FrameTagBuffer=TxPacketTagBuffer;

			if((ResultValue=NetManInitRPCClient())==0){
				ResultValue=NetManInit(&RpcStack);
			}

			((struct NetManInitResult *)SifRpcTxBuffer)->result=ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_DEINIT:
			NetManDeinit();
			NetManDeinitRPCClient();
			result=NULL;
			break;
		case NETMAN_IOP_RPC_FUNC_SEND_PACKETS:
			for(i=0; i<((struct PacketReqs*)TxPacketTagBuffer)->NumPackets; i++){
				NetManNetIFSendPacket(&((unsigned char*)buffer)[((struct PacketReqs*)TxPacketTagBuffer)->tags[i].offset], ((struct PacketReqs*)TxPacketTagBuffer)->tags[i].length);
			}

			ResultValue=0;
			result=&ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_IOCTL:
			((struct NetManIoctlResult*)SifRpcTxBuffer)->result=NetManIoctl(((struct NetManIoctl*)buffer)->command, ((struct NetManIoctl*)buffer)->args, ((struct NetManIoctl*)buffer)->args_len, ((struct NetManIoctlResult*)SifRpcTxBuffer)->output, ((struct NetManIoctl*)buffer)->length);
			result=SifRpcTxBuffer;
			break;
		default:
			printf("NETMAN IOP RPC: Unrecognized command: 0x%x\n", fno);
			ResultValue=-1;
			result=&ResultValue;
	}

	return result;
}

static void NETMAN_RPC_srv(void *args){
	sceSifSetRpcQueue(&rpc_qdata, GetThreadId());

	sceSifRegisterRpc(&rpc_sdata, NETMAN_RPC_NUMBER, &NETMAN_rpc_handler, rpc_buffer, NULL, NULL, &rpc_qdata);
	sceSifRpcLoop(&rpc_qdata);
}

int NetmanInitRPCServer(void){
	iop_thread_t ThreadData;

	if(!IsInitialized){
		ThreadData.attr=TH_C;
		ThreadData.option=0;
		ThreadData.thread=&NETMAN_RPC_srv;
		ThreadData.stacksize=0x800;
		ThreadData.priority=0x27;

		StartThread(RpcThreadID=CreateThread(&ThreadData), NULL);
		IsInitialized=1;
	}

	return 0;
}

void NetmanDeinitRPCServer(void){
	if(IsInitialized){
		TerminateThread(RpcThreadID);
		DeleteThread(RpcThreadID);
		IsInitialized=0;
	}
}

int DMATransferDataToEEAligned(const void *src, void *dest, unsigned int size){
	SifDmaTransfer_t dmat;
	int DMATransferID, oldstate;

	dmat.src=(void *)src;
	dmat.dest=dest;
	dmat.size=size;
	dmat.attr=0;

	do{
		CpuSuspendIntr(&oldstate);
		DMATransferID=sceSifSetDma(&dmat, 1);
		CpuResumeIntr(oldstate);
	}while(DMATransferID==0);

	return DMATransferID;
}

