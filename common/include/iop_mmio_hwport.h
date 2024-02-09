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
 * Definitions for memory-mapped I/O for IOP.
 */

#ifndef __IOP_MMIO_HWPORT__
#define __IOP_MMIO_HWPORT__

typedef struct dmac_channel_
{
	vu32 madr;
	vu32 bcr;
	vu32 chcr;
	vu32 tadr;
} dmac_channel_t;

typedef struct dmac1_mmio_hwport_
{
	dmac_channel_t oldch[7];
	vu32 dpcr1;
	vu32 dicr1;
} dmac1_mmio_hwport_t;

typedef struct dmac2_mmio_hwport_
{
	dmac_channel_t newch[6];
	dmac_channel_t new_unusedch;
	vu32 dpcr2;
	vu32 dicr2;
	vu32 dmacen;
	vu32 dmacinten;
} dmac2_mmio_hwport_t;

typedef struct dmac_channel3_
{
	vu32 madr;
	vu32 dlen;
	vu32 slice;
	vu32 chcr;
	vu32 rtar;
	vu32 DmarReadStart;
	vu32 DmarReadEnd;
} dmac_channel3_t;

typedef struct dmac3_mmio_hwport_
{
	dmac_channel3_t dmac_channel3_0;
	u8 unused_1c[0x4];
	dmac_channel3_t dmac_channel3_1;
	u8 unused_3c[0x4];
	dmac_channel3_t dmac_channel3_2;
	vu32 DmarWriteStart;
	vu32 DmarWriteEnd;
	u8 unused_64[0x1C];
} dmac3_mmio_hwport_t;

typedef struct ssbus1_mmio_hwport_
{
	vu32 ind_0_address;
	vu32 ind_8_address;
	vu32 ind_0_delay;
	vu32 ind_1_delay;
	vu32 ind_2_delay;
	vu32 ind_4_delay;
	vu32 ind_5_delay;
	vu32 ind_8_delay;
	vu32 common_delay;
} ssbus1_mmio_hwport_t;

typedef struct ssbus2_mmio_hwport_
{
	vu32 ind_1_address;
	vu32 ind_4_address;
	vu32 ind_5_address;
	vu32 ind_9_address;
	vu32 ind_B_address;
	vu32 ind_9_delay;
	vu32 ind_A_delay;
	vu32 ind_B_delay;
	vu32 ind_C_delay;
} ssbus2_mmio_hwport_t;

typedef struct sio0_1_mmio_hwport_
{
	vu32 data;
	vu32 stat;
	vu16 mode;
	vu16 ctrl;
	vu16 misc;
	vu16 baud;
} sio0_1_mmio_hwport_t;

typedef struct sio2_mmio_hwport_
{
	vu8 send3_buf[0x40];
	vu8 send1_2_buf[0x20];
	vu32 out_fifo; /* PCSX2 says in */
	vu32 in_fifo;  /* PCSX2 says out */
	vu32 ctrl;
	vu32 recv1;
	vu32 recv2;
	vu32 recv3;
	vu32 unk_78;
	vu32 unk_7c;
	vu32 stat;
	u8 unused[0x7c];
} sio2_mmio_hwport_t;

typedef struct usb_mmio_hwport_
{
	vu32 HcRevision;
	vu32 HcControl;
	vu32 HcCommandStatus;
	vu32 HcInterruptStatus;
	vu32 HcInterruptEnable;
	vu32 HcInterruptDisable;
	vu32 HcHCCA;
	vu32 HcPeriodCurrentEd;
	vu32 HcControlHeadEd;
	vu32 HcControlCurrentEd;
	vu32 HcBulkHeadEd;
	vu32 HcBulkCurrentEd;
	vu32 HcDoneHead;
	vu32 HcFmInterval;
	vu32 HcFmRemaining;
	vu32 HcFmNumber;
	vu32 HcPeriodicStart;
	vu32 HcLsThreshold;
	vu32 HcRhDescriptorA;
	vu32 HcRhDescriptorB;
	vu32 HcRhStatus;
	vu32 HcRhPortStatus[2]; /* PCSX2 says 15 or 11 */
	u8 unused[0xa4];
} usb_mmio_hwport_t;

