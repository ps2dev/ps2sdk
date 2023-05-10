/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include <kerr.h>
#include <modload.h>

#ifdef _IOP
IRX_ID("Moldule_File_loader", 1, 1);
#endif
// Mostly based on the module from SCE SDK 1.3.4

extern struct irx_export_table _exp_modload;

SecrCardBootFile_callback_t SecrCardBootFile_func_ptr;
SecrDiskBootFile_callback_t SecrDiskBootFile_func_ptr;
SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_func_ptr;
CheckKelfPath_callback_t CheckKelfPath_func_ptr;

int modLoadCB;
int ModuleLoaderMutex;
int ModuleLoaderSync;

typedef struct module_thread_args_
{
    int command;
    int modid;
    void *data;
    void *buffer;
    int arglen;
    const char *args;
    void *addr;
    int offset;
    int *result;
    int *module_id;
    s32 thread_ef;
} module_thread_args_t;


extern int modload_post_boot_callback(iop_init_entry_t *next, int delayed);
extern void get_updater_boot_argument(char *a1, int *updater_argc, char **updater_argv, int updater_argv_count);
extern int ModuleLoaderThread(module_thread_args_t *a1);
extern void *do_load_seek(const char *filename, int *result_out);
extern ModuleInfo_t *do_load(char *filename, void *buffer, void *addr, int offset, int *result_out);
extern ModuleInfo_t *SearchModuleCBByID(int modid);
extern int start_module(ModuleInfo_t *module_info, const char *data, size_t arglen, const char *args, int *result_2);
extern ModuleInfo_t *do_load_noseek(void *buffer, void *addr, int offset, int *result_out);

int _start(int argc, char *argv[])
{
    iop_thread_t v3;
    iop_sema_t v4;
    iop_event_t v5;

    if (RegisterLibraryEntries(&_exp_modload) < 0) {
        return 1;
    }
    {
        int *BootMode;

        BootMode = QueryBootMode(4);
        if (BootMode) {
            if ((((u32 *)BootMode)[0] & 0xff) == 2) {
                RegisterPostBootCallback((BootupCallback_t)modload_post_boot_callback, 1, 0);
            }
        }
    }
    v3.attr           = 0x2000000;
    v3.thread         = (void (*)(void *))ModuleLoaderThread;
    v3.priority       = 8;
    v3.stacksize      = 2048;
    v3.option         = 0;
    modLoadCB         = CreateThread(&v3);
    v4.attr           = 0;
    v4.initial        = 1;
    v4.max            = 1;
    v4.option         = 0;
    ModuleLoaderMutex = CreateSema(&v4);
    memset(&v5, 0, sizeof(v5));
    ModuleLoaderSync = CreateEventFlag(&v5);
    return 0;
}

s32 ModuleThreadCmd(int command, void *data, void *buffer, void *addr, int offset)
{
    s32 result;
    module_thread_args_t v10;
    int v11;
    u32 v12;

    result = GetThreadId();
    if (result >= 0) {
        v10.command   = command;
        v10.data      = data;
        v10.buffer    = buffer;
        v10.addr      = addr;
        v10.offset    = offset;
        v10.module_id = &v11;
        if (result == modLoadCB) {
            v10.thread_ef = 0;
            ModuleLoaderThread(&v10);
        } else {
            v10.thread_ef = ModuleLoaderSync;
            result        = WaitSema(ModuleLoaderMutex);
            if (result < 0) {
                return result;
            }
            StartThread(modLoadCB, &v10);
            WaitEventFlag(ModuleLoaderSync, 1u, 17, &v12);
            SignalSema(ModuleLoaderMutex);
        }
        return v11;
    }
    return result;
}

int LoadModuleAddress(const char *name, void *addr, int offset)
{
    return ModuleThreadCmd(1, (void *)name, 0, addr, offset);
}

int LoadModule(const char *name)
{
    return LoadModuleAddress(name, 0, 0);
}

int LoadModuleBufferAddress(void *buffer, void *addr, int offset)
{
    return ModuleThreadCmd(4, 0, buffer, addr, offset);
}

int LoadModuleBuffer(void *buffer)
{
    return LoadModuleBufferAddress(buffer, 0, 0);
}

