/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# SMAP (PS2 Network Adapter) register definitions.
*/

#ifndef SMAPREGS_H
#define SMAPREGS_H

#include "types.h"
#include "speedregs.h"

/* SMAP interrupt status bits (selected from the SPEED device).  */
#define	SMAP_INTR_EMAC3			(1<<6)
#define	SMAP_INTR_RXEND			(1<<5)
#define	SMAP_INTR_TXEND			(1<<4)
#define	SMAP_INTR_RXDNV			(1<<3)	/* descriptor not valid */
#define	SMAP_INTR_TXDNV			(1<<2)	/* descriptor not valid */
#define	SMAP_INTR_CLR_ALL		(SMAP_INTR_RXEND|SMAP_INTR_TXEND|SMAP_INTR_RXDNV)
#define	SMAP_INTR_ENA_ALL		(SMAP_INTR_EMAC3|SMAP_INTR_CLR_ALL)
#define	SMAP_INTR_BITMSK		0x7C

/* SMAP Register Definitions.  */

#define SMAP_REGBASE			(SPD_REGBASE + 0x100)

#define USE_SMAP_REGS	volatile u8 *smap_regbase = \
	(volatile u8 *)SMAP_REGBASE

#define	SMAP_REG8(offset)	(*(volatile u8 *)(smap_regbase + (offset)))
#define	SMAP_REG16(offset)	(*(volatile u16 *)(smap_regbase + (offset)))
#define	SMAP_REG32(offset)	(*(volatile u32 *)(smap_regbase + (offset)))

#define	SMAP_R_BD_MODE			0x02
#define	  SMAP_BD_SWAP		(1<<0)

#define	SMAP_R_INTR_CLR			0x28

/* SMAP FIFO Registers.  */

#define	SMAP_R_TXFIFO_CTRL		0xf00
#define	  SMAP_TXFIFO_RESET	(1<<0)
#define   SMAP_TXFIFO_DMAEN	(1<<1)
#define	SMAP_R_TXFIFO_WR_PTR		0xf04
#define SMAP_R_TXFIFO_SIZE		0xf08
#define	SMAP_R_TXFIFO_FRAME_CNT		0xf0C
#define	SMAP_R_TXFIFO_FRAME_INC		0xf10
#define	SMAP_R_TXFIFO_DATA		0x1000

#define	SMAP_R_RXFIFO_CTRL		0xf30
#define	  SMAP_RXFIFO_RESET	(1<<0)
#define   SMAP_RXFIFO_DMAEN	(1<<1)
#define	SMAP_R_RXFIFO_RD_PTR		0xf34
#define SMAP_R_RXFIFO_SIZE		0xf38
#define	SMAP_R_RXFIFO_FRAME_CNT		0xf3C
#define	SMAP_R_RXFIFO_FRAME_DEC		0xf40
#define	SMAP_R_RXFIFO_DATA		0x1100

#define	SMAP_R_FIFO_ADDR		0x1200
#define	  SMAP_FIFO_CMD_READ	(1<<1)
#define	  SMAP_FIFO_DATA_SWAP	(1<<0)
#define	SMAP_R_FIFO_DATA		0x1208

/* EMAC3 Registers.  */

#define	SMAP_EMAC3_REGBASE		0x1f00

/* Seperating out the EMAC3 registers makes it a lot easier to debug register
   assigns.  Besides, I am a staunch supporter of preprocessor macro abuse. */
#define USE_SMAP_EMAC3_REGS	volatile u8 *emac3_regbase = \
	(volatile u8 *)(SMAP_REGBASE + SMAP_EMAC3_REGBASE)

#define SMAP_EMAC3_REG(offset)	(*(volatile u16 *)(emac3_regbase + (offset)))

#define SMAP_EMAC3_GET(offset)	((SMAP_EMAC3_REG((offset)) << 16) | \
	(SMAP_EMAC3_REG((offset)+2)))

#define SMAP_EMAC3_SET(offset, val)					\
	do {								\
		SMAP_EMAC3_REG((offset))   = ((val) >> 16) & 0xffff;	\
		SMAP_EMAC3_REG((offset)+2) = (val) & 0xffff;		\
	} while (0);

