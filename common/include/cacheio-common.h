/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __CACHEIO_COMMON_H__
#define __CACHEIO_COMMON_H__

#include <tamtypes.h>

#define CACHEIO_RPC_ID 0x8000CA10

#define CACHEIO_MAX_PATH 256
#define CACHEIO_MAX_READ_SIZE (64 * 1024)

enum cacheio_rpc_command {
    CACHEIO_RPC_OPEN = 1,
    CACHEIO_RPC_READ_AT,
    CACHEIO_RPC_CLOSE
};

struct cacheio_open_packet {
    char path[CACHEIO_MAX_PATH];
};

struct cacheio_read_packet {
    u32 handle;
    u32 size;
    u64 offset;
    void *ee_buffer;
};

struct cacheio_close_packet {
    u32 handle;
};

#endif /* __CACHEIO_COMMON_H__ */
