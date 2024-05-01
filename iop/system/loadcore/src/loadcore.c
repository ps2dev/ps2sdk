/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "loadcore.h"
#include "irx_imports.h"
#include "kerr.h"
#include "xloadcore.h"

extern struct irx_export_table _exp_loadcore;

#ifdef _IOP
IRX_ID("Module_Manager", 2, 6);
#endif
// Based on the module from SCE SDK 3.1.0.

static lc_internals_t loadcore_internals;
static u32 *reboot_handlers;

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

struct iopmod
{
	IopModuleID_t *mod_id;
	void *EntryPoint;
	void *gp;
	unsigned int text_size;
	unsigned int data_size;
	unsigned int bss_size;
	unsigned short int version;
	// cppcheck-suppress unusedStructMember
	char modname[];
};

struct coff_filehdr
{
	u16 f_magic;  /* magic number */
	u16 f_nscns;  /* number of sections */
	u32 f_timdat; /* time & date stamp */
	u32 f_symptr; /* file pointer to symbolic header */
	u32 f_nsyms;  /* sizeof(symbolic hdr) */
	u16 f_opthdr; /* sizeof(optional hdr) */
	u16 f_flags;  /* flags */
};

#define MIPSELMAGIC 0x0162

#define OMAGIC 0407
#define SOMAGIC 0x0701

typedef struct aouthdr
{
	u16 magic;      /* see above */
	u16 vstamp;     /* version stamp */
	u32 tsize;      /* text size in bytes, padded to DW bdry */
	u32 dsize;      /* initialized data "  " */
	u32 bsize;      /* uninitialized data "   " */
	u32 entry;      /* entry pt. */
	u32 text_start; /* base of text used for this file */
	u32 data_start; /* base of data used for this file */
	u32 bss_start;  /* base of bss used for this file */
	// Instead of the GPR and CPR masks, these 5 fields exist.
	u32 field_20;
	u32 field_24;
	u32 field_28;
	u32 field_2C;
	IopModuleID_t *mod_id;
	u32 gp_value; /* the gp value used for this object */
} AOUTHDR;

struct scnhdr
{
	u8 s_name[8];  /* section name */
	u32 s_paddr;   /* physical address, aliased s_nlib */
	u32 s_vaddr;   /* virtual address */
	u32 s_size;    /* section size */
	u32 s_scnptr;  /* file ptr to raw data for section */
	u32 s_relptr;  /* file ptr to relocation */
	u32 s_lnnoptr; /* file ptr to gp histogram */
	u16 s_nreloc;  /* number of relocation entries */
	u16 s_nlnno;   /* number of gp histogram entries */
	u32 s_flags;   /* flags */
};

enum IOP_MODULE_TYPES
{
	IOP_MOD_TYPE_COFF = 1,
	IOP_MOD_TYPE_2,
	IOP_MOD_TYPE_ELF,
	IOP_MOD_TYPE_IRX
};

enum E_type_name_enum
{
	ET_NONE = 0,
	ET_REL = 1,
	ET_EXEC = 2,
	ET_DYN = 3,
	ET_CORE = 4,
	ET_SCE_IOPRELEXEC = 0xFF80,
	ET_SCE_IOPRELEXEC2 = 0xFF81,
	ET_SCE_EERELEXEC = 0xFF90,
	ET_SCE_EERELEXEC2 = 0xFF91,
};

typedef struct intrman_callbacks_
{
	int (*cbCpuSuspendIntr)(int *state);
	int (*cbCpuResumeIntr)(int state);
	// cppcheck-suppress unusedStructMember
	int (*cbQueryIntrContext)(void);
} intrman_callbacks_t;

static int IsSameLibrary(const struct irx_export_table *src, const struct irx_export_table *dst);
static int compLibMinVersion_major(const struct irx_export_table *src, const struct irx_export_table *dst);
static int compLibMinVersion_minor(const struct irx_export_table *src, const struct irx_export_table *dst);
static int IsLibraryCompliant(const struct irx_export_table *src, const struct irx_export_table *dst);
static int compLibMinVersion_major_1(const struct irx_export_table *src, const struct irx_export_table *dst);
static void cleanStub(const struct irx_import_table *imp);
static int CheckCallerStub(const struct irx_import_table *imp);
static int aLinkLibEntries(struct irx_import_table *imp);
static void aLinkClient(struct irx_import_table *imp, const struct irx_export_table *exp);
static int aUnLinkLibEntries(struct irx_import_table *a1, const struct irx_import_table *a2);
static void lc_memset32(int *b, int c, int len);
static int lc_strlen(const char *s);
static void lc_memmove(char *dst, const char *src, int len);
static int cCpuSuspendIntr(int *state);
static int cCpuResumeIntr(int state);
static void loadcoff(const void *module);
static void loadelf(const void *module);
static void loadrelelf(const void *module, FileInfo_t *ModuleInfo);
static void CopySection(const void *module, void *buffer, unsigned int FileSize);
static void ZeroSection(unsigned int *buffer, unsigned int NumWords);

typedef struct ResetData
{
	unsigned int MemSize;
	unsigned int BootMode;
	const char *command;
	void *StartAddress;
	void *IOPRPBuffer;
	unsigned int IOPRPBufferSize;
	unsigned int NumModules;
	const void **ModData;
} boot_params;

static u32 bootmodes[17];

static u32 **const bootmodes_start_ptr = (void *)0x3F0;
static u32 **const bootmodes_end_ptr = (void *)0x3F4;

// The following are defined in the linker script
extern void *_ftext;
extern void *_etext;
extern void *_end;

// clang-format off
__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global _start" "\n"
	"\t" "_start:" "\n"
	"\t" "  mtc0        $zero, $12" "\n"
	"\t" "  lw          $v0, 0x0($a0)" "\n" // boot_params->MemSize
	"\t" "  nop" "\n"
	"\t" "  sll         $sp, $v0, 20" "\n" // sp = boot_params->MemSize << 20
	"\t" "  addiu       $sp, $sp, -0x40" "\n"
	"\t" "  addu        $fp, $sp, $zero" "\n"
	"\t" "  lui         $gp, %hi(_gp)" "\n"
	"\t" "  j           loadcore_init" "\n"
	"\t" "   addiu      $gp, $gp, %lo(_gp)" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on

