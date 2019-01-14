//In the SONY original, all the calls to DEBUG_PRINTF() were to sceInetPrintf().
#define DEBUG_PRINTF(args...) printf(args)

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
	struct NetManEthRuntimeStats RuntimeStats;
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
