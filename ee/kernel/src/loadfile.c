/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP executable file loader API.
 * @defgroup loadfile EE LOADFILE: ELF and IRX loader client library.
 */

#include <tamtypes.h>
#include <ps2lib_err.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>

#include <loadfile.h>
#include <iopheap.h>
#include <fileio.h>

struct _lf_iop_val_arg {
	union {
		u32	iop_addr;
		int	result;
	} p;
	int	type;
	union {
		u8	b;
		u16	s;
		u32	l;
	} val;
} ALIGNED(16);

extern int _iop_reboot_count;
extern SifRpcClientData_t _lf_cd;
extern int _lf_init;

int _SifLoadElfPart(const char *path, const char *secname, t_ExecData *data, int fno);
int _SifLoadModuleBuffer(void *ptr, int arg_len, const char *args, int *modres);

#if defined(F_SifLoadFileInit)
SifRpcClientData_t _lf_cd;
int _lf_init = 0;

int SifLoadFileInit()
{
	int res;
	static int _rb_count = 0;
	if(_rb_count != _iop_reboot_count)
	{
	    _rb_count = _iop_reboot_count;
	    memset(&_lf_cd, 0, sizeof _lf_cd);
	    _lf_init = 0;
	}

    if (_lf_init)
		return 0;

	SifInitRpc(0);

	while ((res = SifBindRpc(&_lf_cd, 0x80000006, 0)) >= 0 && !_lf_cd.server)
		nopdelay();

	if (res < 0)
		return -E_SIF_RPC_BIND;

	_lf_init = 1;
	return 0;
}
#endif

#if defined(F_SifLoadFileExit)
void SifLoadFileExit()
{
	_lf_init = 0;
	memset(&_lf_cd, 0, sizeof _lf_cd);
}
#endif

#ifdef F__SifLoadModule
struct _lf_module_load_arg {
	union {
		int	arg_len;
		int	result;
	} p;
	int	modres;
	char	path[LF_PATH_MAX];
	char	args[LF_ARG_MAX];
} ALIGNED(16);

