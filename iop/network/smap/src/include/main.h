//In the SONY original, all the calls to DEBUG_PRINTF() were to sceInetPrintf().
#define DEBUG_PRINTF(args...) printf(args)

/*
	Sorry, but even I can't explain the syntax used here. :(
	I know that _ori_gp has to be "early-clobbered" and the GP register will get clobbered... but I don't really know why GCC can't determine which registers it can and can't use automatically. And I don't really understand what "clobbering" registers is.
*/
#define SaveGP() \
	void *_ori_gp;	\
	__asm volatile("move %0, $gp\n"	\
	"move $gp, %1" :"=&r"(_ori_gp): "r"(&_gp) : "gp")

#define RestoreGP() \
	__asm volatile("move $gp, %0" :: "r"(_ori_gp) : "gp")

struct RuntimeStats{
	u32 RxDroppedFrameCount;
	u32 RxErrorCount;
	u16 RxFrameOverrunCount;
	u16 RxFrameBadLengthCount;
	u16 RxFrameBadFCSCount;
	u16 RxFrameBadAlignmentCount;
	u32 TxDroppedFrameCount;
	u32 TxErrorCount;
	u16 TxFrameLOSSCRCount;
	u16 TxFrameEDEFERCount;
	u16 TxFrameCollisionCount;
	u16 TxFrameUnderrunCount;
	u16 RxAllocFail;
};

struct SmapDriverData{
	volatile u8 *smap_regbase;
	volatile u8 *emac3_regbase;
	unsigned int TxBufferSpaceAvailable;
	unsigned char NumPacketsInTx;
	unsigned char TxBDIndex;
	unsigned char TxDNVBDIndex;
	unsigned char RxBDIndex;
	void *packetToSend;
	int Dev9IntrEventFlag;
	int IntrHandlerThreadID;
	unsigned char SmapDriverStarted;	//SMAP driver is started.
	unsigned char SmapIsInitialized;	//SMAP driver is initialized (software)
	unsigned char NetDevStopFlag;
	unsigned char EnableLinkCheckTimer;
	unsigned char LinkStatus;		//Ethernet link is initialized (hardware)
	unsigned char LinkMode;
	iop_sys_clock_t LinkCheckTimer;
	struct RuntimeStats RuntimeStats;
	int NetIFID;
};

/* Event flag bits */
#define SMAP_EVENT_START	0x01
#define SMAP_EVENT_STOP		0x02
#define SMAP_EVENT_INTR		0x04
#define SMAP_EVENT_XMIT		0x08
#define SMAP_EVENT_LINK_CHECK	0x10

/* Function prototypes */
int DisplayBanner(void);
int smap_init(int argc, char *argv[]);
int SMAPStart(void);
void SMAPStop(void);
void SMAPXmit(void);
int SMAPGetMACAddress(u8 *buffer);

#include "xfer.h"
