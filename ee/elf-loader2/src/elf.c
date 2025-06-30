/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "elf_loader_common.h"

#include <kernel.h>
#include <string.h>

// Loader ELF variables
extern u8 ldrsrc[];
extern int size_ldrsrc;

int elf_loader_is_elf_ehdr_valid(const elf_loader_elf32_ehdr_t *ehdr)
{
	int ret;

	ret = 0;

	if ( memcmp(
				 ehdr->e_ident,
				 "\x7F"
				 "ELF",
				 4) )
	{
		ret = -9;
	}
	else if ( ehdr->e_entry < 0x80000 || ehdr->e_entry > (32 * 1024 * 1024) )
	{
		ret = -10;
	}
	else if ( ehdr->e_ident[4] != 1 )
	{
		ret = -1;
	}
	else if ( ehdr->e_ident[5] != 1 )
	{
		ret = -2;
	}
	else if ( ehdr->e_type != 2 )
	{
		ret = -3;
	}
	else if ( ehdr->e_machine != 8 )
	{
		ret = -4;
	}
	else if ( ehdr->e_ehsize != sizeof(elf_loader_elf32_ehdr_t) )
	{
		ret = -5;
	}
	else if ( ehdr->e_phentsize != sizeof(elf_loader_elf32_phdr_t) )
	{
		ret = -6;
	}
	else if ( ehdr->e_phnum > ELF_LOADER_MAX_PROGRAM_HEADERS )
	{
		ret = -7;
	}
	else if ( ehdr->e_shnum && ehdr->e_shentsize != 40 )
	{
		ret = -8;
	}
	return ret;
}

int elf_loader_is_elf_valid_for_loading(const void *buf, size_t buf_size)
{
	int i;
	int ret;
	int ph_count;
	int entrypoint_in_ph;
	u32 highest_vaddr;
	u32 highest_offset;
	const elf_loader_elf32_ehdr_t *ehdr;
	const elf_loader_elf32_phdr_t *phdr;

	if ( buf_size < sizeof(elf_loader_elf32_ehdr_t) )
		return -30;
	ehdr = (const elf_loader_elf32_ehdr_t *)buf;
	ret = elf_loader_is_elf_ehdr_valid(ehdr);
	if ( ret )
		return ret;
	if ( buf_size < (ehdr->e_phoff + sizeof(elf_loader_elf32_ehdr_t)) )
		return -31;
	phdr = (const elf_loader_elf32_phdr_t *)(((const u8 *)buf) + ehdr->e_phoff);
	ph_count = 0;
	entrypoint_in_ph = 0;
	highest_vaddr = 0x80000;
	highest_offset = 0;
	for ( i = 0; i < ehdr->e_phnum; i += 1 )
	{
		if ( phdr[i].p_type == 1 && phdr[i].p_filesz != 0 )
		{
			// Program headers need to be pre-sorted
			if ( phdr[i].p_vaddr < highest_vaddr || phdr[i].p_offset < highest_offset )
			{
				return -20;
			}
			highest_vaddr = phdr[i].p_vaddr;
			highest_offset = phdr[i].p_offset;
			if ( ehdr->e_entry >= phdr[i].p_vaddr && (phdr[i].p_vaddr + phdr[i].p_filesz) > ehdr->e_entry )
			{
				entrypoint_in_ph = 1;
			}
			ph_count += 1;
		}
	}
	if ( !ph_count || ph_count > ELF_LOADER_MAX_PROGRAM_HEADERS )
	{
		return -21;
	}
	if ( !entrypoint_in_ph )
	{
		return -22;
	}
	if ( buf_size < highest_offset )
		return -32;
	return 0;
}

