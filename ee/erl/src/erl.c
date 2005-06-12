/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# The relocatable elf loader/linker.
*/

#define DEBUG 1
#define STANDALONE 1
#define FORCE_ALIGN 1

#undef DEBUG
#undef STANDALONE

/**** Note this code should compile on PC-side, mainly for direct check purposes though ****/

#ifdef DEBUG
#ifdef FULLDEBUG
#define dprintf(fmt, args...) printf("(%s:%s:%i): " fmt, __FILE__, __FUNCTION__, __LINE__, ## args)
#else
#define dprintf(fmt, args...) printf(fmt, ## args)
#endif
#define rprintf(fmt, args...) printf(fmt, ## args)
#else
#define dprintf(a...)
#define rprintf(a...)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <io.h>
#endif

#include <erl.h>

#include <hashtab.h>
#include <recycle.h>
#undef align
 

#ifdef _EE
#include <tamtypes.h>
#include <kernel.h>
#else
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
#endif

#ifdef DEBUG

static char * elf_classes[] = {
    "Invalid class",
    "32-bit objects",
    "64-bit objects",
};

static char * elf_encodings[] = {
    "Invalid encoding",
    "Little endian",
    "Big endian",
};

static char * elf_types[] = {
    "No file type",
    "Relocatable file",
    "Executable file",
    "Shared object file",
    "Core file",
};

static char * elf_machines[] = {
    "No machine",
    "AT&T WE 32100",
    "SPARC",
    "Intel Architecture",
    "Motorola 68000",
    "Motorola 88000",
    "Intel 80860",
    "MIPS RS3000 Big-Endian",
    "MIPS RS4000 Big-Endian",
};

static char * section_types[] = {
    "Null",
    "Progbits",
    "Symtab",
    "Strtab",
    "Rela",
    "Hash",
    "Dynamic",
    "Note",
    "Nobits",
    "Rel",
    "Shlib",
    "Dynsym",
};

static char * section_flags[] = {
    "---",
    "--W",
    "-R-",
    "-RW",
    "X--",
    "X-W",
    "XR-",
    "XRW",
};

static char * symbol_types[] = {
    "NoType",
    "Object",
    "Func",
    "Section",
    "File",
    "?? (5)",
    "?? (6)",
    "?? (7)",
    "?? (8)",
    "?? (9)",
    "?? (10)",
    "?? (11)",
    "?? (12)",
    "LoProc",
    "?? (14)",
    "HiProc",
};

static char * binding_types[] = {
    "Local",
    "Global",
    "Weak",
    "?? (3)",
    "?? (4)",
    "?? (5)",
    "?? (6)",
    "?? (7)",
    "?? (8)",
    "?? (9)",
    "?? (10)",
    "?? (11)",
    "?? (12)",
    "LoProc",
    "?? (14)",
    "HiProc",
};

static char * reloc_types[] = {
    "R_MIPS_NONE",
    "R_MIPS_16",
    "R_MIPS_32",
    "R_MIPS_REL32",
    "R_MIPS_26",
    "R_MIPS_HI16",
    "R_MIPS_LO16",
    "R_MIPS_GPREL16",
    "R_MIPS_LITERAL",
    "R_MIPS_GOT16",
    "R_MIPS_PC16",
    "R_MIPS_CALL16",
    "R_MIPS_GPREL32"
};

#endif

#define REL_TYPE 1
#define PROGBITS 1
#define NOBITS 8
#define REL 9
#define GLOBAL 1
#define WEAK 2
#define NOTYPE 0
#define OBJECT 1
#define FUNC 2
#define SECTION 3
#define R_MIPS_32 2
#define R_MIPS_26 4
#define R_MIPS_HI16 5
#define R_MIPS_LO16 6


/* These global names will not be 'exported' to the global space. */

static const char * local_names[] = {
    "_init",
    "_fini",
    "erl_id",
    "erl_dependancies",
    "erl_copyright",
    "erl_version",
    "_start",
    0
};


/* Structures mapped onto the loaded erl file. */

struct elf_header_t {
    union {
	u8 raw[16];
	struct e_ident_t {
	    u8 ei_magic[4];
	    u8 ei_class;
	    u8 ei_data;
	    u8 ei_version;
	} cook;
    } e_ident;
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u32 e_entry;
    u32 e_phoff;
    u32 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};

