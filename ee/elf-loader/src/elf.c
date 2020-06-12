/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 20020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <string.h>
#include <sifrpc.h>
#include <kernel.h>

#include "elf.h"

#define MAX_PATH 1025

extern u8 loader_elf[];
extern int size_loader_elf;

// ELF-loading stuff
#define ELF_MAGIC 0x464c457f
#define ELF_PT_LOAD 1


//------------------------------
//End of func:  int checkELFheader(const char *path)
//--------------------------------------------------------------
// RunLoaderElf loads LOADER.ELF from program memory and passes
// args of selected ELF and partition to it
// Modified version of loader from Independence
//	(C) 2003 Marcus R. Brown <mrbrown@0xd6.org>
//------------------------------
void RunLoaderElf(char *filename, char *party)
{
	u8 *boot_elf;
	elf_header_t *eh;
	elf_pheader_t *eph;
	void *pdata;
	int i;
	char *argv[2];

    argv[0] = filename;
    argv[1] = filename;

	/* NB: LOADER.ELF is embedded  */
	boot_elf = (u8 *)loader_elf;
	eh = (elf_header_t *)boot_elf;
	if (_lw((u32)&eh->ident) != ELF_MAGIC)
		asm volatile("break\n");

	eph = (elf_pheader_t *)(boot_elf + eh->phoff);

	/* Scan through the ELF's program headers and copy them into RAM, then
									zero out any non-loaded regions.  */
	for (i = 0; i < eh->phnum; i++) {
		if (eph[i].type != ELF_PT_LOAD)
			continue;

		pdata = (void *)(boot_elf + eph[i].offset);
		memcpy(eph[i].vaddr, pdata, eph[i].filesz);

		if (eph[i].memsz > eph[i].filesz)
			memset(eph[i].vaddr + eph[i].filesz, 0,
			       eph[i].memsz - eph[i].filesz);
	}

	/* Let's go.  */
	SifExitRpc();
	FlushCache(0);
	FlushCache(2);

	ExecPS2((void *)eh->entry, NULL, 2, argv);
}
