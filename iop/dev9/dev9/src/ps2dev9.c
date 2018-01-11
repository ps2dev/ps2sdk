/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * DEV9 Device Driver.
 */

#include <types.h>
#include <defs.h>
#include <loadcore.h>
#include <intrman.h>
#include <iomanX.h>
#include <dmacman.h>
#include <thbase.h>
#include <thsemap.h>
#include <stdio.h>
#include <sysclib.h>
#include <dev9.h>

#include <aifregs.h>
#include <dev9regs.h>
#include <speedregs.h>
#include <smapregs.h>

#define MODNAME "dev9"
#define DRIVERNAME "dev9"
IRX_ID(MODNAME, 2, 8);

#define M_PRINTF(format, args...)	\
	printf(MODNAME ": " format, ## args)

#define VERSION "1.0"
#define BANNER "\nDEV9 device driver v%s - Copyright (c) 2003 Marcus R. Brown\n\n"

/* SSBUS registers.  */
#define SSBUS_R_1418		0xbf801418
#define SSBUS_R_141c		0xbf80141c
#define SSBUS_R_1420		0xbf801420

static int semaAttrGlobal;	/* Semaphore attribute value to use. */
static const char *mod_name;	/* Module name. */
static int dev9type = -1;	/* 0 for PCMCIA, 1 for expansion bay */
static int using_aif = 0;	/* 1 if using AIF on a T10K */

static int dma_lock_sem;	/* used to arbitrate DMA */

enum PC_CARD_TYPE{
	PC_CARD_TYPE_NONE = 0,
	PC_CARD_TYPE_PCMCIA,
	PC_CARD_TYPE_CARDBUS,
};

enum PC_CARD_VOLTAGE{
	PC_CARD_VOLTAGE_INVALID = 0,
	PC_CARD_VOLTAGE_3V,
	PC_CARD_VOLTAGE_5V,
	PC_CARD_VOLTAGE_04h
};

static int pcic_cardtype;	/* Translated value of bits 0-1 of 0xbf801462 */
static int pcic_voltage;	/* Translated value of bits 2-3 of 0xbf801462 */

static s16 eeprom_data[5];	/* 2-byte EEPROM status (0/-1 = invalid, 1 = valid),
				   6-byte MAC address,
				   2-byte MAC address checksum.  */

typedef int (*dev9IntrDispatchCb_t)(int flag);
static dev9IntrDispatchCb_t p_dev9_intr_cb = NULL;
static void dev9RegisterIntrDispatchCb(dev9IntrDispatchCb_t callback);

/* Each driver can register callbacks that correspond to each bit of the
   SMAP interrupt status register (0xbx000028).  */
static dev9_intr_cb_t dev9_intr_cbs[16];

static dev9_shutdown_cb_t dev9_shutdown_cbs[16];

static dev9_dma_cb_t dev9_predma_cbs[4], dev9_postdma_cbs[4];

static int dev9_intr_dispatch(int flag);

static void dev9_set_stat(int stat);
static int dev9_ssbus_mode(int mode);
static int dev9_device_probe(void);
static int dev9_device_reset(void);
static int dev9_card_find_manfid(u32 manfid);

static int read_eeprom_data(void);
static int dev9_init(int sema_attr);
static int speed_device_init(void);

static void pcmcia_set_stat(int stat);
static int pcic_ssbus_mode(int mode);
static int pcmcia_device_probe(void);
static int pcmcia_device_reset(void);
static int card_find_manfid(u32 manfid);
static int pcmcia_init(int sema_attr);

static void expbay_set_stat(int stat);
static int expbay_device_probe(void);
static int expbay_device_reset(void);
static int expbay_init(int sema_attr);

extern struct irx_export_table _exp_dev9;

static int dev9x_dummy(void)
{
	return 0;
}

static int dev9x_devctl(iop_file_t *f, const char *name, int cmd, void *args, unsigned int arglen, void *buf, unsigned int buflen)
{
	switch(cmd)
	{
		case DDIOC_MODEL:
	        	return dev9type;
		case DDIOC_OFF:
			dev9Shutdown();
			return 0;
		default:
			return 0;
	}
}

static iop_device_ops_t dev9x_ops =
{
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	(void *)&dev9x_dummy,
	&dev9x_devctl
};

static iop_device_t dev9x_device =
{
	"dev9x",
	IOP_DT_FS | IOP_DT_FSEXT,
	1,
	"DEV9",
	&dev9x_ops
};

static int print_help(void)
{	//The original made a printf() call for each line.
	printf(	"Usage:\n"
		"  %s [-sa] <attribute>]\n"
		"      -sa  You can specify attibute of sempahore for queuing thread.\n"
		"           List of possible <attribute>:\n"
		"             SA_THPRI(default), SA_THFIFO\n", mod_name);

	return MODULE_NO_RESIDENT_END;
}

int _start(int argc, char **argv)
{
	USE_DEV9_REGS;
	const char *pModName;
	int idx, res;
	u16 dev9hw;

	semaAttrGlobal = SA_THPRI;

	printf(BANNER, VERSION);

	mod_name = (pModName = strrchr(argv[0], '/')) != NULL ? pModName + 1 : argv[0];

	for(--argc,++argv; argc > 0; argc--,argv++) {
		if((*argv)[0] != '-')
			break;

		if(strcmp("-sa", *argv) == 0) {
			--argc;
			++argv;
			if (argc > 0) {
				if(strcmp("SA_THFIFO", *argv) == 0) {
					semaAttrGlobal = SA_THFIFO;
				} else if(strcmp("SA_THPRI", *argv) == 0) {
					semaAttrGlobal = SA_THPRI;
				} else {
					return print_help();
				}
			} else {
				return print_help();
			}
		} else {
			return print_help();
		}
	}

	for (idx = 0; idx < 16; idx++)
		dev9_shutdown_cbs[idx] = NULL;

	dev9hw = DEV9_REG(DEV9_R_REV) & 0xf0;
	if (dev9hw == 0x20) {		/* CXD9566 (PCMCIA) */
		dev9type = DEV9_TYPE_PCMCIA;
		M_PRINTF("CXD9566 detected.\n");
		res = pcmcia_init(semaAttrGlobal);
	} else if (dev9hw == 0x30) {	/* CXD9611 (Expansion Bay) */
		dev9type = DEV9_TYPE_EXPBAY;
		M_PRINTF("CXD9611 detected.\n");
		res = expbay_init(semaAttrGlobal);
	} else {
		M_PRINTF("unknown dev9 hardware.\n");
		res = MODULE_NO_RESIDENT_END;
	}

	if (res)
		return res;

	DelDrv("dev9x");
	if (AddDrv(&dev9x_device) != 0) {
		return MODULE_NO_RESIDENT_END;
	}

	return MODULE_RESIDENT_END;
}

int _exit(void) { return 0; }

/* Export 4 */
void dev9RegisterIntrCb(int intr, dev9_intr_cb_t cb)
{
	dev9_intr_cbs[intr] = cb;
}

/* Export 12 */
void dev9RegisterPreDmaCb(int ctrl, dev9_dma_cb_t cb){
	dev9_predma_cbs[ctrl] = cb;
}

/* Export 13 */
void dev9RegisterPostDmaCb(int ctrl, dev9_dma_cb_t cb){
	dev9_postdma_cbs[ctrl] = cb;
}

// flag is 1 if a card (pcmcia) was removed or added
static int dev9_intr_dispatch(int flag)
{
	USE_SPD_REGS;
	int i, bit;

	if (flag) {
		for (i = 0; i < 16; i++)
			if (dev9_intr_cbs[i] != NULL)
				dev9_intr_cbs[i](flag);
	}

	while (SPD_REG16(SPD_R_INTR_STAT) & SPD_REG16(SPD_R_INTR_MASK)) {
		for (i = 0; i < 16; i++) {
			if (dev9_intr_cbs[i] != NULL) {
				bit = (SPD_REG16(SPD_R_INTR_STAT) &
					SPD_REG16(SPD_R_INTR_MASK)) >> i;
				if (bit & 0x01)
					dev9_intr_cbs[i](flag);
			}
		}
	}

	return 0;
}

static void dev9_set_stat(int stat)
{
	switch(dev9type){
		case DEV9_TYPE_PCMCIA:
			pcmcia_set_stat(stat);
			break;
		case DEV9_TYPE_EXPBAY:
			expbay_set_stat(stat);
			break;
	}
}

static int dev9_ssbus_mode(int mode)
{
	switch(dev9type){
		case DEV9_TYPE_PCMCIA:
			return pcic_ssbus_mode(mode);
		case DEV9_TYPE_EXPBAY:
			return 0;
	}

	return -1;
}

static int dev9_device_probe(void)
{
	switch(dev9type){
		case DEV9_TYPE_PCMCIA:
			return pcmcia_device_probe();
		case DEV9_TYPE_EXPBAY:
			return expbay_device_probe();
	}

	return -1;
}

static int dev9_device_reset(void)
{
	switch(dev9type){
		case DEV9_TYPE_PCMCIA:
			return pcmcia_device_reset();
		case DEV9_TYPE_EXPBAY:
			return expbay_device_reset();
	}

	return -1;
}

/* Export 6 */
void dev9Shutdown(void)
{
	int idx;
	USE_DEV9_REGS;

	for (idx = 0; idx < 16; idx++)
		if (dev9_shutdown_cbs[idx])
			dev9_shutdown_cbs[idx]();

	if (dev9type == DEV9_TYPE_PCMCIA) {	/* PCMCIA */
		DEV9_REG(DEV9_R_POWER) = 0;
		DEV9_REG(DEV9_R_1474) = 0;
	} else if (dev9type == DEV9_TYPE_EXPBAY) {
		DEV9_REG(DEV9_R_1466) = 1;
		DEV9_REG(DEV9_R_1464) = 0;
		DEV9_REG(DEV9_R_1460) = DEV9_REG(DEV9_R_1464);
		DEV9_REG(DEV9_R_POWER) = DEV9_REG(DEV9_R_POWER) & ~4;
		DEV9_REG(DEV9_R_POWER) = DEV9_REG(DEV9_R_POWER) & ~1;
	}
	DelayThread(1000000);
}

static int dev9_card_find_manfid(u32 manfid)
{
	switch(dev9type){
		case DEV9_TYPE_PCMCIA:
			return card_find_manfid(manfid);
		case DEV9_TYPE_EXPBAY:
			return 0;
	}

	return -1;
}

/* Export 7 */
void dev9IntrEnable(int mask)
{
	USE_SPD_REGS;
	int flags;

	CpuSuspendIntr(&flags);
	SPD_REG16(SPD_R_INTR_MASK) = SPD_REG16(SPD_R_INTR_MASK) | mask;
	CpuResumeIntr(flags);
}

/* Export 8 */
void dev9IntrDisable(int mask)
{
	USE_SPD_REGS;
	int flags;

	CpuSuspendIntr(&flags);
	SPD_REG16(SPD_R_INTR_MASK) = SPD_REG16(SPD_R_INTR_MASK) & ~mask;
	CpuResumeIntr(flags);
}

/* Export 5 */
int dev9DmaTransfer(int device, void *buf, int bcr, int dir)
{
	USE_SPD_REGS;
	volatile iop_dmac_chan_t *dev9_chan = (iop_dmac_chan_t *)DEV9_DMAC_BASE;
	int res, dmactrl, OldState;

	if(device >= 2)
	{
		if (dev9_predma_cbs[device] == NULL)	return -1;
		if (dev9_postdma_cbs[device] == NULL)	return -1;
	}

	if ((res = WaitSema(dma_lock_sem)) < 0)
		return res;

	switch (device)
	{
		case 0:
			dmactrl = 0;
			break;
		case 1:
			dmactrl = 1;
			break;
#ifdef DEV9_PSX_SUPPORT
		//Used for the PSX
		case 2:
			dmactrl = 0x10;
			break;
		case 3:
			dmactrl = 0x20;
			break;
#endif
		default:
			dmactrl = 0;
	}

	SPD_REG16(SPD_R_DMA_CTRL) = (SPD_REG16(SPD_R_REV_1) < 17) ? (dmactrl & 0x03) | 0x04 : dmactrl | 0x06;

	if (dev9_predma_cbs[device])
		dev9_predma_cbs[device](bcr, dir);

	dev9_chan->madr = (u32)buf;

	/* Older versions of DEV9 do not suspend interrupts. Not sure why this must be done though. */
	CpuSuspendIntr(&OldState);
	dev9_chan->bcr  = bcr;
	dev9_chan->chcr = DMAC_CHCR_30|DMAC_CHCR_TR|DMAC_CHCR_CO|(dir & DMAC_CHCR_DR);
	CpuResumeIntr(OldState);

	/* Wait for DMA to complete. Do not use a semaphore as thread switching hurts throughput greatly.  */
	while(dev9_chan->chcr & DMAC_CHCR_TR) {}

	if (dev9_postdma_cbs[device])
		dev9_postdma_cbs[device](bcr, dir);

	SignalSema(dma_lock_sem);
	return 0;
}

static int read_eeprom_data(void)
{
	USE_SPD_REGS;
	int i, j, res = -2;
	u8 val;

	if (eeprom_data[0] < 0)
		goto out;

	SPD_REG8(SPD_R_PIO_DIR)  = 0xe1;
	DelayThread(1);
	SPD_REG8(SPD_R_PIO_DATA) = 0x80;
	DelayThread(1);

	for (i = 0; i < 2; i++) {
		SPD_REG8(SPD_R_PIO_DATA) = 0xa0;
		DelayThread(1);
		SPD_REG8(SPD_R_PIO_DATA) = 0xe0;
		DelayThread(1);
	}
	for (i = 0; i < 7; i++) {
		SPD_REG8(SPD_R_PIO_DATA) = 0x80;
		DelayThread(1);
		SPD_REG8(SPD_R_PIO_DATA) = 0xc0;
		DelayThread(1);
	}
	SPD_REG8(SPD_R_PIO_DATA) = 0xc0;
	DelayThread(1);

	val = SPD_REG8(SPD_R_PIO_DATA);
	DelayThread(1);
	if (val & 0x10) {	/* Error.  */
		SPD_REG8(SPD_R_PIO_DATA) = 0;
		DelayThread(1);
		res = -1;
		eeprom_data[0] = 0;
		goto out;
	}

	SPD_REG8(SPD_R_PIO_DATA) = 0x80;
	DelayThread(1);

	/* Read the MAC address and checksum from the EEPROM.  */
	for (i = 0; i < 4; i++) {
		eeprom_data[i+1] = 0;

		for (j = 15; j >= 0; j--) {
			SPD_REG8(SPD_R_PIO_DATA) = 0xc0;
			DelayThread(1);
			val = SPD_REG8(SPD_R_PIO_DATA);
			if (val & 0x10)
				eeprom_data[i+1] |= (1<<j);
			SPD_REG8(SPD_R_PIO_DATA) = 0x80;
			DelayThread(1);
		}
	}

	SPD_REG8(SPD_R_PIO_DATA) = 0;
	DelayThread(1);
	eeprom_data[0] = 1;	/* The EEPROM data is valid.  */
	res = 0;

out:
	SPD_REG8(SPD_R_PIO_DIR) = 1;
	return res;
}

/* Export 9 */
int dev9GetEEPROM(u16 *buf)
{
	int i;

	if (!eeprom_data[0])
		return -1;
	if (eeprom_data[0] < 0)
		return -2;

	/* We only return the MAC address and checksum.  */
	for (i = 0; i < 4; i++)
		buf[i] = eeprom_data[i+1];

	return 0;
}

/* Export 10 */
void dev9LEDCtl(int ctl)
{
	USE_SPD_REGS;
	SPD_REG8(SPD_R_PIO_DATA) = (ctl == 0);
}

static void dev9RegisterIntrDispatchCb(dev9IntrDispatchCb_t callback)
{
	p_dev9_intr_cb = callback;
}

/* Export 11 */
int dev9RegisterShutdownCb(int idx, dev9_shutdown_cb_t cb)
{
	if (idx < 16)
	{
		dev9_shutdown_cbs[idx] = cb;
		return 0;
	}
	return -1;
}

static int dev9_init(int sema_attr)
{
	iop_sema_t sema;
	int i, flags;

	sema.attr = sema_attr;
	sema.initial = 1;
	sema.max = 1;
	if ((dma_lock_sem = CreateSema(&sema)) < 0)
		return -1;

	CpuSuspendIntr(&flags);
	/* Enable the DEV9 DMAC channel.  */
	dmac_set_dpcr2(dmac_get_dpcr2() | 0x80);
	CpuResumeIntr(flags);

	/* Not quite sure what this enables yet.  */
	dev9_set_stat(0x103);

	/* Disable all device interrupts.  */
	dev9IntrDisable(0xffff);

	/* Register interrupt dispatch callback handler. */
	dev9RegisterIntrDispatchCb(&dev9_intr_dispatch);

	/* Reset the SMAP interrupt callback table. */
	for (i = 0; i < 16; i++)
		dev9_intr_cbs[i] = NULL;

	for (i = 0; i < 4; i++){
		dev9_predma_cbs[i] = NULL;
		dev9_postdma_cbs[i] = NULL;
	}

	/* Read in the MAC address.  */
	read_eeprom_data();
	/* Turn the LED off.  */
	dev9LEDCtl(0);
	return 0;
}

#ifndef DEV9_SKIP_SMAP_INIT
static int dev9_smap_read_phy(volatile u8 *emac3_regbase, unsigned int address, unsigned int *data){
	unsigned int i, PHYRegisterValue;
	int result;

	PHYRegisterValue=(address&SMAP_E3_PHY_REG_ADDR_MSK)|SMAP_E3_PHY_READ|((SMAP_DsPHYTER_ADDRESS&SMAP_E3_PHY_ADDR_MSK)<<SMAP_E3_PHY_ADDR_BITSFT);

	i=0;
	result=0;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_STA_CTRL, PHYRegisterValue);

	do{
		if(SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL)&SMAP_E3_PHY_OP_COMP){
			if(SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL)&SMAP_E3_PHY_OP_COMP){
				if((result=SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL))&SMAP_E3_PHY_OP_COMP){
					result>>=SMAP_E3_PHY_DATA_BITSFT;
					break;
				}
			}
		}

		DelayThread(1000);
		i++;
	}while(i<100);

	if(i>=100){
		return 1;
	}else{
		*data = result;
		return 0;
	}
}

