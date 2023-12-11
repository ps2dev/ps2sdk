/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include "fileio-common.h"
#include "iopheap-common.h"
#define MODNAME "FILEIO_service"
#ifdef _IOP
IRX_ID(MODNAME, 1, 1);
#endif
// Mostly based on the module from SCE SDK 1.3.4.
// Additions from XFILEIO from ROM version 1.1.0a have been added, but disabled.


#ifdef DEBUG
#define DPRINTF(x...) printf(MODNAME ": " x)
#else
#define DPRINTF(x...)
#endif


static struct _fio_read_data read_data_out __attribute__((aligned(16)));
static void *xfer_buffer;
static int xfer_size;
static int fileio_rpc_tmpbuf[0x4c] __attribute__((aligned(16)));

static void fileio_rpc_start_thread(void *param);
static void heap_rpc_start_thread(void *param);

int _start(int argc, char *argv[])
{
    int *BootMode;
    int fileio_rpc_thread;
    int heap_rpc_thread;
    iop_thread_t thread_param;

    FlushDcache();
    BootMode = QueryBootMode(3);
    if (BootMode) {
        int iop_boot_param;

        iop_boot_param = BootMode[1];
        if ((iop_boot_param & 1) != 0) {
            DPRINTF(" No SIF service(fileio)\n");
            return MODULE_NO_RESIDENT_END;
        }
        if ((iop_boot_param & 2) != 0) {
            DPRINTF(" No FILEIO service\n");
            return MODULE_NO_RESIDENT_END;
        }
    }
    CpuEnableIntr();
    thread_param.thread    = fileio_rpc_start_thread;
    thread_param.attr      = 0x2000000;
    thread_param.priority  = 96;
    thread_param.stacksize = 4096;
    thread_param.option    = 0;
    fileio_rpc_thread      = CreateThread(&thread_param);
    if (fileio_rpc_thread <= 0) {
        return MODULE_NO_RESIDENT_END;
    }
    StartThread(fileio_rpc_thread, 0);
    thread_param.thread    = heap_rpc_start_thread;
    thread_param.attr      = 0x2000000;
    thread_param.priority  = 96;
    thread_param.stacksize = 2048;
    thread_param.option    = 0;
    heap_rpc_thread        = CreateThread(&thread_param);
    if (heap_rpc_thread <= 0) {
        return MODULE_NO_RESIDENT_END;
    }
    StartThread(heap_rpc_thread, 0);
    return MODULE_RESIDENT_END;
}

static void *fileio_allocate_buffer_memory()
{
    int i;

    i         = 0;
    xfer_size = 0x4000;
    for (;;) {
        xfer_buffer = AllocSysMemory(1, xfer_size, 0);
        i += 1;
        if (xfer_buffer) {
            break;
        }
        xfer_size /= 2;
        if (i >= 8) {
            return xfer_buffer;
        }
    }
    DPRINTF("read/write allocate memory %x\n", xfer_size);
    return xfer_buffer;
}

static void fileio_rpc_open(struct _fio_open_arg *buffer, int length, int *outbuffer)
{
    if ((xfer_buffer != NULL) || ((xfer_buffer = fileio_allocate_buffer_memory()) != NULL)) {
        int fd;

        DPRINTF("open name %s flag %x data %x\n", buffer->name, buffer->mode, (u32)(buffer));
        fd = io_open(buffer->name, buffer->mode);
        DPRINTF("open fd = %d\n", fd);
        *outbuffer = fd;
    } else {
        DPRINTF("Error:Cannot alloc r/w buffer\n");
        *outbuffer = -1;
    }
}

static void fileio_rpc_close(int *buffer, int length, int *outbuffer)
{
    *outbuffer = io_close(*buffer);
}

static void fileio_rpc_lseek(struct _fio_lseek_arg *buffer, int length, int *outbuffer)
{
    *outbuffer = io_lseek(buffer->p.fd, buffer->offset, buffer->whence);
}

