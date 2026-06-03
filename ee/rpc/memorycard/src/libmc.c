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
 * Function defenitions for mclib.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include "libmc.h"

//#define MC_DEBUG

#ifdef MC_DEBUG
#include <stdio.h>
#endif

enum MC_RPCCMD_NUMBERS
{
	MC_RPCCMD_INIT		= 0x00,
	MC_RPCCMD_GET_INFO,
	MC_RPCCMD_OPEN,
	MC_RPCCMD_CLOSE,
	MC_RPCCMD_SEEK,
	MC_RPCCMD_READ,
	MC_RPCCMD_WRITE,
	MC_RPCCMD_FLUSH,
	MC_RPCCMD_CH_DIR,
	MC_RPCCMD_GET_DIR,
	MC_RPCCMD_SET_INFO,
	MC_RPCCMD_DELETE,
	MC_RPCCMD_FORMAT,
	MC_RPCCMD_UNFORMAT,
	MC_RPCCMD_GET_ENT,
	MC_RPCCMD_CHG_PRITY,
	MC_RPCCMD_CHECK_BLOCK,
	MC_RPCCMD_ERASE_BLOCK	= 0x0E,
	MC_RPCCMD_READ_PAGE,
	MC_RPCCMD_WRITE_PAGE
};

static const int mcRpcCmd[2][17] =
{
	// MCMAN/MCSERV values
	{
	0x70,	// MC_RPCCMD_INIT
	0x78,	// MC_RPCCMD_GET_INFO
	0x71,	// MC_RPCCMD_OPEN
	0x72,	// MC_RPCCMD_CLOSE
	0x75,	// MC_RPCCMD_SEEK
	0x73,	// MC_RPCCMD_READ
	0x74,	// MC_RPCCMD_WRITE
	0x7A,	// MC_RPCCMD_FLUSH
	0x7B,	// MC_RPCCMD_CH_DIR
	0x76,	// MC_RPCCMD_GET_DIR
	0x7C,	// MC_RPCCMD_SET_INFO
	0x79,	// MC_RPCCMD_DELETE
	0x77,	// MC_RPCCMD_FORMAT
	0x80,	// MC_RPCCMD_UNFORMAT
	0x7D,	// MC_RPCCMD_ERASE_BLOCK (calls mcman_funcs: 39, 17, 20, 19, 30)
	0x7E,	// MC_RPCCMD_READ_PAGE
	0x7F,	// MC_RPCCMD_WRITE_PAGE (calls mcman_funcs: 20, 19)
	},
	// XMCMAN/XMCSERV values
	{
	0xFE,	// MC_RPCCMD_INIT
	0x01,	// MC_RPCCMD_GET_INFO
	0x02,	// MC_RPCCMD_OPEN
	0x03,	// MC_RPCCMD_CLOSE
	0x04,	// MC_RPCCMD_SEEK
	0x05,	// MC_RPCCMD_READ
	0x06,	// MC_RPCCMD_WRITE
	0x0A,	// MC_RPCCMD_FLUSH
	0x0C,	// MC_RPCCMD_CH_DIR
	0x0D,	// MC_RPCCMD_GET_DIR
	0x0E,	// MC_RPCCMD_SET_INFO
	0x0F,	// MC_RPCCMD_DELETE
	0x10,	// MC_RPCCMD_FORMAT
	0x11,	// MC_RPCCMD_UNFORMAT
	0x12,	// MC_RPCCMD_GET_ENT
	0x14,	// MC_RPCCMD_CHG_PRITY
	0x33,	// MC_RPCCMD_CHECK_BLOCK (calls xmcman_funcs: 45)
	}
};

struct libmc_name_param_stru
{
	int m_port;
	int m_slot;
	int m_flags;
	int m_maxent;
	union
	{
		sceMcTblGetDir *m_mcT;
		char *m_curdir;
	};
	char m_name[1024];
};

union libmc_name_desc_param_unio
{
	struct libmc_name_param_stru m_name_param;
	mcDescParam_t m_desc_param;
};

union libmc_rdata_param_unio
{ 
	int m_result;
	mcRpcStat_t m_rpcStat;
	u8 m_buffer[2048];
};

struct libmc_page_read_align_data_stru
{
	unsigned int m_size1;
	unsigned int m_size2;
	void *m_dest1;
	void *m_dest2;
	unsigned char m_data1[16];
	unsigned char m_data2[16];
	unsigned char m_padding[16];
};

union libmc_extra_send_recv_unio
{
	int m_end_parameter[48];
	char m_cur_dir[1024];
	sceMcTblGetDir m_file_info_buff;
	struct libmc_page_read_align_data_stru m_page_read_align_data;
};

typedef struct mcExtraEndParam_
{
	union libmc_extra_send_recv_unio *m_extra_send_recv_param;
	int* m_p_type;
	int* m_p_free;
	int* m_p_format;
	int* m_p_result;
	char *m_dst_cur_dir;
} mcExtraEndParam_t;

