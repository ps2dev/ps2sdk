/*
  _____     ___ ____
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2001, Gustavo Scotti (gustavo@scotti.com)
                           (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
  ------------------------------------------------------------------------
  loadfile.c
			   IOP executable file loader API.
*/

/** @defgroup loadfile EE LOADFILE: ELF and IRX loader client library. */

#include <tamtypes.h>
#include <ps2lib_err.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>

#include <loadfile.h>
#include <iopheap.h>
#include <fileio.h>

enum _lf_functions {
	LF_F_MOD_LOAD = 0,
	LF_F_ELF_LOAD,

	LF_F_SET_ADDR,
	LF_F_GET_ADDR,

	LF_F_MG_MOD_LOAD,
	LF_F_MG_ELF_LOAD,

	LF_F_MOD_BUF_LOAD,
};

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

extern SifRpcClientData_t _lf_cd;
extern int _lf_init;

int _SifLoadModule(const char *path, int arg_len, const char *args,
		int *modres, int fno);
int _SifLoadElfPart(const char *path, const char *secname, t_ExecData *data, int fno);
int _SifLoadModuleBuffer(void *ptr, int arg_len, const char *args, int *modres);

#if defined(F_SifLoadFileInit) || defined(DOXYGEN)
SifRpcClientData_t _lf_cd;
int _lf_init = 0;

/** Initialize the LOADFILE library.
    @ingroup loadfile

    @returns 0 on success, E_SIF_RPC_BIND if unable to connect to the server.

    Initializes the LOADFILE API and binds to the remote LOADFILE RPC server.
*/
int SifLoadFileInit()
{
	int res;

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

#if defined(F_SifLoadFileExit) || defined(DOXYGEN)

/** Reset the LOADFILE library.
    @ingroup loadfile

    Deinitializes the LOADFILE library in preparation of an IOP reset.
*/
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
		int fno)
{
	struct _lf_module_load_arg arg;

	if (!_lf_init && SifLoadFileInit() < 0)
		return -E_LIB_API_INIT;

	memset(&arg, 0, sizeof arg);

	strncpy(arg.path, path, LF_PATH_MAX);

	if (args && arg_len) {
		arg.p.arg_len = arg_len > LF_ARG_MAX ? LF_ARG_MAX : arg_len;
		memcpy(arg.args, args, arg.p.arg_len);
	}

	if (SifCallRpc(&_lf_cd, fno, 0, &arg, sizeof arg, &arg, 8, NULL, NULL) < 0)
		return -E_SIF_RPC_CALL;

	if (modres)
		*modres = arg.modres;

	return arg.p.result;
}
#endif

#if defined(F_SifLoadModule) || defined(DOXYGEN)

/** Load and execute an IRX module.
    @ingroup loadfile

    @param path    Path to an IRX module
    @param arg_len Length, in bytes, of the argument list
    @param args    List of arguments to pass to the IRX on startup

    @returns The ID of the loaded module on success, or an error if the module
    couldn't be loaded.

    Loads an IRX module from the specified path, and executes it.  The args
    parameter specifies a list of arguments that are passed to the loaded module
    as the argv[] array.  Each argument string in args must be seperated by NUL ('\\0').
    Pass the length of the entire args list, including the NUL seperators in the args_len
    parameter.

    @note By default, modules cannot be loaded directly from the memory card.  If you need to
    load modules from the memory card, you should use ::SifExecModuleFile or link with the
    sbv-lite library (http://www.0xd6.org).

    @see SifExecModuleFile
*/
int SifLoadModule(const char *path, int arg_len, const char *args)
{
	return _SifLoadModule(path, arg_len, args, NULL, LF_F_MOD_LOAD);
}
#endif

#if defined(F_SifLoadStartModule) || defined(DOXYGEN)