#define	SMAP_R_EMAC3_MODE0		0x00
#define	  SMAP_E3_RXMAC_IDLE		(1<<31)
#define	  SMAP_E3_TXMAC_IDLE		(1<<30)
#define	  SMAP_E3_SOFT_RESET		(1<<29)
#define	  SMAP_E3_TXMAC_ENABLE		(1<<28)
#define	  SMAP_E3_RXMAC_ENABLE		(1<<27)
#define	  SMAP_E3_WAKEUP_ENABLE		(1<<26)

#define	SMAP_R_EMAC3_MODE1		0x04
#define	  SMAP_E3_FDX_ENABLE		(1<<31)
#define	  SMAP_E3_INLPBK_ENABLE		(1<<30)	/* internal loop back */
#define	  SMAP_E3_VLAN_ENABLE		(1<<29)
#define	  SMAP_E3_FLOWCTRL_ENABLE	(1<<28)	/* integrated flow ctrl(pause frame) */
#define	  SMAP_E3_ALLOW_PF		(1<<27)	/* allow pause frame */
#define	  SMAP_E3_ALLOW_EXTMNGIF	(1<<25)	/* allow external management IF */
#define	  SMAP_E3_IGNORE_SQE		(1<<24)
#define	  SMAP_E3_MEDIA_FREQ_BITSFT	(22)
#define	    SMAP_E3_MEDIA_10M		(0<<22)
#define	    SMAP_E3_MEDIA_100M		(1<<22)
#define	    SMAP_E3_MEDIA_1000M		(2<<22)
#define	    SMAP_E3_MEDIA_MSK		(3<<22)
#define	  SMAP_E3_RXFIFO_SIZE_BITSFT	(20)
#define	    SMAP_E3_RXFIFO_512		(0<<20)
#define	    SMAP_E3_RXFIFO_1K		(1<<20)
#define	    SMAP_E3_RXFIFO_2K		(2<<20)
#define	    SMAP_E3_RXFIFO_4K		(3<<20)
#define	  SMAP_E3_TXFIFO_SIZE_BITSFT	(18)
#define	    SMAP_E3_TXFIFO_512		(0<<18)
#define	    SMAP_E3_TXFIFO_1K		(1<<18)
#define	    SMAP_E3_TXFIFO_2K		(2<<18)
#define	  SMAP_E3_TXREQ0_BITSFT		(15)
#define	    SMAP_E3_TXREQ0_SINGLE	(0<<15)
#define	    SMAP_E3_TXREQ0_MULTI	(1<<15)
#define	    SMAP_E3_TXREQ0_DEPEND	(2<<15)
#define	  SMAP_E3_TXREQ1_BITSFT		(13)
#define	    SMAP_E3_TXREQ1_SINGLE	(0<<13)
#define	    SMAP_E3_TXREQ1_MULTI	(1<<13)
#define	    SMAP_E3_TXREQ1_DEPEND	(2<<13)
#define	  SMAP_E3_JUMBO_ENABLE		(1<<12)

#define	SMAP_R_EMAC3_TxMODE0		0x08
#define	  SMAP_E3_TX_GNP_0		(1<<31)	/* get new packet */
#define	  SMAP_E3_TX_GNP_1		(1<<30)	/* get new packet */
#define	  SMAP_E3_TX_GNP_DEPEND		(1<<29)	/* get new packet */
#define	  SMAP_E3_TX_FIRST_CHANNEL	(1<<28)

#define	SMAP_R_EMAC3_TxMODE1		0x0C
#define	  SMAP_E3_TX_LOW_REQ_MSK	(0x1F)	/* low priority request */
#define	  SMAP_E3_TX_LOW_REQ_BITSFT	(27)	/* low priority request */
#define	  SMAP_E3_TX_URG_REQ_MSK	(0xFF)	/* urgent priority request */
#define	  SMAP_E3_TX_URG_REQ_BITSFT	(16)	/* urgent priority request */

