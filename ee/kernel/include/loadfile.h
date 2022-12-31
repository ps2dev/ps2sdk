/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP executable file loader.
 */

#ifndef __LOADFILE_H__
#define __LOADFILE_H__

#include <tamtypes.h>

#include <loadfile-common.h>

/* Extended error codes */
/** Could not bind with RPC server */
#define SCE_EBINDMISS 0x10000
/** Could not call the RPC function */
#define SCE_ECALLMISS 0x10001
/** ELF/Module load failed */
#define SCE_ELOADMISS 0x10003

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the LOADFILE library.
 * @ingroup loadfile
 *
 * @returns 0 on success, E_SIF_RPC_BIND if unable to connect to the server.
 *
 * Initializes the LOADFILE API and binds to the remote LOADFILE RPC server.
 */
int SifLoadFileInit(void);
/** Reset the LOADFILE library.
 * @ingroup loadfile
 *
 * Deinitializes the LOADFILE library in preparation of an IOP reset.
 */
void SifLoadFileExit(void);

/** Load and execute an IRX module.
 * @ingroup loadfile
 *
 * @param path    Path to an IRX module
 * @param arg_len Length, in bytes, of the argument list
 * @param args    List of arguments to pass to the IRX on startup
 *
 * @returns The ID of the loaded module on success, or an error if the module
 * couldn't be loaded.
 *
 * Loads an IRX module from the specified path, and executes it.  The args
 * parameter specifies a list of arguments that are passed to the loaded module
 * as the argv[] array.  Each argument string in args must be seperated by NUL ('\\0').
 * Pass the length of the entire args list, including the NUL seperators in the args_len
 * parameter.
 *
 * @note By default, modules cannot be loaded directly from the memory card.  If you need to
 * load modules from the memory card, you should use ::SifExecModuleFile or link with the
 * sbv-lite library (http://www.0xd6.org).
 *
 * @see SifExecModuleFile
 */
int SifLoadModule(const char *path, int arg_len, const char *args);
/** Load and execute and IRX module, and retrieve the module's load status.
 * @ingroup loadfile
 *
 * @param path    Path to an IRX module
 * @param arg_len Length, in bytes, of the argument list
 * @param args    List of arguments to pass to the IRX on startup
 * @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function
 *
 * @returns The ID of the loaded module on success, or an error if the module
 * couldn't be loaded.
 *
 * Loads an IRX module from the specified path, and executes it.  See ::SifLoadModule for
 * details on the arg_len and args parameters.  If mod_res is non-NULL, the result code from
 * the module's _start() function is stored here.
 *
 * @see SifLoadModule
 */
int SifLoadStartModule(const char *path, int arg_len, const char *args, int *mod_res);

/** Load and execute an IRX module encrypted with MagicGate.
 * @ingroup loadfile
 *
 * @param path    Path to an IRX module
 * @param arg_len Length, in bytes, of the argument list
 * @param args    List of arguments to pass to the IRX on startup
 *
 * @returns The ID of the loaded module on success, or an error if the module
 * couldn't be loaded.
 *
 * There is no real use for this function; it is only included in ps2lib for completeness.
 *
 * @see SifLoadModule
 */
int SifLoadModuleEncrypted(const char *path, int arg_len, const char *args);

/** Load and execute an IRX module from a buffer in IOP RAM.
 * @ingroup loadfile
 *
 * @param ptr     Pointer to a buffer in IOP RAM where the IRX module will be loaded from
 * @param arg_len Length, in bytes, of the argument list
 * @param args    List of arguments to pass to the IRX on startup
 *
 * @returns The ID of the loaded module on success, or an error if the module couldn't
 * be loaded.
 *
 * Loads an IRX module from a buffer located in IOP RAM.  See ::SifLoadModule for details on
 * the arg_len and args parameters.
 *
 * @note If you need to load an IRX module onto the IOP directly from a buffer in EE RAM, then
 * use ::SifExecModuleBuffer.
 *
 * @see SifLoadModule, SifExecModuleBuffer
 */
