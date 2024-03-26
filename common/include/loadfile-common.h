/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Common definitions for loadfile between the client and server sides of the LOADFILE protocol.
 */

#ifndef __LOADFILE_COMMON_H__
#define __LOADFILE_COMMON_H__

#include <tamtypes.h>

// loadfile common definitions

#define LF_PATH_MAX 252
#define LF_ARG_MAX  252

enum _lf_val_types {
    LF_VAL_BYTE = 0,
    LF_VAL_SHORT,
    LF_VAL_LONG
};

enum _lf_functions {
    LF_F_MOD_LOAD = 0,
    LF_F_ELF_LOAD,

    LF_F_SET_ADDR,
    LF_F_GET_ADDR,

    LF_F_MG_MOD_LOAD,
    LF_F_MG_ELF_LOAD,

    LF_F_MOD_BUF_LOAD,

    LF_F_MOD_STOP,
    LF_F_MOD_UNLOAD,

    LF_F_SEARCH_MOD_BY_NAME,
    LF_F_SEARCH_MOD_BY_ADDRESS,

    LF_F_GET_VERSION = 0xFF,
};

typedef struct
{
    u32 epc;
    u32 gp;
    u32 sp;
    u32 dummy;
} t_ExecData;

struct _lf_iop_val_arg
{
    union
    {
        u32 iop_addr;
        int result;
    } p;
    int type;
    union
    {
        u8 b;
        u16 s;
        u32 l;
    } val;
} __attribute__((aligned(16)));

struct _lf_module_load_arg
{
    union
    {
        int arg_len;
        int result;
    } p;
    int modres;
    char path[LF_PATH_MAX];
    char args[LF_ARG_MAX];
} __attribute__((aligned(16)));

struct _lf_module_stop_arg
{
    union
    {
        int id;
        int result;
    } p;
    union
    {
        int arg_len;
        int modres;
    } q;
    char dummy[LF_PATH_MAX];
    char args[LF_ARG_MAX];
} __attribute__((aligned(16)));

union _lf_module_unload_arg
{
    int id;
    int result;
} __attribute__((aligned(16)));

struct _lf_search_module_by_name_arg
{
    int id;
    int dummy1;
    char name[LF_PATH_MAX];
    char dummy2[LF_ARG_MAX];
} __attribute__((aligned(16)));

struct _lf_search_module_by_address_arg
{
    union
    {
        const void *ptr;
        int id;
    } p;
} __attribute__((aligned(16)));

struct _lf_elf_load_arg
{
    u32 epc;
    u32 gp;
    char path[LF_PATH_MAX];
    char secname[LF_ARG_MAX];
} __attribute__((aligned(16)));

struct _lf_module_buffer_load_arg
{
    union
    {
        void *ptr;
        int result;
    } p;
    union
    {
        int arg_len;
        int modres;
    } q;
    char unused[LF_PATH_MAX];
    char args[LF_ARG_MAX];
} __attribute__((aligned(16)));

#endif
