/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __CACHEIO_H__
#define __CACHEIO_H__

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int cacheioInit(void);
void cacheioExit(void);
int cacheioOpen(const char *path);
int cacheioReadAt(int handle, u64 offset, void *buffer, unsigned int size);
int cacheioClose(int handle);

#ifdef __cplusplus
}
#endif

#endif /* __CACHEIO_H__ */