static void fileio_rpc_read(struct _fio_read_arg *buffer, int length, int *outbuffer)
{
    int size;
    int fd;
    unsigned int ptr;
    size_t v6;
    int v7;
    char *v8;
    signed int v9;
    signed int v10;
    int v11;
    int v12;
    size_t v13;
    int v14;
    int v15;
    int v16;
    struct _fio_read_data *read_data;
    SifDmaTransfer_t v19;
    int state;
    void *v21;
    void *v22;

    size = buffer->size;
    fd   = buffer->fd;
    ptr  = (unsigned int)buffer->ptr;
    v6   = 0;
#if 0
    if (size >= 64) {
        if ((ptr & 0x3F) != 0) {
            v7 = (ptr >> 6 << 6) - (ptr - 64);
        } else {
            v7 = 0;
        }
        v21 = ptr;
        v8  = (char *)(ptr + v7);
        v9  = ((ptr + size) >> 6 << 6) - (ptr + v7);
        v10 = ptr + size - ((ptr + size) >> 6 << 6);
        v22 = (ptr + size) >> 6 << 6;
    }
#else
    if (size >= 16) {
        if ((ptr & 0xF) != 0) {
            v7 = (ptr >> 4 << 4) - (ptr - 16);
        } else {
            v7 = 0;
        }
        v21 = (void *)ptr;
        v8  = (char *)(ptr + v7);
        v9  = ((ptr + size) >> 4 << 4) - (ptr + v7);
        v10 = ptr + size - ((ptr + size) >> 4 << 4);
        v22 = (void *)((ptr + size) >> 4 << 4);
    }
#endif
    else
    {
        v7  = size;
        v8  = 0;
        v9  = 0;
        v10 = 0;
        v21 = (void *)ptr;
        v22 = 0;
    }
    v11 = 0;
    if (v7 > 0) {
        v12 = io_read(fd, read_data_out.buf1, v7);
        if (v7 != v12) {
            v7 = 0;
            if (v12 > 0) {
                v7 = v12;
            }
            v6 = v7;
            goto LABEL_26;
        }
        v6 = v7;
    }
    if (v9 <= 0) {
    LABEL_21:
        if (v10 > 0) {
            v16 = io_read(fd, read_data_out.buf2, v10);
            if (v10 != v16) {
                v10 = 0;
                if (v16 > 0) {
                    v10 = v16;
                }
            }
            v6 += v10;
        }
    } else {
        for (;;) {
            v13 = v9;
            if (xfer_size < v9)
                v13 = xfer_size;
            while (sceSifDmaStat(v11) >= 0)
                ;
            v14 = io_read(fd, xfer_buffer, v13);
            v15 = v14;
            if (v13 != v14) {
                break;
            }
            v6 += v14;
            v9 -= v14;
            v19.dest = v8;
            v8 += v14;
            v19.size = v14;
            v19.attr = 0;
            v19.src  = xfer_buffer;
            CpuSuspendIntr(&state);
            v11 = sceSifSetDma(&v19, 1);
            CpuResumeIntr(state);
            if (v9 <= 0) {
                goto LABEL_21;
            }
        }
        if (v14 > 0) {
            v19.dest = v8;
            v19.size = v14;
            v19.attr = 0;
            v19.src  = xfer_buffer;
            CpuSuspendIntr(&state);
            sceSifSetDma(&v19, 1);
            v6 += v15;
            CpuResumeIntr(state);
        }
    }
LABEL_26:
    read_data_out.size1 = v7;
    read_data_out.size2 = v10;
    read_data_out.dest1 = v21;
    read_data_out.dest2 = v22;
    v19.src             = &read_data_out;
    read_data           = buffer->read_data;
#if 0
    v19.size = 144;
#else
    v19.size = 48;
#endif
    v19.attr = 0;
    v19.dest = read_data;
    CpuSuspendIntr(&state);
    sceSifSetDma(&v19, 1);
    CpuResumeIntr(state);
    *outbuffer = v6;
}

