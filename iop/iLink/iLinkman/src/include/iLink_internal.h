// #define DEBUG_TTY_FEEDBACK /* Comment out to disable generation of debug TTY messages */

#ifdef DEBUG_TTY_FEEDBACK
#define DEBUG_PRINTF(args...)  printf("ILINKMAN: "args)
#define iDEBUG_PRINTF(args...) Kprintf("ILINKMAN: "args)
#else
#define DEBUG_PRINTF(args...) \
    do {                      \
    } while (0)
#define iDEBUG_PRINTF(args...) \
    do {                       \
    } while (0)
#endif

#define REQ_CHECK_MEM_BOUNDARIES  /* Define this to enable memory address boundary checking for incoming requests. */
#define REQ_CHECK_CONSOLE_VERSION /* Define this to enable version checks on the Playstation 2 console. */
// #define REQ_CHECK_DMAC_STAT       /* Define this to enable memory DMAC status checking. */
// #define REQ_CHECK_ERRORS          /* Define this to enable checking for HdrErr and SntBsyAck interrupts. Serves no purpose except for debugging. */

#define iLink_INTR0_DRFR      0x00000001
#define iLink_INTR0_DRFO      0x00000002
#define iLink_INTR0_TxStk     0x00000040
#define iLink_INTR0_FmtE      0x00000080
#define iLink_INTR0_UResp     0x00000100
#define iLink_INTR0_PBCntR    0x00000200
#define iLink_INTR0_STO       0x00000400
#define iLink_INTR0_RetEx     0x00000800
#define iLink_INTR0_InvAck    0x00001000
#define iLink_INTR0_AckMiss   0x00002000
#define iLink_INTR0_AckRcvd   0x00004000
#define iLink_INTR0_CycArbFl  0x00008000
#define iLink_INTR0_CycLost   0x00010000
#define iLink_INTR0_CycSt     0x00080000
#define iLink_INTR0_CycTL     0x00200000
#define iLink_INTR0_URx       0x00400000
#define iLink_INTR0_SubActGap 0x00800000
#define iLink_INTR0_TCErr     0x01000000
#define iLink_INTR0_HdrErr    0x02000000
#define iLink_INTR0_SntBsyAck 0x04000000
#define iLink_INTR0_CmdRst    0x08000000
#define iLink_INTR0_ArbRstGap 0x10000000
#define iLink_INTR0_PhyRst    0x20000000
#define iLink_INTR0_PhyRRx    0x40000000
#define iLink_INTR0_PhyInt    0x80000000

#define iLink_INTR1_DTFO 0x00000001
#define iLink_INTR1_UTD  0x00000002

#define iLink_CTRL0_URcvM         0x00000010
#define iLink_CTRL0_RSP0          0x00000020
#define iLink_CTRL0_RetLim(x)     (x << 12)
#define iLink_CTRL0_LooseTightIso 0x00010000
#define iLink_CTRL0_STardy        0x00020000
#define iLink_CTRL0_BRDE          0x00040000
#define iLink_CTRL0_Root          0x00080000
#define iLink_CTRL0_ExtCyc        0x00100000
#define iLink_CTRL0_CycTmrEn      0x00200000
#define iLink_CTRL0_CMstr         0x00400000
#define iLink_CTRL0_BusIDRst      0x00800000
#define iLink_CTRL0_RxRst         0x01000000
#define iLink_CTRL0_TxRst         0x02000000
#define iLink_CTRL0_RxEn          0x04000000
#define iLink_CTRL0_TxEn          0x08000000
#define iLink_CTRL0_DELim(x)      (x << 28)
#define iLink_CTRL0_SIDF          0x40000000
#define iLink_CTRL0_RcvSelfID     0x80000000

#define iLink_CTRL2_LPSRst 0x00000001
#define iLink_CTRL2_LPSEn  0x00000002
#define iLink_CTRL2_SRst   0x00000004
#define iLink_CTRL2_SOK    0x00000008

