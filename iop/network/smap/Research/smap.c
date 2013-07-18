#include <stdio.h>
#include <loadcore.h>
#include <modload.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>
#include <irx.h>

#include <smapregs.h>
#include <speedregs.h>

#define DEV9_SMAP_ALL_INTR_MASK	(SMAP_INTR_EMAC3|SMAP_INTR_RXEND|SMAP_INTR_TXEND|SMAP_INTR_RXDNV|SMAP_INTR_TXDNV)	/* 0x7C */
#define DEV9_SMAP_INTR_MASK	(SMAP_INTR_EMAC3|SMAP_INTR_RXEND|SMAP_INTR_RXDNV|SMAP_INTR_TXDNV)			/* 0x6C */
#define DEV9_SMAP_INTR_MASK2	(SMAP_INTR_EMAC3|SMAP_INTR_RXEND|SMAP_INTR_RXDNV)					/* 0x68 */

/* Event flag bits */
#define NETDEV_EVENT_START	1
#define NETDEV_EVENT_STOP	2
#define NETDEV_EVENT_INTR	4
#define NETDEV_EVENT_XMIT	8
#define NETDEV_EVENT_LINK_CHECK	0x10

IRX_ID("INET_SMAP_driver", 0x02, 0x19);

extern struct irx_export_table _exp_smap;

/* This seems to be a really huge structure. */
struct SmapDriverData{
	volatile u8 *smap_regbase;	/* 0x00 */
	volatile u8 *emac3_regbase;	/* 0x04 */
	unsigned int TxBufferSpaceAvailable;	/* 0x08 */
	unsigned int NumPacketsInTx;	/* 0x0C */
	unsigned int TxBDIndex;		/* 0x10 */
	unsigned int TxDNVBDIndex;	/* 0x14 */
	unsigned int RxBDIndex;		/* 0x18 */
	sceInetPkt_t *packet;		/* 0x1C */
	int Dev9IntrEventFlag;		/* 0x20 */
	int IntrHandlerThreadID;
	unsigned int SmapDriverStarted;
	unsigned int SmapIsInitialized;
	unsigned int NetDevStopFlag;	/* 0x30 */
	unsigned int EnableLinkCheckTimer;	/* 0x34 */
	unsigned int LinkStatus;		/* 0x38 */
	unsigned int LinkMode;		/* 0x3C */
	iop_sys_clock_t LinkCheckTimer;		/* 0x40 */
	unsigned int unknown7[62];
	sceINetDevOps_t NetDevOps;	/* 0x13C */
};

static struct SmapDriverData SmapDriverData;	/* 0x000032a0 */

static const char VersionString[]="Version 2.25.0";
static unsigned int ThreadPriority=0x28;		/* 0x000031F8 */
static unsigned int ThreadStackSize=0x2000;	/* 0x000031FC */
static unsigned int VerbosityLevel=0;		/* 0x00003200 */
static unsigned int EnableAutoNegotiation=1;	/* 0x00003204 */
static unsigned int EnablePinStrapConfig=0;	/* 0x00003208 */
static unsigned int SmapConfiguration=0x5E0;	/* 0x0000320C */

static unsigned int var_00003280=0;		/* 0x00003280 */
static unsigned int var_00003284=0;		/* 0x00003284 */
static unsigned int var_00003288=0;		/* 0x00003288 */

static unsigned int IntrCounter;		/* 0x00003290 */
static char NetDeviceName[]="Ethernet (Network Adaptor)";	/* 0x00002ea4 */
static char NetDeviceModuleName[]="smap";	/* 0x00003238 */
static char NetDeviceVendorName[]="SCE";	/* 0x00003240 */

extern void *_gp;

/* 0x00001f60 */
static int DisplayBanner(void){
	printf("SMAP (%s)\n", VersionString);
	return 1;
}

/* 0x00000000 */
static void _smap_write_phy(volatile u8 *emac3_regbase, unsigned char address, unsigned short int value){
	unsigned int i, PHYRegisterValue;

	PHYRegisterValue=(address&SMAP_E3_PHY_REG_ADDR_MSK)|SMAP_E3_PHY_WRITE|((SMAP_DsPHYTER_ADDRESS&SMAP_E3_PHY_ADDR_MSK)<<SMAP_E3_PHY_ADDR_BITSFT);
	PHYRegisterValue|=((unsigned int)value)<<SMAP_E3_PHY_DATA_BITSFT;

	i=0;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_STA_CTRL, PHYRegisterValue);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL);

	for(; !(SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL)&SMAP_E3_PHY_OP_COMP); i++){
		DelayThread(0x3E8);
		if(i>=0x64) break;
	}

	if(i>=0x64) sceInetPrintf("smap: %s: > %d ms\n", "_smap_write_phy", i);
}

/* 0x000000A0 */
static int _smap_read_phy(volatile u8 *emac3_regbase, unsigned int address){
	unsigned int i, PHYRegisterValue;
	int result;

	PHYRegisterValue=(address&SMAP_E3_PHY_REG_ADDR_MSK)|SMAP_E3_PHY_READ|((SMAP_DsPHYTER_ADDRESS&SMAP_E3_PHY_ADDR_MSK)<<SMAP_E3_PHY_ADDR_BITSFT);

	i=0;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_STA_CTRL, PHYRegisterValue);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL);

	do{
		if(SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL)&SMAP_E3_PHY_OP_COMP){
			if(SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL)&SMAP_E3_PHY_OP_COMP){
				if((result=SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL))&SMAP_E3_PHY_OP_COMP){
					result>>=SMAP_E3_PHY_DATA_BITSFT;
					break;
				}
			}
		}

		DelayThread(0x3E8);
		i++;
	}while(i<0x64);

	if(i>=0x64){
		sceInetPrintf("smap: %s: > %d ms\n", "_smap_read_phy", i);
		result>>=SMAP_E3_PHY_DATA_BITSFT;
	}

	return result;
}

/* 0x00001f8c */
static int DisplayHelpMessage(void){
	DisplayBanner();

	printf("Usage: smap [<option>] [thpri=<prio>] [thstack=<stack>] [<conf>]\n");
	printf("  <option>:\n");
	printf("    -verbose       display verbose messages\n");
	printf("    -auto          auto nego enable            [default]\n");
	printf("    -no_auto       fixed mode\n");
	printf("    -strap         use pin-strap config\n");
	printf("    -no_strap      do not use pin-strap config [default]\n");

	return 2;
}

static inline void RestartAutoNegotiation(volatile u8 *emac3_regbase){
	if(VerbosityLevel) sceInetPrintf("smap: restarting auto nego (BMCR=0x%x, BMSR=0x%x)\n", _smap_read_phy(emac3_regbase, SMAP_DsPHYTER_BMCR), value);
	_smap_write_phy(emac3_regbase, SMAP_DsPHYTER_BMCR, SMAP_PHY_BMCR_ANEN|SMAP_PHY_BMCR_RSAN);	/* 0x1200 */
}

