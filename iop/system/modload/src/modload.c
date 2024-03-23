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
#include <kerr.h>
#include <modload.h>

#ifdef _IOP
IRX_ID("Moldule_File_loader", 1, 1);
#endif
// Mostly based on the module from SCE SDK 1.3.4

extern struct irx_export_table _exp_modload;

SecrCardBootFile_callback_t SecrCardBootFile_func_ptr;
SecrDiskBootFile_callback_t SecrDiskBootFile_func_ptr;
SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_func_ptr;
CheckKelfPath_callback_t CheckKelfPath_func_ptr;

int modLoadCB;
int ModuleLoaderMutex;
int ModuleLoaderSync;

typedef struct module_thread_args_
{
	int command;
	int modid;
	void *data;
	void *buffer;
	int arglen;
	const char *args;
	void *addr;
	int offset;
	int *result;
	int *ret_ptr;
	s32 thread_ef;
} module_thread_args_t;

extern int modload_post_boot_callback(iop_init_entry_t *next, int delayed);
extern void get_updater_boot_argument(char *str, int *updater_argc, char **updater_argv, int updater_argv_count);
extern int ModuleLoaderThread(module_thread_args_t *mltargs);
extern void *do_load_seek(const char *filename, int *result_out);
extern ModuleInfo_t *do_load(const char *filename, void *buffer, void *addr, int offset, int *result_out);
extern ModuleInfo_t *SearchModuleCBByID(int modid);
extern int start_module(ModuleInfo_t *module_info, const char *data, int arglen, const char *args, int *result_out);
extern ModuleInfo_t *allocate_link_module_info(void *buffer, void *addr, int offset, int *result_out);

int _start(int argc, char *argv[])
{
	iop_thread_t thparam;
	iop_sema_t semaparam;
	iop_event_t efparam;

	(void)argc;
	(void)argv;

	if ( RegisterLibraryEntries(&_exp_modload) < 0 )
	{
		return 1;
	}
	{
		int *BootMode_4;

		BootMode_4 = QueryBootMode(4);
		if ( BootMode_4 )
		{
			if ( (((u32 *)BootMode_4)[0] & 0xff) == 2 )
			{
				RegisterPostBootCallback((BootupCallback_t)modload_post_boot_callback, 1, 0);
			}
		}
	}
	thparam.attr = 0x2000000;
	thparam.thread = (void (*)(void *))ModuleLoaderThread;
	thparam.priority = 8;
	thparam.stacksize = 2048;
	thparam.option = 0;
	modLoadCB = CreateThread(&thparam);
	semaparam.attr = 0;
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.option = 0;
	ModuleLoaderMutex = CreateSema(&semaparam);
	memset(&efparam, 0, sizeof(efparam));
	ModuleLoaderSync = CreateEventFlag(&efparam);
	return 0;
}

int ModuleThreadCmd(int command, void *data, void *buffer, void *addr, int offset)
{
	int retval;
	module_thread_args_t mltargs;
	int ret_tmp;
	u32 efbits;

	retval = GetThreadId();
	if ( retval < 0 )
	{
		return retval;
	}
	mltargs.command = command;
	mltargs.data = data;
	mltargs.buffer = buffer;
	mltargs.addr = addr;
	mltargs.offset = offset;
	mltargs.ret_ptr = &ret_tmp;
	if ( retval == modLoadCB )
	{
		mltargs.thread_ef = 0;
		ModuleLoaderThread(&mltargs);
	}
	else
	{
		mltargs.thread_ef = ModuleLoaderSync;
		retval = WaitSema(ModuleLoaderMutex);
		if ( retval < 0 )
		{
			return retval;
		}
		StartThread(modLoadCB, &mltargs);
		WaitEventFlag(ModuleLoaderSync, 1u, 17, &efbits);
		SignalSema(ModuleLoaderMutex);
	}
	return ret_tmp;
}

int LoadModuleAddress(const char *name, void *addr, int offset)
{
	return ModuleThreadCmd(1, (void *)name, 0, addr, offset);
}

int LoadModule(const char *name)
{
	return LoadModuleAddress(name, 0, 0);
}

int LoadModuleBufferAddress(void *buffer, void *addr, int offset)
{
	return ModuleThreadCmd(4, 0, buffer, addr, offset);
}

