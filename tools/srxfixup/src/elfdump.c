/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "srxfixup_internal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rellink
{
	int rid;
	const elf_rel *rp;
	const elf_rel *mhrp;
};
struct name2num
{
	const char *name;
	unsigned int num;
};

static void search_rel_section(
	const elf_file *elf, const elf_section *scp, elf_rel **result, unsigned int *relentries, unsigned int *baseoff);
static void search_rel_data(const elf_rel *rpbase, unsigned int relentries, unsigned int addr, struct rellink *result);
static void dumpb(const char *head, unsigned int address, unsigned int size, const uint8_t *data);
static void dumph(const char *head, unsigned int address, unsigned int size, const uint16_t *data);
static void dumpw(const char *head, unsigned int address, unsigned int size, const uint32_t *data);
static const char *num2name(const struct name2num *table, unsigned int num);

void print_elf(const elf_file *elf, unsigned int flag)
{
	if ( elf == NULL )
	{
		return;
	}
	print_elf_ehdr(elf, flag);
	print_elf_phdr(elf, flag);
	print_elf_sections(elf, flag);
	if ( (flag & 0x100) != 0 )
	{
		Elf_file_slot *order;

		order = build_file_order_list(elf);
		dump_file_order_list(elf, order);
		free(order);
	}
}

// clang-format off
static const struct name2num Ei_class_name[] =
{
#define X(d) { #d, d },
	XEACH_Ei_class_name_enum()
#undef X
	{ NULL, 0 },
};
static const struct name2num E_type_name[] =
{
#define X(d) { #d, d },
	XEACH_E_type_name_enum()
#undef X
	{ NULL, 0 },
};
static const struct name2num Ei_data_name[] =
{
#define X(d) { #d, d },
	XEACH_Ei_data_name_enum()
#undef X
	{ NULL, 0 },
};
static const struct name2num E_version_name[] =
{
#define X(d) { #d, d },
	XEACH_E_version_name_enum()
#undef X
	{ NULL, 0 },
};
static const struct name2num E_machine_name[] =
{
#define X(d) { #d, d },
	XEACH_E_machine_name_enum()
#undef X
	{ NULL, 0 },
};
// clang-format on

void print_elf_ehdr(const elf_file *elf, unsigned int flag)
{
	if ( (flag & 1) == 0 )
	{
		return;
	}
	printf(" Elf header\n");
	printf(
		"   e_ident = 0x%02x+\"%c%c%c\", class = %s, encode = %s\n",
		elf->ehp->e_ident[0],
		elf->ehp->e_ident[1],
		elf->ehp->e_ident[2],
		elf->ehp->e_ident[3],
		num2name(Ei_class_name, elf->ehp->e_ident[4]),
		num2name(Ei_data_name, elf->ehp->e_ident[5]));
	printf("             version = %s\n", num2name(E_version_name, elf->ehp->e_ident[6]));
	printf(
		"   e_type = %s, e_machine = %s, e_version = %s\n",
		num2name(E_type_name, elf->ehp->e_type),
		num2name(E_machine_name, elf->ehp->e_machine),
		num2name(E_version_name, elf->ehp->e_version));
	printf(
		"   e_entry = 0x%08x,  e_phoff = 0x%08x, e_shoff = 0x%08x\n",
		elf->ehp->e_entry,
		elf->ehp->e_phoff,
		elf->ehp->e_shoff);
	printf("   e_flags = 0x%08x ", elf->ehp->e_flags);
	if ( (elf->ehp->e_flags & EF_MIPS_NOREORDER) != 0 )
	{
		printf("EF_MIPS_NOREORDER ");
	}
	if ( (elf->ehp->e_flags & EF_MIPS_PIC) != 0 )
	{
		printf("EF_MIPS_PIC");
	}
	if ( (elf->ehp->e_flags & EF_MIPS_CPIC) != 0 )
	{
		printf("EF_MIPS_CPIC");
	}
	printf("\n");
	printf(
		"   e_ehsize = 0x%04x,  e_phentsize = 0x%04x, e_phnum = 0x%04x\n",
		elf->ehp->e_ehsize,
		elf->ehp->e_phentsize,
		elf->ehp->e_phnum);
	printf(
		"   e_shentsize = 0x%04x, e_shnum = 0x%04x, e_shstrndx = 0x%04x\n",
		elf->ehp->e_shentsize,
		elf->ehp->e_shnum,
		elf->ehp->e_shstrndx);
}

// clang-format off
static const struct name2num P_type_name[] =
{
#define X(d) { #d, d },
	XEACH_P_type_name_enum()
#undef X
	{ NULL, 0 },
};
// clang-format on

