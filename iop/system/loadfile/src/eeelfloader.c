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
static void *heap_buffer      = NULL;
static void *heap_buffer_end  = NULL;

static int *allocate_heap_buffer(unsigned int lower_bound, unsigned int upper_bound)
{
    unsigned int v1;

    v1 = upper_bound;
    // Align to 4 bytes
    if ((v1 & 3) != 0) {
        v1 += (4 - (v1 & 3));
    }
    CpuDisableIntr();
    heap_buffer_base = AllocSysMemory(ALLOC_LAST, v1, NULL);
    CpuEnableIntr();
    if (heap_buffer_base != NULL) {
        heap_buffer     = heap_buffer_base;
        heap_buffer_end = &(((u8 *)heap_buffer_base)[v1]);
        return heap_buffer_base;
    }
    printf("memory allocation failed.\n");
    return NULL;
}

static void *elf_load_alloc_buffer_from_heap(u32 alloc_size)
{
    u32 v1;

    v1 = alloc_size;
    // Align to 4 bytes
    if ((alloc_size & 3) != 0) {
        v1 += (4 - (alloc_size & 3));
    }
    // Validity check...
    {
        u8 *new_ptr;

        new_ptr = heap_buffer;
        new_ptr += 4;
        new_ptr += v1;
        if ((u8 *)new_ptr > (u8 *)heap_buffer_end) {
            return NULL;
        }
    }
    // Do the allocation
    {
        u8 *new_ptr;
        u8 *ret_ptr;

        new_ptr = heap_buffer;

        ((u32 *)new_ptr)[0] = v1;
        new_ptr += sizeof(v1);

        ret_ptr = new_ptr;
        new_ptr += v1;


        heap_buffer = new_ptr;
        return ret_ptr;
    }
}