static void fileio_rpc_write(struct _fio_write_arg *buffer, int length, int *outbuffer)
{
    u32 size;
    s32 mis;
    int fd;
    int v10;
    char *v11;
    int v13;
    SifRpcReceiveData_t v14;
    int v15;

    if (!xfer_buffer) {
        xfer_buffer = fileio_allocate_buffer_memory();
        if (!xfer_buffer) {
            Kprintf("Error:Cannot alloc r/w buffer\n");
            *outbuffer = -1;
            return;
        }
    }
    size = buffer->size;
    v15  = 0;
    mis  = buffer->mis;
    fd   = buffer->fd;
    if (mis > 0) {
        int v9;

        v9 = io_write(fd, buffer->aligned, mis);
        if (v9 != mis) {
            if (v9 > 0) {
                v15 += v9;
            }
            *outbuffer = v15;
            return;
        }
        v15 += v9;
    }
    v10 = size - mis;
    v11 = (char *)buffer->ptr + mis;
    if (!v10) {
        *outbuffer = v15;
        return;
    }
    for (;;) {
        int v12;

        v12 = v10;
        if (xfer_size < v10) {
            v12 = xfer_size;
        }
        sceSifGetOtherData(&v14, v11, xfer_buffer, v12, 0);
        v13 = io_write(fd, xfer_buffer, v12);
        v10 -= v12;
        if (v13 != v12) {
            break;
        }
        v11 += v12;
        v15 += v12;
        if (!v10) {
            *outbuffer = v15;
            return;
        }
    }
    if (v13 > 0) {
        v15 += v13;
    }
    *outbuffer = v15;
}

static void fileio_rpc_ioctl(struct _fio_ioctl_arg *buffer, int length, int *outbuffer)
{
    *outbuffer = io_ioctl(buffer->p.fd, buffer->request, buffer->data);
}

static void fileio_rpc_remove(const char *buffer, int length, int *outbuffer)
{
    DPRINTF("remove file %s \n", buffer);
    *outbuffer = io_remove(buffer);
}

static void fileio_rpc_mkdir(const char *buffer, int length, int *outbuffer)
{
    DPRINTF("mkdir name %s \n", buffer);
    *outbuffer = io_mkdir(buffer);
}

static void fileio_rpc_rmdir(const char *buffer, int length, int *outbuffer)
{
    DPRINTF("rmdir name %s \n", buffer);
    *outbuffer = io_rmdir(buffer);
}

static void fileio_rpc_format(const char *buffer, int length, int *outbuffer)
{
    DPRINTF("format name %s \n", buffer);
    *outbuffer = io_format(buffer);
}

static void fileio_rpc_adddrv(iop_io_device_t **buffer, int length, int *outbuffer)
{
    DPRINTF("adddrv device addr %x\n", *buffer);
    *outbuffer = io_AddDrv(*buffer);
}

static void fileio_rpc_deldrv(const char *buffer, int length, int *outbuffer)
{
    DPRINTF("deldrv device name %s \n", buffer);
    *outbuffer = io_DelDrv(buffer);
}

static void fileio_rpc_dopen(const char *buffer, int length, int *outbuffer)
{
    int fd;

    DPRINTF("dopen name %s \n", buffer);
    // FIXME: mode parameter is not passed
    fd = io_dopen(buffer, 0);
    DPRINTF("dopen fd = %d\n", fd);
    *outbuffer = fd;
}

static void fileio_rpc_dclose(int *buffer, int length, int *outbuffer)
{
    *outbuffer = io_dclose(*buffer);
}

static void fileio_rpc_dread(struct _fio_dread_arg *buffer, int length, int *outbuffer)
{
    int v6;
    SifDmaTransfer_t v8;

    v6 = io_dread(buffer->p.fd, (io_dirent_t *)fileio_rpc_tmpbuf);
    if (v6 >= 0) {
        v8.src  = fileio_rpc_tmpbuf;
        v8.size = 300;
        v8.attr = 0;
        v8.dest = (void *)buffer->buf;
        if (!sceSifSetDma(&v8, 1)) {
            v6 = -1;
        }
    }
    *outbuffer = v6;
}

