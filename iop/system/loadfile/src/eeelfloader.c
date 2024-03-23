/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include <kerr.h>

// NOTE: The following heap related functions have been simplified compared to the original LOADFILE functions.
// It has been changed to be basically a stack allocator

static void *heap_buffer_base = NULL;
static void *heap_buffer = NULL;
static void *heap_buffer_end = NULL;

static int *allocate_heap_buffer(unsigned int lower_bound, unsigned int upper_bound)
{
	unsigned int upper_bound_rounded;

	(void)lower_bound;

	upper_bound_rounded = upper_bound;
	// Align to 4 bytes
	if ( (upper_bound_rounded & 3) != 0 )
	{
		upper_bound_rounded += (4 - (upper_bound_rounded & 3));
	}
	CpuDisableIntr();
	heap_buffer_base = AllocSysMemory(ALLOC_LAST, upper_bound_rounded, NULL);
	CpuEnableIntr();
	if ( heap_buffer_base != NULL )
	{
		heap_buffer = heap_buffer_base;
		heap_buffer_end = &(((u8 *)heap_buffer_base)[upper_bound_rounded]);
		return heap_buffer_base;
	}
	printf("memory allocation failed.\n");
	return NULL;
}

static void *elf_load_alloc_buffer_from_heap(u32 alloc_size)
{
	u32 alloc_size_rounded;

	alloc_size_rounded = alloc_size;
	// Align to 4 bytes
	if ( (alloc_size & 3) != 0 )
	{
		alloc_size_rounded += (4 - (alloc_size & 3));
	}
	// Validity check...
	{
		u8 *new_ptr;

		new_ptr = heap_buffer;
		new_ptr += 4;
		new_ptr += alloc_size_rounded;
		if ( (u8 *)new_ptr > (u8 *)heap_buffer_end )
		{
			return NULL;
		}
	}
	// Do the allocation
	{
		u8 *new_ptr;
		u8 *ret_ptr;

		new_ptr = heap_buffer;

		((u32 *)new_ptr)[0] = alloc_size_rounded;
		new_ptr += sizeof(alloc_size_rounded);

		ret_ptr = new_ptr;
		new_ptr += alloc_size_rounded;

		heap_buffer = new_ptr;
		return ret_ptr;
	}
}

static void elf_load_dealloc_buffer_from_heap(void *alloc_memory)
{
	u32 chunk_size;
	// Check if NULL
	if ( alloc_memory == NULL )
	{
		return;
	}
	// Check if in range of heap buffer
	if ( (((u8 *)alloc_memory) - 4 < ((u8 *)heap_buffer_base)) || ((u8 *)alloc_memory) >= ((u8 *)heap_buffer_end) )
	{
		return;
	}
	// Get size
	chunk_size = *((u32 *)(((u8 *)alloc_memory) - 4));
	// Check if this is the current heap position
	if ( ((u8 *)alloc_memory) + chunk_size != ((u8 *)heap_buffer) )
	{
		return;
	}
	// Do the deallocation
	heap_buffer = ((u8 *)alloc_memory) - 4;
}

typedef struct loadfile_elf_program_header_size_offset_
{
	int index_of_ph_contents;
	int offset_of_ph_contents;
} loadfile_elf_program_header_size_offset_t;

static int check_elf_header(const loadfile_elf32_ehdr_t **pehdr)
{
	const loadfile_elf32_ehdr_t *ehdr;

	ehdr = *pehdr;
	if ( ehdr->e_ident[0] != '\x7F' )
	{
		return -1;
	}
	if ( ehdr->e_ident[1] != 'E' )
	{
		return -1;
	}
	if ( ehdr->e_ident[2] != 'L' )
	{
		return -1;
	}
	if ( ehdr->e_ident[3] != 'F' )
	{
		return -1;
	}
	return 0;
}

static int check_elf_architecture(const loadfile_file_load_handler_struct_t *flhs)
{
	if ( flhs->elf_header.e_ident[4] != 1 )
	{
		return -1;
	}
	if ( flhs->elf_header.e_ident[5] != 1 )
	{
		return -2;
	}
	if ( flhs->elf_header.e_type != 2 )
	{
		return -3;
	}
	if ( flhs->elf_header.e_machine != 8 )
	{
		return -4;
	}
	if ( flhs->elf_header.e_ehsize != 52 )
	{
		return -5;
	}
	if ( flhs->elf_header.e_phentsize != 32 )
	{
		return -6;
	}
	if ( !flhs->elf_header.e_shnum )
	{
		return 1;
	}
	if ( flhs->elf_header.e_shentsize != 40 )
	{
		return -7;
	}
	return 1;
}