#define	SMAP_R_EMAC3_RxMODE		0x10
#define	  SMAP_E3_RX_STRIP_PAD		(1<<31)
#define	  SMAP_E3_RX_STRIP_FCS		(1<<30)
#define	  SMAP_E3_RX_RX_RUNT_FRAME	(1<<29)
#define	  SMAP_E3_RX_RX_FCS_ERR		(1<<28)
#define	  SMAP_E3_RX_RX_TOO_LONG_ERR	(1<<27)
#define	  SMAP_E3_RX_RX_IN_RANGE_ERR	(1<<26)
#define	  SMAP_E3_RX_PROP_PF		(1<<25)	/* propagate pause frame */
#define	  SMAP_E3_RX_PROMISC		(1<<24)
#define	  SMAP_E3_RX_PROMISC_MCAST	(1<<23)
#define	  SMAP_E3_RX_INDIVID_ADDR	(1<<22)
#define	  SMAP_E3_RX_INDIVID_HASH	(1<<21)
#define	  SMAP_E3_RX_BCAST		(1<<20)
#define	  SMAP_E3_RX_MCAST		(1<<19)

#define	SMAP_R_EMAC3_INTR_STAT		0x14
#define	SMAP_R_EMAC3_INTR_ENABLE	0x18
#define	  SMAP_E3_INTR_OVERRUN		(1<<25)	/* this bit does NOT WORKED */
#define	  SMAP_E3_INTR_PF		(1<<24)
#define	  SMAP_E3_INTR_BAD_FRAME	(1<<23)
#define	  SMAP_E3_INTR_RUNT_FRAME	(1<<22)
#define	  SMAP_E3_INTR_SHORT_EVENT	(1<<21)
#define	  SMAP_E3_INTR_ALIGN_ERR	(1<<20)
#define	  SMAP_E3_INTR_BAD_FCS		(1<<19)
#define	  SMAP_E3_INTR_TOO_LONG		(1<<18)
#define	  SMAP_E3_INTR_OUT_RANGE_ERR	(1<<17)
#define	  SMAP_E3_INTR_IN_RANGE_ERR	(1<<16)
#define	  SMAP_E3_INTR_DEAD_DEPEND	(1<<9)
#define	  SMAP_E3_INTR_DEAD_0		(1<<8)
#define	  SMAP_E3_INTR_SQE_ERR_0	(1<<7)
#define	  SMAP_E3_INTR_TX_ERR_0		(1<<6)
#define	  SMAP_E3_INTR_DEAD_1		(1<<5)
#define	  SMAP_E3_INTR_SQE_ERR_1	(1<<4)
#define	  SMAP_E3_INTR_TX_ERR_1		(1<<3)
#define	  SMAP_E3_INTR_MMAOP_SUCCESS	(1<<1)
#define	  SMAP_E3_INTR_MMAOP_FAIL	(1<<0)
#define	  SMAP_E3_INTR_ALL	\
	(SMAP_E3_INTR_OVERRUN|SMAP_E3_INTR_PF|SMAP_E3_INTR_BAD_FRAME| \
	 SMAP_E3_INTR_RUNT_FRAME|SMAP_E3_INTR_SHORT_EVENT| \
	 SMAP_E3_INTR_ALIGN_ERR|SMAP_E3_INTR_BAD_FCS| \
	 SMAP_E3_INTR_TOO_LONG|SMAP_E3_INTR_OUT_RANGE_ERR| \
	 SMAP_E3_INTR_IN_RANGE_ERR| \
	 SMAP_E3_INTR_DEAD_DEPEND|SMAP_E3_INTR_DEAD_0| \
	 SMAP_E3_INTR_SQE_ERR_0|SMAP_E3_INTR_TX_ERR_0| \
	 SMAP_E3_INTR_DEAD_1|SMAP_E3_INTR_SQE_ERR_1| \
	 SMAP_E3_INTR_TX_ERR_1| \
	 SMAP_E3_INTR_MMAOP_SUCCESS|SMAP_E3_INTR_MMAOP_FAIL)
#define	  SMAP_E3_DEAD_ALL	\
	(SMAP_E3_INTR_DEAD_DEPEND|SMAP_E3_INTR_DEAD_0| \
	 SMAP_E3_INTR_DEAD_1)

#define	SMAP_R_EMAC3_ADDR_HI		0x1C
#define	SMAP_R_EMAC3_ADDR_LO		0x20

#define	SMAP_R_EMAC3_VLAN_TPID		0x24
#define	  SMAP_E3_VLAN_ID_MSK		0xFFFF

#define	SMAP_R_EMAC3_VLAN_TCI		0x28
#define	  SMAP_E3_VLAN_TCITAG_MSK	0xFFFF