struct elf_section_t {
    u32 sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size;
    u32 sh_link, sh_info, sh_addralign, sh_entsize;
};

struct elf_symbol_t {
    u32 st_name, st_value, st_size;
    u8 st_info, st_other;
    u16 st_shndx;
};

struct elf_reloc_t {
    u32 r_offset, r_info;
};


/* Our internal structures. */

struct loosy_t {
    u8 * reloc;
    int type;
    struct loosy_t * next;
    struct erl_record_t * erl;
};

struct dependancy_t {
    struct erl_record_t * depender, * provider;
    struct dependancy_t * next, * prev;
};


/* And our global variables. */
static struct erl_record_t * erl_record_root = 0;

static htab * global_symbols = 0;
static htab * loosy_relocs = 0;

char _init_erl_prefix[256] = "";

static struct dependancy_t * dependancy_root = 0;


static u32 align(u32 x, int align) {
#ifdef FORCE_ALIGN
    if (align < 16)
	align = 16;
#endif

    align--;

    if (x & align) {
	x |= align;
	x++;
    }
    
    return x;
}

static struct loosy_t * create_loosy(struct erl_record_t * erl, u8 * reloc, int type) {
    struct loosy_t * r;
    
    if (!(r = (struct loosy_t *) malloc(sizeof(struct loosy_t))))
	return 0;

    r->reloc = reloc;
    r->type = type;
    r->next = 0;
    r->erl = erl;
    
    return r;
}

static void destroy_loosy(struct loosy_t * l) {
    free(l);
}

static void r_destroy_loosy(struct loosy_t * l) {
    if (!l)
	return;

    r_destroy_loosy(l->next);

    destroy_loosy(l);
}

static reroot * symbol_recycle = 0;

static struct symbol_t * create_symbol(struct erl_record_t * provider, u32 address) {
    struct symbol_t * r;
    
    if (!symbol_recycle)
	symbol_recycle = remkroot(sizeof(struct symbol_t));
    
    r = (struct symbol_t *) renew(symbol_recycle);
    
    r->provider = provider;
    r->address = address;
    
    return r;
}

static void destroy_symbol(struct symbol_t * s) {
    redel(symbol_recycle, s);
}

static struct erl_record_t * allocate_erl_record() {
    struct erl_record_t * r;

    if (!(r = (struct erl_record_t *) malloc(sizeof(struct erl_record_t))))
	return 0;
    
    r->bytes = NULL;
    r->symbols = hcreate(6);
    
    if ((r->next = erl_record_root))
	r->next->prev = r;
    r->prev = 0;
    erl_record_root = r;
    
    r->flags = 0;
    
    return r;
}

static void destroy_erl_record(struct erl_record_t * erl) {
    if (!erl)
	return;

    if (erl->bytes)
	free(erl->bytes);

    erl_flush_symbols(erl);

    if (erl->prev)
	erl->prev->next = erl->next;
    else
	erl_record_root = erl->next;

    if (erl->next)
	erl->next->prev = erl->prev;

    free(erl);
}

static int apply_reloc(u8 * reloc, int type, u32 addr) {
    u32 u_current_data = *((u32 *) reloc);
    s32 s_current_data = *((s32 *) reloc);
    switch (type) {
    case R_MIPS_32:
	*(u32 *)reloc = s_current_data + addr;
	break;
    case R_MIPS_26:
	*(u32 *)reloc = (u_current_data & 0xfc000000) | (((u_current_data & 0x03ffffff) + (addr >> 2)) & 0x3ffffff);
	break;
    case R_MIPS_HI16:
        *(u32 *)reloc = (u_current_data & 0xffff0000) | ((((s_current_data << 16) >> 16) + (addr >> 16) + ((addr & 0xffff) >= 0x8000 ? 1 : 0)) & 0xffff);
	break;
    case R_MIPS_LO16:
        *(u32 *)reloc = (u_current_data & 0xffff0000) | ((((s_current_data << 16) >> 16) + (addr & 0xffff)) & 0xffff);
	break;
    default:
	return -1;
    }
    dprintf("Changed data at %08X from %08X to %08X.\n", reloc, u_current_data, *(u32 *)reloc);
    return 0;
}