void loadcore_init(boot_params *in_params)
{
	const void **ModData;
	ModuleInfo_t *sysmemmi;
	u32 MemSize;
	unsigned int i;
	char *blocksize_mask_1;
	int BlockSize;
	int blocksize_mask_2;
	void *curmodaddr_align;
	elf_header_t **cur_module_addr;
	int executable_type;
	int memalloctype;
	void *memallocaddr;
	int entrypoint_ret;
	ModuleInfo_t *mi;
	int *frame_pointer_curfunc;
	FileInfo_t fi;
	boot_params params;
	u32 *stack_reboot_handlers;

	// cppcheck-suppress unreadVariable
	params.MemSize = in_params->MemSize;
	params.BootMode = in_params->BootMode;
	params.command = in_params->command;
	params.StartAddress = in_params->StartAddress;
	params.IOPRPBuffer = in_params->IOPRPBuffer;
	params.IOPRPBufferSize = in_params->IOPRPBufferSize;
	params.NumModules = in_params->NumModules;
	ModData = in_params->ModData;
	loadcore_internals.let_next = (iop_library_t *)params.StartAddress;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	*bootmodes_start_ptr = bootmodes;
	*bootmodes_end_ptr = bootmodes;
#pragma GCC diagnostic pop
	loadcore_internals.intr_suspend_tbl = NULL;
	loadcore_internals.let_prev = (iop_library_t *)params.StartAddress;
	params.ModData = ModData;
	loadcore_internals.let_prev->prev = NULL;
	loadcore_internals.module_count = 2;
	loadcore_internals.mda_next = 0;
	loadcore_internals.mda_prev = 0;
	loadcore_internals.module_index = 3;
	for ( i = 0; i < 17; i += 1 )
	{
		bootmodes[i] = 0;
	}
	{
		u32 bootmode_tmp[1];

		bootmode_tmp[0] = (params.BootMode & 0xFFFF) | 0x40000;
		RegisterBootMode((iop_bootmode_t *)bootmode_tmp);
	}
	sysmemmi = (ModuleInfo_t *)((char *)loadcore_internals.let_prev - 0x30);
	loadcore_internals.image_info = sysmemmi;
	sysmemmi->id = 1;
	sysmemmi->newflags = 3;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	sysmemmi->next = (ModuleInfo_t *)((u8 *)&_ftext - 0x30);
	((ModuleInfo_t *)loadcore_internals.image_info->next)->id = 2;
	((ModuleInfo_t *)loadcore_internals.image_info->next)->newflags = 3;
#pragma GCC diagnostic pop
	// cppcheck-suppress comparePointers
	LinkLibraryEntries((u32 *)&_ftext, (u8 *)&_etext - (u8 *)&_ftext);
	RegisterLibraryEntries(&_exp_loadcore);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	AllocSysMemory(
		2,
		(u32)(((u8 *)&_end) - ((u32)(((u8 *)&_ftext) - 0x30) >> 8 << 8)),
		(void *)(((u32)(((u8 *)&_ftext) - 0x30) >> 8 << 8) & 0x1FFFFFFF));
#pragma GCC diagnostic pop
	if ( params.IOPRPBuffer )
		params.IOPRPBuffer = AllocSysMemory(2, params.IOPRPBufferSize, params.IOPRPBuffer);
	frame_pointer_curfunc = __builtin_frame_address(0);
#if 0
	// FIXME: wipe current stack contents
	lc_memset32((int *)((unsigned int)frame_pointer_curfunc & 0x1FFFFF00), stack_pointer_curfunc, 0x11111111);
#endif
	if ( ((unsigned int)frame_pointer_curfunc & 0x1FFFFF00) < QueryMemSize() )
	{
		MemSize = QueryMemSize();
		AllocSysMemory(
			2,
			MemSize - ((unsigned int)frame_pointer_curfunc & 0x1FFFFF00),
			(void *)((unsigned int)frame_pointer_curfunc & 0x1FFFFF00));
	}
	if ( params.command )
	{
		char *stack_command;
		int stack_command_size;
		u32 bootmode_tmp[2];

		stack_command_size = lc_strlen(params.command) + 1;
		stack_command = __builtin_alloca(stack_command_size);
		lc_memmove(stack_command, params.command, stack_command_size);
		// cppcheck-suppress unreadVariable
		params.command = stack_command;
		bootmode_tmp[0] = 0x1050000;
		bootmode_tmp[1] = (u32)stack_command;
		RegisterBootMode((iop_bootmode_t *)bootmode_tmp);
	}
	{
		char *stack_moddata;
		int stack_moddata_size;

		stack_moddata_size = (params.NumModules + 1) * 4;
		stack_moddata = __builtin_alloca(stack_moddata_size);
		lc_memmove(stack_moddata, (const char *)params.ModData, stack_moddata_size);
		params.ModData = (const void **)stack_moddata;
	}
	stack_reboot_handlers = __builtin_alloca(params.NumModules * (sizeof(u32) * 2));
	reboot_handlers = stack_reboot_handlers;

	for ( i = (unsigned int)QueryBlockTopAddress(0); i != 0xFFFFFFFF;
				i = (unsigned int)QueryBlockTopAddress(&blocksize_mask_1[blocksize_mask_2]) )
	{
		blocksize_mask_1 = (char *)(i & 0x7FFFFFFF);
		BlockSize = QueryBlockSize(blocksize_mask_1);
		blocksize_mask_2 = BlockSize & 0x7FFFFFFF;
		if ( BlockSize < 0 )
			lc_memset32((int *)blocksize_mask_1, 0x400D, blocksize_mask_2);
	}
	curmodaddr_align = 0;
	stack_reboot_handlers[0] = 0;
#if 0
	u8 module_index = 0;
#endif
	cur_module_addr = (elf_header_t **)(params.ModData + 2);
	while ( 1 )
	{
		if ( !*cur_module_addr )
		{
			if ( params.IOPRPBuffer )
			{
				lc_memset32((int *)params.IOPRPBuffer, 0x400D, params.IOPRPBufferSize);
				FreeSysMemory(params.IOPRPBuffer);
			}
			for ( i = 0; i < 4; i += 1 )
			{
				u32 *reboot_handler_ptr;

				if ( i == 3 )
					reboot_handlers = NULL;
				reboot_handler_ptr = stack_reboot_handlers;
				while ( *reboot_handler_ptr )
				{
					if ( (*reboot_handler_ptr & 3) == i )
					{
						iop_init_entry_t next;

						next.callback = (void *)*stack_reboot_handlers;
						if ( i == 3 )
							next.callback = (void *)*reboot_handler_ptr;
						__asm__ __volatile__("\tmove $gp, %0\n" : : "r"(reboot_handler_ptr[1]));
						((void (*)(iop_init_entry_t *, int))(*reboot_handler_ptr & (~3)))(&next, 1);
					}
					reboot_handler_ptr += 2;
				}
				if ( i == 2 )
				{
					while ( stack_reboot_handlers < reboot_handler_ptr )
					{
						reboot_handler_ptr -= 2;
						if ( (*reboot_handler_ptr & 3) == 3 )
							break;
						*reboot_handler_ptr = 0;
					}
				}
			}
			break;
		}
		if ( ((unsigned int)*cur_module_addr & 1) != 0 )
		{
			if ( ((unsigned int)*cur_module_addr & 0xF) == 1 )
				curmodaddr_align = (void *)((unsigned int)*cur_module_addr >> 2);
		}
		else
		{
#if 0
			module_index += 1;
			module_index &= 0xF;
#endif
			executable_type = ProbeExecutableObject(*cur_module_addr, &fi);
			if ( executable_type == IOP_MOD_TYPE_IRX )
			{
				if ( curmodaddr_align )
				{
					memalloctype = 2;
					memallocaddr = curmodaddr_align;
				}
				else
				{
					memalloctype = 0;
					memallocaddr = 0;
				}
				fi.text_start = AllocSysMemory(memalloctype, fi.MemSize + 0x30, memallocaddr);
				if ( !fi.text_start )
					break;
				fi.text_start = (char *)fi.text_start + 0x30;
			}
			mi = (ModuleInfo_t *)((char *)fi.text_start - 48);
			if ( executable_type == IOP_MOD_TYPE_COFF || executable_type == IOP_MOD_TYPE_ELF )
			{
				if ( !AllocSysMemory(
							 2,
							 (int)fi.text_start + fi.MemSize - ((((u32)mi) >> 8 << 8) & 0x1FFFFFFF),
							 (void *)((((u32)mi) >> 8 << 8) & 0x1FFFFFFF)) )
					break;
			}
			LoadExecutableObject(*cur_module_addr, &fi);
			if ( LinkLibraryEntries(fi.text_start, fi.text_size) == 0 )
			{
				FlushIcache();
				entrypoint_ret = ((int (*)(u32, u32, elf_header_t **, u32))fi.EntryPoint)(0, 0, cur_module_addr, 0);
				if ( (entrypoint_ret & 3) != 1 )
				{
					RegisterModule(mi);
					mi->newflags = 3;
					if ( (entrypoint_ret & 3) == 2 )
						mi->newflags |= 0x10;
					if ( (entrypoint_ret & (~3)) != 0 )
						AddRebootNotifyHandler((BootupCallback_t)(entrypoint_ret & (~3)), 2, 0);
				}
				else
				{
					UnLinkLibraryEntries(fi.text_start, fi.text_size);
					FreeSysMemory((void *)(((u32)mi) >> 8 << 8));
				}
			}
			else
			{
				FreeSysMemory((void *)(((u32)mi) >> 8 << 8));
			}
			curmodaddr_align = 0;
		}
		cur_module_addr += 1;
	}
	while ( 1 )
		*(vu8 *)0x80000000 = 2;
}