int LoadStartModule(const char *name, int arglen, const char *args, int *result)
{
    int ThreadId;
    module_thread_args_t v9;
    int v10;
    u32 v11;

    ThreadId = GetThreadId();
    if (ThreadId < 0) {
        return ThreadId;
    }
    v9.data      = (void *)name;
    v9.command   = 3;
    v9.buffer    = 0;
    v9.addr      = 0;
    v9.offset    = 0;
    v9.arglen    = arglen;
    v9.args      = args;
    v9.result    = result;
    v9.module_id = &v10;
    if (ThreadId == modLoadCB) {
        v9.thread_ef = 0;
        ModuleLoaderThread(&v9);
    } else {
        v9.thread_ef = ModuleLoaderSync;
        ThreadId     = WaitSema(ModuleLoaderMutex);
        if (ThreadId < 0) {
            return ThreadId;
        }
        StartThread(modLoadCB, &v9);
        WaitEventFlag(ModuleLoaderSync, 1u, 17, &v11);
        SignalSema(ModuleLoaderMutex);
    }
    return v10;
}

int StartModule(int modid, const char *name, int arglen, const char *args, int *result)
{
    int ThreadId;
    module_thread_args_t v10;
    int v11;
    u32 v12;

    ThreadId = GetThreadId();
    if (ThreadId < 0) {
        return ThreadId;
    }
    v10.command   = 2;
    v10.modid     = modid;
    v10.data      = (void *)name;
    v10.arglen    = arglen;
    v10.args      = args;
    v10.result    = result;
    v10.module_id = &v11;
    if (ThreadId == modLoadCB) {
        v10.thread_ef = 0;
        ModuleLoaderThread(&v10);
    } else {
        v10.thread_ef = ModuleLoaderSync;
        ThreadId      = WaitSema(ModuleLoaderMutex);
        if (ThreadId < 0) {
            return ThreadId;
        }
        StartThread(modLoadCB, &v10);
        WaitEventFlag(ModuleLoaderSync, 1u, 17, &v12);
        SignalSema(ModuleLoaderMutex);
    }
    return v11;
}

int LoadStartKelfModule(const char *name, int arglen, const char *args, int *result)
{
    void *v8;
    void *v9;
    int ModuleBuffer;
    int started;
    int v13;
    int v14;
    int state;

    v8 = 0;
    v9 = do_load_seek(name, &started);
    if (v9 == 0) {
        return started;
    }
    if (CheckKelfPath_func_ptr && CheckKelfPath_func_ptr(name, &v13, &v14)) {
        if (SecrCardBootFile_func_ptr) {
            v8 = SecrCardBootFile_func_ptr(v13, v14, v9);
        }
    } else if (SecrDiskBootFile_func_ptr) {
        v8 = SecrDiskBootFile_func_ptr(v9);
    }
    if (!v8) {
        CpuSuspendIntr(&state);
        FreeSysMemory(v9);
        CpuResumeIntr(state);
        return KE_ILLEGAL_OBJECT;
    }
    ModuleBuffer = LoadModuleBuffer(v8);
    started      = ModuleBuffer;
    if (ModuleBuffer > 0) {
        started = StartModule(ModuleBuffer, name, arglen, args, result);
    }
    CpuSuspendIntr(&state);
    FreeSysMemory(v9);
    CpuResumeIntr(state);
    return started;
}