struct symbol_t * erl_find_local_symbol(const char * symbol, struct erl_record_t * erl) {
    if (!erl)
	return 0;
    if (hfind(erl->symbols, symbol, strlen(symbol)))
	return hstuff(erl->symbols);
    return 0;
}

static struct symbol_t * r_find_symbol(const char * symbol, struct erl_record_t * erl) {
    if (!erl)
	return 0;
    if (hfind(erl->symbols, symbol, strlen(symbol)))
	return hstuff(erl->symbols);
    return r_find_symbol(symbol, erl->next);
}

struct symbol_t * erl_find_symbol(const char * symbol) {
    if (global_symbols)
	if (hfind(global_symbols, symbol, strlen(symbol)))
	    return hstuff(global_symbols);
    return r_find_symbol(symbol, erl_record_root);
}

static struct dependancy_t * add_dependancy(struct erl_record_t * depender, struct erl_record_t * provider) {
    struct dependancy_t * d;
    
    if (depender == provider)
	return 0;
    
    if (!(d = (struct dependancy_t *) malloc(sizeof(struct dependancy_t))))
	return 0;
    
    d->depender = depender;
    d->provider = provider;
    
    if ((d->next = dependancy_root))
	d->next->prev = d;

    d->prev = 0;
    dependancy_root = d;
    
    return d;
}

static void destroy_dependancy(struct dependancy_t * d) {
    if (d->prev)
	d->prev->next = d->next;
    else
	dependancy_root = d->next;

    if (d->next)
	d->next->prev = d->prev;

    free(d);
}

static void r_destroy_dependancy_r(struct erl_record_t * erl, struct dependancy_t * d) {
    if (!d)
	return;

    r_destroy_dependancy_r(erl, d->next);
    
    if (erl == d->depender)
	destroy_dependancy(d);

    return;
}

static void destroy_dependancy_r(struct erl_record_t * erl) {
    r_destroy_dependancy_r(erl, dependancy_root);
}

static void add_loosy(struct erl_record_t * erl, u8 * reloc, int type, const char * symbol) {
    struct loosy_t * l;
    
    l = create_loosy(erl, reloc, type);
    
    if (!loosy_relocs)
	loosy_relocs = hcreate(6);

    if (!hadd(loosy_relocs, symbol, strlen(symbol), l)) {
	l->next = hstuff(loosy_relocs);
	hstuff(loosy_relocs) = l;
    } else {
	hkey(loosy_relocs) = strdup(symbol);
    }
}

static int fix_loosy(struct erl_record_t * provider, const char * symbol, u32 address) {
    struct loosy_t * l;
    int count = 0;
    
    if (!loosy_relocs)
	return count;
    
    if (hfind(loosy_relocs, symbol, strlen(symbol))) {
	for (l = hstuff(loosy_relocs); l; l = l->next) {
	    apply_reloc(l->reloc, l->type, address);
	    add_dependancy(l->erl, provider);
	    count++;
	}
	r_destroy_loosy(hstuff(loosy_relocs));
	free(hkey(loosy_relocs));
	hdel(loosy_relocs);
    }
    
    return count;
}

static int is_local(const char * symbol) {
    const char ** p;
    
    for (p = local_names; *p; p++)
	if (!strcmp(*p, symbol))
	    return 1;
    
    return 0;
}

static int add_symbol(struct erl_record_t * erl, const char * symbol, u32 address) {
    htab * symbols;

    if (erl) {
	symbols = erl->symbols;
    } else {
	if (!global_symbols)
	    global_symbols = hcreate(6);
	symbols = global_symbols;
    }
    
    if (!is_local(symbol) && erl_find_symbol(symbol))
	return -1;

    dprintf("Adding symbol %s at address %08X\n", symbol, address);

    if (fix_loosy(erl, symbol, address)) {
#ifdef _EE
        FlushCache(2);
	FlushCache(0);
#endif
    }
    
    hadd(symbols, strdup(symbol), strlen(symbol), create_symbol(erl, address));
    
    return 0;
}

int erl_add_global_symbol(const char * symbol, u32 address) {
    return add_symbol(0, symbol, address);
}