/* 0x00000998 */
static int InitPHY(struct SmapDriverData *SmapDrivPrivData){
	int i, result;
	unsigned int value, value2, LinkSpeed100M, LinkFDX, FlowControlEnabled, AutoNegoRetries;
	unsigned short int RegDump[6];
	volatile u8 *emac3_regbase;

	LinkSpeed100M=0;
	if(VerbosityLevel!=0) sceInetPrintf("smap: R\n");

	_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR, SMAP_PHY_BMCR_RST);
	for(i=0; _smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR)&SMAP_PHY_BMCR_RST; i++){
		if(i<=0){
			sceInetPrintf("smap: PHY reset error\n");
			return -1;
		}
		if(SmapDrivPrivData->NetDevStopFlag) return 0;

		DelayThread(0x3E8);
	}

	if(!EnableAutoNegotiation){	/* 0x00000a38 */
		if(VerbosityLevel!=0) sceInetPrintf("smap: no auto mode (conf=0x%x)\n", SmapConfiguration);

		LinkSpeed100M=0<(SmapConfiguration&0x180);	/* Toggles between SMAP_PHY_BMCR_10M and SMAP_PHY_BMCR_100M. */
		value=LinkSpeed100M<<13;
		if(SmapConfiguration&0x140) value|=SMAP_PHY_BMCR_DUPM;
		_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR, value);

WaitLink:
		sceInetPrintf("smap: Waiting Valid Link for %dMbps\n", LinkSpeed100M?100:10);

		i=0;
		while(1){
			DelayThread(0x30d40);
			if(SmapDrivPrivData->NetDevStopFlag) return 0;
			i++;
			if(_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMSR)&SMAP_PHY_BMSR_LINK) break;
			if(i>=5) SmapDrivPrivData->LinkStatus=0;
		}

		SmapDrivPrivData->LinkStatus=1;
	}
	else{
		if(!EnablePinStrapConfig){	/* 0x00000af0 */
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR, 0);
			value=_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMSR)&0xFFFF;
			if(!(value&0x4000)) SmapConfiguration=SmapConfiguration&0xFFFFFEFF;	/* 100Base-TX FDX */
			if(!(value&0x2000)) SmapConfiguration=SmapConfiguration&0xFFFFFF7F;	/* 100Base-TX HDX */
			if(!(value&0x1000)) SmapConfiguration=SmapConfiguration&0xFFFFFFBF;	/* 10Base-TX FDX */
			if(!(value&0x0800)) SmapConfiguration=SmapConfiguration&0xFFFFFFDF;	/* 10Base-TX HDX */

			sceInetPrintf("smap: no strap mode (conf=0x%x->0x%x, bmsr=0x%x)\n", SmapConfiguration, value);

			value=_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_ANAR)&0xFFFF;
			value=(SmapConfiguration&0x5E0)|(value&0x1F);
			sceInetPrintf("smap: anar=0x%x->0x%x\n", value);
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_ANAR, value);
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR, SMAP_PHY_BMCR_ANEN|SMAP_PHY_BMCR_RSAN);	/* 0x1200 */
		}
		else{	/* 0x00000c10 */
			if(!(_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR)&SMAP_PHY_BMCR_ANEN)){
				goto WaitLink;
			}
		}

		/* 0x00000c10 */
		sceInetPrintf("smap: auto mode (BMCR=0x%x ANAR=0x%x)\n", _smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR)&0xFFFF, _smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_ANAR)&0xFFFF);