static int
check_valid_ee_elf(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs)
{
	const loadfile_elf32_ehdr_t **pehdr;
	int hdrchkres;
	loadfile_elf32_ehdr_t *ehdr;

	pehdr = (const loadfile_elf32_ehdr_t **)&allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
	hdrchkres = check_elf_header(pehdr);
	ehdr = &flhs->elf_header;
	if ( hdrchkres < 0 )
	{
		printf("File is not ELF format(%d)\n", hdrchkres);
		return KE_ILLEGAL_OBJECT;
	}
	memcpy(ehdr, *pehdr, sizeof(*ehdr));
	hdrchkres = check_elf_architecture(flhs);
	if ( hdrchkres < 0 )
	{
		printf("File is not for target architecture(%d)\n", hdrchkres);
		return KE_ILLEGAL_OBJECT;
	}
	return 0;
}

static int
elf_get_program_header(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs)
{
	int totalphsize;
	u8 *alloc_buffer_from_heap;

	totalphsize = flhs->elf_header.e_phentsize * flhs->elf_header.e_phnum;
	alloc_buffer_from_heap = elf_load_alloc_buffer_from_heap(totalphsize);
	flhs->program_header = (loadfile_elf32_phdr_t *)alloc_buffer_from_heap;
	if ( alloc_buffer_from_heap == NULL )
	{
		return KE_NO_MEMORY;
	}
	memcpy(
		alloc_buffer_from_heap,
		&allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index].buffer_base[flhs->elf_header.e_phoff],
		totalphsize);
	return 0;
}

static int elf_read_header_section_headers(loadfile_file_load_handler_struct_t *flhs)
{
	int totahshsize;
	u8 *alloc_buffer_from_heap;

	lseek(flhs->fd, flhs->elf_header.e_shoff, 0);
	totahshsize = flhs->elf_header.e_shentsize * flhs->elf_header.e_shnum;
	alloc_buffer_from_heap = elf_load_alloc_buffer_from_heap(totahshsize);
	flhs->section_headers = (loadfile_elf32_shdr_t *)alloc_buffer_from_heap;
	if ( alloc_buffer_from_heap == NULL )
	{
		return KE_NO_MEMORY;
	}
	if ( read(flhs->fd, alloc_buffer_from_heap, totahshsize) != totahshsize )
	{
		elf_load_dealloc_buffer_from_heap(flhs->section_headers);
		flhs->section_headers = 0;
		return KE_FILEERR;
	}
	return 0;
}

static int elf_read_section_contents(loadfile_file_load_handler_struct_t *flhs)
{
	loadfile_elf32_shdr_t *shdr;
	u8 *alloc_buffer_from_heap;

	shdr = &flhs->section_headers[flhs->elf_header.e_shstrndx];
	if ( shdr->sh_type != 3 || shdr->sh_flags != 0 || shdr->sh_link != 0 || shdr->sh_info != 0 )
	{
		flhs->section_contents = 0;
		return KE_ILLEGAL_OBJECT;
	}
	alloc_buffer_from_heap = elf_load_alloc_buffer_from_heap(shdr->sh_size);
	flhs->section_contents = alloc_buffer_from_heap;
	if ( alloc_buffer_from_heap == NULL )
	{
		return KE_NO_MEMORY;
	}
	lseek(flhs->fd, shdr->sh_offset, 0);
	if ( (u32)(read(flhs->fd, flhs->section_contents, shdr->sh_size)) != shdr->sh_size )
	{
		elf_load_dealloc_buffer_from_heap(flhs->section_contents);
		flhs->section_contents = 0;
		return KE_FILEERR;
	}
	return 0;
}

static void sort_ph_contents(int ph_count, loadfile_elf_program_header_size_offset_t *ph_size_offset_data)
{
	int ph_i1;
	loadfile_elf_program_header_size_offset_t *szo1;

	ph_i1 = 1;
	szo1 = ph_size_offset_data + 1;
	while ( ph_i1 < ph_count )
	{
		int ph_i2;
		loadfile_elf_program_header_size_offset_t *szo2;
		int index_of_ph_contents;
		unsigned int offset_of_ph_contents;
		loadfile_elf_program_header_size_offset_t *szo3;

		index_of_ph_contents = szo1->index_of_ph_contents;
		offset_of_ph_contents = szo1->offset_of_ph_contents;
		ph_i2 = ph_i1 - 1;
		szo3 = &ph_size_offset_data[ph_i2];
		while ( ph_i2 >= 0 && offset_of_ph_contents < (u32)(szo3->offset_of_ph_contents) )
		{
			szo3[1].index_of_ph_contents = szo3->index_of_ph_contents;
			szo3[1].offset_of_ph_contents = szo3->offset_of_ph_contents;
			ph_i2 -= 1;
			szo3 -= 1;
		}
		szo2 = &ph_size_offset_data[ph_i2];
		szo2[1].index_of_ph_contents = index_of_ph_contents;
		szo2[1].offset_of_ph_contents = offset_of_ph_contents;
		ph_i1 += 1;
		szo1 += 1;
	}
}

