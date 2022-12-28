/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Malloc tester
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <kernel.h>
#include <rom0_info.h>

#if defined(SCREEN_DEBUG)
#include <unistd.h>
#include <debug.h>
#endif

extern void __start(void);
extern void *_end;
extern void *_heap_size;

#define MB_SIZE 1024 * 1024
#define KB_SIZE 1024

#define SIZE_PATTERN 2

#if defined(SCREEN_DEBUG)
#define custom_printf(args...) \
    printf(args);              \
    scr_printf(args);
#else
#define custom_printf(args...) printf(args);
#endif

#if !defined(SCREEN_DEBUG)
// sleep requires it
void _libcglue_init() {}
void _libcglue_deinit() {}
#endif

void _libcglue_timezone_update() {}
void _libcglue_rtc_update() {}

// "weak" function called by crt0.o
void _ps2sdk_memory_init()
{
    if (GetMemorySize() == (32 * 1024 * 1024) && IsDESRMachine() == 1) {
        // Switch to 64MiB mode
        SetMemoryMode(0);
        _InitTLB();
        // Restart application
        __start();
    }
}

static int max_malloc(size_t initial_value, int increment, const char *desc)
{
    char *p_block;
    size_t chunk = initial_value;

    custom_printf("Check maximum contigous block we can allocate (%s accurate)\n", desc);
    while ((p_block = malloc(++chunk * increment)) != NULL) {
        free(p_block);
    }
    chunk--;
#if defined(VERBOSE)
    custom_printf("Maximum possible %s we can allocate is %zu\n", desc, chunk);
#endif

    return chunk;
}

static int mem_integrity(char *p_block, size_t initial_value, size_t end_value, int increment, const char *desc)
{
    unsigned char patterns[SIZE_PATTERN] = {0xA, 0x5};
    int i, j, failures;
    char *block_start, *block_tmp;
#if defined(VERBOSE)
    char *block_end;
#endif

    failures = 0;

    custom_printf("Checking %s chunks...\n", desc);
    for (i = initial_value; i < end_value; i++) {
        for (j = 0; j < SIZE_PATTERN; j++) {
            block_start = p_block + i * increment;
#if defined(VERBOSE)
            block_end = block_start + increment;
            custom_printf("Checking from %p to %p with pattern 0x%X\n", block_start, block_end, patterns[j]);
#endif
            memset(block_start, patterns[j], increment);
            for (int x = 0; x < increment; x++) {
                block_tmp = block_start + x;
                if (block_tmp[0] != patterns[j]) {
                    failures++;
                    custom_printf("Failure, mem pos: %p\n", block_tmp);
                    custom_printf("Expected value 0x%X, readed: 0x%X\n", patterns[j], block_tmp[0]);
                }
            }
        }
    }

    return failures;
}

int main(int argc, char *argv[])
{
    int failures;
    size_t size_mb, size_kb, size_b;
    char *p_block;

    failures = 0;
    size_mb = 0;
    size_kb = 0;
    size_b = 0;

#if defined(SCREEN_DEBUG)
    init_scr();
    sleep(3);
    scr_printf("\n\nStarting MEM TESTS...\n");
#endif

    custom_printf("Program: [%p, %p], program size %i, heap size %p\n", &__start, &_end, (int)&_end - (int)&__start, &_heap_size);
    custom_printf("EndOfHeap %p, memorySize %i, machineType %i\n", EndOfHeap(), GetMemorySize(), MachineType());
    custom_printf("Stack start at %p\n", &failures);

    size_mb = max_malloc(size_mb, MB_SIZE, "MB");

    size_kb = size_mb * KB_SIZE;
    size_kb = max_malloc(size_kb, KB_SIZE, "KB");

    size_b = size_kb * KB_SIZE;
    size_b = max_malloc(size_b, 1, "Bytes");

    custom_printf("Start memory integration\n");
    p_block = malloc(size_b);

    failures += mem_integrity(p_block, 0, size_mb, MB_SIZE, "MB");
    failures += mem_integrity(p_block, size_mb * KB_SIZE, size_kb, KB_SIZE, "KB");
    failures += mem_integrity(p_block, size_kb * KB_SIZE, size_b, 1, "Bytes");

    free(p_block);
    custom_printf("End memory integration\n");

    custom_printf("\n\n\n----------SUMMARY-----------\n");
    custom_printf("Maximum possible MB we can allocate is %zu MB\n", size_mb);
    custom_printf("Maximum possible KB we can allocate is %zu KB\n", size_kb);
    custom_printf("Maximum possible Bytes we can allocate is %zu Bytes\n", size_b);
    if (failures) {
        custom_printf("Memory integrity: Opps :'( The memory failed checking bytes, %i times\n", failures);
    } else {
        custom_printf("Memory integrity: The memory is working as expected!\n");
    }
    custom_printf("----------------------------\n\n\n");

    SleepThread();
    return 0;
}
