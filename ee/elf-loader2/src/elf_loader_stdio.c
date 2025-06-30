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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct elf_loader_reader_info_stdio_
{
	FILE *m_fp;
	elf_loader_reader_info_t m_reader_info;
	elf_loader_reader_info_stdio_resultbuf_t m_resultbuf;
} elf_loader_reader_info_stdio_t;

static void *elf_loader_reader_read_elf_file_stdlib_allocation_callback(
	void *userdata, void *pointer, ptrdiff_t old_size, ptrdiff_t new_size)
{
	void *new_ptr;
	(void)userdata;
	(void)old_size;

	if ( !new_size )
	{
		free(pointer);
		return NULL;
	}
	new_ptr = realloc(pointer, new_size);
	if ( !new_ptr )
	{
		free(pointer);
	}
	return new_ptr;
}

static int elf_loader_reader_read_elf_file_stdio_read_callback(
	void *userdata, elf_loader_reader_stage_t stage, const elf_loader_reader_segment_info_t *segm_info, size_t segm_count)
{
	elf_loader_reader_info_stdio_t *procinfo;
	size_t i;

	procinfo = (elf_loader_reader_info_stdio_t *)userdata;
	if ( !procinfo->m_fp )
		return 0;
	for ( i = 0; i < segm_count; i += 1 )
	{
		if ( fseek(procinfo->m_fp, segm_info[i].m_segment_offset, SEEK_SET) )
		{
			return 0;
		}
		if ( ftell(procinfo->m_fp) != segm_info[i].m_segment_offset )
		{
			return 0;
		}
		if ( fread(segm_info[i].m_segment_addr, segm_info[i].m_segment_size, 1, procinfo->m_fp) != 1 )
		{
			return 0;
		}
	}
	return 1;
}

static void
elf_loader_reader_read_elf_file_stdio_result_callback(void *userdata, void *pointer, ptrdiff_t pointer_size, int errval)
{
	elf_loader_reader_info_stdio_t *procinfo;

	procinfo = (elf_loader_reader_info_stdio_t *)userdata;
	procinfo->m_resultbuf.m_buf = pointer;
	procinfo->m_resultbuf.m_bufsize = pointer_size;
	procinfo->m_resultbuf.m_result = errval;
}

int elf_loader_reader_read_elf_file_stdio(
	elf_loader_reader_info_stdio_resultbuf_t *resultbuf, const char *filename, const char *flags)
{
	elf_loader_reader_info_stdio_t procinfo;

	procinfo.m_reader_info.m_userdata = &procinfo;
	procinfo.m_reader_info.m_alloc_callback = elf_loader_reader_read_elf_file_stdlib_allocation_callback;
	procinfo.m_reader_info.m_read_callback = elf_loader_reader_read_elf_file_stdio_read_callback;
	procinfo.m_reader_info.m_result_callback = elf_loader_reader_read_elf_file_stdio_result_callback;
	procinfo.m_fp = fopen(filename, flags);
	if ( !procinfo.m_fp )
		return -30;
	elf_loader_reader_read_elf_file(&procinfo.m_reader_info);
	if ( procinfo.m_fp )
	{
		fclose(procinfo.m_fp);
		procinfo.m_fp = NULL;
	}
	if ( resultbuf )
		memcpy(resultbuf, &procinfo.m_resultbuf, sizeof(elf_loader_reader_info_stdio_resultbuf_t));
	return procinfo.m_resultbuf.m_result;
}