static int fileio_reader_function(int fd, loadfile_allocate_handler_struct_t *allocate_info, void *userdata)
{
	int read_buffer_offset;
	loadfile_ee_elf_ringbuffer_content_t *rbc;
	int read_res;

	(void)userdata;

	read_buffer_offset = allocate_info->read_buffer_offset;
	rbc = &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
	rbc->buffer_offset = read_buffer_offset;
	read_res = read(fd, rbc->buffer_base, allocate_info->read_buffer_length);
	rbc->buffer_length = read_res;
	allocate_info->read_buffer_offset += read_res;
	return 0;
}

static int elf_load_proc(
	loadfile_allocate_handler_struct_t *allocate_info,
	loadfile_file_load_handler_struct_t *flhs,
	void *read_callback_userdata,
	loadfile_read_chunk_callback_t read_callback)
{
	int ph_i2;
	loadfile_ee_elf_ringbuffer_content_t *rbc;
	unsigned int sh_offset_total;
	unsigned int sh_size_for_offset;
	int sh_size_for_alignment;
	SifDmaTransfer_t dmat;
	loadfile_elf_program_header_size_offset_t phso[32];
	int state;
	int ph_count;

	if ( flhs->program_header == NULL )
	{
		return KE_ILLEGAL_OBJECT;
	}
	ph_count = 0;
	{
		int i;

		for ( i = 0; i < flhs->elf_header.e_phnum; i += 1 )
		{
			const loadfile_elf32_phdr_t *phdr1;

			phdr1 = &flhs->program_header[i];
			if ( phdr1->p_type == 1 && phdr1->p_filesz )
			{
				phso[ph_count].index_of_ph_contents = i;
				phso[ph_count].offset_of_ph_contents = phdr1->p_offset;
				ph_count += 1;
			}
		}
	}
	sort_ph_contents(ph_count, phso);
	printf("Input ELF format filename = %s\n", flhs->filename);
	for ( ph_i2 = 0; ph_i2 < ph_count; ph_i2 += 1 )
	{
		int total_offset;
		int index_of_ph_contents;
		const loadfile_elf32_phdr_t *phdr2;
		unsigned int sh_size_cur;

		index_of_ph_contents = phso[ph_i2].index_of_ph_contents;
		phdr2 = &flhs->program_header[index_of_ph_contents];
		sh_size_cur = phdr2->p_filesz;
		printf("%d %08x %08x ", index_of_ph_contents, phdr2->p_vaddr, sh_size_cur);
		total_offset = 0;
		while ( sh_size_cur != 0 )
		{
			printf(".");
			rbc = &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
			while ( sceSifDmaStat(rbc->dma_handle) >= 0 )
				;
			sh_offset_total = phdr2->p_offset + total_offset;
			while ( sh_offset_total >= (u32)(allocate_info->read_buffer_offset) )
			{
				if ( read_callback(flhs->fd, allocate_info, read_callback_userdata) != 0 )
				{
					return KE_FILEERR;
				}
			}
			sh_size_for_offset = sh_offset_total - rbc->buffer_offset;
			sh_size_for_alignment = sh_size_cur;
			if ( rbc->buffer_length - sh_size_for_offset < sh_size_cur )
				sh_size_for_alignment = rbc->buffer_length - sh_size_for_offset;
			dmat.src = &rbc->buffer_base[sh_size_for_offset];
			sh_size_cur -= sh_size_for_alignment;
			dmat.size = sh_size_for_alignment;
			dmat.attr = 0;
			dmat.dest = (void *)((phdr2->p_vaddr) + total_offset);
			total_offset += sh_size_for_alignment;
			CpuSuspendIntr(&state);
			rbc->dma_handle = sceSifSetDma(&dmat, 1);
			CpuResumeIntr(state);
			allocate_info->ring_buffer_index = (allocate_info->ring_buffer_index + 1) & 1;
		}
		printf("\n");
	}
	printf("Loaded, %s\n", flhs->filename);
	return 0;
}