struct libmc_interface_data_stru
{
	SifRpcClientData_t m_client_data __attribute__((aligned(64))); // target->m_interface_data->m_client_data
	unsigned int m_current_command; // target->m_interface_data->m_current_command
	int m_mc_rpc_type; // target->m_interface_data->m_mc_rpc_type
	mcExtraEndParam_t m_extra_end_param; // target->m_interface_data->m_extra_end_param
	union libmc_name_desc_param_unio m_name_desc_param __attribute__((aligned(64))); // target->m_interface_data->m_name_desc_param
	union libmc_rdata_param_unio m_rpc_rdata __attribute__((aligned(64))); // target->m_interface_data->m_rpc_rdata
	union libmc_extra_send_recv_unio m_extra_send_recv_param __attribute__((aligned(64))); // target->m_interface_data->m_extra_send_recv_param
};

enum MC_INTERFACE
{
	MC_INTERFACE_SIO2 = 0,
	MC_INTERFACE_DEV9,
	MC_INTERFACE_MAX,
};

typedef struct libmc_internal_data_
{
	struct libmc_interface_data_stru m_interface_data[MC_INTERFACE_MAX];
} libmc_internal_data_t;

static libmc_internal_data_t g_libmc_internal_data;

typedef struct libmc_target_desc_
{
    struct libmc_interface_data_stru *m_interface_data;
    int m_interface;
    int m_port;
    int m_slot;
    int m_fd;
} libmc_target_desc_t;

#define LIBMC_PRE_CHECK_FLAG_ONLY_TYPE 1

static inline int libmc_pre_rpc_impl(const libmc_target_desc_t *target, int flags, int only_type)
{
	// check mc lib is inited
	if (!target->m_interface_data->m_client_data.server)
		return -1;
	// If MCSERV version does not support a feature, return error
	if ((flags & LIBMC_PRE_CHECK_FLAG_ONLY_TYPE) != 0 && target->m_interface_data->m_mc_rpc_type != only_type)
		return -1;
	// check nothing else is processing
	if (target->m_interface_data->m_current_command != MC_FUNC_NONE)
		return target->m_interface_data->m_current_command;
	return 0;
}

#define LIBMC_PRE_RPC(target, flags, only_type) \
	{ \
		int cur_ret; \
		cur_ret = libmc_pre_rpc_impl(target, flags, only_type); \
		if (cur_ret) \
			return cur_ret; \
	}

static inline int libmc_post_rpc(const libmc_target_desc_t *target, int cmd, int desc_param_or_name_param, SifRpcEndFunc_t end_function, void *end_param)
{
	int ret;

	// send sif command
	ret = sceSifCallRpc(
		&target->m_interface_data->m_client_data,
		mcRpcCmd[target->m_interface_data->m_mc_rpc_type][cmd],
		SIF_RPC_M_NOWAIT,
		desc_param_or_name_param ? (void *)&(target->m_interface_data->m_name_desc_param.m_desc_param) : (void *)&(target->m_interface_data->m_name_desc_param.m_name_param),
		desc_param_or_name_param ? sizeof(target->m_interface_data->m_name_desc_param.m_desc_param) : sizeof(target->m_interface_data->m_name_desc_param.m_name_param),
		&(target->m_interface_data->m_rpc_rdata.m_result),
		sizeof(target->m_interface_data->m_rpc_rdata.m_result),
		end_function,
		end_param
	);
	
	if (ret)
		return ret;
	target->m_interface_data->m_current_command = cmd;
	return 0;
}

static inline void libmc_setup_target(libmc_target_desc_t *target, int interface, int port, int slot, int fd)
{
	target->m_interface_data = &(g_libmc_internal_data.m_interface_data[interface]);
	target->m_interface = interface;
	target->m_port = port;
	target->m_slot = slot;
	target->m_fd = fd;
}

/** function that gets called when mcGetInfo ends
 * and interrupts are disabled (old RPC)
 */
static void mcGetInfoApdxOld(void* arg)
{
	mcExtraEndParam_t *ep = (mcExtraEndParam_t *)arg;
	const mcEndParam_t	*ptr	= (const mcEndParam_t*)(ep->m_extra_send_recv_param->m_end_parameter);

	// older MCSERV doesnt support retrieving whether card is formatted
	// so if a card is present, determine whether its formatted based on the return value from MCSERV
	if (ep->m_p_type)
		*(ep->m_p_type) = ptr->type;

	if (ep->m_p_free)
		*(ep->m_p_free) = ptr->free;

	if (ep->m_p_format)
		*(ep->m_p_format) = (ptr->type == MC_TYPE_NONE || *(ep->m_p_result) == -2) ? 0 : 1;
}

/** function that gets called when mcGetInfo ends
 * and interrupts are disabled (new RPC)
 */
static void mcGetInfoApdxNew(void* arg)
{
	mcExtraEndParam_t *ep = (mcExtraEndParam_t *)arg;
	const mcEndParam2_t	*ptr	= (const mcEndParam2_t*)(ep->m_extra_send_recv_param->m_end_parameter);

	if (ep->m_p_type)
		*(ep->m_p_type) = ptr->type;

	if (ep->m_p_free)
		*(ep->m_p_free) = ptr->free;

	if (ep->m_p_format)
		*(ep->m_p_format) = ptr->formatted;
}

/** function that gets called when mcRead ends
 * and interrupts are disabled (old RPC)
 */
