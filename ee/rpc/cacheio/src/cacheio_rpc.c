/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <cacheio-common.h>
#include <cacheio.h>
#include <errno.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>

static SifRpcClientData_t rpc_client;
static u8 rpc_buffer[CACHEIO_MAX_PATH] __attribute__((aligned(64)));
static int lock_sema = -1;

static int cacheio_call_locked(int command, int send_size)
{
    int result;

    result = sceSifCallRpc(&rpc_client, command, 0, rpc_buffer, send_size,
        rpc_buffer, sizeof(s32), NULL, NULL);
    if (result >= 0)
        result = *(s32 *)rpc_buffer;
    return result;
}

int cacheioInit(void)
{
    ee_sema_t sema;
    int result;

    if (rpc_client.server != NULL)
        return 0;

    memset(&rpc_client, 0, sizeof(rpc_client));
    while ((result = sceSifBindRpc(&rpc_client, CACHEIO_RPC_ID, 0)) >= 0 &&
        rpc_client.server == NULL) {
        nopdelay();
    }
    if (result < 0)
        return result;

    sema.init_count = 1;
    sema.max_count = 1;
    sema.option = 0;
    lock_sema = CreateSema(&sema);
    if (lock_sema < 0) {
        memset(&rpc_client, 0, sizeof(rpc_client));
        return lock_sema;
    }

    return 0;
}

void cacheioExit(void)
{
    if (lock_sema >= 0) {
        DeleteSema(lock_sema);
        lock_sema = -1;
    }
    memset(&rpc_client, 0, sizeof(rpc_client));
}

int cacheioOpen(const char *path)
{
    struct cacheio_open_packet *packet = (struct cacheio_open_packet *)rpc_buffer;
    size_t len;
    int result;

    if (path == NULL)
        return -EINVAL;
    len = strlen(path);
    if (len >= sizeof(packet->path))
        return -ENAMETOOLONG;

    if (cacheioInit() < 0)
        return -ENOPKG;
    WaitSema(lock_sema);

    memset(packet, 0, sizeof(*packet));
    memcpy(packet->path, path, len + 1);
    result = cacheio_call_locked(CACHEIO_RPC_OPEN, sizeof(*packet));
    SignalSema(lock_sema);
    return result;
}

int cacheioReadAt(int handle, u64 offset, void *buffer, unsigned int size)
{
    struct cacheio_read_packet *packet = (struct cacheio_read_packet *)rpc_buffer;
    int result;

    if (handle <= 0 || buffer == NULL || size == 0 || size > CACHEIO_MAX_READ_SIZE)
        return -EINVAL;
    if (((u32)buffer & 0x0f) != 0)
        return -EINVAL;

    if (cacheioInit() < 0)
        return -ENOPKG;
    WaitSema(lock_sema);

    sceSifWriteBackDCache(buffer, size);
    packet->handle = (u32)handle;
    packet->size = size;
    packet->offset = offset;
    packet->ee_buffer = buffer;
    result = cacheio_call_locked(CACHEIO_RPC_READ_AT, sizeof(*packet));
    SignalSema(lock_sema);
    return result;
}

int cacheioClose(int handle)
{
    struct cacheio_close_packet *packet = (struct cacheio_close_packet *)rpc_buffer;
    int result;

    if (handle <= 0)
        return -EINVAL;

    if (cacheioInit() < 0)
        return -ENOPKG;
    WaitSema(lock_sema);

    packet->handle = (u32)handle;
    result = cacheio_call_locked(CACHEIO_RPC_CLOSE, sizeof(*packet));
    SignalSema(lock_sema);
    return result;
}
