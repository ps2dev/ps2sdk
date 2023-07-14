/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <mcman.h>
#include "mcman-internal.h"

#if defined(BUILDING_VMCMAN)
typedef struct vmc_cardinfo_
{
	int mounted;
	int has_ecc;
	int fd;
	s16 pagesize;
	u16 blocksize;
	int cardsize;
	u8 flags;
} vmc_cardinfo_t;

static vmc_cardinfo_t mcman_vmc_cardinfo[MCMAN_MAXSLOT];

static void mcman_iomanx_backing_clear_slot(int slot)
{
	vmc_cardinfo_t *cardinfo;
	cardinfo = &mcman_vmc_cardinfo[slot];
	if (cardinfo->fd >= 0) {
		iomanX_close(cardinfo->fd);
	}
	memset(cardinfo, 0, sizeof(vmc_cardinfo_t));
	cardinfo->fd = -1;

	// Just to clear any cached information
	mcman_probePS2Card(0, slot);
}

static int mcman_iomanx_backing_validate(int slot, int check_mounted)
{
	vmc_cardinfo_t *cardinfo;

	if (slot > MCMAN_MAXSLOT) {
		return -EOVERFLOW;
	}

	cardinfo = &mcman_vmc_cardinfo[slot];

	if (check_mounted) {
		if (cardinfo->mounted == 0) {
			return -ENXIO;
		}
	}
	return 0;
}

int mcman_iomanx_backing_mount(int port, int slot, const char *filename)
{
	int fd;
	int cardsize;
	int r;
	vmc_cardinfo_t *cardinfo;

	(void)port;

	r = mcman_iomanx_backing_validate(slot, 0);
	if (r < 0) {
		goto cleanup;
	}

	r = -EINVAL;

	cardinfo = &mcman_vmc_cardinfo[slot];

	fd = iomanX_open(filename, FIO_O_RDWR, 0);
	if (fd < 0) {
		// Propogate open error
		r = fd;
		goto cleanup;
	}
	cardinfo->fd = fd;

	cardsize = iomanX_lseek(fd, 0, FIO_SEEK_END);
	cardinfo->cardsize = cardsize;
	iomanX_lseek(fd, 0, FIO_SEEK_SET);

	// Check superblock for correct parameters
	{
		MCDevInfo superblock;
		int read_result;
		int total_pages;

		read_result = iomanX_read(fd, &superblock, sizeof(superblock));
		if (read_result != sizeof(superblock)) {
			if (read_result >= 0) {
				r = -EINVAL;
			}
			else {
				r = read_result;
			}
			goto cleanup;
		}
		// Check magic
		if (strncmp(superblock.magic, SUPERBLOCK_MAGIC, 28) != 0) {
			r = -EINVAL;
			goto cleanup;
		}
		// Check card type
		if (superblock.cardtype != sceMcTypePS2) {
			r = -EINVAL;
			goto cleanup;
		}
		total_pages = superblock.pages_per_cluster * superblock.blocksize;
		// Check card ECC
		if (cardsize == ((superblock.pagesize + 0x10) * total_pages)) {
			cardinfo->has_ecc = 1;
		}
		else if (cardsize == ((superblock.pagesize) * total_pages)) {
			cardinfo->has_ecc = 0;
		}
		else {
			// size is invalid
			r = -EINVAL;
			goto cleanup;
		}
		// Check page size
		if (superblock.pagesize != 512 && superblock.pagesize != 528) {
			// Not a tested page size
			r = -EINVAL;
			goto cleanup;
		}

		// Retrieve rest of info
		cardinfo->pagesize = superblock.pagesize;
		cardinfo->blocksize = superblock.blocksize;
		cardinfo->flags = superblock.cardflags;
	}

	mcman_probePS2Card(port, slot);
	if (McGetFormat(port, slot) > 0) {
		r = 0; // Success
		goto cleanup;
	}
	if (McGetFormat(port, slot) < 0) {
		r = -EIO;
		goto cleanup;
	}

cleanup:
	if (r < 0) {
		mcman_iomanx_backing_clear_slot(slot);
	}
	return r;
}

int mcman_iomanx_backing_umount(int port, int slot)
{
	int r;

	(void)port;

	r = mcman_iomanx_backing_validate(slot, 1);
	if (r < 0) {
		goto cleanup;
	}

	mcman_iomanx_backing_clear_slot(slot);

cleanup:
	return r;
}