#define	SMAP_R_EMAC3_PAUSE_TIMER	0x2C
#define	  SMAP_E3_PTIMER_MSK		0xFFFF

#define	SMAP_R_EMAC3_INDIVID_HASH1	0x30
#define	SMAP_R_EMAC3_INDIVID_HASH2	0x34
#define	SMAP_R_EMAC3_INDIVID_HASH3	0x38
#define	SMAP_R_EMAC3_INDIVID_HASH4	0x3C
#define	SMAP_R_EMAC3_GROUP_HASH1	0x40
#define	SMAP_R_EMAC3_GROUP_HASH2	0x44
#define	SMAP_R_EMAC3_GROUP_HASH3	0x48
#define	SMAP_R_EMAC3_GROUP_HASH4	0x4C
#define	  SMAP_E3_HASH_MSK		0xFFFF

#define	SMAP_R_EMAC3_LAST_SA_HI		0x50
#define	SMAP_R_EMAC3_LAST_SA_LO		0x54

#define	SMAP_R_EMAC3_INTER_FRAME_GAP	0x58
#define	  SMAP_E3_IFGAP_MSK		0x3F

#define	SMAP_R_EMAC3_STA_CTRL		0x5C
#define	  SMAP_E3_PHY_DATA_MSK		(0xFFFF)
#define	  SMAP_E3_PHY_DATA_BITSFT	(16)
#define	  SMAP_E3_PHY_OP_COMP		(1<<15)	/* operation complete */
#define	  SMAP_E3_PHY_ERR_READ		(1<<14)
#define	  SMAP_E3_PHY_STA_CMD_BITSFT	(12)
#define	    SMAP_E3_PHY_READ		(1<<12)
#define	    SMAP_E3_PHY_WRITE		(2<<12)
#define	  SMAP_E3_PHY_OPBCLCK_BITSFT	(10)
#define	    SMAP_E3_PHY_50M		(0<<10)
#define	    SMAP_E3_PHY_66M		(1<<10)
#define	    SMAP_E3_PHY_83M		(2<<10)
#define	    SMAP_E3_PHY_100M		(3<<10)
#define	  SMAP_E3_PHY_ADDR_MSK		(0x1F)
#define	  SMAP_E3_PHY_ADDR_BITSFT	(5)
#define	  SMAP_E3_PHY_REG_ADDR_MSK	(0x1F)

#define	SMAP_R_EMAC3_TX_THRESHOLD	0x60
#define	  SMAP_E3_TX_THRESHLD_MSK	(0x1F)
#define	  SMAP_E3_TX_THRESHLD_BITSFT	(27)

#define	SMAP_R_EMAC3_RX_WATERMARK	0x64
#define	  SMAP_E3_RX_LO_WATER_MSK	(0x1FF)
#define	  SMAP_E3_RX_LO_WATER_BITSFT	(23)
#define	  SMAP_E3_RX_HI_WATER_MSK	(0x1FF)
#define	  SMAP_E3_RX_HI_WATER_BITSFT	(7)

#define	SMAP_R_EMAC3_TX_OCTETS		0x68
#define	SMAP_R_EMAC3_RX_OCTETS		0x6C

/* Buffer descriptors.  */

typedef struct _smap_bd {
	u16	ctrl_stat;
	u16	reserved;	/* must be zero */
	u16	length;		/* number of bytes in pkt */
	u16	pointer;
} smap_bd_t;

#define	SMAP_BD_REGBASE			0x2f00
#define	SMAP_BD_TX_BASE			(SMAP_BD_REGBASE + 0x0000)
#define SMAP_TX_BASE			0x1000
#define SMAP_TX_BUFSIZE			4096
#define	SMAP_BD_RX_BASE			(SMAP_BD_REGBASE + 0x0200)
#define	  SMAP_BD_SIZE		512
#define	  SMAP_BD_MAX_ENTRY	64

#define USE_SMAP_TX_BD	volatile smap_bd_t *tx_bd =	\
		(volatile smap_bd_t *)(SMAP_REGBASE + SMAP_BD_TX_BASE)
#define USE_SMAP_RX_BD	volatile smap_bd_t *rx_bd =	\
		(volatile smap_bd_t *)(SMAP_REGBASE + SMAP_BD_RX_BASE)

