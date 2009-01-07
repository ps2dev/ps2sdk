/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# USB Driver function prototypes and constants.
*/
#ifndef __USBDPRIV_H__
#define __USBDPRIV_H__

#include "usbd.h"
#include "types.h"

#define OHCI_REG_BASE 0xBF801600

#ifdef DEBUG
#define dbg_printf			printf
#else
#define dbg_printf(a...)	(void)0
#endif

#define READ_UINT16(a) (((uint8*)a)[0] | (((uint8 *)a)[1] << 8))

typedef struct {
	int maxDevices;
	int maxEndpoints;
	int maxTransfDesc;
	int maxIsoTransfDesc;
	int maxIoReqs;
	int maxStaticDescSize;
	int maxHubDevices;
	int maxPortsPerHub;

	int hcdThreadPrio;
    int cbThreadPrio;	
} UsbdConfig;

extern UsbdConfig usbConfig;

struct _device;
struct _ioRequest;
struct _hcTd;
struct _hcIsoTd;
struct _hcEd;
struct _endpoint;
struct _UsbDriver;
struct _ioRequest;

typedef void (*TimerCallback)(void *arg);
typedef void (*InternCallback)(struct _ioRequest *arg);

typedef struct _timerCbStruct {
	uint32 isActive;
	struct _timerCbStruct *prev, *next;
	TimerCallback callbackProc;
	void   *callbackArg;
	uint32 delayCount;
} TimerCbStruct __attribute__ ((packed));

typedef struct _ioRequest {
	uint32 busyFlag;
	struct _ioRequest *next, *prev;
	struct _endpoint *correspEndpoint;
	UsbDeviceRequest devReq;
    void   *destPtr;
	uint32 length;	// length of destPtr buffer
	InternCallback callbackProc;
	uint32 resultCode;
	uint32 transferedBytes;
	uint32 waitFrames; // number of frames to wait for isochronous transfers
	UsbCallbackProc userCallbackProc;
	void   *userCallbackArg;
    void   *gpSeg;
} IoRequest __attribute__ ((packed));

typedef struct _device {
	uint32 id;
	struct _device *next, *prev;
	struct _device *nextConnected, *prevConnected;
	struct _endpoint *endpointListStart, *endpointListEnd;
	UsbDriver  *devDriver;
	uint8  deviceStatus;
	uint8  functionAddress;
	uint8  isLowSpeedDevice;
	uint8  resetFlag;
	struct _device *childListStart, *childListEnd;
	struct _device *parent;
	uint32 attachedToPortNo;
	void   *privDataField;
	TimerCbStruct timer;
	IoRequest ioRequest;
	uint32 functionDelay;		// is this necessary?
	void   *staticDeviceDescPtr;
	void   *staticDeviceDescEndPtr;
	uint32 fetchDescriptorCounter;
} Device __attribute__ ((packed));

typedef struct _hcTd {
	uint32 HcArea;
	void   *curBufPtr;
	struct _hcTd *next;
	void   *bufferEnd;
} HcTD __attribute__ ((packed));

typedef struct _hcIsoTd {
	uint32 hcArea;
	void   *bufferPage0;
	struct _hcIsoTd *next;
	void   *bufferEnd;
	uint16 psw[8];
} HcIsoTD __attribute__ ((packed));

typedef struct _hcEd {
	uint16 hcArea;
	uint16 maxPacketSize;
	HcTD   *tdTail;
	HcTD   *tdHead;
	struct _hcEd *next;
} HcED __attribute__ ((packed));

typedef struct _endpoint {
	uint32			id;
	uint8			endpointType;
	uint8			inTdQueue;
	uint8			alignFlag;
	uint8			pad;
	struct _endpoint *next, *prev;
	struct _endpoint *busyNext, *busyPrev;
	Device		*correspDevice;
	IoRequest	*ioReqListStart;
	IoRequest	*ioReqListEnd;
	uint32		isochronLastFrameNum; // 40
	TimerCbStruct timer;	// sizeof(TimerCbStruct) => 24 bytes
	HcED		hcEd;		// HcED has to be aligned to 0x10 bytes!
} Endpoint __attribute__ ((packed));

typedef struct _usbHub {
	struct _usbHub  *next;
	Endpoint *controlEp, *statusChangeEp;
    IoRequest controlIoReq, statusIoReq;
	UsbHubDescriptor desc;
    uint32 numChildDevices;
	uint32 portCounter;
	uint32 hubStatusCounter;
	uint16 hubStatus;			//
	uint16 hubStatusChange;		// unite to uint32 to make it match portStatusChange
	uint32 portStatusChange;
	uint8 statusChangeInfo[8]; // depends on number of ports
} UsbHub __attribute__ ((packed));

typedef struct {
	volatile HcED   *InterruptTable[32];
	volatile uint16 FrameNumber;
	volatile uint16 pad;
	volatile HcTD   *DoneHead;
	volatile uint8  reserved[116];
	volatile uint32 pad2; // expand struct to 256 bytes for alignment
} HcCA __attribute__ ((packed));

typedef struct {
	volatile uint32 HcRevision;
	volatile uint32 HcControl;
	volatile uint32 HcCommandStatus;
	volatile uint32 HcInterruptStatus;
	volatile uint32 HcInterruptEnable;
	volatile uint32 HcInterruptDisable;
	volatile HcCA   *HcHCCA;
	volatile HcED   *HcPeriodCurrentEd;
	volatile HcED   *HcControlHeadEd;
	volatile HcED	*HcControlCurrentEd;
	volatile HcED   *HcBulkHeadEd;
	volatile HcED	*HcBulkCurrentEd;
	volatile uint32 HcDoneHead;
	volatile uint32 HcFmInterval;
	volatile uint32 HcFmRemaining;
	volatile uint32 HcFmNumber;
	volatile uint32 HcPeriodicStart;
	volatile uint32 HcLsThreshold;
	volatile uint32 HcRhDescriptorA;
	volatile uint32 HcRhDescriptorB;
	volatile uint32 HcRhStatus;
	volatile uint32 HcRhPortStatus[2];
} OhciRegs __attribute__ ((packed));

