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
static unsigned char rpc_buffer[NETMAN_RPC_BUFF_SIZE];
static SifRpcDataQueue_t rpc_qdata;

static union{
	s32 result;
	struct NetManIoctlResult IoctlResult;
	struct NetManQueryMainNetIFResult QueryMainNetIFResult;
	struct NetManRegNetworkStackResult RegNetworkStackResult;
	u8 buffer[80];
} SifRpcTxBuffer;

/* Packet transmission buffer. The EE will DMA transfer the packet to be transmitted directly into this buffer before invoking FUNC_SEND_PACKET. */
static u8 *TxFrameBuffer = NULL;
static unsigned short int IOPFrameBufferRdPtr;

static int RpcThreadID = -1;
static unsigned char IsInitialized=0, IsRpcStackInitialized=0;

static void LinkStateUp(void)
{
	NetManRpcToggleGlobalNetIFLinkState(1);
}

static void LinkStateDown(void)
{
	NetManRpcToggleGlobalNetIFLinkState(0);
}

static void *AllocRxPacket(unsigned int length, void **payload)
{
	return NetManRpcNetProtStackAllocRxPacket(length, payload);
}

static void FreeRxPacket(void *packet)
{
	NetManRpcNetProtStackFreeRxPacket(packet);
}

static void EnQRxPacket(void *packet)
{
	NetManRpcProtStackEnQRxPacket(packet);
}

static struct NetManNetProtStack RpcStack={
	&LinkStateUp,
	&LinkStateDown,
	&AllocRxPacket,
	&FreeRxPacket,
	&EnQRxPacket
};

static void unregisterEENetworkStack(void)
{
	if(IsRpcStackInitialized)
	{
		IsRpcStackInitialized=0;
		NetManUnregisterNetworkStack();

		if(TxFrameBuffer != NULL)
		{
			free(TxFrameBuffer);
			TxFrameBuffer = NULL;
		}

		IOPFrameBufferRdPtr = 0;
	}
}

static void *NETMAN_rpc_handler(int fno, void *buffer, int size)
{
	static int ResultValue;
	void *result;
	unsigned int i;

	switch(fno)
	{
		case NETMAN_IOP_RPC_FUNC_INIT:
			IOPFrameBufferRdPtr = 0;
			ResultValue=NetManInitRPCClient();
			result=&ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_DEINIT:
			unregisterEENetworkStack();
			NetManDeinitRPCClient();
			result=NULL;
			break;
		case NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK:
			result = &SifRpcTxBuffer;

			if(TxFrameBuffer == NULL) TxFrameBuffer = malloc(NETMAN_MAX_FRAME_SIZE*NETMAN_RPC_BLOCK_SIZE);

			if(TxFrameBuffer != NULL)
			{
				SifRpcTxBuffer.RegNetworkStackResult.FrameBuffer = TxFrameBuffer;
				ResultValue = NetManRegisterNetworkStack(&RpcStack);
			}else{
				ResultValue = -ENOMEM;
			}

			IsRpcStackInitialized=1;
			SifRpcTxBuffer.RegNetworkStackResult.result=ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK:
			unregisterEENetworkStack();
			result=NULL;
			break;
		case NETMAN_IOP_RPC_FUNC_SEND_PACKETS:
			if(TxFrameBuffer!=NULL)
			{
				for(i=0; i<((struct PacketReqs*)buffer)->count; i++)
				{
					NetManNetIFSendPacket(&TxFrameBuffer[IOPFrameBufferRdPtr * NETMAN_MAX_FRAME_SIZE], ((struct PacketReqs*)buffer)->length[IOPFrameBufferRdPtr]);
					//Increment read pointer by one place.
					IOPFrameBufferRdPtr = (IOPFrameBufferRdPtr + 1) % NETMAN_RPC_BLOCK_SIZE;
				}

				ResultValue = i;
			}else ResultValue = 0;

			result=&ResultValue;
			break;
		case NETMAN_IOP_RPC_FUNC_IOCTL:
			SifRpcTxBuffer.IoctlResult.result=NetManIoctl(((struct NetManIoctl*)buffer)->command, ((struct NetManIoctl*)buffer)->args, ((struct NetManIoctl*)buffer)->args_len, SifRpcTxBuffer.IoctlResult.output, ((struct NetManIoctl*)buffer)->length);
			result=&SifRpcTxBuffer;
			break;
		case NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF:
			SifRpcTxBuffer.result=NetManSetMainIF(buffer);
			result=&SifRpcTxBuffer;
			break;
		case NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF:
			SifRpcTxBuffer.QueryMainNetIFResult.result=NetManQueryMainIF(SifRpcTxBuffer.QueryMainNetIFResult.name);
			result=&SifRpcTxBuffer;
			break;
		case NETMAN_IOP_RPC_FUNC_SET_LINK_MODE:
			ResultValue=NetManSetLinkMode(*(s32*)buffer);
			result=&ResultValue;
			break;
		default:
			printf("NETMAN [IOP]: Unrecognized command: 0x%x\n", fno);
			ResultValue=-1;
			result=&ResultValue;
	}

	return result;
}

static void NETMAN_RPC_srv(void *args)
{
	sceSifSetRpcQueue(&rpc_qdata, RpcThreadID);

	sceSifRegisterRpc(&rpc_sdata, NETMAN_RPC_NUMBER, &NETMAN_rpc_handler, rpc_buffer, NULL, NULL, &rpc_qdata);
	sceSifRpcLoop(&rpc_qdata);
}

int NetmanInitRPCServer(void)
{
	iop_thread_t ThreadData;

	if(!IsInitialized)
	{
		ThreadData.attr=TH_C;
		ThreadData.option=0;
		ThreadData.thread=&NETMAN_RPC_srv;
		ThreadData.stacksize=0x600;
		ThreadData.priority=0x27;	//Should have higher priority than the IF driver thread, in order to dump frames in the IOP before returning.

		StartThread(RpcThreadID=CreateThread(&ThreadData), NULL);
		IsInitialized=1;
	}

	return 0;
}

void NetmanDeinitRPCServer(void)
{
	if(IsInitialized)
	{
		TerminateThread(RpcThreadID);
		DeleteThread(RpcThreadID);
		IsInitialized=0;
	}
}