static void mcReadFixAlignOld(void* arg)
{
	mcExtraEndParam_t *ep = (mcExtraEndParam_t *)arg;
	mcEndParam_t	*ptr	= (mcEndParam_t*)(ep->m_extra_send_recv_param->m_end_parameter);
	u8 *dest;
	int i;

	for (i = 0, dest = (u8*)ptr->dest1; i < ptr->size1; i++)
		dest[i] = ptr->src1[i];
	for (i = 0, dest = (u8*)ptr->dest2; i < ptr->size2; i++)
		dest[i] = ptr->src2[i];
}

/** function that gets called when mcRead ends
 * and interrupts are disabled (new RPC)
 */
static void mcReadFixAlignNew(void* arg)
{
	mcExtraEndParam_t *ep = (mcExtraEndParam_t *)arg;
	mcEndParam2_t	*ptr	= (mcEndParam2_t*)(ep->m_extra_send_recv_param->m_end_parameter);
	u8 *dest;
	int i;

	for (i = 0, dest = (u8*)ptr->dest1; i < ptr->size1; i++)
		dest[i] = ptr->src1[i];
	for (i = 0, dest = (u8*)ptr->dest2; i < ptr->size2; i++)
		dest[i] = ptr->src2[i];
}

/** function that gets called when mcChDir ends
 * and interrupts are disabled
 */
static void mcStoreDir(void* arg)
{
	mcExtraEndParam_t *ep = (mcExtraEndParam_t *)arg;
	int len;
	char *currentDir = ep->m_extra_send_recv_param->m_cur_dir;
	len = strlen(currentDir);
	if (len >= 1024)
		len = strlen(currentDir+1023);
	memcpy(ep->m_dst_cur_dir, currentDir, len);
	*(currentDir+len) = 0;
}

static void libmc_ReadAlignFunction(void *arg)
{
	mcExtraEndParam_t *ep = (mcExtraEndParam_t *)arg;
	const struct libmc_page_read_align_data_stru *data = UNCACHED_SEG(&(ep->m_extra_send_recv_param->m_page_read_align_data));
	unsigned int misaligned;

	if ((misaligned=(unsigned int)data->m_dest1&0xF)!=0)
	{
		memcpy(UNCACHED_SEG(data->m_dest1), UNCACHED_SEG(data->m_data1), sizeof(data->m_data1)-misaligned);
		memcpy(UNCACHED_SEG((unsigned int)data->m_dest1+(sizeof(data->m_data1)-misaligned)), UNCACHED_SEG((unsigned int)data->m_data1+(sizeof(data->m_data1)-misaligned)+0x1F0), misaligned);
	}
}

static int libmc_rpc_reset(const libmc_target_desc_t *target);

static int libmc_rpc_init(const libmc_target_desc_t *target)
{
	int ret, err, rpc_id;
	const mcRpcStat_t *rpcStat = (const mcRpcStat_t*)UNCACHED_SEG(&target->m_interface_data->m_rpc_rdata.m_rpcStat);

	{
		static int _rb_count;
		extern int _iop_reboot_count;
		if (_rb_count != _iop_reboot_count) {
			_rb_count = _iop_reboot_count;
			libmc_rpc_reset(NULL);
		}
	}
	switch (target->m_interface)
	{
	case MC_INTERFACE_SIO2:
	default:
		rpc_id = 0x80000400;
		break;
	case MC_INTERFACE_DEV9:
		rpc_id = 0x80000480;
		break;
	}

	if (target->m_interface_data->m_client_data.server)
		return -1;

	sceSifInitRpc(0);

	// bind to mc rpc on iop
	do
	{
		if ((ret=sceSifBindRpc(&target->m_interface_data->m_client_data, rpc_id, 0)) < 0)
		{
			#ifdef MC_DEBUG
				printf("libmc: bind error\n");
			#endif

			return ret;
		}
		if (target->m_interface_data->m_client_data.server == NULL)
			nopdelay();
	}
	while (target->m_interface_data->m_client_data.server == NULL);

	// for some reason calling this init sif function with 'mcserv' makes all other
	// functions not work properly. although NOT calling it means that detecting
	// whether or not cards are formatted doesnt seem to work :P

	// Start with calling flush with an invalid fd (so it sets the return value to
	// sceMcResDeniedPermit, which MC_RPCCMD_INIT will not return)
	target->m_interface_data->m_name_desc_param.m_desc_param.fd = 0xFFFFFFFF;
	sceSifCallRpc(&target->m_interface_data->m_client_data, mcRpcCmd[MC_TYPE_XMC][MC_RPCCMD_FLUSH], 0, &(target->m_interface_data->m_name_desc_param.m_desc_param), sizeof(target->m_interface_data->m_name_desc_param.m_desc_param), &target->m_interface_data->m_rpc_rdata, 4, NULL, NULL);
	sceSifCallRpc(&target->m_interface_data->m_client_data, mcRpcCmd[MC_TYPE_MC][MC_RPCCMD_FLUSH], 0, &(target->m_interface_data->m_name_desc_param.m_desc_param), sizeof(target->m_interface_data->m_name_desc_param.m_desc_param), &target->m_interface_data->m_rpc_rdata, 4, NULL, NULL);
#ifdef MC_DEBUG
	printf("libmc: using XMCMAN & XMCSERV\n");
#endif

	err = 0;
	// Try XMCSERV RPC
	target->m_interface_data->m_mc_rpc_type = MC_TYPE_XMC;
	// call init function
	if ((ret = sceSifCallRpc(&target->m_interface_data->m_client_data, mcRpcCmd[target->m_interface_data->m_mc_rpc_type][MC_RPCCMD_INIT], 0, &(target->m_interface_data->m_name_desc_param.m_desc_param), sizeof(target->m_interface_data->m_name_desc_param.m_desc_param), &target->m_interface_data->m_rpc_rdata, 12, NULL, NULL)) < 0)
	{
		// init error
#ifdef MC_DEBUG
		printf("libmc: initialisation error\n");
#endif
		err = ret - 100;
	}

	// If result was sceMcResDeniedPermit, RPC was unhandled
	if (!err && rpcStat->result == sceMcResDeniedPermit)
	{
		err = -122;
	}

	if (!err)
	{
		// check if old version of mcserv loaded
		if (rpcStat->mcserv_version < 0x205)
		{
#ifdef MC_DEBUG
			printf("libmc: mcserv is too old (%x)\n", rpcStat->mcserv_version);
#endif
			err = -120;
		}

		// check if old version of mcman loaded
		if (rpcStat->mcman_version < 0x206)
		{
#ifdef MC_DEBUG
			printf("libmc: mcman is too old (%x)\n", rpcStat->mcman_version);
#endif
			err = -121;
		}
	}

	if (!err)
	{
		ret = rpcStat->result;
	}

	if (err && rpcStat->result == sceMcResDeniedPermit)
	{
		err = 0;

		// Try MCSERV RPC
		target->m_interface_data->m_mc_rpc_type = MC_TYPE_MC;
#ifdef MC_DEBUG
		printf("libmc: using MCMAN & MCSERV\n");
#endif

		target->m_interface_data->m_name_desc_param.m_desc_param.offset = -217;

		// call init function
		if ((ret = sceSifCallRpc(&target->m_interface_data->m_client_data, mcRpcCmd[target->m_interface_data->m_mc_rpc_type][MC_RPCCMD_INIT], 0, &(target->m_interface_data->m_name_desc_param.m_desc_param), sizeof(target->m_interface_data->m_name_desc_param.m_desc_param), &target->m_interface_data->m_rpc_rdata, 4, NULL, NULL)) < 0)
		{
			// init error
#ifdef MC_DEBUG
			printf("libmc: initialisation error\n");
#endif
			err = ret - 100;
		}

		// If result was sceMcResDeniedPermit, RPC was unhandled
		if (!err && rpcStat->result == sceMcResDeniedPermit)
		{
			err = -122;
		}

		if (!err)
		{
			ret = target->m_interface_data->m_rpc_rdata.m_result;
		}
	}

	if (err)
	{
		libmc_rpc_reset(target);
		return err;
	}

	// successfully inited
	target->m_interface_data->m_current_command = MC_FUNC_NONE;
	return ret;
}

