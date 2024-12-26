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
#include <sifrpc.h>
#include <kernel.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <iopcontrol.h>
#include <loadfile.h>

// Loader ELF variables
extern u8 loader_elf[];
extern int size_loader_elf;

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

typedef struct elf_loader_file_load_handler_struct_
{
    int fd;
    elf_loader_elf32_ehdr_t elf_header;
    elf_loader_elf32_phdr_t *program_header;
} elf_loader_file_load_handler_struct_t;

static int check_elf_header(const elf_loader_elf32_ehdr_t *elf_header)
{
    if (elf_header->e_ident[0] != '\x7F') {
        return -1;
    }
    if (elf_header->e_ident[1] != 'E') {
        return -1;
    }
    if (elf_header->e_ident[2] != 'L') {
        return -1;
    }
    if (elf_header->e_ident[3] != 'F') {
        return -1;
    }
    return 0;
}

static int check_elf_architecture(const elf_loader_elf32_ehdr_t *elf_header)
{
    if (elf_header->e_ident[4] != 1) {
        return -1;
    }
    if (elf_header->e_ident[5] != 1) {
        return -2;
    }
    if (elf_header->e_type != 2) {
        return -3;
    }
    if (elf_header->e_machine != 8) {
        return -4;
    }
    if (elf_header->e_ehsize != 52) {
        return -5;
    }
    if (elf_header->e_phentsize != 32) {
        return -6;
    }
    if (elf_header->e_phnum > ELF_LOADER_MAX_PROGRAM_HEADERS) {
        return -7;
    }
    if (elf_header->e_shnum == 0) {
        return 1;
    }
    if (elf_header->e_shentsize != 40) {
        return -8;
    }
    return 1;
}

static int check_valid_ee_elf(elf_loader_file_load_handler_struct_t *flhs)
{
    if (lseek(flhs->fd, 0, SEEK_SET) != 0) {
        return -EIO;
    }
    if (read(flhs->fd, &flhs->elf_header, sizeof(elf_loader_elf32_ehdr_t)) != sizeof(elf_loader_elf32_ehdr_t)) {
        return -EIO;
    }
    if (check_elf_header(&flhs->elf_header) < 0) {
        return -ENOEXEC;
    }
    if (check_elf_architecture(&flhs->elf_header) < 0) {
        return -ENOEXEC;
    }
    return 0;
}

static int elf_get_program_header(elf_loader_file_load_handler_struct_t *flhs)
{
    int ph_totalbytes;

    ph_totalbytes = flhs->elf_header.e_phentsize * flhs->elf_header.e_phnum;
    flhs->program_header = (elf_loader_elf32_phdr_t *)malloc(ph_totalbytes);
    if (flhs->program_header == NULL) {
        return -ENOMEM;
    }
    // TODO: check if program header offset is outside of LOADFILE worst case allocation size
    if (lseek(flhs->fd, flhs->elf_header.e_phoff, SEEK_SET) != flhs->elf_header.e_phoff) {
        return -EIO;
    }
    if (read(flhs->fd, flhs->program_header, ph_totalbytes) != ph_totalbytes) {
        return -EIO;
    }
    return 0;
}

typedef struct elf_loader_keyval_
{
    u32 kv_key;
    u32 kv_value;
} elf_loader_keyval_t;

static void sort_keyval_contents(int kv_count, elf_loader_keyval_t *ph_size_offset_data)
{
    if (kv_count > 1) {
        int v2;
        elf_loader_keyval_t *v4;

        v2 = 1;
        v4 = ph_size_offset_data + 1;
        do {
            int v5;
            elf_loader_keyval_t *v9;
            u32 kv_key;
            u32 kv_value;

            kv_key = v4->kv_key;
            kv_value = v4->kv_value;
            v5 = v2 - 1;
            if (v5 >= 0) {
                elf_loader_keyval_t *v6;

                v6 = &ph_size_offset_data[v5];
                for (;;) {
                    if (kv_value >= v6->kv_value)
                        break;
                    v6[1].kv_key = v6->kv_key;
                    v6[1].kv_value = v6->kv_value;
                    v5 -= 1;
                    v6 -= 1;
                    if (v5 < 0) {
                        break;
                    }
                }
            }
            v9 = &ph_size_offset_data[v5];
            v9[1].kv_key = kv_key;
            v9[1].kv_value = kv_value;
            v2 += 1;
            v4 += 1;
        } while (v2 < kv_count);
    }
}

typedef struct elf_loader_psegment_
{
    u32 buf_size;
    u32 zero_size;
    u32 inbuf_offset;
    u32 outbuf_offset;
    u32 load_address;
} elf_loader_psegment_t;

