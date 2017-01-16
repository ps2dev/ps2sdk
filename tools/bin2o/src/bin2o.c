/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Convets any file into a PS2's .o file.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

static int alignment = 16;
static int have_size = 1;
static int have_irx = 0;
static short int put_payload_in_SDATA = 0, put_labels_in_SDATA = 0;
static int SDATA_size_limit = 0;

static u32 LE32(u32 b) {
    u32 t = 0x12345678;
    if (*((unsigned char*)(&t)) == 0x78) {
	return b;
    } else {
	return ((b & 0xff000000) >> 24) |
	       ((b & 0x00ff0000) >> 8 ) |
	       ((b & 0x0000ff00) << 8 ) |
	       ((b & 0x000000ff) << 24);
    }
}

static u16 LE16(u16 b) {
    u32 t = 0x12345678;
    if (*((unsigned char*)(&t)) == 0x78) {
	return b;
    } else {
	return ((b & 0xff00) >> 8) |
	       ((b & 0x00ff) << 8);
    }
}

static unsigned char elf_header[] = {
    0x7f,  'E',  'L',  'F', 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // e_ident
    0x01, 0x00,                                     // e_type (relocatable)
    0x08, 0x00,                                     // e_machine (mips)
    0x01, 0x00, 0x00, 0x00,                         // e_version
    0x00, 0x00, 0x00, 0x00,                         // e_entry
    0x00, 0x00, 0x00, 0x00,                         // e_phoff
    0x34, 0x00, 0x00, 0x00,                         // e_shoff
    0x01, 0x40, 0x92, 0x20,                         // e_flags
    0x34, 0x00,                                     // e_ehsize
    0x00, 0x00,                                     // e_phentsize
    0x00, 0x00,                                     // e_phnum
    0x28, 0x00,                                     // e_shentsize
    0x05, 0x00,                                     // e_shnum
    0x01, 0x00,                                     // e_shstrndx
};

// 0 = NULL
// 1 = .shstrtab
// 2 = .symtab
// 3 = .strtab
// 4 = .data

struct elf_section_t {
    u32 sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size;
    u32 sh_link, sh_info, sh_addralign, sh_entsize;
};

struct elf_symbol_t {
    u32 st_name, st_value, st_size;
    u8 st_info, st_other;
    u16 st_shndx;
};

//                 0 0000000001 11111111 12222222 22233
//                 0 1234567890 12345678 90123456 78901
static const char shstrtab[] = "\0.shstrtab\0.symtab\0.strtab\0.data";
static const char shstrtab_sdata[] = "\0.shstrtab\0.symtab\0.strtab\0.sdata\0.data";