static void elf_load_dealloc_buffer_from_heap(void *alloc_memory)
{
    u32 chunk_size;
    // Check if NULL
    if (alloc_memory == NULL) {
        return;
    }
    // Check if in range of heap buffer
    if ((((u8 *)alloc_memory) - 4 < ((u8 *)heap_buffer_base)) || ((u8 *)alloc_memory) >= ((u8 *)heap_buffer_end)) {
        return;
    }
    // Get size
    chunk_size = *((u32 *)(((u8 *)alloc_memory) - 4));
    // Check if this is the current heap position
    if (((u8 *)alloc_memory) + chunk_size != ((u8 *)heap_buffer)) {
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

static int check_elf_header(const loadfile_elf32_ehdr_t **header)
{
    const loadfile_elf32_ehdr_t *v1;

    v1 = *header;
    if (v1->e_ident[0] != '\x7F') {
        return -1;
    }
    if (v1->e_ident[1] != 'E') {
        return -1;
    }
    if (v1->e_ident[2] != 'L') {
        return -1;
    }
    if (v1->e_ident[3] != 'F') {
        return -1;
    }
    return 0;
}

static int check_elf_architecture(const loadfile_file_load_handler_struct_t *flhs)
{
    if (flhs->elf_header.e_ident[4] != 1) {
        return -1;
    }
    if (flhs->elf_header.e_ident[5] != 1) {
        return -2;
    }
    if (flhs->elf_header.e_type != 2) {
        return -3;
    }
    if (flhs->elf_header.e_machine != 8) {
        return -4;
    }
    if (flhs->elf_header.e_ehsize != 52) {
        return -5;
    }
    if (flhs->elf_header.e_phentsize != 32) {
        return -6;
    }
    if (!flhs->elf_header.e_shnum) {
        return 1;
    }
    if (flhs->elf_header.e_shentsize != 40) {
        return -7;
    }
    return 1;
}

static int check_valid_ee_elf(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs)
{
    const loadfile_elf32_ehdr_t **v3;
    int v4;
    loadfile_elf32_ehdr_t *p_elf_header;

    v3           = (const loadfile_elf32_ehdr_t **)&allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
    v4           = check_elf_header(v3);
    p_elf_header = &flhs->elf_header;
    if (v4 < 0) {
        printf("File is not ELF format(%d)\n", v4);
        return KE_ILLEGAL_OBJECT;
    }
    memcpy(p_elf_header, *v3, sizeof(*p_elf_header));
    v4 = check_elf_architecture(flhs);
    if (v4 < 0) {
        printf("File is not for target architecture(%d)\n", v4);
        return KE_ILLEGAL_OBJECT;
    }
    return 0;
}

static int elf_get_program_header(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs)
{
    int v4;
    u8 *alloc_buffer_from_heap;

    v4                     = flhs->elf_header.e_phentsize * flhs->elf_header.e_phnum;
    alloc_buffer_from_heap = elf_load_alloc_buffer_from_heap(v4);
    flhs->program_header   = (loadfile_elf32_phdr_t *)alloc_buffer_from_heap;
    if (alloc_buffer_from_heap == NULL) {
        return KE_NO_MEMORY;
    }
    memcpy(alloc_buffer_from_heap, &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index].buffer_base[flhs->elf_header.e_phoff], v4);
    return 0;
}

static int elf_read_header_section_headers(loadfile_file_load_handler_struct_t *flhs)
{
    int v2;
    u8 *alloc_buffer_from_heap;

    lseek(flhs->fd, flhs->elf_header.e_shoff, 0);
    v2                     = flhs->elf_header.e_shentsize * flhs->elf_header.e_shnum;
    alloc_buffer_from_heap = elf_load_alloc_buffer_from_heap(v2);
    flhs->section_headers  = (loadfile_elf32_shdr_t *)alloc_buffer_from_heap;
    if (alloc_buffer_from_heap == NULL) {
        return KE_NO_MEMORY;
    }
    if (read(flhs->fd, alloc_buffer_from_heap, v2) != v2) {
        elf_load_dealloc_buffer_from_heap(flhs->section_headers);
        flhs->section_headers = 0;
        return KE_FILEERR;
    }
    return 0;
}

static int elf_read_section_contents(loadfile_file_load_handler_struct_t *flhs)
{
    loadfile_elf32_shdr_t *v2;
    u8 *alloc_buffer_from_heap;

    v2 = &flhs->section_headers[flhs->elf_header.e_shstrndx];
    if (v2->sh_type != 3 || v2->sh_flags != 0 || v2->sh_link != 0 || v2->sh_info != 0) {
        flhs->section_contents = 0;
        return KE_ILLEGAL_OBJECT;
    }
    alloc_buffer_from_heap = elf_load_alloc_buffer_from_heap(v2->sh_size);
    flhs->section_contents = alloc_buffer_from_heap;
    if (alloc_buffer_from_heap == NULL) {
        return KE_NO_MEMORY;
    }
    lseek(flhs->fd, v2->sh_offset, 0);
    if ((u32)(read(flhs->fd, flhs->section_contents, v2->sh_size)) != v2->sh_size) {
        elf_load_dealloc_buffer_from_heap(flhs->section_contents);
        flhs->section_contents = 0;
        return KE_FILEERR;
    }
    return 0;
}

static void sort_ph_contents(int ph_count, loadfile_elf_program_header_size_offset_t *ph_size_offset_data)
{
    if (ph_count > 1) {
        int v2;
        loadfile_elf_program_header_size_offset_t *v4;

        v2 = 1;
        v4 = ph_size_offset_data + 1;
        do {
            int v5;
            loadfile_elf_program_header_size_offset_t *v9;
            int index_of_ph_contents;
            unsigned int offset_of_ph_contents;

            index_of_ph_contents  = v4->index_of_ph_contents;
            offset_of_ph_contents = v4->offset_of_ph_contents;
            v5                    = v2 - 1;
            if (v5 >= 0) {
                loadfile_elf_program_header_size_offset_t *v6;

                v6 = &ph_size_offset_data[v5];
                for (;;) {
                    if (offset_of_ph_contents >= (u32)(v6->offset_of_ph_contents))
                        break;
                    v6[1].index_of_ph_contents  = v6->index_of_ph_contents;
                    v6[1].offset_of_ph_contents = v6->offset_of_ph_contents;
                    v5 -= 1;
                    v6 -= 1;
                    if (v5 < 0) {
                        break;
                    }
                }
            }
            v9                          = &ph_size_offset_data[v5];
            v9[1].index_of_ph_contents  = index_of_ph_contents;
            v9[1].offset_of_ph_contents = offset_of_ph_contents;
            v2 += 1;
            v4 += 1;
        } while (v2 < ph_count);
    }
}

static int fileio_reader_function(int fd, loadfile_allocate_handler_struct_t *allocate_info, void *userdata)
{
    int read_buffer_offset;
    loadfile_ee_elf_ringbuffer_content_t *v5;
    int v7;

    read_buffer_offset = allocate_info->read_buffer_offset;
    v5                 = &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
    v5->buffer_offset  = read_buffer_offset;
    v7                 = read(fd, v5->buffer_base, allocate_info->read_buffer_length);
    v5->buffer_length  = v7;
    allocate_info->read_buffer_offset += v7;
    return 0;
}

static int elf_load_proc(loadfile_allocate_handler_struct_t *allocate_info, loadfile_file_load_handler_struct_t *flhs, void *read_callback_userdata, loadfile_read_chunk_callback_t read_callback)
{
    int v12;
    int v13;
    int index_of_ph_contents;
    loadfile_elf32_phdr_t *v15;
    unsigned int p_filesz;
    loadfile_ee_elf_ringbuffer_content_t *v17;
    unsigned int v18;
    unsigned int v20;
    int v21;
    int v23;
    int v24;
    SifDmaTransfer_t v25;
    loadfile_elf_program_header_size_offset_t v26[32];
    int state;
    int ph_count;

    if (flhs->program_header == NULL) {
        return KE_ILLEGAL_OBJECT;
    }
    ph_count = 0;
    {
        int i;

        for (i = 0; i < flhs->elf_header.e_phnum; i += 1) {
            loadfile_elf32_phdr_t *v10;

            v10 = &flhs->program_header[i];
            if (v10->p_type == 1 && v10->p_filesz) {
                v26[ph_count].index_of_ph_contents  = i;
                v26[ph_count].offset_of_ph_contents = v10->p_offset;
                ph_count += 1;
            }
        }
    }
    sort_ph_contents(ph_count, v26);
    v12 = 0;
    printf("Input ELF format filename = %s\n", flhs->filename);
    v13 = 0;
    if (ph_count <= 0) {
    LABEL_21:
        printf("Loaded, %s\n", flhs->filename);
        return 0;
    } else {
        for (;;) {
            index_of_ph_contents = v26[v12].index_of_ph_contents;
            v15                  = &flhs->program_header[index_of_ph_contents];
            p_filesz             = v15->p_filesz;
            printf("%d %08x %08x ", index_of_ph_contents, v15->p_vaddr, p_filesz);
            if (p_filesz)
                break;
        LABEL_20:
            v12 += 1;
            printf("\n");
            v13 = 0;
            if (v12 >= ph_count)
                goto LABEL_21;
        }
        for (;;) {
            printf(".");
            v17 = &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
            while (sceSifDmaStat(v17->dma_handle) >= 0)
                ;
            v18 = v15->p_offset + v13;
            if (v18 >= (u32)(allocate_info->read_buffer_offset))
                break;
        LABEL_16:
            v20 = v18 - v17->buffer_offset;
            v21 = p_filesz;
            if (v17->buffer_length - v20 < p_filesz)
                v21 = v17->buffer_length - v20;
            v25.src = &v17->buffer_base[v20];
            p_filesz -= v21;
            v25.size = v21;
            v25.attr = 0;
            v25.dest = (void *)((v15->p_vaddr) + v13);
            CpuSuspendIntr(&state);
            v23 = sceSifSetDma(&v25, 1);
            v24 = state;
            v13 += v21;
            v17->dma_handle = v23;
            CpuResumeIntr(v24);
            if (!p_filesz)
                goto LABEL_20;
            allocate_info->ring_buffer_index = (allocate_info->ring_buffer_index + 1) & 1;
        }
        for (;;) {
            if (read_callback(flhs->fd, allocate_info, read_callback_userdata) != 0)
                break;
            if (v18 < (u32)(allocate_info->read_buffer_offset))
                goto LABEL_16;
        }
    }
    return KE_FILEERR;
}

static int elf_load_single_section(
    loadfile_allocate_handler_struct_t *allocate_info,
    loadfile_file_load_handler_struct_t *flhs,
    int epc,
    const char *section_name)
{
    loadfile_elf32_shdr_t *v7;
    int result;
    int v12;
    unsigned int sh_size;
    loadfile_ee_elf_ringbuffer_content_t *v14;
    SifDmaTransfer_t v22;
    int state;

    v7 = NULL;
    if (read(flhs->fd, &flhs->elf_header, 0x34) != 0x34) {
        return KE_FILEERR;
    }
    result = elf_read_header_section_headers(flhs);
    if (result < 0) {
        return result;
    }
    result = elf_read_section_contents(flhs);
    if (result < 0) {
        return result;
    }
    {
        int i;

        for (i = 0; i < flhs->elf_header.e_shnum; i += 1) {
            v7 = &flhs->section_headers[i];
            if (!strcmp((const char *)&flhs->section_contents[v7->sh_name], section_name) && v7->sh_size)
                break;
            v7 = NULL;
        }
    }
    if (v7 == NULL || v7->sh_addr < 0x80000) {
        return KE_ILLEGAL_OBJECT;
    }
    v12 = 0;
    lseek(flhs->fd, v7->sh_offset, 0);
    allocate_info->read_buffer_offset = v7->sh_offset;
    sh_size                           = v7->sh_size;
    printf("%s: %08x %08x ", section_name, v7->sh_addr, sh_size);
    while (sh_size != 0) {
        loadfile_allocate_handler_struct_t *i;
        unsigned int v15;
        unsigned int v17;
        int v18;
        int v20;
        int v21;


        printf(".");
        v14 = &allocate_info->ring_buffer_contents[allocate_info->ring_buffer_index];
        while (sceSifDmaStat(v14->dma_handle) >= 0)
            ;
        v15 = v7->sh_offset + v12;
        for (i = allocate_info; v15 >= (u32)(allocate_info->read_buffer_offset); i = allocate_info)
            fileio_reader_function(flhs->fd, i, 0);
        v17 = v15 - v14->buffer_offset;
        v18 = sh_size;
        if (v14->buffer_length - v17 < sh_size)
            v18 = v14->buffer_length - v17;
        v22.src = &v14->buffer_base[v17];
        sh_size -= v18;
        v22.size = v18;
        v22.attr = 0;
        v22.dest = (void *)((v7->sh_addr) + v12);
        CpuSuspendIntr(&state);
        v20 = sceSifSetDma(&v22, 1);
        v21 = state;
        v12 += v18;
        v14->dma_handle = v20;
        CpuResumeIntr(v21);
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
    if (result < 0) {
        return result;
    }
    result = elf_get_program_header(allocate_info, flhs);
    if (result < 0) {
        return result;
    }
    {
        int i;
        loadfile_elf32_phdr_t *program_headers;

        program_headers = flhs->program_header;
        for (i = 0; i < flhs->elf_header.e_phnum; i += 1) {
            if (program_headers[i].p_type == 1 && program_headers[i].p_filesz && program_headers[i].p_vaddr < 0x80000) {
                return KE_ILLEGAL_OBJECT;
            }
        }
    }
    result = elf_load_proc(allocate_info, flhs, 0, fileio_reader_function);
    if (result < 0) {
        return result;
    }
    *result_out        = flhs->elf_header.e_entry;
    *result_module_out = 0;
    printf("start address %#08x\n", *result_out);
    printf("gp address %#08x\n", *result_module_out);
    return 0;
}

static void empty_loadfile_information(loadfile_file_load_handler_struct_t *flhs)
{
    flhs->fd               = -1;
    flhs->filename         = 0;
    flhs->section_contents = 0;
    flhs->unknown_0C       = 0;
    flhs->program_header   = 0;
    flhs->section_headers  = 0;
    flhs->unknown_4C       = 0;
}

static int elf_load_common(const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out, int is_mg_elf)
{
    int *heap_buffer;
    int v11;
    int v12;
    char *v13;
    loadfile_allocate_handler_struct_t *p_allocate_info;
    int result;
    loadfile_allocate_handler_struct_t allocate_info;
    loadfile_file_load_handler_struct_t flhs;
    SetLoadfileCallbacks_struct_t loadfile_functions;
    CheckKelfPath_callback_t CheckKelfPath_fnc;
    SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_fnc;
    int v23;
    int v24;

    printf("%s", "loadelf version 3.30\n");
    empty_loadfile_information(&flhs);
    flhs.filename = filename;
    flhs.fd       = open(filename, 1);
    if (flhs.fd < 0) {
        printf("Cannot openfile\n");
        result = KE_NOFILE;
        goto finish_returnresult;
    }
    heap_buffer = allocate_heap_buffer(0, 0x10000);
    v11         = 0x20000;
    if (heap_buffer == NULL) {
        printf("Error Can't Get heap buffer\n");
        result = KE_NO_MEMORY;
        goto finish_closefd;
    }
    v12 = 0;
    for (;;) {
        CpuDisableIntr();
        v13 = (char *)AllocSysMemory(ALLOC_LAST, v11, NULL);
        CpuEnableIntr();
        if (v13)
            break;
        v11 /= 2;
        if (++v12 >= 8) {
            printf("Error Can't Get read buffer\n");
            result = KE_NO_MEMORY;
            goto finish_freeheapbuffer;
        }
    }
    p_allocate_info                  = &allocate_info;
    allocate_info.ring_buffer_index  = 0;
    allocate_info.read_buffer_length = (unsigned int)v11 >> 1;
    allocate_info.read_buffer_offset = 0;
    {
        int i;

        for (i = 0; i < 2; i += 1) {
            int v16;

            v16                                                    = allocate_info.read_buffer_length * i;
            p_allocate_info->ring_buffer_contents[i].buffer_offset = 0;
            p_allocate_info->ring_buffer_contents[i].buffer_length = 0;
            p_allocate_info->ring_buffer_contents[i].dma_handle    = 0;
            p_allocate_info->ring_buffer_contents[i].buffer_base   = (u8 *)&v13[v16];
        }
    }
    result = KE_ILLEGAL_OBJECT;
    if (is_mg_elf) {
        GetLoadfileCallbacks(&CheckKelfPath_fnc, &SetLoadfileCallbacks_fnc);
        if (SetLoadfileCallbacks_fnc) {
            loadfile_functions.elf_load_proc                     = elf_load_proc;
            loadfile_functions.check_valid_ee_elf                = check_valid_ee_elf;
            loadfile_functions.elf_get_program_header            = elf_get_program_header;
            loadfile_functions.elf_load_alloc_buffer_from_heap   = elf_load_alloc_buffer_from_heap;
            loadfile_functions.elf_load_dealloc_buffer_from_heap = elf_load_dealloc_buffer_from_heap;
            SetLoadfileCallbacks_fnc(&loadfile_functions);
            if (CheckKelfPath_fnc && CheckKelfPath_fnc(filename, &v23, &v24)) {
                if (loadfile_functions.load_kelf_from_card) {
                    result = loadfile_functions.load_kelf_from_card(
                        &allocate_info,
                        &flhs,
                        v23,
                        v24,
                        result_out,
                        result_module_out);
                }
            } else if (loadfile_functions.load_kelf_from_disk) {
                result = loadfile_functions.load_kelf_from_disk(
                    &allocate_info,
                    &flhs,
                    result_out,
                    result_module_out);
            }
        }
    } else if (!strcmp(section_name, "all")) {
        result = elf_load_all_section(&allocate_info, &flhs, result_out, result_module_out);
    } else {
        result = elf_load_single_section(&allocate_info, &flhs, epc, section_name);
    }
    FreeSysMemory(v13);
finish_freeheapbuffer:
    FreeSysMemory(heap_buffer);
finish_closefd:
    close(flhs.fd);
finish_returnresult:
    return result;
}

int loadfile_elfload_innerproc(const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out)
{
    if (IsIllegalBootDevice(filename) != 0) {
        return KE_ILLEGAL_OBJECT;
    }
    return elf_load_common(filename, epc, section_name, result_out, result_module_out, 0);
}

int loadfile_mg_elfload_proc(const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out)
{
    if (strcmp(section_name, "all") != 0) {
        return KE_ILLEGAL_MODE;
    }
    return elf_load_common(filename, epc, section_name, result_out, result_module_out, 1);
}