static int elf_load_single_section(
	loadfile_allocate_handler_struct_t *allocate_info,
	loadfile_file_load_handler_struct_t *flhs,
	int epc,
	const char *section_name)
{
	loadfile_elf32_shdr_t *shdr;
	int result;
	int total_offset;
	unsigned int sh_size;
	loadfile_ee_elf_ringbuffer_content_t *rbc;
	SifDmaTransfer_t dmat;
	int state;

	(void)epc;

	shdr = NULL;
	if ( read(flhs->fd, &flhs->elf_header, 0x34) != 0x34 )
	{
		return KE_FILEERR;
	}
	result = elf_read_header_section_headers(flhs);
	if ( result < 0 )
	{
		return result;
	}
	result = elf_read_section_contents(flhs);
	if ( result < 0 )
	{
		return result;
	}
	{
		int i;

		for ( i = 0; i < flhs->elf_header.e_shnum; i += 1 )
		{
			shdr = &flhs->section_headers[i];
			if ( !strcmp((const char *)&flhs->section_contents[shdr->sh_name], section_name) && shdr->sh_size )
				break;
			shdr = NULL;
		}
	}
	if ( shdr == NULL || shdr->sh_addr < 0x80000 )
	{
		return KE_ILLEGAL_OBJECT;
	}
	total_offset = 0;
	lseek(flhs->fd, shdr->sh_offset, 0);
	allocate_info->read_buffer_offset = shdr->sh_offset;
	sh_size = shdr->sh_size;
	printf("%s: %08x %08x ", section_name, shdr->sh_addr, sh_size);
	while ( sh_size != 0 )
	{
		loadfile_allocate_handler_struct_t *i;
		unsigned int sh_offset_total;
		unsigned int sh_size_for_offset;
		int sh_size_for_alignment;

		printf(".");
		rbc = &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
		while ( sceSifDmaStat(rbc->dma_handle) >= 0 )
			;
		sh_offset_total = shdr->sh_offset + total_offset;
		for ( i = allocate_info; sh_offset_total >= (u32)(allocate_info->read_buffer_offset); i = allocate_info )
			fileio_reader_function(flhs->fd, i, 0);
		sh_size_for_offset = sh_offset_total - rbc->buffer_offset;
		sh_size_for_alignment = sh_size;
		if ( rbc->buffer_length - sh_size_for_offset < sh_size )
			sh_size_for_alignment = rbc->buffer_length - sh_size_for_offset;
		dmat.src = &rbc->buffer_base[sh_size_for_offset];
		sh_size -= sh_size_for_alignment;
		dmat.size = sh_size_for_alignment;
		dmat.attr = 0;
		dmat.dest = (void *)((shdr->sh_addr) + total_offset);
		CpuSuspendIntr(&state);
		total_offset += sh_size_for_alignment;
		rbc->dma_handle = sceSifSetDma(&dmat, 1);
		CpuResumeIntr(state);
		allocate_info->ring_buffer_index = (allocate_info->ring_buffer_index + 1) & 1;
	}
	printf("\nLoaded, %s:%s\n", flhs->filename, section_name);
	return 0;
}

int elf_load_all_section(
	loadfile_allocate_handler_struct_t *allocate_info,
	loadfile_file_load_handler_struct_t *flhs,
	int *result_out,
	int *result_module_out)
{
	int result;

	fileio_reader_function(flhs->fd, allocate_info, 0);
	result = check_valid_ee_elf(allocate_info, flhs);
	if ( result < 0 )
	{
		return result;
	}
	result = elf_get_program_header(allocate_info, flhs);
	if ( result < 0 )
	{
		return result;
	}
	{
		int i;

		for ( i = 0; i < flhs->elf_header.e_phnum; i += 1 )
		{
			const loadfile_elf32_phdr_t *phdr1;

			phdr1 = &(flhs->program_header[i]);
			if ( phdr1->p_type == 1 && phdr1->p_filesz && phdr1->p_vaddr < 0x80000 )
			{
				return KE_ILLEGAL_OBJECT;
			}
		}
	}
	result = elf_load_proc(allocate_info, flhs, 0, fileio_reader_function);
	if ( result < 0 )
	{
		return result;
	}
	*result_out = flhs->elf_header.e_entry;
	*result_module_out = 0;
	printf("start address %#08x\n", *result_out);
	printf("gp address %#08x\n", *result_module_out);
	return 0;
}

static void empty_loadfile_information(loadfile_file_load_handler_struct_t *flhs)
{
	flhs->fd = -1;
	flhs->filename = 0;
	flhs->section_contents = 0;
	flhs->unknown_0C = 0;
	flhs->program_header = 0;
	flhs->section_headers = 0;
	flhs->unknown_4C = 0;
}

