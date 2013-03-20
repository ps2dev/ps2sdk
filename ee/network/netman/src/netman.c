#include <errno.h>
#include <string.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_client.h"
#include "rpc_server.h"

static struct NetManNetProtStack MainNetProtStack;
static unsigned char IsInitialized=0;

void NetManToggleGlobalNetIFLinkState(unsigned char state){
	if(IsInitialized){
		if(state){
			MainNetProtStack.LinkStateUp();
		}
		else{
			MainNetProtStack.LinkStateDown();
		}
	}
}

int NetManInit(const struct NetManNetProtStack *stack){
	int result;

	if(!IsInitialized){
		if(((result=NetManInitRPCServer())==0) && ((result=NetManInitRPCClient())==0)){
			memcpy(&MainNetProtStack, stack, sizeof(MainNetProtStack));
			IsInitialized=1;
		}
	}
	else result=0;

	return result;
}

void NetManDeinit(void){
	NetManDeinitRPCClient();
	NetManDeinitRPCServer();
	memset(&MainNetProtStack, 0, sizeof(MainNetProtStack));

	IsInitialized=0;
}

int NetManNetIFSendPacket(const void *packet, unsigned int length){
	return IsInitialized?NetManRpcNetIFSendPacket(packet, length):-1;
}

int NetManIoctl(unsigned int command, void *arg, unsigned int arg_len, void *output, unsigned int length){
	return IsInitialized?NetManRpcIoctl(command, arg, arg_len, output, length):-1;
}

struct NetManPacketBuffer *NetManNetProtStackAllocRxPacket(unsigned int length){
	return IsInitialized?MainNetProtStack.AllocRxPacket(length):NULL;
}

void NetManNetProtStackFreeRxPacket(struct NetManPacketBuffer *packet){
	if(IsInitialized) MainNetProtStack.FreeRxPacket(packet);
}

int NetManNetProtStackEnQRxPacket(struct NetManPacketBuffer *packet){
	return IsInitialized?MainNetProtStack.EnQRxPacket(packet):-1;
}

int NetManNetProtStackFlushInputQueue(void){
	return IsInitialized?MainNetProtStack.FlushInputQueue():-1;
}
