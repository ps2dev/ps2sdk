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
# ATA hardware types and definitions.
*/

#ifndef DRV_ATAHW_H
#define DRV_ATAHW_H

#include "types.h"
#include "speedregs.h"

#define ATA_DEV9_HDD_BASE		(SPD_REGBASE + 0x40)
/* AIF on T10Ks - Not supported yet.  */
#define ATA_AIF_HDD_BASE		(SPD_REGBASE + 0x4000000 + 0x60)

/* A port contains all of the ATA controller registers.  */
typedef struct _ata_hwport {
	u16	r_data;		/* 00 */
	u16	r_error;	/* 02 */
#define r_feature r_error
	u16	r_nsector;	/* 04 */
	u16	r_sector;	/* 06 */
	u16	r_lcyl;		/* 08 */
	u16	r_hcyl;		/* 0a */
	u16	r_select;	/* 0c */
	u16	r_status;	/* 0e */
#define r_command r_status
	u16	pad[6];
	u16	r_control;	/* 1c */
} ata_hwport_t;

/* This is set to either ATA_AIF_HDD_BASE or ATA_DEV9_HDD_BASE.  */
u32 ata_active_port;

#define USE_ATA_REGS		volatile ata_hwport_t *ata_hwport = \
	(volatile ata_hwport_t *)ATA_DEV9_HDD_BASE

/* r_error bits.  */
#define ATA_ERR_MARK		0x01
#define ATA_ERR_TRACK0		0x02
#define ATA_ERR_ABORT		0x04
#define ATA_ERR_MCR		0x08
#define ATA_ERR_ID		0x10
#define ATA_ERR_MC		0x20
#define ATA_ERR_ECC		0x40
#define ATA_ERR_ICRC		0x80

/* r_status bits.  */
#define ATA_STAT_ERR		0x01
#define ATA_STAT_INDEX		0x02
#define ATA_STAT_ECC		0x04
#define	ATA_STAT_DRQ		0x08
#define ATA_STAT_SEEK		0x10
#define ATA_STAT_WRERR		0x20
#define ATA_STAT_READY		0x40
#define ATA_STAT_BUSY		0x80

/* ATA command codes.  */
#define ATA_C_SCE_SEC_CONTROL 0x8e
#define ATA_C_IDENTIFY_PKT_DEVICE 0xa1
#define ATA_C_IDLE		0xe3
#define ATA_C_FLUSH_CACHE	0xe7
#define ATA_C_FLUSH_CACHE_EXT	0xea
#define ATA_C_IDENTIFY_DEVICE	0xec

#define ATA_C_SET_FEATURES	0xef

#define ATA_C_SMART		0xb0
#define   ATA_C_SMART_SAVE_ATTR		0xd3
#define   ATA_C_SMART_ENABLE 		0xd8
#define   ATA_C_SMART_GET_STATUS	0xda

#define ATA_C_READ_DMA		0xc8
#define ATA_C_READ_DMA_EXT	0x25
#define ATA_C_WRITE_DMA		0xca
#define ATA_C_WRITE_DMA_EXT	0x35

#define ATA_C_SEC_SET_PASSWORD	0xf1
#define ATA_C_SEC_UNLOCK	0xf2
#define ATA_C_SEC_ERASE_PREPARE	0xf3
#define ATA_C_SEC_ERASE_UNIT	0xf4

/* Offsets for the data returned from IDENTIFY DEVICE commands.  */
enum _ata_identify_offsets {
	ATA_ID_SECTOTAL_LO = 60, ATA_ID_SECTOTAL_HI = 61,
	ATA_ID_COMMAND_SETS_SUPPORTED = 83,
	ATA_ID_48BIT_SECTOTAL_LO = 100, ATA_ID_48BIT_SECTOTAL_MI = 101, ATA_ID_48BIT_SECTOTAL_HI = 102,
	ATA_ID_SECURITY_STATUS = 128
};

/* Bits in the security status word.  */
#define ATA_F_SEC_ENABLED	(1<<1)
#define ATA_F_SEC_LOCKED	(1<<2)

#endif /* DRV_ATAHW_H */

