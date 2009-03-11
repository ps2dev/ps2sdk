/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# The relocatable elf loader/linker.
*/

#ifndef __ERL_H__
#define __ERL_H__

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

struct htab;

/* ERL is sticky and cannot be unloaded */
#define ERL_FLAG_STICKY 1
/* ERL is relocated at a static memory address; set by *_to_addr functions */
#define ERL_FLAG_STATIC 2
/* Clear ERL bytes on unload */
#define ERL_FLAG_CLEAR  4

struct erl_record_t {
    u8 * bytes;
    u32 fullsize;
    char * name;
    char ** dependancies;
    u32 flags;

    /* Private */
    struct htab * symbols;
    struct erl_record_t * next, * prev;
}; // 32 bytes.

struct symbol_t {
    struct erl_record_t * provider;
    u32 address;
};

extern char _init_erl_prefix[];

typedef struct erl_record_t * (*erl_loader_t)(char * erl_id);

extern erl_loader_t _init_load_erl;

struct erl_record_t * load_erl_from_mem(u8 * mem, int argc, char ** argv);
struct erl_record_t * load_erl_from_file(const char * fname, int argc, char ** argv);
struct erl_record_t * _init_load_erl_from_file(const char * fname, char * erl_id);

/*
 * By default, ERLs are relocated at a dynamic memory address -- a pointer
 * returned by malloc().  The *_to_addr functions allow you to define a static
 * address (must be different from ERL_DYN_ADDR) without allocating any memory.
 */
#define ERL_DYN_ADDR 0

struct erl_record_t * load_erl_from_mem_to_addr(u8 * mem, u32 addr, int argc, char ** argv);
struct erl_record_t * load_erl_from_file_to_addr(const char * fname, u32 addr, int argc, char ** argv);
struct erl_record_t * _init_load_erl_from_file_to_addr(const char * fname, u32 addr, char * erl_id);

int unload_erl(struct erl_record_t * erl);

int erl_add_global_symbol(const char * symbol, u32 address);

struct erl_record_t * find_erl(const char * name);
struct erl_record_t * erl_resolve(u32 address);

struct symbol_t * erl_find_local_symbol(const char * symbol, struct erl_record_t * erl);
struct symbol_t * erl_find_symbol(const char * symbol);

void erl_flush_symbols(struct erl_record_t * erl);

#ifdef __cplusplus
}
#endif

#endif // __ERL_H__
