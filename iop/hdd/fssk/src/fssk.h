/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __FSSK_H__
#define __FSSK_H__

typedef struct
{
    u32 totalLBA;
    u32 partitionMaxSize;
    int format;
    int status;
} hdd_device_t;

struct hdskBitmap
{
    struct hdskBitmap *next; // 0x00
    struct hdskBitmap *prev; // 0x04
    u32 start;               // 0x08
    u32 length;              // 0x0C
    u32 type;                // 0x10
};

#define HDSK_BITMAP_SIZE 0x4001

#endif