static void create_elf(FILE * dest, const unsigned char * source, u32 size, const char * label) {
    int i, l_size;
    struct elf_section_t section;
    struct elf_symbol_t symbol;
    u32 strtab_size;
    char strtab[512];
    u32 data_size[4];
    unsigned int shrtabSectionNameLength, NumSections;
    const char *pshstrtab_ptr;

    if (have_irx) {
	elf_header[36] = 1;
	elf_header[37] = 0;
	elf_header[38] = 0;
	elf_header[39] = 0;
    }

    NumSections=(put_payload_in_SDATA || put_labels_in_SDATA)?6:5;
    elf_header[sizeof(elf_header)-4]=NumSections;

    for (i = 0; i < sizeof(elf_header); i++) {
	fputc(elf_header[i], dest);
    }

    l_size = strlen(label);

    strtab[0] = 0;
    strcpy(strtab + 1, label);
    strcat(strtab + 1, "_start");
    strcpy(strtab + 1 + l_size + 7, label);
    strcat(strtab + 1 + l_size + 7, "_end");
    if (have_size) {
        strtab_size = (l_size * 3 + 1 + 7 + 5 + 6);
        strcpy(strtab + 1 + l_size + 7 + l_size + 5, label);
        strcat(strtab + 1 + l_size + 7 + l_size + 5, "_size");
    } else {
        strtab_size = (l_size * 2 + 1 + 7 + 5);
    }

    // section 0 (NULL)
    section.sh_name = section.sh_type = section.sh_flags = section.sh_addr = section.sh_offset = section.sh_size =
    section.sh_link = section.sh_info = section.sh_addralign = section.sh_entsize = 0;

    fwrite(&section, 1, sizeof(section), dest);

    // section 1 (.shstrtab)
    if(put_payload_in_SDATA || put_labels_in_SDATA){
	pshstrtab_ptr=shstrtab_sdata;
	shrtabSectionNameLength=sizeof(shstrtab_sdata);
    }else{
	pshstrtab_ptr=shstrtab;
	shrtabSectionNameLength=sizeof(shstrtab);
    }
    section.sh_name = LE32(1);
    section.sh_type = LE32(3); // STRTAB
    section.sh_flags = 0;
    section.sh_addr = 0;
    section.sh_offset = LE32(40 * NumSections + 0x34);
    section.sh_size = LE32(shrtabSectionNameLength);
    section.sh_link = 0;
    section.sh_info = 0;
    section.sh_addralign = LE32(1);
    section.sh_entsize = 0;

    fwrite(&section, 1, sizeof(section), dest);


    // section 2 (.symtab)
    section.sh_name = LE32(11);
    section.sh_type = LE32(2); // SYMTAB
    section.sh_flags = 0;
    section.sh_addr = 0;
    section.sh_offset = LE32(40 * NumSections + 0x34 + shrtabSectionNameLength);
    section.sh_size = LE32(have_size ? 0x30 : 0x20);
    section.sh_link = LE32(3);
    section.sh_info = 0;
    section.sh_addralign = LE32(4);
    section.sh_entsize = LE32(0x10);

    fwrite(&section, 1, sizeof(section), dest);

    // section 3 (.strtab)
    section.sh_name = LE32(19);
    section.sh_type = LE32(3); // STRTAB
    section.sh_flags = 0;
    section.sh_addr = 0;
    section.sh_offset = LE32(40 * NumSections + 0x34 + shrtabSectionNameLength + (have_size ? 0x30 : 0x20));
    section.sh_size = LE32(strtab_size);
    section.sh_link = 0;
    section.sh_info = 0;
    section.sh_addralign = LE32(1);
    section.sh_entsize = 0;

    fwrite(&section, 1, sizeof(section), dest);

    if(put_payload_in_SDATA || put_labels_in_SDATA){
	// section 4 (.sdata)
	section.sh_name = LE32(27);
	section.sh_type = LE32(1); // PROGBITS
	section.sh_flags = LE32(0x10000003); // SHF_MIPS_GPREL + write + alloc
	section.sh_addr = 0;
	section.sh_offset = LE32(40 * NumSections + 0x34 + shrtabSectionNameLength + (have_size ? 0x30 : 0x20) + strtab_size);
	section.sh_size = LE32(((put_payload_in_SDATA?size:0) + (put_labels_in_SDATA?have_size*16:0)));
	section.sh_link = 0;
	section.sh_addralign = LE32(alignment);
	section.sh_entsize = 0;

	fwrite(&section, 1, sizeof(section), dest);
    }

    // section 4/5 (.data)
    section.sh_name = LE32((put_payload_in_SDATA||put_labels_in_SDATA)?35:27);
    section.sh_type = LE32(1); // PROGBITS
    section.sh_flags = LE32(3); // Write + Alloc
    section.sh_addr = 0;
    if(!put_payload_in_SDATA || !put_labels_in_SDATA){
	section.sh_offset = LE32(40 * NumSections + 0x34 + shrtabSectionNameLength + (have_size ? 0x30 : 0x20) + strtab_size + ((put_payload_in_SDATA?size:0) + (put_labels_in_SDATA?have_size*16:0)));
	section.sh_size = LE32(((put_payload_in_SDATA?0:size) + (put_labels_in_SDATA?0:have_size*16)));
    }
    else{
	section.sh_offset = 0;
	section.sh_size = 0;
    }
    section.sh_link = 0;
    section.sh_addralign = LE32(alignment);
    section.sh_entsize = 0;

    fwrite(&section, 1, sizeof(section), dest);

    fwrite(pshstrtab_ptr, 1, shrtabSectionNameLength, dest);

    symbol.st_name = LE32(1);
    symbol.st_value = LE32((have_size&&put_payload_in_SDATA==put_labels_in_SDATA)?16:0);	//If the size label and the payload exist in the same section, the size label will exist before the payload.
    symbol.st_shndx = LE16((put_labels_in_SDATA&&!put_payload_in_SDATA)?5:4);
    symbol.st_size = LE32(size);
    symbol.st_info = 0x11;
    symbol.st_other = 0;
    fwrite(&symbol, 1, sizeof(symbol), dest);

    symbol.st_name = LE32(l_size + 1 + 7);
    symbol.st_value = LE32(size + have_size * 16);
    symbol.st_size = 0;
    symbol.st_info = 0x11;
    symbol.st_other = 0;
    symbol.st_shndx = LE16(4);
    fwrite(&symbol, 1, sizeof(symbol), dest);

    if (have_size) {
        symbol.st_name = LE32(l_size * 2 + 1 + 7 + 5);
        symbol.st_value = 0;
        symbol.st_size = LE32(4);
        symbol.st_info = 0x11;
        symbol.st_other = 0;
       	symbol.st_shndx = LE16(4);
        fwrite(&symbol, 1, sizeof(symbol), dest);
    }

    fwrite(strtab, 1, strtab_size, dest);

    if (have_size) {
        data_size[0] = LE32(size);
        data_size[1] = 0;
        data_size[2] = 0;
        data_size[3] = 0;
        fwrite(data_size, 4, 4, dest);
    }
    fwrite(source, 1, size, dest);
}

