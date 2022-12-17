/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <sysclib.h>
#include <hdd-ioctl.h>

#include "apa-opt.h"
#include "libapa.h"
#include "hdsk-devctl.h"
#include "hdsk.h"
#include "sim.h"

extern u32 TotalCopied;
extern struct hdskBitmap hdskBitmap[];

void hdskSimGetFreeSectors(s32 device, struct hdskStat *stat, apa_device_t *deviceinfo)
{
    u32 sectors, partMax;
    int i;

    sectors    = 0;
    stat->free = 0;
    for (i = 0; hdskBitmap[i].next != hdskBitmap; sectors += hdskBitmap[i].length, i++) {
        if (hdskBitmap[i].type == 0) {
            if ((0x001FFFFF < hdskBitmap[i].length) || ((stat->free & hdskBitmap[i].length) == 0)) {
                stat->free += hdskBitmap[i].length;
            }
            sectors += hdskBitmap[i].length;
        }
    }

    for (partMax = deviceinfo[device].partitionMaxSize; 0x0003FFFF < partMax; partMax = deviceinfo[device].partitionMaxSize) { // As weird as it looks, this was how it was done in the original HDD.IRX.
        for (; 0x0003FFFF < partMax; partMax /= 2) {
            // Non-SONY: Perform 64-bit arithmetic here to avoid overflows when dealing with large disks.
            if ((sectors % partMax == 0) && ((u64)sectors + partMax < deviceinfo[device].totalLBA)) {
                if ((0x001FFFFF < partMax) || (stat->free & partMax) == 0) {
                    stat->free += partMax;
                }
                sectors += partMax;
                break;
            }
        }

        if (0x0003FFFF >= partMax) {
            break;
        }
    }

    APA_PRINTF(APA_DRV_NAME ": total = %08lx sectors, installable = %08lx sectors.\n", sectors, stat->free);
}

static void hdskSimClearHeader(struct hdskBitmap *header)
{
    memset(header, 0, sizeof(struct hdskBitmap));
}

static struct hdskBitmap *hdskSimDeleteFixPrev(struct hdskBitmap *part)
{
    struct hdskBitmap *prev;
    u32 length;

    while (hdskBitmap[0].next != part) {
        prev = part->prev;
        if (prev->type == 0) {
            length = prev->length + part->length;
            if (prev->start % length != 0) {
                break;
            }

            if ((length & (length - 1)) == 0) {
                prev->length = length;
                hdskSimClearHeader((struct hdskBitmap *)apaCacheUnLink((apa_cache_t *)part));
            } else {
                break;
            }
        } else {
            break;
        }

        part = prev;
    }

    return part;
}

static struct hdskBitmap *hdskSimDeleteFixNext(struct hdskBitmap *part)
{
    struct hdskBitmap *next;
    u32 length;

    while (hdskBitmap[0].prev != part) {
        next = part->next;
        if (next->type == 0) {
            length = next->length + part->length;
            if (part->start % length != 0) {
                break;
            }

            if ((length & (length - 1)) == 0) {
                part->length = length;
                part->type   = 0;
                hdskSimClearHeader((struct hdskBitmap *)apaCacheUnLink((apa_cache_t *)part));
            } else {
                break;
            }
        } else {
            break;
        }
    }

    return part;
}

static void hdskSimSwapPartition(struct hdskBitmap *start)
{
    u32 OrigStart, OrigLen;
    struct hdskBitmap *part;

    part = hdskBitmap[0].prev;
    if (start == part) {
        do {
            hdskSimClearHeader((struct hdskBitmap *)apaCacheUnLink((apa_cache_t *)part));
            part = hdskBitmap[0].prev;
            if (part == NULL) {
                break;
            }
        } while (part->type != 0);
    } else {
        int i;

        OrigStart = start->start;
        OrigLen   = start->length;
        for (i = 0; i < 2; i++) {
            start = hdskSimDeleteFixNext(hdskSimDeleteFixPrev(start));
        }

        if (start->start == OrigStart && start->length == OrigLen) {
            start->type = 0;
        }
    }
}

void hdskSimMovePartition(struct hdskBitmap *dest, struct hdskBitmap *start)
{
    APA_PRINTF("MovePartition: %08lx to %08lx. sector count = %08lx.\n", start->start, dest->start, start->length);

    TotalCopied += start->length;
    APA_PRINTF("swap partition start...");

    dest->type = start->type;
    hdskSimSwapPartition(start);

    printf("done\n");
    APA_PRINTF("MovePartition: done\n");
}