void RegisterBootMode(iop_bootmode_t *b)
{
	u32 *bootmode_dst;
	int effective_len;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	bootmode_dst = *bootmodes_end_ptr;
#pragma GCC diagnostic pop
	effective_len = b->len + 1;
	if ( sizeof(bootmodes) >= (effective_len * sizeof(bootmodes[0])) )
	{
		int i;

		for ( i = 0; i < effective_len; i += 1 )
		{
			bootmode_dst[i] = ((u32 *)b)[i];
		}
		bootmode_dst[effective_len] = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
		*bootmodes_end_ptr = bootmode_dst;
#pragma GCC diagnostic pop
	}
}

int *QueryBootMode(int mode)
{
	iop_bootmode_t *bootmode_cur;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	bootmode_cur = (iop_bootmode_t *)*bootmodes_start_ptr;
#pragma GCC diagnostic pop
	while ( *(u32 *)bootmode_cur )
	{
		if ( mode == bootmode_cur->id )
			return (int *)bootmode_cur;
		bootmode_cur = (iop_bootmode_t *)((u8 *)bootmode_cur + ((bootmode_cur->len + 1)) * 4);
	}
	return NULL;
}

int AddRebootNotifyHandler(BootupCallback_t func, int priority, int *stat)
{
	u32 gp_val;
	iop_init_entry_t next;

	next.callback = (void *)1;
	gp_val = 0;
	__asm__ __volatile__("\tmove %0, $gp\n" : "=r"(gp_val) :);

	if ( !reboot_handlers )
	{
		int stat_tmp;

		stat_tmp = ((int (*)(iop_init_entry_t *, u32))func)(&next, 0);
		if ( stat )
			*stat = stat_tmp;
		return 0;
	}

	reboot_handlers[0] = (u32)func + (priority & 3);
	reboot_handlers[1] = gp_val;
	reboot_handlers += 2;
	reboot_handlers[0] = 0;
	return 1;
}

void RegisterModule(ModuleInfo_t *mi)
{
	ModuleInfo_t *image_info;
	int module_index;

	image_info = loadcore_internals.image_info;
	while ( image_info && image_info->next && (image_info->next < mi) )
	{
		image_info = image_info->next;
	}
	mi->next = image_info;
	loadcore_internals.image_info = mi;
	module_index = loadcore_internals.module_index;
	mi->id = loadcore_internals.module_index & 0xFFFF;
	loadcore_internals.module_index = module_index + 1;
	loadcore_internals.module_count += 1;
}

// cppcheck-suppress constParameterPointer
void ReleaseModule(ModuleInfo_t *mi)
{
	ModuleInfo_t *image_info;

	if ( !mi )
	{
		return;
	}
	image_info = loadcore_internals.image_info;
	while ( image_info && (image_info->next != mi) )
	{
		image_info = image_info->next;
	}
	if ( !image_info )
	{
		return;
	}
	image_info->next = image_info->next->next;
	loadcore_internals.module_count -= 1;
}

ModuleInfo_t *SearchModuleCBByAddr(void *addr)
{
	ModuleInfo_t *image_info;

	image_info = loadcore_internals.image_info;
	while ( image_info )
	{
		if ( (unsigned int)addr >= image_info->text_start )
		{
			if (
				(unsigned int)addr
				< image_info->text_start + image_info->text_size + image_info->data_size + image_info->bss_size )
				return image_info;
		}
		image_info = image_info->next;
	}
	return NULL;
}

