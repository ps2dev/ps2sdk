#include <errno.h>
#include <intrman.h>
#include <limits.h>
#include <sifcmd.h>
#include <sifman.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#include <netman.h>
#include <netman_rpc.h>

#include "internal.h"
#include "rpc_client.h"

static SifRpcClientData_t EEClient;
static union{
	s32 result;
	struct NetManEEInitResult EEInitResult;
	u8 buffer[64];
} SifRpcRxBuffer;
static union{
	s32 state;
	u8 buffer[64];
} SifRpcTxBuffer;

//Data for IOP -> EE transfers
static unsigned short int EEFrameBufferWrPtr, NumFramesInQueue;
static SifDmaTransfer_t dmatReqs[NETMAN_FRAME_GROUP_SIZE*2];

static int NetManIOSemaID = -1;

struct NetManPacketBuffer{
	void *handle;
	void *payload;
	u32 length;
};

//Data for SPEED -> IOP transfers
static struct NetManPacketBuffer pbufs[NETMAN_RPC_BLOCK_SIZE];
static u8 *FrameBuffer = NULL;
static struct NetManBD *FrameBufferStatus = NULL;
static struct NetManBD *EEFrameBufferStatus = NULL;

int NetManInitRPCClient(void)
{
	static const char NetManID[] = "NetMan";
	iop_sema_t sema;
	int result;

	if(FrameBuffer == NULL) FrameBuffer = malloc(NETMAN_RPC_BLOCK_SIZE * NETMAN_MAX_FRAME_SIZE);
	if(FrameBufferStatus == NULL) FrameBufferStatus = malloc(NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));

	if(FrameBuffer != NULL && FrameBufferStatus != NULL)
	{
		int i;

		memset(FrameBuffer, 0, NETMAN_RPC_BLOCK_SIZE * NETMAN_MAX_FRAME_SIZE);
		memset(FrameBufferStatus, 0, NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));
		EEFrameBufferWrPtr = 0;
		NumFramesInQueue = 0;

		for(i = 0; i < NETMAN_RPC_BLOCK_SIZE; i++)	//Mark all descriptors as "in-use", until the EE-side allocates buffers.
			FrameBufferStatus[i].length = USHRT_MAX;

		while((result=sceSifBindRpc(&EEClient, NETMAN_RPC_NUMBER, 0))<0 || EEClient.server==NULL) DelayThread(500);

		if((result=sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_INIT, 0, &FrameBufferStatus, 4, &SifRpcRxBuffer, sizeof(struct NetManEEInitResult), NULL, NULL))>=0)
		{
			if((result=SifRpcRxBuffer.EEInitResult.result) == 0)
			{
				EEFrameBufferStatus = SifRpcRxBuffer.EEInitResult.FrameBufferStatus;

				sema.attr = 0;
				sema.option = (u32)NetManID;
				sema.initial = 1;
				sema.max = 1;
				NetManIOSemaID = CreateSema(&sema);
			}
		}
	}else result = -ENOMEM;

	return result;
}

void NetManDeinitRPCClient(void)
{
	if(FrameBuffer != NULL)
	{
		free(FrameBuffer);
		FrameBuffer = NULL;
	}
	if(FrameBufferStatus != NULL)
	{
		free(FrameBufferStatus);
		FrameBufferStatus = NULL;
	}
	if(NetManIOSemaID >= 0)
	{
		DeleteSema(NetManIOSemaID);
		NetManIOSemaID = -1;
	}

	memset(&EEClient, 0, sizeof(EEClient));
}

void NetManRpcToggleGlobalNetIFLinkState(int state)
{
	WaitSema(NetManIOSemaID);
	SifRpcTxBuffer.state=state;
	sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_HANDLE_LINK_STATUS_CHANGE, 0, &SifRpcTxBuffer, sizeof(s32), NULL, 0, NULL, NULL);
	SignalSema(NetManIOSemaID);
}