int _SifLoadModule(const char *path, int arg_len, const char *args, int *modres,
		int fno, int dontwait)
{
	struct _lf_module_load_arg arg;

	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	memset(&arg, 0, sizeof arg);

	strncpy(arg.path, path, LF_PATH_MAX - 1);
	arg.path[LF_PATH_MAX - 1] = 0;

	if (args && arg_len) {
		arg.p.arg_len = arg_len > LF_ARG_MAX ? LF_ARG_MAX : arg_len;
		memcpy(arg.args, args, arg.p.arg_len);
	} else {
		arg.p.arg_len = 0;
	}

	if (SifCallRpc(&_lf_cd, fno, dontwait, &arg, sizeof arg, &arg, 8, NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	if (modres)
		*modres = arg.modres;

	return arg.p.result;
}
#endif

#if defined(F_SifLoadModule)
int SifLoadModule(const char *path, int arg_len, const char *args)
{
	return _SifLoadModule(path, arg_len, args, NULL, LF_F_MOD_LOAD, 0);
}
#endif

#if defined(F_SifLoadStartModule)
int SifLoadStartModule(const char *path, int arg_len, const char *args, int *mod_res)
{
	return _SifLoadModule(path, arg_len, args, mod_res, LF_F_MOD_LOAD, 0);
}
#endif

#if defined(F_SifLoadModuleEncrypted)
int SifLoadModuleEncrypted(const char *path, int arg_len, const char *args)
{
	return _SifLoadModule(path, arg_len, args, NULL, LF_F_MG_MOD_LOAD, 0);
}
#endif

#ifdef F_SifStopModule
struct _lf_module_stop_arg {
	union {
		int	id;
		int	result;
	} p;
	union {
    		int	arg_len;
		int	modres;
	} q;
	char	dummy[LF_PATH_MAX];
	char	args[LF_ARG_MAX];
} ALIGNED(16);

int SifStopModule(int id, int arg_len, const char *args, int *mod_res)
{
	struct _lf_module_stop_arg arg;

	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	arg.p.id = id;

	if (args && arg_len) {
		arg.q.arg_len = arg_len > LF_ARG_MAX ? LF_ARG_MAX : arg_len;
		memcpy(arg.args, args, arg.q.arg_len);
	} else {
		arg.q.arg_len = 0;
	}

	if (SifCallRpc(&_lf_cd, LF_F_MOD_STOP, 0, &arg, sizeof arg, &arg, 8, NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	if (mod_res)
		*mod_res = arg.q.modres;

	return arg.p.result;
}
#endif

#ifdef F_SifUnloadModule
union _lf_module_unload_arg {
	int	id;
	int	result;
} ALIGNED(16);

int SifUnloadModule(int id)
{
	union _lf_module_unload_arg arg;

	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	arg.id = id;

	if (SifCallRpc(&_lf_cd, LF_F_MOD_UNLOAD, 0, &arg, sizeof arg, &arg, 4, NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	return arg.result;
}
#endif

#ifdef F_SifSearchModuleByName
struct _lf_search_module_by_name_arg {
	int	id;
	int	dummy1;
	char	name[LF_PATH_MAX];
	char	dummy2[LF_ARG_MAX];
} ALIGNED(16);

int SifSearchModuleByName(const char * name)
{
	struct _lf_search_module_by_name_arg arg;
	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	strncpy(arg.name, name, LF_PATH_MAX - 1);
	arg.name[LF_PATH_MAX - 1] = 0;

	if (SifCallRpc(&_lf_cd, LF_F_SEARCH_MOD_BY_NAME, 0, &arg, sizeof arg, &arg, 4, NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	return arg.id;
}
#endif

#ifdef F_SifSearchModuleByAddress
struct _lf_search_module_by_address_arg {
	union {
		const void	*ptr;
		int	id;
	} p;
} ALIGNED(16);

int SifSearchModuleByAddress(const void *ptr)
{
	struct _lf_search_module_by_address_arg arg;
	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	arg.p.ptr = ptr;

	if (SifCallRpc(&_lf_cd, LF_F_SEARCH_MOD_BY_ADDRESS, 0, &arg, sizeof arg, &arg, 4, NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	return arg.p.id;
}
#endif

#ifdef F__SifLoadElfPart
struct _lf_elf_load_arg {
	u32	epc;
	u32	gp;
	char	path[LF_PATH_MAX];
	char	secname[LF_ARG_MAX];
} ALIGNED(16);

int _SifLoadElfPart(const char *path, const char *secname, t_ExecData *data, int fno)
{
	struct _lf_elf_load_arg arg;

	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	strncpy(arg.path, path, LF_PATH_MAX - 1);
	strncpy(arg.secname, secname, LF_ARG_MAX - 1);
	arg.path[LF_PATH_MAX - 1] = 0;
	arg.secname[LF_ARG_MAX - 1] = 0;

	if (SifCallRpc(&_lf_cd, fno, 0, &arg, sizeof arg, &arg,
				sizeof(t_ExecData), NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	if (arg.epc != 0)
	{
		data->epc = arg.epc;
		data->gp  = arg.gp;

		return 0;
	} else
		return -SCE_ELOADMISS;
}
#endif

#if defined(F_SifLoadElfPart)
int SifLoadElfPart(const char *path, const char *secname, t_ExecData *data)
{
	return _SifLoadElfPart(path, secname, data, LF_F_ELF_LOAD);
}
#endif

#if defined(F_SifLoadElf)
int SifLoadElf(const char *path, t_ExecData *data)
{
	u32 secname = 0x6c6c61;  /* "all" */
	return _SifLoadElfPart(path, (char *)&secname, data, LF_F_ELF_LOAD);
}
#endif

#if defined(F_SifLoadElfEncrypted)
int SifLoadElfEncrypted(const char *path, t_ExecData *data)
{
	u32 secname = 0x6c6c61;  /* "all" */
	return _SifLoadElfPart(path, (char *)&secname, data, LF_F_MG_ELF_LOAD);
}
#endif

#if defined(F_SifIopSetVal)
int SifIopSetVal(u32 iop_addr, int val, int type)
{
	struct _lf_iop_val_arg arg;

	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	switch (type) {
		case LF_VAL_BYTE:
			arg.val.b = (u8)(val & 0xff);
			break;
		case LF_VAL_SHORT:
			arg.val.s = (u16)(val & 0xffff);
			break;
		case LF_VAL_LONG:
			arg.val.l = val;
			break;
		default:
			return -E_LIB_INVALID_ARG;
	}

	arg.p.iop_addr = iop_addr;
	arg.type = type;

	if (SifCallRpc(&_lf_cd, LF_F_SET_ADDR, 0, &arg, sizeof arg, &arg, 4,
				NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	return arg.p.result;
}
#endif

#if defined(F_SifIopGetVal)
int SifIopGetVal(u32 iop_addr, void *val, int type)
{
	struct _lf_iop_val_arg arg;

	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	arg.p.iop_addr = iop_addr;
	arg.type = type;

	if (SifCallRpc(&_lf_cd, LF_F_GET_ADDR, 0, &arg, sizeof arg, &arg, 4,
				NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	if (val) {
		switch (type) {
			case LF_VAL_BYTE:
				*(u8 *)val  = (u8)arg.p.result & 0xff;
				break;
			case LF_VAL_SHORT:
				*(u16 *)val = (u16)arg.p.result & 0xffff;
				break;
			case LF_VAL_LONG:
				*(u32 *)val = arg.p.result;
				break;
		}
	}

	return 0;
}
#endif

#ifdef F__SifLoadModuleBuffer
struct _lf_module_buffer_load_arg {
	union {
		void	*ptr;
		int	result;
	} p;
	union {
		int	arg_len;
		int	modres;
	} q;
	char	unused[LF_PATH_MAX];
	char	args[LF_ARG_MAX];
} ALIGNED(16);

int _SifLoadModuleBuffer(void *ptr, int arg_len, const char *args, int *modres)
{
	struct _lf_module_buffer_load_arg arg;

	if (SifLoadFileInit() < 0)
		return -SCE_EBINDMISS;

	memset(&arg, 0, sizeof arg);

	arg.p.ptr = ptr;
	if (args && arg_len) {
		arg.q.arg_len = arg_len > LF_ARG_MAX ? LF_ARG_MAX : arg_len;
		memcpy(arg.args, args, arg.q.arg_len);
	} else {
		arg.q.arg_len = 0;
	}

	if (SifCallRpc(&_lf_cd, LF_F_MOD_BUF_LOAD, 0, &arg, sizeof arg, &arg, 8,
				NULL, NULL) < 0)
		return -SCE_ECALLMISS;

	if (modres)
		*modres = arg.q.modres;

	return arg.p.result;
}
#endif

#if defined(F_SifLoadModuleBuffer)
int SifLoadModuleBuffer(void *ptr, int arg_len, const char *args)
{
	return _SifLoadModuleBuffer(ptr, arg_len, args, NULL);
}
#endif

#if defined(F_SifLoadStartModuleBuffer)
int SifLoadStartModuleBuffer(void *ptr, int arg_len, const char *args, int *mod_res)
{
	return _SifLoadModuleBuffer(ptr, arg_len, args, mod_res);
}
#endif

#if defined(F_SifExecModuleBuffer)
int SifExecModuleBuffer(void *ptr, u32 size, u32 arg_len, const char *args, int *mod_res)
{
	SifDmaTransfer_t dmat;
	void *iop_addr;
	int res;
	unsigned int qid;

	/* Round the size up to the nearest 16 bytes. */
	size = (size + 15) & -16;

	if (!(iop_addr = SifAllocIopHeap(size)))
		return -E_IOP_NO_MEMORY;

	dmat.src = ptr;
	dmat.dest = iop_addr;
	dmat.size = size;
	dmat.attr = 0;
	SifWriteBackDCache(ptr, size);
	qid = SifSetDma(&dmat, 1);

	if (!qid)
	    return -1; // should have a better error here...

	while(SifDmaStat(qid) >= 0);

	res = _SifLoadModuleBuffer(iop_addr, arg_len, args, mod_res);
	SifFreeIopHeap(iop_addr);

	return res;
}
#endif

#if defined(F_SifExecModuleFile)
int SifExecModuleFile(const char *path, u32 arg_len, const char *args, int *mod_res)
{
	void *iop_addr;
	int res, size, fd;

	if ((fd = fioOpen(path, O_RDONLY)) < 0)
		return fd;

	if ((size = fioLseek(fd, 0, SEEK_END)) < 0)
		return size;

	fioClose(fd);

	if (!(iop_addr = SifAllocIopHeap(size)))
		return -E_IOP_NO_MEMORY;

	if ((res = SifLoadIopHeap(path, iop_addr)) < 0) {
		SifFreeIopHeap(iop_addr);
		return res;
	}

	res = _SifLoadModuleBuffer(iop_addr, arg_len, args, mod_res);
	SifFreeIopHeap(iop_addr);

	return res;
}
#endif