typedef struct ieee1394_mmio_hwport_
{
	vu32 NodeID;
	vu32 CycleTime;

	vu32 ctrl0;
	vu32 ctrl1;
	vu32 ctrl2;

	vu32 PHYAccess;

	vu32 UnknownRegister18;
	vu32 UnknownRegister1C;

	vu32 intr0;
	vu32 intr0Mask;

	vu32 intr1;
	vu32 intr1Mask;

	vu32 intr2;
	vu32 intr2Mask;

	vu32 dmar;
	vu32 ack_status;
	vu32 ubufTransmitNext;
	vu32 ubufTransmitLast;
	vu32 ubufTransmitClear;
	vu32 ubufReceiveClear;
	vu32 ubufReceive;
	vu32 ubufReceiveLevel;

	vu32 unmapped1[0x06];

	vu32 UnknownRegister70;
	vu32 UnknownRegister74;
	vu32 UnknownRegister78;
	vu32 UnknownRegister7C;

	vu32 PHT_ctrl_ST_R0;
	vu32 PHT_split_TO_R0;
	vu32 PHT_ReqResHdr0_R0;
	vu32 PHT_ReqResHdr1_R0;
	vu32 PHT_ReqResHdr2_R0;

	vu32 STRxNIDSel0_R0;
	vu32 STRxNIDSel1_R0;

	vu32 STRxHDR_R0;
	vu32 STTxHDR_R0;

	vu32 DTransCTRL0;
	vu32 CIPHdrTx0_R0;
	vu32 CIPHdrTx1_R0;

	vu32 padding4;
	vu32 STTxTimeStampOffs_R0;

	vu32 dmaCtrlSR0;
	vu32 dmaTransTRSH0;
	vu32 dbufFIFO_lvlR0;
	vu32 dbufTxDataR0;
	vu32 dbufRxDataR0;

	vu32 dbufWatermarksR0;
	vu32 dbufFIFOSzR0;

	vu32 unmapped2[0x0B];

	vu32 PHT_ctrl_ST_R1;
	vu32 PHT_split_TO_R1;
	vu32 PHT_ReqResHdr0_R1;
	vu32 PHT_ReqResHdr1_R1;
	vu32 PHT_ReqResHdr2_R1;

	vu32 STRxNIDSel0_R1;
	vu32 STRxNIDSel1_R1;

	vu32 STRxHDR_R1;
	vu32 STTxHDR_R1;

	vu32 DTransCTRL1;
	vu32 CIPHdrTx0_R1;
	vu32 CIPHdrTx1_R1;

	vu32 padding5;
	vu32 STTxTimeStampOffs_R1;

	vu32 dmaCtrlSR1;
	vu32 dmaTransTRSH1;
	vu32 dbufFIFO_lvlR1;
	vu32 dbufTxDataR1;
	vu32 dbufRxDataR1;

	vu32 dbufWatermarksR1;
	vu32 dbufFIFOSzR1;
} ieee1394_mmio_hwport_t;

typedef struct iop_counter_mmio_hwport_
{
	vu32 count;
	vu32 mode;
	vu32 target;
	vu32 unused_c;
} iop_counter_mmio_hwport_t;

typedef struct iop_mmio_hwport_ /* base -> 0xBF800000 */
{
	u8 scratchpad_cache0[0x400];
	u8 scratchpad_cache1[0x400];
	u8 unv_0800[0x800];
	ssbus1_mmio_hwport_t ssbus1; /* 0x1000 */
	u8 unv_1024[0x1c];
	sio0_1_mmio_hwport_t sio0;
	sio0_1_mmio_hwport_t sio1;
	vu32 iop_ram_size;
	u8 unv_1064[0xC];
	vu32 istat;
	vu32 imask;
	vu32 iop_sbus_info;
	vu32 unk_107c;
	dmac1_mmio_hwport_t dmac1; /* 0x1080 */
	u8 unv_10f8[0x8];
	iop_counter_mmio_hwport_t counter1[3]; /* 0x1100 */
	u8 unv_1130[0x2d0];
	ssbus2_mmio_hwport_t ssbus2; /* 0x1400 */
	u8 unv_1424[0x2c];
	vu32 iop_sbus_ctrl[2]; /* 0x1450 */
	u8 unk_1458[0x8];
	u8 dev9c[0x20];
	iop_counter_mmio_hwport_t counter2[3]; /* 0x1480 */
	u8 unk_14b0[0x10];
	vu32 rtc_holdmode;
	u8 unk_14c4[0x3c];
	dmac2_mmio_hwport_t dmac2; /* 0x1500 */
	dmac3_mmio_hwport_t dmac3; /* 0x1580 */
	usb_mmio_hwport_t usb;     /* 0x1600 */
	u8 unk_1700[0x100];
	vu32 ps1_cdrom;
	u8 unk_1804[0xc];
	vu32 ps1_gpu1;
	vu32 ps1_gpu2;
	u8 unk_1818[0x8];
	vu32 ps1_mdec1;
	vu32 ps1_mdec2;
	u8 unk_1828[0x8];
	u8 unk_1830[0xd0];
	u8 deckard_i2c[0x20]; /* 0x1900 */
	u8 unv_1920[0x2e0];
	u8 spu_core0[0x400]; /* 0x1C00 */
	u8 exp2_r2[0x2000];  /* 0x2000 */
	u8 unk_4000[0x4000];
	u8 sio2_internal[0x200]; /* 0x8000 */
	sio2_mmio_hwport_t sio2; /* 0x8200 */
	u8 unk_8300[0x100];
	ieee1394_mmio_hwport_t ieee1394; /* 0x8400 */
} iop_mmio_hwport_t;

#if !defined(USE_IOP_MMIO_HWPORT) && defined(_IOP)
// cppcheck-suppress-macro constVariablePointer
#define USE_IOP_MMIO_HWPORT() iop_mmio_hwport_t *const iop_mmio_hwport = (iop_mmio_hwport_t *)0xBF800000
#endif
#if !defined(USE_IOP_MMIO_HWPORT)
#define USE_IOP_MMIO_HWPORT()
#endif

#endif /* __IOP_MMIO_HWPORT__ */
