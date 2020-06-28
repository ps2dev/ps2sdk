/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <string.h>
#include <sifrpc.h>
#include <kernel.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <malloc.h>

#include "elf.h"

// Loader ELF variables
extern u8 loader_elf[];
extern int size_loader_elf;

// ELF-loading stuff
#define ELF_MAGIC 0x464c457f
#define ELF_PT_LOAD 1

static bool file_exists(const char *filename) {
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

int LoadELFFromFile(const char *filename, int argc, char *argv[])
{
	u8 *boot_elf;
	elf_header_t *eh;
	elf_pheader_t *eph;
	void *pdata;
	int i;
	int new_argc = argc + 1;
	
	// We need to check that the ELF file before continue
	if (!file_exists(filename)) {
		return -1; // ELF file doesn't exists
	}
	// ELF Exists
	char *new_argv[new_argc];

	new_argv[0] = (char *)filename;
	for (i = 0; i < argc; i++) {
		new_argv[i + 1] = argv[i];
	}
	
	/* NB: LOADER.ELF is embedded  */
	boot_elf = (u8 *)loader_elf;
	eh = (elf_header_t *)boot_elf;
	if (_lw((u32)&eh->ident) != ELF_MAGIC)
		asm volatile("break\n");

	eph = (elf_pheader_t *)(boot_elf + eh->phoff);

	/* Scan through the ELF's program headers and copy them into RAM, then zero out any non-loaded regions.  */
	for (i = 0; i < eh->phnum; i++) {
		if (eph[i].type != ELF_PT_LOAD)
			continue;

		pdata = (void *)(boot_elf + eph[i].offset);
		memcpy(eph[i].vaddr, pdata, eph[i].filesz);

		if (eph[i].memsz > eph[i].filesz)
			memset(eph[i].vaddr + eph[i].filesz, 0, eph[i].memsz - eph[i].filesz);
	}

	/* Let's go.  */
	SifExitRpc();
	FlushCache(0);
	FlushCache(2);
	
	return ExecPS2((void *)eh->entry, NULL, new_argc, new_argv);
}