static int libmc_rpc_get_info(const libmc_target_desc_t *target, int* type, int* free, int* format)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.port	= target->m_port;
	target->m_interface_data->m_name_desc_param.m_desc_param.slot	= target->m_slot;
	if (target->m_interface_data->m_mc_rpc_type == MC_TYPE_MC)
	{
		target->m_interface_data->m_name_desc_param.m_desc_param.size	= (type)	? 1 : 0;
		target->m_interface_data->m_name_desc_param.m_desc_param.offset	= (free)	? 1 : 0;
		target->m_interface_data->m_name_desc_param.m_desc_param.origin	= (format)	? 1 : 0;
	}
	else
	{
		target->m_interface_data->m_name_desc_param.m_desc_param.size	= (format)	? 1 : 0;
		target->m_interface_data->m_name_desc_param.m_desc_param.offset	= (free)	? 1 : 0;
		target->m_interface_data->m_name_desc_param.m_desc_param.origin	= (type)	? 1 : 0;
	}
	target->m_interface_data->m_name_desc_param.m_desc_param.param	= target->m_interface_data->m_extra_send_recv_param.m_end_parameter;
	target->m_interface_data->m_extra_end_param.m_p_type		= type;
	target->m_interface_data->m_extra_end_param.m_p_free		= free;
	target->m_interface_data->m_extra_end_param.m_p_format		= format;
	sceSifWriteBackDCache(target->m_interface_data->m_extra_send_recv_param.m_end_parameter, sizeof(target->m_interface_data->m_extra_send_recv_param.m_end_parameter));
	target->m_interface_data->m_extra_end_param.m_extra_send_recv_param = UNCACHED_SEG(&target->m_interface_data->m_extra_send_recv_param);
	target->m_interface_data->m_extra_end_param.m_p_result = UNCACHED_SEG(&target->m_interface_data->m_rpc_rdata.m_result);

	return libmc_post_rpc(target, MC_RPCCMD_GET_INFO, 1, (target->m_interface_data->m_mc_rpc_type == MC_TYPE_MC) ? (SifRpcEndFunc_t)mcGetInfoApdxOld : (SifRpcEndFunc_t)mcGetInfoApdxNew, &target->m_interface_data->m_extra_end_param);
}

static int libmc_rpc_open(const libmc_target_desc_t *target, const char *name, int mode)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_name_param.m_port		= target->m_port;
	target->m_interface_data->m_name_desc_param.m_name_param.m_slot		= target->m_slot;
	target->m_interface_data->m_name_desc_param.m_name_param.m_flags		= mode;
	strncpy(target->m_interface_data->m_name_desc_param.m_name_param.m_name, name, sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1);
	target->m_interface_data->m_name_desc_param.m_name_param.m_name[sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1] = 0;

	return libmc_post_rpc(target, MC_RPCCMD_OPEN, 0, NULL, NULL);
}