/** Load and execute and IRX module, and retrieve the module's load status.
    @ingroup loadfile

    @param path    Path to an IRX module
    @param arg_len Length, in bytes, of the argument list
    @param args    List of arguments to pass to the IRX on startup
    @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function

    @returns The ID of the loaded module on success, or an error if the module
    couldn't be loaded.

    Loads an IRX module from the specified path, and executes it.  See ::SifLoadModule for
    details on the arg_len and args parameters.  If mod_res is non-NULL, the result code from
    the module's _start() function is stored here.

    @see SifLoadModule
*/
int SifLoadStartModule(const char *path, int arg_len, const char *args, int *mod_res)
{
	return _SifLoadModule(path, arg_len, args, mod_res, LF_F_MOD_LOAD);
}
#endif

#if defined(F_SifLoadModuleEncrypted) || defined(DOXYGEN)

/** Load and execute an IRX module encrypted with MagicGate.
    @ingroup loadfile

    @param path    Path to an IRX module
    @param arg_len Length, in bytes, of the argument list
    @param args    List of arguments to pass to the IRX on startup

    @returns The ID of the loaded module on success, or an error if the module
    couldn't be loaded.

    There is no real use for this function; it is only included in ps2lib for completeness.

    @see SifLoadModule
*/
int SifLoadModuleEncrypted(const char *path, int arg_len, const char *args)
{
	return _SifLoadModule(path, arg_len, args, NULL, LF_F_MG_MOD_LOAD);
}
#endif

#ifdef F__SifLoadElfPart
struct _lf_elf_load_arg {
	union {
		u32	epc;
		int	result;
	} p;
	u32	gp;
	char	path[LF_PATH_MAX];
	char	secname[LF_ARG_MAX];
} ALIGNED(16);

int _SifLoadElfPart(const char *path, const char *secname, t_ExecData *data, int fno)
{
	struct _lf_elf_load_arg arg;

	if (!_lf_init && SifLoadFileInit() < 0)
		return -E_LIB_API_INIT;

	strncpy(arg.path, path, LF_PATH_MAX);
	strncpy(arg.secname, secname, LF_ARG_MAX);

	if (SifCallRpc(&_lf_cd, fno, 0, &arg, sizeof arg, &arg,
				sizeof(t_ExecData), NULL, NULL) < 0)
		return -E_SIF_RPC_CALL;

	if (arg.p.result < 0)
		return arg.p.result;

	if (data) {
		data->epc = arg.p.epc;
		data->gp  = arg.gp;
	}

	return 0;
}
#endif

#if defined(F_SifLoadElfPart) || defined(DOXYGEN)

/** Load the specified section of an ELF executable into EE RAM.
    @ingroup loadfile

    @param path    Path to an ELF executable
    @param secname The name of a single section to load from the ELF, or "all" to load all sections
    @param data    Pointer to a variable that will store information about the loaded executable

    @returns 0 on success, or an error code if an error occurred.

    Loads the section secname from the ELF file specified by path into EE RAM.

    data points to a t_ExecData structure where information about the loaded
    section will be returned.

    @warning Don't ever use this function, it does not work as intended.
*/
int SifLoadElfPart(const char *path, const char *secname, t_ExecData *data)
{
	return _SifLoadElfPart(path, secname, data, LF_F_ELF_LOAD);
}
#endif

#if defined(F_SifLoadElf) || defined(DOXYGEN)

/** Load an ELF executable into EE RAM.
    @ingroup loadfile

    @param path Path to an ELF executable
    @param data Pointer to a variable that will store information about the loaded executable

    @returns 0 on success, or an error code if an error occurred.

    Loads the ELF executable specified by path into EE RAM.  data points to a
    t_ExecData structure where information about the loaded executable will be returned.

    @warning It's possible to overwrite your program if the ELF that you're trying to
    load overlaps your program's memory space.  Always ensure that the target ELF will
    load before or after your program's code and data addresses.
*/
int SifLoadElf(const char *path, t_ExecData *data)
{
	u32 secname = 0x6c6c61;  /* "all" */
	return _SifLoadElfPart(path, (char *)&secname, data, LF_F_ELF_LOAD);
}
#endif

#if defined(F_SifLoadElfEncrypted) || defined(DOXYGEN)