static int dev9_smap_write_phy(volatile u8 *emac3_regbase, unsigned char address, unsigned short int value){
	unsigned int i, PHYRegisterValue;

	PHYRegisterValue=(address&SMAP_E3_PHY_REG_ADDR_MSK)|SMAP_E3_PHY_WRITE|((SMAP_DsPHYTER_ADDRESS&SMAP_E3_PHY_ADDR_MSK)<<SMAP_E3_PHY_ADDR_BITSFT);
	PHYRegisterValue|=((unsigned int)value)<<SMAP_E3_PHY_DATA_BITSFT;

	i=0;
	SMAP_EMAC3_SET(SMAP_R_EMAC3_STA_CTRL, PHYRegisterValue);

	for(; !(SMAP_EMAC3_GET(SMAP_R_EMAC3_STA_CTRL)&SMAP_E3_PHY_OP_COMP); i++){
		DelayThread(1000);
		if(i>=100) break;
	}

	return((i>=100)?1:0);
}

static int dev9_smap_init(void)
{
	unsigned int value;
	USE_SPD_REGS;
	USE_SMAP_REGS;
	USE_SMAP_EMAC3_REGS;
	USE_SMAP_TX_BD;
	USE_SMAP_RX_BD;
	int i;

	//Do not perform SMAP initialization if the SPEED device does not have such an interface
	if (!(SPD_REG16(SPD_R_REV_3)&SPD_CAPS_SMAP)
#ifdef DEV9_GAMESTAR_WORKAROUND
	/*	If this adaptor is a compatible adaptor, do not initialize SMAP.
		Official adaptors appear to have a 0x0001 set for this register, but not compatibles.
		While official I/O to this register are 8-bit, some compatibles have a 0x01 for the lower 8-bits,
		but the upper 8-bits contain some random value. Hence perform a 16-bit read instead. */
	|| (SPD_REG16(0x20) != 1)
#endif
        ) return 0;

	SMAP_REG8(SMAP_R_TXFIFO_CTRL) = SMAP_TXFIFO_RESET;
	for(i = 9; SMAP_REG8(SMAP_R_TXFIFO_CTRL)&SMAP_TXFIFO_RESET; i--)
	{
		if (i <= 0) return 1;
		DelayThread(1000);
	}

	SMAP_REG8(SMAP_R_RXFIFO_CTRL) = SMAP_RXFIFO_RESET;
	for(i = 9; SMAP_REG8(SMAP_R_RXFIFO_CTRL)&SMAP_RXFIFO_RESET; i--)
	{
		if (i <= 0) return 1;
		DelayThread(1000);
	}

	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, SMAP_E3_SOFT_RESET);
	for (i = 9; SMAP_EMAC3_GET(SMAP_R_EMAC3_MODE0)&SMAP_E3_SOFT_RESET; i--)
	{
		if (i <= 0) return 3;
		DelayThread(1000);
	}

	//Unlike the SMAP driver, this reset operation is done in big-endian.
	if (SPD_REG16(SPD_R_REV_1) >= 0x11) SMAP_REG8(SMAP_R_BD_MODE) = SMAP_BD_SWAP;

	for(i = 0; i < SMAP_BD_MAX_ENTRY; i++)
	{
		tx_bd[i].ctrl_stat = 0;
		tx_bd[i].reserved = 0;
		tx_bd[i].length = 0;
		tx_bd[i].pointer = 0;
	}

	for(i = 0; i < SMAP_BD_MAX_ENTRY; i++)
	{
		rx_bd[i].ctrl_stat = 0x80;	//SMAP_BD_RX_EMPTY
		rx_bd[i].reserved = 0;
		rx_bd[i].length = 0;
		rx_bd[i].pointer = 0;
	}

	SMAP_REG16(SMAP_R_INTR_CLR) = SMAP_INTR_BITMSK;
	if (SPD_REG16(SPD_R_REV_1) < 0x11) SPD_REG8(0x100) = 1;

	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE1, SMAP_E3_FDX_ENABLE|SMAP_E3_IGNORE_SQE|SMAP_E3_MEDIA_100M|SMAP_E3_RXFIFO_2K|SMAP_E3_TXFIFO_1K|SMAP_E3_TXREQ0_MULTI|SMAP_E3_TXREQ1_SINGLE);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_TxMODE1, 7<<SMAP_E3_TX_LOW_REQ_BITSFT | 15<<SMAP_E3_TX_URG_REQ_BITSFT);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_RxMODE, SMAP_E3_RX_RX_RUNT_FRAME|SMAP_E3_RX_RX_FCS_ERR|SMAP_E3_RX_RX_TOO_LONG_ERR|SMAP_E3_RX_RX_IN_RANGE_ERR|SMAP_E3_RX_PROP_PF|SMAP_E3_RX_PROMISC);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTR_STAT, SMAP_E3_INTR_TX_ERR_0|SMAP_E3_INTR_SQE_ERR_0|SMAP_E3_INTR_DEAD_0);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTR_ENABLE, SMAP_E3_INTR_TX_ERR_0|SMAP_E3_INTR_SQE_ERR_0|SMAP_E3_INTR_DEAD_0);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_ADDR_HI, 0);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_ADDR_LO, 0);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_PAUSE_TIMER, 0xFFFF);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_INTER_FRAME_GAP, 4);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_TX_THRESHOLD, 12<<SMAP_E3_TX_THRESHLD_BITSFT);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_RX_WATERMARK, 16<<SMAP_E3_RX_LO_WATER_BITSFT|128<<SMAP_E3_RX_HI_WATER_BITSFT);

	dev9_smap_write_phy(emac3_regbase, SMAP_DsPHYTER_BMCR, SMAP_PHY_BMCR_RST);
	for (i = 9;; i--)
	{
		if (dev9_smap_read_phy(emac3_regbase, SMAP_DsPHYTER_BMCR, &value)) return 4;
		if (!(value & SMAP_PHY_BMCR_RST)) break;
		if (i <= 0) return 5;
	}

	dev9_smap_write_phy(emac3_regbase, SMAP_DsPHYTER_BMCR, SMAP_PHY_BMCR_LPBK|SMAP_PHY_BMCR_100M|SMAP_PHY_BMCR_DUPM);
	DelayThread(10000);
	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, SMAP_E3_TXMAC_ENABLE|SMAP_E3_RXMAC_ENABLE);
	value = SMAP_REG16(SMAP_R_TXFIFO_WR_PTR) + SMAP_TX_BASE;

	for(i=0; i<0x5EA; i+=4) SMAP_REG32(SMAP_R_TXFIFO_DATA) = i;

	tx_bd[0].length = 0xEA05;
	tx_bd[0].pointer = (value >> 8) | (value << 8);
	SMAP_REG8(SMAP_R_TXFIFO_FRAME_INC) = 0;
	tx_bd[0].ctrl_stat = 0x83;	//SMAP_BD_TX_READY|SMAP_BD_TX_GENFCS|SMAP_BD_TX_GENPAD

	SMAP_EMAC3_SET(SMAP_R_EMAC3_TxMODE0, SMAP_E3_TX_GNP_0);
	for (i = 9;; i--)
	{
		value = SPD_REG16(SPD_R_INTR_STAT);

		if ((value & (SMAP_INTR_RXEND|SMAP_INTR_TXEND|SMAP_INTR_TXDNV)) == (SMAP_INTR_RXEND|SMAP_INTR_TXEND|SMAP_INTR_TXDNV)) break;
		if (i <= 0) return 6;
		DelayThread(1000);
	}
	SMAP_EMAC3_SET(SMAP_R_EMAC3_MODE0, SMAP_E3_SOFT_RESET);

	return 0;
}
#endif