int SifLoadModuleBuffer(void *ptr, int arg_len, const char *args);
/** Load and execute an IRX module from a buffer in IOP RAM, and retrieve the module's load status.
 * @ingroup loadfile
 *
 * @param ptr     Pointer to a buffer in IOP RAM where the IRX module will be loaded from
 * @param arg_len Length, in bytes, of the argument list
 * @param args    List of arguments to pass to the IRX on startup
 * @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function
 *
 * @returns The ID of the loaded module on success, or an error if the module
 * couldn't be loaded.
 *
 * Loads an IRX module from a buffer located in IOP RAM.  See ::SifLoadModule for details on
 * the arg_len and args parameters.  If mod_res is non-NULL, the result code from the module's
 * _start() function is stored here.
 *
 * @note If you need to load an IRX module onto the IOP directly from a buffer in EE RAM, then
 * use ::SifExecModuleBuffer.
 *
 * @see SifLoadModule, SifExecModuleBuffer
 */
int SifLoadStartModuleBuffer(void *ptr, int arg_len, const char *args, int *mod_res);

int SifStopModule(int id, int arg_len, const char *args, int *mod_res);
int SifUnloadModule(int id);
int SifSearchModuleByName(const char *name);
int SifSearchModuleByAddress(const void *ptr);

/** Load the specified section of an ELF executable into EE RAM.
 * @ingroup loadfile
 *
 * @param path    Path to an ELF executable
 * @param secname The name of a single section to load from the ELF, or "all" to load all sections
 * @param data    Pointer to a variable that will store information about the loaded executable
 *
 * @returns 0 on success, or an error code if an error occurred.
 *
 * Loads the section secname from the ELF file specified by path into EE RAM.
 *
 * data points to a t_ExecData structure where information about the loaded
 * section will be returned.
 *
 * @warning Don't ever use this function, it does not work as intended.
 */
int SifLoadElfPart(const char *path, const char *secname, t_ExecData *data);
/** Load an ELF executable into EE RAM.
 * @ingroup loadfile
 *
 * @param path Path to an ELF executable
 * @param data Pointer to a variable that will store information about the loaded executable
 *
 * @returns 0 on success, or an error code if an error occurred.
 *
 * Loads the ELF executable specified by path into EE RAM.  data points to a
 * t_ExecData structure where information about the loaded executable will be returned.
 *
 * @warning It's possible to overwrite your program if the ELF that you're trying to
 * load overlaps your program's memory space.  Always ensure that the target ELF will
 * load before or after your program's code and data addresses.
 */
int SifLoadElf(const char *path, t_ExecData *data);
/** Load a MagicGate-encrypted ELF executable into EE RAM.
 * @ingroup loadfile
 *
 * @param path Path to an ELF executable
 * @param data Pointer to a variable that will store information about the loaded executable
 *
 * @returns 0 on success, or an error code if an error occurred.
 *
 * There is no real use for this function; it is only included in ps2lib for completeness.
 *
 * @see SifLoadElf
 */
int SifLoadElfEncrypted(const char *path, t_ExecData *data);

/** Write a value to an address within the IOP's memory space.
 * @ingroup loadfile
 *
 * @param iop_addr Address to modify in the IOP's address space
 * @param val      Data to write at iop_addr
 * @param type     A type describing the size of the val argument
 *
 * @returns Previous value at iop_addr  on success (?), or an error code if an
 * error occurred.
 *
 * Writes the data passed in val to the IOP address specified by iop_addr.  Use
 * the following types specify the size of data write:
 * - ::LF_VAL_BYTE to write an 8-bit value
 * - ::LF_VAL_SHORT to write a 16-bit value
 * - ::LF_VAL_LONG to write a 32-bit value
 *
 * @note Because iop_addr can be any address within the IOP's address space, it's
 * possible to modify IOP hardware registers.  However, in most cases the EE can
 * modify the IOP's hardware registers directly so there is little use for this function.
 *
 * @see SifIopGetVal
 */
