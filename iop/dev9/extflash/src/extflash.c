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
# NAND Flash via Dev9 driver.
*/

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "loadcore.h"
#include "thbase.h"
#include "stdio.h"
#include "sysclib.h"

#include "dev9regs.h"
#include "speedregs.h"

#define MODNAME "flash"
#define M_PRINTF(format, args...)	\
	printf(MODNAME ": " format, ## args)

IRX_ID(MODNAME, 1, 1);

#define FLASH_ID_64MBIT		0xe6
#define FLASH_ID_128MBIT	0x73
#define FLASH_ID_256MBIT	0x75
#define FLASH_ID_512MBIT	0x76
#define FLASH_ID_1024MBIT	0x79

/* SmartMedia commands.  */
#define SM_CMD_READ1		0x00
#define SM_CMD_READ2		0x01
#define SM_CMD_READ3		0x50
#define SM_CMD_RESET		0xff
#define SM_CMD_WRITEDATA	0x80
#define SM_CMD_PROGRAMPAGE	0x10
#define SM_CMD_ERASEBLOCK	0x60
#define SM_CMD_ERASECONFIRM	0xd0
#define SM_CMD_GETSTATUS	0x70
#define SM_CMD_READID		0x90

typedef struct {
	u32	id;
	u32	mbits;
	u32	page_bytes;	/* bytes/page */
	u32	block_pages;	/* pages/block */
	u32	blocks;
} flash_info_t;

static flash_info_t devices[] = {
	{ FLASH_ID_64MBIT, 64, 528, 16, 1024 },
	{ FLASH_ID_128MBIT, 128, 528, 32, 1024 },
	{ FLASH_ID_256MBIT, 256, 528, 32, 2048 },
	{ FLASH_ID_512MBIT, 512, 528, 32, 4096 },
	{ FLASH_ID_1024MBIT, 1024, 528, 32, 8192 }
};
#define NUM_DEVICES	(sizeof(devices)/sizeof(flash_info_t))

static iop_sys_clock_t timeout;
static u32 timeout_cb(void *);

int flash_detect(void);
int flash_call5(void);
int flash_get_info(flash_info_t *info);

struct irx_export_table _exp_fls;

int _start(int argc, char *argv[])
{
	if (flash_detect() < 0)
		return MODULE_NO_RESIDENT_END;

	return RegisterLibraryEntries(&_exp_fls) != 0;
}

/* #4: Check to see if flash is connected to the expansion bay.  */
int flash_detect()
{
	USE_DEV9_REGS;
	USE_SPD_REGS;
	u16 rev;

	if ((DEV9_REG(DEV9_R_REV) & 0xf0) != DEV9_DEV9C_9611) {
		M_PRINTF("CXD9611 required for flash interface.\n");
		return -1;
	}

	/* Check bit 5 of the revision register.  */
	if ((rev = SPD_REG16(SPD_R_REV_3)) & 0x20) {
		M_PRINTF("Detected flash (rev3 0x%02x).\n", rev);
		return 0;
	}

	M_PRINTF("No flash detected (rev3 0x%02x).\n", rev);
	return -1;
}

/* #5: Reset the device.  */
int flash_device_reset()
{
	USE_SPD_REGS;
	u32 has_timedout = 0;
	u16 tmp;

	SPD_REG16(0x480c) = 0x100;
	SPD_REG16(0x4804) = SM_CMD_RESET;
	tmp = SPD_REG16(0x480c);
	tmp = SPD_REG16(0x480c);

	SetAlarm(&timeout, timeout_cb, &has_timedout);

	while ((SPD_REG16(0x480c) & 0x01) == 0) {
		if (has_timedout) {
			if (SPD_REG16(0x480c) & 0x01)
				break;

			M_PRINTF("Device erase: timed out.\n");
			return -3;
		}
	}

	CancelAlarm(timeout_cb, &has_timedout);

	SPD_REG16(0x4804) = SM_CMD_GETSTATUS;
	if (SPD_REG16(0x4814) & 0x01) {
		M_PRINTF("Device erase: status error.\n");
		return -6;
	}

	SPD_REG16(0x480c) = 0;
	return 0;
}

/* #6: Get information about the attached device.  */
int flash_get_info(flash_info_t *info)
{
	USE_SPD_REGS;
	int i;
	u16 tmp, id;

	SPD_REG16(0x480c) = 0x100;
	SPD_REG16(0x4804) = SM_CMD_READID;
	SPD_REG16(0x4808) = 0;
	tmp = SPD_REG16(0x480c);
	tmp = SPD_REG16(0x4814);
	id = SPD_REG16(0x4814);
	SPD_REG16(0x480c) = 0;

	memset(info, 0, sizeof(flash_info_t));

	for (i = 0; i < NUM_DEVICES; i++) {
		if (id != devices[i].id)
			continue;

		memcpy(info, &devices[i], sizeof(flash_info_t));
		M_PRINTF("Device: ID 0x%02x, %dMbit, %d bytes/page, %d pages/block, %d blocks total.\n",
				id, info->mbits, info->page_bytes, info->block_pages, info->blocks);
		return 0;
	}

	return -5;
}

/* Set the page offset register based on the type of flash (id).  */
static void flash_set_page(u32 id, u32 pageofs)
{
	USE_SPD_REGS;

	switch (id) {
		case FLASH_ID_128MBIT:
			SPD_REG16(0x4808) = ((pageofs >> 9) & 0xf0) | 0x100;
			SPD_REG16(0x4808) = (pageofs >> 17) & 0x7f;
			break;
		case FLASH_ID_256MBIT:
			SPD_REG16(0x4808) = ((pageofs >> 9) & 0xf0) | 0x100;
			SPD_REG16(0x4808) = (pageofs >> 17) & 0xff;
			break;
		case FLASH_ID_512MBIT:
			SPD_REG16(0x4808) = ((pageofs >> 9) & 0xe0) | 0x100;
			SPD_REG16(0x4808) = ((pageofs >> 17) & 0xff) | 0x100;
			SPD_REG16(0x4808) = (pageofs >> 25) & 0x01;
			break;
		case FLASH_ID_1024MBIT:
			SPD_REG16(0x4808) = ((pageofs >> 9) & 0xe0) | 0x100;
			SPD_REG16(0x4808) = ((pageofs >> 17) & 0xff) | 0x100;
			SPD_REG16(0x4808) = (pageofs >> 25) & 0x02;
			break;
		case FLASH_ID_64MBIT:
			SPD_REG16(0x4808) = ((pageofs >> 9) & 0xf0) | 0x100;
			SPD_REG16(0x4808) = (pageofs >> 17) & 0x3f;
			break;
	}
}

/* #7: Erase a page.  */
int flash_page_erase(flash_info_t *info, u32 page)
{
	USE_SPD_REGS;
	u32 pageofs = page * 512;
	u32 has_timedout = 0;
	u16 tmp;

	SPD_REG16(0x480c) = 0x180;
	SPD_REG16(0x4804) = SM_CMD_ERASEBLOCK;

	flash_set_page(info->id, pageofs);

	SPD_REG16(0x4804) = SM_CMD_ERASECONFIRM;
	tmp = SPD_REG16(0x480c);
	tmp = SPD_REG16(0x480c);

	SetAlarm(&timeout, timeout_cb, &has_timedout);

	while ((SPD_REG16(0x480c) & 0x01) == 0) {
		if (has_timedout) {
			if (SPD_REG16(0x480c) & 0x01)
				break;

			M_PRINTF("Page erase: timed out.\n");
			return -3;
		}
	}

	CancelAlarm(timeout_cb, &has_timedout);

	SPD_REG16(0x4804) = SM_CMD_GETSTATUS;
	if (SPD_REG16(0x4814) & 0x01) {
		M_PRINTF("Page erase: status error.\n");
		return -6;
	}

	SPD_REG16(0x480c) = 0;
	return 0;
}

/* #8: Read one or more pages. You can select how much of the page you want to
 * read by setting the page_bytes member of the info struct. The default set
 * by the driver is 528 bytes:
 * Bytes 0 - 511: Data
 * Bytes 512 - 527: Metadata?  
 * 
 * You can also set page_bytes to 512 or 16 bytes.  */
int flash_page_read(flash_info_t *info, u32 page, u32 count, void *buf)
{
	USE_SPD_REGS;
	u32 *buf_w;
	u16 *buf_h;
	int i, j;
	u32 byteofs, pageofs = page * 512;
	u32 has_timedout;
	u16 tmp, func = 0x100;

	if (info->page_bytes == 512)
		func = 0x1100;

	SPD_REG16(0x480c) = func;

	if (info->page_bytes == 16) {
		SPD_REG16(0x4804) = SM_CMD_READ3;
		byteofs = pageofs & 0x0f;
	} else {
		SPD_REG16(0x4804) = SM_CMD_READ1;
		byteofs = pageofs & 0x1ff;
	}

	SPD_REG16(0x4808) = (byteofs & 0xff) | 0x100;
	/* Set the rest of the page info.  */
	flash_set_page(info->id, pageofs);

	buf_w = (u32 *)buf;
	buf_h = (u16 *)buf;

	for (i = 0; i < count; i++) {
		tmp = SPD_REG16(0x480c);
		tmp = SPD_REG16(0x480c);

		has_timedout = 0;
		SetAlarm(&timeout, timeout_cb, &has_timedout);

		while ((SPD_REG16(0x480c) & 0x01) == 0) {
			if (has_timedout) {
				if (SPD_REG16(0x480c) & 0x01)
					break;

				M_PRINTF("Page read: timed out.\n");
				return -3;
			}
		}

		CancelAlarm(timeout_cb, &has_timedout);

		SPD_REG16(0x480c) = func | 0x800;

		if (byteofs == 0) {
			/* 32-bit copy.  */
			for (j = 0; j < (info->page_bytes / 4); j++)
				*buf_w++ = SPD_REG32(0x4800);
		} else {
			/* 16-bit copy.  */
			for (j = (byteofs + (byteofs >> 31)) / 2; j < (info->page_bytes / 2); j++)
				*buf_h++ = SPD_REG16(0x4800);
		}

		byteofs = 0;
		SPD_REG16(0x480c) = func;
	}

	SPD_REG16(0x480c) = 0;
	return 0;
}

/* #9: Write a single page.
 * Bytes 0 - 511: Data
 * Bytes 512 - 527: Metadata?  */
int flash_page_write(flash_info_t *info, u32 page, void *buf)
{
	USE_SPD_REGS;
	u16 *buf_h = buf;
	int i;
	u32 pageofs = page * 512;
	u32 has_timedout = 0;
	u16 tmp;

	SPD_REG16(0x480c) = 0x180;
	SPD_REG16(0x4804) = SM_CMD_WRITEDATA;

	SPD_REG16(0x4808) = 0x100;
	flash_set_page(info->id, pageofs);

	for (i = 0; i < (info->page_bytes / 2); i++)
		SPD_REG16(0x4800) = *buf_h++;

	SPD_REG16(0x4804) = SM_CMD_PROGRAMPAGE;
	tmp = SPD_REG16(0x480c);
	tmp = SPD_REG16(0x480c);

	SetAlarm(&timeout, timeout_cb, &has_timedout);

	while ((SPD_REG16(0x480c) & 0x01) == 0) {
		if (has_timedout) {
			if (SPD_REG16(0x480c) & 0x01)
				break;

			M_PRINTF("Page write: timed out.\n");
			return -3;
		}
	}

	CancelAlarm(timeout_cb, &has_timedout);

	SPD_REG16(0x4804) = SM_CMD_GETSTATUS;
	if (SPD_REG16(0x4814) & 0x01) {
		M_PRINTF("Page write: status error.\n");
		return -6;
	}

	SPD_REG16(0x480c) = 0;
	return 0;
}

static u32 timeout_cb(void *arg)
{
	u32 *has_timedout = (u32 *)arg;

	*has_timedout = 1;
	return 0;
}