void print_elf_phdr(const elf_file *elf, unsigned int flag)
{
	int i;

	if ( elf->php == NULL || (flag & 1) == 0 )
	{
		return;
	}
	printf(" Program header\n");
	for ( i = 0; i < elf->ehp->e_phnum; i += 1 )
	{
		printf(" %2d: p_type=%s, ", i, num2name(P_type_name, elf->php[i].phdr.p_type));
		printf(
			"p_offset=0x%06x, p_vaddr,p_paddr=0x%06x,0x%06x\n",
			elf->php[i].phdr.p_offset,
			elf->php[i].phdr.p_vaddr,
			elf->php[i].phdr.p_paddr);
		printf(
			"     p_filesz=0x%06x, p_memsiz=0x%06x, p_align=%d\n",
			elf->php[i].phdr.p_filesz,
			elf->php[i].phdr.p_memsz,
			(int)(elf->php[i].phdr.p_align));
		printf("     p_flags=0x%08x ( ", elf->php[i].phdr.p_flags);
		if ( (elf->php[i].phdr.p_flags & PF_X) != 0 )
		{
			printf("PF_X ");
		}
		if ( (elf->php[i].phdr.p_flags & PF_W) != 0 )
		{
			printf("PF_W ");
		}
		if ( (elf->php[i].phdr.p_flags & PF_R) != 0 )
		{
			printf("PF_R ");
		}
		printf(")\n");
		if ( elf->php[i].scp )
		{
			int j;

			printf("     include sections = ");
			for ( j = 0; elf->php[i].scp[j]; j += 1 )
			{
				printf("%s ", elf->php[i].scp[j]->name);
			}
			printf("\n");
		}
	}
}

// clang-format off
static const struct name2num S_type_name[] =
{
#define X(d) { #d, d },
	XEACH_S_type_name_enum()
#undef X
	{ NULL, 0 },
};
// clang-format on

void print_elf_sections(const elf_file *elf, unsigned int flag)
{
	int i;

	if ( elf->scp == NULL )
	{
		return;
	}
	if ( (flag & 1) != 0 )
	{
		printf(" Section header\n");
	}
	for ( i = 1; i < elf->ehp->e_shnum; i += 1 )
	{
		if ( (flag & 1) != 0 || ((flag & 2) != 0 && elf->scp[i]->shr.sh_type == SHT_REL) )
		{
			printf(
				" %2d: %-12s sh_name=0x%04x sh_type=%s\n",
				i,
				elf->scp[i]->name,
				elf->scp[i]->shr.sh_name,
				num2name(S_type_name, elf->scp[i]->shr.sh_type));
			printf("        sh_flas=0x%08x ( ", elf->scp[i]->shr.sh_flags);
			if ( (elf->scp[i]->shr.sh_flags & SHF_WRITE) != 0 )
			{
				printf("SHF_WRITE ");
			}
			if ( (elf->scp[i]->shr.sh_flags & SHF_ALLOC) != 0 )
			{
				printf("SHF_ALLOC ");
			}
			if ( (elf->scp[i]->shr.sh_flags & SHF_EXECINSTR) != 0 )
			{
				printf("SHF_EXECINSTR ");
			}
			if ( (elf->scp[i]->shr.sh_flags & SHF_MIPS_GPREL) != 0 )
			{
				printf("SHF_MIPS_GPREL ");
			}
			printf(")\n");
			printf(
				"        sh_addr,sh_offset,sh_size=0x%06x,0x%06x,0x%06x, sh_addralign=%2d\n",
				elf->scp[i]->shr.sh_addr,
				elf->scp[i]->shr.sh_offset,
				elf->scp[i]->shr.sh_size,
				(int)(elf->scp[i]->shr.sh_addralign));
			printf(
				"        sh_link=0x%08x, sh_info=0x%08x, sh_entsize=0x%02x(%d)\n",
				elf->scp[i]->shr.sh_link,
				elf->scp[i]->shr.sh_info,
				elf->scp[i]->shr.sh_entsize,
				(int)(elf->scp[i]->shr.sh_entsize));
		}
		switch ( elf->scp[i]->shr.sh_type )
		{
			case SHT_SYMTAB:
			case SHT_DYNSYM:
				print_elf_symtbl(elf->scp[i], flag);
				continue;
			case SHT_REL:
				print_elf_reloc(elf->scp[i], flag);
				continue;
			case SHT_MIPS_DEBUG:
				print_elf_mips_symbols((elf_mips_symbolic_data *)elf->scp[i]->data, flag);
				continue;
			case SHT_SCE_IOPMOD:
				if ( (flag & 1) != 0 )
				{
					const Elf32_IopMod *data;

					data = (Elf32_IopMod *)elf->scp[i]->data;
					printf(
						"        moduleinfo=0x%08x, entry=0x%08x, gpvalue=0x%08x\n", data->moduleinfo, data->entry, data->gp_value);
					printf(
						"        text_size=0x%08x, data_size=0x%08x, bss_size=0x%08x\n",
						data->text_size,
						data->data_size,
						data->bss_size);
				}
				break;
			case SHT_SCE_EEMOD:
				if ( (flag & 1) != 0 )
				{
					const Elf32_EeMod *data;

					data = (Elf32_EeMod *)elf->scp[i]->data;
					printf(
						"        moduleinfo=0x%08x, entry=0x%08x, gpvalue=0x%08x\n", data->moduleinfo, data->entry, data->gp_value);
					printf(
						"        text_size=0x%08x, data_size=0x%08x, bss_size=0x%08x\n",
						data->text_size,
						data->data_size,
						data->bss_size);
					printf("        erx_lib_addr=0x%08x, erx_lib_size=0x%08x\n", data->erx_lib_addr, data->erx_lib_size);
					printf("        erx_stub_addr=0x%08x, erx_stub_size=0x%08x\n", data->erx_stub_addr, data->erx_stub_size);
				}
				break;
			case SHT_MIPS_REGINFO:
				if ( (flag & 1) != 0 )
				{
					const Elf32_RegInfo *data;

					data = (Elf32_RegInfo *)elf->scp[i]->data;
					printf(
						"        gpmask=0x08x, cprmask=%08x,%08x,%08x,%08x\n",
						data->ri_gprmask,
						data->ri_cprmask[0],
						data->ri_cprmask[1],
						data->ri_cprmask[3]);
					printf("        gp=0x%08x\n", data->ri_gp_value);
				}
				break;
			default:
				break;
		}
		if ( elf->scp[i]->data )
		{
			if ( (elf->scp[i]->shr.sh_flags & SHF_EXECINSTR) != 0 && elf->ehp->e_machine == EM_MIPS )
			{
				if (
					((elf->ehp->e_flags & EF_MIPS_MACH) == EF_MIPS_MACH_5900)
					&& ((elf->ehp->e_flags & EF_MIPS_ARCH) == EF_MIPS_ARCH_3) )
				{
					initdisasm(2, -1, 0, 0, 0);
				}
				else
				{
					initdisasm(1, -1, 0, 0, 0);
				}
				print_elf_disasm(elf, elf->scp[i], flag);
			}
			else
			{
				print_elf_datadump(elf, elf->scp[i], flag);
			}
		}
	}
}