int mcman_iomanx_backing_getcardspec(int port, int slot, s16 *pagesize, u16 *blocksize, int *cardsize, u8 *flags)
{
	int r;
	vmc_cardinfo_t *cardinfo;

	(void)port;

	r = mcman_iomanx_backing_validate(slot, 1);
	if (r < 0) {
		return 1;
	}

	cardinfo = &mcman_vmc_cardinfo[slot];

	if (pagesize) {
		*pagesize = cardinfo->pagesize;
	}

	if (blocksize) {
		*blocksize = cardinfo->blocksize;
	}

	if (cardsize) {
		*cardsize = cardinfo->cardsize;
	}

	if (flags) {
		*flags = cardinfo->flags;
	}

	return 0;
}

int mcman_iomanx_backing_erase(int port, int slot, int page)
{
	int r;
	vmc_cardinfo_t *cardinfo;

	(void)port;

	r = mcman_iomanx_backing_validate(slot, 1);
	if (r < 0) {
		return 1;
	}

	cardinfo = &mcman_vmc_cardinfo[slot];

	{
		char buf[528];
		int effective_page_size;

		memset(buf, ((cardinfo->flags & CF_ERASE_ZEROES) != 0) ? 0x0 : 0xFF, sizeof(buf));
		effective_page_size = (cardinfo->pagesize + (cardinfo->has_ecc ? 0x10 : 0));
		iomanX_lseek(cardinfo->fd, page * effective_page_size, FIO_SEEK_SET);
		iomanX_write(cardinfo->fd, buf, effective_page_size);
	}

	return 0;
}

int mcman_iomanx_backing_write(int port, int slot, int page, void *pagebuf, void *eccbuf)
{
	int r;
	vmc_cardinfo_t *cardinfo;

	(void)port;

	r = mcman_iomanx_backing_validate(slot, 1);
	if (r < 0) {
		return 1;
	}

	cardinfo = &mcman_vmc_cardinfo[slot];

	{
		int effective_page_size;

		effective_page_size = (cardinfo->pagesize + (cardinfo->has_ecc ? 0x10 : 0));
		iomanX_lseek(cardinfo->fd, page * effective_page_size, FIO_SEEK_SET);
		if (pagebuf != NULL) {
			iomanX_write(cardinfo->fd, pagebuf, 512);
		}
		else {
			iomanX_lseek(cardinfo->fd, 512, FIO_SEEK_CUR);
		}
		
		if (cardinfo->has_ecc) {
			if (eccbuf != NULL) {
				iomanX_write(cardinfo->fd, eccbuf, 16);
			}
			else {
				iomanX_lseek(cardinfo->fd, 16, FIO_SEEK_CUR);
			}
		}
	}

	// FIXME: implement

	return 0;
}

int mcman_iomanx_backing_read(int port, int slot, int page, void *pagebuf, void *eccbuf)
{
	int r;
	vmc_cardinfo_t *cardinfo;

	(void)port;

	r = mcman_iomanx_backing_validate(slot, 1);
	if (r < 0) {
		return 1;
	}

	cardinfo = &mcman_vmc_cardinfo[slot];

	{
		int pagesize;
		int effective_page_size;

		pagesize = cardinfo->pagesize;

		effective_page_size = (cardinfo->pagesize + (cardinfo->has_ecc ? 0x10 : 0));
		iomanX_lseek(cardinfo->fd, page * effective_page_size, FIO_SEEK_SET);
		if (pagebuf != NULL) {
			iomanX_read(cardinfo->fd, pagebuf, 512);
		}
		else {
			iomanX_lseek(cardinfo->fd, 512, FIO_SEEK_CUR);
		}
		
		if (cardinfo->has_ecc) {
			if (eccbuf != NULL) {
				iomanX_read(cardinfo->fd, eccbuf, 16);
			}
			else {
				iomanX_lseek(cardinfo->fd, 16, FIO_SEEK_CUR);
			}
		} else if (pagebuf != NULL && eccbuf != NULL) {
			int i;
			int ecc_count;
			u8 *p_page, *p_ecc;

			// Make our own ECC

			memset(eccbuf, ((cardinfo->flags & CF_ERASE_ZEROES) != 0) ? 0x0 : 0xFF, 16);
			p_page = pagebuf;
			p_ecc = eccbuf;

			i = 0;	//s1
			do {
				if (pagesize < 0)
					ecc_count = (pagesize + 0x7f) >> 7;
				else
					ecc_count = pagesize >> 7;

				if (i >= ecc_count)
					break;

				McDataChecksum(p_page, p_ecc);

				p_ecc = (void *)((u8 *)p_ecc + 3);
				p_page = (void *)((u8 *)p_page + 128);
				i++;
			} while (1);
		}
	}

	return 0;
}
#endif
