/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2022, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

#include "types.h"
#include "elftypes.h"
#include "irxtypes.h"

/* Arrangement of ELF file after stripping
 *
 * ELF Header - 52 bytes
 * Program Headers
 * .text data
 * .data data
 * Section Headers
 * Relocation data
 * Section Header String Table
 *
 * When stripping the sections remove anything which isn't an allocated section or a relocation section.
 * The section string section we will rebuild.
 */

static const char *g_outfile;
static const char *g_infile;
static unsigned char *g_elfdata = NULL;
static struct ElfHeader g_elfhead = {0};
static struct ElfSection *g_elfsections = NULL;
static struct ElfSection *g_iopmod = NULL;
static int g_out_sects = 2;
static int g_alloc_size = 0;
static int g_mem_size = 0;
static int g_reloc_size = 0;
static int g_str_size = 1;

/* Base addresses in the Elf */
static int g_phbase = 0;
static int g_iopmodbase = 0;
static int g_allocbase = 0;
static int g_shbase = 0;
static int g_relocbase = 0;
static int g_shstrbase = 0;

/* Specifies that the current usage is to print additional debugging information */
static int g_verbose = 0;

static struct option arg_opts[] =
{
	{"verbose", no_argument, NULL, 'v'},
	{ NULL, 0, NULL, 0 }
};

/* Process the arguments */
int process_args(int argc, char **argv)
{
	int ch;

	g_outfile = NULL;
	g_infile = NULL;

	ch = getopt_long(argc, argv, "v", arg_opts, NULL);
	while(ch != -1)
	{
		switch(ch)
		{
			case 'v' : g_verbose = 1;
					   break;
			default  : break;
		};

		ch = getopt_long(argc, argv, "v", arg_opts, NULL);
	}

	argc -= optind;
	argv += optind;

	if(argc < 2)
	{
		return 0;
	}

	g_infile = argv[0];
	g_outfile = argv[1];

	if(g_verbose)
	{
		fprintf(stderr, "Loading %s, outputting to %s\n", g_infile, g_outfile);
	}

	return 1;
}