int RegisterLibraryEntries(struct irx_export_table *exports)
{
	struct irx_export_table *let_next;
	struct irx_export_table *nexttmp1;
	struct irx_export_table *next;
	struct irx_export_table *nexttmp2;
	iop_library_t **p_mda_next;
	iop_library_t *nexttmp3;
	iop_library_t *caller;
	struct irx_export_table *nexttmp4;
	struct irx_import_table *nexttmp5;
	int state;

	if ( !exports || exports->magic != 0x41C00000 )
		return KE_ILLEGAL_LIBRARY;
	cCpuSuspendIntr(&state);
	let_next = (struct irx_export_table *)loadcore_internals.let_next;
	nexttmp1 = 0;
	while ( let_next )
	{
		if ( IsSameLibrary(exports, let_next) && !compLibMinVersion_major(exports, let_next) )
		{
			struct irx_export_table **p_next;

			if ( compLibMinVersion_minor(exports, let_next) <= 0 )
			{
				cCpuResumeIntr(state);
				return KE_LIBRARY_FOUND;
			}
			p_next = &let_next->next;
			next = let_next->next;
			nexttmp2 = let_next->next;
			let_next->next = 0;
			while ( nexttmp2 )
			{
				nexttmp2 = next->next;
				if ( (next->mode & 1) != 0 )
				{
					*p_next = next;
					p_next = &next->next;
					next->next = 0;
				}
				else
				{
					next->next = nexttmp1;
					nexttmp1 = next;
				}
				next = nexttmp2;
			}
		}
		let_next = (struct irx_export_table *)let_next->magic;
	}
	p_mda_next = &loadcore_internals.mda_next;
	while ( p_mda_next[1] )
	{
		if (
			!IsLibraryCompliant(exports, (struct irx_export_table *)p_mda_next[1])
			|| compLibMinVersion_major_1(exports, (struct irx_export_table *)p_mda_next[1]) )
		{
			p_mda_next = (iop_library_t **)p_mda_next[1];
		}
		else
		{
			nexttmp3 = p_mda_next[1];
			caller = (iop_library_t *)nexttmp3->caller;
			nexttmp3->caller = (struct irx_import_table *)nexttmp1;
			nexttmp1 = (struct irx_export_table *)p_mda_next[1];
			p_mda_next[1] = caller;
		}
	}
	exports->next = 0;
	nexttmp5 = (struct irx_import_table *)nexttmp1;
	while ( nexttmp1 )
	{
		nexttmp4 = nexttmp1->next;
		aLinkClient(nexttmp5, exports);
		nexttmp1->next = exports->next;
		exports->next = nexttmp1;
		nexttmp1 = nexttmp4;
		nexttmp5 = (struct irx_import_table *)nexttmp4;
	}
	exports->mode &= ~1;
	exports->magic = (u32)loadcore_internals.let_next;
	loadcore_internals.let_next = (iop_library_t *)exports;
	cCpuResumeIntr(state);
	FlushIcache();
	return KE_OK;
}

int RegisterNonAutoLinkEntries(struct irx_export_table *exports)
{
	if ( !exports || exports->magic != 0x41C00000 )
		return KE_ILLEGAL_LIBRARY;
	exports->mode |= 1;
	exports->magic = (u32)loadcore_internals.let_next;
	loadcore_internals.let_next = (iop_library_t *)exports;
	FlushIcache();
	return KE_OK;
}

int ReleaseLibraryEntries(struct irx_export_table *exports)
{
	lc_internals_t *lcitmp;
	struct irx_export_table *let_next;
	u32 magic;
	int state;

	cCpuSuspendIntr(&state);
	lcitmp = &loadcore_internals;
	let_next = (struct irx_export_table *)loadcore_internals.let_next;
	while ( let_next && let_next != exports )
	{
		lcitmp = (lc_internals_t *)let_next;
		let_next = (struct irx_export_table *)let_next->magic;
	}
	if ( let_next != exports )
	{
		cCpuResumeIntr(state);
		return KE_LIBRARY_NOTFOUND;
	}
	if ( exports->next )
	{
		cCpuResumeIntr(state);
		return KE_LIBRARY_INUSE;
	}
	magic = exports->magic;
	exports->next = 0;
	lcitmp->let_next = (iop_library_t *)magic;
	exports->magic = 0x41C00000;
	cCpuResumeIntr(state);
	return KE_OK;
}

void *QueryLibraryEntryTable(iop_library_t *library)
{
	struct irx_export_table *let_next;

	let_next = (struct irx_export_table *)loadcore_internals.let_next;
	while ( let_next )
	{
		if ( IsLibraryCompliant(let_next, (struct irx_export_table *)library) )
		{
			if ( compLibMinVersion_major_1(let_next, (struct irx_export_table *)library) == 0 )
				return let_next->fptrs;
		}
		let_next = (struct irx_export_table *)let_next->magic;
	}
	return NULL;
}

int LinkLibraryEntries(void *addr, int size)
{
	unsigned int i;

	for ( i = 0; i < (unsigned int)size >> 2; i += 1 )
	{
		struct irx_import_table *importtmp1;

		importtmp1 = (struct irx_import_table *)((u32 *)addr)[i];
		if ( importtmp1->magic == 0x41E00000 && CheckCallerStub(importtmp1) && (importtmp1->mode & 7) == 0 )
		{
			if ( aLinkLibEntries(importtmp1) )
			{
				UnLinkLibraryEntries(addr, size);
				return KE_ERROR;
			}
		}
	}
	return KE_OK;
}