RepeatAutoNegoProcess:
		for(AutoNegoRetries=0; AutoNegoRetries<3; AutoNegoRetries++){
			for(i=0; i<3; i++){
				DelayThread(0xf4240);
				if(SmapDrivPrivData->NetDevStopFlag) return 0;
			}

			value=_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMSR)&0xFFFF;
			if((value&(SMAP_PHY_BMSR_ANCP|0x10))==SMAP_PHY_BMSR_ANCP){	/* 0x30: SMAP_PHY_BMSR_ANCP and Remote fault. */
				/* This seems to be checking for the link-up status. */
				for(i=0; !(_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMSR)&SMAP_PHY_BMSR_LINK); i++){
					DelayThread(0x30d40);
					if(SmapDrivPrivData->NetDevStopFlag) return 0;
					if(i>=0x14) break;
				}

				if(i<0x14){
					/* Auto negotiaton completed successfully. */
					SmapDrivPrivData->LinkStatus=1;
					break;
				}
				else RestartAutoNegotiation(SmapDrivPrivData->emac3_regbase);
			}
			else RestartAutoNegotiation(SmapDrivPrivData->emac3_regbase);
		}

		/* 0x00000d38: If automatic negotiation fails, manually figure out which speed and duplex mode to use. */
		if(AutoNegoRetries>=3){
			if(VerbosityLevel) sceInetPrintf("smap: waiting valid link for 100Mbps Half-Duplex\n");

			_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR, SMAP_PHY_BMCR_100M);
			DelayThread(0xf4240);
			if(SmapDrivPrivData->NetDevStopFlag) return 0;

			for(i=0; !(_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMSR)&SMAP_PHY_BMSR_LINK); i++){
				DelayThread(0x186a0);
				if(SmapDrivPrivData->NetDevStopFlag) return 0;
				if(i>=0x1E) break;
			}

			if(i>=0x1E){
				if(VerbosityLevel) sceInetPrintf("smap: waiting valid link for 10Mbps Half-Duplex\n");

				_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR, 0);
				DelayThread(0xf4240);
				if(SmapDrivPrivData->NetDevStopFlag) return 0;

				for(i=0; !(_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMSR)&SMAP_PHY_BMSR_LINK); i++){
					DelayThread(0x186a0);
					if(SmapDrivPrivData->NetDevStopFlag) return 0;
					if(i>=0x1E) break;
				}

				if(i>=0x1E) goto RepeatAutoNegoProcess;
				else SmapDrivPrivData->LinkStatus=1;
			}
			else SmapDrivPrivData->LinkStatus=1;
		}
	}

	/* 0x00000e54 */
	for(i=0; i<6; i++) RegDump[i]=_smap_read_phy(SmapDrivPrivData->emac3_regbase, i);

	if(VerbosityLevel) sceInetPrintf("smap: PHY: %04x %04x %04x %04x %04x %04x\n", RegDump[SMAP_DsPHYTER_BMCR], RegDump[SMAP_DsPHYTER_BMSR], RegDump[SMAP_DsPHYTER_PHYIDR1], RegDump[SMAP_DsPHYTER_PHYIDR2], RegDump[SMAP_DsPHYTER_ANAR], RegDump[SMAP_DsPHYTER_ANLPAR]);

	/* if SMAP_DsPHYTER_PHYIDR1 == SMAP_PHY_IDR1_VAL && SMAP_DsPHYTER_PHYIDR2 == SMAP_PHY_IDR2_VAL */
	if(RegDump[SMAP_DsPHYTER_PHYIDR1]==SMAP_PHY_IDR1_VAL && (RegDump[SMAP_DsPHYTER_PHYIDR2]&SMAP_PHY_IDR2_MSK)==SMAP_PHY_IDR2_VAL){
		if(EnableAutoNegotiation){
			_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_FCSCR);
			_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_RECR);
			DelayThread(0x7a120);
			value=_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_FCSCR);
			value2=_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_RECR);
			if((value2&0xFFFF)!=0 || (value&0xFFFF)>=0x11){
				if(VerbosityLevel) sceInetPrintf("smap: FCSCR=%d RECR=%d\n", value&0xFFFF, value2&0xFFFF);
				_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMCR, 0);
				goto WaitLink;
			}
		}

		/* 0x00000f7c */
		sceInetPrintf("smap: PHY chip: DP83846A%d\n", RegDump[SMAP_DsPHYTER_PHYIDR2]&SMAP_PHY_IDR2_REV_MSK);

		if(!EnableAutoNegotiation){
			if(RegDump[SMAP_DsPHYTER_BMCR]&(SMAP_PHY_BMCR_DUPM|SMAP_PHY_BMCR_100M)){	/* if SMAP_DsPHYTER_BMCR&0x2100 */
				_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_10BTSCR, 0x104);
			}
		}
		else{
			if((RegDump[SMAP_DsPHYTER_ANAR]&0x1E0)==0x20){	/* SMAP_DsPHYTER_ANAR */
				_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_10BTSCR, 0x104);
			}
		}

		if((RegDump[SMAP_DsPHYTER_PHYIDR2]&SMAP_PHY_IDR2_REV_MSK)==0){	/* SMAP_DsPHYTER_PHYIDR2 */
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, 0x13, 1);
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_PHYCTRL, 0x1819);
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, 0x1F, 0);
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, 0x1D, 0x5040);
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, 0x1E, 0x8C);
			_smap_write_phy(SmapDrivPrivData->emac3_regbase, 0x13, 0);
		}
	}
	/* 0x00001048 */
	FlowControlEnabled=0;
	if(RegDump[SMAP_DsPHYTER_BMCR]&SMAP_PHY_BMCR_ANEN){	/* if SMAP_DsPHYTER_BMCR & SMAP_PHY_BMCR_ANEN */
		value=RegDump[SMAP_DsPHYTER_ANAR]&RegDump[SMAP_DsPHYTER_ANLPAR];
		LinkSpeed100M=0<(value&0x180);
		LinkFDX=0<(value&0x140);
		if(LinkFDX) FlowControlEnabled=0<(value&0x400);
	}
	else{	/* 0x0000108c */
		LinkSpeed100M=RegDump[SMAP_DsPHYTER_BMCR]>>13&1;
		LinkFDX=RegDump[SMAP_DsPHYTER_BMCR]>>8&1;
		FlowControlEnabled=SmapConfiguration>>10&1;
	}

	/* 0x000010ac */
	if(LinkSpeed100M) result=LinkFDX?8:4;
	else result=LinkFDX?2:1;

	/* 0x000010f4 */
	SmapDrivPrivData->LinkMode=result;
	if(FlowControlEnabled) SmapDrivPrivData->LinkMode|=0x40;

	/* 0x0000110c */
	sceInetPrintf("smap: %s %s Duplex Mode %s Flow Control\n", LinkSpeed100M?"100BaseTX":"10BaseT", LinkFDX?"Full":"Half", FlowControlEnabled?"with":"without");

	/* 0x0000114c */
	emac3_regbase=SmapDrivPrivData->emac3_regbase;
	value=SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE1)&0x67FFFFFF;
	if(LinkFDX) value|=SMAP_E3_FDX_ENABLE;	/* 0x80000000 */
	if(FlowControlEnabled) value|=SMAP_E3_FLOWCTRL_ENABLE|SMAP_E3_ALLOW_PF;	/* 0x18000000 */
	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE1, value);

	return 0;
}

/* 0x00001210 */
static unsigned int LinkCheckTimerCB(struct SmapDriverData *SmapDrivPrivData){
	iSetEventFlag(SmapDrivPrivData->Dev9IntrEventFlag, NETDEV_EVENT_LINK_CHECK);
	return SmapDrivPrivData->LinkCheckTimer.lo;
}

/* 0x00000584 */
static int HandleTxDNVIntr(struct SmapDriverData *SmapDrivPrivData){
	int result;
	volatile u8 *smap_regbase;
	unsigned int RegisterOffset, i;
	USE_SMAP_TX_BD;
	unsigned short int ctrl_stat;

	smap_regbase=SmapDrivPrivData->smap_regbase;
	result=0;
	if(SmapDrivPrivData->NumPacketsInTx>0){
		do{
			if(!((ctrl_stat=tx_bd[SmapDrivPrivData->TxDNVBDIndex&(SMAP_BD_MAX_ENTRY-1)].ctrl_stat)&SMAP_BD_TX_READY)){	/* 0x8000 */
				tx_bd[SmapDrivPrivData->TxDNVBDIndex&(SMAP_BD_MAX_ENTRY-1)].pointer;	/* Seems to get loaded, but never used? */
				if(ctrl_stat&(SMAP_BD_TX_UNDERRUN|SMAP_BD_TX_LCOLL|SMAP_BD_TX_ECOLL|SMAP_BD_TX_EDEFER|SMAP_BD_TX_LOSSCR)){	/* 0xF2 */
					for(i=0; i<16; i++){
						if(ctrl_stat>>i&1) SmapDrivPrivData->unknown7[17]++;	/* 0x000005E0: (offset*4+0x88)++ */
					}

					/* 0x00000614 */
					SmapDrivPrivData->unknown7[38]++;	/* Offset 0xDC */
					if(ctrl_stat&SMAP_BD_TX_LOSSCR){	/* 0x80 */
						SmapDrivPrivData->unknown7[58]++;	/* Offset0x12C */
					}
					if(ctrl_stat&SMAP_BD_TX_EDEFER){	/* 0x40 */
						SmapDrivPrivData->unknown7[61]++;	/* Offset0x138 */
					}
					/* 0x00000660 */
					if(ctrl_stat&(SMAP_BD_TX_SCOLL|SMAP_BD_TX_MCOLL|SMAP_BD_TX_LCOLL|SMAP_BD_TX_ECOLL)){	/* 0x3C */
						SmapDrivPrivData->unknown7[50]++;	/* Offset0x10C */
					}
					if(ctrl_stat&SMAP_BD_TX_UNDERRUN){	/* 0x2 */
						SmapDrivPrivData->unknown7[59]++;	/* Offset0x130 */
					}
				}
			}
			else return result;

			/* 0x00000694 */
			result++;
			SmapDrivPrivData->TxBufferSpaceAvailable+=(tx_bd[SmapDrivPrivData->TxDNVBDIndex&(SMAP_BD_MAX_ENTRY-1)].length+3)&~3;
			SmapDrivPrivData->TxDNVBDIndex++;
			SmapDrivPrivData->NumPacketsInTx--;
		}while(SmapDrivPrivData->NumPacketsInTx>0);
	}

	return result;
}