int ModuleLoaderThread(module_thread_args_t *a1)
{
    int command;
    ModuleInfo_t *v3;
    int v4;
    ModuleInfo_t *v5;
    ModuleInfo_t *v6;
    int v8[2];

    command = a1->command;
    if (command == 1) {
        v3 = do_load((char *)a1->data, 0, a1->addr, a1->offset, v8);
        v4 = v8[0];
        if (v8[0]) {
            *a1->module_id = v4;
        } else {
            *a1->module_id = v3->id;
        }
    }
    if (command == 2) {
        v5 = SearchModuleCBByID(a1->modid);
        if (!v5) {
            *a1->module_id = KE_UNKNOWN_MODULE;
        } else {
            *a1->module_id = v5->id;
            start_module(v5, (const char *)a1->data, a1->arglen, a1->args, a1->result);
        }
    }
    if (command == 3) {
        v6 = do_load((char *)a1->data, 0, a1->addr, a1->offset, v8);
        v4 = v8[0];
        v5 = v6;
        if (v8[0]) {
            *a1->module_id = v4;
        } else {
            *a1->module_id = v5->id;
            start_module(v5, (const char *)a1->data, a1->arglen, a1->args, a1->result);
        }
    }
    if (command == 4) {
        v3 = do_load(0, a1->buffer, a1->addr, a1->offset, v8);
        v4 = v8[0];
        if (v8[0]) {
            *a1->module_id = v4;
        } else {
            *a1->module_id = v3->id;
        }
    }
    if (a1->thread_ef) {
        ChangeThreadPriority(0, 1);
        SetEventFlag(a1->thread_ef, 1u);
    }
    return 0;
}

ModuleInfo_t *SearchModuleCBByID(int modid)
{
    ModuleInfo_t *image_info;

    image_info = GetLoadcoreInternalData()->image_info;
    while (image_info != NULL) {
        if (modid == image_info->id) {
            return image_info;
        }
        image_info = image_info->next;
    }
    return NULL;
}

int modload_post_boot_callback(iop_init_entry_t *next, int delayed)
{
    void *iop_exec_buffer;
    int v3;
    int *BootMode;
    int *v5;
    int updater_argc;
    int v11;
    int v12;
    int v13;
    int state;

    iop_exec_buffer = 0;
    v3              = 0;
    BootMode        = QueryBootMode(4);
    if (BootMode) {
        // See reboot_start_proc for when this variable gets set
        v3 = (((u32 *)BootMode)[0] >> 8) & 0xff;
    }
    v5 = QueryBootMode(5);
    if (v5) {
        ModuleInfo_t *module_info;
        char *updater_argv[16];

        memset(updater_argv, 0, sizeof(updater_argv));
        module_info = 0;
        get_updater_boot_argument((char *)v5[1], &updater_argc, updater_argv, (sizeof(updater_argv) / sizeof(updater_argv[0])) - 1);
        if (v3 == 0) {
            module_info = do_load(updater_argv[0], 0, (void *)0x100000, 0, &v11);
        } else if (v3 == 1) {
            void *iop_exec_encrypted_buffer;

            iop_exec_encrypted_buffer = do_load_seek(updater_argv[0], &v11);
            if (iop_exec_encrypted_buffer) {
                if (CheckKelfPath_func_ptr && CheckKelfPath_func_ptr(updater_argv[0], &v12, &v13)) {
                    if (SecrCardBootFile_func_ptr) {
                        iop_exec_buffer = SecrCardBootFile_func_ptr(v12, v13, iop_exec_encrypted_buffer);
                    }
                } else if (SecrDiskBootFile_func_ptr) {
                    iop_exec_buffer = SecrDiskBootFile_func_ptr(iop_exec_encrypted_buffer);
                }
                if (iop_exec_buffer) {
                    module_info = do_load_noseek(iop_exec_buffer, (void *)0x100000, 0, &v11);
                } else {
                    v11 = -1;
                }
                CpuSuspendIntr(&state);
                FreeSysMemory(iop_exec_encrypted_buffer);
                CpuResumeIntr(state);
            }
        } else {
            v11 = -1;
        }
        if (v11 == 0) {
            v11 = ((int (*)(int, char **, u32, ModuleInfo_t *))module_info->entry)(updater_argc, updater_argv, 0, module_info);
            printf("return from updater '%s' return value = %d\n", updater_argv[0], v11);
            __asm("break\n");
        }
        printf("updater '%s' can't load\n", updater_argv[0]);
        __asm("break\n");
    }
    printf("Reboot fail! need file name argument\n");
    return 0;
}