int UnLinkLibraryEntries(void *addr, int size)
{
	struct irx_import_table *let_next;
	const struct irx_import_table *nexttmp1;
	struct irx_import_table *next;
	struct irx_import_table *magic;
	struct irx_import_table *p_mda_next;
	struct irx_import_table *nexttmp2;
	struct irx_import_table *nexttmp3;

	let_next = (struct irx_import_table *)loadcore_internals.let_next;
	nexttmp1 = (struct irx_import_table *)((char *)addr + 4 * ((unsigned int)size >> 2));
	while ( let_next )
	{
		next = let_next->next;
		magic = (struct irx_import_table *)let_next->magic;
		while ( next )
		{
			if ( (u8 *)next >= (u8 *)addr && next < nexttmp1 )
			{
				if ( aUnLinkLibEntries(let_next, next) != 0 )
					return KE_ERROR;
				next->mode &= ~7;
				cleanStub(next);
			}
			next = next->next;
		}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
		if ( (u8 *)let_next >= (u8 *)addr && let_next < nexttmp1 )
			ReleaseLibraryEntries((void *)let_next);
#pragma GCC diagnostic pop
		let_next = magic;
	}
	p_mda_next = (struct irx_import_table *)&loadcore_internals.mda_next;
	while ( p_mda_next->next )
	{
		next = p_mda_next->next;
		if ( next < (struct irx_import_table *)addr || next >= nexttmp1 )
		{
			p_mda_next = p_mda_next->next;
		}
		else
		{
			next->mode &= ~7;
			cleanStub(p_mda_next->next);
			nexttmp2 = p_mda_next->next;
			nexttmp3 = nexttmp2->next;
			nexttmp2->next = 0;
			p_mda_next->next = nexttmp3;
		}
	}
	return KE_OK;
}

lc_internals_t *GetLoadcoreInternalData(void)
{
	return &loadcore_internals;
}

void LockLibraryClient(struct irx_export_table *export)
{
	export->mode |= 1;
}

void UnLockLibraryClient(struct irx_export_table *export)
{
	export->mode &= ~1;
}

int SetRebootTimeLibraryHandlingMode(struct irx_export_table *exports, int mode)
{
	struct irx_export_table *let_next;
	int state;

	if ( !exports )
		return KE_ILLEGAL_LIBRARY;
	cCpuSuspendIntr(&state);
	let_next = (struct irx_export_table *)loadcore_internals.let_next;
	while ( let_next && let_next->magic )
	{
		let_next = (struct irx_export_table *)let_next->magic;
	}
	if ( let_next != exports && exports->magic != 0x41C00000 )
	{
		cCpuResumeIntr(state);
		return KE_LIBRARY_NOTFOUND;
	}
	exports->mode &= ~6;
	exports->mode |= (mode & 6);
	cCpuResumeIntr(state);
	return KE_OK;
}

static int IsSameLibrary(const struct irx_export_table *src, const struct irx_export_table *dst)
{
	return (*(u32 *)src->name == *(u32 *)dst->name) && (*(u32 *)&src->name[4] == *(u32 *)&dst->name[4]);
}

static int compLibMinVersion_major(const struct irx_export_table *src, const struct irx_export_table *dst)
{
	return ((src->version & 0xFF00) >> 8) - ((dst->version & 0xFF00) >> 8);
}

static int compLibMinVersion_minor(const struct irx_export_table *src, const struct irx_export_table *dst)
{
	return (src->version & 0xFF) - (dst->version & 0xFF);
}

static int IsLibraryCompliant(const struct irx_export_table *src, const struct irx_export_table *dst)
{
	return (*(u32 *)dst->name == *(u32 *)src->name) && (*(u32 *)&dst->name[4] == *(u32 *)&src->name[4]);
}

static int compLibMinVersion_major_1(const struct irx_export_table *src, const struct irx_export_table *dst)
{
	return ((src->version & 0xFF00) >> 8) - ((dst->version & 0xFF00) >> 8);
}

static void cleanStub(const struct irx_import_table *imp)
{
	const void **stubs;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
	stubs = (const void **)(imp->stubs);
#pragma GCC diagnostic pop
	while ( stubs[0] && (unsigned int)stubs[1] >> 26 != 9 )
	{
		stubs[0] = (void *)0x3E00008;
		stubs += 2;
	}
}

static int CheckCallerStub(const struct irx_import_table *imp)
{
	const void **stubs;

	if ( imp->magic != 0x41E00000 )
		return 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
	stubs = (const void **)(imp->stubs);
#pragma GCC diagnostic pop
	while ( stubs[0] && (unsigned int)stubs[1] >> 26 == 9
					&& (stubs[0] == (void *)0x3E00008 || (unsigned int)(stubs[0]) >> 26 == 2) )
	{
		stubs += 2;
	}
	if ( stubs[0] || stubs[1] )
		return 0;
	return (const void **)(imp->stubs) < stubs;
}

static int aLinkLibEntries(struct irx_import_table *imp)
{
	iop_library_t *let_next;

	let_next = loadcore_internals.let_next;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
	while (
		let_next
		&& ((let_next->flags & 1) != 0 || !IsLibraryCompliant((struct irx_export_table *)let_next, (struct irx_export_table *)imp) || compLibMinVersion_major_1((struct irx_export_table *)let_next, (struct irx_export_table *)imp)) )
	{
		let_next = let_next->prev;
	}
#pragma GCC diagnostic pop
	if ( !let_next )
		return KE_ERROR;
	aLinkClient(imp, (const struct irx_export_table *)let_next);
	imp->next = let_next->caller;
	let_next->caller = imp;
	FlushIcache();
	return KE_OK;
}

static void aLinkClient(struct irx_import_table *imp, const struct irx_export_table *exp)
{
	void **stubs;
	unsigned int fptrs_count;
	const void **fptrs;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
	stubs = imp->stubs;
#pragma GCC diagnostic pop
	fptrs_count = 0;
	fptrs = (const void **)exp->fptrs;
	while ( fptrs[fptrs_count] )
	{
		fptrs_count += 1;
	}
	for ( ; *stubs; stubs += 2 )
	{
		unsigned int stubtmp;

		stubtmp = (u16)(uiptr)stubs[1];
		if ( (unsigned int)stubs[1] >> 26 != 9 )
			break;
		*stubs =
			(void *)(stubtmp >= fptrs_count ? 0x3E00008 : (((unsigned int)fptrs[stubtmp] >> 2) & 0x3FFFFFF) | 0x8000000);
	}
	imp->mode &= ~6;
	imp->mode |= 2;
}

static int aUnLinkLibEntries(struct irx_import_table *a1, const struct irx_import_table *a2)
{
	struct irx_import_table *next;
	struct irx_import_table *nexttmp1;

	next = a1->next;
	if ( next == a2 )
	{
		a1->next = a2->next;
		return KE_OK;
	}
	while ( next->next )
	{
		nexttmp1 = next->next;
		if ( nexttmp1 == a2 )
		{
			next->next = nexttmp1->next;
			nexttmp1->next = 0;
			return KE_OK;
		}
		if ( !nexttmp1->next )
			break;
		next = nexttmp1;
	}
	return KE_ERROR;
}

static void lc_memset32(int *b, int c, int len)
{
	int i;
	for ( i = 0; i < len; i += 1 )
	{
		b[i] = c;
	}
}

