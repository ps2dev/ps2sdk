/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * USB Driver function prototypes and constants.
 */

#ifndef __USBDPRIV_H__
#define __USBDPRIV_H__

#include "irx_imports.h"

#include "defs.h"
#include "types.h"
#include "usbd.h"

#define OHCI_REG_BASE 0xBF801600

#ifdef DEBUG
#define dbg_printf(a...) printf("usbd: " a)
#else
#define dbg_printf(a...) (void)0
#endif

#define READ_UINT16(a) (((u8 *)a)[0] | (((u8 *)a)[1] << 8))

typedef struct _argOption
{
	const char *param;
	int *value;
	int *value2;
} UsbdArgOption_t;

typedef struct _usbdConfig
{
	int m_maxDevices;
	int m_maxEndpoints;
	int m_maxTransfDesc;
	int m_maxIsoTransfDesc;
	int m_maxIoReqs;
	int m_maxStaticDescSize;
	int m_maxHubDevices;
	int m_maxPortsPerHub;
	int m_allocatedSize_unused;
	int m_hcdThreadPrio;
	int m_cbThreadPrio;
	int m_curDescNum;
} UsbdConfig_t;

extern UsbdConfig_t usbConfig;

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

typedef struct _timerCbStruct
{
	u32 m_isActive;
	struct _timerCbStruct *m_prev;
	struct _timerCbStruct *m_next;
	TimerCallback m_callbackProc;
	void *m_callbackArg;
	u32 m_delayCount;
} UsbdTimerCbStruct_t;

typedef struct _ioRequest
{
	u32 m_id;
	u32 m_busyFlag;
	struct _ioRequest *m_next;
	struct _ioRequest *m_prev;
	struct _endpoint *m_correspEndpoint;
	UsbDeviceRequest m_devReq;
	void *m_destPtr;
	u32 m_length;  // length of m_destPtr buffer
	InternCallback m_callbackProc;
	u32 m_resultCode;
	u32 m_transferedBytes;
	u32 m_waitFrames;  // number of frames to wait for isochronous transfers
	void *m_userCallbackArg;
	union
	{
		void *m_userCallbackProc;
		sceUsbdDoneCallback m_userCallbackProcRegular;
		sceUsbdMultiIsochronousDoneCallback m_userCallbackProcMultiIsochronous;
	};
	void *m_gpSeg;
	sceUsbdMultiIsochronousRequest m_req;
} UsbdIoRequest_t;

typedef struct _device
{
	u32 m_id;
	struct _device *m_next;
	struct _device *m_prev;
	struct _endpoint *m_endpointListStart;
	struct _endpoint *m_endpointListEnd;
	sceUsbdLddOps *m_devDriver;
	u8 m_deviceStatus;
	u8 m_functionAddress;
	u8 m_isLowSpeedDevice;
	u8 m_resetFlag;
	u32 m_magicPowerValue;
	struct _device *m_childListStart;
	struct _device *m_childListEnd;
	struct _device *m_parent;
	u32 m_attachedToPortNo;
	void *m_privDataField;
	UsbdTimerCbStruct_t m_timer;
	UsbdIoRequest_t m_ioRequest;
	u32 m_functionDelay;
	void *m_staticDeviceDescPtr;
	void *m_staticDeviceDescEndPtr;
	u32 m_fetchDescriptorCounter;
	struct _usbdReportDescriptor *m_reportDescriptorCurForFetch;
	struct _usbdReportDescriptor *m_reportDescriptorStart;
	struct _usbdReportDescriptor *m_reportDescriptorEnd;
} UsbdDevice_t;

typedef struct _hcTd
{
	u32 m_hcArea;
	void *m_curBufPtr;
	struct _hcTd *m_next;
	void *m_bufferEnd;
} UsbdHcTD_t;

typedef struct _hcIsoTd
{
	u32 m_hcArea;
	void *m_bufferPage0;
	struct _hcIsoTd *m_next;
	void *m_bufferEnd;
	u16 m_psw[8];
} UsbdHcIsoTD_t;

struct _hcEdHcArea
{
	u16 m_hcArea;
	u16 m_maxPacketSize;
};

union _hcEdHcAreaU
{
	struct _hcEdHcArea stru;
	u32 asu32;
};

typedef struct _hcEd
{
	union _hcEdHcAreaU m_hcArea;
	UsbdHcTD_t *m_tdTail;
	UsbdHcTD_t *m_tdHead;
	struct _hcEd *m_next;
} UsbdHcED_t;

