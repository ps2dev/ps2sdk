#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>

/* ELF-loading stuff */
#define ELF_MAGIC 0x464c457f
#define ELF_TYPE_IRX 0xFF80  /* SCE IOP Relocatable eXcutable file version 1.0 */
#define ELF_TYPE_ERX2 0xFF91 /* SCE EE Relocatable eXcutable file version 2.0 */
#define ELF_PT_LOAD 1

/*------------------------------*/
typedef struct
{
	uint8_t ident[16]; /* Structure of a ELF header */
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
} elf_header_t;
/*------------------------------*/
typedef struct
{
	uint32_t type; /* Structure of a header a sections in an ELF */
	uint32_t offset;
	void *vaddr;
	uint32_t paddr;
	uint32_t filesz;
	uint32_t memsz;
	uint32_t flags;
	uint32_t align;
} elf_pheader_t;

typedef struct
{
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t addr;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t addralign;
	uint32_t entsize;
} elf_shdr_t;

typedef struct
{
	uint32_t offset;
	uint32_t info;
} elf_rel;

typedef struct
{
	uint32_t offset;
	uint32_t info;
	uint32_t addend;
} elf_rela;

enum ELF_SHT_types {
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

typedef struct iopmod_struct
{
	uint32_t moduleinfo;
	uint32_t entry;
	uint32_t gp_value;
	uint32_t text_size;
	uint32_t data_size;
	uint32_t bss_size;
	uint16_t version;
	char modname[];
} iopmod_t;

typedef struct eemod_struct
{
	uint32_t moduleinfo;
	uint32_t entry;
	uint32_t gp_value;
	uint32_t text_size;
	uint32_t data_size;
	uint32_t bss_size;
	uint32_t ERX_lib_addr;
	uint32_t ERX_lib_size;
	uint32_t ERX_stub_addr;
	uint32_t ERX_stub_size;
	uint16_t version;
	char modname[];
} eemod_t;

#define SHT_LOPROC 0x70000000
#define SHT_LOPROC_IOPMOD_TAB 0x80
#define SHT_LOPROC_EEMOD_TAB 0x90
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000

#endif /* __ELF_H__ */