/* 0x00000214 */
static int SmapDmaTransfer(volatile u8 *smap_regbase, void *buffer, unsigned int size, int direction){
	unsigned short int NumBlocks;
	int result;

	if((NumBlocks=size>>7)<=0){
		if(dev9DmaTransfer(1, buffer, size<<16|0x20, direction)>=0){
			result=NumBlocks<<7;
		}
		else result=0;
	}
	else result=0;

	return result;
}

/* 0x00000254 */
static int HandleRxIntr(struct SmapDriverData *SmapDrivPrivData){
	USE_SMAP_REGS;
	USE_SMAP_RX_BD;
	int NumPacketsReceived, result;
	sceInetPkt_t *packet;
	unsigned int PacketLength, i, PacketBDPointer;
	volatile smap_bd_t *PktBdPtr;

	NumPacketsReceived=0;
	while(1){
		PktBdPtr=&rx_bd[SmapDrivPrivData->RxBDIndex&(SMAP_BD_MAX_ENTRY-1)];
		PacketLength=PktBdPtr->length;
		PacketBDPointer=PktBdPtr->pointer;
		if(!(PktBdPtr->ctrl_stat&SMAP_BD_RX_EMPTY)){	/* 0x8000 */
			if(PktBdPtr->ctrl_stat&(SMAP_BD_RX_INRANGE|SMAP_BD_RX_OUTRANGE|SMAP_BD_RX_FRMTOOLONG|SMAP_BD_RX_BADFCS|SMAP_BD_RX_ALIGNERR|SMAP_BD_RX_SHORTEVNT|SMAP_BD_RX_RUNTFRM|SMAP_BD_RX_OVERRUN)){	/* 0x27F */
				/* 0x000002c8 */
				for(i=0; i<16; i++){
					if(PktBdPtr->ctrl_stat>>i&1) SmapDrivPrivData->unknown7[i]++;
				}

				SmapDrivPrivData->unknown7[37]++;

				if(PktBdPtr->ctrl_stat&SMAP_BD_RX_OVERRUN) SmapDrivPrivData->unknown7[52]++;	/* 0x200 */
				if(PktBdPtr->ctrl_stat&(SMAP_BD_RX_INRANGE|SMAP_BD_RX_OUTRANGE|SMAP_BD_RX_FRMTOOLONG|SMAP_BD_RX_SHORTEVNT|SMAP_BD_RX_RUNTFRM)) SmapDrivPrivData->unknown7[51]++;	/* 0x67 */
				if(PktBdPtr->ctrl_stat&SMAP_BD_RX_BADFCS) SmapDrivPrivData->unknown7[53]++;	/* 0x08 */
				if(PktBdPtr->ctrl_stat&SMAP_BD_RX_ALIGNERR) SmapDrivPrivData->unknown7[54]++;	/* 0x10 */

				SMAP_REG16(SMAP_R_RXFIFO_RD_PTR)=PacketBDPointer+((PacketLength+3)&~3);
			}
			else{	/* 0x00000388 */
				var_00003280++;
				if((packet=sceInetAllocPkt(&SmapDrivPrivData->NetDevOps, (PacketLength+3)&~3))==NULL){
					/* 0x000003b8 */
					SmapDrivPrivData->unknown7[39]++;
					var_00003284++;
					SMAP_REG16(SMAP_R_RXFIFO_RD_PTR)=PacketBDPointer+((PacketLength+3)&~3);
				}
				else{
					/* 0x000003e8 */
					SMAP_REG16(SMAP_R_RXFIFO_RD_PTR)=PacketBDPointer;
					if((result=SmapDmaTransfer(SmapDrivPrivData->smap_regbase, packet->wp, PacketLength, 0))<=0){	/* packet + 0x14 */
						packet->wp+=result;
					}

					if(result<PacketLength){
						for(i=result; i<PacketLength; i+=4){
							*(unsigned int *)packet->wp=SMAP_REG32(SMAP_R_RXFIFO_DATA);
							packet->wp+=4;
						}
					}

					/* 0x0000044c */
					SmapDrivPrivData->unknown7[33]++;
					SmapDrivPrivData->unknown7[35]+=PacketLength;

					if(packet->rp[0]&1){
						SmapDrivPrivData->unknown7[49]++;
						if(*(unsigned int *)packet->rp!=0xFFFFFFFF && ((unsigned short int *)packet->rp)[1]==0xFFFF){
							/* 0x000004c0 */
							SmapDrivPrivData->unknown7[41]++;
							SmapDrivPrivData->unknown7[43]+=PacketLength&0xFFFF;
						}
						else{
							/* 000004DC */
							SmapDrivPrivData->unknown7[45]++;
							SmapDrivPrivData->unknown7[47]+=PacketLength;
						}
					}

					/* 0x00000500 */
					NumPacketsReceived++;
					packet->wp=packet->rp+PacketSize&0xFFFF;
					sceInetPktEnQ(&SmapDrivPrivData->NetDevOps, packet);
				}

				SMAP_REG8(SMAP_R_RXFIFO_FRAME_DEC)=0;
				PktBdPtr->ctrl_stat=SMAP_BD_RX_EMPTY;
				SmapDrivPrivData->RxBDIndex++;
			}
		}
		else{
			if(NumPacketsReceived!=0) SetEventFlag(SmapDrivPrivData+0x188, 4);
			return NumPacketsReceived;
		}
	}
}

