/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include "eeelfloader.h"
#include <loadfile-common.h>
#include <kerr.h>

#ifdef _IOP
IRX_ID("LoadModuleByEE", 0, 0);
#endif
// Mostly based on the module from SCE SDK 1.3.4, with additions from 110U ROM.

static void loadfile_rpc_start_thread(void *param);

int _start(int argc, char *argv[])
{
    int *BootMode;
    int thread_id;
    iop_thread_t thread_param;

    FlushDcache();
    BootMode = QueryBootMode(3);
    if (BootMode) {
        int iop_boot_param;

        iop_boot_param = BootMode[1];
        if ((iop_boot_param & 1) != 0) {
            printf(" No SIF service(loadfile)\n");
            return MODULE_NO_RESIDENT_END;
        }
        if ((iop_boot_param & 2) != 0) {
            printf(" No LoadFile service\n");
            return MODULE_NO_RESIDENT_END;
        }
    }
    CpuEnableIntr();
    thread_param.attr      = 0x2000000;
    thread_param.thread    = loadfile_rpc_start_thread;
    thread_param.priority  = 88;
    thread_param.stacksize = 4096;
    thread_param.option    = 0;
    thread_id              = CreateThread(&thread_param);
    if (thread_id <= 0) {
        return MODULE_NO_RESIDENT_END;
    }
    StartThread(thread_id, 0);
    return MODULE_RESIDENT_END;
}

static int *loadfile_modload(struct _lf_module_load_arg *in_packet, int length, int *outbuffer)
{
    char *path;

    path = in_packet->path;
    if (IsIllegalBootDevice(path)) {
        outbuffer[0] = KE_ILLEGAL_OBJECT;
    } else {
        printf("loadmodule: fname %s args %d arg %s\n", path, in_packet->p.arg_len, in_packet->args);
        outbuffer[0] = LoadStartModule(path, in_packet->p.arg_len, in_packet->args, &outbuffer[1]);
        printf("loadmodule: id %d, ret %d\n", outbuffer[0], outbuffer[1]);
    }
    return outbuffer;
}

static int *loadfile_elfload(struct _lf_elf_load_arg *in_packet, int length, int *outbuffer)
{
    char *path;
    int v5;
    int v6;

    path = in_packet->path;
    if (IsIllegalBootDevice(path)) {
        outbuffer[0] = KE_FILEERR;
    } else {
        int v4;

        printf("loadelf: fname %s secname %s\n", path, in_packet->secname);
        v4           = loadfile_elfload_innerproc(path, in_packet->epc, in_packet->secname, &v5, &v6);
        outbuffer[0] = v4;
        if (v4 >= 0) {
            outbuffer[2] = 0;
            outbuffer[0] = v5;
            outbuffer[1] = v6;
        } else {
            outbuffer[3] = v4;
            outbuffer[0] = 0;
        }
    }
    return outbuffer;
}

static int *loadfile_setaddr(struct _lf_iop_val_arg *in_packet, int length, int *outbuffer)
{
    void *iop_addr;
    int type;

    iop_addr = (void *)in_packet->p.iop_addr;
    type     = in_packet->type;
    printf("set val add %p type %x ", iop_addr, type);
    switch (type) {
        case LF_VAL_BYTE:
            *(u8 *)iop_addr = in_packet->val.b;
            break;
        case LF_VAL_SHORT:
            *(u16 *)iop_addr = in_packet->val.s;
            break;
        case LF_VAL_LONG:
            *(u32 *)iop_addr = in_packet->val.l;
            break;
        default:
            break;
    }
    outbuffer[0] = 0;
    return outbuffer;
}

static int *loadfile_getaddr(struct _lf_iop_val_arg *in_packet, int length, int *outbuffer)
{
    void *iop_addr;
    int type;

    iop_addr = (void *)in_packet->p.iop_addr;
    type     = in_packet->type;
    printf("get val add %p type %x ", iop_addr, type);
    switch (type) {
        case LF_VAL_BYTE:
            outbuffer[0] = *(u8 *)iop_addr;
            break;
        case LF_VAL_SHORT:
            outbuffer[0] = *(u16 *)iop_addr;
            break;
        case LF_VAL_LONG:
            outbuffer[0] = *(u32 *)iop_addr;
            break;
        default:
            break;
    }
    printf("ret %x\n", outbuffer[0]);
    return outbuffer;
}