static int speed_device_init(void)
{
	USE_SPD_REGS;
	const char *spdnames[] = { "(unknown)", "TS", "ES1", "ES2" };
	int idx, res;
	u16 spdrev;
#ifndef DEV9_SKIP_SMAP_INIT
	int i;
#endif

	eeprom_data[0] = 0;

#ifndef DEV9_SKIP_SMAP_INIT
	for(i = 0; i < 8; i++) {
#endif
		if (dev9_device_probe() < 0) {
			M_PRINTF("No device.\n");
			return -1;
		}

		dev9_device_reset();

		/* Locate the SPEED Lite chip and get the bus ready for the
		   PCMCIA device.  */
		if ((res = dev9_card_find_manfid(0xf15300)))
			M_PRINTF("SPEED Lite not found.\n");

		if (!res && (res = dev9_ssbus_mode(5)))
			M_PRINTF("Unable to change SSBUS mode.\n");

		if (res) {
			dev9Shutdown();
			return -1;
		}

#ifndef DEV9_SKIP_SMAP_INIT
		if((res = dev9_smap_init()) == 0){
			break;
		}

		dev9Shutdown();
		DelayThread(4500000);
	}

	if (res) {
		M_PRINTF("SMAP initialization failed: %d\n", res);
		eeprom_data[0] = -1;
	}
#endif

	/* Print out the SPEED chip revision.  */
	spdrev = SPD_REG16(SPD_R_REV_1);
	idx    = (spdrev & 0xffff) - 14;
	if (spdrev == 9)
		idx = 1;	/* TS */
	else if (spdrev < 9 || (spdrev < 16 || spdrev > 17))
		idx = 0;	/* Unknown revision */

	M_PRINTF("SPEED chip '%s', revision %0x\n", spdnames[idx], spdrev);
	return 0;
}

