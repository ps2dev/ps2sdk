#include <errno.h>
#include <intrman.h>
#include <sifcmd.h>
#include <sifman.h>
#include <sysclib.h>
#include <thbase.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_client.h"

static SifRpcClientData_t EEClient;
static unsigned char SifRpcRxBuffer[64];
static unsigned char SifRpcTxBuffer[64];

static unsigned char *EEFrameBuffer = NULL;	/* On the EE side */

static struct PacketReqs PacketReqs;
static unsigned char *FrameBuffer = NULL;

int NetManInitRPCClient(void){
	int result;

	memset(&PacketReqs, 0, sizeof(PacketReqs));
	if(FrameBuffer == NULL) FrameBuffer = malloc(NETMAN_RPC_BLOCK_SIZE*MAX_FRAME_SIZE);

	if(FrameBuffer != NULL){
		while((result=sceSifBindRpc(&EEClient, NETMAN_RPC_NUMBER, 0))<0 || EEClient.server==NULL) DelayThread(500);

		if((result=sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_INIT, 0, NULL, 0, SifRpcRxBuffer, sizeof(struct NetManEEInitResult), NULL, NULL))>=0){
			if((result=((struct NetManEEInitResult*)SifRpcRxBuffer)->result) == 0){
				EEFrameBuffer=((struct NetManEEInitResult*)SifRpcRxBuffer)->FrameBuffer;
			}
		}
	}else result = -ENOMEM;

	return result;
}

void NetManDeinitRPCClient(void){
	if(FrameBuffer != NULL){
		free(FrameBuffer);
		FrameBuffer = NULL;
	}

	memset(&EEClient, 0, sizeof(EEClient));
}

void NetManRpcToggleGlobalNetIFLinkState(unsigned int state){
	*(unsigned int *)SifRpcTxBuffer=state;
	sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE, 0, SifRpcTxBuffer, sizeof(unsigned int), NULL, 0, NULL, NULL);
}

static struct NetManPacketBuffer pbufs[NETMAN_RPC_BLOCK_SIZE];

struct NetManPacketBuffer *NetManRpcNetProtStackAllocRxPacket(unsigned int length){
	unsigned int length_aligned;
	struct NetManPacketBuffer *result;

	length_aligned=(length+3)&~3;
	if((PacketReqs.NumPackets+1<NETMAN_RPC_BLOCK_SIZE) && (PacketReqs.TotalLength+length_aligned<NETMAN_RPC_BLOCK_SIZE*MAX_FRAME_SIZE)){
		result=&pbufs[PacketReqs.NumPackets];
		PacketReqs.tags[PacketReqs.NumPackets].offset=PacketReqs.TotalLength;
		PacketReqs.tags[PacketReqs.NumPackets].length=length;
		result->handle=&PacketReqs.tags[PacketReqs.NumPackets];
		result->payload=&FrameBuffer[PacketReqs.TotalLength];
		result->length=length;
		PacketReqs.NumPackets++;
		PacketReqs.TotalLength+=length_aligned;
	}
	else result=NULL;

	return result;
}

void NetManRpcNetProtStackFreeRxPacket(struct NetManPacketBuffer *packet){
	((struct PacketTag*)packet->handle)->length=0;
}

int NetManRpcProtStackEnQRxPacket(struct NetManPacketBuffer *packet){
	return 0;
}

int NetmanRpcFlushInputQueue(void){
	int result;

	if(PacketReqs.NumPackets>0){
		/*	It seems like a good idea if this could be modified to not block while the EE receives and processes data,
			but I've tried that before and it actually worsened performance over TCP. Perhaps the IOP ends up spending too much
			time receiving data that the PlayStation 2 can't send acks in a timely manner.	*/
		DMATransferDataToEEAligned(FrameBuffer, EEFrameBuffer, PacketReqs.TotalLength);
		if((result=sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_HANDLE_PACKETS, 0, &PacketReqs, 8+sizeof(struct PacketTag)*PacketReqs.NumPackets, SifRpcRxBuffer, sizeof(int), NULL, NULL))>=0){
			result=*(int*)SifRpcRxBuffer;
		}

		PacketReqs.NumPackets=0;
		PacketReqs.TotalLength=0;
	}
	else result=0;

	return result;
}
