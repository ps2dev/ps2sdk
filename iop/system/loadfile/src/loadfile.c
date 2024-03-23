/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "eeelfloader.h"
#include "irx_imports.h"
#include <kerr.h>
#include <loadfile-common.h>

#ifdef _IOP
IRX_ID("LoadModuleByEE", 0, 0);
#endif
// Mostly based on the module from SCE SDK 1.3.4, with additions from 110U ROM.

static void loadfile_rpc_service_thread(void *param);

int _start(int argc, char *argv[])
{
	const int *BootMode_3;
	int thid;
	iop_thread_t thparam;

	(void)argc;
	(void)argv;

	FlushDcache();
	BootMode_3 = QueryBootMode(3);
	if ( BootMode_3 )
	{
		int BootMode_3_1;

		BootMode_3_1 = BootMode_3[1];
		if ( (BootMode_3_1 & 1) != 0 )
		{
			printf(" No SIF service(loadfile)\n");
			return MODULE_NO_RESIDENT_END;
		}
		if ( (BootMode_3_1 & 2) != 0 )
		{
			printf(" No LoadFile service\n");
			return MODULE_NO_RESIDENT_END;
		}
	}
	CpuEnableIntr();
	thparam.attr = 0x2000000;
	thparam.thread = loadfile_rpc_service_thread;
	thparam.priority = 88;
	thparam.stacksize = 4096;
	thparam.option = 0;
	thid = CreateThread(&thparam);
	if ( thid <= 0 )
	{
		return MODULE_NO_RESIDENT_END;
	}
	StartThread(thid, 0);
	return MODULE_RESIDENT_END;
}

static int *loadfile_modload(const struct _lf_module_load_arg *in_packet, int length, int *outbuffer)
{
	const char *path;

	(void)length;

	path = in_packet->path;
	if ( IsIllegalBootDevice(path) )
	{
		outbuffer[0] = KE_ILLEGAL_OBJECT;
	}
	else
	{
		printf("loadmodule: fname %s args %d arg %s\n", path, in_packet->p.arg_len, in_packet->args);
		outbuffer[0] = LoadStartModule(path, in_packet->p.arg_len, in_packet->args, &outbuffer[1]);
		printf("loadmodule: id %d, ret %d\n", outbuffer[0], outbuffer[1]);
	}
	return outbuffer;
}

static int *loadfile_elfload(const struct _lf_elf_load_arg *in_packet, int length, int *outbuffer)
{
	const char *path;
	int result_out;
	int result_module_out;

	(void)length;

	path = in_packet->path;
	if ( IsIllegalBootDevice(path) )
	{
		outbuffer[0] = KE_FILEERR;
	}
	else
	{
		printf("loadelf: fname %s secname %s\n", path, in_packet->secname);
		outbuffer[0] =
			loadfile_elfload_innerproc(path, in_packet->epc, in_packet->secname, &result_out, &result_module_out);
		if ( outbuffer[0] >= 0 )
		{
			outbuffer[2] = 0;
			outbuffer[0] = result_out;
			outbuffer[1] = result_module_out;
		}
		else
		{
			outbuffer[3] = outbuffer[0];
			outbuffer[0] = 0;
		}
	}
	return outbuffer;
}

static int *loadfile_setaddr(const struct _lf_iop_val_arg *in_packet, int length, int *outbuffer)
{
	void *iop_addr;
	int type;

	(void)length;

	iop_addr = (void *)in_packet->p.iop_addr;
	type = in_packet->type;
	printf("set val add %p type %x ", iop_addr, type);
	switch ( type )
	{
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

static int *loadfile_getaddr(const struct _lf_iop_val_arg *in_packet, int length, int *outbuffer)
{
	void *iop_addr;
	int type;

	(void)length;

	iop_addr = (void *)in_packet->p.iop_addr;
	type = in_packet->type;
	printf("get val add %p type %x ", iop_addr, type);
	switch ( type )
	{
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

static int *loadfile_mg_modload(const struct _lf_module_load_arg *in_packet, int length, int *outbuffer)
{
	(void)length;

	outbuffer[0] = LoadStartKelfModule(in_packet->path, in_packet->p.arg_len, in_packet->args, &outbuffer[1]);
	return outbuffer;
}

static int *loadfile_mg_elfload(const struct _lf_elf_load_arg *in_packet, int length, int *outbuffer)
{
	int result_out;
	int result_module_out;

	(void)length;

	outbuffer[0] =
		loadfile_mg_elfload_proc(in_packet->path, in_packet->epc, in_packet->secname, &result_out, &result_module_out);
	if ( outbuffer[0] >= 0 )
	{
		outbuffer[2] = 0;
		outbuffer[0] = result_out;
		outbuffer[1] = result_module_out;
	}
	else
	{
		outbuffer[0] = 0;
	}
	return outbuffer;
}

// The following function was added in 110U ROM.
static int *loadfile_loadmodulebuffer(const struct _lf_module_buffer_load_arg *in_packet, int length, int *outbuffer)
{
	int ModuleBuffer;

	(void)length;

	printf("loadbuffer: addrres %x args %d arg %s\n", in_packet->p.result, in_packet->q.arg_len, in_packet->args);
	ModuleBuffer = LoadModuleBuffer(in_packet->p.ptr);
	if ( ModuleBuffer >= 0 )
	{
		outbuffer[0] = StartModule(ModuleBuffer, "LBbyEE", in_packet->q.arg_len, in_packet->args, &outbuffer[1]);
	}
	else
	{
		outbuffer[0] = ModuleBuffer;
	}
	printf("loadbuffer: id %d, ret %d\n", outbuffer[0], outbuffer[1]);
	return outbuffer;
}

static int loadfile_rpc_outbuf[0x4] __attribute__((aligned(16)));

static int *loadfile_rpc_service_handler(int fno, void *buffer, int length)
{
	switch ( fno )
	{
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

static void loadfile_rpc_service_thread(void *param)
{
	(void)param;

	printf("Load File service.(99/11/05)\n");
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&loadfile_rpc_service_queue, GetThreadId());
	sceSifRegisterRpc(
		&loadfile_rpc_service_data,
		0x80000006,
		(SifRpcFunc_t)loadfile_rpc_service_handler,
		loadfile_rpc_service_in_buf,
		0,
		0,
		&loadfile_rpc_service_queue);
	sceSifRpcLoop(&loadfile_rpc_service_queue);
}