/* TX Control */
#define	SMAP_BD_TX_READY	(1<<15)	/* set:driver, clear:HW */
#define	SMAP_BD_TX_GENFCS	(1<<9)	/* generate FCS */
#define	SMAP_BD_TX_GENPAD	(1<<8)	/* generate padding */
#define	SMAP_BD_TX_INSSA	(1<<7)	/* insert source address */
#define	SMAP_BD_TX_RPLSA	(1<<6)	/* replace source address */
#define	SMAP_BD_TX_INSVLAN	(1<<5)	/* insert VLAN Tag */
#define	SMAP_BD_TX_RPLVLAN	(1<<4)	/* replace VLAN Tag */

/* TX Status */
#define	SMAP_BD_TX_READY	(1<<15) /* set:driver, clear:HW */
#define	SMAP_BD_TX_BADFCS	(1<<9)	/* bad FCS */
#define	SMAP_BD_TX_BADPKT	(1<<8)	/* bad previous pkt in dependent mode */
#define	SMAP_BD_TX_LOSSCR	(1<<7)	/* loss of carrior sense */
#define	SMAP_BD_TX_EDEFER	(1<<6)	/* excessive deferal */
#define	SMAP_BD_TX_ECOLL	(1<<5)	/* excessive collision */
#define	SMAP_BD_TX_LCOLL	(1<<4)	/* late collision */
#define	SMAP_BD_TX_MCOLL	(1<<3)	/* multiple collision */
#define	SMAP_BD_TX_SCOLL	(1<<2)	/* single collision */
#define	SMAP_BD_TX_UNDERRUN	(1<<1)	/* underrun */
#define	SMAP_BD_TX_SQE		(1<<0)	/* SQE */

#define SMAP_BD_TX_ERROR (SMAP_BD_TX_LOSSCR|SMAP_BD_TX_EDEFER|SMAP_BD_TX_ECOLL|	\
		SMAP_BD_TX_LCOLL|SMAP_BD_TX_UNDERRUN)

/* RX Control */
#define	SMAP_BD_RX_EMPTY	(1<<15)	/* set:driver, clear:HW */

/* RX Status */
#define	SMAP_BD_RX_EMPTY	(1<<15)	/* set:driver, clear:HW */
#define	SMAP_BD_RX_OVERRUN	(1<<9)	/* overrun */
#define	SMAP_BD_RX_PFRM		(1<<8)	/* pause frame */
#define	SMAP_BD_RX_BADFRM	(1<<7)	/* bad frame */
#define	SMAP_BD_RX_RUNTFRM	(1<<6)	/* runt frame */
#define	SMAP_BD_RX_SHORTEVNT	(1<<5)	/* short event */
#define	SMAP_BD_RX_ALIGNERR	(1<<4)	/* alignment error */
#define	SMAP_BD_RX_BADFCS	(1<<3)	/* bad FCS */
#define	SMAP_BD_RX_FRMTOOLONG	(1<<2)	/* frame too long */
#define	SMAP_BD_RX_OUTRANGE	(1<<1)	/* out of range error */
#define	SMAP_BD_RX_INRANGE	(1<<0)	/* in range error */

#define SMAP_BD_RX_ERROR (SMAP_BD_RX_OVERRUN|SMAP_BD_RX_RUNTFRM|SMAP_BD_RX_SHORTEVNT|	\
		SMAP_BD_RX_ALIGNERR|SMAP_BD_RX_BADFCS|SMAP_BD_RX_FRMTOOLONG|		\
		SMAP_BD_RX_OUTRANGE|SMAP_BD_RX_INRANGE)

/* PHY registers (National Semiconductor DP83846A).  */

#define	SMAP_NS_OUI			0x080017
#define	SMAP_DsPHYTER_ADDRESS		0x1