static int elf_load_common(
	const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out, int is_mg_elf)
{
	int *heap_buffer_cur;
	int bufsz;
	int bufsz_divisor;
	char *read_buffer;
	int result;
	loadfile_allocate_handler_struct_t allocate_info;
	loadfile_file_load_handler_struct_t flh;
	SetLoadfileCallbacks_struct_t loadfile_functions;
	CheckKelfPath_callback_t CheckKelfPath_fnc;
	SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_fnc;
	int card_port;
	int card_slot;

	printf("%s", "loadelf version 3.30\n");
	empty_loadfile_information(&flh);
	flh.filename = filename;
	flh.fd = open(filename, 1);
	if ( flh.fd < 0 )
	{
		printf("Cannot openfile\n");
		result = KE_NOFILE;
		goto finish_returnresult;
	}
	heap_buffer_cur = allocate_heap_buffer(0, 0x10000);
	bufsz = 0x20000;
	if ( heap_buffer_cur == NULL )
	{
		printf("Error Can't Get heap buffer\n");
		result = KE_NO_MEMORY;
		goto finish_closefd;
	}
	bufsz_divisor = 0;
	read_buffer = NULL;
	while ( read_buffer == NULL )
	{
		CpuDisableIntr();
		read_buffer = (char *)AllocSysMemory(ALLOC_LAST, bufsz, NULL);
		CpuEnableIntr();
		if ( read_buffer )
			break;
		bufsz /= 2;
		bufsz_divisor += 1;
		if ( bufsz_divisor >= 8 )
		{
			printf("Error Can't Get read buffer\n");
			result = KE_NO_MEMORY;
			goto finish_freeheapbuffer;
		}
	}
	allocate_info.ring_buffer_index = 0;
	allocate_info.read_buffer_length = (unsigned int)bufsz >> 1;
	allocate_info.read_buffer_offset = 0;
	{
		int i;

		for ( i = 0; i < 2; i += 1 )
		{
			allocate_info.ring_buffer_contents[i].buffer_offset = 0;
			allocate_info.ring_buffer_contents[i].buffer_length = 0;
			allocate_info.ring_buffer_contents[i].dma_handle = 0;
			allocate_info.ring_buffer_contents[i].buffer_base = (u8 *)&read_buffer[allocate_info.read_buffer_length * i];
		}
	}
	result = KE_ILLEGAL_OBJECT;
	if ( is_mg_elf )
	{
		GetLoadfileCallbacks(&CheckKelfPath_fnc, &SetLoadfileCallbacks_fnc);
		if ( SetLoadfileCallbacks_fnc )
		{
			loadfile_functions.elf_load_proc = elf_load_proc;
			loadfile_functions.check_valid_ee_elf = check_valid_ee_elf;
			loadfile_functions.elf_get_program_header = elf_get_program_header;
			loadfile_functions.elf_load_alloc_buffer_from_heap = elf_load_alloc_buffer_from_heap;
			loadfile_functions.elf_load_dealloc_buffer_from_heap = elf_load_dealloc_buffer_from_heap;
			SetLoadfileCallbacks_fnc(&loadfile_functions);
			if ( CheckKelfPath_fnc && CheckKelfPath_fnc(filename, &card_port, &card_slot) )
			{
				if ( loadfile_functions.load_kelf_from_card )
				{
					result = loadfile_functions.load_kelf_from_card(
						&allocate_info, &flh, card_port, card_slot, result_out, result_module_out);
				}
			}
			else if ( loadfile_functions.load_kelf_from_disk )
			{
				result = loadfile_functions.load_kelf_from_disk(&allocate_info, &flh, result_out, result_module_out);
			}
		}
	}
	else if ( !strcmp(section_name, "all") )
	{
		result = elf_load_all_section(&allocate_info, &flh, result_out, result_module_out);
	}
	else
	{
		result = elf_load_single_section(&allocate_info, &flh, epc, section_name);
	}
	FreeSysMemory(read_buffer);
finish_freeheapbuffer:
	FreeSysMemory(heap_buffer_cur);
finish_closefd:
	close(flh.fd);
finish_returnresult:
	return result;
}

int loadfile_elfload_innerproc(
	const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out)
{
	if ( IsIllegalBootDevice(filename) != 0 )
	{
		return KE_ILLEGAL_OBJECT;
	}
	return elf_load_common(filename, epc, section_name, result_out, result_module_out, 0);
}

int loadfile_mg_elfload_proc(
	const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out)
{
	if ( strcmp(section_name, "all") != 0 )
	{
		return KE_ILLEGAL_MODE;
	}
	return elf_load_common(filename, epc, section_name, result_out, result_module_out, 1);
}
