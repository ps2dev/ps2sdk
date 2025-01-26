/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __SRXFIXUP_INTERNAL_H__
#define __SRXFIXUP_INTERNAL_H__

#include <stdint.h>

// Type definitions

// srxfixup.c, elflib.c, elfdump.c, srxgen.c, readconf.c

typedef short unsigned int Elf32_Half;
typedef unsigned int Elf32_Addr;
typedef unsigned int Elf32_Off;
typedef unsigned int Elf32_Word;
typedef int Elf32_SWord;
typedef struct _Elf32_ehdr
{
	unsigned char e_ident[16];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;
typedef struct _Elf32_Phdr
{
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;
typedef struct _Elf32_Shdr
{
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;
typedef struct _Elf32_Sym
{
	Elf32_Word st_name;
	Elf32_Addr st_value;
	Elf32_Word st_size;
	unsigned char st_info;
	unsigned char st_other;
	Elf32_Half st_shndx;
} Elf32_Sym;
typedef struct _Elf_Note
{
	Elf32_Word namesz;
	Elf32_Word descsz;
	Elf32_Word type;
	char name[];
} Elf_Note;
typedef struct _Elf32_Rel
{
	Elf32_Addr r_offset;
	Elf32_Word r_info;
} Elf32_Rel;
typedef struct _Elf32_RegInfo
{
	Elf32_Word ri_gprmask;
	Elf32_Word ri_cprmask[4];
	Elf32_SWord ri_gp_value;
} Elf32_RegInfo;
typedef struct _Elf32_IopMod
{
	Elf32_Word moduleinfo;
	Elf32_Addr entry;
	Elf32_Addr gp_value;
	Elf32_Word text_size;
	Elf32_Word data_size;
	Elf32_Word bss_size;
	Elf32_Half moduleversion;
	char modulename[];
} Elf32_IopMod;
typedef struct _Elf32_EeMod
{
	Elf32_Word moduleinfo;
	Elf32_Addr entry;
	Elf32_Addr gp_value;
	Elf32_Word text_size;
	Elf32_Word data_size;
	Elf32_Word bss_size;
	Elf32_Addr erx_lib_addr;
	Elf32_Word erx_lib_size;
	Elf32_Addr erx_stub_addr;
	Elf32_Word erx_stub_size;
	Elf32_Half moduleversion;
	char modulename[];
} Elf32_EeMod;
typedef struct _hdrr
{
	short int magic;
	short int vstamp;
	unsigned int ilineMax;
	unsigned int cbLine;
	unsigned int cbLineOffset;
	unsigned int idnMax;
	unsigned int cbDnOffset;
	unsigned int ipdMax;
	unsigned int cbPdOffset;
	unsigned int isymMax;
	unsigned int cbSymOffset;
	unsigned int ioptMax;
	unsigned int cbOptOffset;
	unsigned int iauxMax;
	unsigned int cbAuxOffset;
	unsigned int issMax;
	unsigned int cbSsOffset;
	unsigned int issExtMax;
	unsigned int cbSsExtOffset;
	unsigned int ifdMax;
	unsigned int cbFdOffset;
	unsigned int crfd;
	unsigned int cbRfdOffset;
	unsigned int iextMax;
	unsigned int cbExtOffset;
} hdrr;
typedef struct _fdr
{
	unsigned int adr;
	int rss;
	int issBase;
	int cbSs;
	int isymBase;
	int csym;
	int ilineBase;
	int cline;
	int ioptBase;
	int copt;
	short unsigned int ipdFirst;
	short int cpd;
	int iauxBase;
	int caux;
	int rfdBase;
	int crfd;
	unsigned int fdr_bits;
	int cbLineOffset;
	int cbLine;
} fdr;
typedef struct _symr
{
	int iss;
	int value;
	unsigned int sy_bits;
} symr;
typedef struct _extr
{
	short int reserved;
	short int ifd;
	symr asym;
} extr;
typedef struct _dnr
{
	unsigned int d_rfd;
	unsigned int d_index;
} dnr;
typedef struct _pdr
{
	unsigned int adr;
	int isym;
	int iline;
	int regmask;
	int regoffset;
	int iopt;
	int fregmask;
	int fregoffset;
	int frameoffset;
	short int framereg;
	short int pcreg;
	int lnLow;
	int lnHigh;
	unsigned int cbLineOffset;
} pdr;
typedef struct _tir
{
	unsigned int fBitfield;
	unsigned int continued;
	unsigned int bt;
	unsigned int tq4;
	unsigned int tq5;
	unsigned int tq0;
	unsigned int tq1;
	unsigned int tq2;
	unsigned int tq3;
} tir;
typedef struct _rndxr
{
	unsigned int rfd;
	unsigned int index;
} rndxr;
typedef union _auxu
{
	tir ti;
	rndxr rndx;
	int dnLow;
	int dnHigh;
	int isym;
	int iss;
	int width;
	int count;
} auxu;
typedef struct _optr
{
	unsigned int optr_bits;
	rndxr rndx;
	unsigned int offset;
} optr;
typedef int rfdt;
typedef struct _elf_mips_symbolic
{
	hdrr head;
	char *cbLine_Ptr;
	char *cbDn_Ptr;
	char *cbPd_Ptr;
	char *cbSym_Ptr;
	char *cbOpt_Ptr;
	char *cbAux_Ptr;
	char *cbSs_Ptr;
	char *cbSsExt_Ptr;
	char *cbFd_Ptr;
	char *cbRfd_Ptr;
	char *cbExt_Ptr;
} elf_mips_symbolic_data;
typedef struct _elf_section
{
	Elf32_Shdr shr;
	uint8_t *data;
	const char *name;
	unsigned int number;
	struct _elf_section *link;
	struct _elf_section *info;
	void *optdata;
} elf_section;
typedef struct _syment
{
	Elf32_Sym sym;
	const char *name;
	unsigned int number;
	int bind;
	unsigned int type;
	elf_section *shptr;
	int refcount;
} elf_syment;
typedef struct _rel
{
	Elf32_Rel rel;
	elf_syment *symptr;
	unsigned int type;
} elf_rel;
typedef struct _elf_proghead
{
	Elf32_Phdr phdr;
	elf_section **scp;
	void *optdata;  // Not used
} elf_proghead;
typedef struct _elffile
{
	Elf32_Ehdr *ehp;
	elf_section *shstrptr;
	elf_proghead *php;
	elf_section **scp;
	void *optdata;
} elf_file;

// elflib.c, elfdump.c, srxgen.c

typedef struct _elf_file_slot
{
	unsigned int offset;
	unsigned int size;
	unsigned int align;
	int type;
	union _elf_file_slot_d
	{
		elf_proghead *php;
		elf_section *scp;
	} d;
} Elf_file_slot;

// readconf.c, srxfixup.c, srxgen.c

typedef struct _segconf
{
	const char *name;
	int bitid;
	const char **sect_name_patterns;
	elf_section *empty_section;
	int nsect;
	elf_section **scp;
	unsigned int addr;
	unsigned int size;
} SegConf;
typedef struct _pheader_info
{
	int sw;
	union _pheader_info_d
	{
		const char *section_name;
		SegConf **segment_list;
	} d;
} PheaderInfo;
typedef struct _sectconf
{
	const char *sect_name_pattern;
	unsigned int flag;
	int secttype;
	int sectflag;
} SectConf;
typedef struct _crtsymconf
{
	const char *name;
	int bind;
	int type;
	SegConf *segment;
	const char *sectname;
	int shindex;
	int seflag;
} CreateSymbolConf;
typedef struct _srx_gen_table
{
	int target;
	SegConf *segment_list;
	CreateSymbolConf *create_symbols;
	PheaderInfo *program_header_order;
	SectConf *section_list;
	const char **removesection_list;
	const char **section_table_order;
	const char **file_layout_order;
} Srx_gen_table;

// anaarg.c, srxfixup.c

typedef struct _opt_strings
{
	struct _opt_strings *next;
	const char *string;
} Opt_strings;
typedef struct _opttable
{
	const char *option;
	int havearg;
	char vartype;
	void *var;
} Opttable;

// ring.c, anaarg.c, srxfixup.c

typedef struct _slink
{
	struct _slink *next;
} SLink;

// mipsdis.c, elfdump.c

typedef enum OperandTag
{
	OprTag_none = 0,
	OprTag_reg = 1,
	OprTag_c0reg_iop = 2,
	OprTag_c0reg_ee = 3,
	OprTag_czreg = 4,
	OprTag_c1reg = 5,
	OprTag_imm = 6,
	OprTag_shamt = 7,
	OprTag_jtarget = 8,
	OprTag_regoffset = 9,
	OprTag_code20 = 10,
	OprTag_code25 = 11
} OperandTag;
typedef struct operand
{
	enum OperandTag tag;
	unsigned char reg;
	unsigned int data;
} Operand;
typedef struct disasm_result
{
	unsigned int addr;
	unsigned int data;
	char mnemonic[16];
	Operand operands[4];
} Disasm_result;

// Function definitions

// anaarg.c
extern int analize_arguments(const Opttable *dopttable, int argc, char **argv);

// elfdump.c
extern void print_elf(const elf_file *elf, unsigned int flag);
extern void print_elf_ehdr(const elf_file *elf, unsigned int flag);
extern void print_elf_phdr(const elf_file *elf, unsigned int flag);
extern void print_elf_sections(const elf_file *elf, unsigned int flag);
extern void print_elf_reloc(const elf_section *scp, unsigned int flag);
extern void print_elf_disasm(const elf_file *elf, const elf_section *scp, unsigned int flag);
extern void print_elf_datadump(const elf_file *elf, const elf_section *scp, unsigned int flag);
extern void print_elf_symtbl(const elf_section *scp, unsigned int flag);
extern void print_elf_mips_symbols(const elf_mips_symbolic_data *symbol, unsigned int flag);

// elflib.c
extern elf_file *read_elf(const char *filename);
extern int layout_elf_file(elf_file *elf);
extern int write_elf(elf_file *elf, const char *filename);
extern void add_section(elf_file *elf, elf_section *scp);
extern elf_section *remove_section(elf_file *elf, Elf32_Word shtype);
extern elf_section *remove_section_by_name(elf_file *elf, const char *secname);
extern elf_section *search_section(elf_file *elf, Elf32_Word stype);
extern elf_section *search_section_by_name(elf_file *elf, const char *secname);
extern unsigned int *get_section_data(elf_file *elf, unsigned int addr);
extern elf_syment *search_global_symbol(const char *name, elf_file *elf);
extern int is_defined_symbol(const elf_syment *sym);
extern elf_syment *
add_symbol(elf_file *elf, const char *name, int bind, int type, int value, elf_section *scp, int st_shndx);
extern unsigned int get_symbol_value(const elf_syment *sym, const elf_file *elf);
extern void reorder_symtab(elf_file *elf);
extern unsigned int adjust_align(unsigned int value, unsigned int align);
extern void rebuild_section_name_strings(elf_file *elf);
extern void rebuild_symbol_name_strings(elf_file *elf);
extern Elf_file_slot *build_file_order_list(const elf_file *elf);
extern void shrink_file_order_list(Elf_file_slot *efs);
extern void writeback_file_order_list(elf_file *elf, Elf_file_slot *efs);
extern void dump_file_order_list(const elf_file *elf, const Elf_file_slot *efs);

// mipsdis.c
extern void gen_asmmacro(Disasm_result *result);
extern void initdisasm(int arch, int regform0, int regform1, int regform2, int regform3);
extern Disasm_result *disassemble(unsigned int addr, unsigned int data);
extern void shex(char *buf, unsigned int data);
extern void format_operand(const Operand *opr, char *buf);
extern void format_disasm(Disasm_result *dis, char *buf);

// readconf.c
extern Srx_gen_table *read_conf(const char *indata, const char *infile, int dumpopt);

// ring.c
extern SLink *add_ring_top(SLink *tailp, SLink *elementp);
extern SLink *add_ring_tail(SLink *tailp, SLink *elementp);
extern SLink *joint_ring(SLink *tailp, SLink *otherring);
extern SLink *ring_to_liner(SLink *tailp);

// srxfixup.c
extern void usage(const char *myname);
extern void stripusage(const char *myname);

// srxgen.c
extern int convert_rel2srx(elf_file *elf, const char *entrysym, int needoutput, int cause_irx1);
extern int layout_srx_file(elf_file *elf);
extern void strip_elf(elf_file *elf);
extern SegConf *lookup_segment(Srx_gen_table *conf, const char *segname, int msgsw);
extern void fixlocation_elf(elf_file *elf, unsigned int startaddr);
extern int relocation_is_version2(elf_section *relsect);
extern void dump_srx_gen_table(Srx_gen_table *tp);

// swapmem.c
extern void swapmemory(void *aaddr, const char *format, unsigned int times);

// eefixconf.c
extern const char *ee_defaultconf;

// iopfixconf.c
extern const char *iop_defaultconf;

enum Ei_class_name_enum
{
	ELFCLASSNONE = 0,
	ELFCLASS32 = 1,
	ELFCLASS64 = 2,
};

#define XEACH_Ei_class_name_enum()                                                                                     \
	X(ELFCLASSNONE)                                                                                                      \
	X(ELFCLASS32)                                                                                                        \
	X(ELFCLASS64)

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

#define XEACH_E_type_name_enum()                                                                                       \
	X(ET_NONE)                                                                                                           \
	X(ET_REL)                                                                                                            \
	X(ET_EXEC)                                                                                                           \
	X(ET_DYN)                                                                                                            \
	X(ET_CORE)                                                                                                           \
	X(ET_SCE_IOPRELEXEC)                                                                                                 \
	X(ET_SCE_IOPRELEXEC2)                                                                                                \
	X(ET_SCE_EERELEXEC)                                                                                                  \
	X(ET_SCE_EERELEXEC2)

enum Ei_data_name_enum
{
	ELFDATANONE = 0,
	ELFDATA2LSB = 1,
	ELFDATA2MSB = 2,
};

#define XEACH_Ei_data_name_enum()                                                                                      \
	X(ELFDATANONE)                                                                                                       \
	X(ELFDATA2LSB)                                                                                                       \
	X(ELFDATA2MSB)

enum E_version_name_enum
{
	EV_NONE = 0,
	EV_CURRENT = 1,
};

#define XEACH_E_version_name_enum()                                                                                    \
	X(EV_NONE)                                                                                                           \
	X(EV_CURRENT)

enum E_machine_name_enum
{
	EM_NONE = 0,
	EM_M32 = 1,
	EM_SPARC = 2,
	EM_386 = 3,
	EM_68K = 4,
	EM_88K = 5,
	EM_860 = 7,
	EM_MIPS = 8,
	EM_MIPS_RS4_BE = 10,
	EM_SPARC64 = 11,
	EM_PARISC = 15,
	EM_SPARC32PLUS = 18,
	EM_PPC = 20,
	EM_SH = 42,
};

#define XEACH_E_machine_name_enum()                                                                                    \
	X(EM_NONE)                                                                                                           \
	X(EM_M32)                                                                                                            \
	X(EM_SPARC)                                                                                                          \
	X(EM_386)                                                                                                            \
	X(EM_68K)                                                                                                            \
	X(EM_88K)                                                                                                            \
	X(EM_860)                                                                                                            \
	X(EM_MIPS)                                                                                                           \
	X(EM_MIPS_RS4_BE)                                                                                                    \
	X(EM_SPARC64)                                                                                                        \
	X(EM_PARISC)                                                                                                         \
	X(EM_SPARC32PLUS)                                                                                                    \
	X(EM_PPC)                                                                                                            \
	X(EM_SH)

enum P_type_name_enum
{
	PT_NULL = 0,
	PT_LOAD = 1,
	PT_DYNAMIC = 2,
	PT_INTERP = 3,
	PT_NOTE = 4,
	PT_SHLIB = 5,
	PT_PHDR = 6,
	PT_MIPS_REGINFO = 0x70000000,
	PT_MIPS_RTPROC = 0x70000001,
	PT_SCE_IOPMOD = 0x70000080,
	PT_SCE_EEMOD = 0x70000090,
};

#define XEACH_P_type_name_enum()                                                                                       \
	X(PT_NULL)                                                                                                           \
	X(PT_LOAD)                                                                                                           \
	X(PT_DYNAMIC)                                                                                                        \
	X(PT_INTERP)                                                                                                         \
	X(PT_NOTE)                                                                                                           \
	X(PT_SHLIB)                                                                                                          \
	X(PT_PHDR)                                                                                                           \
	X(PT_MIPS_REGINFO)                                                                                                   \
	X(PT_MIPS_RTPROC)                                                                                                    \
	X(PT_SCE_IOPMOD)                                                                                                     \
	X(PT_SCE_EEMOD)

enum S_type_name_enum
{
	SHT_NULL = 0,
	SHT_PROGBITS = 1,
	SHT_SYMTAB = 2,
	SHT_STRTAB = 3,
	SHT_RELA = 4,
	SHT_HASH = 5,
	SHT_DYNAMIC = 6,
	SHT_NOTE = 7,
	SHT_NOBITS = 8,
	SHT_REL = 9,
	SHT_SHLIB = 10,
	SHT_DYNSYM = 11,
	SHT_MIPS_LIBLIST = 0x70000000,
	SHT_MIPS_CONFLICT = 0x70000002,
	SHT_MIPS_GPTAB = 0x70000003,
	SHT_MIPS_UCODE = 0x70000004,
	SHT_MIPS_DEBUG = 0x70000005,
	SHT_MIPS_REGINFO = 0x70000006,
	SHT_SCE_IOPMOD = 0x70000080,
	SHT_SCE_EEMOD = 0x70000090,
};

#define XEACH_S_type_name_enum()                                                                                       \
	X(SHT_NULL)                                                                                                          \
	X(SHT_PROGBITS)                                                                                                      \
	X(SHT_SYMTAB)                                                                                                        \
	X(SHT_STRTAB)                                                                                                        \
	X(SHT_RELA)                                                                                                          \
	X(SHT_HASH)                                                                                                          \
	X(SHT_DYNAMIC)                                                                                                       \
	X(SHT_NOTE)                                                                                                          \
	X(SHT_NOBITS)                                                                                                        \
	X(SHT_REL)                                                                                                           \
	X(SHT_SHLIB)                                                                                                         \
	X(SHT_DYNSYM)                                                                                                        \
	X(SHT_MIPS_LIBLIST)                                                                                                  \
	X(SHT_MIPS_CONFLICT)                                                                                                 \
	X(SHT_MIPS_GPTAB)                                                                                                    \
	X(SHT_MIPS_UCODE)                                                                                                    \
	X(SHT_MIPS_DEBUG)                                                                                                    \
	X(SHT_MIPS_REGINFO)                                                                                                  \
	X(SHT_SCE_IOPMOD)                                                                                                    \
	X(SHT_SCE_EEMOD)

enum R_MIPS_Type_enum
{
	R_MIPS_NONE = 0,
	R_MIPS_16 = 1,
	R_MIPS_32 = 2,
	R_MIPS_REL32 = 3,
	R_MIPS_26 = 4,
	R_MIPS_HI16 = 5,
	R_MIPS_LO16 = 6,
	R_MIPS_GPREL16 = 7,
	R_MIPS_LITERAL = 8,
	R_MIPS_GOT16 = 9,
	R_MIPS_PC16 = 10,
	R_MIPS_CALL16 = 11,
	R_MIPS_GPREL32 = 12,
	R_MIPS_GOTHI16 = 21,
	R_MIPS_GOTLO16 = 22,
	R_MIPS_CALLHI16 = 30,
	R_MIPS_CALLLO16 = 31,
	R_MIPS_DVP_11_PCREL = 120,
	R_MIPS_DVP_27_S4 = 121,
	R_MIPS_DVP_11_S4 = 122,
	R_MIPS_DVP_U15_S3 = 123,
	R_MIPSSCE_MHI16 = 250,
	R_MIPSSCE_ADDEND = 251,
};

#define XEACH_R_MIPS_Type_enum()                                                                                       \
	X(R_MIPS_NONE)                                                                                                       \
	X(R_MIPS_16)                                                                                                         \
	X(R_MIPS_32)                                                                                                         \
	X(R_MIPS_REL32)                                                                                                      \
	X(R_MIPS_26)                                                                                                         \
	X(R_MIPS_HI16)                                                                                                       \
	X(R_MIPS_LO16)                                                                                                       \
	X(R_MIPS_GPREL16)                                                                                                    \
	X(R_MIPS_LITERAL)                                                                                                    \
	X(R_MIPS_GOT16)                                                                                                      \
	X(R_MIPS_PC16)                                                                                                       \
	X(R_MIPS_CALL16)                                                                                                     \
	X(R_MIPS_GPREL32)                                                                                                    \
	X(R_MIPS_GOTHI16)                                                                                                    \
	X(R_MIPS_GOTLO16)                                                                                                    \
	X(R_MIPS_CALLHI16)                                                                                                   \
	X(R_MIPS_CALLLO16)                                                                                                   \
	X(R_MIPS_DVP_11_PCREL)                                                                                               \
	X(R_MIPS_DVP_27_S4)                                                                                                  \
	X(R_MIPS_DVP_11_S4)                                                                                                  \
	X(R_MIPS_DVP_U15_S3)                                                                                                 \
	X(R_MIPSSCE_MHI16)                                                                                                   \
	X(R_MIPSSCE_ADDEND)

enum SymbolBinding_enum
{
	STB_LOCAL = 0,
	STB_GLOBAL = 1,
	STB_WEAK = 2,
};

#define XEACH_SymbolBinding_enum()                                                                                     \
	X(STB_LOCAL)                                                                                                         \
	X(STB_GLOBAL)                                                                                                        \
	X(STB_WEAK)

enum SymbolType_enum
{
	STT_NOTYPE = 0,
	STT_OBJECT = 1,
	STT_FUNC = 2,
	STT_SECTION = 3,
	STT_FILE = 4,
};

#define XEACH_SymbolType_enum()                                                                                        \
	X(STT_NOTYPE)                                                                                                        \
	X(STT_OBJECT)                                                                                                        \
	X(STT_FUNC)                                                                                                          \
	X(STT_SECTION)                                                                                                       \
	X(STT_FILE)

enum SymbolSpSection_enum
{
	SHN_UNDEF = 0,
	SHN_MIPS_ACOMMON = 0xFF00,
	SHN_MIPS_TEXT = 0xFF01,
	SHN_MIPS_DATA = 0xFF02,
	SHN_MIPS_SCOMMON = 0xFF03,
	SHN_MIPS_SUNDEFINED = 0xFF04,
	SHN_RADDR = 0xFF1F,
	SHN_ABS = 0xFFF1,
	SHN_COMMON = 0xFFF2,
};

#define XEACH_SymbolSpSection_enum()                                                                                   \
	X(SHN_UNDEF)                                                                                                         \
	X(SHN_MIPS_ACOMMON)                                                                                                  \
	X(SHN_MIPS_TEXT)                                                                                                     \
	X(SHN_MIPS_DATA)                                                                                                     \
	X(SHN_MIPS_SCOMMON)                                                                                                  \
	X(SHN_MIPS_SUNDEFINED)                                                                                               \
	X(SHN_RADDR)                                                                                                         \
	X(SHN_ABS)                                                                                                           \
	X(SHN_COMMON)

enum SymbolTypes_enum
{
	stNil = 0,
	stGlobal = 1,
	stStatic = 2,
	stParam = 3,
	stLocal = 4,
	stLabel = 5,
	stProc = 6,
	stBlock = 7,
	stEnd = 8,
	stMember = 9,
	stTypedef = 10,
	stFile = 11,
	stRegReloc = 12,
	stForward = 13,
	stStaticProc = 14,
	stConstant = 15,
	stStaParam = 16,
	stStruct = 26,
	stUnion = 27,
	stEnum = 28,
	stIndirect = 34,
	stStr = 60,
	stNumber = 61,
	stExpr = 62,
	stType = 63,
	stMax = 64,
};

#define XEACH_SymbolTypes_enum()                                                                                       \
	X(stNil)                                                                                                             \
	X(stGlobal)                                                                                                          \
	X(stStatic)                                                                                                          \
	X(stParam)                                                                                                           \
	X(stLocal)                                                                                                           \
	X(stLabel)                                                                                                           \
	X(stProc)                                                                                                            \
	X(stBlock)                                                                                                           \
	X(stEnd)                                                                                                             \
	X(stMember)                                                                                                          \
	X(stTypedef)                                                                                                         \
	X(stFile)                                                                                                            \
	X(stRegReloc)                                                                                                        \
	X(stForward)                                                                                                         \
	X(stStaticProc)                                                                                                      \
	X(stConstant)                                                                                                        \
	X(stStaParam)                                                                                                        \
	X(stStruct)                                                                                                          \
	X(stUnion)                                                                                                           \
	X(stEnum)                                                                                                            \
	X(stIndirect)                                                                                                        \
	X(stStr)                                                                                                             \
	X(stNumber)                                                                                                          \
	X(stExpr)                                                                                                            \
	X(stType)                                                                                                            \
	X(stMax)

enum StorageClasse_enum
{
	scNil = 0,
	scText = 1,
	scData = 2,
	scBss = 3,
	scRegister = 4,
	scAbs = 5,
	scUndef = 6,
	scUndefined = 6,
	scCdbLocal = 7,
	scBits = 8,
	scCdbSystem = 9,
	scDbx = 9,
	scRegImage = 10,
	scInfo = 11,
	scUserStruct = 12,
	scSData = 13,
	scSBss = 14,
	scRData = 15,
	scVar = 16,
	scCommon = 17,
	scSCommon = 18,
	scVarRegister = 19,
	scVariant = 20,
	scSUndefined = 21,
	scInit = 22,
	scBasedVar = 23,
	scXData = 24,
	scPData = 25,
	scFini = 26,
	scRConst = 27,
	scMax = 32,
};

#define XEACH_StorageClasse_enum()                                                                                     \
	X(scNil)                                                                                                             \
	X(scText)                                                                                                            \
	X(scData)                                                                                                            \
	X(scBss)                                                                                                             \
	X(scRegister)                                                                                                        \
	X(scAbs)                                                                                                             \
	X(scUndef)                                                                                                           \
	X(scUndefined)                                                                                                       \
	X(scCdbLocal)                                                                                                        \
	X(scBits)                                                                                                            \
	X(scCdbSystem)                                                                                                       \
	X(scDbx)                                                                                                             \
	X(scRegImage)                                                                                                        \
	X(scInfo)                                                                                                            \
	X(scUserStruct)                                                                                                      \
	X(scSData)                                                                                                           \
	X(scSBss)                                                                                                            \
	X(scRData)                                                                                                           \
	X(scVar)                                                                                                             \
	X(scCommon)                                                                                                          \
	X(scSCommon)                                                                                                         \
	X(scVarRegister)                                                                                                     \
	X(scVariant)                                                                                                         \
	X(scSUndefined)                                                                                                      \
	X(scInit)                                                                                                            \
	X(scBasedVar)                                                                                                        \
	X(scXData)                                                                                                           \
	X(scPData)                                                                                                           \
	X(scFini)                                                                                                            \
	X(scRConst)                                                                                                          \
	X(scMax)

enum elf_header_flags
{
	EF_MIPS_NOREORDER = 1,
	EF_MIPS_PIC = 2,
	EF_MIPS_CPIC = 4,
	EF_MIPS_MACH = 0x00FF0000,
	EF_MIPS_MACH_5900 = 0x00920000,
	EF_MIPS_ARCH_3 = 0x20000000,
	EF_MIPS_ARCH = 0xF0000000,
};

enum elf_program_header_flags
{
	PF_X = 1,
	PF_W = 2,
	PF_R = 4,
};

enum elf_section_header_flags
{
	SHF_WRITE = 1,
	SHF_ALLOC = 2,
	SHF_EXECINSTR = 4,
	SHF_MIPS_GPREL = 0x10000000,
};

enum anaarg_havearg_param
{
	ARG_HAVEARG_NONE = 0,
	ARG_HAVEARG_UNK1 = 1,
	ARG_HAVEARG_REQUIRED = 2,
	ARG_HAVEARG_UNK3 = 3,
	ARG_HAVEARG_UNK4 = 4,
};

enum elf_file_slot_type
{
	EFS_TYPE_NONE = 0,
	EFS_TYPE_ELF_HEADER = 1,
	EFS_TYPE_PROGRAM_HEADER_TABLE = 2,
	EFS_TYPE_PROGRAM_HEADER_ENTRY = 3,
	EFS_TYPE_SECTION_HEADER_TABLE = 4,
	EFS_TYPE_SECTION_DATA = 5,
	EFS_TYPE_END = 100,
};

enum srx_target_enum
{
	SRX_TARGET_IOP = 1,
	SRX_TARGET_EE = 2,
};

enum srx_program_header_type
{
	SRX_PH_TYPE_MOD = 1,
	SRX_PH_TYPE_TEXT = 2,
};

#endif
