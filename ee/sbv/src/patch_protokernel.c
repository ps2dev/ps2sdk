#include <tamtypes.h>
#include <kernel.h>
#include <string.h>

extern void protokernel_patch_bin;

#define	KSEG0(vaddr)	(0x80000000 | vaddr)
#define	JAL(addr)	(0x0c000000 | ((addr & 0x03ffffff) >> 2))

#define ELF_PT_LOAD	1

typedef struct {
	u8	ident[16];	// struct definition for ELF object header
	u16	type;
	u16	machine;
	u32	version;
	u32	entry;
	u32	phoff;
	u32	shoff;
	u32	flags;
	u16	ehsize;
	u16	phentsize;
	u16	phnum;
	u16	shentsize;
	u16	shnum;
	u16	shstrndx;
} elf_header_t;

typedef struct {
	u32	type;		// struct definition for ELF program section header
	u32	offset;
	void	*vaddr;
	u32	paddr;
	u32	filesz;
	u32	memsz;
	u32	flags;
	u32	align;
} elf_pheader_t;

/*
	The SetOsdConfigParam() and GetOsdConfigParam() functions of a Protokernel functions slightly differently because they were based on an older set of specifications.
	If the region field is set to any other value other than 0, it gets reset. On newer kernels, it'll retain the value that is set.
*/
static inline int PatchIsNeeded(void){
	ConfigParam original_config;
	ConfigParam config;

	GetOsdConfigParam(&original_config);
	config=original_config;
	config.region=1;	/* config &0xFFFF1FFF, config |= 0x2000 */
	SetOsdConfigParam(&config);
	GetOsdConfigParam(&config);
	SetOsdConfigParam(&original_config);

	return(config.region<1);
}

int sbv_patch_protokernel(void)
{
	int res = -1;

	if(PatchIsNeeded()){
		DI();
		ee_kmode_enter();

		/* we copy the patch to its placement in kernel memory */
		u8 *elfptr = (u8 *)&protokernel_patch_bin;
		elf_header_t *eh = (elf_header_t *)elfptr;
		elf_pheader_t *eph = (elf_pheader_t *)&elfptr[eh->phoff];
		int i;

		for (i = 0; i < eh->phnum; i++) {
			if (eph[i].type != ELF_PT_LOAD) continue;

			memcpy(eph[i].vaddr, &elfptr[eph[i].offset], eph[i].filesz);

			if (eph[i].memsz > eph[i].filesz)
				memset((void *)(eph[i].vaddr + eph[i].filesz), 0, eph[i].memsz - eph[i].filesz);
		}

		/* insert a JAL to our kernel code into the ExecPS2 syscall */
		_sw(JAL(eh->entry), KSEG0(0x00002f88));

		res = 0;

		ee_kmode_exit();
		EI();
	}

	return res;
}