static int read_erl(int elf_handle, u8 * elf_mem, struct erl_record_t ** p_erl_record) {
    struct elf_header_t head;
    struct elf_section_t * sec = 0;
    struct elf_symbol_t * sym = 0;
    struct elf_reloc_t reloc;
    int i, j, erx_compressed;
    char * names = 0, * strtab_names = 0, * reloc_section = 0;
    int symtab = 0, strtab = 0, linked_strtab = 0;
    u8 * magic;
    u32 fullsize = 0;
    struct erl_record_t * erl_record = 0;
    struct symbol_t * s;
    
    *p_erl_record = 0;

#define free_and_return(code) if (!elf_mem) { \
    if (names) free(names); \
    if (strtab_names) free(strtab_names); \
    if (sec) free(sec); \
    if (sym) free(sym); \
    if ((code < 0) && erl_record) destroy_erl_record(erl_record); \
} \
return code

    if (!(erl_record = allocate_erl_record())) {
	dprintf("Memory allocation error.\n");
	free_and_return(-1);
    }

   // Reading the main ELF header.
    if (elf_mem) {
	memcpy(&head, elf_mem, sizeof(head));
    } else {
        lseek(elf_handle, 0, SEEK_SET);
        read(elf_handle, &head, sizeof(head));
    }
    
    magic = head.e_ident.cook.ei_magic;
    
    if ((magic[0] != 0x7f) || (magic[1] != 'E') || (magic[2] != 'L') || ((magic[3] != 'F') && (magic[3] != 'X'))) {
	dprintf("Not an ELF file.\n");
	free_and_return(-1);
    }
    
    erx_compressed = magic[3] == 'X';
    
    dprintf("ELF Class    : %s\n", elf_classes[head.e_ident.cook.ei_class]);
    dprintf("Data encoding: %s\n", elf_encodings[head.e_ident.cook.ei_data]);
    dprintf("Elf version  : %i\n", head.e_ident.cook.ei_version);
    if (head.e_type == 0xffff) {
	dprintf("Object type  : Processor specific (hi)\n");
    } else if (head.e_type == 0xff00) {
	dprintf("Object type  : Processor specific (lo)\n");
    } else {
	dprintf("Object type  : %s\n", elf_types[head.e_type]);
    }
    dprintf("Machine type : %s\n", elf_machines[head.e_machine]);
    dprintf("Object ver.  : %i\n", head.e_version);
    dprintf("Elf entry    : %08X\n", head.e_entry);
    dprintf("PH offset    : %08X\n", head.e_phoff);
    dprintf("SH offset    : %08X\n", head.e_shoff);
    dprintf("Flags        : %08X\n", head.e_flags);
    dprintf("Header size  : %04X\n", head.e_ehsize);
    dprintf("PH ent. size : %04X\n", head.e_phentsize);
    dprintf("PH number    : %04X\n", head.e_phnum);
    dprintf("SH ent. size : %04X\n", head.e_shentsize);
    dprintf("SH number    : %04X\n", head.e_shnum);
    dprintf("SH str index : %04X\n", head.e_shstrndx);
    
    if (head.e_type != REL_TYPE) {
	dprintf("File isn't a relocatable ELF file.\n");
	free_and_return(-1);
    }
    
   // Reading the section table.
    if (sizeof(struct elf_section_t) != head.e_shentsize) {
	dprintf("Inconsistancy in section table entries.\n");
	free_and_return(-1);
    }
    
    // **TODO** handle compession
    if (elf_mem) {
	sec = (struct elf_section_t *) (elf_mem + head.e_shoff);
    } else {
        if (!(sec = (struct elf_section_t *) malloc(sizeof(struct elf_section_t) * head.e_shnum))) {
    	    dprintf("Not enough memory.\n");
	    free_and_return(-1);
	}
	lseek(elf_handle, head.e_shoff, SEEK_SET);    
        read(elf_handle, sec, sizeof(struct elf_section_t) * head.e_shnum);
    }
    
   // Reading the section names's table.
    // **TODO** handle compession
    if (elf_mem) {
	names = (char *) (elf_mem + sec[head.e_shstrndx].sh_offset);
    } else {
	if (!(names = (char *) malloc(sec[head.e_shstrndx].sh_size))) {
	    dprintf("Not enough memory.\n");
	    free_and_return(-1);
	}
        lseek(elf_handle, sec[head.e_shstrndx].sh_offset, SEEK_SET);
	read(elf_handle, names, sec[head.e_shstrndx].sh_size);
    }


   // Parsing the sections, and displaying them at the same time.
    dprintf("##: type   flags offset   size       link info align entsize name\n");
    for (i = 1; i < head.e_shnum; i++) {
	if (!strcmp(names + sec[i].sh_name, ".symtab")) {
	    symtab = i;
	    linked_strtab = sec[i].sh_link;
    	} else if (!strcmp(names + sec[i].sh_name, ".strtab")) {
	    strtab = i;
	}
	
	if ((sec[i].sh_type == PROGBITS) || (sec[i].sh_type == NOBITS)) {
	   // Let's use this, it's not filled for relocatable objects.
	    fullsize = align(fullsize, sec[i].sh_addralign);
	    sec[i].sh_addr = fullsize;
	    fullsize += sec[i].sh_size;
	    dprintf("Section to load at %08X:\n", sec[i].sh_addr);
	}

	dprintf("%2i: ", i);
	if (sec[i].sh_type <= 0xff) {
	    rprintf("%-8s ", section_types[sec[i].sh_type]);
	} else if (sec[i].sh_type == 0x70000006) {
	    rprintf("Reginfo  ");
	} else {
	    rprintf("UNKNOW   ");
	}

	rprintf(  "%3s ", section_flags[sec[i].sh_flags & 7]);
	rprintf( "%08X ", sec[i].sh_offset);
	rprintf( "%08X ", sec[i].sh_size);
	rprintf(  "%5i ", sec[i].sh_link);
	rprintf(  "%5i ", sec[i].sh_info);
	rprintf(  "%5i ", sec[i].sh_addralign);
	rprintf("%5i   ", sec[i].sh_entsize);
	rprintf(  "%s\n", names + sec[i].sh_name);
    }
    
    if (symtab) {
	dprintf("Discovered symtab = %i\n", symtab);
    } else {
	dprintf("No symbol table.\n");
	free_and_return(-1);
    }
    
    if (strtab) {
	dprintf("Discovered strtab = %i\n", strtab);
    } else {
	dprintf("No string table.\n");
	free_and_return(-1);
    }
    
    if (strtab != linked_strtab) {
	dprintf("Warning, inconsistancy: strtab != symtab.sh_link (%i != %i)\n", strtab, linked_strtab);
	free_and_return(-1);
    }
    
    if (sizeof(struct elf_symbol_t) != sec[symtab].sh_entsize) {
	dprintf("Symbol entries not consistant.\n");
	free_and_return(-1);
    }
    
    dprintf("Computed needed size to load the erl file: %i\n", fullsize);
    
   // Loading progbits sections.
    erl_record->bytes = (u8 *) malloc(fullsize);
    erl_record->fullsize = fullsize;
    dprintf("Base address: %08X\n", erl_record->bytes);
    for (i = 1; i < head.e_shnum; i++) {
	switch (sec[i].sh_type) {
	case PROGBITS:
            // **TODO** handle compession
	    dprintf("Reading section %s at %08X.\n", names + sec[i].sh_name, erl_record->bytes + sec[i].sh_addr);
	    if (elf_mem) {
		memcpy(erl_record->bytes + sec[i].sh_addr, elf_mem + sec[i].sh_offset, sec[i].sh_size);
	    } else {
		lseek(elf_handle, sec[i].sh_offset, SEEK_SET);
		read(elf_handle, erl_record->bytes + sec[i].sh_addr, sec[i].sh_size);
	    }
	    break;
	case NOBITS:
	    dprintf("Zeroing section %s at %08X.\n", names + sec[i].sh_name, erl_record->bytes + sec[i].sh_addr);
	    memset(erl_record->bytes + sec[i].sh_addr, 0, sec[i].sh_size);
	    break;
	}
    }
    
    
   // Loading strtab.
    // **TODO** handle compession
    if (elf_mem) {
	strtab_names = elf_mem + sec[strtab].sh_offset;
    } else {
        if (!(strtab_names = (char *) malloc(sec[strtab].sh_size))) {
    	    dprintf("Not enough memory.\n");
	    free_and_return(-1);
	}
	lseek(elf_handle, sec[strtab].sh_offset, SEEK_SET);
	read(elf_handle, strtab_names, sec[strtab].sh_size);
    }
    
    
   // Loading symtab.
    // **TODO** handle compession
    if (elf_mem) {
	sym = (struct elf_symbol_t *) (elf_mem + sec[symtab].sh_offset);
    } else {
	if (!(sym = (struct elf_symbol_t *) malloc(sec[symtab].sh_size))) {
	    dprintf("Not enough memory.\n");
	    free_and_return(-1);
	}
        lseek(elf_handle, sec[symtab].sh_offset, SEEK_SET);
	read(elf_handle, sym, sec[symtab].sh_size);
    }
    
   // Parsing sections to find relocation sections.
    for (i = 0; i < head.e_shnum; i++) {
	if (sec[i].sh_type != REL)
	    continue;
	dprintf("Section %i (%s) contains relocations for section %i (%s):\n",
		i, names + sec[i].sh_name, sec[i].sh_info, names + sec[sec[i].sh_info].sh_name);
	
	if (sec[i].sh_entsize != sizeof(struct elf_reloc_t)) {
	    dprintf("Warning: inconsistancy in relocation table.\n");
	    free_and_return(-1);
	}
	
       // Loading relocation section.
        // **TODO** handle compession
        if (elf_mem) {
  	    reloc_section = elf_mem + sec[i].sh_offset;
        } else {
	    lseek(elf_handle, sec[i].sh_offset, SEEK_SET);
	    if (!(reloc_section = (char *) malloc(sec[i].sh_size))) {
	        dprintf("Not enough memory.\n");
	        free_and_return(-1);
  	    }
            lseek(elf_handle, sec[i].sh_offset, SEEK_SET);
	    read(elf_handle, reloc_section, sec[i].sh_size);
        }

       // We found one relocation section, let's parse it to relocate.
	dprintf("   Num: Offset   Type           Symbol\n");
	for (j = 0; j < (sec[i].sh_size / sec[i].sh_entsize); j++) {
	    int sym_n;
	    
	    reloc = *((struct elf_reloc_t *) (reloc_section + j * sec[i].sh_entsize));
	    
	    sym_n = reloc.r_info >> 8;
	    dprintf("%6i: %08X %-14s %3i: ", j, reloc.r_offset, reloc_types[reloc.r_info & 255], sym_n);
	    
	    switch(sym[sym_n].st_info & 15) {
	    case NOTYPE:
		rprintf("external symbol reloc to symbol %s\n", strtab_names + sym[sym_n].st_name);
		if (!(s = erl_find_symbol(strtab_names + sym[sym_n].st_name))) {
		    dprintf("Symbol not found, adding as loosy relocation.\n");
		    add_loosy(erl_record, erl_record->bytes + sec[sec[i].sh_info].sh_addr + reloc.r_offset, reloc.r_info & 255, strtab_names + sym[sym_n].st_name);
		} else {
		    dprintf("Found symbol at %08X, relocating.\n", s->address);
		    if (apply_reloc(erl_record->bytes + sec[sec[i].sh_info].sh_addr + reloc.r_offset, reloc.r_info & 255, s->address) < 0) {
			dprintf("Something went wrong in relocation.");
			free_and_return(-1);
		    }
		    add_dependancy(erl_record, s->provider);
		}
		break;
	    case SECTION:
		rprintf("internal section reloc to section %i (%s)\n", sym[sym_n].st_shndx, names + sec[sym[sym_n].st_shndx].sh_name);
		dprintf("Relocating at %08X.\n", erl_record->bytes + sec[sym[sym_n].st_shndx].sh_addr);
		if (apply_reloc(erl_record->bytes + sec[sec[i].sh_info].sh_addr + reloc.r_offset, reloc.r_info & 255, (u32) (erl_record->bytes + sec[sym[sym_n].st_shndx].sh_addr)) < 0) {
		    dprintf("Something went wrong in relocation.");
		    free_and_return(-1);
		}
		break;
	    case OBJECT:
	    case FUNC:
		rprintf("internal relocation to symbol %s\n", strtab_names + sym[sym_n].st_name);
		if ((s = erl_find_symbol(strtab_names + sym[sym_n].st_name))) {
		    dprintf("Symbol already exists at %08X. Let's use it instead.\n", s->address);
		    if (apply_reloc(erl_record->bytes + sec[sec[i].sh_info].sh_addr + reloc.r_offset, reloc.r_info & 255, s->address) < 0) {
			dprintf("Something went wrong in relocation.");
			free_and_return(-1);
		    }
		    add_dependancy(erl_record, s->provider);
		} else {
    		    dprintf("Relocating at %08X.\n", erl_record->bytes + sec[sym[sym_n].st_shndx].sh_addr + sym[sym_n].st_value);
		    if (apply_reloc(erl_record->bytes + sec[sec[i].sh_info].sh_addr + reloc.r_offset, reloc.r_info & 255, (u32) (erl_record->bytes + sec[sym[sym_n].st_shndx].sh_addr + sym[sym_n].st_value)) < 0) {
			dprintf("Something went wrong in relocation.");
			free_and_return(-1);
		    }
		}
		break;
	    default:
		rprintf("Unknown relocation. Bug inside.\n");
		free_and_return(-1);
	    }
	}
	if (!elf_mem)
	    free(reloc_section);
    }

    dprintf("   Num: Value    Size     Type    Bind      Ndx Name\n");
    for (i = 0; i < sec[symtab].sh_size / sec[symtab].sh_entsize; i++) {
	if (((sym[i].st_info >> 4) == GLOBAL) || ((sym[i].st_info >> 4) == WEAK)) {
	    if ((sym[i].st_info & 15) != NOTYPE) {
		dprintf("Export symbol:\n");
		if (add_symbol(erl_record, strtab_names + sym[i].st_name, ((u32)erl_record->bytes) + sec[sym[i].st_shndx].sh_addr + sym[i].st_value) < 0) {
		    dprintf("Symbol probably already exists, let's ignore that.\n");
//		    free_and_return(-1);
		}
	    }
	}

	dprintf("%6i: %08X %08X %-7s %-6s %6i %-10s : %s\n", i,
	  sym[i].st_value, sym[i].st_size, symbol_types[sym[i].st_info & 15],
	  binding_types[sym[i].st_info >> 4], sym[i].st_shndx,
	  sym[i].st_name ? strtab_names + sym[i].st_name : "(null)",
	  sym[i].st_shndx ? names + sec[sym[i].st_shndx].sh_name : "(null)");
    }

