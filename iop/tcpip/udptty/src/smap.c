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
# SMAP driver for UDPTTY.
*/

#include "udptty.h"
#include "sysclib.h"

typedef struct {
	u16	txfree;		/* Number of bytes free in the TX FIFO.  */
	u16	txbwp;
	int	txbdsi;		/* Saved index into TX BD.  */
	int	txbdi;		/* Index into current TX BD.  */
	int	txbd_used;	/* Keeps track of how many TX BD's have been used.  */

	int	has_link;
} smap_tx_state_t;

static smap_tx_state_t tx_state;

/* SMAP driver.  */

static int smap_phy_read(int reg, u16 *data)
{
	USE_SMAP_EMAC3_REGS;
	u32 i, val;

	val = SMAP_E3_PHY_READ|(SMAP_DsPHYTER_ADDRESS << SMAP_E3_PHY_ADDR_BITSFT)|
		(reg & SMAP_E3_PHY_REG_ADDR_MSK);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_STA_CTRL, val);

	/* Wait for the read operation to complete.  */
	for (i = 0; i < 100; i++) {
		if (SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL) & SMAP_E3_PHY_OP_COMP)
			break;
		DelayThread(1000);
	}
	if (i == 100 || !data)
		return 1;

	*data = SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL) >> SMAP_E3_PHY_DATA_BITSFT;
	return 0;
}

static int smap_phy_write(int reg, u16 data)
{
	USE_SMAP_EMAC3_REGS;
	u32 i, val;

	val = ((data & SMAP_E3_PHY_DATA_MSK) << SMAP_E3_PHY_DATA_BITSFT)|
		SMAP_E3_PHY_WRITE|(SMAP_DsPHYTER_ADDRESS << SMAP_E3_PHY_ADDR_BITSFT)|
		(reg & SMAP_E3_PHY_REG_ADDR_MSK);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_STA_CTRL, val);

	/* Wait for the write operation to complete.  */
	for (i = 0; i < 100; i++) {
		if (SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL) & SMAP_E3_PHY_OP_COMP)
			break;
		DelayThread(1000);
	}
	return (i == 100);
}

static int smap_phy_init()
{
	USE_SMAP_EMAC3_REGS;
	u32 val;
	int i, j;
	u16 phydata, idr1, idr2;

	/* Reset the PHY.  */
	smap_phy_write(SMAP_DsPHYTER_BMCR, SMAP_PHY_BMCR_RST);
	/* Wait for it to come out of reset.  */
	for (i = 9; i; i--) {
		if (smap_phy_read(SMAP_DsPHYTER_BMCR, &phydata) != 0)
			return 1;
		if (!(phydata & SMAP_PHY_BMCR_RST))
			break;
		DelayThread(1000);
	}
	if (!i)
		return 2;

	val = SMAP_PHY_BMCR_ANEN|SMAP_PHY_BMCR_RSAN;
	smap_phy_write(SMAP_DsPHYTER_BMCR, val);

	/* Attempt to complete autonegotiation up to 3 times.  */
	for (i = 0; i < 3; i++) {
		DelayThread(3000000);
		if (smap_phy_read(SMAP_DsPHYTER_BMSR, &phydata) != 0)
			return 3;

		if (phydata & SMAP_PHY_BMSR_ANCP) {
			for (j = 0; j < 20; j++) {
				DelayThread(200000);
				if (smap_phy_read(SMAP_DsPHYTER_BMSR, &phydata) != 0)
					return 4;

				if (phydata & SMAP_PHY_BMSR_LINK) {
					/*state->has_link = 1;*/
					goto auto_done;
				}
			}
		}
		/* If autonegotiation failed, we got here, so restart it.  */
		smap_phy_write(SMAP_DsPHYTER_BMCR, val);
	}
	/* Autonegotiation failed.  */
	return 5;

auto_done:
	/* Now, read our speed and duplex mode from the PHY.  */
	if (smap_phy_read(SMAP_DsPHYTER_PHYSTS, &phydata) != 0)
		return 6;

	val = SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE1);
	if (phydata & SMAP_PHY_STS_FDX)
		val |= SMAP_E3_FDX_ENABLE|SMAP_E3_FLOWCTRL_ENABLE|SMAP_E3_ALLOW_PF;
	else
		val &= ~(SMAP_E3_FDX_ENABLE|SMAP_E3_FLOWCTRL_ENABLE|SMAP_E3_ALLOW_PF);

	val &= ~SMAP_E3_MEDIA_MSK;
	if (phydata & SMAP_PHY_STS_10M) {
		val &= ~SMAP_E3_IGNORE_SQE;
		val |= SMAP_E3_MEDIA_10M;
	} else {
		val |= SMAP_E3_MEDIA_100M;
	}

	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE1, val);

	/* DSP setup.  */
	if (smap_phy_read(SMAP_DsPHYTER_PHYIDR1, &idr1) != 0)
		return 7;
	if (smap_phy_read(SMAP_DsPHYTER_PHYIDR2, &idr2) != 0)
		return 8;

	if (idr1 == SMAP_PHY_IDR1_VAL &&
			((idr2 & SMAP_PHY_IDR2_MSK) == SMAP_PHY_IDR2_VAL)) {
		if (phydata & SMAP_PHY_STS_10M)
			smap_phy_write(0x1a, 0x104);

		smap_phy_write(0x13, 0x0001);
		smap_phy_write(0x19, 0x1898);
		smap_phy_write(0x1f, 0x0000);
		smap_phy_write(0x1d, 0x5040);
		smap_phy_write(0x1e, 0x008c);
		smap_phy_write(0x13, 0x0000);
	}

	return 0;
}