int LoadModuleBuffer(void *buffer)
{
	return LoadModuleBufferAddress(buffer, 0, 0);
}

int LoadStartModule(const char *name, int arglen, const char *args, int *result)
{
	int retval;
	module_thread_args_t mltargs;
	int ret_tmp;
	u32 efbits;

	retval = GetThreadId();
	if ( retval < 0 )
	{
		return retval;
	}
	mltargs.data = (void *)name;
	mltargs.command = 3;
	mltargs.buffer = 0;
	mltargs.addr = 0;
	mltargs.offset = 0;
	mltargs.arglen = arglen;
	mltargs.args = args;
	mltargs.result = result;
	mltargs.ret_ptr = &ret_tmp;
	if ( retval == modLoadCB )
	{
		mltargs.thread_ef = 0;
		ModuleLoaderThread(&mltargs);
	}
	else
	{
		mltargs.thread_ef = ModuleLoaderSync;
		retval = WaitSema(ModuleLoaderMutex);
		if ( retval < 0 )
		{
			return retval;
		}
		StartThread(modLoadCB, &mltargs);
		WaitEventFlag(ModuleLoaderSync, 1u, 17, &efbits);
		SignalSema(ModuleLoaderMutex);
	}
	return ret_tmp;
}

int StartModule(int modid, const char *name, int arglen, const char *args, int *result)
{
	int retval;
	module_thread_args_t mltargs;
	int ret_tmp;
	u32 efbits;

	retval = GetThreadId();
	if ( retval < 0 )
	{
		return retval;
	}
	mltargs.command = 2;
	mltargs.modid = modid;
	mltargs.data = (void *)name;
	mltargs.arglen = arglen;
	mltargs.args = args;
	mltargs.result = result;
	mltargs.ret_ptr = &ret_tmp;
	if ( retval == modLoadCB )
	{
		mltargs.thread_ef = 0;
		ModuleLoaderThread(&mltargs);
	}
	else
	{
		mltargs.thread_ef = ModuleLoaderSync;
		retval = WaitSema(ModuleLoaderMutex);
		if ( retval < 0 )
		{
			return retval;
		}
		StartThread(modLoadCB, &mltargs);
		WaitEventFlag(ModuleLoaderSync, 1u, 17, &efbits);
		SignalSema(ModuleLoaderMutex);
	}
	return ret_tmp;
}

int LoadStartKelfModule(const char *name, int arglen, const char *args, int *result)
{
	void *iop_exec_buffer;
	void *iop_exec_encrypted_buffer;
	int ModuleBuffer;
	int started;
	int card_port;
	int card_slot;
	int state;

	iop_exec_buffer = 0;
	iop_exec_encrypted_buffer = do_load_seek(name, &started);
	if ( iop_exec_encrypted_buffer == 0 )
	{
		return started;
	}
	if ( CheckKelfPath_func_ptr && CheckKelfPath_func_ptr(name, &card_port, &card_slot) )
	{
		if ( SecrCardBootFile_func_ptr )
		{
			iop_exec_buffer = SecrCardBootFile_func_ptr(card_port, card_slot, iop_exec_encrypted_buffer);
		}
	}
	else if ( SecrDiskBootFile_func_ptr )
	{
		iop_exec_buffer = SecrDiskBootFile_func_ptr(iop_exec_encrypted_buffer);
	}
	if ( !iop_exec_buffer )
	{
		CpuSuspendIntr(&state);
		FreeSysMemory(iop_exec_encrypted_buffer);
		CpuResumeIntr(state);
		return KE_ILLEGAL_OBJECT;
	}
	ModuleBuffer = LoadModuleBuffer(iop_exec_buffer);
	started = ModuleBuffer;
	if ( ModuleBuffer > 0 )
	{
		started = StartModule(ModuleBuffer, name, arglen, args, result);
	}
	CpuSuspendIntr(&state);
	FreeSysMemory(iop_exec_encrypted_buffer);
	CpuResumeIntr(state);
	return started;
}