//For this implementation, a packet buffer is only taken if it is enqueued.
void *NetManRpcNetProtStackAllocRxPacket(unsigned int length, void **payload)
{
	struct NetManPacketBuffer *result;
	volatile const struct NetManBD *bd = &FrameBufferStatus[EEFrameBufferWrPtr];

	//Wait for a free spot to appear in the ring buffer.
	while(bd->length != 0) { }

	//Allocation of PBUF descriptors is tied with the allocation of frame slots in the ring buffer by EnQ.
	result = &pbufs[EEFrameBufferWrPtr];
	result->handle = (void*)bd;
	result->payload = &FrameBuffer[EEFrameBufferWrPtr * NETMAN_MAX_FRAME_SIZE];
	result->length = length;

	//The write pointer is not incremented here because the interface driver might discard the frame and free this allocated buffer.

	*payload = result->payload;

	return result;
}

void NetManRpcNetProtStackFreeRxPacket(void *packet)
{
	((struct NetManPacketBuffer*)packet)->handle = NULL;
}

//Only one thread can enter this critical section!
static int sendFramesToEE(int mode)
{
	int OldState;

	if (NumFramesInQueue > 0)
	{
		int res;

		dmatReqs[(NumFramesInQueue-1) * 2 + 1].attr = SIF_DMA_INT_O;	//Mark the last entry to notify the receive thread of the incoming frame(s). This will stall SIF0.

		//Transfer the frame over to the EE
		do{
			if (mode == 0)
				CpuSuspendIntr(&OldState);
			res = sceSifSetDma(dmatReqs, 2*NumFramesInQueue);
			if (mode == 0)
				CpuResumeIntr(OldState);

			/*	In interrupt mode, do not loop around sceSifSetDma as its status can only be updated by the SIF0 interrupt handler,
				which would be blocked by this interrupt handler. */
			if (mode != 0 && res == 0)
				return -1;
		}while(res == 0);

		NumFramesInQueue = 0;
	}

	return 0;
}

//The SMAP Rx FIFO may only hold 16384 / 1518 = 10 frames. In 1ms, roughly 8 full-length frames can be transferred at 100Mbit within 1ms. 1ms = 36864 ticks.
#define FRAME_GROUPING_INTERVAL	36864

static unsigned int FrameSendCB(void *arg)
{
	(void)arg;

	return(sendFramesToEE(1) == 0 ? 0 : FRAME_GROUPING_INTERVAL); //If sending failed, try again later.
}

//Only one thread can enter this critical section!
static void EnQFrame(const void *frame, unsigned int length)
{
	SifDmaTransfer_t *dmat;
	struct NetManBD *bd;
	iop_sys_clock_t clock;

	//Cancel any ongoing callbacks.
	CancelAlarm(&FrameSendCB, NULL);

	if (NumFramesInQueue >= NETMAN_FRAME_GROUP_SIZE)
	{	/* If there are already sufficient frames, the frames can be sent right away.
		   This may happen here if sending failed within the interrupt callback and there are more frames to send. */
		sendFramesToEE(0);
	}

	//No need to wait for a free spot to appear, as Alloc already took care of that.
	bd = &FrameBufferStatus[EEFrameBufferWrPtr];

	//Record the frame length.
	bd->length = length;

	//Prepare DMA transfer.
	dmat = &dmatReqs[NumFramesInQueue * 2];
	dmat[0].src = (void*)frame;
	dmat[0].dest = bd->payload;
	dmat[0].size = (length + 3) & ~3;
	dmat[0].attr = 0;

	dmat[1].src = bd;
	dmat[1].dest = &EEFrameBufferStatus[EEFrameBufferWrPtr];
	dmat[1].size = sizeof(struct NetManBD);
	dmat[1].attr = 0;
	NumFramesInQueue++;

	//Increase the write (IOP -> EE) pointer by one place.
	EEFrameBufferWrPtr = (EEFrameBufferWrPtr + 1) % NETMAN_RPC_BLOCK_SIZE;

	if (NumFramesInQueue >= NETMAN_FRAME_GROUP_SIZE)
	{	//If there are sufficient frames, the frames can be sent right away.
		sendFramesToEE(0);
	} else {
		//Wait a while in case further frames can be grouped, to allow sceSifSetDma() to chain the requests together.
		clock.lo = FRAME_GROUPING_INTERVAL;
		clock.hi = 0;
		SetAlarm(&clock, &FrameSendCB, NULL);
	}
}

//Frames will be enqueued in the order that they were allocated.
void NetManRpcProtStackEnQRxPacket(void *packet)
{
	EnQFrame(((struct NetManPacketBuffer*)packet)->payload, ((struct NetManPacketBuffer*)packet)->length);
}
