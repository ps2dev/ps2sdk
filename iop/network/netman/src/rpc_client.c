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
static unsigned short int EEFrameBufferWrPtr;

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
static SifCmdHeader_t *SifCmdBuffer = NULL;

int NetManInitRPCClient(void)
{
	static const char NetManID[] = "NetMan";
	iop_sema_t sema;
	int result, i;

	if(FrameBuffer == NULL) FrameBuffer = malloc(NETMAN_RPC_BLOCK_SIZE * NETMAN_MAX_FRAME_SIZE);
	if(FrameBufferStatus == NULL) FrameBufferStatus = malloc(NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));
	if(SifCmdBuffer == NULL) SifCmdBuffer = malloc(NETMAN_RPC_BLOCK_SIZE * sizeof(SifCmdHeader_t));

	if(FrameBuffer != NULL && FrameBufferStatus != NULL && SifCmdBuffer != NULL)
	{
		memset(FrameBuffer, 0, NETMAN_RPC_BLOCK_SIZE * NETMAN_MAX_FRAME_SIZE);
		memset(FrameBufferStatus, 0, NETMAN_RPC_BLOCK_SIZE * sizeof(struct NetManBD));
		EEFrameBufferWrPtr = 0;

		for(i = 0; i < NETMAN_RPC_BLOCK_SIZE; i++)	//Mark all descriptors as "in-use", until the EE-side allocates buffers.
			FrameBufferStatus[i].length = UINT_MAX;

		while((result=sceSifBindRpc(&EEClient, NETMAN_RPC_NUMBER, 0))<0 || EEClient.server==NULL) DelayThread(500);

		if((result=sceSifCallRpc(&EEClient, NETMAN_EE_RPC_FUNC_INIT, 0, &FrameBufferStatus, 4, &SifRpcRxBuffer, sizeof(struct NetManEEInitResult), NULL, NULL))>=0)
		{
			if((result=SifRpcRxBuffer.EEInitResult.result) == 0)
			{
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
	if(SifCmdBuffer != NULL)
	{
		free(SifCmdBuffer);
		SifCmdBuffer = NULL;
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
static void EnQFrame(const void *frame, unsigned int length)
{
	SifCmdHeader_t *pcmd;
	struct NetManPktCmd *cmd;
	struct NetManBD *bd;

	//No need to wait for a free spot to appear, as Alloc already took care of that.
	pcmd = &SifCmdBuffer[EEFrameBufferWrPtr];
	bd = &FrameBufferStatus[EEFrameBufferWrPtr];

	//Prepare SIFCMD packet.
	//Record the frame length.
	cmd = (struct NetManPktCmd*)&pcmd->opt;
	cmd->id = EEFrameBufferWrPtr;
	cmd->length = length;
	bd->length = length;

	//Transfer the frame over to the EE
	//Notify the receive thread of the incoming frame.
	while(sceSifSendCmd(NETMAN_SIFCMD_ID, pcmd, sizeof(SifCmdHeader_t),
				(void*)frame,
				bd->payload,
				(length + 3) & ~3) == 0){ };

	//Increase the write (IOP -> EE) pointer by one place.
	EEFrameBufferWrPtr = (EEFrameBufferWrPtr + 1) % NETMAN_RPC_BLOCK_SIZE;
}

//Frames will be enqueued in the order that they were allocated.
void NetManRpcProtStackEnQRxPacket(void *packet)
{
	EnQFrame(((struct NetManPacketBuffer*)packet)->payload, ((struct NetManPacketBuffer*)packet)->length);
}
