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

#if defined(SCREEN_DEBUG)
#include <unistd.h>
#include <debug.h>
#endif

#define MB_SIZE 1024 * 1024
#define KB_SIZE 1024

#define SIZE_PATTERN 2
static unsigned char patterns[SIZE_PATTERN] = { 0xA, 0x5 };

#if defined(SCREEN_DEBUG)
#define custom_printf(args...) printf(args); scr_printf(args);
#else
#define custom_printf(args...) printf(args); 
#endif

int main(int argc, char *argv[]) {
    int i, j;
    size_t size_mb, size_kb, size_b;
    char *p_block, *block_start, *block_tmp;
#if VERBOSE
    char *block_end;
#endif

#if defined(SCREEN_DEBUG)
   init_scr();
   sleep(3);
   scr_printf("\n\nStarting MEM TESTS...\n");
#endif
    
    custom_printf("Stack start at %p\n", &i);
    size_mb = 0;
    size_kb = 0;
    size_b = 0;

    custom_printf("Check maximum contigous block we can allocate (MB accurate)\n");
    while ( (p_block = malloc(++size_mb * MB_SIZE)) != NULL) {
        free(p_block);
    }
    size_mb--;
    custom_printf("Maximum possible MB we can allocate is %i\n", size_mb);

    custom_printf("Check maximum contigous block we can allocate (KB accurate)\n");
    size_kb = size_mb * KB_SIZE;
    while ( (p_block = malloc(++size_kb * KB_SIZE)) != NULL) {
        free(p_block);
    }
    size_kb--;
    custom_printf("Maximum possible KB we can allocate is %i\n", size_kb);

    custom_printf("Check maximum contigous block we can allocate (Byte accurate)\n");
    size_b = size_kb * KB_SIZE;
    while ( (p_block = malloc(++size_b)) != NULL) {
        free(p_block);
    }
    size_b--;
    custom_printf("Maximum possible Bytes we can allocate is %i\n", size_b);


    custom_printf("Start memory integration\n");
    p_block = malloc(size_b);

    custom_printf("Checking MB chunks...\n");
    for (i = 0; i < size_mb; i++) {
        for (j = 0; j < SIZE_PATTERN; j++) {
            block_start = p_block + (i * MB_SIZE);
#if VERBOSE
            block_end = p_block + ((i+1) * MB_SIZE);
            custom_printf("Checking from %p to %p with pattern 0x%X\n", block_start, block_end, patterns[j]);
#endif
            memset(block_start, patterns[j], MB_SIZE);
            for(int x = 0; x < MB_SIZE; x++) {
                block_tmp = block_start + x; 
                assert(block_tmp[0] == patterns[j]);
            }
        }
    }
    custom_printf("End of Checking MB chunks...\n");

    custom_printf("Checking KB chunks...\n");
    for (i = size_mb * KB_SIZE; i < size_kb; i++) {
        for (j = 0; j < SIZE_PATTERN; j++) {
            block_start = p_block + (i * KB_SIZE);
#if VERBOSE
            block_end = p_block + ((i+1) * KB_SIZE);
            custom_printf("Checking from %p to %p with pattern 0x%X\n", block_start, block_end, patterns[j]);
#endif
            memset(block_start, patterns[j], KB_SIZE);
            for(int x = 0; x < KB_SIZE; x++) {
                block_tmp = block_start + x; 
                assert(block_tmp[0] == patterns[j]);
            }
        }
    }
    custom_printf("End of Checking KB chunks...\n");

    custom_printf("Checking Byte chunks...\n");
    for (i = size_kb * KB_SIZE; i < size_b; i++) {
        for (j = 0; j < SIZE_PATTERN; j++) {
            block_start = p_block + i;
#if VERBOSE
            block_end = p_block + (i+1);
            custom_printf("Checking from %p to %p with pattern 0x%X\n", block_start, block_end, patterns[j]);
#endif
            memset(block_start, patterns[j], 1);
            assert(block_start[0] == patterns[j]);
        }
    }
    custom_printf("End of Checking Byte chunks...\n");

    free(p_block);
    custom_printf("End memory integration\n");

    custom_printf("\n\n\n----------SUMMARY-----------\n");
    custom_printf("Maximum possible MB we can allocate is %i MB\n", size_mb);
    custom_printf("Maximum possible KB we can allocate is %i KB\n", size_kb);
    custom_printf("Maximum possible Bytes we can allocate is %i Bytes\n", size_b);
    custom_printf("Memory integrity: The memory is working as expected!\n");
    custom_printf("----------------------------\n\n\n");

    SleepThread();
    return 0;
}