int elf_loader_exec_elf_prepare_loadinfo(elf_loader_execinfo_t *execinfo, const void *buf, size_t buf_size)
{
	int ret;
	const elf_loader_elf32_ehdr_t *ehdr;
	const elf_loader_elf32_phdr_t *phdr;
	int i;
	u32 min_load_addr;
	u32 max_load_addr;
	u32 info_count;
	u32 memsize;

	ret = elf_loader_is_elf_valid_for_loading(buf, buf_size);
	if ( ret )
		return ret;
	ehdr = (const elf_loader_elf32_ehdr_t *)buf;
	phdr = (const elf_loader_elf32_phdr_t *)(((const u8 *)buf) + ehdr->e_phoff);
	memsize = GetMemorySize();

	min_load_addr = 0xFFFFFFFF;
	max_load_addr = 0x00100000;

	info_count = 0;
	for ( i = 0; i < ehdr->e_phnum; i += 1 )
	{
		if ( phdr[i].p_type == 1 && phdr[i].p_filesz != 0 )
		{
			// Item: move memory down
			execinfo->loaderinfo.items[info_count].dest_addr = (void *)(phdr[i].p_vaddr);
			execinfo->loaderinfo.items[info_count].src_addr = (void *)(((u8 *)buf) + phdr[i].p_offset);
			execinfo->loaderinfo.items[info_count].size = phdr[i].p_filesz;
			{
				u32 load_address;
				u32 end_load_address;
				load_address = phdr[i].p_vaddr;
				end_load_address = load_address + phdr[i].p_filesz;
				if ( load_address < min_load_addr )
				{
					min_load_addr = load_address;
				}
				if ( end_load_address > max_load_addr )
				{
					max_load_addr = end_load_address;
				}
			}
			if (
				(u8 *)(execinfo->loaderinfo.items[info_count].dest_addr)
				>= ((u8 *)execinfo->loaderinfo.items[info_count].src_addr) )
				break;
			info_count += 1;
		}
	}
	for ( i = ehdr->e_phnum - 1; i >= 0; i -= 1 )
	{
		if ( phdr[i].p_type == 1 && phdr[i].p_filesz != 0 )
		{
			// Item: move memory up
			execinfo->loaderinfo.items[info_count].dest_addr = (void *)(phdr[i].p_vaddr);
			execinfo->loaderinfo.items[info_count].src_addr = (void *)(((u8 *)buf) + phdr[i].p_offset);
			execinfo->loaderinfo.items[info_count].size = phdr[i].p_filesz;
			{
				u32 load_address;
				u32 end_load_address;
				load_address = phdr[i].p_vaddr;
				end_load_address = load_address + phdr[i].p_filesz;
				if ( load_address < min_load_addr )
				{
					min_load_addr = load_address;
				}
				if ( end_load_address > max_load_addr )
				{
					max_load_addr = end_load_address;
				}
			}
			if (
				(u8 *)(execinfo->loaderinfo.items[info_count].dest_addr)
				<= ((u8 *)execinfo->loaderinfo.items[info_count].src_addr) )
				break;
			info_count += 1;
		}
	}
	if ( min_load_addr != 0xFFFFFFFF )
	{
		u32 wanted_size;
		wanted_size = min_load_addr - 0x00100000;
		if ( wanted_size != 0 )
		{
			// Item: clear memory
			execinfo->loaderinfo.items[info_count].dest_addr = (void *)0x00100000;
			execinfo->loaderinfo.items[info_count].src_addr = (void *)NULL;
			execinfo->loaderinfo.items[info_count].size = wanted_size;
			info_count += 1;
		}
	}
	if ( max_load_addr != 0x00100000 )
	{
		u32 wanted_size;
		wanted_size = memsize - (uiptr)max_load_addr;
		// Item: clear memory
		if ( wanted_size != 0 )
		{
			execinfo->loaderinfo.items[info_count].dest_addr = (void *)max_load_addr;
			execinfo->loaderinfo.items[info_count].src_addr = (void *)NULL;
			execinfo->loaderinfo.items[info_count].size = wanted_size;
			info_count += 1;
		}
	}
	// Item: execute entry point
	execinfo->loaderinfo.items[info_count].dest_addr = (void *)(ehdr->e_entry);
	execinfo->loaderinfo.items[info_count].src_addr = (void *)NULL;  // GP returned is always NULL
	execinfo->loaderinfo.items[info_count].size = 0;

	return 0;
}