int ModuleLoaderThread(module_thread_args_t *mltargs)
{
	ModuleInfo_t *mi;
	int res_tmp;

	switch ( mltargs->command )
	{
		case 1:
		{
			mi = do_load((char *)mltargs->data, 0, mltargs->addr, mltargs->offset, &res_tmp);
			if ( res_tmp )
			{
				*mltargs->ret_ptr = res_tmp;
			}
			else
			{
				*mltargs->ret_ptr = mi->id;
			}
			break;
		}
		case 2:
		{
			mi = SearchModuleCBByID(mltargs->modid);
			if ( !mi )
			{
				*mltargs->ret_ptr = KE_UNKNOWN_MODULE;
			}
			else
			{
				*mltargs->ret_ptr = mi->id;
				start_module(mi, (const char *)mltargs->data, mltargs->arglen, mltargs->args, mltargs->result);
			}
			break;
		}
		case 3:
		{
			mi = do_load((char *)mltargs->data, 0, mltargs->addr, mltargs->offset, &res_tmp);
			if ( res_tmp )
			{
				*mltargs->ret_ptr = res_tmp;
			}
			else
			{
				*mltargs->ret_ptr = mi->id;
				start_module(mi, (const char *)mltargs->data, mltargs->arglen, mltargs->args, mltargs->result);
			}
			break;
		}
		case 4:
		{
			mi = do_load(0, mltargs->buffer, mltargs->addr, mltargs->offset, &res_tmp);
			if ( res_tmp )
			{
				*mltargs->ret_ptr = res_tmp;
			}
			else
			{
				*mltargs->ret_ptr = mi->id;
			}
			break;
		}
		default:
		{
			break;
		}
	}
	if ( mltargs->thread_ef )
	{
		ChangeThreadPriority(0, 1);
		SetEventFlag(mltargs->thread_ef, 1u);
	}
	return 0;
}

ModuleInfo_t *SearchModuleCBByID(int modid)
{
	ModuleInfo_t *image_info;

	image_info = GetLoadcoreInternalData()->image_info;
	while ( image_info != NULL )
	{
		if ( modid == image_info->id )
		{
			return image_info;
		}
		image_info = image_info->next;
	}
	return NULL;
}

int modload_post_boot_callback(iop_init_entry_t *next, int delayed)
{
	void *iop_exec_buffer;
	int reboot_type;
	int *BootMode_4;
	int *BootMode_5;
	int updater_argc;
	int module_result;
	int card_port;
	int card_slot;
	int state;

	(void)next;
	(void)delayed;

	iop_exec_buffer = 0;
	reboot_type = 0;
	BootMode_4 = QueryBootMode(4);
	if ( BootMode_4 )
	{
		// See reboot_start_proc for when this variable gets set
		reboot_type = (((u32 *)BootMode_4)[0] >> 8) & 0xff;
	}
	BootMode_5 = QueryBootMode(5);
	if ( BootMode_5 )
	{
		ModuleInfo_t *module_info;
		char *updater_argv[16];

		memset(updater_argv, 0, sizeof(updater_argv));
		module_info = 0;
		get_updater_boot_argument(
			(char *)BootMode_5[1], &updater_argc, updater_argv, (sizeof(updater_argv) / sizeof(updater_argv[0])) - 1);
		if ( reboot_type == 0 )
		{
			module_info = do_load(updater_argv[0], 0, (void *)0x100000, 0, &module_result);
		}
		else if ( reboot_type == 1 )
		{
			void *iop_exec_encrypted_buffer;

			iop_exec_encrypted_buffer = do_load_seek(updater_argv[0], &module_result);
			if ( iop_exec_encrypted_buffer )
			{
				if ( CheckKelfPath_func_ptr && CheckKelfPath_func_ptr(updater_argv[0], &card_port, &card_slot) )
				{
					if ( SecrCardBootFile_func_ptr )
					{
						iop_exec_buffer = SecrCardBootFile_func_ptr(card_port, card_slot, iop_exec_encrypted_buffer);
					}
				}
				else if ( SecrDiskBootFile_func_ptr )
				{
					iop_exec_buffer = SecrDiskBootFile_func_ptr(iop_exec_encrypted_buffer);
				}
				if ( iop_exec_buffer )
				{
					module_info = allocate_link_module_info(iop_exec_buffer, (void *)0x100000, 0, &module_result);
				}
				else
				{
					module_result = -1;
				}
				CpuSuspendIntr(&state);
				FreeSysMemory(iop_exec_encrypted_buffer);
				CpuResumeIntr(state);
			}
		}
		else
		{
			module_result = -1;
		}
		if ( module_result == 0 )
		{
			module_result =
				((int (*)(int, char **, u32, ModuleInfo_t *))module_info->entry)(updater_argc, updater_argv, 0, module_info);
			printf("return from updater '%s' return value = %d\n", updater_argv[0], module_result);
			__asm("break\n");
		}
		printf("updater '%s' can't load\n", updater_argv[0]);
		__asm("break\n");
	}
	printf("Reboot fail! need file name argument\n");
	return 0;
}

