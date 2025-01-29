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

#include <string.h>

void elf_loader_reader_read_elf_file(elf_loader_reader_info_t *info)
{
	void *ptr;
	int ret;
	ptrdiff_t cur_alloc_block_size;

	ret = -10;
	cur_alloc_block_size = 0;
	ptr = info->m_alloc_callback(
		info->m_userdata,
		NULL,
		0,
		sizeof(elf_loader_elf32_ehdr_t) + (sizeof(elf_loader_elf32_phdr_t) * ELF_LOADER_MAX_PROGRAM_HEADERS));
	if ( ptr )
	{
		elf_loader_reader_segment_info_t elf_segment_info[1];

		ret = -11;
		cur_alloc_block_size =
			sizeof(elf_loader_elf32_phdr_t) + (sizeof(elf_loader_elf32_phdr_t) * ELF_LOADER_MAX_PROGRAM_HEADERS);
		elf_segment_info[0].m_segment_addr = ptr;
		elf_segment_info[0].m_segment_offset = 0;
		elf_segment_info[0].m_segment_size = cur_alloc_block_size;

		if ( !info->m_read_callback(
					 info->m_userdata,
					 ELF_LOADER_READER_STAGE_ELF_HEADER,
					 &elf_segment_info[0],
					 sizeof(elf_segment_info) / sizeof(elf_segment_info[0])) )
			ptr = info->m_alloc_callback(info->m_userdata, ptr, cur_alloc_block_size, 0);
	}
	if ( ptr )
	{
		int ret;

		ret = elf_loader_is_elf_ehdr_valid(ptr);
		if ( ret )
			ptr = info->m_alloc_callback(info->m_userdata, ptr, cur_alloc_block_size, 0);
	}
	if ( ptr )
	{
		elf_loader_elf32_ehdr_t *elf_header;
		elf_loader_reader_segment_info_t elf_segment_info[1];
		u32 phoff;

		ret = -12;
		elf_header = (void *)ptr;
		elf_header->e_shoff = 0;
		elf_header->e_shentsize = 0;
		elf_header->e_shnum = 0;
		elf_header->e_shstrndx = 0;
		phoff = elf_header->e_phoff;
		elf_header->e_phoff = sizeof(elf_loader_elf32_ehdr_t);

		elf_segment_info[0].m_segment_addr = ((u8 *)ptr) + elf_header->e_phoff;
		elf_segment_info[0].m_segment_offset = phoff;
		elf_segment_info[0].m_segment_size = sizeof(elf_loader_elf32_phdr_t) * elf_header->e_phnum;

		if ( !info->m_read_callback(
					 info->m_userdata,
					 ELF_LOADER_READER_STAGE_PROGRAM_HEADERS,
					 &elf_segment_info[0],
					 sizeof(elf_segment_info) / sizeof(elf_segment_info[0])) )
			ptr = info->m_alloc_callback(info->m_userdata, ptr, cur_alloc_block_size, 0);
	}
	if ( ptr )
	{
		elf_loader_elf32_ehdr_t *elf_header;
		elf_loader_elf32_phdr_t *elf_pheader;
		int i;
		u32 offs_sum;

		ret = -13;
		elf_header = (void *)ptr;
		elf_pheader = (void *)(((u8 *)ptr) + elf_header->e_phoff);
		offs_sum = 0;
		// Remove any unused program headers
		for ( i = 0; i < elf_header->e_phnum; i += 1 )
		{
			while ( i < elf_header->e_phnum && (elf_pheader[i].p_type != 1 || elf_pheader[i].p_filesz == 0) )
			{
				elf_header->e_phnum -= 1;
				memmove(&elf_pheader[i], &elf_pheader[i + 1], sizeof(elf_loader_elf32_phdr_t) * (elf_header->e_phnum - i));
			}
		}
		// Sort program headers by address
		for ( i = 1; i < elf_header->e_phnum; i += 1 )
		{
			int j;
			elf_loader_elf32_phdr_t phdr_tmp;

			memcpy(&phdr_tmp, &elf_pheader[i], sizeof(elf_loader_elf32_phdr_t));
			for ( j = i - 1; j >= 0 && phdr_tmp.p_vaddr < elf_pheader[j].p_vaddr; j -= 1 )
			{
				memcpy(&elf_pheader[j + 1], &elf_pheader[j], sizeof(elf_loader_elf32_phdr_t));
			}
			memcpy(&elf_pheader[j + 1], &phdr_tmp, sizeof(elf_loader_elf32_phdr_t));
		}
		offs_sum = sizeof(elf_loader_elf32_ehdr_t) + (sizeof(elf_loader_elf32_phdr_t) * elf_header->e_phnum);
		for ( i = 0; i < elf_header->e_phnum; i += 1 )
		{
			offs_sum += elf_pheader[i].p_filesz;
		}
		ptr = info->m_alloc_callback(info->m_userdata, ptr, cur_alloc_block_size, offs_sum);
		cur_alloc_block_size = offs_sum;
	}
	if ( ptr )
	{
		elf_loader_elf32_ehdr_t *elf_header;
		elf_loader_elf32_phdr_t *elf_pheader;
		elf_loader_reader_segment_info_t elf_segment_info[ELF_LOADER_MAX_PROGRAM_HEADERS];
		int i;
		u32 offs_sum;

		ret = -14;
		elf_header = (void *)ptr;
		elf_pheader = (void *)(((u8 *)ptr) + elf_header->e_phoff);
		offs_sum = sizeof(elf_loader_elf32_ehdr_t) + (sizeof(elf_loader_elf32_phdr_t) * elf_header->e_phnum);
		for ( i = 0; i < elf_header->e_phnum; i += 1 )
		{
			elf_segment_info[i].m_segment_addr = ((u8 *)ptr) + offs_sum;
			elf_segment_info[i].m_segment_offset = elf_pheader[i].p_offset;
			elf_segment_info[i].m_segment_size = elf_pheader[i].p_filesz;
			elf_pheader[i].p_offset = offs_sum;
			offs_sum += elf_segment_info[i].m_segment_size;
		}
		// Sort segment info by offset in file
		for ( i = 1; i < elf_header->e_phnum; i += 1 )
		{
			int j;
			elf_loader_reader_segment_info_t segment_info_tmp;

			memcpy(&segment_info_tmp, &elf_segment_info[i], sizeof(elf_loader_reader_segment_info_t));
			for ( j = i - 1; j >= 0 && segment_info_tmp.m_segment_offset < elf_segment_info[j].m_segment_offset; j -= 1 )
			{
				memcpy(&elf_segment_info[j + 1], &elf_segment_info[j], sizeof(elf_loader_reader_segment_info_t));
			}
			memcpy(&elf_segment_info[j + 1], &segment_info_tmp, sizeof(elf_loader_reader_segment_info_t));
		}
		if ( !info->m_read_callback(
					 info->m_userdata, ELF_LOADER_READER_STAGE_SEGMENTS, &elf_segment_info[0], elf_header->e_phnum) )
			ptr = info->m_alloc_callback(info->m_userdata, ptr, cur_alloc_block_size, 0);
	}
	if ( ptr )
	{
		ret = 0;
	}
	info->m_result_callback(info->m_userdata, ptr, cur_alloc_block_size, ret);
}