static int *loadfile_mg_modload(struct _lf_module_load_arg *in_packet, int length, int *outbuffer)
{
    outbuffer[0] = LoadStartKelfModule(in_packet->path, in_packet->p.arg_len, in_packet->args, &outbuffer[1]);
    return outbuffer;
}

static int *loadfile_mg_elfload(struct _lf_elf_load_arg *in_packet, int length, int *outbuffer)
{
    int v2;
    int v3;

    outbuffer[0] = loadfile_mg_elfload_proc(in_packet->path, in_packet->epc, in_packet->secname, &v2, &v3);
    if (outbuffer[0] >= 0) {
        outbuffer[2] = 0;
        outbuffer[0] = v2;
        outbuffer[1] = v3;
    } else {
        outbuffer[0] = 0;
    }
    return outbuffer;
}

// The following function was added in 110U ROM.
static int *loadfile_loadmodulebuffer(struct _lf_module_buffer_load_arg *in_packet, int length, int *outbuffer)
{
    int ModuleBuffer;

    printf("loadbuffer: addrres %x args %d arg %s\n", in_packet->p.result, in_packet->q.arg_len, in_packet->args);
    ModuleBuffer = LoadModuleBuffer(in_packet->p.ptr);
    if (ModuleBuffer >= 0) {
        outbuffer[0] = StartModule(ModuleBuffer, "LBbyEE", in_packet->q.arg_len, in_packet->args, &outbuffer[1]);
    } else {
        outbuffer[0] = ModuleBuffer;
    }
    printf("loadbuffer: id %d, ret %d\n", outbuffer[0], outbuffer[1]);
    return outbuffer;
}

static int loadfile_rpc_outbuf[0x4] __attribute__((aligned(16)));

static int *loadfile_rpc_service_handler(int fno, void *buffer, int length)
{
    switch (fno) {
        case LF_F_MOD_LOAD:
            return loadfile_modload((struct _lf_module_load_arg *)buffer, length, loadfile_rpc_outbuf);
        case LF_F_ELF_LOAD:
            return loadfile_elfload((struct _lf_elf_load_arg *)buffer, length, loadfile_rpc_outbuf);
        case LF_F_SET_ADDR:
            return loadfile_setaddr((struct _lf_iop_val_arg *)buffer, length, loadfile_rpc_outbuf);
        case LF_F_GET_ADDR:
            return loadfile_getaddr((struct _lf_iop_val_arg *)buffer, length, loadfile_rpc_outbuf);
        case LF_F_MG_MOD_LOAD:
            return loadfile_mg_modload((struct _lf_module_load_arg *)buffer, length, loadfile_rpc_outbuf);
        case LF_F_MG_ELF_LOAD:
            return loadfile_mg_elfload((struct _lf_elf_load_arg *)buffer, length, loadfile_rpc_outbuf);
        case LF_F_MOD_BUF_LOAD:
            return loadfile_loadmodulebuffer((struct _lf_module_buffer_load_arg *)buffer, length, loadfile_rpc_outbuf);
        default:
            return NULL;
    }
}

static SifRpcDataQueue_t loadfile_rpc_service_queue __attribute__((aligned(16)));
static SifRpcServerData_t loadfile_rpc_service_data __attribute__((aligned(16)));
static int loadfile_rpc_service_in_buf[0x112] __attribute__((aligned(16)));

static void loadfile_rpc_start_thread(void *param)
{
    (void)param;

    printf("Load File service.(99/11/05)\n");
    sceSifInitRpc(0);
    sceSifSetRpcQueue(&loadfile_rpc_service_queue, GetThreadId());
    sceSifRegisterRpc(&loadfile_rpc_service_data, 0x80000006, (SifRpcFunc_t)loadfile_rpc_service_handler, loadfile_rpc_service_in_buf, 0, 0, &loadfile_rpc_service_queue);
    sceSifRpcLoop(&loadfile_rpc_service_queue);
}
