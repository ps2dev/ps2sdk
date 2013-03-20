/*	iLinkman.h
 *	Purpose:	Main header file containing function prototypes, structure declaration and other declarations.
 *
 *	Last Updated:	2012/02/28
 *	Programmer:	SP193
 */

#define BSWAP32(x) ((x<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|(x>>24))
#define BSWAP16(x) ((((unsigned short int)x)<<8)|(x>>8))

/* IEEE1394 speeds */
#define S100	0
#define S200	1
#define S400	2

#define iLink_NODE_IRMC		0x10
#define iLink_NODE_CMC		0x08
#define iLink_NODE_ISC		0x04
#define iLink_NODE_BMC		0x02
#define iLink_NODE_PMC		0x01

#define iLink_CB_WRITE_REQUEST	0x01
#define iLink_CB_READ_REQUEST	0x02
#define iLink_CB_LOCK_REQUEST	0x04
#define iLink_CB_BUS_RESET	0x08

/* Module function import stuff. */
#define iLinkman_IMPORTS_start DECLARE_IMPORT_TABLE(iLinkman, 1, 1)
#define iLinkman_IMPORTS_end END_IMPORT_TABLE

void iLinkEnableSBus(void);
#define I_iLinkEnableSBus DECLARE_IMPORT(4, iLinkEnableSBus)
void iLinkDisableSBus(void);
#define I_iLinkDisableSBus DECLARE_IMPORT(5, iLinkDisableSBus)
void iLinkResetSBus(void);
#define I_iLinkResetSBus DECLARE_IMPORT(6, iLinkResetSBus)
unsigned int iLinkSetNodeCapabilities(unsigned int capabilities);
#define I_iLinkSetNodeCapabilities DECLARE_IMPORT(7, iLinkSetNodeCapabilities)
void *iLinkSetTrCallbackHandler(void *function);
#define I_iLinkSetTrCallbackHandler DECLARE_IMPORT(8, iLinkSetTrCallbackHandler)
int iLinkGetGenerationNumber(void);
#define I_iLinkGetGenerationNumber DECLARE_IMPORT(9, iLinkGetGenerationNumber)

int iLinkGetLocalNodeID(void);
#define I_iLinkGetLocalNodeID DECLARE_IMPORT(10, iLinkGetLocalNodeID)
int iLinkGetNodeCount(void);
#define I_iLinkGetNodeCount DECLARE_IMPORT(11, iLinkGetNodeCount)

int iLinkTrAlloc(unsigned short int NodeID, unsigned char speed);
#define I_iLinkTrAlloc DECLARE_IMPORT(12, iLinkTrAlloc)
void iLinkTrFree(int trContext);
#define I_iLinkTrFree DECLARE_IMPORT(13, iLinkTrFree)
int iLinkTrWrite(int trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes);
#define I_iLinkTrWrite DECLARE_IMPORT(14, iLinkTrWrite)
int iLinkTrRead(int trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes);
#define I_iLinkTrRead DECLARE_IMPORT(15, iLinkTrRead)

unsigned short int iLinkCalculateCRC16(void *data, unsigned int nQuads);
#define I_iLinkCalculateCRC16 DECLARE_IMPORT(16, iLinkCalculateCRC16)
int iLinkAddCROMUnit(unsigned int *data, unsigned int nQuads);
#define I_iLinkAddCROMUnit DECLARE_IMPORT(17, iLinkAddCROMUnit)
void iLinkDeleteCROMUnit(unsigned int id);
#define I_iLinkDeleteCROMUnit DECLARE_IMPORT(18, iLinkDeleteCROMUnit)
int iLinkGetNodeCapabilities(unsigned short NodeID);
#define I_iLinkGetNodeCapabilities DECLARE_IMPORT(19, iLinkGetNodeCapabilities)
int iLinkGetNodeMaxSpeed(unsigned short int NodeID);
#define I_iLinkGetNodeMaxSpeed DECLARE_IMPORT(20, iLinkGetNodeMaxSpeed)

int iLinkGetNodeTrSpeed(int trContext);
#define I_iLinkGetNodeTrSpeed DECLARE_IMPORT(21, iLinkGetNodeTrSpeed)
int iLinkSetNodeTrSpeed(int trContext, unsigned char speed);
#define I_iLinkSetNodeTrSpeed DECLARE_IMPORT(22, iLinkSetNodeTrSpeed)

int iLinkFindUnit(int UnitInList, unsigned int UnitSpec, unsigned int UnitSW_Version);
#define I_iLinkFindUnit DECLARE_IMPORT(23, iLinkFindUnit)
int iLinkReadCROM(unsigned short int NodeID, unsigned int Offset, unsigned int nQuads, unsigned int *buffer);
#define I_iLinkReadCROM DECLARE_IMPORT(24, iLinkReadCROM)

u64 iLinkGetLocalNodeEUI64(void);
#define I_iLinkGetLocalNodeEUI64 DECLARE_IMPORT(12, iLinkGetLocalNodeEUI64)
