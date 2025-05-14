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
#include <kerr.h>
#include <modload.h>
#include <xmodload.h>

#ifdef _IOP
IRX_ID("Moldule_File_loader", 2, 9);
#endif
// Mostly based on the module from SCE SDK 3.1.0

extern struct irx_export_table _exp_modload;

static SecrCardBootFile_callback_t SecrCardBootFile_func_ptr;
static SecrDiskBootFile_callback_t SecrDiskBootFile_func_ptr;
static SetLoadfileCallbacks_callback_t SetLoadfileCallbacks_func_ptr;
static CheckKelfPath_callback_t CheckKelfPath_func_ptr;

static int modLoadCB;
static int ModuleLoaderMutex;
static int ModuleLoaderSync;

typedef struct module_thread_args_
{
	char command;
	char position;
	char access;
	// cppcheck-suppress unusedStructMember
	char unk03;
	int modid;
	int modid_2;
	const char *filename;
	void *buffer;
	int arglen;
	const char *args;
	void *distaddr;
	int distoffset;
	int *result;
	int *ret_ptr;
	int thread_ef;
	LDfilefunc *functable;
	void *funcopt;
} module_thread_args_t;

typedef struct modload_ll_
{
	struct modload_ll_ *next;
	struct modload_ll_ *prev;
} modload_ll_t;

typedef struct modload_load_memory_
{
	modload_ll_t ll[8];
} modload_load_memory_t;

static modload_ll_t load_memory_ll;

typedef struct
{
	u8 ident[16]; /* Structure of a ELF header */
	u16 type;
	u16 machine;
	u32 version;
	u32 entry;
	u32 phoff;
	u32 shoff;
	u32 flags;
	u16 ehsize;
	u16 phentsize;
	u16 phnum;
	u16 shentsize;
	u16 shnum;
	u16 shstrndx;
} elf_header_t;

typedef struct
{
	u32 type; /* Structure of a header a sections in an ELF */
	u32 offset;
	// cppcheck-suppress unusedStructMember
	void *vaddr;
	u32 paddr;
	u32 filesz;
	u32 memsz;
	u32 flags;
	u32 align;
} elf_pheader_t;

typedef struct
{
	u32 name;
	u32 type;
	u32 flags;
	u32 addr;
	u32 offset;
	u32 size;
	u32 link;
	u32 info;
	u32 addralign;
	u32 entsize;
} elf_shdr_t;

typedef struct
{
	u32 offset;
	u32 info;
} elf_rel;

typedef struct
{
	u32 offset;
	u32 info;
	u32 addend;
} elf_rela;

enum ELF_SHT_types
{
	SHT_NULL = 0,
	SHT_PROGBITS,
	SHT_SYMTAB,
	SHT_STRTAB,
	SHT_RELA,
	SHT_HASH,
	SHT_DYNAMIC,
	SHT_NOTE,
	SHT_NOBITS,
	SHT_REL,
	SHT_SHLIB,
	SHT_DYNSYM
};

enum ELF_reloc_types
{
	R_MIPS_NONE = 0,
	R_MIPS_16,
	R_MIPS_32,
	R_MIPS_REL32,
	R_MIPS_26,
	R_MIPS_HI16,
	R_MIPS_LO16,
	R_MIPSSCE_MHI16 = 250,
	R_MIPSSCE_ADDEND = 251,
};

struct modload_load_seek_header_
{
	elf_header_t elfhdr;
	elf_pheader_t phdr[2];
	u8 padding[28];
};

#define SHT_LOPROC 0x70000000
#define SHT_LOPROC_EE_IMPORT_TAB 0x90
#define SHT_LOPROC_IOPMOD 0x80
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000

typedef struct intrman_callbacks_
{
	// cppcheck-suppress unusedStructMember
	int (*cbCpuSuspendIntr)(int *state);
	// cppcheck-suppress unusedStructMember
	int (*cbCpuResumeIntr)(int state);
	// cppcheck-suppress unusedStructMember
	int (*cbQueryIntrContext)(void);
} intrman_callbacks_t;

static ModuleInfo_t *search_for_module_by_name(const char *name);
static void linked_list_add_after(modload_ll_t *ll1, modload_ll_t *ll2);
static void linked_list_remove(modload_ll_t *ll);
static int linked_list_next_is_self(const modload_ll_t *ll);
static void linked_list_set_self(modload_ll_t *ll);
static void get_updater_boot_argument(char *str, int *updater_argc, char **updater_argv, int updater_argv_count);
static int spGetfsize(void *userdata, int in_fd);
static int spLseek(void *userdata, int in_fd, long offset, int whence);
static int spRead(void *userdata, int in_fd, void *buffer, size_t read_size);
static int spBread(void *opt, int fd, size_t nbyte);
static int spSetBufSize(void *opt, int fd, size_t size);
static int spClose(void *userdata, int in_fd);
static int spAfterOpen(void *opt, int fd);
static int spBeforeOpen(void *opt, const char *filename, int flag);
static int spOpen(LDfilefunc *functbl, void *userdata, const char *filename, int filemode);
static void free_module_block(modload_ll_t *buf, char flags);
static int load_memory_helper_cmpinner(modload_ll_t *a1, int a2, modload_ll_t *a3, modload_ll_t *a4);
static ModuleInfo_t *
do_load_from_buffer(void *buffer, int position, int *distaddr, unsigned int distoffset, int *result_out);
static int seek_read(
	LDfilefunc *functbl, void *userdata, int module_fd, int read_offset, void *buf, int read_size, int *result_out);
static void *read_entire_loadfile(
	LDfilefunc *functbl, void *userdata, int module_fd, int *result_out, int module_filesize, int memory_region);
static int open_loadfile(LDfilefunc *functbl, void *userdata, const char *filename, int *out_filesize);
static ModuleInfo_t *do_load(module_thread_args_t *mltargs, int *result_out);
static int unload_module(ModuleInfo_t *module_info);
static int
stop_module(ModuleInfo_t *module_info, int command, int modid_2, int arglen, const char *args, int *result_out);
static int start_module(ModuleInfo_t *module_info, const char *data, int arglen, const char *args, int *result_out);
static int modload_post_boot_callback(iop_init_entry_t *next, int delayed);
static int check_in_linked_list(const modload_ll_t *ll);
static ModuleInfo_t *SearchModuleCBByID(int modid);
static int allocate_module_block(
	void *buf,
	FileInfo_t *fi,
	int memalloctype,
	modload_load_memory_t *loadmem,
	unsigned int distoffset,
	int *result_out);
static int ModuleLoaderThread(module_thread_args_t *mltargs);
static void ExecModuleLoad(module_thread_args_t *mltargs);

static intrman_callbacks_t intrman_callbacks = {&CpuSuspendIntr, &CpuResumeIntr, &QueryIntrContext};