#ifdef _EE
    FlushCache(2);
    FlushCache(0);
#endif

    *p_erl_record = erl_record;

    free_and_return(0);
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

typedef int (*func_t)(void);
typedef int (*start_t)(int argc, char ** argv);

static struct erl_record_t * _init_load_erl_wrapper_from_file(const char * erl_id) {
    char tmpnam[256];
    strcpy(tmpnam, erl_id);
    strcat(tmpnam, ".erl");
    return _init_load_erl_from_file(tmpnam, erl_id);
}

erl_loader_t _init_load_erl = _init_load_erl_wrapper_from_file;

static struct erl_record_t * load_erl(const char * fname, u8 * elf_mem, int argc, char ** argv) {
    struct erl_record_t * r;
    struct symbol_t * s;
    int elf_handle = 0;
    
    dprintf("Reading ERL file.\n");
    
    if (fname) {
        if ((elf_handle = open(fname, O_RDONLY | O_BINARY)) < 0) {
    	    dprintf("Error operning erl file: %s\n", fname);
	    return 0;
	}
    }
        
    if (read_erl(elf_handle, elf_mem, &r) < 0) {
	dprintf("Error loading erl file.\n");
	if (fname) {
	    close(elf_handle);
	}
	return 0;
    }
    
    if (fname) {
	close(elf_handle);
    }
    
