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

int SifInitIopHeap(void);
void SifExitIopHeap(void);

void *SifAllocIopHeap(int size);
int SifFreeIopHeap(void *addr);

int SifLoadIopHeap(const char *path, void *addr);

#ifdef __cplusplus
}
#endif

#endif /* __IOPHEAP_H__ */