static LDfilefunc default_filefunc_functable = {
	&spBeforeOpen, &spAfterOpen, &spClose, &spSetBufSize, &spBread, &spRead, &spLseek, &spGetfsize};

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
				AddRebootNotifyHandler((BootupCallback_t)modload_post_boot_callback, 1, 0);
			}
		}
	}
	{
		const ModuleInfo_t *sysmem_module;
		const ModuleInfo_t *loadcore_module;
		sysmem_module = search_for_module_by_name("System_Memory_Manager");
		if ( sysmem_module && sysmem_module->version >= 0x201u )
		{
			GetSysmemInternalData()->intr_suspend_tbl = (void *)&intrman_callbacks;
		}
		loadcore_module = search_for_module_by_name("Module_Manager");
		if ( loadcore_module )
		{
			if ( loadcore_module->version >= 0x205u )
			{
				GetLoadcoreInternalData()->intr_suspend_tbl = (void *)&intrman_callbacks;
			}
		}
	}
	thparam.attr = 0x2000000;
	thparam.thread = (void (*)(void *))ExecModuleLoad;
	thparam.priority = 8;
	thparam.stacksize = 4096;
	thparam.option = 0;
	modLoadCB = CreateThread(&thparam);
	semaparam.attr = 0;
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.option = 0;
	ModuleLoaderMutex = CreateSema(&semaparam);
	memset(&efparam, 0, sizeof(efparam));
	ModuleLoaderSync = CreateEventFlag(&efparam);
	linked_list_set_self(&load_memory_ll);
	return 0;
}

void *AllocLoadMemory(int type, unsigned int size, void *addr)
{
	modload_load_memory_t *newbuf;

	newbuf = (modload_load_memory_t *)AllocSysMemory(type, size, addr);
	if ( !newbuf )
		return NULL;
	memset(newbuf, -1, 32);
	linked_list_set_self(&newbuf->ll[2]);
	newbuf->ll[3].next = (struct modload_ll_ *)0x10;
	linked_list_add_after(&load_memory_ll, newbuf->ll);
	return newbuf;
}

int FreeLoadMemory(void *in_area)
{
	int ret;
	int state;
	modload_load_memory_t *area;

	area = in_area;
	CpuSuspendIntr(&state);
	if ( check_in_linked_list(area->ll) )
	{
		ret = KE_ERROR;
	}
	else if ( !linked_list_next_is_self(&area->ll[2]) )
	{
		ret = KE_MEMINUSE;
	}
	else
	{
		linked_list_remove(area->ll);
		ret = FreeSysMemory(area);
	}
	CpuResumeIntr(state);
	return ret;
}

int SetModuleFlags(int modid, int flag)
{
	ModuleInfo_t *image_info;
	int state;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	if ( (flag & 0xFFFFFFBF) != 0 )
	{
		return KE_ILLEGAL_FLAG;
	}
	CpuSuspendIntr(&state);
	image_info = GetLoadcoreInternalData()->image_info;
	while ( image_info && modid != image_info->id )
	{
		image_info = image_info->next;
	}
	if ( !image_info )
	{
		CpuResumeIntr(state);
		return KE_UNKNOWN_MODULE;
	}
	image_info->newflags |= flag;
	CpuResumeIntr(state);
	return KE_OK;
}

int LoadModuleWithOption(const char *filename, const LMWOoption *option)
{
	LDfilefunc *functable;
	module_thread_args_t mltargs;

	if ( (unsigned int)option->position >= 3 )
		return KE_ILLEGAL_POSITION;
	switch ( option->access )
	{
		case 1:
		case 2:
		case 4:
			break;
		default:
			return KE_ILLEGAL_ACCESS;
	}
	mltargs.command = 0;
	mltargs.position = option->position;
	mltargs.filename = filename;
	mltargs.buffer = 0;
	mltargs.access = option->access & 0xFF;
	mltargs.distaddr = option->distaddr;
	mltargs.distoffset = option->distoffset;
	functable = option->functable;
	if ( !functable )
		functable = &default_filefunc_functable;
	mltargs.functable = functable;
	mltargs.funcopt = option->funcopt;
	return ModuleLoaderThread(&mltargs);
}

int LoadModuleAddress(const char *name, void *addr, int offset)
{
	module_thread_args_t mltargs;

	mltargs.command = 0;
	if ( addr )
		mltargs.position = 2;
	else
		mltargs.position = 0;
	mltargs.access = 1;
	mltargs.filename = name;
	mltargs.buffer = 0;
	mltargs.distaddr = addr;
	mltargs.distoffset = offset;
	mltargs.functable = &default_filefunc_functable;
	mltargs.funcopt = 0;
	return ModuleLoaderThread(&mltargs);
}

int LoadModule(const char *name)
{
	return LoadModuleAddress(name, 0, 0);
}

int LoadModuleBufferAddress(void *buffer, void *addr, int offset)
{
	module_thread_args_t mltargs;

	mltargs.command = 2;
	if ( addr )
		mltargs.position = 2;
	else
		mltargs.position = 0;
	mltargs.buffer = buffer;
	mltargs.access = 0;
	mltargs.filename = 0;
	mltargs.distaddr = addr;
	mltargs.distoffset = offset;
	return ModuleLoaderThread(&mltargs);
}

int LoadModuleBuffer(void *buffer)
{
	return LoadModuleBufferAddress(buffer, 0, 0);
}

int LoadStartModule(const char *name, int arglen, const char *args, int *result)
{
	module_thread_args_t mltargs;

	mltargs.command = 3;
	mltargs.access = 1;
	mltargs.filename = name;
	mltargs.position = 0;
	mltargs.buffer = 0;
	mltargs.distaddr = 0;
	mltargs.distoffset = 0;
	mltargs.arglen = arglen;
	mltargs.args = args;
	mltargs.result = result;
	mltargs.functable = &default_filefunc_functable;
	mltargs.funcopt = 0;
	return ModuleLoaderThread(&mltargs);
}

int StartModule(int modid, const char *name, int arglen, const char *args, int *result)
{
	module_thread_args_t mltargs;

	mltargs.modid = modid;
	mltargs.command = 1;
	mltargs.filename = name;
	mltargs.arglen = arglen;
	mltargs.args = args;
	mltargs.result = result;
	return ModuleLoaderThread(&mltargs);
}

int StopModule(int modid, int arglen, const char *args, int *result)
{
	int modid_2;
	module_thread_args_t mltargs;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	modid_2 = SearchModuleByAddress(__builtin_return_address(0));
	if ( modid_2 < 0 )
	{
		Kprintf("StopModule(): panic !!! call from unknown Module !!!\n");
		return KE_CAN_NOT_STOP;
	}
	mltargs.command = 4;
	mltargs.modid = modid;
	mltargs.modid_2 = modid_2;
	mltargs.filename = 0;
	mltargs.arglen = arglen;
	mltargs.args = args;
	mltargs.result = result;
	return ModuleLoaderThread(&mltargs);
}