typedef struct _memPool {
	volatile OhciRegs	*ohciRegs;
	volatile HcCA		*hcHCCA;

	struct _hcEd		*hcEdBuf;
	
	struct _hcTd		*freeHcTdList;
	struct _hcTd		*hcTdBuf;
	struct _hcTd		*hcTdBufEnd;

	struct _hcIsoTd		*freeHcIsoTdList;
	struct _hcIsoTd		*hcIsoTdBuf;
	struct _hcIsoTd		*hcIsoTdBufEnd;
	
	struct _ioRequest	**hcTdToIoReqLUT;
	struct _ioRequest	**hcIsoTdToIoReqLUT;
	
	struct _ioRequest	*ioReqBufPtr;
	struct _ioRequest	*freeIoReqList;
	struct _ioRequest	*freeIoReqListEnd;
	
	struct _device		*deviceTreeBuf;
	struct _device		*freeDeviceListStart;
	struct _device		*freeDeviceListEnd;
	
	struct _endpoint	*endpointBuf;
	struct _endpoint	*freeEpListStart;
	struct _endpoint	*freeEpListEnd;

	struct _endpoint	*tdQueueStart[2], *tdQueueEnd[2];

	struct _timerCbStruct *timerListStart;
	struct _timerCbStruct *timerListEnd;

	struct _device		*deviceTreeRoot;

	struct _device		*deviceConnectedListStart;
	struct _device		*deviceConnectedListEnd;

	uint32 delayResets;
} MemoryPool;

#define GENTD_QUEUE 1
#define ISOTD_QUEUE 2

#define TYPE_CONTROL	0x3F
#define TYPE_BULK		0x40
#define TYPE_ISOCHRON	0x41

#define DEVICE_NOTCONNECTED		0
#define DEVICE_CONNECTED		1
#define DEVICE_RESETDELAYED		2
#define DEVICE_RESETPENDING		3
#define DEVICE_RESETCOMPLETE	4
#define DEVICE_READY			5

#define PORT_CONNECTION		0
#define PORT_ENABLE			1
#define PORT_SUSPEND		2
#define PORT_OVER_CURRENT	3
#define PORT_RESET			4
#define PORT_POWER			8
#define PORT_LOW_SPEED		9

#define C_HUB_LOCAL_POWER	0
#define C_HUB_OVER_CURRENT	1

#define C_PORT_CONNECTION	16
#define C_PORT_ENABLE		17
#define C_PORT_SUSPEND		18
#define C_PORT_OVER_CURRENT	19
#define C_PORT_RESET		20

#define BIT(x)	(1 << (x))

#define C_PORT_FLAGS		(BIT(C_PORT_CONNECTION) | BIT(C_PORT_ENABLE) | BIT(C_PORT_SUSPEND) | BIT(C_PORT_OVER_CURRENT) | BIT(C_PORT_RESET))

#define HCED_DIR_OUT	BIT(11)		// Direction field
#define HCED_DIR_IN		BIT(12)		// Direction field
#define HCED_SPEED		BIT(13)		// Speed bit
#define HCED_SKIP		BIT(14)		// sKip bit
#define HCED_ISOC		BIT(15)		// Format bit
#define HCED_DIR_MASK	(HCED_DIR_OUT | HCED_DIR_IN)

#define ED_HALTED(a)	((uint32)((a).tdHead) & 1)
#define ED_SKIPPED(a)	((uint32)((a).hcArea) & HCED_SKIP)

#define TD_HCAREA(CC, T, DI, DP, R)		(((CC) << 12) | ((T) << 8) | ((DI) << 5) | ((DP) << 3) | ((R) << 2))

#define TD_SETUP	0
#define TD_OUT		1
#define TD_IN		2

#define OHCI_INT_SO		BIT(0)
#define OHCI_INT_WDH	BIT(1)
#define OHCI_INT_SF		BIT(2)
#define OHCI_INT_RD		BIT(3)
#define OHCI_INT_UE		BIT(4)
#define OHCI_INT_FNO	BIT(5)
#define OHCI_INT_RHSC	BIT(6)
#define OHCI_INT_OC		BIT(30)
#define OHCI_INT_MIE	BIT(31)

#define OHCI_COM_HCR	BIT(0)
#define OHCI_COM_CLF	BIT(1)
#define OHCI_COM_BLF	BIT(2)

#define OHCI_CTR_PLE	BIT(2)	// Periodic List Enable
#define OHCI_CTR_IE		BIT(3)	// Isochronous Enable
#define OHCI_CTR_CLE	BIT(4)	// Control List Enable
#define OHCI_CTR_BLE	BIT(5)	// Bulk List Enable
#define OHCI_CTR_USB_RESET		 (0 << 6)
#define OHCI_CTR_USB_RESUME		 (1 << 6)
#define OHCI_CTR_USB_OPERATIONAL (2 << 6)
#define OHCI_CTR_USB_SUSPEND	 (3 << 6)

int usbdLock(void);
int usbdUnlock(void);
int doGetDeviceLocation(Device *dev, uint8 *path);
void processDoneQueue_IsoTd(HcIsoTD *arg);
void processDoneQueue_GenTd(HcTD *arg);
void handleTimerList(void);


#endif // __USBDPRIV_H__

