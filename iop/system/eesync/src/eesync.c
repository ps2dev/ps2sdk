/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <loadcore.h>
#include <intrman.h>
#include <ioman.h>
#include <sifman.h>
#include <sysmem.h>
#include <sysclib.h>

#define MODNAME "SyncEE"
IRX_ID(MODNAME, 0x01, 0x01);

static int PostResetCallback(iop_init_entry_t *arg1, int arg2)
{
#ifdef EESYNC_WIPE_EE_KERNEL_SPACE
	void *block;
	SifDmaTransfer_t dmat;

	block = AllocSysMemory(0, 0x7C000, 0);
	memset(block, 0, 0x7C000);
	dmat.src = block;
	dmat.dest = (void *)0x84000;
	dmat.size = 0x7C000;
	dmat.attr = 0;
	sceSifSetDma(&dmat, 1);
	FreeSysMemory(block);
#endif
	(void)arg1;
	(void)arg2;

	sceSifSetSMFlag(SIF_STAT_BOOTEND);

	return 0;
}

#ifdef EESYNC_SECRMAN_DUMMY
static void allocate_empty_space_for_secrman(void)
{
	int fd;

	// In the original SCE version, there was obfuscation applied to the file string.
	fd = open("rom0:SECRMAN", O_RDONLY);
	if (fd >= 0)
	{
		int lseek_result;
		int close_result;

		lseek_result = lseek(fd, 0, SEEK_END);
		close_result = close(fd);
		if ((lseek_result >= 0) && (close_result >= 0))
		{
			AllocSysMemory(0, (lseek_result == 10033) ? 6400 : 256, NULL);
		}
	}
}
#endif

#ifdef EESYNC_REGISTER_EXPORTS
extern struct irx_export_table _exp_eesync;
#endif

int _start(int argc, char **argv)
{
#ifdef EESYNC_CHECK_ILOADP
	int *BootMode;
#endif

	(void)argc;
	(void)argv;

#ifdef EESYNC_CHECK_ILOADP
	BootMode = QueryBootMode(3);
	// If ILOADP bit 0 (Init SIF) is set or ILOADP bit 1 (Do not perform FILEIO services) is set, do not initialize SIF
	if (BootMode != NULL && (((BootMode[1] & 1) != 0) || (BootMode[1] & 2) != 0))
	{
		return MODULE_NO_RESIDENT_END;
	}
#endif
#ifdef EESYNC_REGISTER_EXPORTS
	if (RegisterLibraryEntries(&_exp_eesync) < 0)
	{
		return MODULE_NO_RESIDENT_END;
	}
#endif
#ifdef EESYNC_SECRMAN_DUMMY
	allocate_empty_space_for_secrman();
#endif
	RegisterPostBootCallback(&PostResetCallback, 2, 0);

	return MODULE_RESIDENT_END;
}