int UnloadModule(int modid)
{
	module_thread_args_t mltargs;

	mltargs.modid = modid;
	mltargs.command = 6;
	mltargs.filename = 0;
	mltargs.arglen = 0;
	mltargs.args = 0;
	mltargs.result = 0;
	return ModuleLoaderThread(&mltargs);
}

int SelfStopModule(int arglen, const char *args, int *result)
{
	int modid;
	module_thread_args_t mltargs;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	modid = SearchModuleByAddress(__builtin_return_address(0));
	if ( modid < 0 )
	{
		Kprintf("SelfStopModule(): panic !!! call from unknown Module !!!\n");
		return KE_ILLEGAL_CONTEXT;
	}
	mltargs.command = 5;
	mltargs.modid = modid;
	mltargs.modid_2 = modid;
	mltargs.filename = 0;
	mltargs.arglen = arglen;
	mltargs.args = args;
	mltargs.result = result;
	return ModuleLoaderThread(&mltargs);
}

void SelfUnloadModule(void)
{
	int ThreadId;
	int modid;
	int sema_res;
	module_thread_args_t mltargs;
	int ret_tmp;
	u32 efbits;

	if ( QueryIntrContext() != 0 )
	{
		Kprintf("SelfUnloadModule(): panic !!! illegal context !!!\n");
	}
	ThreadId = GetThreadId();
	if ( ThreadId < 0 )
	{
		Kprintf("SelfUnloadModule(): panic !!! can't get ThreadID !!!\n");
	}
	modid = SearchModuleByAddress(__builtin_return_address(0));
	if ( modid < 0 )
	{
		Kprintf("SelfUnloadModule(): panic !!! call from unknown Module !!!\n");
	}
	mltargs.command = 7;
	mltargs.modid = modid;
	mltargs.modid_2 = ThreadId;
	mltargs.filename = 0;
	mltargs.arglen = 0;
	mltargs.args = 0;
	mltargs.result = 0;
	mltargs.ret_ptr = &ret_tmp;
	if ( ThreadId == modLoadCB )
	{
		Kprintf("SelfUnloadModule(): panic !!! Unexpected case !!!\n");
	}
	mltargs.thread_ef = ModuleLoaderSync;
	sema_res = WaitSema(ModuleLoaderMutex);
	if ( sema_res < 0 )
	{
		Kprintf("SelfUnloadModule(): panic !!! Unload fail semerror=%d !!!\n", sema_res);
	}
	StartThread(modLoadCB, &mltargs);
	ChangeThreadPriority(0, 123);
	WaitEventFlag(ModuleLoaderSync, 1u, 17, &efbits);
	SignalSema(ModuleLoaderMutex);
	Kprintf("SelfUnloadModule(): panic !!! Unload fail error=%d !!!\n", ret_tmp);
	for ( ;; )
	{
		SleepThread();
		Kprintf("Thread 0x%x. Unload Fail\n", GetThreadId());
	}
}

int SearchModuleByName(const char *name)
{
	int modid;
	ModuleInfo_t *i;
	int state;

	CpuSuspendIntr(&state);
	modid = 0;
	for ( i = GetLoadcoreInternalData()->image_info; i; i = i->next )
	{
		if ( !strcmp(name, i->name) && modid < i->id )
			modid = i->id;
	}
	CpuResumeIntr(state);
	if ( !modid )
		return KE_UNKNOWN_MODULE;
	return modid;
}

int SearchModuleByAddress(const void *addr)
{
	const ModuleInfo_t *image_info;
	int state;

	CpuSuspendIntr(&state);
	image_info = SearchModuleCBByAddr((void *)addr);
	CpuResumeIntr(state);
	if ( !image_info )
		return KE_UNKNOWN_MODULE;
	return image_info->id;
}

int LoadStartKelfModule(const char *name, int arglen, const char *args, int *result)
{
	// At some point between SDK 1.6.0 (exclusive) and SDK 3.1.0 (inclusive), this function was stubbed.
	void *iop_exec_buffer;
	void *iop_exec_encrypted_buffer;
	LDfilefunc *functable;
	int module_filesize;
	int module_fd;
	int ret_tmp;
	int card_port;
	int card_slot;
	int state;

	functable = &default_filefunc_functable;
	iop_exec_buffer = NULL;
	if ( !name )
	{
		return KE_ERROR;
	}
	module_fd = open_loadfile(functable, NULL, name, &module_filesize);
	if ( module_fd < 0 )
	{
		return module_fd;
	}
	iop_exec_encrypted_buffer = read_entire_loadfile(functable, NULL, module_fd, &ret_tmp, module_filesize, 1);
	if ( iop_exec_encrypted_buffer == NULL )
	{
		return ret_tmp;
	}
	if ( CheckKelfPath_func_ptr && CheckKelfPath_func_ptr(name, &card_port, &card_slot) && SecrCardBootFile_func_ptr )
	{
		iop_exec_buffer = SecrCardBootFile_func_ptr(card_port, card_slot, iop_exec_encrypted_buffer);
	}
	else if ( SecrDiskBootFile_func_ptr )
	{
		iop_exec_buffer = SecrDiskBootFile_func_ptr(iop_exec_encrypted_buffer);
	}
	ret_tmp = KE_ILLEGAL_OBJECT;
	if ( iop_exec_buffer )
	{
		ret_tmp = LoadModuleBuffer(iop_exec_buffer);
	}
	if ( ret_tmp > 0 )
	{
		ret_tmp = StartModule(ret_tmp, name, arglen, args, result);
	}
	CpuSuspendIntr(&state);
	FreeSysMemory(iop_exec_encrypted_buffer);
	CpuResumeIntr(state);
	return ret_tmp;
}

int GetModuleIdListByName(const char *name, int *readbuf, int readbufsize, int *modulecount)
{
	int modcount;
	int readbufoffset;
	ModuleInfo_t *image_info;
	int state;

	modcount = 0;
	CpuSuspendIntr(&state);
	readbufoffset = 0;
	for ( image_info = GetLoadcoreInternalData()->image_info; image_info; image_info = image_info->next )
	{
		if ( name )
		{
			if ( strcmp(name, image_info->name) != 0 )
				continue;
		}
		if ( readbufoffset < readbufsize )
		{
			readbufoffset += 1;
			readbuf[readbufoffset] = image_info->id;
		}
		modcount += 1;
	}
	CpuResumeIntr(state);
	if ( modulecount )
		*modulecount = modcount;
	return readbufoffset;
}

int GetModuleIdList(int *readbuf, int readbufsize, int *modulecount)
{
	return GetModuleIdListByName(0, readbuf, readbufsize, modulecount);
}