static int pcic_get_cardtype(void)
{
	USE_DEV9_REGS;
	u16 val = DEV9_REG(DEV9_R_1462) & 0x03;

	if (val == 0)
		return PC_CARD_TYPE_PCMCIA;	/* 16-bit */
	else
	if (val < 3)	//If the bit pattern is either 10b or 01b
		return PC_CARD_TYPE_CARDBUS;	/* CardBus */
	return PC_CARD_TYPE_NONE;
}

static int pcic_get_voltage(void)
{
	USE_DEV9_REGS;
	u16 val = DEV9_REG(DEV9_R_1462) & 0x0c;

	if (val == 0x04)
		return PC_CARD_VOLTAGE_04h;
	if (val == 0 || val == 0x08)
		return PC_CARD_VOLTAGE_3V;
	if (val == 0x0c)
		return PC_CARD_VOLTAGE_5V;
	return PC_CARD_VOLTAGE_INVALID;
}

static int pcic_power(int voltage, int flag)
{
	USE_DEV9_REGS;
	u16 cstc1, cstc2;
	u16 val = (voltage == 1) << 2;

	DEV9_REG(DEV9_R_POWER) = 0;

	if (voltage == 2)
		val |= 0x08;
	if (flag == 1)
		val |= 0x10;

	DEV9_REG(DEV9_R_POWER) = val;
	DelayThread(22000);

	if (DEV9_REG(DEV9_R_1462) & 0x100)
		return 0;

	DEV9_REG(DEV9_R_POWER) = 0;
	DEV9_REG(DEV9_R_1464) = cstc1 = DEV9_REG(DEV9_R_1464);
	DEV9_REG(DEV9_R_1466) = cstc2 = DEV9_REG(DEV9_R_1466);
	return -1;
}

