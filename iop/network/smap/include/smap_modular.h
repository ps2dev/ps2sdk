
#ifndef _SMAP_MODULAR_H
#define _SMAP_MODULAR_H

#include <tamtypes.h>

// This struct needs to be the exact same layout as struct RuntimeStats!
typedef struct SmapModularRuntimeStats
{
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
} SmapModularRuntimeStats_t;

typedef struct SmapModularHookTable
{
	int Version;
	int (*TxPacketNext)(void **data);
	int (*TxPacketDeQ)(void **data);
	int (*LinkStateDown)(void);
	int (*LinkStateUp)(void);
	void *(*StackAllocRxPacket)(u16 LengthRounded, void **payload);
	int (*EnQRxPacket)(void *pbuf);
} SmapModularHookTable_t;

typedef struct SmapModularExportTable
{
	int Version;
	int (*GetMACAddress)(u8 *buffer);
	void (*Xmit)(void);
	void (*OutputDebugInformation)(void);
	int (*RegisterHook)(const SmapModularHookTable_t *hooktbl, int priority);
	unsigned char *RxBDIndexPtr;
	SmapModularRuntimeStats_t *RuntimeStatsPtr;
} SmapModularExportTable_t;

extern const SmapModularExportTable_t *SmapModularGetExportTable(void);

#define smapmodu_IMPORTS_start DECLARE_IMPORT_TABLE(smapmodu, 1, 1)
#define smapmodu_IMPORTS_end END_IMPORT_TABLE

#define I_SmapModularGetExportTable DECLARE_IMPORT(4, SmapModularGetExportTable)

#endif