typedef struct _endpoint
{
	u32 m_id;
	u32 m_inTdQueue;
	struct _endpoint *m_next;
	struct _endpoint *m_prev;
	struct _endpoint *m_busyNext;
	struct _endpoint *m_busyPrev;
	UsbdDevice_t *m_correspDevice;
	UsbdIoRequest_t *m_ioReqListStart;
	UsbdIoRequest_t *m_ioReqListEnd;
	UsbdHcED_t *m_hcEd;
	u32 m_endpointType;
	u32 m_isochronLastFrameNum;   // 40
	UsbdTimerCbStruct_t m_timer;  // sizeof(UsbdTimerCbStruct_t) => 24 bytes
	u32 m_alignFlag;
	u8 m_schedulingIndex;
	u8 m_waitHigh;
	u8 m_waitLow;
	u8 m_packetSizeForScheduling;
} UsbdEndpoint_t;

typedef struct _usbHub
{
	struct _usbHub *m_next;
	u32 m_pad1[1];
	u32 m_curAllocatedCount;
	UsbdDevice_t *m_dev;
	UsbdEndpoint_t *m_controlEp;
	UsbdEndpoint_t *m_statusChangeEp;
	UsbdIoRequest_t m_controlIoReq;
	UsbdIoRequest_t m_statusIoReq;
	u32 m_maxPower;
	u32 m_isSelfPowered;
	UsbHubDescriptor m_desc;
	u32 m_pad2[6];
	u32 m_numChildDevices;
	u32 m_portCounter;
	u32 m_hubStatusCounter;
	u16 m_hubStatus;
	u16 m_hubStatusChange;  // unite to u32 to make it match portStatusChange
	u32 m_portStatusChange;
	u8 m_statusChangeInfo[8];  // depends on number of ports
	u32 m_pad3[6];
} UsbdUsbHub_t;

typedef struct _hcCA
{
	volatile UsbdHcED_t *InterruptTable[32];
	volatile u16 FrameNumber;
	volatile u16 pad;
	volatile UsbdHcTD_t *DoneHead;
	volatile u8 reserved[116];
	volatile u32 pad2;  // expand struct to 256 bytes for alignment
} HcCA;

typedef struct _ohciRegs
{
	volatile u32 HcRevision;
	volatile u32 HcControl;
	volatile u32 HcCommandStatus;
	volatile u32 HcInterruptStatus;
	volatile u32 HcInterruptEnable;
	volatile u32 HcInterruptDisable;
	volatile HcCA *HcHCCA;
	volatile UsbdHcED_t *HcPeriodCurrentEd;
	volatile UsbdHcED_t *HcControlHeadEd;
	volatile UsbdHcED_t *HcControlCurrentEd;
	volatile UsbdHcED_t *HcBulkHeadEd;
	volatile UsbdHcED_t *HcBulkCurrentEd;
	volatile u32 HcDoneHead;
	volatile u32 HcFmInterval;
	volatile u32 HcFmRemaining;
	volatile u32 HcFmNumber;
	volatile u32 HcPeriodicStart;
	volatile u32 HcLsThreshold;
	volatile u32 HcRhDescriptorA;
	volatile u32 HcRhDescriptorB;
	volatile u32 HcRhStatus;
	volatile u32 HcRhPortStatus[2];
} OhciRegs;

typedef struct _memPool
{
	volatile OhciRegs *m_ohciRegs;
	volatile HcCA *m_hcHCCA;

	struct _hcEd *m_hcEdBuf;

	struct _hcTd *m_freeHcTdList;
	struct _hcTd *m_hcTdBuf;
	struct _hcTd *m_hcTdBufEnd;

	struct _hcIsoTd *m_freeHcIsoTdList;
	struct _hcIsoTd *m_hcIsoTdBuf;
	struct _hcIsoTd *m_hcIsoTdBufEnd;

	struct _ioRequest **m_hcTdToIoReqLUT;
	struct _ioRequest **m_hcIsoTdToIoReqLUT;

	struct _ioRequest *m_ioReqBufPtr;
	struct _ioRequest *m_freeIoReqList;
	struct _ioRequest *m_freeIoReqListEnd;

	struct _device *m_deviceTreeBuf;
	struct _device *m_freeDeviceListStart;
	struct _device *m_freeDeviceListEnd;

	struct _endpoint *m_endpointBuf;
	struct _endpoint *m_freeEpListStart;
	struct _endpoint *m_freeEpListEnd;

	struct _endpoint *m_tdQueueStart;
	struct _endpoint *m_tdQueueEnd;

	u32 m_interruptBandwidthSchedulingValues[32];
	u32 m_delayResets;
	int m_interruptCounters[9];

	struct _timerCbStruct *m_timerListStart;
	struct _timerCbStruct *m_timerListEnd;

	struct _device *m_deviceTreeRoot;
} UsbdMemoryPool_t;