static void pcmcia_set_stat(int stat)
{
	USE_DEV9_REGS;
	u16 val = stat & 0x01;

	if (stat & 0x10)
		val = 1;
	if (stat & 0x02)
		val |= 0x02;
	if (stat & 0x20)
		val |= 0x02;
	if (stat & 0x04)
		val |= 0x08;
	if (stat & 0x08)
		val |= 0x10;
	if (stat & 0x200)
		val |= 0x20;
	if (stat & 0x100)
		val |= 0x40;
	if (stat & 0x400)
		val |= 0x80;
	if (stat & 0x800)
		val |= 0x04;
	DEV9_REG(DEV9_R_1476) = val & 0xff;
}

static int pcic_ssbus_mode(int mode)
{
	USE_DEV9_REGS;
	USE_SPD_REGS;
	u16 stat = DEV9_REG(DEV9_R_1474) & 7;

	if (mode != 3 && mode != 5)
		return -1;

	DEV9_REG(DEV9_R_1460) = 2;
	if (stat)
		return -1;

	if (mode == 3) {
		DEV9_REG(DEV9_R_1474) = 1;
		DEV9_REG(DEV9_R_1460) = 1;
		SPD_REG8(0x20) = 1;
		DEV9_REG(DEV9_R_1474) = mode;
	} else if (mode == 5) {
		DEV9_REG(DEV9_R_1474) = mode;
		DEV9_REG(DEV9_R_1460) = 1;
		SPD_REG8(0x20) = 1;
		DEV9_REG(DEV9_R_1474) = 7;
	}
	_sw(0xe01a3043, SSBUS_R_1418);

	DelayThread(5000);
	DEV9_REG(DEV9_R_POWER) = DEV9_REG(DEV9_R_POWER) & ~1;
	return 0;
}

