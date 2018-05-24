#include <errno.h>
#include <string.h>
#include <kernel.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_client.h"
#include "rpc_server.h"

static struct NetManNetProtStack MainNetProtStack;
static unsigned char IsInitialized=0, IsNetStackInitialized=0;
static char NIFLinkState = 0;

/*	Upon stack registration on the IOP side, this function will be called.
	But the network stack won't be updated then because the RPC wouldn't have been completely initialized,
	which would prevent the stack from performing some actions like sending Gratuitous Arp packets.

	The NIF status updated will be sent again when the RPC is fully initialized, as shown in NetManRegisterNetworkStack().	*/
void NetManToggleGlobalNetIFLinkState(unsigned char state)
{
	NIFLinkState = state;

	NetManUpdateStackNIFLinkState();
}

int NetManGetGlobalNetIFLinkState(void)
{
	return NIFLinkState;
}

void NetManUpdateStackNIFLinkState(void)
{
	if(IsNetStackInitialized)
	{
		if(NIFLinkState)
			MainNetProtStack.LinkStateUp();
		else
			MainNetProtStack.LinkStateDown();
	}
}

int NetManInit(void)
{
	int result;

	if(!IsInitialized)
	{
		if((result=NetManInitRPCServer())==0)
		{
			if((result=NetManInitRPCClient())==0) IsInitialized = 1;
		}
	}else result = 0;

	return result;
}

void NetManDeinit(void)
{
	if(IsInitialized)
	{
		NetManUnregisterNetworkStack();

		NetManDeinitRPCClient();
		NetManDeinitRPCServer();
		IsInitialized = 0;
	}
}

int NetManRegisterNetworkStack(const struct NetManNetProtStack *stack)
{
	int result;

	if((result=NetManInit())==0)
	{
		if(!IsNetStackInitialized)
		{
			if((result=NetManRPCRegisterNetworkStack())==0)
			{
				memcpy(&MainNetProtStack, stack, sizeof(MainNetProtStack));
				IsNetStackInitialized=1;
				if((result=NetManRPCAllocRxBuffers()) == 0)
					NetManUpdateStackNIFLinkState();
			}
		}
		else result=0;
	}

	return result;
}

void NetManUnregisterNetworkStack(void)
{
	if(IsNetStackInitialized)
	{
		NetManRPCUnregisterNetworkStack();
		memset(&MainNetProtStack, 0, sizeof(MainNetProtStack));

		IsNetStackInitialized=0;
	}
}

void NetManNetIFXmit(void)
{
	if(IsInitialized)
		NetManRpcNetIFXmit();
}

int NetManIoctl(unsigned int command, void *arg, unsigned int arg_len, void *output, unsigned int length)
{
	return IsInitialized?NetManRpcIoctl(command, arg, arg_len, output, length):-1;
}

void *NetManNetProtStackAllocRxPacket(unsigned int length, void **payload)
{
	return IsNetStackInitialized?MainNetProtStack.AllocRxPacket(length, payload):NULL;
}

void NetManNetProtStackFreeRxPacket(void *packet)
{
	if(IsNetStackInitialized) MainNetProtStack.FreeRxPacket(packet);
}

void NetManNetProtStackEnQRxPacket(void *packet)
{
	if(IsNetStackInitialized)
		MainNetProtStack.EnQRxPacket(packet);
}

int NetManTxPacketNext(void **payload)
{
	return IsInitialized?MainNetProtStack.NextTxPacket(payload):-1;
}

void NetManTxPacketDeQ(void)
{
	if(IsInitialized)
		MainNetProtStack.DeQTxPacket();
}

int NetManTxPacketAfter(void **payload)
{
	return IsInitialized?MainNetProtStack.AfterTxPacket(payload):-1;
}

void NetManNetProtStackReallocRxPacket(void *packet, unsigned int length)
{
	if(IsNetStackInitialized) MainNetProtStack.ReallocRxPacket(packet, length);
}
