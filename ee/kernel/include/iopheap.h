#ifndef _IOP_HEAP_H_
#define _IOP_HEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

int SifInitIopHeap(void);
void SifExitIopHeap(void);

void * SifAllocIopHeap(int size);
int SifFreeIopHeap(void *addr);

int SifLoadIopHeap(const char *path, void *addr);

#ifdef __cplusplus
}
#endif

#endif