static int pcmcia_device_probe(void)
{
	const char *pcic_ct_names[] = { "No", "16-bit", "CardBus" };
	int voltage;

	pcic_voltage = pcic_get_voltage();
	pcic_cardtype = pcic_get_cardtype();
	voltage = (pcic_voltage == PC_CARD_VOLTAGE_5V ? 5 : (pcic_voltage == PC_CARD_VOLTAGE_3V ? 3 : 0));

	M_PRINTF("%s PCMCIA card detected. Vcc = %dV\n",
			pcic_ct_names[pcic_cardtype], voltage);

	if (pcic_voltage == PC_CARD_VOLTAGE_04h || pcic_cardtype != PC_CARD_TYPE_PCMCIA)
		return -1;

	return 0;
}

static int pcmcia_device_reset(void)
{
	USE_DEV9_REGS;
	u16 cstc1, cstc2;

	/* The card must be 16-bit (type 2?) */
	if ((DEV9_REG(DEV9_R_1462) & 0x03) != 0)
		return -1;

	DEV9_REG(DEV9_R_147E) = 1;
	if (pcic_power(pcic_voltage, 1) < 0)
		return -1;

	DEV9_REG(DEV9_R_POWER) = DEV9_REG(DEV9_R_POWER) | 0x02;
	DelayThread(500000);

	DEV9_REG(DEV9_R_POWER) = DEV9_REG(DEV9_R_POWER) | 0x01;
	DEV9_REG(DEV9_R_1464) = cstc1 = DEV9_REG(DEV9_R_1464);
	DEV9_REG(DEV9_R_1466) = cstc2 = DEV9_REG(DEV9_R_1466);
	return 0;
}