void print_help(void)
{
	fprintf(stderr, "Usage: ps2-irxgen [-v] infile.elf outfile.irx\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "-v, --verbose           : Verbose output\n");
}

unsigned char *load_file(const char *file)
{
	unsigned char *data = NULL;

	do
	{
		FILE *fp;

		fp = fopen(file, "rb");
		if(fp != NULL)
		{
			unsigned int size;

			(void) fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			rewind(fp);

			if(size < sizeof(Elf32_Ehdr))
			{
				fprintf(stderr, "Error, invalid file size\n");
				fclose(fp);
				break;
			}

			data = (unsigned char *) malloc(size);
			if(data == NULL)
			{
				fprintf(stderr, "Error, could not allocate memory for ELF\n");
				fclose(fp);
				break;
			}

			(void) fread(data, 1, size, fp);
			fclose(fp);
		}
		else
		{
			fprintf(stderr, "Error, could not find file %s\n", file);
		}
	}
	while(0);

	return data;
}

/* Validate the ELF header */
int validate_header(unsigned char *data)
{
	Elf32_Ehdr *head;
	int ret = 0;

	head = (Elf32_Ehdr*) data;

	do
	{
		/* Read in the header structure */
		g_elfhead.iMagic = LW(head->e_magic);
		g_elfhead.iClass = head->e_class;
		g_elfhead.iData = head->e_data;
		g_elfhead.iIdver = head->e_idver;
		g_elfhead.iType = LH(head->e_type);
		g_elfhead.iMachine = LH(head->e_machine);
		g_elfhead.iVersion = LW(head->e_version);
		g_elfhead.iEntry = LW(head->e_entry);
		g_elfhead.iPhoff = LW(head->e_phoff);
		g_elfhead.iShoff = LW(head->e_shoff);
		g_elfhead.iFlags = LW(head->e_flags);
		g_elfhead.iEhsize = LH(head->e_ehsize);
		g_elfhead.iPhentsize = LH(head->e_phentsize);
		g_elfhead.iPhnum = LH(head->e_phnum);
		g_elfhead.iShentsize = LH(head->e_shentsize);
		g_elfhead.iShnum = LH(head->e_shnum);
		g_elfhead.iShstrndx = LH(head->e_shstrndx);

		if(g_verbose)
		{
			fprintf(stderr, "Magic %08X, Class %02X, Data %02X, Idver %02X\n", g_elfhead.iMagic,
					g_elfhead.iClass, g_elfhead.iData, g_elfhead.iIdver);
			fprintf(stderr, "Type %04X, Machine %04X, Version %08X, Entry %08X\n", g_elfhead.iType,
					g_elfhead.iMachine, g_elfhead.iVersion, g_elfhead.iEntry);
			fprintf(stderr, "Phoff %08X, Shoff %08X, Flags %08X, Ehsize %08X\n", g_elfhead.iPhoff,
					g_elfhead.iShoff, g_elfhead.iFlags, g_elfhead.iEhsize);
			fprintf(stderr, "Phentsize %04X, Phnum %04X\n", g_elfhead.iPhentsize, g_elfhead.iPhnum);
			fprintf(stderr, "Shentsize %04X, Shnum %08X, Shstrndx %04X\n", g_elfhead.iShentsize,
					g_elfhead.iShnum, g_elfhead.iShstrndx);
		}

		if(g_elfhead.iMagic != ELF_MAGIC)
		{
			fprintf(stderr, "Error, invalid magic in the header\n");
			break;
		}

		if((g_elfhead.iType != ELF_EXEC_TYPE) && (g_elfhead.iType != ELF_IRX_TYPE))
		{
			fprintf(stderr, "Error, not EXEC type elf\n");
			break;
		}

		if(g_elfhead.iMachine != ELF_MACHINE_MIPS)
		{
			fprintf(stderr, "Error, not MIPS type ELF\n");
			break;
		}

		if(g_elfhead.iShnum < g_elfhead.iShstrndx)
		{
			fprintf(stderr, "Error, number of headers is less than section string index\n");
			break;
		}

		ret = 1;
	}
	while(0);

	return ret;
}

/* Load sections into ram */
int load_sections(unsigned char *data)
{
	int ret = 0;

	if(g_elfhead.iShnum > 0)
	{
		do
		{
			int i;
			unsigned int load_addr = 0xFFFFFFFF;
			int found_rel = 0;

			g_elfsections = (struct ElfSection *) malloc(sizeof(struct ElfSection) * g_elfhead.iShnum);
			if(g_elfsections == NULL)
			{
				fprintf(stderr, "Error, could not allocate memory for sections\n");
				break;
			}

			memset(g_elfsections, 0, sizeof(struct ElfSection) * g_elfhead.iShnum);

			for(i = 0; i < g_elfhead.iShnum; i++)
			{
				Elf32_Shdr *sect;

				sect = (Elf32_Shdr *) (g_elfdata + g_elfhead.iShoff + (i * g_elfhead.iShentsize));

				g_elfsections[i].iName = LW(sect->sh_name);
				g_elfsections[i].iType = LW(sect->sh_type);
				g_elfsections[i].iAddr = LW(sect->sh_addr);
				g_elfsections[i].iFlags = LW(sect->sh_flags);
				g_elfsections[i].iOffset = LW(sect->sh_offset);
				g_elfsections[i].iSize = LW(sect->sh_size);
				g_elfsections[i].iLink = LW(sect->sh_link);
				g_elfsections[i].iInfo = LW(sect->sh_info);
				g_elfsections[i].iAddralign = LW(sect->sh_addralign);
				g_elfsections[i].iEntsize = LW(sect->sh_entsize);
				g_elfsections[i].iIndex = i;

				if(g_elfsections[i].iOffset != 0)
				{
					g_elfsections[i].pData = g_elfdata + g_elfsections[i].iOffset;
				}

				if(g_elfsections[i].iFlags & SHF_ALLOC)
				{
					g_elfsections[i].blOutput = 1;
					if(g_elfsections[i].iAddr < load_addr)
					{
						load_addr = g_elfsections[i].iAddr;
					}
				}
			}

			/* Okay so we have loaded all the sections, lets find relocations and fix up the names */
			for(i = 0; i < g_elfhead.iShnum; i++)
			{
				if(((g_elfsections[i].iType == SHT_REL))
						&& (g_elfsections[g_elfsections[i].iInfo].iFlags & SHF_ALLOC))
				{
					g_elfsections[i].pRef = &g_elfsections[g_elfsections[i].iInfo];
					found_rel = 1;
					g_elfsections[i].blOutput = 1;
				}

				strcpy(g_elfsections[i].szName, (char *) (g_elfsections[g_elfhead.iShstrndx].pData + g_elfsections[i].iName));
				if(strcmp(g_elfsections[i].szName, PS2_MODULE_INFO_NAME) == 0)
				{
					g_elfsections[i].blOutput = 1;
					g_iopmod = &g_elfsections[i];
					g_iopmod->iType = SHT_LOPROC | SHT_LOPROC_IOPMOD;
				}
			}

			if(g_verbose)
			{
				for(i = 0; i < g_elfhead.iShnum; i++)
				{
					fprintf(stderr, "\nSection %d: %s\n", i, g_elfsections[i].szName);
					fprintf(stderr, "Name %08X, Type %08X, Flags %08X, Addr %08X\n",
							g_elfsections[i].iName, g_elfsections[i].iType,
							g_elfsections[i].iFlags, g_elfsections[i].iAddr);
					fprintf(stderr, "Offset %08X, Size %08X, Link %08X, Info %08X\n",
							g_elfsections[i].iOffset, g_elfsections[i].iSize,
							g_elfsections[i].iLink, g_elfsections[i].iInfo);
					fprintf(stderr, "Addralign %08X, Entsize %08X pData %p\n",
							g_elfsections[i].iAddralign, g_elfsections[i].iEntsize,
							g_elfsections[i].pData);
				}

				fprintf(stderr, "ELF Load Base address %08X\n", load_addr);
			}

			if(!found_rel)
			{
				fprintf(stderr, "Error, found no relocation sections\n");
				break;
			}

			if(g_iopmod == NULL)
			{
				fprintf(stderr, "Error, found no iopmod sections\n");
				break;
			}

			if(load_addr != 0)
			{
				fprintf(stderr, "Error, ELF not loaded to address 0 (%08X)\n", load_addr);
				break;
			}

			ret = 1;
		}
		while(0);
	}
	else
	{
		fprintf(stderr, "Error, no sections in the ELF\n");
	}

	return ret;
}

int remove_weak_relocs(struct ElfSection *pReloc, struct ElfSection *pSymbol, struct ElfSection *pString)
{
	unsigned int iCount;
	unsigned int iMaxSymbol;
	void *pNewRel = NULL;
	Elf32_Rel *pInRel;
	Elf32_Rel *pOutRel;
	Elf32_Sym *pSymData = (Elf32_Sym *) pSymbol->pData;
	char *pStrData = NULL;
	unsigned int iOutput;
	unsigned int i;

	if(pString != NULL)
	{
		pStrData = (char *) pString->pData;
	}

	iMaxSymbol = pSymbol->iSize / sizeof(Elf32_Sym);
	iCount = pReloc->iSize / sizeof(Elf32_Rel);

	pNewRel = malloc(pReloc->iSize);
	if(pNewRel == NULL)
	{
		return 0;
	}
	pOutRel = (Elf32_Rel *) pNewRel;
	pInRel = (Elf32_Rel *) pReloc->pData;
	iOutput = 0;

	if(g_verbose)
	{
		fprintf(stderr, "[%s] Processing %u relocations, %u symbols\n", pReloc->szName, iCount, iMaxSymbol);
	}

	for(i = 0; i < iCount; i++)
	{
		unsigned int iSymbol;

		iSymbol = ELF32_R_SYM(LW(pInRel->r_info));
		if(g_verbose)
		{
			fprintf(stderr, "Relocation %u - Symbol %x\n", iOutput, iSymbol);
		}

		if(iSymbol >= iMaxSymbol)
		{
			fprintf(stderr, "Warning: Ignoring relocation as cannot find matching symbol\n");
		}
		else
		{
			if(g_verbose)
			{
				if(pStrData != NULL)
				{
					fprintf(stderr, "Symbol %u - Name %s info %x ndx %x\n", iSymbol, &pStrData[pSymData[iSymbol].st_name],
							pSymData[iSymbol].st_info, pSymData[iSymbol].st_shndx);
				}
				else
				{
					fprintf(stderr, "Symbol %u - Name %u info %x ndx %x\n", iSymbol, pSymData[iSymbol].st_name,
							pSymData[iSymbol].st_info, pSymData[iSymbol].st_shndx);
				}
			}

			/* We are keeping this relocation, copy it across */
			*pOutRel = *pInRel;
			pOutRel++;
			iOutput++;
		}

		pInRel++;
	}

	/* If we deleted some relocations */
	if(iOutput < iCount)
	{
		uint32_t iSize;

		iSize = iOutput * sizeof(Elf32_Rel);
		if(g_verbose)
		{
			fprintf(stderr, "Old relocation size %u, new %u\n", pReloc->iSize, iSize);
		}
		pReloc->iSize = iSize;
		/* If size is zero then delete this section */
		if(iSize == 0)
		{
			pReloc->blOutput = 0;
		}
		else
		{
			/* Copy across the new relocation data */
			memcpy(pReloc->pData, pNewRel, pReloc->iSize);
		}
	}

	free(pNewRel);

	return 1;
}

/* Let's remove the weak relocations from the list */
int process_relocs(void)
{
	int i;

	for(i = 0; i < g_elfhead.iShnum; i++)
	{
		if((g_elfsections[i].blOutput) && (g_elfsections[i].iType == SHT_REL))
		{
			struct ElfSection *pReloc;

			pReloc = &g_elfsections[i];
			if((pReloc->iLink < g_elfhead.iShnum) && (g_elfsections[pReloc->iLink].iType == SHT_SYMTAB))
			{
				struct ElfSection *pStrings = NULL;
				struct ElfSection *pSymbols;

				pSymbols = &g_elfsections[pReloc->iLink];
				if((pSymbols->iLink < g_elfhead.iShnum) && (g_elfsections[pSymbols->iLink].iType == SHT_STRTAB))
				{
					pStrings = &g_elfsections[pSymbols->iLink];
				}

				if(!remove_weak_relocs(pReloc, pSymbols, pStrings))
				{
					return 0;
				}
			}
			else
			{
				if(g_verbose)
				{
					fprintf(stderr, "Ignoring relocation section %d, invalid link number\n", i);
				}
			}
		}
	}

	return 1;
}

/* Reindex the sections we are keeping */
void reindex_sections(void)
{
	int i;
	int sect = 1;

	for(i = 0; i < g_elfhead.iShnum; i++)
	{
		if(g_elfsections[i].blOutput)
		{
			g_elfsections[i].iIndex = sect++;
		}
	}
}

/* Load an ELF file */
int load_elf(const char *elf)
{
	int ret = 0;

	do
	{
		g_elfdata = load_file(elf);
		if(g_elfdata == NULL)
		{
			break;
		}

		if(!validate_header(g_elfdata))
		{
			break;
		}

		if(!load_sections(g_elfdata))
		{
			break;
		}

		if(!process_relocs())
		{
			break;
		}

		reindex_sections();

		ret = 1;
	}
	while(0);

	return ret;
}

int calculate_outsize(void)
{
	/* out_sects starts at two for the null section and the section string table */
	int out_sects = 2;
	int alloc_size = 0;
	int reloc_size = 0;
	int mem_size = 0;
	/* 1 for the NUL for the NULL section */
	int str_size = 1;
	int i;

	/* Calculate how big our output file needs to be */
	/* We have elf header + 1 PH + allocated data + section headers + relocation data */

	/* Note that the ELF should be based from 0, we use this to calculate the alloc and mem sizes */

	/* Skip null section */
	for(i = 1; i < g_elfhead.iShnum; i++)
	{
		if(g_elfsections[i].blOutput)
		{
			if(g_elfsections[i].iType == SHT_PROGBITS)
			{
				unsigned int top_addr;

				top_addr = g_elfsections[i].iAddr + g_elfsections[i].iSize;

				if(top_addr > alloc_size)
				{
					alloc_size = top_addr;
				}

				if(top_addr > mem_size)
				{
					mem_size = top_addr;
				}
				
				str_size += strlen(g_elfsections[i].szName) + 1;
			}
			else if(g_elfsections[i].iType == SHT_REL)
			{
				reloc_size += g_elfsections[i].iSize;
				str_size += strlen(g_elfsections[i].szName) + 1;
			}
			else
			{
				unsigned int top_addr;

				top_addr = g_elfsections[i].iAddr + g_elfsections[i].iSize;

				if(top_addr > mem_size)
				{
					mem_size = top_addr;
				}

				str_size += strlen(g_elfsections[i].szName) + 1;
			}
			out_sects++;
		}
	}

	alloc_size = (alloc_size + 3) & ~3;
	mem_size = (mem_size + 3) & ~3;
	str_size = (str_size + 3) & ~3;
	str_size += strlen(ELF_SH_STRTAB) + 1;

	if(g_verbose)
	{
		fprintf(stderr, "Out_sects %d, alloc_size %d, reloc_size %d, str_size %d, mem_size %d\n",
				out_sects, alloc_size, reloc_size, str_size, mem_size);
	}

	/* Save them for future use */
	g_out_sects = out_sects;
	g_alloc_size = alloc_size;
	g_reloc_size = reloc_size;
	g_mem_size = mem_size;
	g_str_size = str_size;

	/* Lets build the offsets */
	g_phbase = sizeof(Elf32_Ehdr);
	/* The allocated data needs to be 4 byte aligned */
	g_iopmodbase = (g_phbase + (2 * sizeof(Elf32_Phdr)) + 0x3) & ~0x3;
	/* The allocated data needs to be 16 byte aligned */
	g_allocbase = (g_iopmodbase + g_iopmod->iSize + 0xF) & ~0xF;
	g_shbase = g_allocbase + g_alloc_size;
	g_relocbase = g_shbase + (g_out_sects * sizeof(Elf32_Shdr));
	g_shstrbase = g_relocbase + g_reloc_size;

	if(g_verbose)
	{
		fprintf(stderr, "PHBase %08X, AllocBase %08X, SHBase %08X\n", g_phbase, g_allocbase, g_shbase);
		fprintf(stderr, "Relocbase %08X, Shstrbase %08X\n", g_relocbase, g_shstrbase);
		fprintf(stderr, "Total size %d\n", g_shstrbase + g_str_size);
	}

	return (g_shstrbase + g_str_size);
}

/* Output the ELF header */
void output_header(unsigned char *data)
{
	Elf32_Ehdr *head;

	head = (Elf32_Ehdr*) data;

	SW(&head->e_magic, g_elfhead.iMagic);
	head->e_class = g_elfhead.iClass;
	head->e_data = g_elfhead.iData;
	head->e_idver = g_elfhead.iIdver;
	SH(&head->e_type, ELF_IRX_TYPE);
	SH(&head->e_machine, g_elfhead.iMachine);
	SW(&head->e_version, g_elfhead.iVersion);
	SW(&head->e_entry, g_elfhead.iEntry);
	SW(&head->e_phoff, g_phbase);
	SW(&head->e_shoff, g_shbase);
	SW(&head->e_flags, g_elfhead.iFlags);
	SH(&head->e_ehsize, sizeof(Elf32_Ehdr));
	SH(&head->e_phentsize, sizeof(Elf32_Phdr));
	SH(&head->e_phnum, 2);
	SH(&head->e_shentsize, sizeof(Elf32_Shdr));
	SH(&head->e_shnum, g_out_sects);
	SH(&head->e_shstrndx, g_out_sects-1);
}

/* Output the iopmod program header */
void output_ph_iopmod(unsigned char *data)
{
	Elf32_Phdr *phdr;

	phdr = (Elf32_Phdr*) data;

	SW(&phdr->p_type, SHT_LOPROC | SHT_LOPROC_IOPMOD);
	/* Starts after the program header */
	SW(&phdr->p_offset, g_iopmodbase);
	SW(&phdr->p_vaddr, 0);
	SW(&phdr->p_paddr, 0);
	SW(&phdr->p_filesz, g_iopmod->iSize);
	SW(&phdr->p_memsz, 0);
	SW(&phdr->p_flags, 4);
	SW(&phdr->p_align, 0x4);
}

/* Output the program header */
void output_ph(unsigned char *data)
{
	Elf32_Phdr *phdr;

	phdr = (Elf32_Phdr*) data;

	SW(&phdr->p_type, 1);
	/* Starts after the program header */
	SW(&phdr->p_offset, g_allocbase);
	SW(&phdr->p_vaddr, 0);
	SW(&phdr->p_paddr, 0);
	SW(&phdr->p_filesz, g_alloc_size);
	SW(&phdr->p_memsz, g_mem_size);
	SW(&phdr->p_flags, 7);
	SW(&phdr->p_align, 0x10);
}

/* Output the allocated sections */
void output_alloc(unsigned char *data)
{
	int i;

	for(i = 0; i < g_elfhead.iShnum; i++)
	{
		if((g_elfsections[i].blOutput) && (g_elfsections[i].iType == SHT_PROGBITS))
		{
			memcpy(&data[g_elfsections[i].iAddr], g_elfsections[i].pData, g_elfsections[i].iSize);
		}
	}
}

/* Output the iopmod section */
void output_iopmod(unsigned char *data)
{
	memcpy(data, g_iopmod->pData, g_iopmod->iSize);
}

/* Output the section headers */
void output_sh(unsigned char *data)
{
	unsigned int reloc_ofs;
	unsigned int str_ofs;
	Elf32_Shdr *shdr;
	int i;

	shdr = (Elf32_Shdr*) data;
	/* For the NULL section */
	shdr++;
	memset(data, 0, g_out_sects * sizeof(Elf32_Shdr));

	reloc_ofs = g_relocbase;
	str_ofs = 1;

	for(i = 1; i < g_elfhead.iShnum; i++)
	{
		if(g_elfsections[i].blOutput)
		{
			SW(&shdr->sh_name, str_ofs);
			str_ofs += strlen(g_elfsections[i].szName) + 1;
			SW(&shdr->sh_flags, g_elfsections[i].iFlags);
			SW(&shdr->sh_addr, g_elfsections[i].iAddr);
			SW(&shdr->sh_size, g_elfsections[i].iSize);
			/* Note: this is safe set to zero for REL sections, despite the readelf warnings */
			SW(&shdr->sh_link, 0);
			SW(&shdr->sh_addralign, g_elfsections[i].iAddralign);
			SW(&shdr->sh_entsize, g_elfsections[i].iEntsize);

			if(g_elfsections[i].iType == SHT_REL)
			{
				SW(&shdr->sh_type, SHT_REL);
				if (g_elfsections[i].pRef)
					SW(&shdr->sh_info, g_elfsections[i].pRef->iIndex);
				else
					SW(&shdr->sh_info, 0);
				SW(&shdr->sh_offset, reloc_ofs);
				reloc_ofs += g_elfsections[i].iSize;
			}
			else if(g_elfsections[i].iType == SHT_PROGBITS)
			{
				SW(&shdr->sh_type, g_elfsections[i].iType);
				SW(&shdr->sh_info, 0);
				SW(&shdr->sh_offset, g_allocbase + g_elfsections[i].iAddr);
			}
			else if(&g_elfsections[i] == g_iopmod)
			{
				SW(&shdr->sh_type, g_elfsections[i].iType);
				SW(&shdr->sh_info, 0);
				SW(&shdr->sh_offset, g_iopmodbase);
			}
			else
			{
				SW(&shdr->sh_type, g_elfsections[i].iType);
				SW(&shdr->sh_info, 0);
				/* Point it to the end of the allocated section */
				SW(&shdr->sh_offset, g_allocbase + g_alloc_size);
			}

			shdr++;
		}
	}

	/* Fill in the shstrtab section */
	SW(&shdr->sh_name, str_ofs);
	SW(&shdr->sh_flags, 0);
	SW(&shdr->sh_addr, 0);
	SW(&shdr->sh_size, g_str_size);
	SW(&shdr->sh_link, 0);
	SW(&shdr->sh_addralign, 1);
	SW(&shdr->sh_entsize, 0);
	SW(&shdr->sh_type, SHT_STRTAB);
	SW(&shdr->sh_info, 0);
	SW(&shdr->sh_offset, g_shstrbase);
}

/* Output relocations */
void output_relocs(unsigned char *data)
{
	int i;
	unsigned char *pReloc;

	pReloc = data;

	for(i = 0; i < g_elfhead.iShnum; i++)
	{
		if((g_elfsections[i].blOutput) &&
				((g_elfsections[i].iType == SHT_REL)))
		{
			memcpy(pReloc, g_elfsections[i].pData, g_elfsections[i].iSize);
			pReloc += g_elfsections[i].iSize;
		}
	}
}

/* Output the section header string table */
void output_shstrtab(unsigned char *data)
{
	int i;
	char *pData;

	/* For the NULL section, memory should be zeroed anyway */
	memset(data, 0, g_str_size);
	pData = (char *) (data + 1);

	for(i = 1; i < g_elfhead.iShnum; i++)
	{
		if(g_elfsections[i].blOutput)
		{
			if(g_verbose)
			{
				fprintf(stderr, "String %d: %s\n", i, g_elfsections[i].szName);
			}

			strcpy(pData, g_elfsections[i].szName);
			pData += strlen(g_elfsections[i].szName) + 1;
		}
	}

	strcpy(pData, ELF_SH_STRTAB);
}

/* Output a stripped irx file */
int output_irx(const char *irxfile)
{
	do
	{
		int size;
		unsigned char *data;
		FILE *fp;

		size = calculate_outsize();
		data = (unsigned char *) malloc(size);
		if(data == NULL)
		{
			fprintf(stderr, "Error, couldn't allocate output data\n");
			break;
		}

		memset(data, 0, size);

		output_header(data);
		output_ph_iopmod(data + g_phbase);
		output_ph(data + g_phbase + sizeof(Elf32_Phdr));
		output_iopmod(data + g_iopmodbase);
		output_alloc(data + g_allocbase);
		output_sh(data + g_shbase);
		output_relocs(data + g_relocbase);
		output_shstrtab(data + g_shstrbase);

		fp = fopen(irxfile, "wb");
		if(fp != NULL)
		{
			fwrite(data, 1, size, fp);
			fclose(fp);
		}
		else
		{
			fprintf(stderr, "Error, could not open output file %s\n", irxfile);
		}

		free(data);
	}
	while(0);

	return 0;
}

/* Free allocated memory */
void free_data(void)
{
	if(g_elfdata != NULL)
	{
		free(g_elfdata);
		g_elfdata = NULL;
	}

	if(g_elfsections != NULL)
	{
		free(g_elfsections);
		g_elfsections = NULL;
	}
}

int main(int argc, char **argv)
{
	if(process_args(argc, argv))
	{
		if(load_elf(g_infile))
		{
			(void) output_irx(g_outfile);
			free_data();
		}
	}
	else
	{
		print_help();
	}

	return 0;
}