/* 0x00000750 */
static int HandleTxReqs(struct SmapDriverData *SmapDrivPrivData){
	int result, PacketSize, i;
	void *pkt;
	USE_SMAP_REGS;
	USE_SMAP_TX_BD;
	volatile u8 *smap_regbase;
	volatile smap_bd_t *BD_ptr;
	u16 BD_data_ptr;

	result=0;
	while(1){
		if((pkt=SmapDrivPrivData->packet)==NULL){
			if((pkt=sceInetPktDeQ(SmapDrivPrivData+0x194))==NULL){
				return result;
			}
			SmapDrivPrivData->packet=pkt;
			return result;
		}

		/* 0x000007b4 */
		if(SmapDrivPrivData->NumPacketsInTx<SMAP_BD_MAX_ENTRY){	//<0x40
			PacketSize=pkt->wp-pkt->rp;
			if(PacketSize<=0 || SmapDrivPrivData->NumPacketsInTx&3){
				sceInetPrintf("smap: dropped\n");
			}
			else{
				/* 0x0000080c */
				if(SmapDrivPrivData->TxBufferSpaceAvailable>=(PacketSize+3)&~3){
					var_00003288++;
					if(pkt->buffer[0]&1 && *(unsigned int *)pkt->buffer!=0xFFFFFFFF && ((unsigned short int *)pkt->buffer)[1]==0xFFFF){
						/* 0x00000858 */
						SmapDrivPrivData->unknown7[42]++;
						SmapDrivPrivData->unknown7[44]+=PacketSize;
					}
					else{
						/* 0x00000874 */
						SmapDrivPrivData->unknown7[46]++;
						SmapDrivPrivData->unknown7[48]+=PacketSize;
					}

					/* 0x0000088c */
					smap_regbase=SmapDrivPrivData->smap_regbase;
					BD_data_ptr=SMAP_REG16(SMAP_R_TXFIFO_WR_PTR)+SMAP_TX_BASE;
					BD_ptr=&tx_bd[SmapDrivPrivData->TxBDIndex&(SMAP_BD_MAX_ENTRY-1)];

					/* 0x000008b4 */
					if((i=SmapDmaTransfer(SmapDrivPrivData->smap_regbase, pkt->buffer, PacketSize, 1))>0){
						pkt->buffer+=i;
					}
					else i=0;

					for(; i<PacketSize; i+=4){
						SMAP_REG32(SMAP_R_TXFIFO_DATA)=*(unsigned int*)pkt->buffer;		/* pkt + 0x10 */
						pkt->buffer+=4;
					}

					/* 0x000008f8 */
					result++;
					BD_ptr->length=PacketSize;
					BD_ptr->pointer=BD_data_ptr;
					SMAP_REG8(SMAP_R_TXFIFO_FRAME_INC)=0;
					BD_ptr->ctrl_stat=SMAP_BD_TX_READY|SMAP_BD_TX_GENFCS|SMAP_BD_TX_GENPAD;	/* 0x8300 */
					SmapDrivPrivData->TxBDIndex++;
					SmapDrivPrivData->NumPacketsInTx++;
					SmapDrivPrivData->TxBufferSpaceAvailable-=((PacketSize+3)&~3);
					SmapDrivPrivData->unknown7[36]+=PacketSize;
					SmapDrivPrivData->unknown7[34]++;
				}
				else return result;
			}
		}
		else return result;

		SmapDrivPrivData->packet=NULL;
		sceInetFreePkt(&SmapDrivPrivData->NetDevOps, pkt);
	}
}

/* 0x000011c4 */
static void CheckLinkStatus(struct SmapDriverData *SmapDrivPrivData){
	if(!(_smap_read_phy(SmapDrivPrivData->emac3_regbase, SMAP_DsPHYTER_BMSR)&SMAP_PHY_BMSR_LINK)){
		SmapDrivPrivData->LinkStatus=0;
		InitPHY(SmapDrivPrivData);
		ClearPacketQueue(SmapDrivPrivData);
	}
}

/* 0x00001298 */
static void IntrHandlerThread(struct SmapDriverData *SmapDrivPrivData){
	unsigned int EFBits, ResetCounterFlag, IntrReg;
	int result, counter;
	volatile u8 *emac3_regbase;
	USE_SMAP_REGS;
	USE_SPD_REGS;

	counter=3;
	emac3_regbase=SmapDrivPrivData->emac3_regbase;
	while(1){
		if((result=WaitEventFlag(SmapDriverData.Dev9IntrEventFlag, 0x1F, WEF_OR|WEF_CLEAR, &EFBits))!=0){
			sceInetPrintf("smap: WaitEventFlag -> %d\n", result);
			break;
		}

		if(EFBits&NETDEV_EVENT_STOP){	/* 0x0000131c */
			if(!SmapDrivPrivData->SmapIsInitialized){
				dev9IntrDisable(DEV9_SMAP_INTR_MASK2);
				SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, 0);
				SmapDrivPrivData->NetDevStopFlag=0;
				SmapDrivPrivData->SmapIsInitialized=0;
				SmapDrivPrivData->SmapDriverStarted=0;
			}
		}
		if(EFBits&NETDEV_EVENT_START){	/* 0x0000135c */
			if(!SmapDrivPrivData->SmapIsInitialized){
				SmapDrivPrivData->SmapDriverStarted=1;
				dev9IntrEnable(DEV9_SMAP_INTR_MASK2);
				if((result=InitPHY(SmapDrivPrivData))!=0) break;
				if(SmapDrivPrivData->NetDevStopFlag){
					SmapDrivPrivData->NetDevStopFlag=0;
					continue;
				}

				SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, SMAP_E3_TXMAC_ENABLE|SMAP_E3_RXMAC_ENABLE);	/* 0x1800000 */
				SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE0);
				DelayThread(0x2710);
				SmapDrivPrivData->SmapIsInitialized=1;
				SetEventFlag(SmapDrivPrivData + 0x188, 1);

				if(!SmapDrivPrivData->EnableLinkCheckTimer){
					USec2SysClock(0xf4240, &SmapDrivPrivData->LinkCheckTimer);
					SetAlarm(&SmapDrivPrivData->LinkCheckTimer, (void*)&LinkCheckTimerCB, SmapDrivPrivData);
					SmapDrivPrivData->EnableLinkCheckTimer=1;
				}
			}
		}

		if(SmapDrivPrivData->SmapIsInitialized==0) continue;

		ResetCounterFlag=0;
		if(EFBits&NETDEV_EVENT_INTR){	/* 0x00001424 */
			if((IntrReg=SPD_REG16(SPD_R_INTR_STAT)&DEV9_SMAP_INTR_MASK)!=0){
				IntrCounter++;
				if(IntrReg&SMAP_INTR_EMAC3){
					SMAP_EMAC3_GET(SMAP_R_EMAC3_INTR_STAT);
					SMAP_REG16(SMAP_R_INTR_CLR)=SMAP_INTR_EMAC3;
					SMAP_EMAC3_SET(SMAP_R_EMAC3_INTR_STAT, SMAP_E3_INTR_TX_ERR_0|SMAP_E3_INTR_SQE_ERR_0|SMAP_E3_INTR_DEAD_0);
					SMAP_EMAC3_GET(SMAP_R_EMAC3_INTR_STAT);
				}
				if(IntrReg&SMAP_INTR_RXEND){
					SMAP_REG16(SMAP_R_INTR_CLR)=SMAP_INTR_RXEND;
					ResetCounterFlag=HandleRxIntr(SmapDrivPrivData);
				}
				if(IntrReg&SMAP_INTR_RXDNV){
					SMAP_REG16(SMAP_R_INTR_CLR)=SMAP_INTR_RXDNV;
					SmapDrivPrivData->unknown7[52]++;
				}
				if(IntrReg&SMAP_INTR_TXDNV){
					HandleTxDNVIntr(SmapDrivPrivData);
				}
			}
		}

		if(EFBits&NETDEV_EVENT_XMIT){	/* 0x000014e8 */
			HandleTxReqs(SmapDrivPrivData);
		}
		HandleTxDNVIntr(SmapDrivPrivData);

		dev9IntrEnable(DEV9_SMAP_INTR_MASK2);

		if(SmapDrivPrivData->NumPacketsInTx>0){
			SMAP_EMAC3_SET(SMAP_R_EMAC3_TxMODE0, SMAP_E3_TX_GNP_0);	/* 0x00008000 */
			SMAP_EMAC3_GET(SMAP_R_EMAC3_TxMODE0);
			dev9IntrEnable(4);
		}

		if(ResetCounterFlag) counter=3;

		if(EFBits&0x10){	/* 0x00001548 */
			if(--counter<=0){
				CheckLinkStatus(SmapDrivPrivData);
			}
		}
	}
}