int start_module(ModuleInfo_t *module_info, const char *data, int arglen, const char *args, int *result_out)
{
	const char *args_ptr;
	char *in_argv_strs_ptr;
	int in_argc;
	int module_result;
	int data_strlen;
	int in_argv_size_strs;
	char *in_argv_strs;
	char **in_argv_ptrs;
	int i;

	in_argc = 1;
	data_strlen = strlen(data) + 1;
	in_argv_size_strs = data_strlen;
	for ( args_ptr = args; args_ptr < &args[arglen]; )
	{
		int str_len = strlen(args_ptr) + 1;
		in_argv_size_strs += str_len;
		args_ptr += str_len;
		in_argc += 1;
	}
	if ( in_argv_size_strs < data_strlen + arglen )
	{
		in_argv_size_strs = data_strlen + arglen;
	}
	in_argv_strs = __builtin_alloca(in_argv_size_strs);
	memcpy(in_argv_strs, data, data_strlen);
	memcpy(in_argv_strs + data_strlen, args, arglen);
	in_argv_ptrs = __builtin_alloca((in_argc + 1) * sizeof(char *));
	for ( i = 0, in_argv_strs_ptr = in_argv_strs; i < in_argc && in_argv_strs_ptr < &in_argv_strs[in_argv_size_strs]; )
	{
		int str_len = strlen(in_argv_strs_ptr) + 1;
		in_argv_ptrs[i] = in_argv_strs_ptr;
		i += 1;
		in_argv_strs_ptr += str_len;
	}
	in_argv_ptrs[in_argc] = NULL;
	module_result =
		((int (*)(int, char **, u32, ModuleInfo_t *))module_info->entry)(in_argc, in_argv_ptrs, 0, module_info);
	ChangeThreadPriority(0, 8);
	if ( result_out )
	{
		*result_out = module_result;
	}
	if ( (module_result & 3) != 0 )
	{
		int state;

		CpuSuspendIntr(&state);
		UnlinkImports((void *)module_info->text_start, module_info->text_size);
		UnlinkModule(module_info);
		FreeSysMemory((void *)((unsigned int)module_info >> 8 << 8));
		CpuResumeIntr(state);
		return 0;
	}
	return 0;
}

int IsIllegalBootDevice(const char *arg1)
{
	(void)arg1;

	// Unofficial: Always succeed
	return 0;
}

ModuleInfo_t *do_load(const char *filename, void *buffer, void *addr, int offset, int *result_out)
{
	void *seek;
	void *iop_exec_buffer;
	ModuleInfo_t *ExecutableObject;
	int state;

	*result_out = 0;
	// cppcheck-suppress knownConditionTrueFalse
	if ( IsIllegalBootDevice(filename) )
	{
		*result_out = KE_ILLEGAL_OBJECT;
		return 0;
	}
	seek = 0;
	if ( filename )
	{
		seek = do_load_seek(filename, result_out);
		iop_exec_buffer = seek;
	}
	else
	{
		iop_exec_buffer = buffer;
	}
	if ( iop_exec_buffer == 0 )
	{
		return 0;
	}
	ExecutableObject = allocate_link_module_info(iop_exec_buffer, addr, offset, result_out);
	if ( seek )
	{
		CpuSuspendIntr(&state);
		FreeSysMemory(seek);
		CpuResumeIntr(state);
	}
	return ExecutableObject;
}