#define iLinkDMA_CTRL_SR_DWidth(x) (x << 17)
#define iLinkDMA_CTRL_SR_DEn       0x080000
#define iLinkDMA_CTRL_SR_RActl     0x010000
#define iLinkDMA_CTRL_SR_LFirst    0x100000

#define PHT_CTRL_ST_PRBR   0x00000200
#define PHT_CTRL_ST_PStk   0x00000400
#define PHT_CTRL_ST_EnDMAS 0x00001000
#define PHT_CTRL_ST_EBCNT  0x00008000
#define PHT_CTRL_ST_EWREQ  0x00010000
#define PHT_CTRL_ST_ERREQ  0x00020000
#define PHT_CTRL_ST_EPCNT  0x00100000
#define PHT_CTRL_ST_PHTRst 0x00200000
#define PHT_CTRL_ST_IHdr   0x00400000
#define PHT_CTRL_ST_EHdr   0x02000000

#define DBUF_FIFO_RESET_TX 0x00008000
#define DBUF_FIFO_RESET_RX 0x80000000

#define PACKET_TCODE (((x) >> 4) & 0xF)

/* PHY configuration packet data */
#define PHY_CONFIG_PACKET     0xE0
#define PHY_CONFIG_GAP_CNT(x) (((x) >> 16) & 0x3F)
#define PHY_CONFIG_T(x)       (((x) >> 22) & 0x01)
#define PHY_CONFIG_R(x)       (((x) >> 23) & 0x01)
#define PHY_CONFIG_ROOT_ID(x) (((x) >> 24) & 0x3F)

/* SELF-ID packet data */
#define PHY_SELF_ID_PACKET   0xE1
#define PHY_SELF_ID_PKT_SIG  0x2 /* 10b */
#define SELF_ID_SIGNATURE(x) (((x) >> 30) & 0x03)
#define SELF_ID_NODEID(x)    (((x) >> 24) & 0x3F)
#define SELF_ID_L(x)         (((x) >> 22) & 0x01)
#define SELF_ID_GAP_CNT(x)   (((x) >> 16) & 0x3F)
#define SELF_ID_SPEED(x)     (((x) >> 14) & 0x03)
#define SELF_ID_POWER(x)     (((x) >> 8) & 0x07)
#define SELF_ID_M(x)         ((x)&0x01)

/* PHY Register bits. */
#define REG01_IBR      0x40
#define REG01_RHB      0x80
#define REG04_LCTRL    0x80
#define REG05_ISBR     0x40
#define REG05_EN_ACCL  0x02
#define REG05_EN_MULTI 0x01

/* Event flag bits. */
#define iLinkEventInterrupt    0x00000001
#define iLinkEventBusReady     0x00000002
#define iLinkEventGotSELFIDs   0x00000004
#define iLinkEventDataSent     0x00000008
#define iLinkEventDataReceived 0x00000010
#define iLinkEventDMATransEnd  0x00000020
#define iLinkEventBusReset     0x00000040
#define iLinkEventURx          0x00000080
#define iLinkEventError        0x80000000

/* Structures. */
#define IEEE1394_TCODE_WRITEQ         0
#define IEEE1394_TCODE_WRITEB         1
#define IEEE1394_TCODE_WRITE_RESPONSE 2
#define IEEE1394_TCODE_READQ          4
#define IEEE1394_TCODE_READB          5
#define IEEE1394_TCODE_READQ_RESPONSE 6
#define IEEE1394_TCODE_READB_RESPONSE 7

struct ieee1394_TrPacketHdr
{
    unsigned int header;
    unsigned int offset_high;
    unsigned int offset_low;
    unsigned int misc;
};

struct ieee1394_TrResponsePacketHdr
{
    unsigned int header;
    unsigned int header2;
    unsigned int reserved;
    unsigned int LastField;
};

#define ILINK_REGISTER_BASE 0xBF808400

struct ILINKMemMap
{
    volatile unsigned int NodeID;    /* 0x00 */
    volatile unsigned int CycleTime; /* 0x04 */