int SifIopSetVal(u32 iop_addr, int val, int type);
/** Read a value from an address within the IOP's memory space.
 * @ingroup loadfile
 *
 * @param iop_addr Address within the IOP's address space to read from
 * @param val      Pointer to a variable that stores the value read
 * @param type     A type describing the size of the data to read
 *
 * @returns 0 on success, or an error code if an error occurred.
 *
 * Reads data of the size specified by type from the IOP address specified by
 * iop_addr.  Use the following types to specify the size of the data to read:
 * - ::LF_VAL_BYTE to write an 8-bit value
 * - ::LF_VAL_SHORT to write a 16-bit value
 * - ::LF_VAL_LONG to write a 32-bit value
 *
 * @note Because iop_addr can be any address within the IOP's address space, it's
 * possible to read from IOP hardware registers.  However, in most cases the EE can
 * read the IOP's hardware registers directly so there is little use for this function.
 * @see SifIopSetVal
 */
int SifIopGetVal(u32 iop_addr, void *val, int type);

/** Transfer an IRX module from EE RAM to IOP RAM and execute it.
 * @ingroup loadfile
 *
 * @param ptr     Pointer to a buffer in EE RAM where the IRX module will be loaded from
 * @param size    Size of the buffer in EE RAM that contains the IRX module
 * @param arg_len Length, in bytes, of the argument list
 * @param args    List of arguments to pass to the IRX on startup
 * @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function
 *
 * @returns The ID of the loaded module on success, or an error if the module couldn't
 * be loaded.
 *
 * Transfers an IRX module stored in a buffer in EE RAM to the IOP, and calls ::SifLoadModuleBuffer to
 * load and execute the module.  See ::SifLoadModule for details on the arg_len and args parameters.
 * If mod_res is non-NULL, the result code from the module's _start() function is stored here.
 *
 * @see SifLoadModule, SifLoadModuleBuffer
 */
int SifExecModuleBuffer(void *ptr, u32 size, u32 arg_len, const char *args, int *mod_res);
/** Read an IRX module from a file into IOP RAM and execute it.
 * @ingroup loadfile
 *
 * @param path    Path to an IRX module
 * @param arg_len Length, in bytes, of the argument list
 * @param args    List of arguments to pass to the IRX on startup
 * @param mod_res Pointer to a variable that will store the return value from the IRX's _start() function
 *
 * @returns The ID of the loaded module on success, or an error if the module
 * couldn't be loaded.
 *
 * Loads an IRX module from the specified path into a buffer on the IOP, then calls ::SifLoadModuleBuffer
 * to load and execute the module.  See ::SifLoadModule for details on the arg_len and args parameters.
 * This function is useful if you need to load an IRX module from a prohibited device, such as the memory card.
 *
 * @see SifLoadModule, SifLoadModuleBuffer
 */
int SifExecModuleFile(const char *path, u32 arg_len, const char *args, int *mod_res);


/** Low-level function for loading and executing an IRX module.
 * @ingroup loadfile
 *
 * @param path     Path to an IRX module
 * @param arg_len  Length, in bytes, of the argument list
 * @param args     List of arguments to pass to the IRX on startup
 * @param *modres  Pointer to a variable that will store the return value from the IRX's _start() function
 * @param fno      Function number of function to execute.
 * @param dontwait Whether to wait for the module's _start function to finish executing.
 *
 * @returns The ID of the loaded module on success, or an error if the module
 * couldn't be loaded.
 *
 * Unless required, use the higher-level functions instead.
 *
 * @see SifLoadModule, SifLoadModuleBuffer, SifLoadModuleEncrypted
 */
int _SifLoadModule(const char *path, int arg_len, const char *args,
                   int *modres, int fno, int dontwait);

#ifdef __cplusplus
}
#endif

#endif /* __LOADFILE_H__ */