static int lc_strlen(const char *s)
{
	int len;

	len = 0;
	if ( !s )
		return 0;
	while ( s[len] )
		len += 1;
	return len;
}

static void lc_memmove(char *dst, const char *src, int len)
{
	if ( dst )
	{
		if ( dst < src )
		{
			char *v3;

			v3 = dst;

			for ( ; len > 0; v3 += 1 )
			{
				*v3 = *src;
				src += 1;
				len -= 1;
			}
		}
		else
		{
			int i;
			char *v5;
			char v6;

			for ( i = len - 1; i >= 0; *v5 = v6 )
			{
				v5 = &dst[i];
				v6 = src[i];
				i -= 1;
			}
		}
	}
}

static int cCpuSuspendIntr(int *state)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(loadcore_internals.intr_suspend_tbl);
	if ( intrman_callbacks && intrman_callbacks->cbCpuSuspendIntr )
		return intrman_callbacks->cbCpuSuspendIntr(state);
	else
		return 0;
}

static int cCpuResumeIntr(int state)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(loadcore_internals.intr_suspend_tbl);
	if ( intrman_callbacks && intrman_callbacks->cbCpuResumeIntr )
		return intrman_callbacks->cbCpuResumeIntr(state);
	else
		return 0;
}

#if 0
static int cQueryIntrContext(void)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(loadcore_internals.intr_suspend_tbl);
	if ( intrman_callbacks && intrman_callbacks->cbQueryIntrContext )
		return intrman_callbacks->cbQueryIntrContext();
	else
		return 0;
}
#endif

int ProbeExecutableObject(void *image, FileInfo_t *result)
{
	const struct scnhdr *COFF_ScnHdr;
	const AOUTHDR *COFF_AoutHdr;
	const struct iopmod *iopmod;

	COFF_AoutHdr = (AOUTHDR *)((unsigned int)image + sizeof(struct coff_filehdr));
	COFF_ScnHdr = (struct scnhdr *)((unsigned int)image + sizeof(struct coff_filehdr) + sizeof(AOUTHDR));
	if (
		((struct coff_filehdr *)image)->f_magic == MIPSELMAGIC && COFF_AoutHdr->magic == OMAGIC
		&& ((struct coff_filehdr *)image)->f_nscns < 0x20 && (((struct coff_filehdr *)image)->f_opthdr == 0x38)
		&& ((((struct coff_filehdr *)image)->f_flags & 0x2) != 0) && COFF_ScnHdr->s_paddr == COFF_AoutHdr->text_start )
	{
		if ( COFF_AoutHdr->vstamp != 0x7001 )
		{
			result->ModuleType = IOP_MOD_TYPE_COFF;
			result->EntryPoint = (void *)COFF_AoutHdr->entry;
			result->gp = (void *)COFF_AoutHdr->gp_value;
			result->text_start = (void *)COFF_AoutHdr->text_start;
			result->text_size = COFF_AoutHdr->tsize;
			result->data_size = COFF_AoutHdr->dsize;
			result->bss_size = COFF_AoutHdr->bsize;
			result->MemSize = COFF_AoutHdr->bss_start + COFF_AoutHdr->bsize - COFF_AoutHdr->text_start;
			result->mod_id = COFF_AoutHdr->mod_id;

			return result->ModuleType;
		}
	}
	else
	{
		const elf_header_t *ELF_Hdr;
		const elf_pheader_t *ELF_phdr;

		ELF_Hdr = image;
		ELF_phdr = (elf_pheader_t *)((unsigned int)image + ELF_Hdr->phoff);

		if (
			((unsigned short int *)ELF_Hdr->ident)[2] == 0x101 && ELF_Hdr->machine == 8
			&& ELF_Hdr->phentsize == sizeof(elf_pheader_t) && ELF_Hdr->phnum == 2
			&& (ELF_phdr->type == (SHT_LOPROC | SHT_LOPROC_IOPMOD))
			&& (ELF_Hdr->type == ET_SCE_IOPRELEXEC || ELF_Hdr->type == ET_SCE_IOPRELEXEC2 || ELF_Hdr->type == ET_EXEC) )
		{
			result->ModuleType = (ELF_Hdr->type == ET_SCE_IOPRELEXEC || ELF_Hdr->type == ET_SCE_IOPRELEXEC2) ?
														 IOP_MOD_TYPE_IRX :
														 IOP_MOD_TYPE_ELF;

			iopmod = (struct iopmod *)((unsigned int)image + ELF_phdr->offset);
			result->EntryPoint = (void *)iopmod->EntryPoint;
			result->gp = (void *)iopmod->gp;
			result->text_start = (void *)ELF_phdr[1].vaddr;
			result->text_size = iopmod->text_size;
			result->data_size = iopmod->data_size;
			result->bss_size = iopmod->bss_size;
			result->MemSize = ELF_phdr[1].memsz;
			result->mod_id = iopmod->mod_id;

			return result->ModuleType;
		}
	}
	return KE_ERROR;
}

// cppcheck-suppress constParameterPointer
int LoadExecutableObject(void *image, FileInfo_t *fi)
{
	switch ( fi->ModuleType )
	{
		case IOP_MOD_TYPE_ELF:
			loadelf(image);
			break;
		case IOP_MOD_TYPE_COFF:
			loadcoff(image);
			break;
		case IOP_MOD_TYPE_IRX:
			loadrelelf(image, fi);
			break;
		default:
			return KE_ERROR;
	}

	CopyModInfo(fi, (void *)((u8 *)(fi->text_start) - 0x30));

	return KE_OK;
}

void CopyModInfo(FileInfo_t *fi, ModuleInfo_t *mi)
{
	mi->next = 0;
	mi->name = 0;
	mi->version = 0;
	mi->newflags = 0;
	mi->id = 0;
	mi->flags = 0;
	if ( fi->mod_id != (IopModuleID_t *)0xFFFFFFFF )
	{
		mi->name = (char *)(fi->mod_id->name);
		mi->version = fi->mod_id->version;
	}
	mi->entry = (u32)fi->EntryPoint;
	mi->gp = (u32)fi->gp;
	mi->text_start = (u32)fi->text_start;
	mi->text_size = fi->text_size;
	mi->data_size = fi->data_size;
	mi->bss_size = fi->bss_size;
}