    if ((s = erl_find_local_symbol("erl_id", r))) {
	r->name = (char *) s->address;
    } else {
	r->name = 0;
    }
    
    dprintf("erl_id = %08X.\n", r->name);
    
    if ((s = erl_find_local_symbol("erl_dependancies", r))) {
	r->dependancies = (char **) s->address;
    } else {
	r->dependancies = 0;
    }
    
    dprintf("erl_dependancies = %08X.\n", r->dependancies);
    
    if (r->dependancies) {
	char ** d;
	for (d = r->dependancies; *d; d++) {
	    dprintf("Loading dependancy: %s.\n", *d);
	    _init_load_erl(*d);
	}
    }

    if ((s = erl_find_local_symbol("_init", r))) {
	dprintf("_init = %08X\n", s->address);
#ifdef _EE
        ((func_t)s->address)();
#endif
    }
    
    if ((s = erl_find_local_symbol("_start", r))) {
	int _start_ret;
	dprintf("_start = %08X\n", s->address);
#ifdef _EE
        if ((_start_ret = ((start_t)s->address)(argc, argv))) {
	    dprintf("Module's _start returned %i, unloading module.\n", _start_ret);
	    if (unload_erl(r))
		return 0;
	}
#endif
    }
    
    return r;
}

struct erl_record_t * _init_load_erl_from_file(const char * fname, const char * erl_id) {
    char tfname[1024];
    struct erl_record_t * r;
    char * argv[2];
    