int ReferModuleStatus(int modid, ModuleStatus *status)
{
	const ModuleInfo_t *image_info;
	const char *name;
	int state;

	CpuSuspendIntr(&state);
	image_info = GetLoadcoreInternalData()->image_info;
	while ( image_info && modid != image_info->id )
	{
		image_info = image_info->next;
	}
	if ( !image_info )
	{
		CpuResumeIntr(state);
		return KE_UNKNOWN_MODULE;
	}
	status->name[0] = 0;
	name = image_info->name;
	if ( name )
		strncpy(status->name, name, 56);
	status->name[55] = 0;
	status->version = image_info->version;
	status->id = image_info->id;
	status->flags = image_info->newflags;
	status->entry_addr = image_info->entry;
	status->gp_value = image_info->gp;
	status->text_addr = image_info->text_start;
	status->text_size = image_info->text_size;
	status->data_size = image_info->data_size;
	status->bss_size = image_info->bss_size;
	CpuResumeIntr(state);
	return KE_OK;
}

void GetModloadInternalData(void **pInternalData)
{
	// FIXME: create internal data structure
	*pInternalData = NULL;
}

static void ExecModuleLoad(module_thread_args_t *mltargs)
{
	ModuleInfo_t *mi;
	int modid_2;
	int res_tmp;

	mi = NULL;
	res_tmp = 0;
	switch ( mltargs->command )
	{
		case 0:
			mi = do_load(mltargs, &res_tmp);
			ChangeThreadPriority(0, 8);
			break;
		case 1:
			mi = SearchModuleCBByID(mltargs->modid);
			if ( !mi )
			{
				break;
			}
			res_tmp = start_module(mi, mltargs->filename, mltargs->arglen, mltargs->args, mltargs->result);
			break;
		case 2:
			mi = do_load_from_buffer(
				mltargs->buffer, mltargs->position, (int *)mltargs->distaddr, mltargs->distoffset, &res_tmp);
			break;
		case 3:
			mi = do_load(mltargs, &res_tmp);
			ChangeThreadPriority(0, 8);
			if ( !mi )
			{
				break;
			}
			res_tmp = start_module(mi, mltargs->filename, mltargs->arglen, mltargs->args, mltargs->result);
			break;
		case 4:
		case 5:
			mi = SearchModuleCBByID(mltargs->modid);
			if ( !mi )
			{
				break;
			}
			res_tmp = stop_module(mi, mltargs->command, mltargs->modid_2, mltargs->arglen, mltargs->args, mltargs->result);
			break;
		case 6:
		case 7:
			mi = SearchModuleCBByID(mltargs->modid);
			if ( !mi )
			{
				break;
			}
			res_tmp = unload_module(mi);
			if ( res_tmp )
			{
				break;
			}
			if ( mltargs->command != 7 )
			{
				break;
			}
			modid_2 = mltargs->modid_2;
			ChangeThreadPriority(0, 1);
			TerminateThread(modid_2);
			DeleteThread(modid_2);
			SignalSema(ModuleLoaderMutex);
			return;
		default:
			break;
	}
	if ( res_tmp )
	{
		*mltargs->ret_ptr = res_tmp;
	}
	else if ( !mi )
	{
		*mltargs->ret_ptr = KE_UNKNOWN_MODULE;
	}
	else
	{
		*mltargs->ret_ptr = mi->id;
	}
	if ( mltargs->thread_ef )
	{
		ChangeThreadPriority(0, 1);
		SetEventFlag(mltargs->thread_ef, 1u);
	}
	return;
}

static int ModuleLoaderThread(module_thread_args_t *mltargs)
{
	int result;
	int ret_tmp;
	u32 efbits;

	if ( QueryIntrContext() != 0 )
	{
		return KE_ILLEGAL_CONTEXT;
	}
	result = GetThreadId();
	if ( result < 0 )
	{
		return result;
	}
	ret_tmp = 0;
	// cppcheck-suppress autoVariables
	mltargs->ret_ptr = &ret_tmp;
	if ( result == modLoadCB )
	{
		mltargs->thread_ef = 0;
		ExecModuleLoad(mltargs);
	}
	else
	{
		mltargs->thread_ef = ModuleLoaderSync;
		result = WaitSema(ModuleLoaderMutex);
		if ( result < 0 )
		{
			return result;
		}
		StartThread(modLoadCB, mltargs);
		WaitEventFlag(ModuleLoaderSync, 1u, 17, &efbits);
		SignalSema(ModuleLoaderMutex);
	}
	return ret_tmp;
}

static ModuleInfo_t *SearchModuleCBByID(int modid)
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

static ModuleInfo_t *search_for_module_by_name(const char *name)
{
	ModuleInfo_t *image_info;

	image_info = GetLoadcoreInternalData()->image_info;
	while ( image_info != NULL )
	{
		if ( strcmp(name, image_info->name) == 0 )
		{
			return image_info;
		}
		image_info = image_info->next;
	}
	return NULL;
}

static int check_in_linked_list(const modload_ll_t *ll)
{
	modload_ll_t *next;

	next = load_memory_ll.next;
	if ( load_memory_ll.next == &load_memory_ll )
		return -1;
	while ( next != ll )
	{
		next = next->next;
		if ( next == &load_memory_ll )
			return -1;
	}
	return 0;
}

static int modload_post_boot_callback(iop_init_entry_t *next, int delayed)
{
	int reboot_type;
	int *BootMode_4;
	int *BootMode_5;
	int updater_argc;
	int module_result;

	(void)next;
	(void)delayed;

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
			module_thread_args_t mltargs;

			mltargs.position = 2;
			mltargs.access = 1;
			mltargs.distaddr = (void *)0x100000;
			mltargs.distoffset = 0;
			mltargs.functable = &default_filefunc_functable;
			mltargs.funcopt = 0;
			mltargs.filename = updater_argv[0];
			module_info = do_load(&mltargs, &module_result);
		}
		else if ( reboot_type == 1 )
		{
			// At some point between SDK 1.6.0 (exclusive) and SDK 3.1.0 (inclusive), this block was stubbed.
			void *iop_exec_buffer;
			void *iop_exec_encrypted_buffer;
			LDfilefunc *functable;
			int module_filesize;
			int module_fd;
			int card_port;
			int card_slot;
			int state;

			functable = &default_filefunc_functable;
			iop_exec_buffer = NULL;
			iop_exec_encrypted_buffer = NULL;
			module_fd = open_loadfile(functable, NULL, updater_argv[0], &module_filesize);
			if ( module_fd >= 0 )
			{
				iop_exec_encrypted_buffer =
					read_entire_loadfile(functable, NULL, module_fd, &module_result, module_filesize, 1);
				if ( iop_exec_encrypted_buffer )
				{
					if (
						CheckKelfPath_func_ptr && CheckKelfPath_func_ptr(updater_argv[0], &card_port, &card_slot)
						&& SecrCardBootFile_func_ptr )
					{
						iop_exec_buffer = SecrCardBootFile_func_ptr(card_port, card_slot, iop_exec_encrypted_buffer);
					}
					else if ( SecrDiskBootFile_func_ptr )
					{
						iop_exec_buffer = SecrDiskBootFile_func_ptr(iop_exec_encrypted_buffer);
					}
					if ( iop_exec_buffer )
					{
						module_info = do_load_from_buffer(iop_exec_buffer, 2, (void *)0x100000, 0, &module_result);
					}
					else
					{
						module_result = -1;
					}
					CpuSuspendIntr(&state);
					FreeSysMemory(iop_exec_encrypted_buffer);
					CpuResumeIntr(state);
				}
				else
				{
					module_result = -1;
				}
			}
			else
			{
				module_result = -1;
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
			__builtin_trap();
		}
		printf("updater '%s' can't load\n", updater_argv[0]);
		__builtin_trap();
	}
	printf("Reboot fail! need file name argument\n");
	return 0;
}