int elf_loader_exec_elf_prepare_arginfo(
	elf_loader_execinfo_t *execinfo, const char *filename, const char *partition, int argc, char *argv[])
{
	char *ptr;
	int len, i;

	ptr = execinfo->arginfo.payload;
	argc = (argc > 15) ? 15 : argc;
	execinfo->arginfo.argc = argc + 1;
	// Make pointer relative to payload, so that it can be relocated later
	execinfo->arginfo.argv[0] = (char *)((uiptr)ptr - (uiptr)(execinfo->arginfo.payload));
	if ( partition != NULL )
	{
		len = strlen(partition);  // Not including NULL terminator
		if ( ((ptr - execinfo->arginfo.payload) + len) > sizeof(execinfo->arginfo.payload) )
			return -40;
		memcpy(ptr, partition, len);
		ptr += len;
	}
	len = strlen(filename) + 1;
	if ( ((ptr - execinfo->arginfo.payload) + len) > sizeof(execinfo->arginfo.payload) )
		return -40;
	memcpy(ptr, filename, len);
	ptr += len;
	for ( i = 0; i < argc; i += 1 )
	{
		execinfo->arginfo.argv[i + 1] = (char *)((uiptr)ptr - (uiptr)(execinfo->arginfo.payload));
		len = strlen(argv[i]) + 1;
		if ( ((ptr - execinfo->arginfo.payload) + len) > sizeof(execinfo->arginfo.payload) )
			return -40;
		memcpy(ptr, argv[i], len);
		ptr += len;
	}
	return 0;
}

static void elf_loader_ldr_entrypoint_stack();

static char args_storage[1024];
static elf_loader_loaderinfo_t sg_loaderinfo;

// Modified crt0
// bss zero is skipped, and argument handling removed
static void elf_loader_ldr_entrypoint(void)
{
	// clang-format off
    __asm__ __volatile__(
        // SetupThread
        // Arg0: gp value (NULL since not using small data)
        "move $4, $0" "\n"
        // Arg1: Stack address (at EE scratchpad)
        "move $5, $0" "\n"
        "lui $5, 0x7000" "\n"
        // Arg2: Stack size (16 KB)
        "move $6, $0" "\n"
        "ori $6, $6, 0x4000" "\n"
        // Arg3: args buffer (needs to not be NULL since kernel unconditionally writes argc to it)
        "la     $7, %0          \n"
        // Arg4: root function
        "la     $8, ExitThread  \n"
        "move   $gp, $4         \n"
        "addiu  $3, $0, 60      \n"
        "syscall                \n"
        "move   $sp, $2         \n"
        // Jump to entrypoint that can use stack
        "j      %1           \n"
        : // No output registers
        : "R"(args_storage), "Csy"(elf_loader_ldr_entrypoint_stack));
	// clang-format on
}

typedef void *(*ldr_getinternalinfo_callback)(void);
#define LDR_ENTRYPOINT_ADDR 0x00084000

static void elf_loader_ldr_entrypoint_stack()
{
	ldr_getinternalinfo_callback cb;

	// Use VU1 data memory as heap
	SetupHeap((void *)0x1100C000, 0x4000);

	FlushCache(0);

	memcpy((void *)LDR_ENTRYPOINT_ADDR, ldrsrc, size_ldrsrc);
	// Use VU0 data memory as storage for loader information
	memcpy((void *)0x11004000, &sg_loaderinfo, sizeof(elf_loader_loaderinfo_t));

	FlushCache(0);
	FlushCache(2);

	cb = (void *)LDR_ENTRYPOINT_ADDR;
	cb();
	// Should be unreachable here
	__builtin_trap();
}

int elf_loader_exec_elf(elf_loader_execinfo_t *execinfo)
{
	elf_loader_arginfo_t *low_arginfo;
	int i;

	if ( execinfo->arginfo.argc == 0 )
		return -1;
	memcpy(&sg_loaderinfo, &(execinfo->loaderinfo), sizeof(elf_loader_loaderinfo_t));
	// Copy argument info to low EE memory (because EE wipes VU0, VU1, or SPR memory before reading it)
	low_arginfo = (void *)0x00088000;
	memcpy((void *)low_arginfo, &(execinfo->arginfo), sizeof(elf_loader_execinfo_t));
	// Relocate the arguments to the new address
	for ( i = 0; i < low_arginfo->argc; i += 1 )
	{
		low_arginfo->argv[i] = (char *)((uiptr)(low_arginfo->argv[i]) + (uiptr)(low_arginfo->payload));
	}
	ExecPS2(&elf_loader_ldr_entrypoint, NULL, 0, NULL);
	return -1;
}