typedef struct _usbdReportDescriptor
{
	struct _usbdReportDescriptor *m_next;
	struct _usbdReportDescriptor *m_prev;
	u32 m_cfgNum;
	u32 m_ifNum;
	u32 m_length;
	u8 m_data[];
} UsbdReportDescriptor_t;

typedef struct _usbdKernelResources
{
	int m_usbdSema;
	int m_hcdTid;
	int m_hcdIrqEvent;
	int m_callbackTid;
	int m_callbackEvent;
} UsbdKernelResources_t;

#define NOTIN_QUEUE 0
#define GENTD_QUEUE 1
#define ISOTD_QUEUE 2

#define TYPE_CONTROL 0x3F
#define TYPE_BULK 0x40
#define TYPE_ISOCHRON 0x41

#define DEVICE_NOTCONNECTED 1
#define DEVICE_CONNECTED 3
#define DEVICE_RESETDELAYED 4
#define DEVICE_RESETPENDING 5
#define DEVICE_RESETCOMPLETE 6
#define DEVICE_FETCHINGDESCRIPTOR 7
#define DEVICE_READY 8

#define PORT_CONNECTION 0
#define PORT_ENABLE 1
#define PORT_SUSPEND 2
#define PORT_OVER_CURRENT 3
#define PORT_RESET 4
#define PORT_POWER 8
#define PORT_LOW_SPEED 9

#define C_HUB_LOCAL_POWER 0
#define C_HUB_OVER_CURRENT 1

#define C_PORT_CONNECTION 16
#define C_PORT_ENABLE 17
#define C_PORT_SUSPEND 18
#define C_PORT_OVER_CURRENT 19
#define C_PORT_RESET 20

#define BIT(x) (((u32)1) << (x))

#define C_PORT_FLAGS                                                                                                   \
	(BIT(C_PORT_CONNECTION) | BIT(C_PORT_ENABLE) | BIT(C_PORT_SUSPEND) | BIT(C_PORT_OVER_CURRENT) | BIT(C_PORT_RESET))

#define HCED_DIR_OUT BIT(11)  // Direction field
#define HCED_DIR_IN BIT(12)   // Direction field
#define HCED_SPEED BIT(13)    // Speed bit
#define HCED_SKIP BIT(14)     // Skip bit
#define HCED_ISOC BIT(15)     // Format bit
#define HCED_DIR_MASK (HCED_DIR_OUT | HCED_DIR_IN)

#define ED_HALTED(a) ((u32)((a).m_tdHead) & 1)
#define ED_SKIPPED(a) ((u32)((a).m_hcArea.stru.m_hcArea) & HCED_SKIP)

#define TD_HCAREA(CC, T, DI, DP, R) (u32)((((CC) << 12) | ((T) << 8) | ((DI) << 5) | ((DP) << 3) | ((R) << 2)))

#define TD_SETUP 0
#define TD_OUT 1
#define TD_IN 2

#define OHCI_INT_SO BIT(0)
#define OHCI_INT_WDH BIT(1)
#define OHCI_INT_SF BIT(2)
#define OHCI_INT_RD BIT(3)
#define OHCI_INT_UE BIT(4)
#define OHCI_INT_FNO BIT(5)
#define OHCI_INT_RHSC BIT(6)
#define OHCI_INT_OC BIT(30)
#define OHCI_INT_MIE BIT(31)

#define OHCI_COM_HCR BIT(0)
#define OHCI_COM_CLF BIT(1)
#define OHCI_COM_BLF BIT(2)

#define OHCI_CTR_CBSR (3 << 0)  // Control / Bulk Service Ratio
#define OHCI_CTR_PLE BIT(2)     // Periodic List Enable
#define OHCI_CTR_IE BIT(3)      // Isochronous Enable
#define OHCI_CTR_CLE BIT(4)     // Control List Enable
#define OHCI_CTR_BLE BIT(5)     // Bulk List Enable
#define OHCI_CTR_USB_RESET (0 << 6)
#define OHCI_CTR_USB_RESUME (1 << 6)
#define OHCI_CTR_USB_OPERATIONAL (2 << 6)
#define OHCI_CTR_USB_SUSPEND (3 << 6)

// The following is defined in hub.c
extern void hubResetDevicePort(UsbdDevice_t *dev);
extern int initHubDriver(void);
extern void deinitHubDriver(void);

