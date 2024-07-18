#include <errno.h>
#include <string.h>
#include <kernel.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_client.h"
#include "rpc_server.h"

#ifdef F___netman_MainNetProtStack
struct NetManNetProtStack __netman_MainNetProtStack;
#else
extern struct NetManNetProtStack __netman_MainNetProtStack;
#endif

#ifdef F___netman_IsInitialized
unsigned char __netman_IsInitialized=0;
#else
extern unsigned char __netman_IsInitialized;
#endif

#ifdef F___netman_IsNetStackInitialized
unsigned char __netman_IsNetStackInitialized=0;
#else
extern unsigned char __netman_IsNetStackInitialized;
#endif

#ifdef F___netman_NIFLinkState
char __netman_NIFLinkState = 0;
#else
extern char __netman_NIFLinkState;
#endif

static inline void NetManUpdateStack__netman_NIFLinkState(void)
{
	if(__netman_IsNetStackInitialized)
	{
		if(__netman_NIFLinkState)
			__netman_MainNetProtStack.LinkStateUp();
		else
			__netman_MainNetProtStack.LinkStateDown();
	}
}

#ifdef F_NetManToggleGlobalNetIFLinkState
/*	Upon stack registration on the IOP side, this function will be called.
	But the network stack won't be updated then because the RPC wouldn't have been completely initialized,
	which would prevent the stack from performing some actions like sending Gratuitous Arp packets.

	The NIF status updated will be sent again when the RPC is fully initialized, as shown in NetManRegisterNetworkStack().	*/
void NetManToggleGlobalNetIFLinkState(unsigned char state)
{
	__netman_NIFLinkState = state;

	NetManUpdateStack__netman_NIFLinkState();
}
#endif

#ifdef F_NetManGetGlobalNetIFLinkState
int NetManGetGlobalNetIFLinkState(void)
{
	return __netman_NIFLinkState;
}
#endif

#ifdef F_NetManInit
int NetManInit(void)
{
	int result;

	if(!__netman_IsInitialized)
	{
		if((result=_NetManInitRPCServer())==0)
		{
			if((result=NetManInitRPCClient())==0) __netman_IsInitialized = 1;
		}
	}else result = 0;

	return result;
}
#endif

#ifdef F_NetManDeinit
void NetManDeinit(void)
{
	if(__netman_IsInitialized)
	{
		NetManUnregisterNetworkStack();

		NetManDeinitRPCClient();
		_NetManDeinitRPCServer();
		__netman_IsInitialized = 0;
	}
}
#endif

#ifdef F_NetManRegisterNetworkStack
int NetManRegisterNetworkStack(const struct NetManNetProtStack *stack)
{
	int result;

	if((result=NetManInit())==0)
	{
		if(!__netman_IsNetStackInitialized)
		{
			if((result=NetManRPCRegisterNetworkStack())==0)
			{
				memcpy(&__netman_MainNetProtStack, stack, sizeof(__netman_MainNetProtStack));
				__netman_IsNetStackInitialized=1;
				if((result=_NetManRPCAllocRxBuffers()) == 0)
					NetManUpdateStack__netman_NIFLinkState();
			}
		}
		else result=0;
	}

	return result;
}
#endif

#ifdef F_NetManUnregisterNetworkStack
void NetManUnregisterNetworkStack(void)
{
	if(__netman_IsNetStackInitialized)
	{
		NetManRPCUnregisterNetworkStack();
		memset(&__netman_MainNetProtStack, 0, sizeof(__netman_MainNetProtStack));

		__netman_IsNetStackInitialized=0;
	}
}
#endif

#ifdef F_NetManNetIFXmit
void NetManNetIFXmit(void)
{
	if(__netman_IsInitialized)
		NetManRpcNetIFXmit();
}
#endif

#ifdef F_NetManIoctl
int NetManIoctl(unsigned int command, void *arg, unsigned int arg_len, void *output, unsigned int length)
{
	return __netman_IsInitialized?NetManRpcIoctl(command, arg, arg_len, output, length):-1;
}
#endif

#ifdef F_NetManNetProtStackAllocRxPacket
void *NetManNetProtStackAllocRxPacket(unsigned int length, void **payload)
{
	return __netman_IsNetStackInitialized? __netman_MainNetProtStack.AllocRxPacket(length, payload) : NULL;
}
#endif

#ifdef F_NetManNetProtStackFreeRxPacket
void NetManNetProtStackFreeRxPacket(void *packet)
{
	if(__netman_IsNetStackInitialized) __netman_MainNetProtStack.FreeRxPacket(packet);
}
#endif

#ifdef F_NetManNetProtStackEnQRxPacket
void NetManNetProtStackEnQRxPacket(void *packet)
{
	if(__netman_IsNetStackInitialized)
		__netman_MainNetProtStack.EnQRxPacket(packet);
}
#endif

#ifdef F_NetManTxPacketNext
int NetManTxPacketNext(void **payload)
{
	return __netman_IsInitialized? __netman_MainNetProtStack.NextTxPacket(payload) : -1;
}
#endif

#ifdef F_NetManTxPacketDeQ
void NetManTxPacketDeQ(void)
{
	if(__netman_IsInitialized)
		__netman_MainNetProtStack.DeQTxPacket();
}
#endif

#ifdef F_NetManTxPacketAfter
int NetManTxPacketAfter(void **payload)
{
	return __netman_IsInitialized? __netman_MainNetProtStack.AfterTxPacket(payload) : -1;
}
#endif

#ifdef F_NetManNetProtStackReallocRxPacket
void NetManNetProtStackReallocRxPacket(void *packet, unsigned int length)
{
	if(__netman_IsNetStackInitialized) __netman_MainNetProtStack.ReallocRxPacket(packet, length);
}
#endif