    if (erl_id)
        if ((r = find_erl(erl_id)))
    	    return r;
    
    argv[0] = erl_id;
    argv[1] = 0;
    
    strcpy(tfname, _init_erl_prefix);
    strcat(tfname, fname);
    
    return load_erl_from_file(tfname, 1, argv);
}

struct erl_record_t * load_erl_from_file(const char * fname, int argc, char ** argv) {
    return load_erl(fname, 0, argc, argv);
}

struct erl_record_t * load_erl_from_mem(u8 * mem, int argc, char ** argv) {
    return load_erl(0, mem, argc, argv);
}

void r_unload_dependancies(char ** d) {
    struct erl_record_t * erl;
    if (!(*d))
	return;

    r_unload_dependancies(d + 1);
    
    if ((erl = find_erl(*d)))
	unload_erl(erl);
}

int unload_erl(struct erl_record_t * erl) {
    struct symbol_t * s;
    struct dependancy_t * p;
    
    dprintf("Unloading module %s.\n", erl->name ? erl->name : "(noname)");
    
    if ((erl->flags) & ERL_FLAG_STICKY) {
	dprintf("Module is sticky, won't unload.\n");
	return 0;
    }
    
    for (p = dependancy_root; p; p = p->next) {
	if (p->provider == erl) {
	    dprintf("Other modules depend on it, won't unload.\n");
	    return 0;
	}
    }
    
    if ((s = erl_find_local_symbol("_fini", erl))) {
	dprintf("_fini = %08X\n", s->address);
#ifdef _EE
        ((func_t)s->address)();
#endif
    }
    
    if (erl->dependancies)
	r_unload_dependancies(erl->dependancies);

    erl_flush_symbols(erl);
    
    destroy_dependancy_r(erl);
    
    destroy_erl_record(erl);
    
    return 1;
}