    volatile unsigned int ctrl0; /* 0x08 */
    volatile unsigned int ctrl1; /* 0x0C */
    volatile unsigned int ctrl2; /* 0x10 */

    volatile unsigned int PHYAccess; /* 0x14 */

    volatile unsigned int UnknownRegister18; /* 0x18 */
    volatile unsigned int UnknownRegister1C; /* 0x1C */

    volatile unsigned int intr0;     /* 0x20 */
    volatile unsigned int intr0Mask; /* 0x24 */

    volatile unsigned int intr1;     /* 0x28 */
    volatile unsigned int intr1Mask; /* 0x2C */

    volatile unsigned int intr2;     /* 0x30 */
    volatile unsigned int intr2Mask; /* 0x34 */

    volatile unsigned int dmar;              /* 0x38 */
    volatile unsigned int ack_status;        /* 0x3C */
    volatile unsigned int ubufTransmitNext;  /* 0x40 */
    volatile unsigned int ubufTransmitLast;  /* 0x44 */
    volatile unsigned int ubufTransmitClear; /* 0x48 */
    volatile unsigned int ubufReceiveClear;  /* 0x4C */
    volatile unsigned int ubufReceive;       /* 0x50 */
    volatile unsigned int ubufReceiveLevel;  /* 0x54 */

    volatile unsigned int unmapped1[0x06]; /* Registers 0x58-0x6C are unmapped. */

    volatile unsigned int UnknownRegister70; /* 0x70 */
    volatile unsigned int UnknownRegister74; /* 0x74 */
    volatile unsigned int UnknownRegister78; /* 0x78 */
    volatile unsigned int UnknownRegister7C; /* 0x7C */

    volatile unsigned int PHT_ctrl_ST_R0;    /* 0x80 */
    volatile unsigned int PHT_split_TO_R0;   /* 0x84 */
    volatile unsigned int PHT_ReqResHdr0_R0; /* 0x88 */
    volatile unsigned int PHT_ReqResHdr1_R0; /* 0x8C */
    volatile unsigned int PHT_ReqResHdr2_R0; /* 0x90 */

    volatile unsigned int STRxNIDSel0_R0; /* 0x94 */
    volatile unsigned int STRxNIDSel1_R0; /* 0x98 */

    volatile unsigned int STRxHDR_R0; /* 0x9C */
    volatile unsigned int STTxHDR_R0; /* 0xA0 */

    volatile unsigned int DTransCTRL0;  /* 0xA4 */
    volatile unsigned int CIPHdrTx0_R0; /* 0xA8 */
    volatile unsigned int CIPHdrTx1_R0; /* 0xAC */

    volatile unsigned int padding4;             /* 0xB0 */
    volatile unsigned int STTxTimeStampOffs_R0; /* 0xB4 */

    volatile unsigned int dmaCtrlSR0;     /* 0xB8 */
    volatile unsigned int dmaTransTRSH0;  /* 0xBC */
    volatile unsigned int dbufFIFO_lvlR0; /* 0xC0 */
    volatile unsigned int dbufTxDataR0;   /* 0xC4 */
    volatile unsigned int dbufRxDataR0;   /* 0xC8 */

    volatile unsigned int dbufWatermarksR0; /* 0xCC (Unmapped) */
    volatile unsigned int dbufFIFOSzR0;     /* 0xD0 (Unmapped) */

    volatile unsigned int unmapped2[0x0B]; /* Unmapped. */

    volatile unsigned int PHT_ctrl_ST_R1;    /* 0x100 */
    volatile unsigned int PHT_split_TO_R1;   /* 0x104 */
    volatile unsigned int PHT_ReqResHdr0_R1; /* 0x108 */
    volatile unsigned int PHT_ReqResHdr1_R1; /* 0x10C */
    volatile unsigned int PHT_ReqResHdr2_R1; /* 0x110 */

    volatile unsigned int STRxNIDSel0_R1; /* 0x114 */
    volatile unsigned int STRxNIDSel1_R1; /* 0x118 */