/** Load a MagicGate-encrypted ELF executable into EE RAM.
    @ingroup loadfile

    @param path Path to an ELF executable
    @param data Pointer to a variable that will store information about the loaded executable

    @returns 0 on success, or an error code if an error occurred.

    There is no real use for this function; it is only included in ps2lib for completeness.

    @see SifLoadElf
*/
int SifLoadElfEncrypted(const char *path, t_ExecData *data)
{
	u32 secname = 0x6c6c61;  /* "all" */
	return _SifLoadElfPart(path, (char *)&secname, data, LF_F_MG_ELF_LOAD);
}
#endif

#if defined(F_SifIopSetVal) || defined(DOXYGEN)

/** Write a value to an address within the IOP's memory space.
    @ingroup loadfile

    @param iop_addr Address to modify in the IOP's address space
    @param val      Data to write at iop_addr
    @param type     A type describing the size of the val argument

    @returns Previous value at iop_addr  on success (?), or an error code if an
    error occurred.

    Writes the data passed in val to the IOP address specified by iop_addr.  Use
    the following types specify the size of data write:
    - ::LF_VAL_BYTE to write an 8-bit value
    - ::LF_VAL_SHORT to write a 16-bit value
    - ::LF_VAL_LONG to write a 32-bit value

    @note Because iop_addr can be any address within the IOP's address space, it's
    possible to modify IOP hardware registers.  However, in most cases the EE can
    modify the IOP's hardware registers directly so there is little use for this function.

    @see SifIopGetVal
*/
int SifIopSetVal(u32 iop_addr, int val, int type)
{
	struct _lf_iop_val_arg arg;

	if (!_lf_init && SifLoadFileInit() < 0)
		return -E_LIB_API_INIT;

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
		return -E_SIF_RPC_CALL;

	return arg.p.result;
}
#endif

#if defined(F_SifIopGetVal) || defined(DOXYGEN)

/** Read a value from an address within the IOP's memory space.
    @ingroup loadfile

    @param iop_addr Address within the IOP's address space to read from
    @param val      Pointer to a variable that stores the value read
    @param type     A type describing the size of the data to read

    @returns 0 on success, or an error code if an error occurred.

    Reads data of the size specified by type from the IOP address specified by
    iop_addr.  Use the following types to specify the size of the data to read:
    - ::LF_VAL_BYTE to write an 8-bit value
    - ::LF_VAL_SHORT to write a 16-bit value
    - ::LF_VAL_LONG to write a 32-bit value

    @note Because iop_addr can be any address within the IOP's address space, it's
    possible to read from IOP hardware registers.  However, in most cases the EE can
    read the IOP's hardware registers directly so there is little use for this function.
    @see SifIopSetVal
*/
int SifIopGetVal(u32 iop_addr, void *val, int type)
{
	struct _lf_iop_val_arg arg;

	if (!_lf_init && SifLoadFileInit() < 0)
		return -E_LIB_API_INIT;

	arg.p.iop_addr = iop_addr;
	arg.type = type;

	if (SifCallRpc(&_lf_cd, LF_F_GET_ADDR, 0, &arg, sizeof arg, &arg, 4,
				NULL, NULL) < 0)
		return -E_SIF_RPC_CALL;

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

	if (!_lf_init && SifLoadFileInit() < 0)
		return -E_LIB_API_INIT;

	memset(&arg, 0, sizeof arg);

	arg.p.ptr = ptr;
	if (args && arg_len) {
		arg.q.arg_len = arg_len > LF_ARG_MAX ? LF_ARG_MAX : arg_len;
		memcpy(arg.args, args, arg.q.arg_len);
	}

	if (SifCallRpc(&_lf_cd, LF_F_MOD_BUF_LOAD, 0, &arg, sizeof arg, &arg, 8,
				NULL, NULL) < 0)
		return -E_SIF_RPC_CALL;

	if (modres)
		*modres = arg.q.modres;

	return arg.p.result;
}
#endif

#if defined(F_SifLoadModuleBuffer) || defined(DOXYGEN)