// clang-format off
static const struct name2num R_MIPS_Type[] =
{
#define X(d) { #d, d },
	XEACH_R_MIPS_Type_enum()
#undef X
	{ NULL, 0 },
};
// clang-format on

void print_elf_reloc(const elf_section *scp, unsigned int flag)
{
	const elf_rel *rp;
	unsigned int entrise;
	unsigned int i;

	if ( (flag & 2) == 0 )
	{
		return;
	}
	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	if ( entrise != 0 )
	{
		printf("   ###: r_offset  r_type                r_sym\n");
		printf("   ---  --------  --------------------  -----------------------------------\n");
	}
	rp = (elf_rel *)scp->data;
	for ( i = 0; i < entrise; i += 1 )
	{
		printf("   %3u: 0x%06x  0x%02x %-16s ", i, rp[i].rel.r_offset, rp[i].type, num2name(R_MIPS_Type, rp[i].type));
		if ( rp[i].symptr && rp[i].symptr->type == STT_SECTION )
		{
			printf("0x%03x[%s]\n", rp[i].rel.r_info >> 8, rp[i].symptr->shptr->name);
		}
		else
		{
			printf("0x%03x %s\n", rp[i].rel.r_info >> 8, (rp[i].symptr && rp[i].symptr->name) ? rp[i].symptr->name : "");
		}
	}
	printf("\n");
}

