/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __ELF_LOADER_COMMON_H__
#define __ELF_LOADER_COMMON_H__

#include <elf-loader.h>
#include <stddef.h>
#include <tamtypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ELF_LOADER_MAX_PROGRAM_HEADERS 32

	typedef struct elf_loader_elf32_ehdr_
	{
		u8 e_ident[16];
		u16 e_type;
		u16 e_machine;
		u32 e_version;
		u32 e_entry;
		u32 e_phoff;
		u32 e_shoff;
		u32 e_flags;
		u16 e_ehsize;
		u16 e_phentsize;
		u16 e_phnum;
		u16 e_shentsize;
		u16 e_shnum;
		u16 e_shstrndx;
	} elf_loader_elf32_ehdr_t;

	typedef struct elf_loader_elf32_phdr_
	{
		u32 p_type;
		u32 p_offset;
		u32 p_vaddr;
		u32 p_paddr;
		u32 p_filesz;
		u32 p_memsz;
		u32 p_flags;
		u32 p_align;
	} elf_loader_elf32_phdr_t;

	// dest_addr != 0, src_addr != 0, size != 0: move
	// dest_addr != 0, src_addr == 0, size != 0: zero
	// dest_addr != 0, size == 0: exec
	typedef struct elf_loader_loaderinfo_item_
	{
		void *dest_addr;
		void *src_addr;
		u32 size;
	} elf_loader_loaderinfo_item_t;

#define ELF_LOADER_MAX_LOADERINFO_ITEMS (ELF_LOADER_MAX_PROGRAM_HEADERS + 3)

	typedef struct elf_loader_loaderinfo_
	{
		elf_loader_loaderinfo_item_t items[ELF_LOADER_MAX_LOADERINFO_ITEMS];
	} elf_loader_loaderinfo_t;

	typedef struct elf_loader_arginfo_
	{
		int argc;
		char *argv[16];
		char payload[256];
	} elf_loader_arginfo_t;

	typedef struct elf_loader_execinfo_
	{
		elf_loader_arginfo_t arginfo;
		elf_loader_loaderinfo_t loaderinfo;
	} elf_loader_execinfo_t;

	typedef struct elf_loader_reader_segment_info_
	{
		void *m_segment_addr;
		int m_segment_offset;
		size_t m_segment_size;
	} elf_loader_reader_segment_info_t;

	typedef enum elf_loader_reader_stage_
	{
		ELF_LOADER_READER_STAGE_ELF_HEADER,
		ELF_LOADER_READER_STAGE_PROGRAM_HEADERS,
		ELF_LOADER_READER_STAGE_SEGMENTS,
		ELF_LOADER_READER_STAGE_END,
	} elf_loader_reader_stage_t;

	typedef void *(*elf_loader_reader_allocation_callback_t)(
		void *userdata, void *pointer, ptrdiff_t old_size, ptrdiff_t new_size);
	typedef int (*elf_loader_reader_read_callback_t)(
		void *userdata,
		elf_loader_reader_stage_t stage,
		const elf_loader_reader_segment_info_t *segm_info,
		size_t segm_count);
	typedef void (*elf_loader_reader_result_callback_t)(
		void *userdata, void *pointer, ptrdiff_t pointer_size, int errval);
	typedef struct elf_loader_reader_info_
	{
		void *m_userdata;
		elf_loader_reader_allocation_callback_t m_alloc_callback;
		elf_loader_reader_read_callback_t m_read_callback;
		elf_loader_reader_result_callback_t m_result_callback;
	} elf_loader_reader_info_t;

	typedef struct elf_loader_reader_info_stdio_resultbuf_
	{
		void *m_buf;
		ptrdiff_t m_bufsize;
		int m_result;
	} elf_loader_reader_info_stdio_resultbuf_t;

	// Defined in elf.c
	extern int elf_loader_is_elf_ehdr_valid(const elf_loader_elf32_ehdr_t *ehdr);
	extern int elf_loader_is_elf_valid_for_loading(const void *buf, size_t buf_size);
	extern int elf_loader_exec_elf_prepare_loadinfo(elf_loader_execinfo_t *execinfo, const void *buf, size_t buf_size);
	extern int elf_loader_exec_elf_prepare_arginfo(
		elf_loader_execinfo_t *execinfo, const char *filename, const char *partition, int argc, char *argv[]);
	extern int elf_loader_exec_elf(elf_loader_execinfo_t *execinfo);

	// Defined in elf_loader_reader.c
	extern void elf_loader_reader_read_elf_file(elf_loader_reader_info_t *info);

	// Defined in elf_loader_stdio.c
	extern int elf_loader_reader_read_elf_file_stdio(
		elf_loader_reader_info_stdio_resultbuf_t *resultbuf, const char *filename, const char *flags);

#ifdef __cplusplus
}
#endif

#endif /* __ELF_LOADER_COMMON_H__ */
