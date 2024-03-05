typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;

/* ELF-loading stuff */
#define ELF_MAGIC 0x464c457f
#define ELF_TYPE_IRX 0xFF80  /* SCE IOP Relocatable eXcutable file version 1.0 */
#define ELF_TYPE_ERX2 0xFF91 /* SCE EE Relocatable eXcutable file version 2.0 */
#define ELF_PT_LOAD 1

/*------------------------------*/
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
/*------------------------------*/
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
	u32 moduleinfo;
	u32 entry;
	u32 gp_value;
	u32 text_size;
	u32 data_size;
	u32 bss_size;
	u16 version;
	char modname[];
} iopmod_t;

typedef struct eemod_struct
{
	u32 moduleinfo;
	u32 entry;
	u32 gp_value;
	u32 text_size;
	u32 data_size;
	u32 bss_size;
	u32 ERX_lib_addr;
	u32 ERX_lib_size;
	u32 ERX_stub_addr;
	u32 ERX_stub_size;
	u16 version;
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