void print_elf_disasm(const elf_file *elf, const elf_section *scp, unsigned int flag)
{
	int v7;
	const elf_rel *rp;
	elf_rel *rpbase;
	struct rellink *rel;
	Disasm_result **dis;
	char pb[200];
	unsigned int baseoff;
	unsigned int addr;
	const unsigned int *codes;
	unsigned int relentries;
	size_t steps;
	int d;
	unsigned int i;

	if ( (flag & 8) == 0 )
	{
		return;
	}
	steps = scp->shr.sh_size >> 2;
	codes = (unsigned int *)scp->data;
	dis = (Disasm_result **)malloc(steps * sizeof(Disasm_result *));
	search_rel_section(elf, scp, &rpbase, &relentries, &baseoff);
	rel = (struct rellink *)calloc(steps, sizeof(struct rellink));
	for ( i = 0, addr = 0; i < steps; i += 1, addr += 4 )
	{
		dis[i] = disassemble(addr, codes[i]);
		gen_asmmacro(dis[i]);
		search_rel_data(rpbase, relentries, baseoff + addr, &rel[i]);
	}
	for ( i = 0; i < steps; i += 1 )
	{
		if ( rel[i].rp && rel[i].rp->type == R_MIPSSCE_MHI16 )
		{
			unsigned int j;

			j = i;
			for ( d = (int16_t)(codes[i] & 0xFFFF); d; d = (int16_t)(codes[j] & 0xFFFF) )
			{
				j += d;
				rel[j].rid = rel[i].rid;
				rel[j].mhrp = rel[i].rp;
			}
		}
	}
	for ( i = 0; i < steps; i += 1 )
	{
		format_disasm(dis[i], pb);
		if ( rel[i].rp || rel[i].mhrp )
		{
			size_t k;

			for ( k = strlen(pb); k <= 47; k += 1 )
			{
				pb[k] = 32;
			}
			pb[48] = 0;
			sprintf(&pb[strlen(pb)], "%3d:", rel[i].rid);
			if ( rel[i].rp )
			{
				rp = rel[i].rp;
				strcat(pb, " ");
			}
			else
			{
				rp = rel[i].mhrp;
				strcat(pb, ">");
			}
			if ( rp->symptr && rp->symptr->type == STT_SECTION )
			{
				sprintf(
					&pb[strlen(pb)],
					"  %s %d '%s'",
					num2name(R_MIPS_Type, rp->type),
					(int)(rp->rel.r_info >> 8),
					rp->symptr->shptr->name);
			}
			else
			{
				v7 = ' ';
				if ( rp->symptr )
				{
					switch ( rp->symptr->bind )
					{
						case STB_GLOBAL:
							v7 = 'E';
							break;
						case STB_LOCAL:
							v7 = 'l';
							break;
						case STB_WEAK:
							v7 = 'w';
							break;
						default:
							break;
					}
				}
				sprintf(
					&pb[strlen(pb)],
					"%c %s %d %s",
					v7,
					num2name(R_MIPS_Type, rp->type),
					(int)(rp->rel.r_info >> 8),
					(rp->symptr && rp->symptr->name) ? rp->symptr->name : "");
			}
		}
		printf("    %s\n", pb);
		free(dis[i]);
	}
	printf("\n");
	free(dis);
	free(rel);
}

static void search_rel_section(
	const elf_file *elf, const elf_section *scp, elf_rel **result, unsigned int *relentries, unsigned int *baseoff)
{
	elf_section *relscp;
	int i;

	relscp = 0;
	for ( i = 1; i < elf->ehp->e_shnum; i += 1 )
	{
		relscp = elf->scp[i];
		if ( relscp->shr.sh_type == SHT_REL && scp == relscp->info )
		{
			break;
		}
	}
	if ( i < elf->ehp->e_shnum )
	{
		*result = (elf_rel *)relscp->data;
		*relentries = relscp->shr.sh_size / relscp->shr.sh_entsize;
		*baseoff = 0;
		switch ( elf->ehp->e_type )
		{
			case ET_SCE_IOPRELEXEC:
			case ET_SCE_IOPRELEXEC2:
			case ET_SCE_EERELEXEC:
			case ET_SCE_EERELEXEC2:
				*baseoff = scp->shr.sh_addr;
				break;
			default:
				break;
		}
	}
	else
	{
		*result = 0;
		*relentries = 0;
		*baseoff = 0;
	}
}

static void search_rel_data(const elf_rel *rpbase, unsigned int relentries, unsigned int addr, struct rellink *result)
{
	int j;

	result->rid = -1;
	result->rp = 0;
	for ( j = 0; j < (int)relentries; j += 1 )
	{
		if ( rpbase[j].type != R_MIPSSCE_ADDEND && addr == rpbase[j].rel.r_offset )
		{
			result->rid = j;
			result->rp = &rpbase[j];
			return;
		}
	}
}