static int libmc_rpc_close(const libmc_target_desc_t *target)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.fd	= target->m_fd;

	return libmc_post_rpc(target, MC_RPCCMD_CLOSE, 1, NULL, NULL);
}

static int libmc_rpc_seek(const libmc_target_desc_t *target, int offset, int origin)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.fd		= target->m_fd;
	target->m_interface_data->m_name_desc_param.m_desc_param.offset	= offset;
	target->m_interface_data->m_name_desc_param.m_desc_param.origin	= origin;

	return libmc_post_rpc(target, MC_RPCCMD_SEEK, 1, NULL, NULL);
}

static int libmc_rpc_read(const libmc_target_desc_t *target, void *buffer, int size)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.fd		= target->m_fd;
	target->m_interface_data->m_name_desc_param.m_desc_param.size	= size;
	target->m_interface_data->m_name_desc_param.m_desc_param.buffer	= buffer;
	target->m_interface_data->m_name_desc_param.m_desc_param.param	= target->m_interface_data->m_extra_send_recv_param.m_end_parameter;
	sceSifWriteBackDCache(buffer, size);
	sceSifWriteBackDCache(target->m_interface_data->m_extra_send_recv_param.m_end_parameter, sizeof(target->m_interface_data->m_extra_send_recv_param.m_end_parameter));
	target->m_interface_data->m_extra_end_param.m_extra_send_recv_param = UNCACHED_SEG(&target->m_interface_data->m_extra_send_recv_param);

	return libmc_post_rpc(target, MC_RPCCMD_READ, 1, (target->m_interface_data->m_mc_rpc_type == MC_TYPE_MC) ? (SifRpcEndFunc_t)mcReadFixAlignOld : (SifRpcEndFunc_t)mcReadFixAlignNew, &target->m_interface_data->m_extra_end_param);
}

static int libmc_rpc_write(const libmc_target_desc_t *target, const void *buffer, int size)
{
	int i;

	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.fd	= target->m_fd;
	if (size < 17)
	{
		target->m_interface_data->m_name_desc_param.m_desc_param.size	= 0;
		target->m_interface_data->m_name_desc_param.m_desc_param.origin	= size;
		target->m_interface_data->m_name_desc_param.m_desc_param.buffer	= 0;
	}
	else
	{
		target->m_interface_data->m_name_desc_param.m_desc_param.size	= size        - ( ((int)((const u8 *)buffer-1) & 0xFFFFFFF0) - (int)((const u8 *)buffer-16) );
		target->m_interface_data->m_name_desc_param.m_desc_param.origin	=               ( ((int)((const u8 *)buffer-1) & 0xFFFFFFF0) - (int)((const u8 *)buffer-16) );
		target->m_interface_data->m_name_desc_param.m_desc_param.buffer	= (void*)((int)(const u8 *)buffer + ( ((int)((const u8 *)buffer-1) & 0xFFFFFFF0) - (int)((const u8 *)buffer-16) ));
	}
	for (i = 0; i < target->m_interface_data->m_name_desc_param.m_desc_param.origin; i++)
	{
		target->m_interface_data->m_name_desc_param.m_desc_param.data[i] = *(char*)((const u8 *)buffer+i);
	}
	FlushCache(0);

	return libmc_post_rpc(target, MC_RPCCMD_WRITE, 1, NULL, NULL);
}

static int libmc_rpc_flush(const libmc_target_desc_t *target)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.fd	= target->m_fd;

	return libmc_post_rpc(target, MC_RPCCMD_FLUSH, 1, NULL, NULL);
}

static int libmc_rpc_mkdir(const libmc_target_desc_t *target, const char* name)
{
	int ret;

	ret = libmc_rpc_open(target, name, 0x40);
	if (ret)
		target->m_interface_data->m_current_command = MC_FUNC_MK_DIR;
	return ret;
}

static int libmc_rpc_chdir(const libmc_target_desc_t *target, const char* newDir, char* currentDir)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_name_param.m_port		= target->m_port;
	target->m_interface_data->m_name_desc_param.m_name_param.m_slot		= target->m_slot;
	target->m_interface_data->m_name_desc_param.m_name_param.m_curdir		= target->m_interface_data->m_extra_send_recv_param.m_cur_dir;
	strncpy(target->m_interface_data->m_name_desc_param.m_name_param.m_name, newDir, sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1);
	target->m_interface_data->m_name_desc_param.m_name_param.m_name[sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1] = 0;
	sceSifWriteBackDCache(target->m_interface_data->m_extra_send_recv_param.m_cur_dir, sizeof(target->m_interface_data->m_extra_send_recv_param.m_cur_dir));
	target->m_interface_data->m_extra_end_param.m_dst_cur_dir = currentDir;
	target->m_interface_data->m_extra_end_param.m_extra_send_recv_param = UNCACHED_SEG(&target->m_interface_data->m_extra_send_recv_param);

	return libmc_post_rpc(target, MC_RPCCMD_CH_DIR, 0, (SifRpcEndFunc_t)mcStoreDir, &target->m_interface_data->m_extra_end_param);
}

