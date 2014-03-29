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

static unsigned char *EEFrameTagBuffer;	/* On the EE side */

static struct PacketReqs PacketReqs;
static unsigned char FrameBuffer[NETMAN_RPC_BLOCK_SIZE*MAX_FRAME_SIZE];

int NetManInitRPCClient(void){
	int result;

	memset(&PacketReqs, 0, sizeof(PacketReqs));
	memset(FrameBuffer, 0, sizeof(FrameBuffer));

	while((result=sceSifBindRpc(&EEClient, NETMAN_RPC_NUMBER, 0))<0 || EEClient.server==NULL) DelayThread(500);

	if((result=sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_INIT, 0, NULL, 0, SifRpcRxBuffer, sizeof(struct NetManInitResult), NULL, NULL))>=0){
		result=((struct NetManInitResult*)SifRpcRxBuffer)->result;
		EEFrameTagBuffer=((struct NetManInitResult*)SifRpcRxBuffer)->FrameTagBuffer;
	}

	return result;
}

void NetManDeinitRPCClient(void){
	memset(&EEClient, 0, sizeof(EEClient));
}

void NetManRpcToggleGlobalNetIFLinkState(unsigned int state){
	*(unsigned int *)SifRpcTxBuffer=state;
	sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE, 0, SifRpcTxBuffer, sizeof(unsigned int), NULL, 0, NULL, NULL);
}

static struct NetManPacketBuffer pbufs[NETMAN_RPC_BLOCK_SIZE];

struct NetManPacketBuffer *NetManRpcNetProtStackAllocRxPacket(unsigned int length){
	struct NetManPacketBuffer *result;

	if(PacketReqs.NumPackets<NETMAN_RPC_BLOCK_SIZE){
		result=&pbufs[PacketReqs.NumPackets];
		PacketReqs.tags[PacketReqs.NumPackets].offset=PacketReqs.TotalLength;
		PacketReqs.tags[PacketReqs.NumPackets].length=length;
		result->handle=&PacketReqs.tags[PacketReqs.NumPackets];
		result->payload=&FrameBuffer[PacketReqs.TotalLength];
		result->length=length;
		PacketReqs.NumPackets++;
		PacketReqs.TotalLength+=(length+3)&~3;
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
		DMATransferDataToEEAligned(&PacketReqs, EEFrameTagBuffer, 8+sizeof(struct PacketTag)*PacketReqs.NumPackets);
		if((result=sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_HANDLE_PACKETS, 0, FrameBuffer, PacketReqs.TotalLength, SifRpcRxBuffer, sizeof(int), NULL, NULL))>=0){
			result=*(int*)SifRpcRxBuffer;
		}

		PacketReqs.NumPackets=0;
		PacketReqs.TotalLength=0;
	}
	else result=0;

	return result;
}