static void dumpb(const char *head, unsigned int address, unsigned int size, const uint8_t *data)
{
	uint8_t cbuf[20];
	unsigned int addr;
	unsigned int off2;
	unsigned int off1;

	addr = address & ~0xF;
	off1 = 0;
	off2 = size;
	strcpy((char *)cbuf, "........ ........");
	for ( ; off1 < off2; addr += 1 )
	{
		if ( (addr & 0xF) == 0 )
		{
			printf("%s%08x: ", head, addr);
		}
		if ( address > addr )
		{
			printf("-- ");
		}
		else
		{
			printf("%02x ", data[off1]);
			if ( data[off1] > 0x1F && data[off1] <= 0x7E )
			{
				unsigned int v4;
				uint8_t *v5;

				v4 = addr & 0xF;
				if ( v4 <= 7 )
				{
					v5 = &cbuf[v4];
				}
				else
				{
					v5 = &cbuf[v4 + 1];
				}
				*v5 = data[off1];
			}
		}
		if ( (((uint8_t)addr + 1) & 3) == 0 )
		{
			printf(" ");
		}
		if ( (addr & 0xF) == 15 )
		{
			printf("%s", cbuf);
			printf("\n");
			strcpy((char *)cbuf, "........ ........");
		}
		if ( address <= addr )
		{
			off1 += 1;
		}
	}
	for ( ; (addr & 0xF) != 0; addr += 1 )
	{
		printf("-- ");
		if ( (((uint8_t)addr + 1) & 3) == 0 )
		{
			printf(" ");
		}
		if ( (addr & 0xF) == 15 )
		{
			printf("%s", cbuf);
			printf("\n");
			strcpy((char *)cbuf, "........ ........");
		}
	}
}

static void dumph(const char *head, unsigned int address, unsigned int size, const uint16_t *data)
{
	unsigned int addr;
	unsigned int off2;
	unsigned int off1;

	addr = address & ~0xF;
	off2 = addr;
	off1 = 0;
	for ( ; off1 < size; off2 += 2 )
	{
		if ( (off2 & 0xF) == 0 )
		{
			printf("%s%08x: ", head, off2);
		}
		if ( address > off2 )
		{
			printf("---- ");
		}
		else
		{
			printf("%04x ", data[off1 >> 1]);
		}
		if ( (((uint8_t)off2 + 2) & 3) == 0 )
		{
			printf("  ");
		}
		if ( (off2 & 0xF) == 14 )
		{
			printf("\n");
		}
		if ( address <= off2 )
		{
			off1 += 2;
		}
	}
	for ( ; (off2 & 0xF) != 0; off2 += 2 )
	{
		printf("---- ");
		if ( (((uint8_t)off2 + 2) & 3) == 0 )
		{
			printf("  ");
		}
		if ( (off2 & 0xF) == 14 )
		{
			printf("\n");
		}
	}
}

static void dumpw(const char *head, unsigned int address, unsigned int size, const uint32_t *data)
{
	unsigned int off1;
	unsigned int addr;

	addr = address & ~0xF;
	off1 = 0;
	for ( ; off1 < size; addr += 4 )
	{
		if ( (addr & 0xF) == 0 )
		{
			printf("%s%08x: ", head, addr);
		}
		if ( address > addr )
		{
			printf("--------  ");
		}
		else
		{
			printf("%08x  ", data[off1 >> 2]);
		}
		if ( (addr & 0xF) == 12 )
		{
			printf("\n");
		}
		if ( address <= addr )
		{
			off1 += 4;
		}
	}
	for ( ; (addr & 0xF) != 0; addr += 4 )
	{
		printf("--------  ");
		if ( (addr & 0xF) == 12 )
		{
			printf("\n");
		}
	}
}