static int libmc_rpc_getdir(const libmc_target_desc_t *target, const char *name, unsigned mode, int maxent, sceMcTblGetDir* table)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_name_param.m_port	= target->m_port;
	target->m_interface_data->m_name_desc_param.m_name_param.m_slot	= target->m_slot;
	target->m_interface_data->m_name_desc_param.m_name_param.m_flags	= mode;
	target->m_interface_data->m_name_desc_param.m_name_param.m_maxent	= maxent;
	target->m_interface_data->m_name_desc_param.m_name_param.m_mcT		= table;
	strncpy(target->m_interface_data->m_name_desc_param.m_name_param.m_name, name, sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1);
	target->m_interface_data->m_name_desc_param.m_name_param.m_name[sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1] = 0;
	sceSifWriteBackDCache(table, maxent * sizeof(sceMcTblGetDir));

	return libmc_post_rpc(target, MC_RPCCMD_GET_DIR, 0, NULL, NULL);
}

static int libmc_rpc_setfileinfo(const libmc_target_desc_t *target, const char* name, const sceMcTblGetDir* info, unsigned flags)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_name_param.m_port	= target->m_port;
	target->m_interface_data->m_name_desc_param.m_name_param.m_slot	= target->m_slot;
	target->m_interface_data->m_name_desc_param.m_name_param.m_flags	= flags;	// NOTE: this was ANDed with 7 so that u cant turn off copy protect! :)
	target->m_interface_data->m_name_desc_param.m_name_param.m_mcT		= &(target->m_interface_data->m_extra_send_recv_param.m_file_info_buff);
	memcpy(&(target->m_interface_data->m_extra_send_recv_param.m_file_info_buff), info, sizeof(sceMcTblGetDir));

	strncpy(target->m_interface_data->m_name_desc_param.m_name_param.m_name, name, sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1);
	target->m_interface_data->m_name_desc_param.m_name_param.m_name[sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1] = 0;
	FlushCache(0);

	return libmc_post_rpc(target, MC_RPCCMD_SET_INFO, 0, NULL, NULL);
}

static int libmc_rpc_delete(const libmc_target_desc_t *target, const char *name)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_name_param.m_port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_name_param.m_slot = target->m_slot;
	target->m_interface_data->m_name_desc_param.m_name_param.m_flags = 0;
	strncpy(target->m_interface_data->m_name_desc_param.m_name_param.m_name, name, sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1);
	target->m_interface_data->m_name_desc_param.m_name_param.m_name[sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1] = 0;

	return libmc_post_rpc(target, MC_RPCCMD_DELETE, 0, NULL, NULL);
}

static int libmc_rpc_format(const libmc_target_desc_t *target)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_desc_param.slot = target->m_slot;

	return libmc_post_rpc(target, MC_RPCCMD_FORMAT, 1, NULL, NULL);
}

static int libmc_rpc_unformat(const libmc_target_desc_t *target)
{
	LIBMC_PRE_RPC(target, 0, 0);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_desc_param.port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_desc_param.slot = target->m_slot;

	return libmc_post_rpc(target, MC_RPCCMD_UNFORMAT, 1, NULL, NULL);
}

static int libmc_rpc_get_ent_space(const libmc_target_desc_t *target, const char* path)
{
	LIBMC_PRE_RPC(target, LIBMC_PRE_CHECK_FLAG_ONLY_TYPE, MC_TYPE_XMC);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_name_param.m_port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_name_param.m_slot = target->m_slot;
	strncpy(target->m_interface_data->m_name_desc_param.m_name_param.m_name, path, sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1);
	target->m_interface_data->m_name_desc_param.m_name_param.m_name[sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1] = 0;

	return libmc_post_rpc(target, MC_RPCCMD_GET_ENT, 0, NULL, NULL);
}

static int libmc_rpc_rename(const libmc_target_desc_t *target, const char* oldName, const char* newName)
{
	// I don't think that the old MCSERV module supports this because the v1.00 and v1.01 OSDSYS doesn't seem to have the sceMcRename function at all and the sceMcRename function was only introduced with SCE PS2SDK v1.50. I see that it doesn't work with rom0:MCSERV either way...
	LIBMC_PRE_RPC(target, LIBMC_PRE_CHECK_FLAG_ONLY_TYPE, MC_TYPE_XMC);

	// set global variables
	target->m_interface_data->m_name_desc_param.m_name_param.m_port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_name_param.m_slot = target->m_slot;
	target->m_interface_data->m_name_desc_param.m_name_param.m_flags = 0x10;
	target->m_interface_data->m_name_desc_param.m_name_param.m_mcT = &(target->m_interface_data->m_extra_send_recv_param.m_file_info_buff);
	strncpy(target->m_interface_data->m_name_desc_param.m_name_param.m_name, oldName, sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1);
	target->m_interface_data->m_name_desc_param.m_name_param.m_name[sizeof(target->m_interface_data->m_name_desc_param.m_name_param.m_name) - 1] = 0;
	strncpy((char*)target->m_interface_data->m_extra_send_recv_param.m_file_info_buff.EntryName, newName, sizeof(target->m_interface_data->m_extra_send_recv_param.m_file_info_buff.EntryName) - 1);
	target->m_interface_data->m_extra_send_recv_param.m_file_info_buff.EntryName[sizeof(target->m_interface_data->m_extra_send_recv_param.m_file_info_buff.EntryName) - 1] = 0;
	FlushCache(0);

	return libmc_post_rpc(target, MC_RPCCMD_SET_INFO, 0, NULL, NULL);
}