static void loadcoff(const void *module)
{
	const AOUTHDR *COFF_AoutHdr;
	const struct scnhdr *ScnHdr;

	COFF_AoutHdr = (AOUTHDR *)((u8 *)module + sizeof(struct coff_filehdr));
	ScnHdr = (struct scnhdr *)((u8 *)module + sizeof(struct coff_filehdr) + sizeof(AOUTHDR));

	CopySection((void *)((u8 *)module + ScnHdr[0].s_size), (void *)COFF_AoutHdr->text_start, COFF_AoutHdr->tsize);
	CopySection((void *)((u8 *)module + COFF_AoutHdr->tsize), (void *)COFF_AoutHdr->data_start, COFF_AoutHdr->dsize);

	if ( COFF_AoutHdr->bss_start != 0 && COFF_AoutHdr->bsize != 0 )
	{
		ZeroSection((unsigned int *)COFF_AoutHdr->bss_start, COFF_AoutHdr->bsize >> 2);
	}
}

static void loadelf(const void *module)
{
	const elf_header_t *ELF_Hdr;
	const elf_pheader_t *ELF_phdr;

	ELF_Hdr = module;
	ELF_phdr = (elf_pheader_t *)((u8 *)ELF_Hdr + ELF_Hdr->phoff);

	CopySection((void *)((u8 *)module + ELF_phdr[1].offset), (void *)ELF_phdr[1].vaddr, ELF_phdr[1].filesz);

	if ( ELF_phdr[1].filesz < ELF_phdr[1].memsz )
	{
		ZeroSection(
			(unsigned int *)((u8 *)(ELF_phdr[1].vaddr) + ELF_phdr[1].filesz), (ELF_phdr[1].memsz - ELF_phdr[1].filesz) >> 2);
	}
}

static void loadrelelf(const void *module, FileInfo_t *ModuleInfo)
{
	const elf_header_t *ELF_hdr;
	const elf_pheader_t *ELF_phdr;
	const elf_shdr_t *ELF_shdr, *CurrentELF_shdr;
	unsigned int NumRelocs, SectionNum;

	ELF_hdr = (elf_header_t *)module;
	ELF_phdr = (elf_pheader_t *)((u8 *)module + ELF_hdr->phoff);

	ModuleInfo->gp = (void *)((u8 *)ModuleInfo->gp + (unsigned int)ModuleInfo->text_start);
	ModuleInfo->EntryPoint = (void *)((u8 *)ModuleInfo->EntryPoint + (unsigned int)ModuleInfo->text_start);

	if ( ModuleInfo->mod_id != (void *)0xFFFFFFFF )
	{
		ModuleInfo->mod_id = (IopModuleID_t *)((u8 *)ModuleInfo->mod_id + (unsigned int)ModuleInfo->text_start);
	}

	ELF_shdr = (elf_shdr_t *)((u8 *)module + ELF_hdr->shoff);

	CopySection((void *)((u8 *)module + ELF_phdr[1].offset), ModuleInfo->text_start, ELF_phdr[1].filesz);

	if ( ELF_phdr[1].filesz < ELF_phdr[1].memsz )
	{
		ZeroSection(
			(unsigned int *)((u8 *)(ModuleInfo->text_start) + ELF_phdr[1].filesz),
			(ELF_phdr[1].memsz - ELF_phdr[1].filesz) >> 2);
	}

	for ( SectionNum = 0, CurrentELF_shdr = ELF_shdr + 1; SectionNum < ELF_hdr->shnum;
				SectionNum += 1, CurrentELF_shdr += 1 )
	{
		if ( CurrentELF_shdr->type == SHT_REL )
		{
			u32 entsize;
			entsize = CurrentELF_shdr->entsize;
			if ( !entsize )
				__builtin_trap();
			NumRelocs = CurrentELF_shdr->size / entsize;
			ApplyElfRelSection(ModuleInfo->text_start, (const elf_rel *)((u8 *)module + CurrentELF_shdr->offset), NumRelocs);
		}
	}
}

void ApplyElfRelSection(void *buffer, const void *module, int element_count)
{
	u32 startaddr;
	int i;
	const elf_rel *ELF_relocation;

	startaddr = (u32)buffer;
	ELF_relocation = module;
	for ( i = 0; i < element_count; i += 1 )
	{
		u32 *datal;
		u32 datai;
		int daddr;

		datal = (u32 *)((u8 *)buffer + ELF_relocation[i].offset);
		switch ( ELF_relocation[i].info & 0xFF )
		{
			case R_MIPS_16:
				datai = startaddr + (s16) * (u32 *)datal;
				*(u32 *)datal &= 0xFFFF0000;
				*(u32 *)datal |= (u16)datai;
				break;
			case R_MIPS_32:
				*(u32 *)datal += startaddr;
				break;
			case R_MIPS_26:
				datai = startaddr + ((ELF_relocation[i].offset & 0xF0000000) | (4 * (*(u32 *)datal & 0x3FFFFFF)));
				*(u32 *)datal &= 0xFC000000;
				*(u32 *)datal |= datai << 4 >> 6;
				break;
			case R_MIPS_HI16:
				datai = startaddr + (s16) * (u32 *)((u8 *)buffer + ELF_relocation[i + 1].offset) + (*(u32 *)datal << 16);
				*(u32 *)datal &= 0xFFFF0000;
				*(u32 *)datal |= (u16)(((datai >> 15) + 1) >> 1);
				break;
			case R_MIPS_LO16:
				datai = (startaddr + *(u32 *)datal) & 0xFFFF;
				*(u32 *)datal &= 0xFFFF0000;
				*(u32 *)datal |= datai;
				break;
			case R_MIPSSCE_MHI16:
				datai = ((((startaddr + ELF_relocation[i + 1].offset) >> 15) + 1) >> 1) & 0xFFFF;
				for ( daddr = 1; daddr != 0; datal += daddr )
				{
					daddr = *(u16 *)datal << 16 >> 14;
					*(u32 *)datal &= 0xFFFF0000;
					*(u32 *)datal |= datai;
				}
				i += 1;
				break;
			default:
				break;
		}
	}
}

static void CopySection(const void *module, void *buffer, unsigned int FileSize)
{
	unsigned int *dst;
	const unsigned int *src;
	const void *src_end;

	dst = buffer;
	src = module;
	src_end = (const void *)((unsigned int)module + (FileSize >> 2 << 2));
	while ( (unsigned int)src < (unsigned int)src_end )
	{
		*dst = *src;
		src += 1;
		dst += 1;
	}
}

