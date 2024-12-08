/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP heap handling prototypes
 */

#ifndef __IOPHEAP_H__
#define __IOPHEAP_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int SifInitIopHeap(void);
extern void SifExitIopHeap(void);

extern void *SifAllocIopHeap(int size);
extern int SifFreeIopHeap(void *addr);

extern int SifLoadIopHeap(const char *path, void *addr);

#ifdef __cplusplus
}
#endif

#endif /* __IOPHEAP_H__ */