int start_module(ModuleInfo_t *module_info, const char *data, size_t arglen, const char *args, int *result_2)
{
    const char *v9;
    char *v10;
    int in_argc;
    int v17;
    int data_strlen;
    int in_argv_size_strs;
    char *in_argv_strs;
    char **in_argv_ptrs;
    int i;

    in_argc           = 1;
    data_strlen       = strlen(data) + 1;
    in_argv_size_strs = data_strlen;
    for (v9 = args; v9 < &args[arglen];) {
        int str_len = strlen(v9) + 1;
        in_argv_size_strs += str_len;
        v9 += str_len;
        in_argc += 1;
    }
    if (in_argv_size_strs < data_strlen + arglen) {
        in_argv_size_strs = data_strlen + arglen;
    }
    in_argv_strs = __builtin_alloca(in_argv_size_strs);
    memcpy(in_argv_strs, data, data_strlen);
    memcpy(in_argv_strs + data_strlen, args, arglen);
    in_argv_ptrs = __builtin_alloca((in_argc + 1) * sizeof(char *));
    for (i = 0, v10 = in_argv_strs; i < in_argc && v10 < &in_argv_strs[in_argv_size_strs];) {
        int str_len     = strlen(v10) + 1;
        in_argv_ptrs[i] = v10;
        i += 1;
        v10 += str_len;
    }
    in_argv_ptrs[in_argc] = NULL;
    v17                   = ((int (*)(int, char **, u32, ModuleInfo_t *))module_info->entry)(in_argc, in_argv_ptrs, 0, module_info);
    ChangeThreadPriority(0, 8);
    if (result_2) {
        *result_2 = v17;
    }
    if ((v17 & 3) != 0) {
        int last_intr;

        CpuSuspendIntr(&last_intr);
        UnlinkImports((void *)module_info->text_start, module_info->text_size);
        UnlinkModule(module_info);
        FreeSysMemory((void *)((unsigned int)module_info >> 8 << 8));
        CpuResumeIntr(last_intr);
        return 0;
    }
    return 0;
}


int IsIllegalBootDevice(const char *arg1)
{
    // Pulled from UDNL
#if 0
    // Eliminate spaces from the path.
    if (arg1[0] == ' ') {
        arg1++;
        do {
            arg1++;
        } while (arg1[0] == ' ');

        arg1--;
    }

    if (((arg1[0] | 0x20) == 'm') && ((arg1[1] | 0x20) == 'c')) { //"mc"
        arg1 += 2;
        goto end_func1;
    } else if (((arg1[0] | 0x20) == 'h') && ((arg1[1] | 0x20) == 'd')) { //"hd"
        arg1 += 2;
        goto end_func1;
    } else if (((arg1[0] | 0x20) == 'n') && ((arg1[1] | 0x20) == 'e') && ((arg1[2] | 0x20) == 't')) { //"net"
        arg1 += 3;
        goto end_func1;
    } else if (((arg1[0] | 0x20) == 'd') && ((arg1[1] | 0x20) == 'e') && ((arg1[2] | 0x20) == 'v')) { //"dev"
        arg1 += 3;
        goto end_func1;
    } else
        return 0;

end_func1:
    return ((*arg1 - 0x30 < 0x0b) ? 1 : 0); // '0' to '9' and ':'
#endif
    // Unofficial: Always succeed
    return 0;
}

ModuleInfo_t *do_load(char *filename, void *buffer, void *addr, int offset, int *result_out)
{
    void *seek;
    void *v11;
    ModuleInfo_t *ExecutableObject;
    int state;

    *result_out = 0;
    if (IsIllegalBootDevice(filename)) {
        *result_out = KE_ILLEGAL_OBJECT;
        return 0;
    }
    seek = 0;
    if (filename) {
        seek = do_load_seek(filename, result_out);
        v11  = seek;
    } else {
        v11 = buffer;
    }
    if (v11 == 0) {
        return 0;
    }
    ExecutableObject = do_load_noseek(v11, addr, offset, result_out);
    if (seek) {
        CpuSuspendIntr(&state);
        FreeSysMemory(seek);
        CpuResumeIntr(state);
    }
    return ExecutableObject;
}