void *do_load_seek(const char *filename, int *result_out)
{
	int fd;
	int file_size;
	void *buffer;
	int state;

	*result_out = 0;
	fd = open(filename, 1);
	if ( fd < 0 )
	{
		*result_out = KE_NOFILE;
		return 0;
	}
	file_size = lseek(fd, 0, 2);
	if ( file_size <= 0 )
	{
		*result_out = KE_FILEERR;
		close(fd);
		return 0;
	}
	CpuSuspendIntr(&state);
	buffer = AllocSysMemory(1, file_size, 0);
	CpuResumeIntr(state);
	if ( !buffer )
	{
		*result_out = KE_NO_MEMORY;
		close(fd);
		return 0;
	}
	lseek(fd, 0, 0);
	if ( read(fd, buffer, file_size) != file_size )
	{
		*result_out = KE_FILEERR;
		close(fd);
		CpuSuspendIntr(&state);
		FreeSysMemory(buffer);
		CpuResumeIntr(state);
		return 0;
	}
	close(fd);
	return buffer;
}

ModuleInfo_t *allocate_link_module_info(void *buffer, void *addr, int offset, int *result_out)
{
	ModuleInfo_t *mi;
	int executable_type;
	char *allocaddr1;
	FileInfo_t fi;
	int state;

	CpuSuspendIntr(&state);
	executable_type = ReadModuleHeader(buffer, &fi);
	if ( executable_type == 2 || executable_type == 4 )
	{
		int position;

		// Relocatable IRX.
		position = 0;
		if ( addr )
		{
			position = 2;
		}
		if ( addr && offset )
		{
			if ( QueryBlockTopAddress(addr) == addr && (unsigned int)QueryBlockSize(addr) >= (unsigned int)(offset + 48) )
			{
				fi.text_start = addr;
			}
			else
			{
				fi.text_start = 0;
			}
		}
		else
		{
			fi.text_start = AllocSysMemory(position, fi.MemSize + 0x30, addr);
		}
		if ( !fi.text_start )
		{
			*result_out = KE_NO_MEMORY;
			CpuResumeIntr(state);
			return NULL;
		}
		fi.text_start = (char *)fi.text_start + 0x30;
	}
	else if ( executable_type == 1 || executable_type == 3 )
	{
		// Fixed COFF or ELF.
		allocaddr1 = (char *)((((unsigned int)fi.text_start - 48) >> 8 << 8) & 0x1FFFFFFF);
		if ( AllocSysMemory(2, (char *)fi.text_start + fi.MemSize - allocaddr1, allocaddr1) == NULL )
		{
			if ( (int)QueryBlockTopAddress(allocaddr1) > 0 )
			{
				*result_out = KE_MEMINUSE;
				CpuResumeIntr(state);
				return NULL;
			}
			*result_out = KE_NO_MEMORY;
			CpuResumeIntr(state);
			return NULL;
		}
	}
	else
	{
		*result_out = KE_ILLEGAL_OBJECT;
		CpuResumeIntr(state);
		return NULL;
	}
	LoadModuleImage(buffer, &fi);
	if ( LinkImports(fi.text_start, fi.text_size) < 0 )
	{
		FreeSysMemory((void *)(((unsigned int)fi.text_start - 48) >> 8 << 8));
		*result_out = KE_LINKERR;
		CpuResumeIntr(state);
		return NULL;
	}
	FlushIcache();
	mi = (ModuleInfo_t *)((char *)fi.text_start - 48);
	LinkModule((ModuleInfo_t *)fi.text_start - 1);
	CpuResumeIntr(state);
	return mi;
}

char *get_next_non_whitespace_string(char *str)
{
	for ( ;; )
	{
		if ( *str != ' ' && *str != '\t' && *str != '\n' )
		{
			return str;
		}
		*str = 0;
		str += 1;
	}
}

char *get_non_null_string(char *str)
{
	while ( *str && *str != ' ' && *str != '\t' && *str != '\n' )
	{
		str += 1;
	}
	return str;
}

void get_updater_boot_argument(char *str, int *updater_argc, char **updater_argv, int updater_argv_count)
{
	char *next_non_whitespace_string;
	int updater_argc_cur;

	next_non_whitespace_string = get_next_non_whitespace_string(str);
	updater_argc_cur = 0;
	while ( *next_non_whitespace_string && updater_argc_cur < updater_argv_count )
	{
		char *non_null_string;

		*updater_argv = next_non_whitespace_string;
		updater_argv += 1;
		non_null_string = get_non_null_string(next_non_whitespace_string);
		updater_argc_cur += 1;
		if ( !*non_null_string )
		{
			break;
		}
		*non_null_string = 0;
		next_non_whitespace_string = get_next_non_whitespace_string(non_null_string + 1);
	}
	*updater_argc = updater_argc_cur;
}