struct hdskBitmap *hdskSimFindEmptyPartition(u32 size)
{
    struct hdskBitmap *part;

    for (part = hdskBitmap; part->next != hdskBitmap; part = part->next) {
        if (part->length == size && part->type == 0) {
            APA_PRINTF("sim: found empty partition at %08lx, size %08lx.\n", part->start, part->length);
            return part;
        }
    }

    return NULL;
}

static int hdskCheckIfPrevPartEmpty(struct hdskBitmap *part, u32 size)
{
    return ((part->prev->length == size) && (part->prev->type == 0));
}

static int hdskCheckIfNextPartEmpty(struct hdskBitmap *part, u32 size)
{
    return ((part->next->length == size) && (part->next->type == 0));
}

struct hdskBitmap *hdskSimFindLastUsedPartition(u32 size, u32 start, int mode)
{
    u32 SliceSize;
    struct hdskBitmap *part;

    SliceSize = size * 2;
    for (part = hdskBitmap[0].prev; start < part->start && part != hdskBitmap; part = part->prev) {
        if (part->length == size && part->type != 0) {
            if (mode != 0) {
                if (part->start % SliceSize != 0) {
                    if (hdskCheckIfPrevPartEmpty(part, size) != 0) {
                        return part;
                    }
                } else {
                    if (part == hdskBitmap[0].prev || hdskCheckIfNextPartEmpty(part, size) != 0) {
                        return part;
                    }
                }
            } else {
                return part;
            }
        }
    }

    return NULL;
}

static int hdskCheckIsLastPart(struct hdskBitmap *part, u32 size, int mode)
{
    struct hdskBitmap *CurrPart;
    u32 PartSize;

    for (CurrPart = part, PartSize = part->length; (PartSize < size) && (CurrPart != hdskBitmap); PartSize += CurrPart->length) {
        CurrPart = CurrPart->next;
        if (CurrPart->type == 0) {
            break;
        }
    }

    if (PartSize == size) {
        if (mode != 0) {
            if (part->start % (size * 2) != 0) {
                return (hdskCheckIfPrevPartEmpty(part, size) == 0 ? 0 : 1);
            } else {
                return (CurrPart != hdskBitmap[0].prev ? (hdskCheckIfNextPartEmpty(CurrPart, size) != 0) : 1);
            }
        } else {
            return 1;
        }
    }

    return 0;
}

struct hdskBitmap *hdskSimFindLastUsedBlock(u32 size, u32 start, int mode)
{
    struct hdskBitmap *part;

    for (part = hdskBitmap[0].prev; start < part->start && part != hdskBitmap; part = part->prev) {
        if ((part->start % size == 0) && (part->length < size) && (part->type != 0)) {
            if (hdskCheckIsLastPart(part, size, mode) != 0) {
                return part;
            }
        }
    }

    return NULL;
}

static struct hdskBitmap *hdskBitmapAlloc(void)
{
    struct hdskBitmap *part;
    unsigned int i;

    for (part = &hdskBitmap[1], i = 1; i < HDSK_BITMAP_SIZE; i++, part++) {
        if (part->length == 0) {
            return part;
        }
    }

    return NULL;
}

static void hdskSimSplitEmptyPartition(struct hdskBitmap *part, u32 length)
{
    APA_PRINTF("split empty partition.\n");
    while (part->length != length) {
        struct hdskBitmap *end;

        end = hdskBitmapAlloc();
        part->length /= 2;
        end->start  = part->start + part->length;
        end->type   = 0;
        end->length = part->length;
        apaCacheLink((apa_cache_t *)part, (apa_cache_t *)end);
    }
}

void hdskSimMovePartitionsBlock(struct hdskBitmap *dest, struct hdskBitmap *src)
{
    u32 size;

    size = dest->length;
    APA_PRINTF("MovePartitionsBlock: %08lx to %08lx. sector count = %08lx.\n", src->start, dest->start, dest->length);
    do {
        hdskSimSplitEmptyPartition(dest, src->length);
        if (src->next == hdskBitmap) {
            break;
        }

        hdskSimMovePartition(dest, src);
        dest = dest->next;
        size -= dest->length;
        if (size == 0) {
            break;
        }

        src = src->next;
    } while (dest != hdskBitmap);

    APA_PRINTF("MovePartitionsBlock: done\n");
}