void *do_load_seek(const char *filename, int *result_out)
{
    int v3;
    int v4;
    void *v6;
    int state[2];

    *result_out = 0;
    v3          = open(filename, 1);
    if (v3 < 0) {
        *result_out = KE_NOFILE;
        return 0;
    }
    v4 = lseek(v3, 0, 2);
    if (v4 <= 0) {
        *result_out = KE_FILEERR;
        close(v3);
        return 0;
    }
    CpuSuspendIntr(state);
    v6 = AllocSysMemory(1, v4, 0);
    CpuResumeIntr(state[0]);
    if (!v6) {
        *result_out = KE_NO_MEMORY;
        close(v3);
        return 0;
    }
    lseek(v3, 0, 0);
    if (read(v3, v6, v4) == v4) {
        close(v3);
        return v6;
    }
    *result_out = KE_FILEERR;
    close(v3);
    CpuSuspendIntr(state);
    FreeSysMemory(v6);
    CpuResumeIntr(state[0]);
    return 0;
}

ModuleInfo_t *do_load_noseek(void *buffer, void *addr, int offset, int *result_out)
{
    ModuleInfo_t *v8;
    int v9;
    char *v10;
    void *v11;
    void *v12;
    int v13;
    u32 MemSize;
    void *v15;
    FileInfo_t v17;
    int state;

    v8 = 0;
    CpuSuspendIntr(&state);
    v9 = ReadModuleHeader(buffer, &v17);
    if (v9 == 2 || v9 == 4) {
        // Relocatable IRX.
        v13 = 0;
        if (addr) {
            v13 = 2;
            if (offset) {
                if (QueryBlockTopAddress(addr) == addr && QueryBlockSize(addr) >= (unsigned int)(offset + 48)) {
                    v17.text_start = addr;
                } else {
                    v17.text_start = 0;
                }
                goto LABEL_20;
            }
            MemSize = v17.MemSize;
            v15     = addr;
        } else {
            MemSize = v17.MemSize;
            v15     = 0;
        }
        v17.text_start = AllocSysMemory(v13, MemSize + 0x30, v15);
    LABEL_20:
        if (!v17.text_start) {
            *result_out = KE_NO_MEMORY;
            goto LABEL_26;
        }
        v17.text_start = (char *)v17.text_start + 0x30;
        v12            = buffer;
    } else if (v9 == 1 || v9 == 3) {
        // Fixed COFF or ELF.
        v10 = (char *)((((unsigned int)v17.text_start - 48) >> 8 << 8) & 0x1FFFFFFF);
        v11 = AllocSysMemory(2, (char *)v17.text_start + v17.MemSize - v10, v10);
        v12 = buffer;
        if (!v11) {
            if ((int)QueryBlockTopAddress(v10) > 0) {
                *result_out = KE_MEMINUSE;
                goto LABEL_26;
            }
            *result_out = KE_NO_MEMORY;
            goto LABEL_26;
        }
    } else {
        *result_out = KE_ILLEGAL_OBJECT;
        goto LABEL_26;
    }
    LoadModuleImage(v12, &v17);
    if (LinkImports(v17.text_start, v17.text_size) >= 0) {
        FlushIcache();
        v8 = (ModuleInfo_t *)((char *)v17.text_start - 48);
        LinkModule((ModuleInfo_t *)v17.text_start - 1);
    } else {
        FreeSysMemory((void *)(((unsigned int)v17.text_start - 48) >> 8 << 8));
        *result_out = KE_LINKERR;
    }
LABEL_26:
    CpuResumeIntr(state);
    return v8;
}

char *get_next_non_whitespace_string(char *a1)
{
    for (;;) {
        if (*a1 != ' ' && *a1 != '\t') {
            if (*a1 != '\n') {
                return a1;
            }
        }
        *a1++ = 0;
    }
}

char *get_non_null_string(char *a1)
{
    if (*a1 && *a1 != ' ' && *a1 != '\t') {
        int v3;

        do {
            if (*a1 == '\n') {
                break;
            }
            v3 = *++a1;
            if (!*a1) {
                break;
            }
            if (v3 == ' ') {
                break;
            }
        } while (v3 != '\t');
    }
    return a1;
}

void get_updater_boot_argument(char *a1, int *updater_argc, char **updater_argv, int updater_argv_count)
{
    char *next_non_whitespace_string;
    int v9;

    next_non_whitespace_string = get_next_non_whitespace_string(a1);
    v9                         = 0;
    while (*next_non_whitespace_string) {
        char *non_null_string;

        if (v9 >= updater_argv_count) {
            break;
        }
        *updater_argv++ = next_non_whitespace_string;
        non_null_string = get_non_null_string(next_non_whitespace_string);
        ++v9;
        if (!*non_null_string) {
            break;
        }
        *non_null_string           = 0;
        next_non_whitespace_string = get_next_non_whitespace_string(non_null_string + 1);
    }
    *updater_argc = v9;
}

