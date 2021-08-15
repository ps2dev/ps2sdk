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
 * Expansion bay flash driver.
 * Supported in ROM in SCPH-5xxxx models and integrated in DESR-xxxx models.
 */

#ifndef __FLS_H__
#define __FLS_H__

typedef struct {
	u32	id;
	u32	mbits;
	u32	page_bytes;	/* bytes/page */
	u32	block_pages;	/* pages/block */
	u32	blocks;
} flash_info_t;

extern int flash_detect(void);
extern int flash_device_reset(void);
extern int flash_get_info(flash_info_t *info);
extern int flash_page_erase(flash_info_t *info, u32 page);
extern int flash_page_read(flash_info_t *info, u32 page, u32 count, void *buf);
extern int flash_page_write(flash_info_t *info, u32 page, void *buf);

#define fls_IMPORTS_start DECLARE_IMPORT_TABLE(fls, 1, 1)
#define fls_IMPORTS_end END_IMPORT_TABLE

#define I_flash_detect DECLARE_IMPORT(4, flash_detect)
#define I_flash_device_reset DECLARE_IMPORT(5, flash_device_reset)
#define I_flash_get_info DECLARE_IMPORT(6, flash_get_info)
#define I_flash_page_erase DECLARE_IMPORT(7, flash_page_erase)
#define I_flash_page_read DECLARE_IMPORT(8, flash_page_read)
#define I_flash_page_write DECLARE_IMPORT(9, flash_page_write)

#endif