void print_elf_datadump(const elf_file *elf, const elf_section *scp, unsigned int flag)
{
	elf_rel *rpbase;
	struct rellink rel;
	unsigned int relentries;
	unsigned int baseoff;
	unsigned int *dumpbuf;

	if ( (flag & 0xF0) == 0 )
	{
		return;
	}
	dumpbuf = (unsigned int *)calloc(1, scp->shr.sh_size + 4);
	memcpy(dumpbuf, scp->data, scp->shr.sh_size);
	switch ( scp->shr.sh_type )
	{
		case SHT_PROGBITS:
		case SHT_HASH:
		case SHT_DYNAMIC:
		case SHT_MIPS_REGINFO:
			swapmemory(dumpbuf, "l", (scp->shr.sh_size + 1) >> 2);
			break;
		default:
			break;
	}
	printf("\n");
	if ( (flag & 0x10) != 0 )
	{
		dumpb("", 0, scp->shr.sh_size, (uint8_t *)dumpbuf);
		printf("\n");
	}
	if ( (flag & 0x20) != 0 )
	{
		dumph("          ", 0, scp->shr.sh_size, (uint16_t *)dumpbuf);
		printf("\n");
	}
	if ( (flag & 0xC0) != 0 )
	{
		unsigned int offset;

		search_rel_section(elf, scp, &rpbase, &relentries, &baseoff);
		for ( offset = 0; offset < scp->shr.sh_size; offset += 16 )
		{
			dumpw(
				"          ",
				offset,
				(scp->shr.sh_size > offset + 16) ? 16 : (scp->shr.sh_size - offset),
				&dumpbuf[offset / 4]);
			if ( (flag & 0x80) != 0 )
			{
				int i;

				for ( i = 0; i <= 15 && (offset + i) < scp->shr.sh_size; i += 4 )
				{
					search_rel_data(rpbase, relentries, baseoff + offset + i, &rel);
					if ( rel.rid >= 0 )
					{
						printf("                  +%02x:", i);
						printf(" [%3d]", rel.rid);
						if ( rel.rp->symptr && rel.rp->symptr->type == STT_SECTION )
						{
							printf(
								"  %s %d '%s'",
								num2name(R_MIPS_Type, rel.rp->type),
								(int)(rel.rp->rel.r_info >> 8),
								rel.rp->symptr->shptr->name);
						}
						else
						{
							int v7;

							v7 = ' ';
							if ( rel.rp->symptr )
							{
								switch ( rel.rp->symptr->bind )
								{
									case STB_GLOBAL:
										v7 = 'E';
										break;
									case STB_LOCAL:
										v7 = 'l';
										break;
									case STB_WEAK:
										v7 = 'w';
										break;
									default:
										break;
								}
							}
							printf(
								"%c %s %d %s",
								v7,
								num2name(R_MIPS_Type, rel.rp->type),
								(int)(rel.rp->rel.r_info >> 8),
								(rel.rp->symptr && rel.rp->symptr->name) ? rel.rp->symptr->name : "");
						}
						printf("\n");
					}
				}
			}
		}
		printf("\n");
	}
	free(dumpbuf);
}

// clang-format off
static const struct name2num SymbolBinding[] =
{
#define X(d) { #d, d },
	XEACH_SymbolBinding_enum()
#undef X
	{ NULL, 0 },
};
static const struct name2num SymbolType[] =
{
#define X(d) { #d, d },
	XEACH_SymbolType_enum()
#undef X
	{ NULL, 0 },
};
static const struct name2num SymbolSpSection[] =
{
#define X(d) { #d, d },
	XEACH_SymbolSpSection_enum()
#undef X
	{ NULL, 0 },
};
// clang-format on

void print_elf_symtbl(const elf_section *scp, unsigned int flag)
{
	elf_syment **syp;
	unsigned int entirse;
	unsigned int i;

	if ( (flag & 4) == 0 )
	{
		return;
	}
	entirse = scp->shr.sh_size / scp->shr.sh_entsize;
	syp = (elf_syment **)scp->data;
	if ( entirse > 1 )
	{
		printf("   ###: name        bind       type        st_value  st_size st_shndx\n");
		printf("   ---  ----------- ---------- ----------- ---------- ------ ------------------\n");
	}
	for ( i = 1; i < entirse; i += 1 )
	{
		printf("   %3u: ", i);
		if ( syp[i]->name && strlen(syp[i]->name) > 0xB )
		{
			printf("%s\n                    ", syp[i]->name);
		}
		else
		{
			printf("%-11s ", syp[i]->name ?: "<no name>");
		}
		printf(
			"%-10s %-11s 0x%08x 0x%04x ",
			num2name(SymbolBinding, syp[i]->bind),
			num2name(SymbolType, syp[i]->type),
			syp[i]->sym.st_value,
			syp[i]->sym.st_size);
		if ( syp[i]->shptr )
		{
			printf("%2d '%s'", syp[i]->sym.st_shndx, syp[i]->shptr->name);
		}
		else
		{
			int st_shndx;

			st_shndx = syp[i]->sym.st_shndx;
			printf("%s(0x%x)", num2name(SymbolSpSection, st_shndx), st_shndx);
		}
		if ( syp[i]->sym.st_other )
		{
			printf("  st_other=0x%x", syp[i]->sym.st_other);
		}
		printf("\n");
	}
	printf("\n");
}

static const char *num2name(const struct name2num *table, unsigned int num)
{
	static char buf_28[30];

	for ( ; table->name; table += 1 )
	{
		if ( num == table->num )
		{
			return table->name;
		}
	}
	sprintf(buf_28, "? 0x%x", num);
	return buf_28;
}

// clang-format off
static const struct name2num SymbolTypes[] =
{
#define X(d) { #d, d },
	XEACH_SymbolTypes_enum()
#undef X
	{ NULL, 0 },
};
static const struct name2num StorageClasse[] =
{
#define X(d) { #d, d },
	XEACH_StorageClasse_enum()
#undef X
	{ NULL, 0 },
};
// clang-format on