struct erl_record_t * erl_resolve(u32 address) {
    struct erl_record_t * r;
    
    for (r = erl_record_root; r; r = r->next) {
	u32 r_ptr = (u32) r->bytes;
	if ((address >= r_ptr) && (address < (r_ptr + r->fullsize)))
	    return r;
    }
    
    return 0;
}

struct erl_record_t * find_erl(const char * name) {
    struct erl_record_t * r;
    
    for (r = erl_record_root; r; r = r->next) {
	if (r->name)
	    if (!strcmp(name, r->name))
		return r;
    }
    
    return 0;
}

void erl_flush_symbols(struct erl_record_t * erl) {
    if (!erl->symbols)
	return;

    if (hfirst(erl->symbols)) do {
	destroy_symbol((struct symbol_t *) hstuff(erl->symbols));
	free(hkey(erl->symbols));
	hdel(erl->symbols);
    } while (hcount(erl->symbols));
    
    hdestroy(erl->symbols);
    
    erl->symbols = 0;
}

#ifdef STANDALONE

int main(int argc, char ** argv) {
    struct erl_record_t * erl;
    char * fname;

    erl_add_global_symbol("printf", (u32) printf);
    
    if (argc == 2) {
	fname = argv[1];
    } else {
	fname = "host:hello-erl.erl";
    }
    
    if (!(erl = load_erl_from_file(fname))) {
	dprintf("Error while loading erl file.\n");
	return -1;
    }
    
    return 0;
}

#endif
