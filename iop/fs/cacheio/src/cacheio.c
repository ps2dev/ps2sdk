/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <bd_defrag.h>
#include <bdm.h>
#include <cacheio-common.h>
#include <errno.h>
#include <intrman.h>
#include <iomanX.h>
#include <iox_stat.h>
#include <irx.h>
#include <loadcore.h>
#include <sifcmd.h>
#include <sifman.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <usbhdfsd-common.h>

#define CACHEIO_MAX_HANDLES 4
#define CACHEIO_MAX_BD 32
#define CACHEIO_MAX_FRAGMENTS 4096
#define CACHEIO_STACK_SIZE 0x2000

IRX_ID("cacheio", 1, 0);

typedef struct cacheio_handle {
    int in_use;
    char driver_name[16];
    u32 device_number;
    u32 sector_size;
    u32 fragment_count;
    u64 file_size;
    bd_fragment_t *fragments;
} cacheio_handle_t;

static cacheio_handle_t handles[CACHEIO_MAX_HANDLES];
static SifRpcDataQueue_t rpc_queue;
static SifRpcServerData_t rpc_server;
static u8 rpc_buffer[CACHEIO_MAX_PATH] __attribute__((aligned(16)));
static u8 read_buffer[CACHEIO_MAX_READ_SIZE] __attribute__((aligned(64)));

static void *alloc_sysmem(unsigned int size)
{
    void *result;
    int old_state;

    CpuSuspendIntr(&old_state);
    result = AllocSysMemory(ALLOC_FIRST, size, NULL);
    CpuResumeIntr(old_state);
    return result;
}

static void free_sysmem(void *ptr)
{
    int old_state;

    if (ptr == NULL)
        return;

    CpuSuspendIntr(&old_state);
    FreeSysMemory(ptr);
    CpuResumeIntr(old_state);
}

static struct block_device *find_raw_block_device(const cacheio_handle_t *handle)
{
    struct block_device *devices[CACHEIO_MAX_BD];
    unsigned int i;

    memset(devices, 0, sizeof(devices));
    bdm_get_bd(devices, CACHEIO_MAX_BD);

    for (i = 0; i < CACHEIO_MAX_BD; ++i) {
        struct block_device *bd = devices[i];
        if (bd == NULL)
            continue;
        if (bd->parNr != 0 || bd->devNr != handle->device_number)
            continue;
        if (bd->name != NULL && strcmp(bd->name, handle->driver_name) == 0)
            return bd;
    }

    return NULL;
}

static int allocate_handle(void)
{
    int i;

    for (i = 0; i < CACHEIO_MAX_HANDLES; ++i) {
        if (!handles[i].in_use)
            return i;
    }
    return -EMFILE;
}

static void release_handle(cacheio_handle_t *handle)
{
    free_sysmem(handle->fragments);
    memset(handle, 0, sizeof(*handle));
}

static int cacheio_open_path(const char *path)
{
    cacheio_handle_t *handle;
    iox_stat_t stat;
    bd_fragment_t *fragments;
    char driver_name[16];
    u32 device_number;
    int fragment_count;
    int handle_index;
    int fd;
    int result;

    handle_index = allocate_handle();
    if (handle_index < 0)
        return handle_index;

    fd = iomanX_open(path, FIO_O_RDONLY, 0);
    if (fd < 0)
        return fd;

    memset(driver_name, 0, sizeof(driver_name));
    result = iomanX_ioctl2(fd, USBMASS_IOCTL_GET_DRIVERNAME, NULL, 0,
        driver_name, sizeof(driver_name));
    if (result < 0)
        goto fail;
    driver_name[sizeof(driver_name) - 1] = '\0';

    result = iomanX_ioctl2(fd, USBMASS_IOCTL_GET_DEVICE_NUMBER, NULL, 0,
        &device_number, sizeof(device_number));
    if (result < 0)
        goto fail;

    fragment_count = iomanX_ioctl2(fd, USBMASS_IOCTL_GET_FRAGLIST, NULL, 0,
        NULL, 0);
    if (fragment_count <= 0 || fragment_count > CACHEIO_MAX_FRAGMENTS) {
        result = fragment_count < 0 ? fragment_count : -EFBIG;
        goto fail;
    }

    fragments = alloc_sysmem((unsigned int)fragment_count * sizeof(*fragments));
    if (fragments == NULL) {
        result = -ENOMEM;
        goto fail;
    }

    result = iomanX_ioctl2(fd, USBMASS_IOCTL_GET_FRAGLIST, NULL, 0,
        fragments, (unsigned int)fragment_count * sizeof(*fragments));
    if (result != fragment_count) {
        free_sysmem(fragments);
        if (result >= 0)
            result = -EIO;
        goto fail;
    }

    result = iomanX_getstat(path, &stat);
    if (result < 0) {
        free_sysmem(fragments);
        goto fail;
    }

    handle = &handles[handle_index];
    memset(handle, 0, sizeof(*handle));
    handle->in_use = 1;
    strncpy(handle->driver_name, driver_name, sizeof(handle->driver_name) - 1);
    handle->device_number = device_number;
    handle->fragment_count = (u32)fragment_count;
    handle->file_size = ((u64)stat.hisize << 32) | stat.size;
    handle->fragments = fragments;

    {
        struct block_device *bd = find_raw_block_device(handle);
        if (bd == NULL) {
            release_handle(handle);
            result = -ENODEV;
            goto fail;
        }
        handle->sector_size = bd->sectorSize;
    }

    iomanX_close(fd);
    return handle_index + 1;

fail:
    iomanX_close(fd);
    return result;
}

