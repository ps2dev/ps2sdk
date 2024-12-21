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

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELF_LOADER_MAX_PROGRAM_HEADERS 32

// dest_addr != 0, src_addr != 0, size != 0: move
// dest_addr != 0, src_addr == 0, size != 0: zero
// dest_addr != 0, src_addr == 0, size == 0xFFFFFFFF: zero to end of memory
// dest_addr != 0, src_addr != 0, size == 0: exec
typedef struct elf_loader_loaderinfo_item_
{
    void *dest_addr;
    void *src_addr;
    u32 size;
} elf_loader_loaderinfo_item_t;

#define ELF_LOADER_MAX_LOADERINFO_ITEMS (ELF_LOADER_MAX_PROGRAM_HEADERS + 3)

typedef struct elf_loader_loaderinfo_
{
    u32 count;
    elf_loader_loaderinfo_item_t items[ELF_LOADER_MAX_LOADERINFO_ITEMS];
} elf_loader_loaderinfo_t;

typedef struct elf_loader_arginfo_ {
    int argc;
    char *argv[16];
    char payload[256];
} elf_loader_arginfo_t;

typedef struct elf_loader_execinfo_
{
    elf_loader_arginfo_t arginfo;
    elf_loader_loaderinfo_t loaderinfo;
} elf_loader_execinfo_t;

#ifdef __cplusplus
}
#endif

#endif /* __ELF_LOADER_COMMON_H__ */