static void fileio_rpc_getstat(struct _fio_getstat_arg *buffer, int length, int *outbuffer)
{
    int v6;
    SifDmaTransfer_t v8;

    v6 = io_getstat(buffer->name, (io_stat_t *)fileio_rpc_tmpbuf);
    if (v6 >= 0) {
        v8.src  = fileio_rpc_tmpbuf;
        v8.size = 40;
        v8.attr = 0;
        v8.dest = (void *)buffer->p.result;
        if (!sceSifSetDma(&v8, 1)) {
            v6 = -1;
        }
    }
    *outbuffer = v6;
}

static void fileio_rpc_chstat(struct _fio_chstat_arg *buffer, int length, int *outbuffer)
{
    *outbuffer = io_chstat(buffer->name, &buffer->stat, buffer->p.cbit);
}

static int fileio_rpc_outbuf[0x4] __attribute__((aligned(16)));

static int *fileio_rpc_service_handler(int fno, void *buffer, int length)
{
    switch (fno) {
        case FIO_F_OPEN:
            fileio_rpc_open((struct _fio_open_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_CLOSE:
            fileio_rpc_close((int *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_READ:
            fileio_rpc_read((struct _fio_read_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_WRITE:
            fileio_rpc_write((struct _fio_write_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_LSEEK:
            fileio_rpc_lseek((struct _fio_lseek_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_IOCTL:
            fileio_rpc_ioctl((struct _fio_ioctl_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_REMOVE:
            fileio_rpc_remove((const char *)buffer, length, fileio_rpc_outbuf);
            // NOTE: in the original version, there was a missing break here which caused it to fall through to mkdir
            break;
        case FIO_F_MKDIR:
            fileio_rpc_mkdir((const char *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_RMDIR:
            fileio_rpc_rmdir((const char *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_DOPEN:
            fileio_rpc_dopen((const char *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_DCLOSE:
            fileio_rpc_dclose((int *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_DREAD:
            fileio_rpc_dread((struct _fio_dread_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_GETSTAT:
            fileio_rpc_getstat((struct _fio_getstat_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_CHSTAT:
            fileio_rpc_chstat((struct _fio_chstat_arg *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_FORMAT:
            fileio_rpc_format((const char *)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_ADDDRV:
            fileio_rpc_adddrv((iop_io_device_t **)buffer, length, fileio_rpc_outbuf);
            break;
        case FIO_F_DELDRV:
            fileio_rpc_deldrv((const char *)buffer, length, fileio_rpc_outbuf);
            break;
        default:
            DPRINTF("sce_fileio: unrecognized code %x\n", fno);
            break;
    }
    return fileio_rpc_outbuf;
}

static SifRpcDataQueue_t fileio_rpc_service_queue __attribute__((aligned(16)));
static SifRpcServerData_t fileio_rpc_service_data __attribute__((aligned(16)));
static int fileio_rpc_service_in_buf[0x112] __attribute__((aligned(16)));

static void fileio_rpc_start_thread(void *param)
{
    (void)param;

    if (!sceSifCheckInit())
        sceSifInit();
    DPRINTF("Multi Threaded Fileio module.(99/11/15) \n");
    xfer_buffer = NULL;
    sceSifInitRpc(0);
    sceSifSetRpcQueue(&fileio_rpc_service_queue, GetThreadId());
    sceSifRegisterRpc(
        &fileio_rpc_service_data,
        0x80000001,
        (SifRpcFunc_t)fileio_rpc_service_handler,
        fileio_rpc_service_in_buf,
        0,
        0,
        &fileio_rpc_service_queue);
    sceSifRpcLoop(&fileio_rpc_service_queue);
}

static void heap_rpc_load_iop_heap(struct _iop_load_heap_arg *buffer, int length, int *outbuffer)
{
    int fd;

    fd = io_open(buffer->path, 1);
    if (fd >= 0) {
        int saved_pos;

        saved_pos = io_lseek(fd, 0, 2);
        io_lseek(fd, 0, 0);
        io_read(fd, buffer->p.addr, saved_pos);
        io_close(fd);
        *outbuffer = 0;
    } else {
        DPRINTF("load heap :error \n");
        *outbuffer = -1;
    }
}

static void heap_rpc_alloc_iop_heap(int *buffer, int length, int *outbuffer)
{
    *outbuffer = (int)AllocSysMemory(0, *buffer, 0);
}

static void heap_rpc_free_iop_heap(void **buffer, int length, int *outbuffer)
{
    *outbuffer = FreeSysMemory(*buffer);
}

static int heap_rpc_outbuf[0x4] __attribute__((aligned(16)));

static int *heap_rpc_service_handler(int fno, void *buffer, int length)
{
    switch (fno) {
        case 1:
            heap_rpc_alloc_iop_heap((int *)buffer, length, heap_rpc_outbuf);
            break;
        case 2:
            heap_rpc_free_iop_heap((void **)buffer, length, heap_rpc_outbuf);
            break;
        case 3:
            heap_rpc_load_iop_heap((struct _iop_load_heap_arg *)buffer, length, heap_rpc_outbuf);
            break;
        default:
            DPRINTF("sce_iopmem: unrecognized code %x\n", fno);
            break;
    }
    return heap_rpc_outbuf;
}

static SifRpcDataQueue_t heap_rpc_service_queue __attribute__((aligned(16)));
static SifRpcServerData_t heap_rpc_service_data __attribute__((aligned(16)));
static int heap_rpc_service_in_buf[0x40] __attribute__((aligned(16)));

static void heap_rpc_start_thread(void *param)
{
    (void)param;

    if (!sceSifCheckInit())
        sceSifInit();
    DPRINTF("iop heap service (99/11/03)\n");
    sceSifInitRpc(0);
    sceSifSetRpcQueue(&heap_rpc_service_queue, GetThreadId());
    sceSifRegisterRpc(
        &heap_rpc_service_data,
        0x80000003,
        (SifRpcFunc_t)heap_rpc_service_handler,
        heap_rpc_service_in_buf,
        0,
        0,
        &heap_rpc_service_queue);
    sceSifRpcLoop(&heap_rpc_service_queue);
}

#if 0
// XXX: unused code
static void iopinfo_rpc_querybootmode(void *buffer, int length, int *outbuffer)
{
    int *BootMode;

    BootMode   = QueryBootMode(6);
    *outbuffer = (BootMode != 0) ? (*(u16 *)BootMode & 0xFFFC) : 0x800;
}

static int iopinfo_rpc_outbuf[0x4] __attribute__((aligned(16)));

static int *iopinfo_rpc_service_handler(int fno, void *buffer, int length)
{
    switch (fno) {
        case 1:
            iopinfo_rpc_querybootmode(buffer, length, iopinfo_rpc_outbuf);
            break;
        default:
            DPRINTF("sce_iopinfo: unrecognized code %x\n", fno);
            break;
    }
    return iopinfo_rpc_outbuf;
}

static SifRpcDataQueue_t iopinfo_rpc_service_queue __attribute__((aligned(16)));
static SifRpcServerData_t iopinfo_rpc_service_data __attribute__((aligned(16)));
static int iopinfo_rpc_service_in_buf[0x10] __attribute__((aligned(16)));

static void iopinfo_rpc_service_start_thread(void *param)
{
    (void)param;

    if (!sceSifCheckInit())
        sceSifInit();
    DPRINTF("iop infomation service (00/02/29)\n");
    sceSifInitRpc(0);
    sceSifSetRpcQueue(&iopinfo_rpc_service_queue, GetThreadId());
    sceSifRegisterRpc(
        &iopinfo_rpc_service_data,
        0x80000007,
        (SifRpcFunc_t)iopinfo_rpc_service_handler,
        iopinfo_rpc_service_in_buf,
        0,
        0,
        &iopinfo_rpc_service_queue);
    sceSifRpcLoop(&iopinfo_rpc_service_queue);
}
#endif