static int start_module(ModuleInfo_t *module_info, const char *data, int arglen, const char *args, int *result_out)
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

	if ( (module_info->newflags & 0xF) != 1 )
	{
		return KE_ALREADY_STARTED;
	}
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
	module_info->newflags &= 0xFFF0;
	module_info->newflags |= 2;
	module_result =
		((int (*)(int, char **, u32, ModuleInfo_t *))module_info->entry)(in_argc, in_argv_ptrs, 0, module_info);
	ChangeThreadPriority(0, 8);
	if ( result_out )
	{
		*result_out = module_result;
	}
	switch ( module_result & 3 )
	{
		case 0:
		{
			module_info->newflags |= 3;
			break;
		}
		case 1:
		{
			int state;

			CpuSuspendIntr(&state);
			UnLinkLibraryEntries((void *)module_info->text_start, module_info->text_size);
			if ( (module_info->newflags & 0x40) != 0 )
			{
				memset((void *)module_info->text_start, 77, module_info->text_size);
				memset(
					(void *)(module_info->text_start + module_info->text_size),
					-1,
					module_info->data_size + module_info->bss_size);
			}
			ReleaseModule(module_info);
			free_module_block((modload_ll_t *)module_info, module_info->newflags);
			CpuResumeIntr(state);
			break;
		}
		case 2:
		{
			module_info->newflags |= 0x13;
			break;
		}
	}
	return KE_OK;
}

static int
stop_module(ModuleInfo_t *module_info, int command, int modid_2, int arglen, const char *args, int *result_out)
{
	const char *data;
	const char *args_ptr;
	char *in_argv_strs_ptr;
	int in_argc;
	int module_result;
	int data_strlen;
	int in_argv_size_strs;
	char *in_argv_strs;
	char **in_argv_ptrs;
	int i;

	switch ( module_info->newflags & 0xF )
	{
		case 1:
		case 2:
			return KE_NOT_STARTED;
		case 4:
		case 5:
			return KE_ALREADY_STOPPING;
		case 6:
		case 7:
			return KE_ALREADY_STOPPED;
		default:
			break;
	}
	if ( (module_info->newflags & 0x10) == 0 )
		return KE_NOT_REMOVABLE;
	if ( command == 4 && modid_2 == module_info->id )
		return KE_CAN_NOT_STOP;
	data = "self";
	if ( command == 4 )
		data = "other";
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

	module_info->newflags &= 0xFFF0u;
	module_info->newflags |= 4;
	if ( command != 4 )
		module_info->newflags |= 1;
	// TODO: save/restore gp register
	module_result =
		((int (*)(int, char **, u32, ModuleInfo_t *))module_info->entry)(-in_argc, in_argv_ptrs, 0, module_info);
	ChangeThreadPriority(0, 8);
	if ( result_out )
		*result_out = module_result;
	module_info->newflags &= 0xFFF0;
	switch ( module_result & 3 )
	{
		case 0:
		{
			module_info->newflags |= 3;
			return KE_CAN_NOT_STOP;
		}
		case 1:
		{
			module_info->newflags |= 6;
			if ( command != 4 )
				module_info->newflags |= 1;
			break;
		}
		case 2:
		{
			module_info->newflags |= 0x13u;
			return KE_CAN_NOT_STOP;
		}
	}
	return KE_OK;
}

static int unload_module(ModuleInfo_t *module_info)
{
	u16 newflags;
	int flags_masked1;
	int state;

	newflags = module_info->newflags;
	flags_masked1 = newflags & 0xF;
	if ( (newflags & 0x10) == 0 && flags_masked1 != 1 )
	{
		return KE_NOT_REMOVABLE;
	}
	switch ( flags_masked1 )
	{
		case 1:
		case 6:
		case 7:
			break;
		default:
			return KE_NOT_STOPPED;
	}
	CpuSuspendIntr(&state);
	UnLinkLibraryEntries((void *)module_info->text_start, module_info->text_size);
	if ( (module_info->newflags & 0x40) != 0 )
	{
		memset((void *)module_info->text_start, 77, module_info->text_size);
		memset(
			(void *)(module_info->text_start + module_info->text_size), -1, module_info->data_size + module_info->bss_size);
	}
	ReleaseModule(module_info);
	free_module_block((modload_ll_t *)module_info, module_info->newflags);
	CpuResumeIntr(state);
	return KE_OK;
}

int IsIllegalBootDevice(const char *arg1)
{
	(void)arg1;

	// Unofficial: Always succeed
	return 0;
}

static ModuleInfo_t *do_load_noseek(
	module_thread_args_t *mltargs, int module_fd, int module_filesize, FileInfo_t *fi, int *module_block, int *result_out)
{
	void *entire_loadfile;
	int state;

	entire_loadfile = read_entire_loadfile(
		mltargs->functable, mltargs->funcopt, module_fd, result_out, module_filesize, mltargs->position != 1);
	if ( !entire_loadfile )
	{
		return NULL;
	}
	CpuSuspendIntr(&state);
	*module_block = allocate_module_block(
		entire_loadfile,
		fi,
		mltargs->position,
		(modload_load_memory_t *)mltargs->distaddr,
		mltargs->distoffset,
		result_out);
	CpuResumeIntr(state);
	if ( !(*module_block) )
	{
		FreeSysMemory(entire_loadfile);
		return NULL;
	}
	LoadExecutableObject(entire_loadfile, fi);
	CpuSuspendIntr(&state);
	FreeSysMemory(entire_loadfile);
	CpuResumeIntr(state);
	return (ModuleInfo_t *)((char *)fi->text_start - 0x30);
}

