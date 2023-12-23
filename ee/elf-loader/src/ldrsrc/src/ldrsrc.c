/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <elf_loader_common.h>
#include <string.h>
#include <stdint.h>
#include <tamtypes.h>
#include <kernel.h>
#include <ps2sdkapi.h>

#ifdef LOADER_ENABLE_DEBUG_COLORS
#define SET_GS_BGCOLOUR(colour) do {*((volatile unsigned long int *)0x120000E0) = colour;} while(0)
#else
#define SET_GS_BGCOLOUR(colour) do {} while(0)
#endif

// Color status helper in BGR format
#define WHITE_BG 0xFFFFFF // start main
#define CYAN_BG 0xFFFF00 // move memory
#define GREEN_BG 0x00FF00 // set memory to 0
#define RED_BG 0x0000FF // never encountered execution command
#define MAGENTA_BG 0xFF00FF // malformed loader info
#define BROWN_BG 0x2A2AA5  // before FlushCache
#define PURPLE_BG 0x800080  // before ExecPS2

//--------------------------------------------------------------
// Redefinition of init/deinit libc:
//--------------------------------------------------------------
// DON'T REMOVE, as it is for reducing binary size. 
// These functions are defined as weak in /libc/src/init.c
//--------------------------------------------------------------
void _libcglue_init() {}
void _libcglue_deinit() {}

DISABLE_PATCHED_FUNCTIONS();
DISABLE_EXTRA_TIMERS_FUNCTIONS();
PS2_DISABLE_AUTOSTART_PTHREAD();

elf_loader_execinfo_t ldr_userarg __attribute__((section(".ldr_userarg")));

static void ldr_proc(void)
{
    elf_loader_execinfo_t *execinfo;
    elf_loader_loaderinfo_t *ldrinfo;
    elf_loader_arginfo_t *arginfo;

    SET_GS_BGCOLOUR(WHITE_BG);
    execinfo = (elf_loader_execinfo_t *)&ldr_userarg;
    ldrinfo = &(execinfo->loaderinfo);
    arginfo = &(execinfo->arginfo);

    {
        int i;
        for (i = 0; i < ldrinfo->count; i += 1) {
            elf_loader_loaderinfo_item_t *item;

            item = &(ldrinfo->items[i]);
            if (item->dest_addr != NULL && item->src_addr != NULL && item->size != 0) {
                SET_GS_BGCOLOUR(CYAN_BG);
                memmove(item->dest_addr, item->src_addr, item->size);
            }
            else if (item->dest_addr != NULL && item->src_addr == NULL && item->size != 0) {
                SET_GS_BGCOLOUR(GREEN_BG);
                if (item->size == 0xFFFFFFFF) {
                    memset(item->dest_addr, 0, ((u32)item->dest_addr) - GetMemorySize());
                }
                else {
                    memset(item->dest_addr, 0, item->size);
                }
            }
            else if (item->dest_addr != NULL && item->src_addr != NULL && item->size == 0) {
                SET_GS_BGCOLOUR(BROWN_BG);
                FlushCache(0);
                FlushCache(2);
                SET_GS_BGCOLOUR(PURPLE_BG);
                ExecPS2(item->dest_addr, item->src_addr, arginfo->argc, arginfo->argv);
            }
            else {
                SET_GS_BGCOLOUR(MAGENTA_BG);
                break;
            }
        }
    }
    SET_GS_BGCOLOUR(RED_BG);
    Exit(126);
}

extern char* _end;
extern char* _heap_size;
extern char* _stack;
extern char* _stack_size;

static void ldr_entrypoint_stack();

static char args_storage[324];

// Modified crt0
// bss zero is skipped, and argument handling removed
static void ldr_entrypoint(void)
{
    asm volatile(
        // SetupThread
        "move   $4, $0          \n"
        "la     $5, _stack      \n"
        "la     $6, _stack_size \n"
        // TODO: can we set args to NULL?
        "la     $7, %0          \n"
        "la     $8, ExitThread  \n"
        "move   $gp, $4         \n"
        "addiu  $3, $0, 60      \n"
        "syscall                \n"
        "move   $sp, $2         \n"
        // Jump to entrypoint that can use stack
        "j      %1           \n"
        : // No output registers
        : "R"(args_storage), "Csy"(ldr_entrypoint_stack));
}

static void ldr_entrypoint_stack()
{
    SetupHeap(&_end, (int)&_heap_size);

    FlushCache(0);

    ldr_proc();
}

static const void *ldr_internalinfo[] = {
    &ldr_userarg,
    &ldr_entrypoint,
};

void *ldr_getinternalinfo(void) __attribute__((section(".start")));

void *ldr_getinternalinfo(void)
{
    return &ldr_internalinfo;
}
