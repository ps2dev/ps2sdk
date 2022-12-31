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
 * Kernel module loader.
 */

#ifndef __MODLOAD_H__
#define __MODLOAD_H__

#include <types.h>
#include <irx.h>

typedef struct loadfile_elf32_ehdr_
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
} loadfile_elf32_ehdr_t;

typedef struct loadfile_elf32_phdr_
{
	u32 p_type;
	u32 p_offset;
	u32 p_vaddr;
	u32 p_paddr;
	u32 p_filesz;
	u32 p_memsz;
	u32 p_flags;
	u32 p_align;
} loadfile_elf32_phdr_t;

typedef struct loadfile_elf32_shdr_
{
	u32 sh_name;
	u32 sh_type;
	u32 sh_flags;
	u32 sh_addr;
	u32 sh_offset;
	u32 sh_size;
	u32 sh_link;
	u32 sh_info;
	u32 sh_addralign;
	u32 sh_entsize;
} loadfile_elf32_shdr_t;

typedef struct loadfile_file_load_handler_struct_
{
	int fd;
	const char *filename;
	u8 *section_contents;
	int unknown_0C;
	loadfile_elf32_ehdr_t elf_header;
	loadfile_elf32_phdr_t *program_header;
	loadfile_elf32_shdr_t *section_headers;
	int unknown_4C;
} loadfile_file_load_handler_struct_t;

typedef struct loadfile_ee_elf_ringbuffer_content_
{
	u8 *buffer_base; // 0x00
	int buffer_offset; // 0x04
	int buffer_length; // 0x08
	int dma_handle; // 0x0C
} loadfile_ee_elf_ringbuffer_content_t;

typedef struct loadfile_allocate_handler_struct_
{
	int read_buffer_length; // 0x00	- Read buffer.
	int read_buffer_offset; // 0x04	- Read buffer offset into the file.
	loadfile_ee_elf_ringbuffer_content_t ring_buffer_contents[2];
	int ring_buffer_index; // 0x28
	int unknown_2C; // 0x2C
} loadfile_allocate_handler_struct_t;

typedef int (*loadfile_read_chunk_callback_t)(int fd, loadfile_allocate_handler_struct_t *allocate_info, void *read_callback_userdata);
typedef int (*loadfile_elf_load_proc_callback_t)(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, void *read_callback_userdata, loadfile_read_chunk_callback_t read_callback);
typedef int (*loadfile_check_valid_ee_elf_callback_t)(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs);
typedef int (*loadfile_elf_get_program_header_callback_t)(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs);
typedef void *(*loadfile_elf_load_alloc_buffer_from_heap_callback_t)(u32 size);
typedef void (*loadfile_elf_load_dealloc_buffer_from_heap_callback_t)(void *buffer);
typedef int (*loadfile_load_kelf_from_card_callback_t)(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int port, int slot, int *result_out, int *result_module_out);
typedef int (*loadfile_load_kelf_from_disk_callback_t)(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, int *result_out, int *result_module_out);

typedef struct SetLoadfileCallbacks_struct_
{
    loadfile_elf_load_proc_callback_t elf_load_proc;
    loadfile_check_valid_ee_elf_callback_t check_valid_ee_elf;
    loadfile_elf_get_program_header_callback_t elf_get_program_header;
    loadfile_elf_load_alloc_buffer_from_heap_callback_t elf_load_alloc_buffer_from_heap;
    loadfile_elf_load_dealloc_buffer_from_heap_callback_t elf_load_dealloc_buffer_from_heap;
    loadfile_load_kelf_from_card_callback_t load_kelf_from_card;
    loadfile_load_kelf_from_disk_callback_t load_kelf_from_disk;
} SetLoadfileCallbacks_struct_t;

typedef void *(*SecrCardBootFile_callback_t)(int port, int slot, void *buffer);
typedef void *(*SecrDiskBootFile_callback_t)(void *buffer);
typedef void (*SetLoadfileCallbacks_callback_t)(SetLoadfileCallbacks_struct_t *callbackinfo);
typedef int (*CheckKelfPath_callback_t)(const char *filename, int *port, int *slot);

void *GetModloadInternalData(void **pInternalData);

int ReBootStart(const char *command, unsigned int flags);
int LoadModuleAddress(const char *name, void *addr, int offset);
int LoadModule(const char *name);
int LoadStartModule(const char *name, int arglen, const char *args, int *result);
int StartModule(int modid, const char *name, int arglen, const char *args, int *result);
int LoadModuleBufferAddress(void *buffer, void *addr, int offset);
int LoadModuleBuffer(void *buffer);
int LoadStartKelfModule(const char *name, int arglen, const char *args, int *result);
void SetSecrmanCallbacks(SecrCardBootFile_callback_t SecrCardBootFile_fnc, SecrDiskBootFile_callback_t SecrDiskBootFile_fnc, SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_fnc);
void SetCheckKelfPathCallback(CheckKelfPath_callback_t CheckKelfPath_fnc);
void GetLoadfileCallbacks(CheckKelfPath_callback_t *CheckKelfPath_fnc, SetLoadfileCallbacks_callback_t *SetLoadfileCallbacks_fnc);
int IsIllegalBootDevice(const char *path);

#define modload_IMPORTS_start DECLARE_IMPORT_TABLE(modload, 1, 1)
#define modload_IMPORTS_end END_IMPORT_TABLE

#define I_GetModloadInternalData DECLARE_IMPORT(3, GetModloadInternalData);
#define I_ReBootStart DECLARE_IMPORT(4, ReBootStart)
#define I_LoadModuleAddress DECLARE_IMPORT(5, LoadModuleAddress)
#define I_LoadModule DECLARE_IMPORT(6, LoadModule)
#define I_LoadStartModule DECLARE_IMPORT(7, LoadStartModule)
#define I_StartModule DECLARE_IMPORT(8, StartModule)
#define I_LoadModuleBufferAddress DECLARE_IMPORT(9, LoadModuleBufferAddress)
#define I_LoadModuleBuffer DECLARE_IMPORT(10, LoadModuleBuffer)
#define I_LoadStartKelfModule DECLARE_IMPORT(11, LoadStartKelfModule)
#define I_SetSecrmanCallbacks DECLARE_IMPORT(12, SetSecrmanCallbacks)
#define I_SetCheckKelfPathCallback DECLARE_IMPORT(13, SetCheckKelfPathCallback)
#define I_GetLoadfileCallbacks DECLARE_IMPORT(14, GetLoadfileCallbacks)
#define I_IsIllegalBootDevice DECLARE_IMPORT(15, IsIllegalBootDevice)

#endif /* __MODLOAD_H__ */