/* 0x00001240 */
static int Dev9IntrCb(int flag){
	__asm(	"\tmov $s0, $gp\n"	\
		"\tla $v0, _gp\n"	\
		"\tlw $gp, 0($v0)\n");

	dev9IntrDisable(DEV9_SMAP_ALL_INTR_MASK);
	iSetEventFlag(SmapDriverData.Dev9IntrEventFlag, NETDEV_EVENT_INTR);

	__asm(	"\tmov $gp, $s0\n");

	return 0;
}

/* 0x00000180 */
static void Dev9PreDmaCbHandler(int bcr, int dir){
	volatile u8 *smap_regbase;
	unsigned short int SliceCount;

	smap_regbase=SmapDriverData.smap_regbase;
	SliceCount=bcr>>16;
	if(dir!=0){
		SMAP_REG16(SMAP_R_TXFIFO_SIZE)=SliceCount;
		SMAP_REG8(SMAP_R_TXFIFO_CTRL)=SMAP_TXFIFO_DMAEN;	/* 0x02 */
	}
	else{
		SMAP_REG16(SMAP_R_RXFIFO_SIZE)=SliceCount;
		SMAP_REG8(SMAP_R_RXFIFO_CTRL)=SMAP_RXFIFO_DMAEN;	/* 0x02 */
	}
}

/* 0x000001B0 */
static void Dev9PostDmaCbHandler(int bcr, int dir){
	volatile u8 *smap_regbase;

	smap_regbase=SmapDriverData.smap_regbase;
	if(dir!=0){
		/* This is stupid, but it's how it is in the driver. The strange thing is that similar the loop below doesn't do this. */
		if(SMAP_REG8(SMAP_R_TXFIFO_CTRL)&SMAP_TXFIFO_DMAEN){
			while(SMAP_REG8(SMAP_R_TXFIFO_CTRL)&SMAP_TXFIFO_DMAEN){};
		}
	}
	else{
		while(SMAP_REG8(SMAP_R_RXFIFO_CTRL)&SMAP_RXFIFO_DMAEN){};
	}
}

/* 0x000015a8 */
static int NetDevStart(void *priv, int flags){
	__asm(	"\tmov $s0, $gp\n"	\
		"\tla $v0, _gp\n"	\
		"\tlw $gp, 0($v0)\n");

	SetEventFlag(((struct SmapDriverData*)priv)->Dev9IntrEventFlag, NETDEV_EVENT_START);

	__asm(	"\tmov $gp, $s0\n");

	return 0;
}

/* 0x000015e4 */
static int NetDevStop(void *priv, int flags){
	__asm(	"\tmov $s0, $gp\n"	\
		"\tla $v0, _gp\n"	\
		"\tlw $gp, 0($v0)\n");

	((struct SmapDriverData*)priv)->NetDevStopFlag=1;
	SetEventFlag(((struct SmapDriverData*)priv)->Dev9IntrEventFlag, NETDEV_EVENT_STOP);

	__asm(	"\tmov $gp, $s0\n");

	return 0;
}

/* 0x000006d8 */
static void ClearPacketQueue(struct SmapDriverData *SmapDrivPrivData){
	int OldState, result;
	sceInetPkt_t *pkt;

	CpuSuspendIntr(&OldState);
	pkt=SmapDrivPrivData->packet;
	CpuResumeIntr(OldState);

	if(pkt!=NULL){
		while((pkt=sceInetPktDeQ(sceInetPktQ_t *que))!=NULL){
			 sceInetFreePkt(&SmapDrivPrivData->NetDevOps, pkt);
		};
	}
}

/* 0x00001628 */
static int NetDevXmit(void *priv, int flags){
	__asm(	"\tmov $s0, $gp\n"	\
		"\tla $v0, _gp\n"	\
		"\tlw $gp, 0($v0)\n");


	if(((struct SmapDriverData*)priv)->LinkStatus==0){
		ClearPacketQueue(priv);
	}
	else{
		SetEventFlag(((struct SmapDriverData*)priv)->Dev9IntrEventFlag, NETDEV_EVENT_XMIT);
	}

	__asm(	"\tmov $gp, $s0\n");

	return 0;
}

/* 0x00001848 */
static int NetDevControl(void *priv, int code, void *ptr, int len){
	__asm(	"\tmov $s0, $gp\n"	\
		"\tla $v0, _gp\n"	\
		"\tlw $gp, 0($v0)\n");







	__asm(	"\tmov $gp, $s0\n");

	return 0;
}