static int card_find_manfid(u32 manfid)
{
	USE_DEV9_REGS;
	USE_SPD_REGS;
	u32 spdaddr, spdend, next, tuple;
	u8 hdr, ofs;

	DEV9_REG(DEV9_R_1460) = 2;
	_sw(0x1a00bb, SSBUS_R_1418);

	/* Scan the card for the MANFID tuple.  */
	spdaddr = 0;
	spdend =  0x1000;
	/* I hate this code, and it hates me.  */
	while (spdaddr < spdend) {
		hdr = SPD_REG8(spdaddr) & 0xff;
		spdaddr += 2;
		if (!hdr)
			continue;
		if (hdr == 0xff)
			break;
		if (spdaddr >= spdend)
			goto error;

		ofs = SPD_REG8(spdaddr) & 0xff;
		spdaddr += 2;
		if (ofs == 0xff)
			break;

		next = spdaddr + (ofs * 2);
		if (next >= spdend)
			goto error;

		if (hdr == 0x20) {
			if ((spdaddr + 8) >= spdend)
				goto error;

			tuple = (SPD_REG8(spdaddr + 2) << 24)|
				(SPD_REG8(spdaddr) << 16)|
				(SPD_REG8(spdaddr + 6) << 8)|
				 SPD_REG8(spdaddr + 4);
			if (manfid == tuple)
				return 0;
			M_PRINTF("MANFID 0x%08lx doesn't match expected 0x%08lx\n",
					tuple, manfid);
			return -1;
		}
		spdaddr = next;
	}

	M_PRINTF("MANFID 0x%08lx not found.\n", manfid);
	return -1;
error:
	M_PRINTF("Invalid tuples at offset 0x%08lx.\n", spdaddr - SPD_REGBASE);
	return -1;
}

static int pcmcia_intr(void *unused)
{
	USE_AIF_REGS;
	USE_DEV9_REGS;
	u16 cstc1, cstc2;

	cstc1 = DEV9_REG(DEV9_R_1464);
	cstc2 = DEV9_REG(DEV9_R_1466);

	if (using_aif) {
		if (aif_regs[AIF_INTSR] & AIF_INTR_PCMCIA)
			aif_regs[AIF_INTCL] = AIF_INTR_PCMCIA;
		else
			return 0;		/* Unknown interrupt.  */
	}

	/* Acknowledge the interrupt.  */
	DEV9_REG(DEV9_R_1464) = cstc1;
	DEV9_REG(DEV9_R_1466) = cstc2;
	if (cstc1 & 0x03 || cstc2 & 0x03) {	/* Card removed or added? */
		if (p_dev9_intr_cb)
			p_dev9_intr_cb(1);

		/* Shutdown the card.  */
		DEV9_REG(DEV9_R_POWER) = 0;
		DEV9_REG(DEV9_R_1474) = 0;

		pcmcia_device_probe();
	}
	if (cstc1 & 0x80 || cstc2 & 0x80) {
		if (p_dev9_intr_cb)
			p_dev9_intr_cb(0);
	}

	DEV9_REG(DEV9_R_147E) = 1;
	DEV9_REG(DEV9_R_147E) = 0;
	return 1;
}