/** Load and execute an IRX module from a buffer in IOP RAM.
    @ingroup loadfile

    @param ptr     Pointer to a buffer in IOP RAM where the IRX module will be loaded from
    @param arg_len Length, in bytes, of the argument list
    @param args    List of arguments to pass to the IRX on startup

    @returns The ID of the loaded module on success, or an error if the module couldn't
    be loaded.

    Loads an IRX module from a buffer located in IOP RAM.  See ::SifLoadModule for details on
    the arg_len and args parameters.

    @note If you need to load an IRX module onto the IOP directly from a buffer in EE RAM, then
    use ::SifExecModuleBuffer.

    @see SifLoadModule, SifExecModuleBuffer
*/
int SifLoadModuleBuffer(void *ptr, int arg_len, const char *args)
{
	return _SifLoadModuleBuffer(ptr, arg_len, args, NULL);
}
#endif

#if defined(F_SifLoadStartModuleBuffer) || defined(DOXYGEN)

/** Load and execute an IRX module from a buffer in IOP RAM, and retrieve the module's load status.
    @ingroup loadfile

    @param ptr     Pointer to a buffer in IOP RAM where the IRX module will be loaded from
    @param arg_len Length, in bytes, of the argument list
    @param args    List of arguments to pass to the IRX on startup
    @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function

    @returns The ID of the loaded module on success, or an error if the module
    couldn't be loaded.

    Loads an IRX module from a buffer located in IOP RAM.  See ::SifLoadModule for details on
    the arg_len and args parameters.  If mod_res is non-NULL, the result code from the module's
    _start() function is stored here.

    @note If you need to load an IRX module onto the IOP directly from a buffer in EE RAM, then
    use ::SifExecModuleBuffer.

    @see SifLoadModule, SifExecModuleBuffer
*/
int SifLoadStartModuleBuffer(void *ptr, int arg_len, const char *args, int *mod_res)
{
	return _SifLoadModuleBuffer(ptr, arg_len, args, mod_res);
}
#endif

#if defined(F_SifExecModuleBuffer) || defined(DOXYGEN)

/** Transfer an IRX module from EE RAM to IOP RAM and execute it.
    @ingroup loadfile

    @param ptr     Pointer to a buffer in EE RAM where the IRX module will be loaded from
    @param size    Size of the buffer in EE RAM that contains the IRX module
    @param arg_len Length, in bytes, of the argument list
    @param args    List of arguments to pass to the IRX on startup
    @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function

    @returns The ID of the loaded module on success, or an error if the module couldn't
    be loaded.

    Transfers an IRX module stored in a buffer in EE RAM to the IOP, and calls ::SifLoadModuleBuffer to
    load and execute the module.  See ::SifLoadModule for details on the arg_len and args parameters.
    If mod_res is non-NULL, the result code from the module's _start() function is stored here.

    @see SifLoadModule, SifLoadModuleBuffer
*/
int SifExecModuleBuffer(void *ptr, u32 size, u32 arg_len, const char *args, int *mod_res)
{
	SifDmaTransfer_t dmat;
	void *iop_addr;
	int res;

	if (!(iop_addr = SifAllocIopHeap(size)))
		return -E_IOP_NO_MEMORY;

	dmat.src = ptr;
	dmat.dest = iop_addr;
	dmat.size = size;
	dmat.attr = 0;
	SifSetDma(&dmat, 1);

	res = _SifLoadModuleBuffer(iop_addr, arg_len, args, mod_res);
	SifFreeIopHeap(iop_addr);

	return res;
}
#endif

#if defined(F_SifExecModuleFile) || defined(DOXYGEN)

/** Read an IRX module from a file into IOP RAM and execute it.
    @ingroup loadfile

    @param path    Path to an IRX module
    @param arg_len Length, in bytes, of the argument list
    @param args    List of arguments to pass to the IRX on startup
    @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function

    @returns The ID of the loaded module on success, or an error if the module
    couldn't be loaded.

    Loads an IRX module from the specified path into a buffer on the IOP, then calls ::SifLoadModuleBuffer
    to load and execute the module.  See ::SifLoadModule for details on the arg_len and args parameters.
    This function is useful if you need to load an IRX module from a prohibited device, such as the memory card.

    @see SifLoadModule, SifLoadModuleBuffer
*/
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