/* 0x00001d70 */
static int SetupNetDev(void){
	int result;
	iop_event_t EventFlagData;
	iop_thread_t ThreadData;
	unsigned int mac_address_lo, mac_address_hi;
	volatile u8 *emac3_regbase;

	emac3_regbase=SmapDriverData.emac3_regbase;
	SmapDriverData.NetDevOps.module_name=NetDeviceModuleName;
	SmapDriverData.NetDevOps.vendor_name=NetDeviceVendorName;
	SmapDriverData.NetDevOps.device_name=NetDeviceName;
	SmapDriverData.NetDevOps.bus_type=sceInetBus_NIC;	/* Type 5 */
	SmapDriverData.NetDevOps.prot_ver=sceInetDevProtVer;	/* Protocol version 2 */
	SmapDriverData.NetDevOps.flags=sceInetDevF_Multicast|sceInetDevF_NIC|sceInetDevF_ARP;	/* 0x490 */	
	SmapDriverData.NetDevOps.start=&NetDevStart;
	SmapDriverData.NetDevOps.stop=&NetDevStop;
	SmapDriverData.NetDevOps.xmit=&NetDevXmit;
	SmapDriverData.NetDevOps.impl_ver=0;			/* Implementation version. */
	SmapDriverData.NetDevOps.priv=&SmapDriverData;
	SmapDriverData.NetDevOps.control=&NetDevControl;
	SmapDriverData.NetDevOps.mtu=1500;

	EventFlagData.attr=0;
	EventFlagData.option=0;
	EventFlagData.bits=0;

	mac_address_hi=SMAP_EMAC3_GET(SMAP_R_EMAC3_ADDR_HI);
	mac_address_lo=SMAP_EMAC3_GET(SMAP_R_EMAC3_ADDR_LO);
	SmapDriverData.NetDevOps.hw_addr[0]=mac_address_hi>>8;
	SmapDriverData.NetDevOps.hw_addr[1]=mac_address_hi;
	SmapDriverData.NetDevOps.hw_addr[2]=mac_address_lo>>24;
	SmapDriverData.NetDevOps.hw_addr[3]=mac_address_lo>>16;
	SmapDriverData.NetDevOps.hw_addr[4]=mac_address_lo>>8;
	SmapDriverData.NetDevOps.hw_addr[5]=mac_address_lo;

	if((result=SmapDriverData.Dev9IntrEventFlag=CreateEventFlag(&EventFlagData))<0){
		sceInetPrintf("smap: CreateEventFlag -> %d\n", result);
		return -6;
	}

	ThreadData.attr=TH_C;
	ThreadData.thread=(void*)&IntrHandlerThread;
	ThreadData.option=0;
	ThreadData.priority=ThreadPriority;
	ThreadData.stacksize=ThreadStackSize;
	if((result=SmapDriverData.IntrHandlerThreadID=CreateThread(&ThreadData))<0){
		sceInetPrintf("smap: CreateThread -> %d\n", result);
		DeleteEventFlag(SmapDriverData.Dev9IntrEventFlag);
		return result;
	}

	if((result=StartThread(SmapDriverData.IntrHandlerThreadID, &SmapDriverData))<0){
		sceInetPrintf("smap: StartThread -> %d\n", result);
		DeleteThread(SmapDriverData.IntrHandlerThreadID);
		DeleteEventFlag(SmapDriverData.Dev9IntrEventFlag);
		return result;
	}

	if((result=sceInetRegisterNetDevice(&SmapDriverData.NetDevOps))<0){
		sceInetPrintf("smap: sceInetRegisterNetDevice -> %d\n", result);
		TerminateThread(SmapDriverData.IntrHandlerThreadID);
		DeleteThread(SmapDriverData.IntrHandlerThreadID);
		DeleteEventFlag(SmapDriverData.Dev9IntrEventFlag);
		return -6;
	}

	return 0;
}

/* 0x00002000 */
static int ParseSmapConfiguration(const char *cmd, unsigned int *configuration){
	const char *CmdStart, *DigitStart;
	unsigned int result, base, character, value;

	DigitStart=CmdStart=cmd;
	base=10;

	if(CmdStart[0]=='0'){
		if(CmdStart[1]!='\0'){
			if(CmdStart[1]=='x'){
				DigitStart+=2;
				base=16;
			}
			else{
				DigitStart++;
			}
		}
	}
	if(DigitStart[0]=='\0'){
		goto fail_end;
	}

	result=0;
	character=DigitStart[0];
	do{
		if(character-'0'<10){
			value=character-'0';
		}
		else if(character-'a'<6){
			value=character-'a'-0x57;
		}
		else goto fail_end;

		if(value>=base) goto fail_end;

		result=result*base+value;
	}while((character=*(++DigitStart))!='\0');
	*configuration=result;

	return 0;
fail_end:
	sceInetPrintf("smap: %s: %s - invalid digit\n", "scan_number", CmdStart);
	return -1;
}

