#include <errno.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <malloc.h>
#include <netman.h>
#include <netman_rpc.h>

#include "rpc_client.h"

static SifRpcClientData_t NETMAN_rpc_cd;
extern void *_gp;

static union
{
	s32 mode;
	struct NetManIoctl IoctlArgs;
	char netifName[NETMAN_NETIF_NAME_MAX_LEN];
	u8 buffer[128];
} TransmitBuffer ALIGNED(64);

static union
{
	s32 result;
	struct NetManRegNetworkStackResult NetStackResult;
	struct NetManIoctlResult IoctlResult;
	struct NetManQueryMainNetIFResult QueryMainNetIFResult;
	u8 buffer[128];
} ReceiveBuffer ALIGNED(64);

static int NetManIOSemaID = -1, NetManTxSemaID = -1;

static unsigned short int IOPFrameBufferWrPtr;
static u8 *IOPFrameBuffer = NULL; /* On the IOP side. */

static struct PacketReqs PacketReqs ALIGNED(64);

static int TxThreadID = -1;

static unsigned char TxThreadStack[0x1000] ALIGNED(16);
static void TxThread(void *arg);

static unsigned char IsInitialized = 0;

static void deinitCleanup(void)
{
	if (TxThreadID >= 0) {
		TerminateThread(TxThreadID);
		DeleteThread(TxThreadID);
		TxThreadID = -1;
	}
	if (NetManIOSemaID >= 0) {
		DeleteSema(NetManIOSemaID);
		NetManIOSemaID = -1;
	}
	if (NetManTxSemaID >= 0) {
		DeleteSema(NetManTxSemaID);
		NetManTxSemaID = -1;
	}
}

int NetManInitRPCClient(void)
{
	static const char NetManID[] = "NetMan";
	int result;
	ee_sema_t SemaData;
	ee_thread_t ThreadData;

	if (!IsInitialized) {
		SemaData.max_count = 1;
		SemaData.init_count = 1;
		SemaData.option = (u32)NetManID;
		SemaData.attr = 0;
		if ((NetManIOSemaID = CreateSema(&SemaData)) < 0) {
			deinitCleanup();
			return NetManIOSemaID;
		}

		SemaData.max_count = 1;
		SemaData.init_count = 1;
		SemaData.option = (u32)NetManID;
		SemaData.attr = 0;
		if ((NetManTxSemaID = CreateSema(&SemaData)) < 0) {
			deinitCleanup();
			return NetManTxSemaID;
		}

		ThreadData.func = &TxThread;
		ThreadData.stack = TxThreadStack;
		ThreadData.stack_size = sizeof(TxThreadStack);
		ThreadData.gp_reg = &_gp;
		ThreadData.initial_priority = 0x57; //Transmissions should have higher priority than the TCP/IP stack's threads.
		ThreadData.attr = ThreadData.option = 0;

		if ((TxThreadID = CreateThread(&ThreadData)) < 0) {
			deinitCleanup();
			return TxThreadID;
		}

		if ((result = StartThread(TxThreadID, NULL)) < 0) {
			deinitCleanup();
			return result;
		}

		while ((SifBindRpc(&NETMAN_rpc_cd, NETMAN_RPC_NUMBER, 0) < 0) || (NETMAN_rpc_cd.server == NULL))
			nopdelay();

		if ((result = SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_INIT, 0, NULL, 0, &ReceiveBuffer, sizeof(s32), NULL, NULL)) >= 0) {
			if ((result = ReceiveBuffer.result) == 0)
				IsInitialized = 1;
			else
				deinitCleanup();
		} else {
			deinitCleanup();
		}
	} else
		result = 0;

	return result;
}

int NetManRPCRegisterNetworkStack(void)
{
	int result;

	WaitSema(NetManIOSemaID);

	if ((result = SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_REG_NETWORK_STACK, 0, NULL, 0, &ReceiveBuffer, sizeof(struct NetManRegNetworkStackResult), NULL, NULL)) >= 0) {
		if ((result = ReceiveBuffer.NetStackResult.result) == 0) {
			IOPFrameBuffer = ReceiveBuffer.NetStackResult.FrameBuffer;
			memset(&PacketReqs, 0, sizeof(PacketReqs));
			IOPFrameBufferWrPtr = 0;
		}
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManRPCUnregisterNetworkStack(void)
{
	int result;

	WaitSema(NetManIOSemaID);

	result = SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_UNREG_NETWORK_STACK, 0, NULL, 0, NULL, 0, NULL, NULL);
	IOPFrameBuffer = NULL;
	IOPFrameBufferWrPtr = 0;

	SignalSema(NetManIOSemaID);

	return result;
}

void NetManDeinitRPCClient(void)
{
	if (IsInitialized) {
		WaitSema(NetManIOSemaID);

		SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_DEINIT, 0, NULL, 0, NULL, 0, NULL, NULL);
		deinitCleanup();

		IsInitialized = 0;
	}
}

int NetManRpcIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length)
{
	int result;
	struct NetManIoctl *IoctlArgs = &TransmitBuffer.IoctlArgs;

	WaitSema(NetManIOSemaID);

	IoctlArgs->command = command;
	memcpy(IoctlArgs->args, args, args_len);
	IoctlArgs->args_len = args_len;
	IoctlArgs->output = output;
	IoctlArgs->length = length;

	if ((result = SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_IOCTL, 0, &TransmitBuffer, sizeof(struct NetManIoctl), &ReceiveBuffer, sizeof(struct NetManIoctlResult), NULL, NULL)) >= 0) {
		result = ReceiveBuffer.IoctlResult.result;
		memcpy(output, ReceiveBuffer.IoctlResult.output, length);
	}

	SignalSema(NetManIOSemaID);

	return result;
}

static int NetmanTxWaitingThread = -1;

static void TxThread(void *arg)
{
	int ThreadToWakeUp, sent;

	while (1) {
		SleepThread();

		if (PacketReqs.count > 0) {
			WaitSema(NetManIOSemaID);
			if (SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SEND_PACKETS, 0, &PacketReqs, sizeof(PacketReqs), &ReceiveBuffer, sizeof(s32), NULL, NULL) >= 0)
				sent = ReceiveBuffer.result;
			else
				sent = 0;
			SignalSema(NetManIOSemaID);

			DI();
			PacketReqs.count -= sent;
			EI();
		}

		if (NetmanTxWaitingThread >= 0) {
			DI();
			ThreadToWakeUp = NetmanTxWaitingThread;
			NetmanTxWaitingThread = -1;
			EI();
			WakeupThread(ThreadToWakeUp);
		}
	}
}

//Only one thread can enter this critical section!
static void EnQFrame(const void *frame, unsigned int length)
{
	SifDmaTransfer_t dmat;
	int dmat_id, ThreadID;

	//Write back D-cache, before performing a DMA transfer.
	SifWriteBackDCache((void *)frame, length);

	//Wait for a spot to be freed up.
	while (PacketReqs.count + 1 >= NETMAN_RPC_BLOCK_SIZE) {
		ThreadID = GetThreadId();
		DI();
		NetmanTxWaitingThread = ThreadID;
		EI();
		WakeupThread(TxThreadID);
		SleepThread();
	}

	//Transfer to IOP RAM
	dmat.src = (void *)frame;
	dmat.dest = &IOPFrameBuffer[IOPFrameBufferWrPtr * NETMAN_MAX_FRAME_SIZE];
	dmat.size = length;
	dmat.attr = 0;
	while ((dmat_id = SifSetDma(&dmat, 1)) == 0) {
	};

	//Record the frame length.
	PacketReqs.length[IOPFrameBufferWrPtr] = length;

	DI();
	//Update the frame count.
	PacketReqs.count++;
	EI();

	//Increase write pointer by one position.
	IOPFrameBufferWrPtr = (IOPFrameBufferWrPtr + 1) % NETMAN_RPC_BLOCK_SIZE;

	//Signal the transmission thread that there are more frames to transmit.
	WakeupThread(TxThreadID);

	//Ensure that the frame is copied over before returning (so that the buffer can be freed).
	while (SifDmaStat(dmat_id) >= 0) {
	};
}

int NetManRpcNetIFSendPacket(const void *packet, unsigned int length)
{
	if (IOPFrameBuffer != NULL) {
		WaitSema(NetManTxSemaID);
		EnQFrame(packet, length);
		SignalSema(NetManTxSemaID);

		return 0;
	} else {
		return -1;
	}
}

int NetManSetMainIF(const char *name)
{
	int result;

	WaitSema(NetManIOSemaID);

	strncpy(TransmitBuffer.netifName, name, NETMAN_NETIF_NAME_MAX_LEN);
	TransmitBuffer.netifName[NETMAN_NETIF_NAME_MAX_LEN - 1] = '\0';
	if ((result = SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_MAIN_NETIF, 0, &TransmitBuffer, NETMAN_NETIF_NAME_MAX_LEN, &ReceiveBuffer, sizeof(s32), NULL, NULL)) >= 0)
		result = ReceiveBuffer.result;

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManQueryMainIF(char *name)
{
	int result;

	WaitSema(NetManIOSemaID);

	if ((result = SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_QUERY_MAIN_NETIF, 0, NULL, 0, &ReceiveBuffer, sizeof(struct NetManQueryMainNetIFResult), NULL, NULL)) >= 0) {
		if ((result = ReceiveBuffer.QueryMainNetIFResult.result) == 0) {
			strncpy(name, ReceiveBuffer.QueryMainNetIFResult.name, NETMAN_NETIF_NAME_MAX_LEN);
			name[NETMAN_NETIF_NAME_MAX_LEN - 1] = '\0';
		}
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManSetLinkMode(int mode)
{
	int result;

	WaitSema(NetManIOSemaID);

	TransmitBuffer.mode = mode;
	if ((result = SifCallRpc(&NETMAN_rpc_cd, NETMAN_IOP_RPC_FUNC_SET_LINK_MODE, 0, &TransmitBuffer, sizeof(s32), &ReceiveBuffer, sizeof(s32), NULL, NULL)) >= 0)
		result = ReceiveBuffer.result;

	SignalSema(NetManIOSemaID);

	return result;
}