static void ZeroSection(unsigned int *buffer, unsigned int NumWords)
{
	while ( NumWords > 0 )
	{
		*buffer = 0;
		NumWords -= 1;
		buffer += 1;
	}
}

// clang-format off
__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global FlushIcache" "\n"
	"\t" "FlushIcache:" "\n"
	"\t" "  mfc0        $t0, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $t4, %hi(FlushIcache_inner)" "\n"
	"\t" "  addiu       $t4, $t4, %lo(FlushIcache_inner)" "\n"
	"\t" "  lui         $at, (0xA0000000 >> 16)" "\n"
	"\t" "  or          $t4, $t4, $at" "\n"
	"\t" "  jr          $t4" "\n"
	"\t" "FlushIcache_inner:" "\n"
	"\t" "   mtc0       $zero, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $t6, 0xBF80" "\n"
	"\t" "  lw          $t6, (0xBF801450 & 0xFFFF)($t6)" "\n"
	"\t" "  nop" "\n"
	"\t" "  addiu       $t7, $zero, -0x2" "\n"
	"\t" "  and         $t1, $t6, $t7" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $t1, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $t7, 0xBF80" "\n"
	"\t" "  lw          $t7, (0xBF801578 & 0xFFFF)($t7)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $zero, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $t5, (0xFFFE0130 >> 16)" "\n"
	"\t" "  lw          $t5, (0xFFFE0130 & 0xFFFF)($t5)" "\n"
	"\t" "  addiu       $t1, $zero, 0xC04" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  sw          $t1, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $t4, (0x10000 >> 16)" "\n"
	"\t" "  mtc0        $t4, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  nop" "\n"
	"\t" "  addiu       $t2, $zero, 0x0" "\n"
	"\t" "  addiu       $t3, $zero, 0xF80" "\n"
	"\t" ".LFlushIcache_1:" "\n"
	"\t" "  sw          $zero, 0x0($t2)" "\n"
	"\t" "  sw          $zero, 0x10($t2)" "\n"
	"\t" "  sw          $zero, 0x20($t2)" "\n"
	"\t" "  sw          $zero, 0x30($t2)" "\n"
	"\t" "  sw          $zero, 0x40($t2)" "\n"
	"\t" "  sw          $zero, 0x50($t2)" "\n"
	"\t" "  sw          $zero, 0x60($t2)" "\n"
	"\t" "  sw          $zero, 0x70($t2)" "\n"
	"\t" "  bne         $t2, $t3, .LFlushIcache_1" "\n"
	"\t" "   addi       $t2, $t2, 0x80" "\n"
	"\t" "  mtc0        $zero, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  sw          $t5, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  lw          $zero, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $t7, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $t6, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  mtc0        $t0, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global FlushDcache" "\n"
	"\t" "FlushDcache:" "\n"
	"\t" "  mfc0        $t0, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $t4, %hi(FlushDcache_inner)" "\n"
	"\t" "  addiu       $t4, $t4, %lo(FlushDcache_inner)" "\n"
	"\t" "  lui         $at, (0xA0000000 >> 16)" "\n"
	"\t" "  or          $t4, $t4, $at" "\n"
	"\t" "  jr          $t4" "\n"
	"\t" "FlushDcache_inner:" "\n"
	"\t" "   mtc0       $zero, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $t6, 0xBF80" "\n"
	"\t" "  lw          $t6, (0xBF801450 & 0xFFFF)($t6)" "\n"
	"\t" "  nop" "\n"
	"\t" "  addiu       $t7, $zero, -0x2" "\n"
	"\t" "  and         $t1, $t6, $t7" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $t1, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $t7, 0xBF80" "\n"
	"\t" "  lw          $t7, (0xBF801578 & 0xFFFF)($t7)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $zero, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $t5, (0xFFFE0130 >> 16)" "\n"
	"\t" "  lw          $t5, (0xFFFE0130 & 0xFFFF)($t5)" "\n"
	"\t" "  addiu       $t1, $zero, 0xC4" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  sw          $t1, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $t4, (0x10000 >> 16)" "\n"
	"\t" "  mtc0        $t4, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  nop" "\n"
	"\t" "  addiu       $t2, $zero, 0x0" "\n"
	"\t" "  addiu       $t3, $zero, 0x380" "\n"
	"\t" ".LFlushDcache_1:" "\n"
	"\t" "  sw          $zero, 0x0($t2)" "\n"
	"\t" "  sw          $zero, 0x10($t2)" "\n"
	"\t" "  sw          $zero, 0x20($t2)" "\n"
	"\t" "  sw          $zero, 0x30($t2)" "\n"
	"\t" "  sw          $zero, 0x40($t2)" "\n"
	"\t" "  sw          $zero, 0x50($t2)" "\n"
	"\t" "  sw          $zero, 0x60($t2)" "\n"
	"\t" "  sw          $zero, 0x70($t2)" "\n"
	"\t" "  bne         $t2, $t3, .LFlushDcache_1" "\n"
	"\t" "   addi       $t2, $t2, 0x80" "\n"
	"\t" "  mtc0        $zero, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  sw          $t5, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  lw          $zero, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $t7, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801578 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  sw          $t6, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, 0xBF80" "\n"
	"\t" "  lw          $zero, (0xBF801450 & 0xFFFF)($at)" "\n"
	"\t" "  mtc0        $t0, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);

__asm__ (
	"\t" ".set push" "\n"
	"\t" ".set noat" "\n"
	"\t" ".set noreorder" "\n"
	"\t" ".global SetCacheCtrl" "\n"
	"\t" "SetCacheCtrl:" "\n"
	"\t" "  mfc0        $t0, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $t4, %hi(SetCacheCtrl_inner)" "\n"
	"\t" "  addiu       $t4, $t4, %lo(SetCacheCtrl_inner)" "\n"
	"\t" "  lui         $at, (0xA0000000 >> 16)" "\n"
	"\t" "  or          $t4, $t4, $at" "\n"
	"\t" "  jr          $t4" "\n"
	"\t" "SetCacheCtrl_inner:" "\n"
	"\t" "   mtc0       $zero, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  sw          $a0, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  lui         $at, (0xFFFE0130 >> 16)" "\n"
	"\t" "  lw          $zero, (0xFFFE0130 & 0xFFFF)($at)" "\n"
	"\t" "  nop" "\n"
	"\t" "  mtc0        $t0, $12" "\n"
	"\t" "  nop" "\n"
	"\t" "  jr          $ra" "\n"
	"\t" "   nop" "\n"
	"\t" ".set pop" "\n"
);
// clang-format on