#define	SMAP_DsPHYTER_BMCR		0x00
#define	  SMAP_PHY_BMCR_RST	(1<<15)		/* ReSeT */
#define	  SMAP_PHY_BMCR_LPBK	(1<<14)		/* LooPBacK */
#define	  SMAP_PHY_BMCR_100M	(1<<13)		/* speed select, 1:100M, 0:10M */
#define	  SMAP_PHY_BMCR_10M	(0<<13)		/* speed select, 1:100M, 0:10M */
#define	  SMAP_PHY_BMCR_ANEN	(1<<12)		/* Auto-Negotiation ENable */
#define	  SMAP_PHY_BMCR_PWDN	(1<<11)		/* PoWer DowN */
#define	  SMAP_PHY_BMCR_ISOL	(1<<10)		/* ISOLate */
#define	  SMAP_PHY_BMCR_RSAN	(1<<9)		/* ReStart Auto-Negotiation */
#define	  SMAP_PHY_BMCR_DUPM	(1<<8)		/* DUPlex Mode, 1:FDX, 0:HDX */
#define	  SMAP_PHY_BMCR_COLT	(1<<7)		/* COLlision Test */

#define	SMAP_DsPHYTER_BMSR		0x01
#define	  SMAP_PHY_BMSR_ANCP	(1<<5)		/* Auto-Negotiation ComPlete */
#define	  SMAP_PHY_BMSR_LINK	(1<<2)		/* LINK status */

#define	SMAP_DsPHYTER_PHYIDR1		0x02
#define	  SMAP_PHY_IDR1_VAL	(((SMAP_NS_OUI<<2)>>8)&0xffff)

#define	SMAP_DsPHYTER_PHYIDR2		0x03
#define	  SMAP_PHY_IDR2_VMDL	0x2		/* Vendor MoDeL number */
#define	  SMAP_PHY_IDR2_VAL	\
		(((SMAP_NS_OUI<<10)&0xFC00)|((SMAP_PHY_IDR2_VMDL<<4)&0x3F0))
#define	  SMAP_PHY_IDR2_MSK	0xFFF0
#define	  SMAP_PHY_IDR2_REV_MSK	0x000F

#define	SMAP_DsPHYTER_ANAR		0x04
#define	SMAP_DsPHYTER_ANLPAR		0x05
#define	SMAP_DsPHYTER_ANLPARNP		0x05
#define	SMAP_DsPHYTER_ANER		0x06
#define	SMAP_DsPHYTER_ANNPTR		0x07

/* Extended registers.  */
#define	SMAP_DsPHYTER_PHYSTS		0x10
#define	  SMAP_PHY_STS_REL	(1<<13)		/* Receive Error Latch */
#define	  SMAP_PHY_STS_POST	(1<<12)		/* POlarity STatus */
#define	  SMAP_PHY_STS_FCSL	(1<<11)		/* False Carrier Sense Latch */
#define	  SMAP_PHY_STS_SD	(1<<10)		/* 100BT unconditional Signal Detect */
#define	  SMAP_PHY_STS_DSL	(1<<9)		/* 100BT DeScrambler Lock */
#define	  SMAP_PHY_STS_PRCV	(1<<8)		/* Page ReCeiVed */
#define	  SMAP_PHY_STS_RFLT	(1<<6)		/* Remote FauLT */
#define	  SMAP_PHY_STS_JBDT	(1<<5)		/* JaBber DetecT */
#define	  SMAP_PHY_STS_ANCP	(1<<4)		/* Auto-Negotiation ComPlete */
#define	  SMAP_PHY_STS_LPBK	(1<<3)		/* LooPBacK status */
#define	  SMAP_PHY_STS_DUPS	(1<<2)		/* DUPlex Status,1:FDX,0:HDX */
#define	  SMAP_PHY_STS_FDX	(1<<2)		/* Full Duplex */
#define	  SMAP_PHY_STS_HDX	(0<<2)		/* Half Duplex */
#define	  SMAP_PHY_STS_SPDS	(1<<1)		/* SPeeD Status */
#define	  SMAP_PHY_STS_10M	(1<<1)		/* 10Mbps */
#define	  SMAP_PHY_STS_100M	(0<<1)		/* 100Mbps */
#define	  SMAP_PHY_STS_LINK	(1<<0)		/* LINK status */
#define	SMAP_DsPHYTER_FCSCR		0x14
#define	SMAP_DsPHYTER_RECR		0x15
#define	SMAP_DsPHYTER_PCSR		0x16
#define	SMAP_DsPHYTER_PHYCTRL		0x19
#define	SMAP_DsPHYTER_10BTSCR		0x1A
#define	SMAP_DsPHYTER_CDCTRL		0x1B

#endif /* SMAPREGS_H */