void SetSecrmanCallbacks(
	SecrCardBootFile_callback_t SecrCardBootFile_fnc,
	SecrDiskBootFile_callback_t SecrDiskBootFile_fnc,
	SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_fnc)
{
	SecrCardBootFile_func_ptr = SecrCardBootFile_fnc;
	SecrDiskBootFile_func_ptr = SecrDiskBootFile_fnc;
	SetLoadfileCallbacks_func_ptr = SetLoadfileCallbacks_fnc;
}

void SetCheckKelfPathCallback(CheckKelfPath_callback_t CheckKelfPath_fnc)
{
	CheckKelfPath_func_ptr = CheckKelfPath_fnc;
}

void GetLoadfileCallbacks(
	CheckKelfPath_callback_t *CheckKelfPath_fnc, SetLoadfileCallbacks_callback_t *SetLoadfileCallbacks_fnc)
{
	*CheckKelfPath_fnc = CheckKelfPath_func_ptr;
	*SetLoadfileCallbacks_fnc = SetLoadfileCallbacks_func_ptr;
}

// Pulled from UDNL
struct ssbus_regs
{
	volatile unsigned int *address, *delay;
};

// Pulled from UDNL
static struct ssbus_regs ssbus_regs[] = {
	{(volatile unsigned int *)0xbf801000, (volatile unsigned int *)0xbf801008},
	{(volatile unsigned int *)0xbf801400, (volatile unsigned int *)0xbf80100C},
	{(volatile unsigned int *)0xbf801404, (volatile unsigned int *)0xbf801014},
	{(volatile unsigned int *)0xbf801408, (volatile unsigned int *)0xbf801018},
	{(volatile unsigned int *)0xbf80140C, (volatile unsigned int *)0xbf801414},
	{(volatile unsigned int *)0xbf801410, (volatile unsigned int *)0xbf80141C},
	{NULL, NULL}};

// Pulled from UDNL
// cppcheck-suppress constParameterPointer
static volatile unsigned int *func_00000f80(volatile unsigned int *address)
{
	struct ssbus_regs *pSSBUS_regs;

	pSSBUS_regs = ssbus_regs;
	while ( pSSBUS_regs->address != NULL )
	{
		if ( pSSBUS_regs->address == address )
			break;
		pSSBUS_regs++;
	}

	return pSSBUS_regs->delay;
}

static const void *GetFileDataFromImage(const void *start, const void *end, const char *filename);

static void ml_strcpy(char *dst, const char *src)
{
	while ( *src )
	{
		*dst++ = *src++;
	}
	*dst = 0;
}