static int cacheio_read_at(const struct cacheio_read_packet *packet)
{
    cacheio_handle_t *handle;
    struct block_device *bd;
    SifDmaTransfer_t dma;
    u64 end_offset;
    u64 sector;
    u32 sector_count;
    int dma_id;
    int old_state;
    int result;

    if (packet->handle == 0 || packet->handle > CACHEIO_MAX_HANDLES)
        return -EBADF;
    if (packet->ee_buffer == NULL || packet->size == 0 ||
        packet->size > CACHEIO_MAX_READ_SIZE)
        return -EINVAL;
    if (((u32)packet->ee_buffer & 0x0f) != 0)
        return -EINVAL;

    handle = &handles[packet->handle - 1];
    if (!handle->in_use || handle->sector_size == 0)
        return -EBADF;
    if ((packet->offset % handle->sector_size) != 0 ||
        (packet->size % handle->sector_size) != 0)
        return -EINVAL;

    end_offset = packet->offset + packet->size;
    if (end_offset < packet->offset || end_offset > handle->file_size)
        return -EINVAL;

    bd = find_raw_block_device(handle);
    if (bd == NULL || bd->sectorSize != handle->sector_size)
        return -ENODEV;

    sector = packet->offset / handle->sector_size;
    sector_count = packet->size / handle->sector_size;
    if (sector_count > 0xffff)
        return -EINVAL;

    result = bd_defrag_read(bd, handle->fragment_count, handle->fragments,
        sector, read_buffer, (u16)sector_count);
    if (result != (int)sector_count)
        return result < 0 ? result : -EIO;

    dma.src = read_buffer;
    dma.dest = packet->ee_buffer;
    dma.size = packet->size;
    dma.attr = 0;

    CpuSuspendIntr(&old_state);
    dma_id = sceSifSetDma(&dma, 1);
    CpuResumeIntr(old_state);
    if (dma_id == 0)
        return -EIO;
    while (sceSifDmaStat(dma_id) >= 0) {
    }

    return (int)packet->size;
}

static int cacheio_close_handle(u32 handle_id)
{
    cacheio_handle_t *handle;

    if (handle_id == 0 || handle_id > CACHEIO_MAX_HANDLES)
        return -EBADF;

    handle = &handles[handle_id - 1];
    if (!handle->in_use)
        return -EBADF;

    release_handle(handle);
    return 0;
}

static void *cacheio_rpc_dispatch(int command, void *data, int size)
{
    s32 result;

    (void)size;

    switch (command) {
        case CACHEIO_RPC_OPEN:
            result = cacheio_open_path(((struct cacheio_open_packet *)data)->path);
            break;
        case CACHEIO_RPC_READ_AT:
            result = cacheio_read_at((const struct cacheio_read_packet *)data);
            break;
        case CACHEIO_RPC_CLOSE:
            result = cacheio_close_handle(((struct cacheio_close_packet *)data)->handle);
            break;
        default:
            result = -EINVAL;
            break;
    }

    *(s32 *)data = result;
    return data;
}

static void cacheio_rpc_thread(void *arg)
{
    (void)arg;

    sceSifInitRpc(0);
    sceSifSetRpcQueue(&rpc_queue, GetThreadId());
    sceSifRegisterRpc(&rpc_server, CACHEIO_RPC_ID, cacheio_rpc_dispatch,
        rpc_buffer, NULL, NULL, &rpc_queue);
    sceSifRpcLoop(&rpc_queue);
}

int _start(int argc, char *argv[])
{
    iop_thread_t thread;
    int thread_id;

    (void)argc;
    (void)argv;

    memset(handles, 0, sizeof(handles));

    thread.attr = TH_C;
    thread.thread = cacheio_rpc_thread;
    thread.priority = 40;
    thread.stacksize = CACHEIO_STACK_SIZE;
    thread.option = 0;

    thread_id = CreateThread(&thread);
    if (thread_id <= 0)
        return MODULE_NO_RESIDENT_END;

    if (StartThread(thread_id, NULL) < 0)
        return MODULE_NO_RESIDENT_END;

    printf("cacheio: extent-aware read-only RPC server v1.0\n");
    return MODULE_RESIDENT_END;
}