static ModuleInfo_t *
do_load_seek(module_thread_args_t *mltargs, int module_fd, FileInfo_t *fi, int *module_block_ptr, int *result_out)
{
	LDfilefunc *functable;
	void *funcopt;
	char *text_start;
	u32 data_size;
	char *text_end;
	char *relocate_offset;
	int i;
	u32 type;
	void *modulearea;
	u32 entsize;
	int shdrcnt;
	int j;
	int relnumi1;
	struct modload_load_seek_header_ lshdr;
	elf_rel elfrelhdr[2];
	int state;
	void *ptr;
	u32 *dest;
	elf_shdr_t *elfshdr;
	ModuleInfo_t *ModInfo;
	int res_tmp;
	int module_block;

	res_tmp = KE_OK;
	functable = mltargs->functable;
	funcopt = mltargs->funcopt;
	ptr = NULL;
	dest = NULL;
	ModInfo = NULL;
	if ( seek_read(functable, funcopt, module_fd, 0, &lshdr, 144, result_out) != 0 )
		return NULL;
	CpuSuspendIntr(&state);
	module_block = allocate_module_block(
		&lshdr, fi, mltargs->position, (modload_load_memory_t *)mltargs->distaddr, mltargs->distoffset, &res_tmp);
	CpuResumeIntr(state);
	for ( ;; )
	{
		int total_section_size;
		IopModuleID_t *mod_id;
		unsigned int size;

		if ( !module_block )
		{
			break;
		}
		if ( fi->ModuleType == 1 )
		{
			res_tmp = KE_ILLEGAL_OBJECT;
			break;
		}
		text_start = (char *)fi->text_start;
		data_size = fi->data_size;
		text_end = &text_start[fi->text_size];
		ModInfo = (ModuleInfo_t *)(text_start - 0x30);
		dest = (u32 *)&text_end[data_size];
		total_section_size = lshdr.elfhdr.shentsize * lshdr.elfhdr.shnum;
		if ( CheckThreadStack() < total_section_size + 768 )
		{
			res_tmp = KE_NO_MEMORY;
			break;
		}
		elfshdr = __builtin_alloca(total_section_size);
		if ( seek_read(
					 functable, funcopt, module_fd, lshdr.phdr[1].offset, fi->text_start, lshdr.phdr[1].filesz, &res_tmp) )
		{
			break;
		}
		if ( seek_read(functable, funcopt, module_fd, lshdr.elfhdr.shoff, elfshdr, total_section_size, &res_tmp) )
		{
			break;
		}
		if ( fi->ModuleType != 4 )
		{
			break;
		}
		relocate_offset = (char *)fi->text_start;
		fi->EntryPoint = &relocate_offset[(u32)(u8 *)fi->EntryPoint];
		fi->gp = &relocate_offset[(u32)(u8 *)fi->gp];
		mod_id = fi->mod_id;
		if ( mod_id != (IopModuleID_t *)-1 )
			fi->mod_id = (IopModuleID_t *)&relocate_offset[(u32)(u8 *)mod_id];
		size = 0;
		for ( i = 1; i < lshdr.elfhdr.shnum; i += 1 )
		{
			type = elfshdr[i].type;
			if ( (type == 9 || type == 4) && size < elfshdr[i].size )
				size = elfshdr[i].size;
		}
		switch ( mltargs->access )
		{
			case 2:
			{
				if ( fi->bss_size >= size )
				{
					modulearea = dest;
				}
				else
				{
					modulearea = AllocSysMemory(1, size, 0);
					ptr = modulearea;
					if ( !modulearea )
					{
						res_tmp = KE_NO_MEMORY;
						break;
					}
				}
				for ( i = 1; i < lshdr.elfhdr.shnum; i += 1 )
				{
					if ( elfshdr[i].type != 9 )
					{
						continue;
					}
					if ( seek_read(functable, funcopt, module_fd, elfshdr[i].offset, modulearea, elfshdr[i].size, &res_tmp) )
						break;
					entsize = elfshdr[i].entsize;
					if ( !entsize )
						__builtin_trap();
					ApplyElfRelSection(fi->text_start, modulearea, elfshdr[i].size / entsize);
				}
				break;
			}
			case 4:
			{
				res_tmp = functable->setBufSize(funcopt, module_fd, size);
				if ( res_tmp )
				{
					break;
				}
				for ( i = 1; i < lshdr.elfhdr.shnum; i += 1 )
				{
					if ( elfshdr[i].type != 9 )
					{
						continue;
					}
					if ( seek_read(functable, funcopt, module_fd, elfshdr[i].offset, 0, 0, &res_tmp) )
						break;
					res_tmp = functable->beforeRead(funcopt, module_fd, elfshdr[i].size);
					if ( res_tmp )
						break;
					shdrcnt = elfshdr[i].size / elfshdr[i].entsize;
					for ( j = 0; j < shdrcnt; j += relnumi1 )
					{
						relnumi1 = 1;
						if ( (elfrelhdr[0].info & 0xFF) == 5 || (elfrelhdr[0].info & 0xFF) == 250 )
						{
							relnumi1 += 1;
						}
						{
							int k;
							for ( k = 0; k < relnumi1; k += 1 )
							{
								if ( seek_read(functable, funcopt, module_fd, -1, &elfrelhdr[k], 8, &res_tmp) )
									break;
							}
							if ( res_tmp )
								break;
						}
						ApplyElfRelSection(fi->text_start, elfrelhdr, relnumi1);
					}
				}
				break;
			}
			default:
			{
				break;
			}
		}
		break;
	}
	functable->close(funcopt, module_fd);
	if ( res_tmp )
	{
		*result_out = res_tmp;
		if ( ptr || (module_block && ModInfo) )
		{
			CpuSuspendIntr(&state);
			if ( ptr )
				FreeSysMemory(ptr);
			if ( module_block && ModInfo )
			{
				free_module_block((modload_ll_t *)ModInfo, module_block);
			}
			CpuResumeIntr(state);
		}
		ModInfo = NULL;
	}
	else
	{
		*module_block_ptr = module_block;
		if ( ptr )
		{
			CpuSuspendIntr(&state);
			FreeSysMemory(ptr);
			CpuResumeIntr(state);
		}
		if ( dest )
		{
			memset(dest, 0, fi->bss_size);
		}
		if ( ModInfo )
		{
			CopyModInfo(fi, ModInfo);
		}
	}
	return ModInfo;
}