static int pcmcia_init(int sema_attr)
{
	USE_DEV9_REGS;
	USE_AIF_REGS;
	int *mode;
	int flags;
	u16 cstc1, cstc2;

	_sw(0x51011, SSBUS_R_1420);
	_sw(0x1a00bb, SSBUS_R_1418);
	_sw(0xef1a3043, SSBUS_R_141c);

	/* If we are a T10K, then we go through AIF.  */
	if ((mode = QueryBootMode(6)) != NULL) {
		if ((*(u16 *)mode & 0xfe) == 0x60) {
			M_PRINTF("T10K detected.\n");

			if (aif_regs[AIF_IDENT] == 0xa1) {
				aif_regs[AIF_INTEN] = AIF_INTR_PCMCIA;
				using_aif = 1;
			} else {
				M_PRINTF("AIF not detected.\n");
				return 1;
			}
		}
	}

	if (DEV9_REG(DEV9_R_POWER) == 0) {
		DEV9_REG(DEV9_R_POWER) = 0;
		DEV9_REG(DEV9_R_147E) = 1;
		DEV9_REG(DEV9_R_1460) = 0;
		DEV9_REG(DEV9_R_1474) = 0;
		DEV9_REG(DEV9_R_1464) = cstc1 = DEV9_REG(DEV9_R_1464);
		DEV9_REG(DEV9_R_1466) = cstc2 = DEV9_REG(DEV9_R_1466);
		DEV9_REG(DEV9_R_1468) = 0x10;
		DEV9_REG(DEV9_R_146A) = 0x90;
		DEV9_REG(DEV9_R_147C) = 1;
		DEV9_REG(DEV9_R_147A) = DEV9_REG(DEV9_R_147C);

		pcic_voltage = pcic_get_voltage();
		pcic_cardtype = pcic_get_cardtype();

		if (speed_device_init() != 0)
			return 1;
	} else {
		_sw(0xe01a3043, SSBUS_R_1418);
	}

	if (dev9_init(sema_attr) != 0)
		return 1;

	CpuSuspendIntr(&flags);
	RegisterIntrHandler(IOP_IRQ_DEV9, 1, &pcmcia_intr, NULL);
	EnableIntr(IOP_IRQ_DEV9);
	CpuResumeIntr(flags);

	DEV9_REG(DEV9_R_147E) = 0;

	if (RegisterLibraryEntries(&_exp_dev9) != 0)
		return 1;

	M_PRINTF("CXD9566 (PCMCIA) driver start.\n");
	return 0;
}

static void expbay_set_stat(int stat)
{
	USE_DEV9_REGS;
	DEV9_REG(DEV9_R_1464) = stat & 0x3f;
}

static int expbay_device_probe(void)
{
	USE_DEV9_REGS;
	return (DEV9_REG(DEV9_R_1462) & 0x01) ? -1 : 0;
}

static int expbay_device_reset(void)
{
	USE_DEV9_REGS;

	if (expbay_device_probe() < 0)
		return -1;

	DEV9_REG(DEV9_R_POWER) = (DEV9_REG(DEV9_R_POWER) & ~1) | 0x04;	// power on
	DelayThread(500000);

	DEV9_REG(DEV9_R_1460) = DEV9_REG(DEV9_R_1460) | 0x01;
	DEV9_REG(DEV9_R_POWER) = DEV9_REG(DEV9_R_POWER) | 0x01;
	DelayThread(500000);
	return 0;
}

static int expbay_intr(void *unused)
{
	USE_DEV9_REGS;

	if (p_dev9_intr_cb)
		p_dev9_intr_cb(0);

	DEV9_REG(DEV9_R_1466) = 1;
	DEV9_REG(DEV9_R_1466) = 0;
	return 1;
}

static int expbay_init(int sema_attr)
{
	USE_DEV9_REGS;
	int flags;

	_sw(0x51011, SSBUS_R_1420);
	_sw(0xe01a3043, SSBUS_R_1418);
	_sw(0xef1a3043, SSBUS_R_141c);

	if ((DEV9_REG(DEV9_R_POWER) & 0x04) == 0) { // if not already powered
		DEV9_REG(DEV9_R_1466) = 1;
		DEV9_REG(DEV9_R_1464) = 0;
		DEV9_REG(DEV9_R_1460) = DEV9_REG(DEV9_R_1464);

		if (speed_device_init() != 0)
			return 1;
	}

	if (dev9_init(sema_attr) != 0)
		return 1;

	CpuSuspendIntr(&flags);
	RegisterIntrHandler(IOP_IRQ_DEV9, 1, &expbay_intr, NULL);
	EnableIntr(IOP_IRQ_DEV9);
	CpuResumeIntr(flags);

	DEV9_REG(DEV9_R_1466) = 0;

	if (RegisterLibraryEntries(&_exp_dev9) != 0)
		return 1;

	M_PRINTF("CXD9611 (SSBUS Buffer) driver start.\n");
	return 0;
}