static int elf_load_proc(elf_loader_file_load_handler_struct_t *flhs, void **buf_out)
{
    elf_loader_psegment_t psegment[ELF_LOADER_MAX_PROGRAM_HEADERS];
    unsigned int psegment_count;
    psegment_count = 0;
    if (flhs->program_header == NULL) {
        return -ENOEXEC;
    }
    
    u32 total_filesize;
    total_filesize = sizeof(elf_loader_loaderinfo_t);
    u32 data_start_offset = 0;
    {
        int i;
        unsigned int kv_count;
        kv_count = 0;
        elf_loader_keyval_t kvinfo[ELF_LOADER_MAX_PROGRAM_HEADERS];

        for (i = 0; i < flhs->elf_header.e_phnum; i += 1) {
            elf_loader_elf32_phdr_t *program_header;

            program_header = &flhs->program_header[i];
            if (program_header->p_type == 1 && program_header->p_filesz != 0) {
                kvinfo[kv_count].kv_key = i;
                kvinfo[kv_count].kv_value = program_header->p_offset;
                kv_count += 1;
            }
        }
        psegment_count = kv_count;
        sort_keyval_contents(kv_count, kvinfo);

        data_start_offset = total_filesize;
        for (i = 0; i < kv_count; i += 1) {
            elf_loader_elf32_phdr_t *program_header;
            u32 key = kvinfo[i].kv_key;

            program_header = &flhs->program_header[kvinfo[i].kv_key];
            psegment[key].buf_size = program_header->p_filesz;
            psegment[key].zero_size = program_header->p_filesz - program_header->p_memsz;
            psegment[key].inbuf_offset = program_header->p_offset;
            psegment[key].outbuf_offset = total_filesize;
            psegment[key].load_address = program_header->p_vaddr;
            total_filesize += program_header->p_filesz;
        }
    }

    int ret;

    void *buf;
    buf = malloc(total_filesize);
    if (buf == NULL) {
        ret = -ENOMEM;
        goto finish;
    }
    {
        int i;
        u8 *buf_cur = buf;
        for (i = 0; i < psegment_count; i += 1) {
            if (lseek(flhs->fd, psegment[i].inbuf_offset, SEEK_SET) != psegment[i].inbuf_offset) {
                ret = -EIO;
                goto finish;
            }
            if (read(flhs->fd, buf_cur + psegment[i].outbuf_offset, psegment[i].buf_size) != psegment[i].buf_size) {
                ret = -EIO;
                goto finish;
            }
        }
    }
    {
        int i;
        u32 min_load_addr;
        u32 max_load_addr;
        elf_loader_loaderinfo_t *buf_cur;
        min_load_addr = 0xFFFFFFFF;
        max_load_addr = 0x00100000;
        buf_cur = buf;
        buf_cur->count = psegment_count;
        u32 info_count;
        info_count = 0;
        for (i = 0; i < psegment_count; i += 1) {
            buf_cur->items[info_count].dest_addr = (void *)(psegment[i].load_address);
            buf_cur->items[info_count].src_addr = (void *)(((u8 *)buf) + data_start_offset + psegment[i].outbuf_offset);
            buf_cur->items[info_count].size = psegment[i].buf_size;
            info_count += 1;
            {
                u32 load_address;
                u32 end_load_address;
                load_address = psegment[i].load_address;
                end_load_address = load_address + psegment[i].buf_size;
                if (load_address < min_load_addr) {
                    min_load_addr = load_address;
                }
                if (end_load_address > max_load_addr) {
                    max_load_addr = end_load_address;
                }
            }
            if (psegment[i].zero_size != 0) {
            }
        }
        if (min_load_addr != 0xFFFFFFFF) {
            u32 wanted_size;
            wanted_size = min_load_addr - 0x00100000;
            if (wanted_size != 0) {
                buf_cur->items[info_count].dest_addr = (void *)0x00100000;
                buf_cur->items[info_count].src_addr = (void *)NULL;
                buf_cur->items[info_count].size = wanted_size;
                info_count += 1;
            }
        }
        if (max_load_addr != 0x00100000) {
            buf_cur->items[info_count].dest_addr = (void *)max_load_addr;
            buf_cur->items[info_count].src_addr = (void *)NULL;
            buf_cur->items[info_count].size = 0xFFFFFFFF;
            info_count += 1;
        }
        buf_cur->items[info_count].dest_addr = (void *)(flhs->elf_header.e_entry);
        buf_cur->items[info_count].src_addr = (void *)NULL; // GP returned is always NULL
        buf_cur->items[info_count].size = 0;
    }
    if (buf_out) {
        *buf_out = buf;
    }
    ret = 0;

finish:
    if (ret < 0 && buf != NULL) {
        free(buf);
        buf = NULL;
    }
    return ret;
}