/* 0x000020e0 */
static int smap_init(int argc, char *argv[]){
	int result, i;
	const char *CmdString;
	unsigned short int eeprom_data[4], checksum16;
	unsigned int mac_address;
	volatile u8 *emac3_regbase;
	USE_SPD_REGS;
	USE_SMAP_REGS;
	USE_SMAP_TX_BD;
	USE_SMAP_RX_BD;

	checksum16=0;
	argc--;
	argv++;
	while(argc>0){
		if(strcmp("-help", *argv)==0){
			return DisplayHelpMessage();
		}
		else if(strcmp("-version", *argv)==0){
			return DisplayBanner();
		}
		else if(strcmp("-verbose", *argv)==0){
			VerbosityLevel=1;
		}
		else if(strcmp("-auto", *argv)==0){
			EnableAutoNegotiation=1;
		}
		else if(strcmp("-no_auto", *argv)==0){
			EnableAutoNegotiation=0;
		}
		else if(strcmp("-strap", *argv)==0){
			EnablePinStrapConfig=1;
		}
		else if(strcmp("-no_strap", *argv)==0){
			EnablePinStrapConfig=0;
		}
		else if(strncmp("thpri=", *argv, 6)==0){
			CmdString=&((unsigned char*)*argv)[6];
			if(look_ctype_table(CmdString[0])&4){
				ThreadPriority=strtoul(&((unsigned char*)*argv)[6], NULL, 10);
				if(ThreadPriority-8>=0x73){
					return DisplayHelpMessage();
				}
				
				if(((unsigned char*)*argv)[6]!='\0'){	/* 0x0000226c */
					while(look_ctype_table(*CmdString)&4){
						CmdString++;
					}
					if(*CmdString!='\0') return DisplayHelpMessage();
				}
			}
			else return DisplayHelpMessage();
		}
		else if(strncmp("thstack=", *argv, 8)==0){	/* 0x000022ac */
			CmdString=&((unsigned char*)*argv)[8];
			if(look_ctype_table(CmdString[0])&4){
				ThreadStackSize=strtoul(&((unsigned char*)*argv)[8], NULL, 10);				
				if(((unsigned char*)*argv)[8]!='\0'){	/* 0x000022f4 */
					while(look_ctype_table(*CmdString)&4){
						CmdString++;
					}
				}

				if(strcmp(CmdString, "KB")==0) ThreadStackSize<<=10;
			}
			else return DisplayHelpMessage();
		}
		else{	/* 0x00002374 */
			if(ParseSmapConfiguration(*argv, &SmapConfiguration)!=0) return DisplayHelpMessage();
		}

		argc--;
		argv++;
	}

	if(argc!=0) return DisplayHelpMessage();

	/* 0x0000239c */
	SmapDriverData.smap_regbase=(volatile u8 *)SMAP_REGBASE;
	emac3_regbase=SmapDriverData.emac3_regbase=(volatile u8 *)(SMAP_REGBASE + SMAP_EMAC3_REGBASE);
	if(!SPD_REG16(SPD_R_REV_3)&SPD_CAPS_SMAP) return -1;
	if(SPD_REG16(SPD_R_REV_1)<0x11) return -6;	// Minimum: revision 17, ES2.

	dev9IntrDisable(DEV9_SMAP_ALL_INTR_MASK);

	/* Reset FIFOs. */
	SMAP_REG8(SMAP_R_TXFIFO_CTRL)=SMAP_TXFIFO_RESET;
	for(i=9; SMAP_REG8(SMAP_R_TXFIFO_CTRL)&SMAP_TXFIFO_RESET; i--){
		if(i<=0) return -2;
		DelayThread(0x3E8);
	}

	SMAP_REG8(SMAP_R_RXFIFO_CTRL)=SMAP_RXFIFO_RESET;
	for(i=9; SMAP_REG8(SMAP_R_RXFIFO_CTRL)&SMAP_RXFIFO_RESET; i--){
		if(i<=0) return -3;
		DelayThread(0x3E8);
	}

	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, SMAP_E3_SOFT_RESET);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE0);
	for(i=9; SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE0)&SMAP_E3_SOFT_RESET; i--){
		if(i<=0) return -4;
		DelayThread(0x3E8);
	}

	for(i=0; i<SMAP_BD_MAX_ENTRY; i++){
		tx_bd[i].ctrl_stat=0;
		tx_bd[i].reserved=0;
		tx_bd[i].length=0;
		tx_bd[i].pointer=0;
	}

	for(i=0; i<SMAP_BD_MAX_ENTRY; i++){
		rx_bd[i].ctrl_stat=SMAP_BD_RX_EMPTY;
		rx_bd[i].reserved=0;
		rx_bd[i].length=0;
		rx_bd[i].pointer=0;
	}

	SMAP_REG16(SMAP_R_INTR_CLR)=DEV9_SMAP_ALL_INTR_MASK;

	/* Retrieve the MAC address and verify it's integrity. */
	bzero(eeprom_data, 8);
	if((result=dev9GetEEPROM(eeprom_data))<0){	/* 0x00002514 */
		return(result==-1?-7:-4);
	}

	for(i=0; i<3; i++) checksum16+=eeprom_data[i];
	if(eeprom_data[0]==0 && eeprom_data[1]==0 && eeprom_data[2]==0){
		return -5;
	}
	if(checksum16!=eeprom_data[3]) return -5;

	/* 0x000025b4 */
	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE1, SMAP_E3_FDX_ENABLE|SMAP_E3_IGNORE_SQE|SMAP_E3_MEDIA_100M|SMAP_E3_RXFIFO_2K|SMAP_E3_TXFIFO_1K|SMAP_E3_TXREQ0_MULTI|SMAP_E3_TXREQ1_SINGLE);	/* SMAP_R_EMAC3_MODE1=0x80008164 */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE1);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_TxMODE1, 7<<SMAP_E3_TX_LOW_REQ_BITSFT | 0xF<<SMAP_E3_TX_URG_REQ_BITSFT);	/* SMAP_R_EMAC3_TxMODE1=0x0000380F */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_TxMODE1);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_RxMODE, SMAP_E3_RX_STRIP_PAD|SMAP_E3_RX_STRIP_FCS|SMAP_E3_RX_INDIVID_ADDR|SMAP_E3_RX_BCAST|SMAP_E3_RX_MCAST);	/* SMAP_R_EMAC3_RxMODE=0x0000C058 */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_RxMODE);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTR_STAT, SMAP_E3_INTR_TX_ERR_0|SMAP_E3_INTR_SQE_ERR_0|SMAP_E3_INTR_DEAD_0);	/* SMAP_R_EMAC3_INTR_STAT=0x01c00000 */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_INTR_STAT);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTR_ENABLE, SMAP_E3_INTR_TX_ERR_0|SMAP_E3_INTR_SQE_ERR_0|SMAP_E3_INTR_DEAD_0);	/* SMAP_R_EMAC3_INTR_ENABLE=0x01c00000 */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_INTR_ENABLE);

	mac_address=eeprom_data[0]>>8 | eeprom_data[0]<<8;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_ADDR_HI, mac_address);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_ADDR_HI);

	mac_address=eeprom_data[1]>>8|eeprom_data[1]<<8;
	mac_address=mac_address<<16|eeprom_data[2]>>8|eeprom_data[2]<<8;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_ADDR_LO, mac_address);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_ADDR_LO);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_PAUSE_TIMER, 0xFFFF);	/* SMAP_R_EMAC3_PAUSE_TIMER=0xFFFF0000 */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_PAUSE_TIMER);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH1, 0);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_GROUP_HASH1);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH2, 0);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_GROUP_HASH2);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH3, 0);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_GROUP_HASH3);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH4, 0);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_GROUP_HASH4);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTER_FRAME_GAP, 4);	/* SMAP_R_EMAC3_INTER_FRAME_GAP=0x00040000 */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_INTER_FRAME_GAP);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_TX_THRESHOLD, 0xC<<SMAP_E3_TX_THRESHLD_BITSFT);	/* SMAP_R_EMAC3_TX_THRESHOLD=0x00006000 */
	SMAP_EMAC3_GET(SMAP_R_EMAC3_TX_THRESHOLD);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_RX_WATERMARK, 0x8<<SMAP_E3_RX_LO_WATER_BITSFT|0x40<<SMAP_E3_RX_HI_WATER_BITSFT);
	SMAP_EMAC3_GET(SMAP_R_EMAC3_RX_WATERMARK);

	/* 0x000026b8 */
	for(i=2; i<7; i++) dev9RegisterIntrCb(i, &Dev9IntrCb);

	dev9RegisterPreDmaCb(1, &Dev9PreDmaCbHandler);
	dev9RegisterPostDmaCb(1, &Dev9PostDmaCbHandler);

	return SetupNetDev();
}

/* 0x00002720 */
static int func_00002720(void){
	int ModuleIdList[1], NumModules, result;

	NumModules=-1;
	GetModuleIdList(ModuleIdList, 1, &NumModules);
	result=~NumModules;	/* NumModules nor $zero */
	return result>>31;
}

int _start(int argc, char *argv[]){
	const char *ErrorMessage;
	int ModuleID, result;

	if(argc<0){
		/* Deinitialization and unregisteration code. */


		return MODULE_NO_RESIDENT_END;
	}
	else{
		if(RegisterLibraryEntries(&_exp_smap)!=0){
			sceInetPrintf("smap: module already loaded\n");
			return MODULE_NO_RESIDENT_END;
		}

		DisplayBanner();

		if((ModuleID=SearchModuleByName("dev9"))<0){
			sceInetPrintf("smap: dev9 module not found\n");
			return MODULE_NO_RESIDENT_END;
		}
		if(ReferModuleStatus(ModuleID, &ModStatus)<0){
			sceInetPrintf("smap: can't get dev9 module status\n");
			return MODULE_NO_RESIDENT_END;
		}
		if(ModStatus.version<0x204){
			sceInetPrintf("smap: dev9 module version must be 2.4 or later\n");
			return MODULE_NO_RESIDENT_END;
		}

		if((result=smap_init(argc, argv))<0){
			sceInetPrintf("smap: smap_init -> %d\n", result);
			ReleaseLibraryEntries(&_exp_smap);
			return MODULE_NO_RESIDENT_END;
		}

		return(func_00002720()<0?MODULE_NO_RESIDENT_END:MODULE_RESIDENT_END);
	}
}
