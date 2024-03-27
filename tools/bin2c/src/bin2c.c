/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2024, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __MINGW32__
#define USE_MMAP
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef USE_MMAP
#include <sys/mman.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char *const hex_lut[256] =
    {
        "0x00, ", "0x01, ", "0x02, ", "0x03, ", "0x04, ", "0x05, ", "0x06, ", "0x07, ", "0x08, ", "0x09, ", "0x0a, ", "0x0b, ", "0x0c, ", "0x0d, ", "0x0e, ", "0x0f, ",
        "0x10, ", "0x11, ", "0x12, ", "0x13, ", "0x14, ", "0x15, ", "0x16, ", "0x17, ", "0x18, ", "0x19, ", "0x1a, ", "0x1b, ", "0x1c, ", "0x1d, ", "0x1e, ", "0x1f, ",
        "0x20, ", "0x21, ", "0x22, ", "0x23, ", "0x24, ", "0x25, ", "0x26, ", "0x27, ", "0x28, ", "0x29, ", "0x2a, ", "0x2b, ", "0x2c, ", "0x2d, ", "0x2e, ", "0x2f, ",
        "0x30, ", "0x31, ", "0x32, ", "0x33, ", "0x34, ", "0x35, ", "0x36, ", "0x37, ", "0x38, ", "0x39, ", "0x3a, ", "0x3b, ", "0x3c, ", "0x3d, ", "0x3e, ", "0x3f, ",
        "0x40, ", "0x41, ", "0x42, ", "0x43, ", "0x44, ", "0x45, ", "0x46, ", "0x47, ", "0x48, ", "0x49, ", "0x4a, ", "0x4b, ", "0x4c, ", "0x4d, ", "0x4e, ", "0x4f, ",
        "0x50, ", "0x51, ", "0x52, ", "0x53, ", "0x54, ", "0x55, ", "0x56, ", "0x57, ", "0x58, ", "0x59, ", "0x5a, ", "0x5b, ", "0x5c, ", "0x5d, ", "0x5e, ", "0x5f, ",
        "0x60, ", "0x61, ", "0x62, ", "0x63, ", "0x64, ", "0x65, ", "0x66, ", "0x67, ", "0x68, ", "0x69, ", "0x6a, ", "0x6b, ", "0x6c, ", "0x6d, ", "0x6e, ", "0x6f, ",
        "0x70, ", "0x71, ", "0x72, ", "0x73, ", "0x74, ", "0x75, ", "0x76, ", "0x77, ", "0x78, ", "0x79, ", "0x7a, ", "0x7b, ", "0x7c, ", "0x7d, ", "0x7e, ", "0x7f, ",
        "0x80, ", "0x81, ", "0x82, ", "0x83, ", "0x84, ", "0x85, ", "0x86, ", "0x87, ", "0x88, ", "0x89, ", "0x8a, ", "0x8b, ", "0x8c, ", "0x8d, ", "0x8e, ", "0x8f, ",
        "0x90, ", "0x91, ", "0x92, ", "0x93, ", "0x94, ", "0x95, ", "0x96, ", "0x97, ", "0x98, ", "0x99, ", "0x9a, ", "0x9b, ", "0x9c, ", "0x9d, ", "0x9e, ", "0x9f, ",
        "0xa0, ", "0xa1, ", "0xa2, ", "0xa3, ", "0xa4, ", "0xa5, ", "0xa6, ", "0xa7, ", "0xa8, ", "0xa9, ", "0xaa, ", "0xab, ", "0xac, ", "0xad, ", "0xae, ", "0xaf, ",
        "0xb0, ", "0xb1, ", "0xb2, ", "0xb3, ", "0xb4, ", "0xb5, ", "0xb6, ", "0xb7, ", "0xb8, ", "0xb9, ", "0xba, ", "0xbb, ", "0xbc, ", "0xbd, ", "0xbe, ", "0xbf, ",
        "0xc0, ", "0xc1, ", "0xc2, ", "0xc3, ", "0xc4, ", "0xc5, ", "0xc6, ", "0xc7, ", "0xc8, ", "0xc9, ", "0xca, ", "0xcb, ", "0xcc, ", "0xcd, ", "0xce, ", "0xcf, ",
        "0xd0, ", "0xd1, ", "0xd2, ", "0xd3, ", "0xd4, ", "0xd5, ", "0xd6, ", "0xd7, ", "0xd8, ", "0xd9, ", "0xda, ", "0xdb, ", "0xdc, ", "0xdd, ", "0xde, ", "0xdf, ",
        "0xe0, ", "0xe1, ", "0xe2, ", "0xe3, ", "0xe4, ", "0xe5, ", "0xe6, ", "0xe7, ", "0xe8, ", "0xe9, ", "0xea, ", "0xeb, ", "0xec, ", "0xed, ", "0xee, ", "0xef, ",
        "0xf0, ", "0xf1, ", "0xf2, ", "0xf3, ", "0xf4, ", "0xf5, ", "0xf6, ", "0xf7, ", "0xf8, ", "0xf9, ", "0xfa, ", "0xfb, ", "0xfc, ", "0xfd, ", "0xfe, ", "0xff, "};