static void usage(void) {
    printf("bin2o - Converts a binary file into a .o file.\n"
           "Usage: bin2o [-a XX] [-n] [-i] [-b XX] [-e XX] [-s XX] [-GXX] infile outfile label\n"
	   "  -i    - create an iop-compatible .o file.\n"
	   "  -n    - don't add label_size symbol.\n"
	   "  -a XX - set .data section alignment to XX.\n"
	   "          (has to be a power of two).\n"
	   "  -b XX - start reading file at this offset.\n"
	   "  -e XX - stop reading file at this offset.\n"
	   "  -s XX - force output data to be of this size.\n"
	   "  -GXX  - Puts data smaller than XX bytes within the sdata section\n"
	   "          (same as the -G option of GCC).\n"
	   "\n"
	  );
}

int main(int argc, char *argv[])
{
    u32 fd_size, start = 0, end = 0xffffffff, size = 0xffffffff;
    unsigned char * buffer;
    FILE * source, * dest;
    char * f_source = 0, * f_dest = 0, * f_label = 0;
    int i;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'a':
		i++;
		if (!argv[i]) {
		    usage();
		    printf("-a requires an argument.\n");
		    return 1;
		}
		if (argv[i][0] == '-') {
		    usage();
		    printf("-a requires an argument.\n");
		    return 1;
		}
		alignment = atoi(argv[i]);
		if ((alignment - 1) & alignment) {
		    printf("Error: alignment must be a power of 2.\n");
		    return 1;
		}
		break;
	    case 'n':
		have_size = 0;
		break;
	    case 'i':
		have_irx = 1;
		break;
	    case 'G':
		SDATA_size_limit = strtoul(&argv[i][2], NULL, 0);
		break;
	    case 'b':
		i++;
		if (!argv[i]) {
		    usage();
		    printf("-b requires an argument.\n");
		    return 1;
		}
		if (argv[i][0] == '-') {
		    usage();
		    printf("-b requires an argument.\n");
		    return 1;
		}
		start = atoi(argv[i]);
		break;
	    case 'e':
		i++;
		if (!argv[i]) {
		    usage();
		    printf("-e requires an argument.\n");
		    return 1;
		}
		if (argv[i][0] == '-') {
		    usage();
		    printf("-e requires an argument.\n");
		    return 1;
		}
		end = atoi(argv[i]);
		break;
	    case 's':
		i++;
		if (!argv[i]) {
		    usage();
		    printf("-s requires an argument.\n");
		    return 1;
		}
		if (argv[i][0] == '-') {
		    usage();
		    printf("-s requires an argument.\n");
		    return 1;
		}
		size = atoi(argv[i]);
		break;
	    default:
		usage();
		printf("Unknow option: %s.\n", argv[i]);
		return 1;
	    }
	} else {
	    if (!f_source) {
		f_source = argv[i];
	    } else if (!f_dest) {
		f_dest = argv[i];
	    } else if (!f_label) {
		f_label = argv[i];
	    } else {
		usage();
		printf("Too many arguments.\n");
		return 1;
	    }
	}
    }

    if (!f_source || !f_dest || !f_label) {
	usage();
	printf("Not enough arguments.\n");
	return 1;
    }

    if (!(source = fopen(f_source, "rb"))) {
	printf("Error opening %s for reading.\n", f_source);
	return 1;
    }

    fseek(source, 0, SEEK_END);
    fd_size = ftell(source);
    fseek(source, start, SEEK_SET);

    if (fd_size < end)
	end = fd_size;

    if (end < (size - start))
	size = end - start;

    if(SDATA_size_limit>0){
	put_payload_in_SDATA=(SDATA_size_limit>=size)?1:0;
	put_labels_in_SDATA=(have_size && SDATA_size_limit>=16)?1:0;
    }
    else{
	put_payload_in_SDATA = put_labels_in_SDATA = 0;
    }

    buffer = malloc(size);
    if (buffer == NULL) {
	printf("Failed to allocate memory.\n");
	return 1;
    }

    if (fread(buffer, 1, size, source) != size) {
	printf("Failed to read file.\n");
	return 1;
    }
    fclose(source);

    if (!(dest = fopen(f_dest, "wb+"))) {
	printf("Failed to open/create %s.\n", f_dest);
	return 1;
    }

    create_elf(dest, buffer, size, f_label);

    fclose(dest);
    free(buffer);

    return 0;
}