static ModuleInfo_t *do_load(module_thread_args_t *mltargs, int *result_out)
{
	int module_fd;
	ModuleInfo_t *module_info;
	FileInfo_t fi;
	int module_filesize;
	int module_block;
	int state;

	*result_out = KE_ILLEGAL_OBJECT;
	if ( !mltargs->filename )
	{
		return NULL;
	}
	// cppcheck-suppress knownConditionTrueFalse
	if ( IsIllegalBootDevice(mltargs->filename) != 0 )
	{
		return NULL;
	}
	module_fd = open_loadfile(mltargs->functable, mltargs->funcopt, mltargs->filename, &module_filesize);
	if ( module_fd < 0 )
	{
		*result_out = module_fd;
		return NULL;
	}
	*result_out = KE_OK;
	module_info = 0;
	switch ( mltargs->access )
	{
		case 1:
		{
			module_info = do_load_noseek(mltargs, module_fd, module_filesize, &fi, &module_block, result_out);
			break;
		}
		case 2:
		case 4:
		{
			module_info = do_load_seek(mltargs, module_fd, &fi, &module_block, result_out);
			break;
		}
		default:
		{
			break;
		}
	}
	if ( !module_info )
	{
		return NULL;
	}
	CpuSuspendIntr(&state);
	if ( LinkLibraryEntries(fi.text_start, fi.text_size) < 0 )
	{
		free_module_block((modload_ll_t *)module_info, module_block);
		module_info = NULL;
		*result_out = KE_LINKERR;
	}
	else
	{
		FlushIcache();
		module_info->newflags = module_block;
		RegisterModule(module_info);
	}
	CpuResumeIntr(state);
	return module_info;
}

static int open_loadfile(LDfilefunc *functbl, void *userdata, const char *filename, int *out_filesize)
{
	int out_fd;
	int filesize;

	out_fd = spOpen(functbl, userdata, filename, 1);
	if ( out_fd < 0 )
		return KE_NOFILE;
	filesize = functbl->getfsize(userdata, out_fd);
	*out_filesize = filesize;
	if ( filesize <= 0 )
	{
		close(out_fd);
		return KE_FILEERR;
	}
	return out_fd;
}

static void *read_entire_loadfile(
	LDfilefunc *functbl, void *userdata, int module_fd, int *result_out, int module_filesize, int memory_region)
{
	void *tmp_mem;
	int state;

	*result_out = KE_OK;
	CpuSuspendIntr(&state);
	tmp_mem = AllocSysMemory(memory_region, module_filesize, 0);
	CpuResumeIntr(state);
	if ( !tmp_mem )
	{
		*result_out = KE_NO_MEMORY;
		functbl->close(userdata, module_fd);
		return NULL;
	}
	if ( seek_read(functbl, userdata, module_fd, 0, tmp_mem, module_filesize, result_out) == 0 )
	{
		CpuSuspendIntr(&state);
		FreeSysMemory(tmp_mem);
		CpuResumeIntr(state);
		return NULL;
	}
	functbl->close(userdata, module_fd);
	return tmp_mem;
}

static int seek_read(
	LDfilefunc *functbl, void *userdata, int module_fd, int read_offset, void *buf, int read_size, int *result_out)
{
	if ( !((read_offset < 0 || (functbl->lseek(userdata, module_fd, read_offset, 0) >= 0))
				 && (read_size <= 0 || (functbl->read(userdata, module_fd, buf, read_size) == read_size))) )
	{
		functbl->close(userdata, module_fd);
		*result_out = KE_FILEERR;
		return -1;
	}
	return 0;
}

static ModuleInfo_t *
do_load_from_buffer(void *buffer, int position, int *distaddr, unsigned int distoffset, int *result_out)
{
	int module_block;
	ModuleInfo_t *module_info;
	FileInfo_t fi;
	int state;

	module_info = NULL;
	CpuSuspendIntr(&state);
	module_block =
		allocate_module_block(buffer, &fi, position, (modload_load_memory_t *)distaddr, distoffset, result_out);
	if ( module_block )
	{
		LoadExecutableObject(buffer, &fi);
		if ( LinkLibraryEntries(fi.text_start, fi.text_size) < 0 )
		{
			free_module_block((modload_ll_t *)fi.text_start - 6, module_block & 0xFFFF);
			*result_out = KE_LINKERR;
		}
		else
		{
			module_info = (ModuleInfo_t *)((char *)fi.text_start - 0x30);
			FlushIcache();
			module_info->newflags = module_block & 0xFFFF;
			RegisterModule(module_info);
		}
	}
	CpuResumeIntr(state);
	return module_info;
}

static int load_memory_helper_cmpinner(modload_ll_t *a1, int a2, modload_ll_t *a3, modload_ll_t *a4)
{
	const char *v4;
	const char *v6;

	if ( a1 >= a3 )
	{
		if ( a1 < (modload_ll_t *)((char *)a4 + (int)a3) )
			return 1;
	}
	v4 = (char *)a1 + a2;
	v6 = v4 - 1;
	if ( v6 < (char *)a3 )
		return 0;
	return v6 < (char *)a4 + (int)a3;
}

static modload_ll_t *
load_memory_helper(const FileInfo_t *fi, modload_load_memory_t *loadmem, unsigned int distoffset, int *result_out)
{
	int memsz_add;
	modload_ll_t *next;
	modload_ll_t *prev;
	modload_ll_t *v8;
	modload_ll_t *i;

	memsz_add = fi->MemSize + 64;
	if ( distoffset == 1 )
	{
		next = &loadmem->ll[2];
		prev = loadmem->ll[2].prev;
		v8 = (modload_ll_t *)((char *)prev + (unsigned int)prev[1].next);
	}
	else
	{
		const modload_ll_t *v10;

		if ( (distoffset < 0x20) || ((distoffset & 0xF) != 0) )
		{
			*result_out = KE_ILLEGAL_OFFSET;
			return NULL;
		}
		next = loadmem->ll[2].next;
		v8 = (modload_ll_t *)((char *)loadmem->ll + distoffset);
		v10 = &loadmem->ll[2];
		for ( i = next; i != v10; i = i->next )
		{
			if ( v8 < next )
				break;
			next = i->next;
		}
		prev = next->prev;
	}
	if (
		(load_memory_helper_cmpinner(v8, memsz_add, prev, prev[1].next) != 0)
		|| (load_memory_helper_cmpinner(v8, memsz_add, next, next[1].next) != 0) )
	{
		*result_out = KE_MEMINUSE;
		return NULL;
	}
	linked_list_add_after(next, v8);
	v8[1].next = (struct modload_ll_ *)memsz_add;
	return v8 + 2;
}