int main(int argc, char *argv[])
{
    int src_fd;
    int dst_fd;

    if (argc != 4)
    {
        printf("bin2c - from bin2s By Sjeep\n"
               "Modified by Fobes/ps2dev\n\n"
               "Usage: bin2c infile outfile label\n\n");
        return 1;
    }

    if ((src_fd = open(argv[1], O_RDONLY, (mode_t)0600)) == -1)
    {
        printf("Error opening %s for reading.\n", argv[1]);
        return 1;
    }

    struct stat src_stat;
    fstat(src_fd, &src_stat);

    if(src_stat.st_size == 0)
    {
        printf("Error: %s is empty.\n", argv[1]);
        return 1;
    }
#ifdef USE_MMAP
    u_int8_t *src_map = mmap(0, src_stat.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);

    if (src_map == MAP_FAILED)
    {
        printf("Failed to map input file.\n");
        return 1;
    }
#endif
    if ((dst_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644)) == -1)
    {
        printf("Failed to open/create %s.\n", argv[2]);
        return 1;
    }

    char prologue[1024];
    snprintf(prologue, 1024, "#ifndef __%s__\n"
                             "#define __%s__\n\n"
                             "unsigned int size_%s = %d;\n"
                             "unsigned char %s[] __attribute__((aligned(16))) = {",
             argv[3], argv[3], argv[3], (int)src_stat.st_size, argv[3]);

    const char *epilogue = "\n};\n\n#endif\n";

    const size_t size_prologue = strlen(prologue);
    const size_t size_epilogue = strlen(epilogue);
    size_t dst_total_size = size_prologue + size_epilogue + (src_stat.st_size * 6) + (src_stat.st_size / 16 * 2) + ((src_stat.st_size % 16) ? 2 : 0);

    lseek(dst_fd, dst_total_size - 1, SEEK_SET);
    write(dst_fd, "", 1);
    lseek(dst_fd, 0, SEEK_SET);

#ifdef USE_MMAP
    char *dst_map = mmap(0, dst_total_size, PROT_WRITE, MAP_SHARED, dst_fd, 0);
    char *dst_map_start = dst_map;
    if (dst_map == MAP_FAILED)
    {
        printf("Failed to map output file.\n");
        close(dst_fd);
        return 1;
    }

    memcpy(dst_map, prologue, size_prologue);
    dst_map += strlen(prologue);

    for (int i = 0; i < src_stat.st_size; i++)
    {
        if ((i % 16) == 0)
        {
            *dst_map++ = '\n';
            *dst_map++ = '\t';
        }
        memcpy(dst_map, hex_lut[src_map[i]], 6);
        dst_map += 6;
    }

    memcpy(dst_map, epilogue, size_epilogue);

    msync(dst_map_start, dst_total_size, MS_SYNC);
    munmap(dst_map_start, dst_total_size);
    munmap(src_map, src_stat.st_size);
#else
    write(dst_fd, prologue, size_prologue);
    for (int i = 0; i < src_stat.st_size; i++)
    {
        if ((i % 16) == 0)
        {
            write(dst_fd, "\n\t", 2);
        }
        unsigned char byte;
        read(src_fd, &byte, 1);
        write(dst_fd, hex_lut[byte], 6);
    }
    write(dst_fd, epilogue, size_epilogue);
#endif

    close(dst_fd);
    close(src_fd);
    return 0;
}