static int libmc_rpc_erase_block(const libmc_target_desc_t *target, int block, int mode)
{
	LIBMC_PRE_RPC(target, LIBMC_PRE_CHECK_FLAG_ONLY_TYPE, MC_TYPE_MC);

	target->m_interface_data->m_name_desc_param.m_desc_param.port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_desc_param.slot = target->m_slot;
	target->m_interface_data->m_name_desc_param.m_desc_param.offset = block;
	target->m_interface_data->m_name_desc_param.m_desc_param.origin = mode;

	return libmc_post_rpc(target, MC_RPCCMD_ERASE_BLOCK, 1, NULL, NULL);
}

static int libmc_rpc_read_page(const libmc_target_desc_t *target, unsigned int page, void *buffer)
{
	LIBMC_PRE_RPC(target, LIBMC_PRE_CHECK_FLAG_ONLY_TYPE, MC_TYPE_MC);

	target->m_interface_data->m_name_desc_param.m_desc_param.fd = page;
	target->m_interface_data->m_name_desc_param.m_desc_param.port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_desc_param.slot = target->m_slot;
	target->m_interface_data->m_name_desc_param.m_desc_param.buffer = buffer;
	target->m_interface_data->m_name_desc_param.m_desc_param.param = &(target->m_interface_data->m_extra_send_recv_param.m_page_read_align_data);

	sceSifWriteBackDCache(buffer, 0x200);

	return libmc_post_rpc(target, MC_RPCCMD_READ_PAGE, 1, &libmc_ReadAlignFunction, UNCACHED_SEG(&target->m_interface_data->m_extra_send_recv_param));
}

static int libmc_rpc_write_page(const libmc_target_desc_t *target, int page, const void *buffer)
{
	int misaligned;

	LIBMC_PRE_RPC(target, LIBMC_PRE_CHECK_FLAG_ONLY_TYPE, MC_TYPE_MC);

	target->m_interface_data->m_name_desc_param.m_desc_param.fd = page;
	target->m_interface_data->m_name_desc_param.m_desc_param.port = target->m_port;
	target->m_interface_data->m_name_desc_param.m_desc_param.slot = target->m_slot;
	target->m_interface_data->m_name_desc_param.m_desc_param.buffer = (void*)buffer;

	sceSifWriteBackDCache((void*)buffer, 512);

	if ((misaligned=(unsigned int)buffer&0xF)!=0)
	{
		memcpy(target->m_interface_data->m_name_desc_param.m_desc_param.data, buffer, 16-misaligned);
		memcpy((void*)(target->m_interface_data->m_name_desc_param.m_desc_param.data+(16-misaligned)), (void*)((const u8 *)buffer+(16-misaligned)+0x1F0), misaligned);
	}

	return libmc_post_rpc(target, MC_RPCCMD_WRITE_PAGE, 1, NULL, NULL);
}

static int libmc_rpc_change_thread_priority(const libmc_target_desc_t *target, int level)
{
	(void)level;

	LIBMC_PRE_RPC(target, LIBMC_PRE_CHECK_FLAG_ONLY_TYPE, MC_TYPE_XMC);

	// set global variables
//	*(u32*)mcCmd.name = level;

	return libmc_post_rpc(target, MC_RPCCMD_CHG_PRITY, 1, NULL, NULL);
}

static int libmc_rpc_sync(const libmc_target_desc_t *target, int mode, int *cmd, int *result)
{
	int funcIsExecuting;

	// check if any functions are registered
	if (target->m_interface_data->m_current_command == MC_FUNC_NONE)
		return -1;

	// check if function is still processing
	funcIsExecuting = sceSifCheckStatRpc(&target->m_interface_data->m_client_data);

	// if mode = 0, wait for function to finish
	if (mode == 0)
	{
		while (sceSifCheckStatRpc(&target->m_interface_data->m_client_data))
		{
			int i;
			for (i=0; i<100000; i++)
    			;
		}
		// function has finished
		funcIsExecuting = 0;
	}

	// get the function that just finished
	if (cmd)
		*cmd = target->m_interface_data->m_current_command;

	// if function is still processing, return 0
	if (funcIsExecuting == 1)
		return 0;

	// function has finished, so clear last command
	target->m_interface_data->m_current_command = MC_FUNC_NONE;

	// get result
	if (result)
		*result = target->m_interface_data->m_rpc_rdata.m_result;

	return 1;
}

static int libmc_rpc_reset(const libmc_target_desc_t *target)
{
	if (target)
	{
		memset(&(target->m_interface_data->m_client_data), 0, sizeof(target->m_interface_data->m_client_data));
	}
	else
	{
		unsigned int i;
		for (i = 0; i < (sizeof(g_libmc_internal_data.m_interface_data)/sizeof(g_libmc_internal_data.m_interface_data[0])); i += 1)
		{
			memset(&(g_libmc_internal_data.m_interface_data[i].m_client_data), 0, sizeof(g_libmc_internal_data.m_interface_data[i].m_client_data));
		}
	}
	return 0;
}