void print_elf_mips_symbols(const elf_mips_symbolic_data *symbol, unsigned int flag)
{
	fdr *fdrp;
	unsigned int ifd;

	if ( (flag & 0x200) == 0 || symbol == NULL )
	{
		return;
	}
	printf(" Symbol header\n");
	printf("      magic=0x%x      vstamp=0x%x\n", symbol->head.magic, symbol->head.vstamp);
	printf(
		"      ilineMax= %4u   cbLine=       %6u    cbLineOffset=%u(0x%05x)\n",
		symbol->head.ilineMax,
		symbol->head.cbLine,
		symbol->head.cbLineOffset,
		symbol->head.cbLineOffset);
	printf(
		"      idnMax=   %4u   cbDnOffset=   %6u(0x%05x)\n",
		symbol->head.idnMax,
		symbol->head.cbDnOffset,
		symbol->head.cbDnOffset);
	printf(
		"      ipdMax=   %4u   cbPdOffset=   %6u(0x%05x)\n",
		symbol->head.ipdMax,
		symbol->head.cbPdOffset,
		symbol->head.cbPdOffset);
	printf(
		"      isymMax=  %4u   cbSymOffset=  %6u(0x%05x)\n",
		symbol->head.isymMax,
		symbol->head.cbSymOffset,
		symbol->head.cbSymOffset);
	printf(
		"      ioptMax=  %4u   cbOptOffset=  %6u(0x%05x)\n",
		symbol->head.ioptMax,
		symbol->head.cbOptOffset,
		symbol->head.cbOptOffset);
	printf(
		"      iauxMax=  %4u   cbAuxOffset=  %6u(0x%05x)\n",
		symbol->head.iauxMax,
		symbol->head.cbAuxOffset,
		symbol->head.cbAuxOffset);
	printf(
		"      issMax=   %4u   cbSsOffset=   %6u(0x%05x)\n",
		symbol->head.issMax,
		symbol->head.cbSsOffset,
		symbol->head.cbSsOffset);
	printf(
		"      issExtMax=%4u   cbSsExtOffset=%6u(0x%05x)\n",
		symbol->head.issExtMax,
		symbol->head.cbSsExtOffset,
		symbol->head.cbSsExtOffset);
	printf(
		"      ifdMax=   %4u   cbFdOffset=   %6u(0x%05x)\n",
		symbol->head.ifdMax,
		symbol->head.cbFdOffset,
		symbol->head.cbFdOffset);
	printf(
		"      crfd=     %4u   cbRfdOffset=  %6u(0x%05x)\n",
		symbol->head.crfd,
		symbol->head.cbRfdOffset,
		symbol->head.cbRfdOffset);
	printf(
		"      iextMax=  %4u   cbExtOffset=  %6u(0x%05x)\n",
		symbol->head.iextMax,
		symbol->head.cbExtOffset,
		symbol->head.cbExtOffset);
	printf("\n");
	if ( symbol->head.issMax > 0 )
	{
		int i_1;
		const char *cp_1;
		const char *cpend_1;

		cp_1 = symbol->cbSs_Ptr;
		cpend_1 = &cp_1[symbol->head.issMax];
		printf("  Local strings\n");
		for ( i_1 = 0; cp_1 < cpend_1; i_1 += 1 )
		{
			printf("   %3d(%5d): %s\n", i_1, (int)(cp_1 - symbol->cbSs_Ptr), cp_1);
			cp_1 += strlen(cp_1) + 1;
		}
		printf("\n");
	}
	if ( symbol->head.issExtMax > 0 )
	{
		int i_2;
		const char *cp_2;
		const char *cpend_2;

		cp_2 = symbol->cbSsExt_Ptr;
		cpend_2 = &cp_2[symbol->head.issExtMax];
		printf("  External strings\n");
		for ( i_2 = 0; cp_2 < cpend_2; i_2 += 1 )
		{
			printf("   %3d(%5d): %s\n", i_2, (int)(cp_2 - symbol->cbSsExt_Ptr), cp_2);
			cp_2 += strlen(cp_2) + 1;
		}
		printf("\n");
	}
	if ( symbol->head.iextMax > 0 )
	{
		extr *ep;
		unsigned int i_3;

		ep = (extr *)symbol->cbExt_Ptr;
		printf("  External symbols\n");
		for ( i_3 = 0; i_3 < symbol->head.iextMax; i_3 += 1 )
		{
			unsigned int idx_1;
			unsigned int class_1;
			unsigned int type_1;

			type_1 = ep->asym.sy_bits & 0x3F;
			class_1 = (ep->asym.sy_bits >> 6) & 0x1F;
			idx_1 = ep->asym.sy_bits >> 12;
			printf("   %3u: res,ifd=%04x,%04hx", i_3, ep->reserved, (unsigned short)(ep->ifd));
			printf(", 0x%08x", ep->asym.value);
			printf(", %8s:%-7s 0x%05x ", num2name(SymbolTypes, type_1), num2name(StorageClasse, class_1), idx_1);
			printf("%s\n", &symbol->cbSsExt_Ptr[ep->asym.iss]);
			ep += 1;
		}
		printf("\n");
	}
	printf("  Local informations\n");
	fdrp = (fdr *)symbol->cbFd_Ptr;
	for ( ifd = 0; ifd < symbol->head.ifdMax; ifd += 1 )
	{
		const char *issBase;

		issBase = &symbol->cbSs_Ptr[fdrp->issBase];
		printf("  %3u: %-40s addr=0x%08x \n", ifd, &issBase[fdrp->rss], fdrp->adr);
		if ( fdrp->csym > 0 )
		{
			symr *syp_1;
			int i_4;

			syp_1 = (symr *)&symbol->cbSym_Ptr[sizeof(symr) * fdrp->isymBase];
			printf("      Local symbols\n");
			for ( i_4 = 0; i_4 < fdrp->csym; i_4 += 1 )
			{
				unsigned int idx_2;
				unsigned int class_2;
				unsigned int type_2;

				type_2 = syp_1->sy_bits & 0x3F;
				class_2 = (syp_1->sy_bits >> 6) & 0x1F;
				idx_2 = syp_1->sy_bits >> 12;
				printf("      %3d: 0x%08x", i_4, syp_1->value);
				printf(", %10s:%-10s 0x%05x ", num2name(SymbolTypes, type_2), num2name(StorageClasse, class_2), idx_2);
				printf("%s\n", &issBase[syp_1->iss]);
				syp_1 += 1;
			}
		}
		if ( fdrp->cline > 0 )
		{
			printf("      Line numbers\n");
			printf("        ilinBase = %d, cline= %d\n", fdrp->ilineBase, fdrp->cline);
		}
		if ( fdrp->copt > 0 )
		{
			printf("      Optimization entries\n");
			printf("        ioptBase = %d, copt = %d\n", fdrp->ioptBase, fdrp->copt);
		}
		if ( fdrp->cpd > 0 )
		{
			pdr *pdrp_2;
			const symr *syp_2;
			int i_5;

			syp_2 = (symr *)&symbol->cbSym_Ptr[sizeof(symr) * fdrp->isymBase];
			pdrp_2 = (pdr *)&symbol->cbPd_Ptr[sizeof(pdr) * fdrp->ipdFirst];
			printf("      Procedures\n");
			for ( i_5 = 0; i_5 < fdrp->cpd; i_5 += 1 )
			{
				printf("      %3d: 0x%08x  %s\n", i_5, pdrp_2->adr, &issBase[syp_2[pdrp_2->isym].iss]);
				printf(
					"           regmask=0x%08x, regoffset=%d, fregmask=0x%08x, fregoffset=%d\n",
					pdrp_2->regmask,
					pdrp_2->regoffset,
					pdrp_2->fregmask,
					pdrp_2->fregoffset);
				printf(
					"           iopt=%d, frameoffset=%d, framereg=%d, pcreg=%d, line %d..%d\n",
					pdrp_2->iopt,
					pdrp_2->frameoffset,
					pdrp_2->framereg,
					pdrp_2->pcreg,
					pdrp_2->lnLow,
					pdrp_2->lnHigh);
				printf("           iline=%d, cbLineOffset=%d\n", pdrp_2->iline, (int)(pdrp_2->cbLineOffset));
				pdrp_2 += 1;
			}
		}
		if ( fdrp->caux > 0 )
		{
			const unsigned int *ip_1;
			int i_6;

			printf("      Auxillary symbol entries\n");
			ip_1 = (unsigned int *)&symbol->cbAux_Ptr[sizeof(unsigned int) * fdrp->iauxBase];
			for ( i_6 = 0; i_6 < fdrp->caux; i_6 += 1 )
			{
				printf("      %3d: 0x%08lx \n", i_6, (unsigned long)(*ip_1));
				ip_1 += 1;
			}
		}
		if ( fdrp->crfd > 0 )
		{
			const unsigned int *ip_2;
			int i_7;

			printf("      File indirect entries\n");
			ip_2 = (unsigned int *)&symbol->cbRfd_Ptr[sizeof(unsigned int) * fdrp->rfdBase];
			for ( i_7 = 0; i_7 < fdrp->crfd; i_7 += 1 )
			{
				printf("      %3d: 0x%08lx \n", i_7, (unsigned long)(*ip_2));
				ip_2 += 1;
			}
		}
		printf("\n");
		fdrp += 1;
	}
	printf("\n");
}