static int elf_load_all_section(
    elf_loader_file_load_handler_struct_t *flhs,
    void **buf_out)
{
    int result;

    result = check_valid_ee_elf(flhs);
    if (result < 0) {
        goto finish;
    }
    result = elf_get_program_header(flhs);
    if (result < 0) {
        goto finish;
    }
    {
        int i;
        int ph_count;

        ph_count = 0;
        for (i = 0; i < flhs->elf_header.e_phnum; i += 1) {
            elf_loader_elf32_phdr_t *program_header;

            program_header = &flhs->program_header[i];
            if (program_header->p_type == 1 && program_header->p_filesz != 0) {
                // LOADFILE checked for before 0x80000.
                // We'll check for usage before user memory to avoid clashing with the loader
                if (program_header->p_vaddr < 0x00100000) {
                    result = -ENOEXEC;
                    goto finish;
                }
                ph_count += 1;
            }
        }
        // TODO: do we need to handle more than one program header?
        if (ph_count > 1) {
            result = -ENOEXEC;
            goto finish;
        }
    }
    // TODO: check if entry point is in one of the program segments
    if (flhs->elf_header.e_entry < 0x00100000) {
        result = -ENOEXEC;
        goto finish;
    }
    result = elf_load_proc(flhs, buf_out);
    if (result < 0) {
        goto finish;
    }
    result = 0;
finish:
    if (flhs->program_header != NULL) {
        free(flhs->program_header);
        flhs->program_header = NULL;
    }
    return result;
}

static void empty_elf_loader_information(elf_loader_file_load_handler_struct_t *flhs)
{
    flhs->fd = -1;
    flhs->program_header = NULL;
}

static int elf_load_common(const char *filename, void **buf_out)
{
    int result;
    elf_loader_file_load_handler_struct_t flhs;

    empty_elf_loader_information(&flhs);
    flhs.fd = open(filename, 1);
    if (flhs.fd < 0) {
        return -ENOENT;
    }
    result = elf_load_all_section(&flhs, buf_out);
    close(flhs.fd);
    return result;
}

static void elf_loader_set_arginfo(elf_loader_arginfo_t *arginfo, const char *filename, const char *partition, int argc, char *argv[])
{
    char *ptr;
    int len, i;

    ptr = arginfo->payload;
    argc = (argc > 15) ? 15 : argc;
    arginfo->argc = argc + 1;
    arginfo->argv[0] = ptr;
    if (partition != NULL) {
        len = strlen(partition); // Not including NULL terminator
        memcpy(ptr, partition, len);
        ptr += len;
    }
    len = strlen(filename) + 1;
    memcpy(ptr, filename, len);
    ptr += len;

    for (i = 0; i < argc; i += 1) {
        arginfo->argv[i + 1] = ptr;
        len = strlen(argv[i]) + 1;
        memcpy(ptr, argv[i], len);
        ptr += len;
    }
}

typedef void *(*ldr_getinternalinfo_callback)(void);
#define LDR_ENTRYPOINT_ADDR 0x00084000

int LoadELFFromFileWithPartition(const char *filename, const char *partition, int argc, char *argv[])
{
    int ret;
    void *buf;
    ldr_getinternalinfo_callback cb;
    void **ldr_internalinfo;

    buf = NULL;
    ret = elf_load_common(filename, &buf);
    if (ret < 0) {
        return ret;
    }
    if (buf == NULL) {
        return -1;
    }

    // No turning back here.
    while (!SifIopReset(NULL, 0)) {};
    while (!SifIopSync()) {};

    SifInitRpc(0);
    SifLoadFileInit();
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:MCMAN", 0, NULL);
    SifLoadModule("rom0:MCSERV", 0, NULL);
    SifLoadFileExit();
    SifExitRpc();

    memset((void *)LDR_ENTRYPOINT_ADDR, 0, 0x7c000);
    memcpy((void *)LDR_ENTRYPOINT_ADDR, loader_elf, size_loader_elf);
    FlushCache(0);
    FlushCache(2);
    cb = (void *)LDR_ENTRYPOINT_ADDR;
    ldr_internalinfo = cb();
    elf_loader_execinfo_t *execinfo;
    execinfo = (elf_loader_execinfo_t *)(ldr_internalinfo[0]);
    elf_loader_set_arginfo(&(execinfo->arginfo), filename, partition, argc, argv);
    memcpy(&(execinfo->loaderinfo), buf, sizeof(elf_loader_execinfo_t));
    // TODO: check if argv can be NULL
    ExecPS2(ldr_internalinfo[1], NULL, 0, &(execinfo->arginfo.argv[1]));
    // Should be unreachable here
    Exit(126);
    return -1;
}

int LoadELFFromFile(const char *filename, int argc, char *argv[])
{
    return LoadELFFromFileWithPartition(filename, NULL, argc, argv);
}