void SetSecrmanCallbacks(SecrCardBootFile_callback_t SecrCardBootFile_fnc, SecrDiskBootFile_callback_t SecrDiskBootFile_fnc, SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_fnc)
{
    SecrCardBootFile_func_ptr     = SecrCardBootFile_fnc;
    SecrDiskBootFile_func_ptr     = SecrDiskBootFile_fnc;
    SetLoadfileCallbacks_func_ptr = SetLoadfileCallbacks_fnc;
}

void SetCheckKelfPathCallback(CheckKelfPath_callback_t CheckKelfPath_fnc)
{
    CheckKelfPath_func_ptr = CheckKelfPath_fnc;
}

void GetLoadfileCallbacks(CheckKelfPath_callback_t *CheckKelfPath_fnc, SetLoadfileCallbacks_callback_t *SetLoadfileCallbacks_fnc)
{
    *CheckKelfPath_fnc        = CheckKelfPath_func_ptr;
    *SetLoadfileCallbacks_fnc = SetLoadfileCallbacks_func_ptr;
}

// Pulled from UDNL
struct ssbus_regs
{
    volatile unsigned int *address, *delay;
};

// Pulled from UDNL
static struct ssbus_regs ssbus_regs[] = {
    {(volatile unsigned int *)0xbf801000,
     (volatile unsigned int *)0xbf801008},
    {(volatile unsigned int *)0xbf801400,
     (volatile unsigned int *)0xbf80100C},
    {(volatile unsigned int *)0xbf801404,
     (volatile unsigned int *)0xbf801014},
    {(volatile unsigned int *)0xbf801408,
     (volatile unsigned int *)0xbf801018},
    {(volatile unsigned int *)0xbf80140C,
     (volatile unsigned int *)0xbf801414},
    {(volatile unsigned int *)0xbf801410,
     (volatile unsigned int *)0xbf80141C},
    {NULL,
     NULL}};

// Pulled from UDNL
static volatile unsigned int *func_00000f80(volatile unsigned int *address)
{
    struct ssbus_regs *pSSBUS_regs;

    pSSBUS_regs = ssbus_regs;
    while (pSSBUS_regs->address != NULL) {
        if (pSSBUS_regs->address == address)
            break;
        pSSBUS_regs++;
    }

    return pSSBUS_regs->delay;
}

static const void *GetFileDataFromImage(const void *start, const void *end, const char *filename);

static void my_strcpy(char *dst, const char *src)
{
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
}

