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

#define ERL_FLAG_STICKY 1

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

typedef struct erl_record_t * (*erl_loader_t)(const char * erl_id);

extern erl_loader_t _init_load_erl;

struct erl_record_t * load_erl_from_mem(u8 * mem, int argc, char ** argv);
struct erl_record_t * load_erl_from_file(const char * fname, int argc, char ** argv);
struct erl_record_t * _init_load_erl_from_file(const char * fname, const char * erl_id);
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