int smap_init()
{
	USE_SPD_REGS;
	USE_SMAP_REGS;
	USE_SMAP_EMAC3_REGS;
	USE_SMAP_TX_BD; USE_SMAP_RX_BD;
	u8 hwaddr[6];
	u32 val;
	int i;

	if (!(SPD_REG16(SPD_R_REV_3) & 0x01) || SPD_REG16(SPD_R_REV_1) <= 16)
		return 1;

	dev9IntrDisable(SMAP_INTR_BITMSK);

	/* Disable TX/RX.  */
	val = SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE0);
	val &= ~( SMAP_E3_TXMAC_ENABLE|SMAP_E3_RXMAC_ENABLE);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, val);

	/* Disable interrupts.  */
	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTR_ENABLE, 0);

	/* Reset the transmit FIFO.  */
	SMAP_REG8(SMAP_R_TXFIFO_CTRL) = SMAP_TXFIFO_RESET;
	for (i = 9; i; i--) {
		if (!(SMAP_REG8(SMAP_R_TXFIFO_CTRL) & SMAP_TXFIFO_RESET))
			break;
		DelayThread(1000);
	}
	if (!i)
		return 2;

	/* Reset the receive FIFO.  */
	SMAP_REG8(SMAP_R_RXFIFO_CTRL) = SMAP_RXFIFO_RESET;
	for (i = 9; i; i--) {
		if (!(SMAP_REG8(SMAP_R_RXFIFO_CTRL) & SMAP_RXFIFO_RESET))
			break;
		DelayThread(1000);
	}
	if (!i)
		return 3;

	/* Perform soft reset of EMAC3.  */
	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, SMAP_E3_SOFT_RESET);
	for (i = 9; i; i--) {
		if (!(SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE0) & SMAP_E3_SOFT_RESET))
			break;
		DelayThread(1000);
	}
	if (!i)
		return 4;

	SMAP_REG8(SMAP_R_BD_MODE) = 0;

	/* Initialize all RX and TX buffer descriptors.  */
	for (i = 0; i < SMAP_BD_MAX_ENTRY; i++, tx_bd++) {
			tx_bd->ctrl_stat = 0;
			tx_bd->reserved  = 0;
			tx_bd->length    = 0;
			tx_bd->pointer   = 0;
	}
	for (i = 0; i < SMAP_BD_MAX_ENTRY; i++, rx_bd++) { 
			rx_bd->ctrl_stat = SMAP_BD_RX_EMPTY;
			rx_bd->reserved  = 0;
			rx_bd->length    = 0;
			rx_bd->pointer   = 0;
	}

	SMAP_REG16(SMAP_R_INTR_CLR) = SMAP_INTR_BITMSK;

	val = SMAP_E3_FDX_ENABLE|SMAP_E3_IGNORE_SQE|SMAP_E3_MEDIA_100M|
		SMAP_E3_RXFIFO_2K|SMAP_E3_TXFIFO_1K|SMAP_E3_TXREQ0_MULTI;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE1, val);

	val = (7 << SMAP_E3_TX_LOW_REQ_BITSFT) | (15 << SMAP_E3_TX_URG_REQ_BITSFT);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_TxMODE1, val);

	val = SMAP_E3_RX_STRIP_PAD|SMAP_E3_RX_STRIP_FCS|SMAP_E3_RX_INDIVID_ADDR|
		SMAP_E3_RX_BCAST|SMAP_E3_RX_MCAST;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_RxMODE, val);

	memcpy(hwaddr, udptty_param.eth_addr_src, 6);
	val = (u16)((hwaddr[0] >> 8)|(hwaddr[0] << 8));
	SMAP_EMAC3_SET(SMAP_R_EMAC3_ADDR_HI, val);
	val = (((hwaddr[1] >> 8)|(hwaddr[1] << 8)) << 16)|(u16)((hwaddr[2] >> 8)|
			(hwaddr[2] << 8));
	SMAP_EMAC3_SET(SMAP_R_EMAC3_ADDR_LO, val);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_PAUSE_TIMER, 0xffff);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH1, 0);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH2, 0);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH3, 0);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_GROUP_HASH4, 0);

	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTER_FRAME_GAP, 4);

	val = 12 << SMAP_E3_TX_THRESHLD_BITSFT;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_TX_THRESHOLD, val);

	val = (16 << SMAP_E3_RX_LO_WATER_BITSFT) | (128 << SMAP_E3_RX_HI_WATER_BITSFT);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_RX_WATERMARK, val);

	/* Initialize the PHY.  */
	if ((i = smap_phy_init()) != 0)
		return -i;

	tx_state.txfree = SMAP_TX_BUFSIZE;
	tx_state.txbwp = SMAP_TX_BASE;
	tx_state.txbdsi = tx_state.txbdi = tx_state.txbd_used = 0;

	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, SMAP_E3_TXMAC_ENABLE);
	DelayThread(10000);

	return 0;
}