static int reboot_start_proc(const char *command, unsigned int flags)
{
    // Pulled from UDNL
    {
        lc_internals_t *LoadcoreData;
        iop_library_t *ModuleData, *NextModule;
        void **ExportTable;
        unsigned int enable_debug;

        enable_debug = flags & 0x80000000;
        if (enable_debug != 0)
            Kprintf(" ReBootStart: Terminate resident Libraries\n");

        if ((LoadcoreData = GetLoadcoreInternalData()) != NULL) {
            ModuleData = LoadcoreData->let_next;
            while (ModuleData != NULL) {
                NextModule = ModuleData->prev;

                ExportTable = ModuleData->exports;
                if (ExportTable[1] != NULL && ExportTable[2] != NULL) {
                    int (*pexit)(int arg1);

                    pexit = ExportTable[2];
                    if (enable_debug != 0)
                        Kprintf("  %.8s %x \n", ModuleData->name, pexit);
                    pexit(0);
                }

                ModuleData = NextModule;
            }
        }
    }

    // Pulled from UDNL
    {
        int prid;
        volatile unsigned int **pReg;

        asm volatile("mfc0 %0, $15"
                     : "=r"(prid)
                     :);

        if (!(flags & 1)) {
            pReg = (prid < 0x10 || ((*(volatile unsigned int *)0xbf801450) & 8)) ? *(volatile unsigned int ***)0xbfc02008 : *(volatile unsigned int ***)0xbfc0200C;

            while (pReg[0] != 0) {
                if (func_00000f80(pReg[0]) != 0)
                    pReg[0] = (void *)0xFF;
                pReg[0] = pReg[1];
                pReg += 2;
            }
        }
        if (!(flags & 2)) {
            SetCacheCtrl((prid < 0x10 || ((*(volatile unsigned int *)0xbf801450) & 8)) ? *(volatile unsigned int *)0xbfc02010 : *(volatile unsigned int *)0xbfc02014);
        }
    }

    // MODLOAD specific
    {
        const char *iopboot_entrypoint;
        u32 ram_size_in_mb;
        int v18;
        char *command_ptr;

        iopboot_entrypoint = GetFileDataFromImage((const void *)0xBFC00000, (const void *)0xBFC10000, "IOPBOOT");
        // Unofficial: Check if command is NULL befire checking its contents
        if (command && command[0]) {
            my_strcpy((char *)0x480, command);
            ram_size_in_mb = (QueryMemSize() + 0x100) >> 20;
            v18            = (flags & 0xFF00) | 2;
            command_ptr    = (char *)0x480;
        } else {
            ram_size_in_mb = (QueryMemSize() + 0x100) >> 20;
            v18            = 1;
            command_ptr    = 0;
        }
        return ((int (*)(u32, int, char *, u32))iopboot_entrypoint)(ram_size_in_mb, v18, command_ptr, 0);
    }
}

// Exactly the same function as INTRMAN's export 14.
// Defined in modload_asm.S
extern int CpuExecuteKmode(void *func, ...);

int ReBootStart(const char *command, unsigned int flags)
{
    return CpuExecuteKmode(reboot_start_proc, command, flags);
}

struct RomDirEntry
{
    char name[10];
    unsigned short int ExtInfoEntrySize;
    unsigned int size;
};

// Similar to the function in UDNL and ROMDRV.
static const void *GetFileDataFromImage(const void *start, const void *end, const char *filename)
{
    const u8 *ImageStart;
    const u8 *RomdirStart;

    ImageStart  = NULL;
    RomdirStart = NULL;
    {
        const u32 *ptr;
        unsigned int offset;
        const struct RomDirEntry *file;

        offset = 0;
        file   = (struct RomDirEntry *)start;
        for (; file < (const struct RomDirEntry *)end; file++, offset += sizeof(struct RomDirEntry)) {
            /* Check for a valid ROM filesystem (Magic: "RESET\0\0\0\0\0"). Must have the right magic and bootstrap code size (size of RESET = bootstrap code size). */
            ptr = (u32 *)file->name;
            if (ptr[0] == 0x45534552 && ptr[1] == 0x54 && (*(u16 *)&ptr[2] == 0) && (((file->size + 15) & ~15) == offset)) {
                ImageStart  = start;
                RomdirStart = (const u8 *)ptr;
                break;
            }
        }
    }
    {
        unsigned int i, offset;
        u8 filename_temp[12];

        offset                    = 0;
        ((u32 *)filename_temp)[0] = 0;
        ((u32 *)filename_temp)[1] = 0;
        ((u32 *)filename_temp)[2] = 0;
        for (i = 0; *filename >= 0x21 && i < sizeof(filename_temp); i++) {
            filename_temp[i] = *filename;
            filename++;
        }

        if (RomdirStart != NULL) {
            const struct RomDirEntry *RomdirEntry;

            RomdirEntry = (const struct RomDirEntry *)RomdirStart;

            do { // Fast comparison of filenames.
                if (((u32 *)filename_temp)[0] == ((u32 *)RomdirEntry->name)[0] && ((u32 *)filename_temp)[1] == ((u32 *)RomdirEntry->name)[1] && (*(u16 *)&((u32 *)filename_temp)[2] == *(u16 *)&((u32 *)RomdirEntry->name)[2])) {
                    return ImageStart + offset;
                }

                offset += (RomdirEntry->size + 15) & ~15;
                RomdirEntry++;
            } while (((u32 *)RomdirEntry->name)[0] != 0x00000000); // Until the terminator entry is reached.
        }
    }
    return NULL;
}