int mcInit(int type)
{
	libmc_target_desc_t target;

	(void)type;
	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, 0);
	return libmc_rpc_init(&target);
}

int mcGetInfo(int port, int slot, int* type, int* free, int* format)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_get_info(&target, type, free, format);
}

int mcOpen(int port, int slot, const char *name, int mode)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_open(&target, name, mode);
}

int mcClose(int fd)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, fd);
	return libmc_rpc_close(&target);
}

int mcSeek(int fd, int offset, int origin)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, fd);
	return libmc_rpc_seek(&target, offset, origin);
}

int mcRead(int fd, void *buffer, int size)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, fd);
	return libmc_rpc_read(&target, buffer, size);
}

int mcWrite(int fd, const void *buffer, int size)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, fd);
	return libmc_rpc_write(&target, buffer, size);
}

int mcFlush(int fd)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, fd);
	return libmc_rpc_flush(&target);
}

int mcMkDir(int port, int slot, const char* name)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_mkdir(&target, name);
}

int mcChdir(int port, int slot, const char* newDir, char* currentDir)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_chdir(&target, newDir, currentDir);
}

int mcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, sceMcTblGetDir* table)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_getdir(&target, name, mode, maxent, table);
}

int mcSetFileInfo(int port, int slot, const char* name, const sceMcTblGetDir* info, unsigned flags)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_setfileinfo(&target, name, info, flags);
}

int mcDelete(int port, int slot, const char *name)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_delete(&target, name);
}

int mcFormat(int port, int slot)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_format(&target);
}

int mcUnformat(int port, int slot)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_unformat(&target);
}

int mcGetEntSpace(int port, int slot, const char* path)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_get_ent_space(&target, path);
}

int mcRename(int port, int slot, const char* oldName, const char* newName)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_rename(&target, oldName, newName);
}

int mcEraseBlock(int port, int slot, int block, int mode)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_erase_block(&target, block, mode);
}

int mcReadPage(int port, int slot, unsigned int page, void *buffer)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_read_page(&target, page, buffer);
}

int mcWritePage(int port, int slot, int page, const void *buffer)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, port, slot, 0);
	return libmc_rpc_write_page(&target, page, buffer);
}

int mcChangeThreadPriority(int level)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, 0);
	return libmc_rpc_change_thread_priority(&target, level);
}

int mcSync(int mode, int *cmd, int *result)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, 0);
	return libmc_rpc_sync(&target, mode, cmd, result);
}

int mcReset(void)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_SIO2, 0, 0, 0);
	return libmc_rpc_reset(&target);
}

int xfromInit(int type)
{
	libmc_target_desc_t target;

	(void)type;
	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, 0);
	return libmc_rpc_init(&target);
}

int xfromGetInfo(int port, int slot, int* type, int* free, int* format)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_get_info(&target, type, free, format);
}

int xfromOpen(int port, int slot, const char *name, int mode)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_open(&target, name, mode);
}

int xfromClose(int fd)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, fd);
	return libmc_rpc_close(&target);
}

int xfromSeek(int fd, int offset, int origin)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, fd);
	return libmc_rpc_seek(&target, offset, origin);
}

int xfromRead(int fd, void *buffer, int size)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, fd);
	return libmc_rpc_read(&target, buffer, size);
}

int xfromWrite(int fd, const void *buffer, int size)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, fd);
	return libmc_rpc_write(&target, buffer, size);
}

int xfromFlush(int fd)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, fd);
	return libmc_rpc_flush(&target);
}

int xfromMkDir(int port, int slot, const char* name)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_mkdir(&target, name);
}

int xfromChdir(int port, int slot, const char* newDir, char* currentDir)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_chdir(&target, newDir, currentDir);
}

int xfromGetDir(int port, int slot, const char *name, unsigned mode, int maxent, sceMcTblGetDir* table)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_getdir(&target, name, mode, maxent, table);
}

int xfromSetFileInfo(int port, int slot, const char* name, const sceMcTblGetDir* info, unsigned flags)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_setfileinfo(&target, name, info, flags);
}

int xfromDelete(int port, int slot, const char *name)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_delete(&target, name);
}

int xfromFormat(int port, int slot)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_format(&target);
}

int xfromUnformat(int port, int slot)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_unformat(&target);
}

int xfromGetEntSpace(int port, int slot, const char* path)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_get_ent_space(&target, path);
}

int xfromRename(int port, int slot, const char* oldName, const char* newName)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_rename(&target, oldName, newName);
}

int xfromEraseBlock(int port, int slot, int block, int mode)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_erase_block(&target, block, mode);
}

int xfromReadPage(int port, int slot, unsigned int page, void *buffer)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_read_page(&target, page, buffer);
}

int xfromWritePage(int port, int slot, int page, const void *buffer)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, port, slot, 0);
	return libmc_rpc_write_page(&target, page, buffer);
}

int xfromChangeThreadPriority(int level)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, 0);
	return libmc_rpc_change_thread_priority(&target, level);
}

int xfromSync(int mode, int *cmd, int *result)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, 0);
	return libmc_rpc_sync(&target, mode, cmd, result);
}

int xfromReset(void)
{
	libmc_target_desc_t target;

	libmc_setup_target(&target, MC_INTERFACE_DEV9, 0, 0, 0);
	return libmc_rpc_reset(&target);
}