static int reboot_start_proc(const char *command, unsigned int flags)
{
	// Pulled from UDNL
	{
		const lc_internals_t *LoadcoreData;
		iop_library_t *ModuleData, *NextModule;
		void **ExportTable;
		unsigned int enable_debug;

		enable_debug = flags & 0x80000000;
		if ( enable_debug != 0 )
			Kprintf(" ReBootStart: Terminate resident Libraries\n");

		if ( (LoadcoreData = GetLoadcoreInternalData()) != NULL )
		{
			ModuleData = LoadcoreData->let_next;
			while ( ModuleData != NULL )
			{
				NextModule = ModuleData->prev;

				ExportTable = ModuleData->exports;
				if ( ExportTable[1] != NULL && ExportTable[2] != NULL )
				{
					int (*pexit)(int arg1);

					pexit = ExportTable[2];
					if ( enable_debug != 0 )
						Kprintf("  %.8s %x \n", ModuleData->name, pexit);
					pexit(0);
				}

				ModuleData = NextModule;
			}
		}
	}

	// Pulled from UDNL
	{
		int prid;
		volatile unsigned int **pReg;

		asm volatile("mfc0 %0, $15" : "=r"(prid) :);

		if ( !(flags & 1) )
		{
			pReg = (prid < 0x10 || ((*(volatile unsigned int *)0xbf801450) & 8)) ? *(volatile unsigned int ***)0xbfc02008 :
																																						 *(volatile unsigned int ***)0xbfc0200C;

			while ( pReg[0] != 0 )
			{
				if ( func_00000f80(pReg[0]) != 0 )
					pReg[0] = (void *)0xFF;
				pReg[0] = pReg[1];
				pReg += 2;
			}
		}
		if ( !(flags & 2) )
		{
			SetCacheCtrl(
				(prid < 0x10 || ((*(volatile unsigned int *)0xbf801450) & 8)) ? *(volatile unsigned int *)0xbfc02010 :
																																				*(volatile unsigned int *)0xbfc02014);
		}
	}

	// MODLOAD specific
	{
		const char *iopboot_entrypoint;
		u32 ram_size_in_mb;
		int flagstmp;
		char *command_ptr;

		iopboot_entrypoint = GetFileDataFromImage((const void *)0xBFC00000, (const void *)0xBFC10000, "IOPBOOT");
		// Unofficial: Check if command is NULL befire checking its contents
		if ( command && command[0] )
		{
			ml_strcpy((char *)0x480, command);
			ram_size_in_mb = (QueryMemSize() + 0x100) >> 20;
			flagstmp = (flags & 0xFF00) | 2;
			command_ptr = (char *)0x480;
		}
		else
		{
			ram_size_in_mb = (QueryMemSize() + 0x100) >> 20;
			flagstmp = 1;
			command_ptr = 0;
		}
		return ((int (*)(u32, int, char *, u32))iopboot_entrypoint)(ram_size_in_mb, flagstmp, command_ptr, 0);
	}
}

// Exactly the same function as INTRMAN's export 14.
// Defined in modload_asm.S
extern int CpuExecuteKmode(void *func, ...);

int ReBootStart(const char *command, unsigned int flags)
{
	return CpuExecuteKmode(reboot_start_proc, command, flags);
}

struct RomDirEntry
{
	char name[10];
	unsigned short int ExtInfoEntrySize;
	unsigned int size;
};

// Similar to the function in UDNL and ROMDRV.
static const void *GetFileDataFromImage(const void *start, const void *end, const char *filename)
{
	const u8 *ImageStart;
	const u8 *RomdirStart;

	ImageStart = NULL;
	RomdirStart = NULL;
	{
		const u32 *ptr;
		unsigned int offset;
		const struct RomDirEntry *file;

		offset = 0;
		file = (struct RomDirEntry *)start;
		for ( ; file < (const struct RomDirEntry *)end; file++, offset += sizeof(struct RomDirEntry) )
		{
			/* Check for a valid ROM filesystem (Magic: "RESET\0\0\0\0\0"). Must have the right magic and bootstrap code size
			 * (size of RESET = bootstrap code size). */
			ptr = (u32 *)file->name;
			if ( ptr[0] == 0x45534552 && ptr[1] == 0x54 && (*(u16 *)&ptr[2] == 0) && (((file->size + 15) & ~15) == offset) )
			{
				ImageStart = start;
				RomdirStart = (const u8 *)ptr;
				break;
			}
		}
	}
	{
		unsigned int i, offset;
		u8 filename_temp[12];

		offset = 0;
		((u32 *)filename_temp)[0] = 0;
		((u32 *)filename_temp)[1] = 0;
		((u32 *)filename_temp)[2] = 0;
		for ( i = 0; *filename >= 0x21 && i < sizeof(filename_temp); i++ )
		{
			filename_temp[i] = *filename;
			filename++;
		}

		if ( RomdirStart != NULL )
		{
			const struct RomDirEntry *RomdirEntry;

			RomdirEntry = (const struct RomDirEntry *)RomdirStart;

			do
			{  // Fast comparison of filenames.
				if (
					((u32 *)filename_temp)[0] == ((u32 *)RomdirEntry->name)[0]
					&& ((u32 *)filename_temp)[1] == ((u32 *)RomdirEntry->name)[1]
					&& (*(u16 *)&((u32 *)filename_temp)[2] == *(u16 *)&((u32 *)RomdirEntry->name)[2]) )
				{
					return ImageStart + offset;
				}

				offset += (RomdirEntry->size + 15) & ~15;
				RomdirEntry++;
			} while ( ((u32 *)RomdirEntry->name)[0] != 0x00000000 );  // Until the terminator entry is reached.
		}
	}
	return NULL;
}