    volatile unsigned int STRxHDR_R1; /* 0x11C */
    volatile unsigned int STTxHDR_R1; /* 0x120 */

    volatile unsigned int DTransCTRL1;  /* 0x124 */
    volatile unsigned int CIPHdrTx0_R1; /* 0x128 */
    volatile unsigned int CIPHdrTx1_R1; /* 0x12C */

    volatile unsigned int padding5;             /* 0x130 */
    volatile unsigned int STTxTimeStampOffs_R1; /* 0x134 */

    volatile unsigned int dmaCtrlSR1;     /* 0x138 */
    volatile unsigned int dmaTransTRSH1;  /* 0x13C */
    volatile unsigned int dbufFIFO_lvlR1; /* 0x140 */
    volatile unsigned int dbufTxDataR1;   /* 0x144 */
    volatile unsigned int dbufRxDataR1;   /* 0x148 */

    volatile unsigned int dbufWatermarksR1; /* 0x14C (Unmapped) */
    volatile unsigned int dbufFIFOSzR1;     /* 0x150 (Unmapped) */
};

#define ILINK_DMAC_REGISTER_BASE 0xBF801580

struct DMAChannelRegBlock
{ /* The PS2 has 3 of these register blocks for DMAC #3 */
    volatile unsigned int madr;
    volatile unsigned int dlen;
    volatile unsigned int slice;
    volatile unsigned int chcr;
    volatile unsigned int rtar;
    volatile unsigned int DmarReadStart;
    volatile unsigned int DmarReadEnd;

    volatile unsigned int unused; /* This region contains either unmapped memory locations or registers that aren't for specific DMA channel control. */
};

#define ILINK_DMAC_DMAR_CTRL_BASE 0xBF8015DC

struct DMARRegBlock
{
    volatile unsigned int DmarWriteStart;
    volatile unsigned int DmarWriteEnd;
};

#define MAX_CONCURRENT_TRANSACTIONS 10

#define DMAC_CHCR_AR (1 << 31) /* Automatic Response. */
#define DMAC_CHCR_11 (1 << 11) /* I don't know what this does, but it appears to be a bit that means something to the DMAC. */

struct TransactionContextData
{
    int GenerationNumber;
    unsigned short int NodeID;
    unsigned char IsConnected;
    unsigned char speed;
};

/* Function prototypes. */
void UBUFThread(void *arg);
int GetConsoleIDs(u64 *guid, char *ModelName);
void iLinkDisableIntr(void);
int iLinkResetHW(void);
void iLinkShutdownHW(void);
void iLinkHWInitialize(void);
void iLinkEnableCMaster(void);
void iLinkBusEnable(void);
void *malloc(unsigned int nBytes);
void free(void *buffer);

void InitializeConfigurationROM(void);

unsigned char iLinkReadPhy(unsigned char address);
void iLinkWritePhy(unsigned char address, unsigned char data);
void iLinkPHY_SetRootBit(int isRoot);
void iLinkPHY_SetGapCount(unsigned char GapCount);
void iLinkPHY_SetLCTRL(int LCTRL_status);
void iLinkPHYBusReset(void);

int iLinkIntrHandler(void *arg);
void iLinkIntrRegister0Handler(void *arg);

void SendResponse(unsigned short int NodeID, unsigned short RcvdBusID, unsigned char rcode, unsigned char tLabel, unsigned char tCode, unsigned char speed, unsigned int *buffer, unsigned int nQuads);
int iLinkReadReq(struct TransactionContextData *trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes);
int iLinkWriteReq(struct TransactionContextData *trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes);

void iLinkInitPHT(void);
void PHTSendResponse(unsigned short int NodeID, unsigned short RcvdBusID, unsigned char rcode, unsigned char tLabel, unsigned char tCode, unsigned char speed, unsigned int *buffer, unsigned int nQuads);
int iLinkReadPHTReq(struct TransactionContextData *trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes);
int iLinkWritePHTReq(struct TransactionContextData *trContext, unsigned short int offset_high, unsigned int offset_low, void *buffer, unsigned int nBytes);
