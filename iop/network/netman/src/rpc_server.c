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
static unsigned char rpc_buffer[(sizeof(struct PacketReqs)+0xF)&~0xF];
static SifRpcDataQueue_t rpc_qdata;

static unsigned char SifRpcTxBuffer[80];

/* Packet transmission buffer. The EE will DMA transfer the packet to be transmitted directly into this buffer before invoking FUNC_SEND_PACKET. */
static void *TxPacketBuffer = NULL;

static int RpcThreadID = -1;
static unsigned char IsInitialized=0, IsRpcStackInitialized=0;

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

static void unregisterEENetworkStack(void){
	if(IsRpcStackInitialized){
		IsRpcStackInitialized=0;
		NetManUnregisterNetworkStack();

		if(TxPacketBuffer != NULL){
			free(TxPacketBuffer);
			TxPacketBuffer = NULL;
		}
	}
}

static void *NETMAN_rpc_handler(int fno, void *buffer, int size){
	static int ResultValue;
	void *result;
	unsigned int i;

	switch(fno){
		case NETMAN_IOP_RPC_FUNC_INIT:
			ResultValue=NetManInitRPCClient();
			result=&ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_DEINIT:
			unregisterEENetworkStack();
			NetManDeinitRPCClient();
			result=NULL;
			break;
		case NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK:
			result = SifRpcTxBuffer;

			if(TxPacketBuffer == NULL) TxPacketBuffer = malloc((MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE+0xF) & ~0xF);

			if(TxPacketBuffer != NULL){
				((struct NetManRegNetworkStackResult *)SifRpcTxBuffer)->FrameBuffer = TxPacketBuffer;
				ResultValue = NetManRegisterNetworkStack(&RpcStack);
			}else{
				ResultValue = -ENOMEM;
			}

			IsRpcStackInitialized=1;
			((struct NetManRegNetworkStackResult *)SifRpcTxBuffer)->result=ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK:
			unregisterEENetworkStack();
			result=NULL;
			break;
		case NETMAN_IOP_RPC_FUNC_SEND_PACKETS:
			if(TxPacketBuffer!=NULL){
				for(i=0; i<((struct PacketReqs*)buffer)->NumPackets; i++){
					NetManNetIFSendPacket(&((unsigned char*)TxPacketBuffer)[((struct PacketReqs*)buffer)->tags[i].offset], ((struct PacketReqs*)buffer)->tags[i].length);
				}

				ResultValue=0;
			}else ResultValue=-1;

			result=&ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_IOCTL:
			((struct NetManIoctlResult*)SifRpcTxBuffer)->result=NetManIoctl(((struct NetManIoctl*)buffer)->command, ((struct NetManIoctl*)buffer)->args, ((struct NetManIoctl*)buffer)->args_len, ((struct NetManIoctlResult*)SifRpcTxBuffer)->output, ((struct NetManIoctl*)buffer)->length);
			result=SifRpcTxBuffer;
			break;
		case NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF:
			*(int*)SifRpcTxBuffer=NetManSetMainIF(buffer);
			result=SifRpcTxBuffer;
			break;
		case NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF:
			((struct NetManQueryMainNetIFResult*)SifRpcTxBuffer)->result=NetManQueryMainIF(((struct NetManQueryMainNetIFResult*)SifRpcTxBuffer)->name);
			result=SifRpcTxBuffer;
			break;
		case NETMAN_IOP_RPC_FUNC_SET_LINK_MODE:
			ResultValue=NetManSetLinkMode(*(int*)buffer);
			result=&ResultValue;
			break;
		default:
			printf("NETMAN [IOP]: Unrecognized command: 0x%x\n", fno);
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