static int allocate_module_block(
	void *buf, FileInfo_t *fi, int memalloctype, modload_load_memory_t *loadmem, unsigned int distoffset, int *result_out)
{
	switch ( ProbeExecutableObject(buf, fi) )
	{
		case 1:
		case 3:
		{
			char *text_start_tmp;
			char *allocaddr1;

			text_start_tmp = (char *)fi->text_start;
			allocaddr1 = (char *)(((unsigned int)(text_start_tmp - 0x30) >> 8 << 8) & 0x1FFFFFFF);
			if ( AllocSysMemory(2, &text_start_tmp[fi->MemSize] - allocaddr1, allocaddr1) != 0 )
				return 1;
			if ( (int)QueryBlockTopAddress(allocaddr1) <= 0 )
				*result_out = KE_NO_MEMORY;
			else
				*result_out = KE_MEMINUSE;
			return 0;
		}
		case 4:
		{
			if ( memalloctype == 2 && distoffset )
			{
				fi->text_start = 0;
				if ( check_in_linked_list(loadmem->ll) == 0 )
				{
					fi->text_start = load_memory_helper(fi, loadmem, distoffset, result_out);
					if ( !fi->text_start )
						return 0;
					fi->text_start = ((char *)fi->text_start) + 0x30;
					return 33;
				}
			}
			else
			{
				modload_load_memory_t *allocaddr2;

				if ( memalloctype == 2 )
				{
					allocaddr2 = loadmem;
				}
				else
				{
					allocaddr2 = 0;
				}
				fi->text_start = AllocSysMemory(memalloctype, fi->MemSize + 0x30, allocaddr2);
			}
			if ( !fi->text_start )
			{
				*result_out = KE_NO_MEMORY;
				return 0;
			}
			fi->text_start = ((char *)fi->text_start) + 0x30;
			return 1;
		}
		default:
		{
			*result_out = KE_ILLEGAL_OBJECT;
			return 0;
		}
	}
}

static void free_module_block(modload_ll_t *buf, char flags)
{
	if ( (flags & 0x20) != 0 )
		linked_list_remove(buf - 2);
	else
		FreeSysMemory((void *)((unsigned int)buf >> 8 << 8));
}

static int spOpen(LDfilefunc *functbl, void *userdata, const char *filename, int filemode)
{
	int ret_fd;

	functbl->beforeOpen(userdata, filename, filemode);
	ret_fd = open(filename, filemode);
	functbl->afterOpen(userdata, ret_fd);
	return ret_fd;
}

static int spBeforeOpen(void *opt, const char *filename, int flag)
{
	(void)opt;
	(void)filename;
	(void)flag;
	return 0;
}

static int spAfterOpen(void *opt, int fd)
{
	(void)opt;
	(void)fd;
	return 0;
}

static int spClose(void *userdata, int in_fd)
{
	(void)userdata;
	(void)in_fd;
	return close(in_fd);
}

static int spSetBufSize(void *opt, int fd, size_t size)
{
	(void)opt;
	(void)fd;
	(void)size;
	return 0;
}

static int spBread(void *opt, int fd, size_t nbyte)
{
	(void)opt;
	(void)fd;
	(void)nbyte;
	return 0;
}

static int spRead(void *userdata, int in_fd, void *buffer, size_t read_size)
{
	(void)userdata;
	(void)buffer;
	(void)read_size;
	return read(in_fd, buffer, read_size);
}

static int spLseek(void *userdata, int in_fd, long offset, int whence)
{
	(void)userdata;
	(void)offset;
	(void)whence;
	return lseek(in_fd, offset, whence);
}

static int spGetfsize(void *userdata, int in_fd)
{
	int ret;

	(void)userdata;
	ret = lseek(in_fd, 0, 2);
	if ( ret < 0 )
		return ret;
	lseek(in_fd, 0, 0);
	return ret;
}

static char *get_next_non_whitespace_string(char *str)
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

static char *get_non_null_string(char *str)
{
	while ( *str && *str != ' ' && *str != '\t' && *str != '\n' )
	{
		str += 1;
	}
	return str;
}

static void get_updater_boot_argument(char *str, int *updater_argc, char **updater_argv, int updater_argv_count)
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

static void ml_strcpy(char *dst, const char *src)
{
	while ( *src )
	{
		*dst++ = *src++;
	}
	*dst = 0;
}

static void TerminateResidentLibraries(const char *message, unsigned int options, int mode)
{
	const lc_internals_t *LoadcoreData;
	iop_library_t *ModuleData, *NextModule;
	void **ExportTable;
	unsigned int enable_debug;

	enable_debug = options & 0x80000000;
	if ( enable_debug != 0 )
		Kprintf(message);

	if ( (LoadcoreData = GetLoadcoreInternalData()) != NULL )
	{
		ModuleData = LoadcoreData->let_next;
		while ( ModuleData != NULL )
		{
			NextModule = ModuleData->prev;

			if ( mode == 2 )
			{
				if ( !(ModuleData->flags & 6) )
				{
					ModuleData = NextModule;
					continue;
				}
			}
			else if ( (ModuleData->flags & 6) == 2 )
			{  // Won't ever happen?
				ModuleData = NextModule;
				continue;
			}

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

static void TerminateResidentEntriesDI(const char *command, unsigned int options)
{
	int prid;
	volatile unsigned int **pReg;

	TerminateResidentLibraries(" ReBootStart:di: Terminate resident Libraries\n", options, 0);

	asm volatile("mfc0 %0, $15" : "=r"(prid) :);

	if ( !(options & 1) )
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
	if ( !(options & 2) )
	{
		SetCacheCtrl(
			(prid < 0x10 || ((*(volatile unsigned int *)0xbf801450) & 8)) ? *(volatile unsigned int *)0xbfc02010 :
																																			*(volatile unsigned int *)0xbfc02014);
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
			flagstmp = (options & 0xFF00) | 2;
			command_ptr = (char *)0x480;
		}
		else
		{
			ram_size_in_mb = (QueryMemSize() + 0x100) >> 20;
			flagstmp = 1;
			command_ptr = 0;
		}
		((int (*)(u32, int, char *, u32))iopboot_entrypoint)(ram_size_in_mb, flagstmp, command_ptr, 0);
	}
}

// Exactly the same function as INTRMAN's export 14.
extern int CpuExecuteKmode(void *func, ...);

// clang-format off
__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" "CpuExecuteKmode:" "\n"
	"\t" "  addiu       $v0, $zero, 0x0C" "\n"
	"\t" "  syscall     0" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on

int ReBootStart(const char *command, unsigned int flags)
{
	ChangeThreadPriority(0, 7);
	TerminateResidentLibraries(" ReBootStart:ei: Terminate resident Libraries\n", flags, 2);
	return CpuExecuteKmode(TerminateResidentEntriesDI, command, flags);
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

static void linked_list_set_self(modload_ll_t *ll)
{
	ll->next = ll;
	ll->prev = ll;
}

static int linked_list_next_is_self(const modload_ll_t *ll)
{
	return ll->next == ll;
}

static void linked_list_remove(modload_ll_t *ll)
{
	ll->next->prev = ll->prev;
	ll->prev->next = ll->next;
}

#if 0
static int linked_list_is_circular(const modload_ll_t *ll)
{
	return ll->prev == ll->next;
}
#endif

static void linked_list_add_after(modload_ll_t *ll1, modload_ll_t *ll2)
{
	ll2->next = ll1;
	ll2->prev = ll1->prev;
	ll1->prev = ll2;
	ll2->prev->next = ll2;
}