// The following is defined in mem.c
extern UsbdDevice_t *fetchDeviceById(int devId);
extern UsbdEndpoint_t *fetchEndpointById(int id);
extern UsbdDevice_t *getDeviceTreeRoot(void);
extern UsbdDevice_t *attachChildDevice(UsbdDevice_t *parent, u32 portNum);
extern void freeDevice(UsbdDevice_t *dev);
extern UsbdIoRequest_t *allocIoRequest(void);
extern void freeIoRequest(UsbdIoRequest_t *req);
extern UsbdEndpoint_t *allocEndpointForDevice(UsbdDevice_t *dev, u32 align);
extern int cleanUpFunc(UsbdDevice_t *dev, UsbdEndpoint_t *ep);
extern UsbdHcTD_t *allocTd(void);
extern void freeTd(UsbdHcTD_t *argTd);
extern UsbdHcIsoTD_t *allocIsoTd(void);
extern void freeIsoTd(UsbdHcIsoTD_t *argTd);

// The following is defined in timer.c
extern int addTimerCallback(UsbdTimerCbStruct_t *arg, TimerCallback func, void *cbArg, int delay);
extern int cancelTimerCallback(UsbdTimerCbStruct_t *arg);
extern void handleTimerList(void);

// The following is defined in endpoint.c
extern UsbdEndpoint_t *openDeviceEndpoint(UsbdDevice_t *dev, const UsbEndpointDescriptor *endpDesc, u32 alignFlag);
extern int removeEndpointFromDevice(UsbdDevice_t *dev, UsbdEndpoint_t *ep);

// The following is defined in io_request.c
extern void handleIoReqList(UsbdEndpoint_t *ep);

// The following is defined in hub_resets.c
extern void usbdRebootInner(void);
extern void hubResetDevice(UsbdDevice_t *dev);
extern int checkDelayedResets(UsbdDevice_t *dev);
extern void handleRhsc(void);

// The following is defined in td_queue.c
extern void processDoneQueue_GenTd(UsbdHcTD_t *arg);
extern void processDoneQueue_IsoTd(UsbdHcIsoTD_t *arg);

// The following is defined in hcd.c
extern void hcdProcessIntr(void);
extern void PostIntrEnableFunction(void);
extern int initHcdStructs(void);
extern void deinitHcd(void);

// The following is defined in usbd_sys.c
extern void *AllocSysMemoryWrap(int size);
extern int FreeSysMemoryWrap(void *ptr);
extern int usbdLock(void);
extern int usbdUnlock(void);

// The following is defined in usbd_main.c
extern void usbdReboot(int ac);

// The following is defined in report_descriptor_init.c
extern int
handleStaticDeviceDescriptor(UsbdDevice_t *dev, UsbDeviceDescriptor *devDescStart, UsbDeviceDescriptor *devDescEnd);

// The following is defined in device_driver.c
extern int callUsbDriverFunc(int (*func)(int devId), int devId, void *gpSeg);
extern int doRegisterDriver(sceUsbdLddOps *drv, void *drvGpSeg);
extern int doRegisterAutoLoader(sceUsbdLddOps *drv, void *drvGpSeg);
extern int doUnregisterDriver(sceUsbdLddOps *drv);
extern int doUnregisterAutoLoader(void);

// The following is defined in device.c
extern void *doGetDeviceStaticDescriptor(int devId, void *data, u8 type);
extern int doGetDeviceLocation(UsbdDevice_t *dev, u8 *path);
extern UsbdEndpoint_t *doOpenEndpoint(UsbdDevice_t *dev, const UsbEndpointDescriptor *endpDesc, u32 alignFlag);
extern int doCloseEndpoint(UsbdEndpoint_t *ep);
extern int attachIoReqToEndpoint(UsbdEndpoint_t *ep, UsbdIoRequest_t *req, void *destdata, u16 length, void *callback);
extern int doControlTransfer(
	UsbdEndpoint_t *ep,
	UsbdIoRequest_t *req,
	u8 requestType,
	u8 request,
	u16 value,
	u16 index,
	u16 length,
	void *destdata,
	void *callback);
extern int hubTimedSetFuncAddress(UsbdDevice_t *dev);
extern void flushPort(UsbdDevice_t *dev);
extern int usbdInitInner(void);

// The following is defined in usbd_main.c
extern UsbdMemoryPool_t *memPool;
extern UsbdKernelResources_t usbKernelResources;
extern UsbdIoRequest_t *cbListStart;
extern UsbdIoRequest_t *cbListEnd;
extern UsbdConfig_t usbConfig;
extern sceUsbdLddOps *drvListStart;
extern sceUsbdLddOps *drvListEnd;
extern sceUsbdLddOps *drvAutoLoader;

#endif  // __USBDPRIV_H__