/* Quick and dirty transmit.  */
static int smap_tx(void *buf, size_t size)
{
	USE_SMAP_REGS;
	USE_SMAP_TX_BD;
	u32 *payload;
	int txbdi, i, transmitted = 0;
	u16 asize;

	if (tx_state.txbd_used >= SMAP_BD_MAX_ENTRY)
		goto out;

	asize = (size + 3) & 0xfffc;
	if (asize > tx_state.txfree)
		goto out;

	SMAP_REG16(SMAP_R_TXFIFO_WR_PTR) = tx_state.txbwp & 0xffc;

	for (i = 0, payload = (u32 *)buf; i < asize; i += 4)
		SMAP_REG32(SMAP_R_TXFIFO_DATA) = *payload++;

	txbdi = tx_state.txbdi & 0x3f;
	tx_bd[txbdi].length = size;
	tx_bd[txbdi].pointer = tx_state.txbwp;
	SMAP_REG8(SMAP_R_TXFIFO_FRAME_INC) = 1;
	tx_bd[txbdi].ctrl_stat = SMAP_BD_TX_READY| \
		SMAP_BD_TX_GENFCS|SMAP_BD_TX_GENPAD;

	tx_state.txbwp = SMAP_TX_BASE + ((tx_state.txbwp + asize) % SMAP_TX_BASE);
	tx_state.txbdi++;
	tx_state.txbd_used++;
	tx_state.txfree -= asize;

	transmitted = asize;

out:
	return transmitted;
}

int smap_txbd_check()
{
	USE_SMAP_TX_BD;
	u16 len;
	int txbdsi, count = 0;

	while (tx_state.txbd_used > 0) {
		txbdsi = tx_state.txbdsi & 0x3f;
		if (tx_bd[txbdsi].ctrl_stat & SMAP_BD_TX_READY)
			return count;

		len = tx_bd[txbdsi].length;

		count++;
		tx_state.txfree += (len + 3) & 0xfffc;
		tx_state.txbdsi++;
		--tx_state.txbd_used;
	}

	return count;
}

int smap_transmit(void *buf, size_t size)
{
	USE_SMAP_EMAC3_REGS;
	int res;

	res = smap_tx(buf, size);
	smap_txbd_check();

	if (tx_state.txbd_used > 0)
		SMAP_EMAC3_SET(SMAP_R_EMAC3_TxMODE0, SMAP_E3_TX_GNP_0);

	return res;
}
