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

typedef struct sect_org_data_
{
	unsigned int org_addr;
	unsigned int org_gp_value;
} Sect_org_data;

static int setup_start_entry(elf_file *elf, const char *entrysym, elf_section *modinfo, int needoutput);
static Elf_file_slot *search_order_slots(const char *ordstr, const elf_file *elf, Elf_file_slot *order);
static void fixlocation_an_rel(elf_section *relsect, unsigned int startaddr);
static void save_org_addrs(elf_file *elf);
static elf_section *add_iopmod(elf_file *elf);
static elf_section *add_eemod(elf_file *elf);
static void modify_eemod(elf_file *elf, elf_section *eemod);
static void add_reserved_symbol_table(
	Srx_gen_table *tp,
	const char *name,
	int bind,
	int type,
	SegConf *segment,
	const char *sectname,
	int shindex,
	int base);
static void define_special_section_symbols(elf_file *elf);
static void create_need_section(elf_file *elf);
static int sect_name_match(const char *pattern, const char *name);
static int reorder_section_table(elf_file *elf);
static void create_phdr(elf_file *elf);
static void check_change_bit(unsigned int oldbit, unsigned int newbit, unsigned int *up, unsigned int *down);
static void segment_start_setup(SegConf *seglist, unsigned int bitid, const unsigned int *moffset);
static void add_section_to_segment(SegConf *seglist, elf_section *scp, unsigned int bitid);
static void segment_end_setup(SegConf *seglist, unsigned int bitid, unsigned int *moffset, int ee);
static void update_modinfo(elf_file *elf);
static void update_mdebug(elf_file *elf);
static void update_programheader(elf_file *elf);
static void remove_unuse_section(elf_file *elf);
static int layout_srx_memory(elf_file *elf);
static CreateSymbolConf *is_reserve_symbol(Srx_gen_table *tp, const char *name);
static int check_undef_symboles_an_reloc(elf_section *relsect);
static int check_undef_symboles(elf_file *elf);
static int create_reserved_symbols(elf_file *elf);
static void symbol_value_update(elf_file *elf);
static void rebuild_relocation(elf_file *elf, unsigned int gpvalue);
static int check_irx12(elf_file *elf, int cause_irx1);
static void setup_module_info(elf_file *elf, elf_section *modsect, const char *modulesymbol);
static void rebuild_an_relocation(elf_section *relsect, unsigned int gpvalue, int target);
static size_t iopmod_size(const Elf32_IopMod *modinfo);
static size_t eemod_size(const Elf32_EeMod *modinfo);

int convert_rel2srx(elf_file *elf, const char *entrysym, int needoutput, int cause_irx1)
{
	Srx_gen_table *tp;
	elf_section *modinfo;

	tp = (Srx_gen_table *)elf->optdata;
	save_org_addrs(elf);
	switch ( tp->target )
	{
		case SRX_TARGET_IOP:
			modinfo = add_iopmod(elf);
			break;
		case SRX_TARGET_EE:
			modinfo = add_eemod(elf);
			break;
		default:
			fprintf(stderr, "Internal error: target unknown\n");
			exit(1);
	}
	remove_unuse_section(elf);
	define_special_section_symbols(elf);
	create_need_section(elf);
	reorder_section_table(elf);
	create_phdr(elf);
	if ( layout_srx_memory(elf) )
	{
		return 1;
	}
	modify_eemod(elf, modinfo);
	if ( create_reserved_symbols(elf) )
	{
		return 1;
	}
	if ( check_undef_symboles(elf) )
	{
		return 1;
	}
	symbol_value_update(elf);
	rebuild_relocation(elf, lookup_segment(tp, "GLOBALDATA", 1)->addr + 0x7FF0);
	if ( check_irx12(elf, cause_irx1) )
	{
		return 1;
	}
	if ( setup_start_entry(elf, entrysym, modinfo, needoutput) )
	{
		return 1;
	}
	{
		const char *module_info_symbol;
		const elf_syment *syp;

		module_info_symbol = "Module";
		syp = search_global_symbol("_irx_id", elf);
		if ( is_defined_symbol(syp) != 0 )
		{
			module_info_symbol = "_irx_id";
		}
		setup_module_info(elf, modinfo, module_info_symbol);
	}
	return layout_srx_file(elf);
}

static int setup_start_entry(elf_file *elf, const char *entrysym, elf_section *modinfo, int needoutput)
{
	const elf_syment *syp;

	if ( entrysym )
	{
		syp = search_global_symbol(entrysym, elf);
		if ( !is_defined_symbol(syp) )
		{
			fprintf(stderr, "Error: Cannot find entry symbol %s\n", entrysym);
			return 1;
		}
		elf->ehp->e_entry = get_symbol_value(syp, elf);
	}
	else
	{
		syp = search_global_symbol("start", elf);
		if ( !syp )
		{
			syp = search_global_symbol("_start", elf);
		}
		if ( !is_defined_symbol(syp) )
		{
			if ( modinfo->shr.sh_type == SHT_SCE_EEMOD )
			{
				elf->ehp->e_entry = -1;
			}
			else if ( needoutput )
			{
				fprintf(stderr, "warning: Cannot find entry symbol `start' and `_start'\n");
			}
		}
		else
		{
			elf->ehp->e_entry = get_symbol_value(syp, elf);
		}
	}
	switch ( modinfo->shr.sh_type )
	{
		case SHT_SCE_IOPMOD:
		case SHT_SCE_EEMOD:
			*((uint32_t *)modinfo->data + 1) = elf->ehp->e_entry;
			break;
		default:
			break;
	}
	return 0;
}

static Elf_file_slot *search_order_slots(const char *ordstr, const elf_file *elf, Elf_file_slot *order)
{
	elf_section **scp;

	if ( !strcmp(ordstr, "@Section_header_table") )
	{
		for ( ; order->type != EFS_TYPE_END; order += 1 )
		{
			if ( order->type == EFS_TYPE_SECTION_HEADER_TABLE )
			{
				return order;
			}
		}
		return 0;
	}
	if ( !strncmp(ordstr, "@Program_header_data ", 0x15) )
	{
		long n;

		n = strtol(ordstr + 21, NULL, 10);
		for ( ; order->type != EFS_TYPE_END; order += 1 )
		{
			if ( order->type == EFS_TYPE_PROGRAM_HEADER_ENTRY && order->d.php == &elf->php[n] )
			{
				return order;
			}
		}
		return 0;
	}
	for ( ; order->type != EFS_TYPE_END; order += 1 )
	{
		switch ( order->type )
		{
			case EFS_TYPE_PROGRAM_HEADER_ENTRY:
				for ( scp = order->d.php->scp; *scp; scp += 1 )
				{
					if ( !sect_name_match(ordstr, (*scp)->name) )
					{
						return order;
					}
				}
				break;
			case EFS_TYPE_SECTION_DATA:
				if ( !sect_name_match(ordstr, order->d.scp->name) )
				{
					return order;
				}
				break;
			default:
				break;
		}
	}
	return 0;
}

int layout_srx_file(elf_file *elf)
{
	elf_section **scp;
	const Srx_gen_table *tp;
	Elf_file_slot *slotp_1;
	Elf_file_slot *slotp_2;
	Elf_file_slot *nslotp;
	Elf_file_slot *neworder;
	Elf_file_slot *order;
	const char **ordstr;
	unsigned int max_seg_align;
	int error;
	int maxslot;

	tp = (Srx_gen_table *)elf->optdata;
	error = 0;
	switch ( tp->target )
	{
		case SRX_TARGET_IOP:
			max_seg_align = 16;
			break;
		case SRX_TARGET_EE:
			max_seg_align = 0x10000;
			break;
		default:
			fprintf(stderr, "Internal error: target unknown\n");
			return 1;
	}
	reorder_symtab(elf);
	rebuild_section_name_strings(elf);
	rebuild_symbol_name_strings(elf);
	order = build_file_order_list(elf);
	for ( maxslot = 0; order[maxslot].type != EFS_TYPE_END; maxslot += 1 )
	{
		;
	}
	neworder = (Elf_file_slot *)calloc(maxslot + 1, sizeof(Elf_file_slot));
	memcpy(neworder, order, sizeof(Elf_file_slot));
	nslotp = neworder + 1;
	order->type = EFS_TYPE_NONE;
	if ( elf->ehp->e_phnum )
	{
		memcpy(nslotp, &order[1], sizeof(Elf_file_slot));
		nslotp = neworder + 2;
		order[1].type = EFS_TYPE_NONE;
	}
	for ( ordstr = tp->file_layout_order; *ordstr; ordstr += 1 )
	{
		for ( ;; )
		{
			slotp_1 = search_order_slots(*ordstr, elf, order);
			if ( !slotp_1 )
			{
				break;
			}
			memcpy(nslotp, slotp_1, sizeof(Elf_file_slot));
			nslotp += 1;
			slotp_1->type = EFS_TYPE_NONE;
		}
	}
	nslotp->type = EFS_TYPE_END;
	shrink_file_order_list(neworder);
	writeback_file_order_list(elf, neworder);
	for ( slotp_2 = neworder; slotp_2->type != EFS_TYPE_END; slotp_2 += 1 )
	{
		if (
			slotp_2->type == EFS_TYPE_PROGRAM_HEADER_ENTRY && slotp_2->d.php->phdr.p_type == PT_LOAD
			&& max_seg_align < slotp_2->align )
		{
			fprintf(stderr, "Program Header Entry: unsupported align %u\n", slotp_2->align);
			error += 1;
			for ( scp = slotp_2->d.php->scp; *scp; scp += 1 )
			{
				if ( max_seg_align < (*scp)->shr.sh_addralign )
				{
					fprintf(
						stderr, "Section '%s' : unsupported section align %d\n", (*scp)->name, (int)((*scp)->shr.sh_addralign));
					error += 1;
				}
			}
		}
	}

	if (error && tp->target == SRX_TARGET_IOP)
	{
		fprintf(stderr, "LOADCORE limits possible alignment to 16 bytes\n");
	}

	free(order);
	free(neworder);
	return error;
}

void strip_elf(elf_file *elf)
{
	elf_syment **syp;
	elf_section *scp;
	unsigned int entrise;
	unsigned int d;
	unsigned int s;

	remove_section(elf, SHT_MIPS_DEBUG);
	scp = search_section(elf, SHT_SYMTAB);
	if ( scp == NULL )
	{
		return;
	}
	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	syp = (elf_syment **)scp->data;
	d = 1;
	for ( s = 1; s < entrise; s += 1 )
	{
		if ( syp[s]->refcount <= 0 )
		{
			syp[s]->number = -1;
		}
		else
		{
			syp[d] = syp[s];
			d += 1;
		}
	}
	scp->shr.sh_size = d * scp->shr.sh_entsize;
}

SegConf *lookup_segment(Srx_gen_table *conf, const char *segname, int msgsw)
{
	SegConf *i;

	for ( i = conf->segment_list; i->name; i += 1 )
	{
		if ( !strcmp(segname, i->name) )
		{
			return i;
		}
	}
	if ( msgsw )
	{
		fprintf(stderr, "segment '%s' not found \n", segname);
	}
	return 0;
}

static void fixlocation_an_rel(elf_section *relsect, unsigned int startaddr)
{
	int daddr1;
	unsigned int data_1;
	uint32_t data_2;
	int data_3;
	unsigned int data_4;
	uint16_t data_5;
	elf_syment **symp;
	elf_rel *rp;
	unsigned int entrise;
	unsigned int i;

	entrise = relsect->shr.sh_size / relsect->shr.sh_entsize;
	rp = (elf_rel *)relsect->data;
	symp = (elf_syment **)relsect->link->data;
	for ( i = 0; i < entrise; i += 1 )
	{
		uint8_t *datal;

		if ( rp->symptr && *symp != rp->symptr )
		{
			fprintf(stderr, "Internal error: Illegal relocation entry\n");
			exit(1);
		}
		if (
			relsect->info->shr.sh_addr > rp->rel.r_offset
			|| rp->rel.r_offset >= relsect->info->shr.sh_size + relsect->info->shr.sh_addr )
		{
			fprintf(
				stderr,
				"Panic !! relocation #%u offset=0x%x range out (section limit addr=0x%x-0x%x)\n",
				i,
				rp->rel.r_offset,
				relsect->info->shr.sh_addr,
				relsect->info->shr.sh_size + relsect->info->shr.sh_addr);
			exit(1);
		}
		datal = &relsect->info->data[rp->rel.r_offset - relsect->info->shr.sh_addr];
		switch ( rp->type )
		{
			case R_MIPS_16:
				data_1 = startaddr + (int16_t)*(uint32_t *)datal;
				if ( (uint16_t)(data_1 >> 16) )
				{
					if ( (uint16_t)(data_1 >> 16) != 0xFFFF )
					{
						fprintf(stderr, "REFHALF data overflow\n");
						exit(1);
					}
				}
				*(uint32_t *)datal &= 0xFFFF0000;
				*(uint32_t *)datal |= (uint16_t)data_1;
				break;
			case R_MIPS_32:
				*(uint32_t *)datal += startaddr;
				break;
			case R_MIPS_26:
				if ( rp->symptr && rp->symptr->bind != STB_LOCAL )
				{
					fprintf(stderr, "R_MIPS_26 Unexcepted bind\n");
					exit(1);
				}
				data_2 = startaddr + ((rp->rel.r_offset & 0xF0000000) | (4 * (*(uint32_t *)datal & 0x3FFFFFF)));
				*(uint32_t *)datal &= 0xFC000000;
				*(uint32_t *)datal |= (16 * data_2) >> 6;
				break;
			case R_MIPS_HI16:
				if ( i == entrise + 1 || rp[1].type != R_MIPS_LO16 || (rp->symptr && rp[1].symptr != rp->symptr) )
				{
					fprintf(stderr, "R_MIPS_HI16 without R_MIPS_LO16\n");
					exit(1);
				}
				data_4 = startaddr + (int16_t)*(uint32_t *)&relsect->info->data[rp[1].rel.r_offset - relsect->info->shr.sh_addr]
							 + (*(uint32_t *)datal << 16);
				*(uint32_t *)datal &= 0xFFFF0000;
				*(uint32_t *)datal |= (uint16_t)(((data_4 >> 15) + 1) >> 1);
				break;
			case R_MIPS_LO16:
				data_5 = startaddr + *(uint32_t *)datal;
				*(uint32_t *)datal &= 0xFFFF0000;
				*(uint32_t *)datal |= data_5;
				break;
			case R_MIPS_GPREL16:
				fprintf(stderr, "Unexcepted R_MIPS_GPREL16\n");
				exit(1);
				return;
			case R_MIPS_LITERAL:
				fprintf(stderr, "Unexcepted R_MIPS_LITERAL\n");
				exit(1);
				return;
			case R_MIPSSCE_MHI16:
				if ( i == entrise + 1 || rp[1].type != R_MIPSSCE_ADDEND )
				{
					fprintf(stderr, "R_MIPSSCE_MHI16 without R_MIPSSCE_ADDEND\n");
					exit(1);
				}
				data_3 = (uint16_t)((((startaddr + rp[1].rel.r_offset) >> 15) + 1) >> 1);
				for ( daddr1 = 1; daddr1; datal += daddr1 )
				{
					daddr1 = *(uint16_t *)datal << 16 >> 14;
					*(uint32_t *)datal &= 0xFFFF0000;
					*(uint32_t *)datal |= data_3;
				}
				rp += 1;
				i += 1;
				break;
			case R_MIPS_REL32:
			case R_MIPS_GOT16:
			case R_MIPS_PC16:
			case R_MIPS_CALL16:
			case R_MIPS_GPREL32:
			case R_MIPS_GOTHI16:
			case R_MIPS_GOTLO16:
			case R_MIPS_CALLHI16:
			case R_MIPS_CALLLO16:
				fprintf(stderr, "unacceptable relocation type: 0x%x\n", rp->type);
				exit(1);
				return;
			default:
				fprintf(stderr, "unknown relocation type: 0x%x\n", rp->type);
				exit(1);
				return;
		}
		rp += 1;
	}
}

void fixlocation_elf(elf_file *elf, unsigned int startaddr)
{
	unsigned int entrise;
	int i;
	unsigned int k;
	int d;
	int s;
	int j;
	elf_syment **syp;
	elf_section *scp;
	elf_section *modsect_1;
	elf_section *modsect_2;

	if ( elf->scp == NULL )
	{
		return;
	}
	d = 1;
	for ( s = 1; s < elf->ehp->e_shnum; s += 1 )
	{
		if ( elf->scp[s]->shr.sh_type == SHT_REL )
		{
			fixlocation_an_rel(elf->scp[s], startaddr);
		}
		else
		{
			elf->scp[d] = elf->scp[s];
			d += 1;
		}
	}
	elf->ehp->e_shnum = d;
	elf->ehp->e_entry += startaddr;
	modsect_1 = search_section(elf, SHT_SCE_IOPMOD);
	if ( modsect_1 )
	{
		Elf32_IopMod *iopmodinfo;

		iopmodinfo = (Elf32_IopMod *)modsect_1->data;
		if ( iopmodinfo->moduleinfo != (Elf32_Word)(-1) )
		{
			iopmodinfo->moduleinfo += startaddr;
		}
		iopmodinfo->entry += startaddr;
		iopmodinfo->gp_value += startaddr;
	}
	modsect_2 = search_section(elf, SHT_SCE_EEMOD);
	if ( modsect_2 )
	{
		Elf32_EeMod *eemodinfo;

		eemodinfo = (Elf32_EeMod *)modsect_2->data;
		if ( eemodinfo->moduleinfo != (Elf32_Word)(-1) )
		{
			eemodinfo->moduleinfo += startaddr;
		}
		eemodinfo->entry += startaddr;
		eemodinfo->gp_value += startaddr;
	}
	for ( i = 0; i < elf->ehp->e_phnum; i += 1 )
	{
		if ( elf->php[i].phdr.p_type == PT_LOAD )
		{
			elf->php[i].phdr.p_vaddr = startaddr;
			elf->php[i].phdr.p_paddr = startaddr;
			break;
		}
	}
	for ( j = 1; j < elf->ehp->e_shnum; j += 1 )
	{
		switch ( elf->scp[j]->shr.sh_type )
		{
			case SHT_PROGBITS:
			case SHT_NOBITS:
				elf->scp[j]->shr.sh_addr += startaddr;
				break;
			default:
				break;
		}
	}
	scp = search_section(elf, SHT_SYMTAB);
	if ( scp == NULL )
	{
		return;
	}
	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	syp = (elf_syment **)scp->data;
	for ( k = 1; k < entrise; k += 1 )
	{
		if ( syp[k]->sym.st_shndx == SHN_RADDR || (syp[k]->sym.st_shndx && syp[k]->sym.st_shndx <= 0xFEFF) )
		{
			syp[k]->sym.st_value += startaddr;
		}
		if ( syp[k]->sym.st_shndx == SHN_RADDR )
		{
			syp[k]->sym.st_shndx = -15;
		}
	}
}

static void save_org_addrs(elf_file *elf)
{
	Elf32_RegInfo *data;
	const Elf32_RegInfo *reginfop;
	elf_section *reginfosec;
	int i;

	reginfosec = search_section(elf, SHT_MIPS_REGINFO);
	if ( reginfosec )
	{
		data = (Elf32_RegInfo *)reginfosec->data;
	}
	else
	{
		data = 0;
	}
	reginfop = data;
	for ( i = 1; i < elf->ehp->e_shnum; i += 1 )
	{
		Sect_org_data *org;

		org = (Sect_org_data *)calloc(1, sizeof(Sect_org_data));
		org->org_addr = elf->scp[i]->shr.sh_addr;
		if ( reginfop )
		{
			org->org_gp_value = reginfop->ri_gp_value;
		}
		elf->scp[i]->optdata = (void *)org;
	}
}

static elf_section *add_iopmod(elf_file *elf)
{
	Elf32_IopMod *iopmodp;
	elf_section *modsect;

	modsect = (elf_section *)malloc(sizeof(elf_section));
	memset(modsect, 0, sizeof(elf_section));
	iopmodp = (Elf32_IopMod *)malloc(sizeof(Elf32_IopMod));
	memset(iopmodp, 0, sizeof(Elf32_IopMod));
	iopmodp->moduleinfo = -1;
	modsect->name = strdup(".iopmod");
	modsect->data = (uint8_t *)iopmodp;
	modsect->shr.sh_type = SHT_SCE_IOPMOD;
	modsect->shr.sh_size = iopmod_size(iopmodp);
	modsect->shr.sh_addralign = 4;
	modsect->shr.sh_entsize = 0;
	add_section(elf, modsect);
	return modsect;
}

static elf_section *add_eemod(elf_file *elf)
{
	Elf32_EeMod *eemodp;
	elf_section *modsect;

	modsect = (elf_section *)malloc(sizeof(elf_section));
	memset(modsect, 0, sizeof(elf_section));
	eemodp = (Elf32_EeMod *)malloc(sizeof(Elf32_EeMod));
	memset(eemodp, 0, sizeof(Elf32_EeMod));
	eemodp->moduleinfo = -1;
	modsect->name = strdup(".eemod");
	modsect->data = (uint8_t *)eemodp;
	modsect->shr.sh_type = SHT_SCE_EEMOD;
	modsect->shr.sh_size = eemod_size(eemodp);
	modsect->shr.sh_addralign = 4;
	modsect->shr.sh_entsize = 0;
	add_section(elf, modsect);
	return modsect;
}

static void modify_eemod(elf_file *elf, elf_section *eemod)
{
	elf_section *scp_1;
	elf_section *scp_2;
	Elf32_EeMod *moddata;

	if ( eemod->shr.sh_type != SHT_SCE_EEMOD )
	{
		return;
	}
	moddata = (Elf32_EeMod *)eemod->data;
	scp_1 = search_section_by_name(elf, ".erx.lib");
	if ( scp_1 )
	{
		moddata->erx_lib_addr = scp_1->shr.sh_addr;
		moddata->erx_lib_size = scp_1->shr.sh_size;
	}
	else
	{
		moddata->erx_lib_addr = -1;
		moddata->erx_lib_size = 0;
	}
	scp_2 = search_section_by_name(elf, ".erx.stub");
	if ( scp_2 )
	{
		moddata->erx_stub_addr = scp_2->shr.sh_addr;
		moddata->erx_stub_size = scp_2->shr.sh_size;
	}
	else
	{
		moddata->erx_stub_addr = -1;
		moddata->erx_stub_size = 0;
	}
}

static void add_reserved_symbol_table(
	Srx_gen_table *tp,
	const char *name,
	int bind,
	int type,
	SegConf *segment,
	const char *sectname,
	int shindex,
	int base)
{
	CreateSymbolConf *newent_1;
	CreateSymbolConf *newent_2;
	int entries;

	entries = 1;
	for ( newent_1 = tp->create_symbols; newent_1->name; newent_1 += 1 )
	{
		entries += 1;
	}
	tp->create_symbols = (CreateSymbolConf *)realloc(tp->create_symbols, (entries + 1) * sizeof(CreateSymbolConf));
	memset(&tp->create_symbols[entries], 0, sizeof(tp->create_symbols[entries]));
	newent_2 = &tp->create_symbols[entries - 1];
	newent_2->name = strdup(name);
	newent_2->bind = bind;
	newent_2->type = type;
	newent_2->segment = segment;
	newent_2->sectname = strdup(sectname);
	newent_2->shindex = shindex;
	newent_2->seflag = base;
}

const char *bos_str = "_begin_of_section_";
const char *eos_str = "_end_of_section_";
static void define_special_section_symbols(elf_file *elf)
{
	char *sectname;
	const elf_syment *sym;
	elf_syment **syp;
	elf_section *scp;
	unsigned int entrise;
	unsigned int i;
	Srx_gen_table *tp;

	tp = (Srx_gen_table *)(elf->optdata);
	scp = search_section(elf, SHT_SYMTAB);
	if ( scp == NULL )
	{
		return;
	}
	sectname = (char *)__builtin_alloca(((elf->shstrptr->shr.sh_size + 22) >> 2) << 2);
	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	syp = (elf_syment **)(scp->data);
	for ( i = 1; i < entrise; i += 1 )
	{
		sym = syp[i];
		if ( sym->bind == STB_GLOBAL && !sym->sym.st_shndx )
		{
			if ( !strncmp(bos_str, sym->name, strlen(bos_str)) )
			{
				strcpy(sectname, ".");
				strcat(sectname, &sym->name[strlen(bos_str)]);
				add_reserved_symbol_table(tp, sym->name, 2, 1, 0, sectname, 0, 0);
			}
			if ( !strncmp(eos_str, sym->name, strlen(eos_str)) )
			{
				strcpy(sectname, ".");
				strcat(sectname, &sym->name[strlen(eos_str)]);
				add_reserved_symbol_table(tp, sym->name, 2, 1, 0, sectname, 65311, 1);
			}
		}
	}
}

static void create_need_section(elf_file *elf)
{
	elf_section *scp;
	unsigned int i;
	CreateSymbolConf *csym;
	Srx_gen_table *tp;

	tp = (Srx_gen_table *)elf->optdata;
	scp = search_section(elf, SHT_SYMTAB);
	if ( scp == NULL )
	{
		return;
	}
	for ( i = 1; i < (scp->shr.sh_size / scp->shr.sh_entsize); i += 1 )
	{
		const elf_syment *syment;

		syment = *(elf_syment **)&scp->data[i * sizeof(void *)];
		if ( !syment->sym.st_shndx )
		{
			csym = is_reserve_symbol(tp, syment->name);
			if ( csym )
			{
				if ( csym->segment )
				{
					elf_section *addscp_1;

					addscp_1 = csym->segment->empty_section;
					if ( addscp_1 )
					{
						if ( !search_section_by_name(elf, addscp_1->name) )
						{
							add_section(elf, addscp_1);
						}
					}
				}
				else if ( csym->sectname && !search_section_by_name(elf, csym->sectname) )
				{
					SectConf *odr;

					for ( odr = tp->section_list; odr->sect_name_pattern; odr += 1 )
					{
						if ( !sect_name_match(odr->sect_name_pattern, csym->sectname) && odr->secttype && odr->sectflag )
						{
							elf_section *addscp_2;

							addscp_2 = (elf_section *)calloc(1, sizeof(elf_section));
							addscp_2->name = strdup(csym->sectname);
							addscp_2->shr.sh_type = odr->secttype;
							addscp_2->shr.sh_flags = odr->sectflag;
							addscp_2->shr.sh_size = 0;
							addscp_2->shr.sh_addralign = 4;
							addscp_2->shr.sh_entsize = 0;
							add_section(elf, addscp_2);
							break;
						}
					}
				}
			}
		}
	}
}

static int sect_name_match(const char *pattern, const char *name)
{
	for ( ; *pattern && *name && *pattern != '*'; pattern += 1, name += 1 )
	{
		if ( *name > *pattern )
		{
			return -1;
		}
		if ( *name < *pattern )
		{
			return 1;
		}
	}
	if ( !*pattern && !*name )
	{
		return 0;
	}
	if ( *pattern == '*' && !pattern[1] )
	{
		return 0;
	}
	if ( *pattern != '*' )
	{
		return strcmp(pattern, name);
	}
	pattern += 1;
	if ( strlen(name) < strlen(pattern) )
	{
		return strcmp(pattern, name);
	}
	return strcmp(pattern, &name[strlen(name) - strlen(pattern)]);
}

static int reorder_section_table(elf_file *elf)
{
	const char **secorder;
	int sections;
	elf_section **scp;
	int d;
	int s;

	sections = elf->ehp->e_shnum;
	secorder = ((Srx_gen_table *)elf->optdata)->section_table_order;
	scp = (elf_section **)calloc(sections + 1, sizeof(elf_section *));
	memcpy(scp, elf->scp, sections * sizeof(elf_section *));
	*elf->scp = *scp;
	d = 1;
	for ( ; *secorder; secorder += 1 )
	{
		for ( s = 1; s < sections; s += 1 )
		{
			if ( scp[s] )
			{
				if ( !sect_name_match(*secorder, scp[s]->name) )
				{
					elf->scp[d] = scp[s];
					scp[s] = 0;
					d += 1;
				}
			}
		}
	}
	free(scp);
	reorder_symtab(elf);
	return 0;
}

static void create_phdr(elf_file *elf)
{
	const PheaderInfo *phip;
	int i;

	phip = ((Srx_gen_table *)(elf->optdata))->program_header_order;
	for ( i = 0; phip[i].sw; i += 1 )
	{
		;
	}
	elf->php = (i != 0) ? (elf_proghead *)malloc(i * sizeof(elf_proghead)) : NULL;
	if ( elf->php != NULL )
	{
		memset(elf->php, 0, i * sizeof(elf_proghead));
	}
	elf->ehp->e_phentsize = sizeof(Elf32_Phdr);
	elf->ehp->e_phnum = i;
	if ( elf->php != NULL )
	{
		int j;

		for ( j = 0; phip[j].sw; j += 1 )
		{
			switch ( phip[j].sw )
			{
				case SRX_PH_TYPE_MOD:
					elf->php[j].phdr.p_flags = PF_R;
					elf->php[j].phdr.p_align = 4;
					if ( !strcmp(".iopmod", phip[j].d.section_name) )
					{
						elf->php[j].phdr.p_type = PT_SCE_IOPMOD;
						elf->php[j].phdr.p_filesz = sizeof(Elf32_IopMod);
						elf->php[j].scp = (elf_section **)calloc(2, sizeof(elf_section *));
						*elf->php[j].scp = search_section(elf, SHT_SCE_IOPMOD);
					}
					else if ( !strcmp(".eemod", phip[j].d.section_name) )
					{
						elf->php[j].phdr.p_type = PT_SCE_EEMOD;
						elf->php[j].phdr.p_filesz = sizeof(Elf32_EeMod);
						elf->php[j].scp = (elf_section **)calloc(2, sizeof(elf_section *));
						*elf->php[j].scp = search_section(elf, SHT_SCE_EEMOD);
					}
					else
					{
						fprintf(stderr, "Unsuport section '%s' for program header\n", phip[j].d.section_name);
					}
					break;
				case SRX_PH_TYPE_TEXT:
					elf->php[j].phdr.p_type = PT_LOAD;
					elf->php[j].phdr.p_flags = PF_X | PF_W | PF_R;
					elf->php[j].phdr.p_align = 16;
					break;
				default:
					break;
			}
		}
	}
}

static void check_change_bit(unsigned int oldbit, unsigned int newbit, unsigned int *up, unsigned int *down)
{
	*up = ~oldbit & newbit & (newbit ^ oldbit);
	*down = ~newbit & oldbit & (newbit ^ oldbit);
}

static void segment_start_setup(SegConf *seglist, unsigned int bitid, const unsigned int *moffset)
{
	for ( ; seglist->name; seglist += 1 )
	{
		if ( (seglist->bitid & bitid) != 0 )
		{
			seglist->addr = *moffset;
			seglist->size = 0;
		}
	}
}

static void add_section_to_segment(SegConf *seglist, elf_section *scp, unsigned int bitid)
{
	for ( ; seglist->name; seglist += 1 )
	{
		if ( (seglist->bitid & bitid) != 0 )
		{
			if ( !seglist->nsect )
			{
				seglist->addr = scp->shr.sh_addr;
			}
			seglist->nsect += 1;
			seglist->scp = (elf_section **)realloc(seglist->scp, (seglist->nsect + 1) * sizeof(elf_section *));
			seglist->scp[seglist->nsect - 1] = scp;
			seglist->scp[seglist->nsect] = 0;
		}
	}
}

static void segment_end_setup(SegConf *seglist, unsigned int bitid, unsigned int *moffset, int ee)
{
	for ( ; seglist->name; seglist += 1 )
	{
		if ( (seglist->bitid & bitid) != 0 )
		{
			if ( ee )
			{
				if ( !strcmp(seglist->name, "TEXT") )
				{
					*moffset += 32;
				}
			}
			seglist->size = *moffset - seglist->addr;
		}
	}
}

static void update_modinfo(elf_file *elf)
{
	Srx_gen_table *tp;
	const SegConf *seginfo;
	const SegConf *seginfo_4;
	const SegConf *seginfo_8;
	const SegConf *seginfo_12;
	elf_section *scp_1;
	elf_section *scp_2;

	tp = (Srx_gen_table *)elf->optdata;
	seginfo = lookup_segment(tp, "TEXT", 1);
	seginfo_4 = lookup_segment(tp, "DATA", 1);
	seginfo_8 = lookup_segment(tp, "BSS", 1);
	seginfo_12 = lookup_segment(tp, "GLOBALDATA", 1);
	if ( !seginfo || !seginfo_4 || !seginfo_8 || !seginfo_12 )
	{
		fprintf(stderr, "TEXT,DATA,BSS,GLOBALDATA segment missing abort");
		exit(1);
	}
	scp_1 = search_section(elf, SHT_SCE_IOPMOD);
	if ( scp_1 )
	{
		Elf32_IopMod *imp;

		scp_1->shr.sh_addr = 0;
		imp = (Elf32_IopMod *)scp_1->data;
		imp->text_size = seginfo_4->addr - seginfo->addr;
		imp->data_size = seginfo_8->addr - seginfo_4->addr;
		imp->bss_size = seginfo_8->size;
		imp->gp_value = seginfo_12->addr + 0x7FF0;
	}
	scp_2 = search_section(elf, SHT_SCE_EEMOD);
	if ( scp_2 )
	{
		Elf32_EeMod *emp;

		scp_2->shr.sh_addr = 0;
		emp = (Elf32_EeMod *)scp_2->data;
		emp->text_size = seginfo_4->addr - seginfo->addr;
		emp->data_size = seginfo_8->addr - seginfo_4->addr;
		emp->bss_size = seginfo_8->size;
		emp->gp_value = seginfo_12->addr + 0x7FF0;
	}
}

static void update_mdebug(elf_file *elf)
{
	elf_section *scp;

	scp = search_section(elf, SHT_MIPS_DEBUG);
	if ( scp )
	{
		scp->shr.sh_addr = 0;
	}
}

static void update_programheader(elf_file *elf)
{
	Srx_gen_table *tp;
	SegConf **segp;
	elf_section **scp;
	PheaderInfo *phip;
	unsigned int minsegalign;
	unsigned int align;
	int nsect_1;
	int nsect_2;
	int nseg_1;
	int nseg_2;
	int s;
	int n;

	tp = (Srx_gen_table *)elf->optdata;
	phip = tp->program_header_order;
	switch ( tp->target )
	{
		case SRX_TARGET_IOP:
			minsegalign = 16;
			break;
		case SRX_TARGET_EE:
			minsegalign = 128;
			break;
		default:
			fprintf(stderr, "Internal error: target unknown\n");
			exit(1);
			return;
	}
	for ( n = 0; phip[n].sw; n += 1 )
	{
		if ( phip[n].sw == SRX_PH_TYPE_TEXT )
		{
			segp = phip[n].d.segment_list;
			if ( segp )
			{
				elf->php[n].phdr.p_vaddr = (*segp)->addr;
				align = minsegalign;
				for ( nsect_1 = 0, nseg_1 = 0; segp[nseg_1]; nsect_1 += segp[nseg_1]->nsect, nseg_1 += 1 )
				{
					;
				}
				scp = (elf_section **)calloc(nsect_1 + 1, sizeof(elf_section *));
				elf->php[n].scp = scp;
				for ( nsect_2 = 0, nseg_2 = 0; segp[nseg_2]; nsect_2 += segp[nseg_2]->nsect, nseg_2 += 1 )
				{
					memcpy(&scp[nsect_2], segp[nseg_2]->scp, segp[nseg_2]->nsect * sizeof(elf_section *));
				}
				for ( s = 0; s < nsect_2 && scp[s]->shr.sh_type == SHT_PROGBITS; s += 1 )
				{
					if ( scp[s]->shr.sh_addralign > align )
					{
						align = scp[s]->shr.sh_addralign;
					}
				}
				elf->php[n].phdr.p_filesz = scp[s - 1]->shr.sh_size + scp[s - 1]->shr.sh_addr - (*scp)->shr.sh_addr;
				elf->php[n].phdr.p_memsz = scp[nsect_2 - 1]->shr.sh_size + scp[nsect_2 - 1]->shr.sh_addr - (*scp)->shr.sh_addr;
				for ( ; s < nsect_2; s += 1 )
				{
					if ( scp[s]->shr.sh_addralign > align )
					{
						align = scp[s]->shr.sh_addralign;
					}
				}
				elf->php[n].phdr.p_align = align;
			}
		}
	}
}

static void remove_unuse_section(elf_file *elf)
{
	const char **sectnames;
	int sections;
	elf_section **dscp;
	int d;
	int i;
	int j;

	sections = elf->ehp->e_shnum;
	sectnames = ((Srx_gen_table *)(elf->optdata))->removesection_list;
	dscp = (elf_section **)calloc(sections + 1, sizeof(elf_section *));
	memset(dscp, 0, sections * sizeof(elf_section *));
	d = 0;
	for ( ; *sectnames; sectnames += 1 )
	{
		for ( i = 1; i < sections; i += 1 )
		{
			if ( elf->scp[i] )
			{
				if ( !sect_name_match(*sectnames, elf->scp[i]->name) )
				{
					dscp[d] = elf->scp[i];
					d += 1;
				}
			}
		}
	}
	for ( j = 0; j < sections; j += 1 )
	{
		if ( dscp[j] )
		{
			remove_section_by_name(elf, dscp[j]->name);
		}
	}
	free(dscp);
}

static int layout_srx_memory(elf_file *elf)
{
	int s_3;
	elf_section **scp;
	SectConf *odr;
	Srx_gen_table *tp;
	int error;
	int is_ee;
	int sections;
	int s_1;
	int s_2;
	unsigned int downdelta;
	unsigned int updelta;
	unsigned int oldbitid;
	unsigned int moffset;

	tp = (Srx_gen_table *)elf->optdata;
	moffset = 0;
	oldbitid = 0;
	error = 0;
	is_ee = tp->target == SRX_TARGET_EE;
	if ( !elf->scp )
	{
		return 1;
	}
	sections = elf->ehp->e_shnum;
	scp = (elf_section **)calloc(sections + 1, sizeof(elf_section *));
	memcpy(scp, elf->scp, sections * sizeof(elf_section *));
	for ( odr = tp->section_list; odr->sect_name_pattern; odr += 1 )
	{
		check_change_bit(oldbitid, odr->flag, &updelta, &downdelta);
		if ( updelta )
		{
			segment_start_setup(tp->segment_list, updelta, &moffset);
		}
		for ( s_1 = 1; s_1 < sections; s_1 += 1 )
		{
			if (
				scp[s_1] && !sect_name_match(odr->sect_name_pattern, scp[s_1]->name)
				&& (scp[s_1]->shr.sh_flags & SHF_ALLOC) != 0 )
			{
				moffset = adjust_align(moffset, scp[s_1]->shr.sh_addralign);
				scp[s_1]->shr.sh_addr = moffset;
				moffset += scp[s_1]->shr.sh_size;
				add_section_to_segment(tp->segment_list, scp[s_1], odr->flag);
				scp[s_1] = 0;
			}
		}
		oldbitid = odr->flag;
		check_change_bit(oldbitid, odr[1].flag, &updelta, &downdelta);
		if ( downdelta )
		{
			segment_end_setup(tp->segment_list, downdelta, &moffset, is_ee);
		}
	}
	for ( s_2 = 1; s_2 < sections; s_2 += 1 )
	{
		if ( scp[s_2] && (scp[s_2]->shr.sh_flags & SHF_ALLOC) != 0 )
		{
			for (
				s_3 = 1;
				s_3 < sections
				&& (!scp[s_3] || (scp[s_3]->shr.sh_type != SHT_RELA && scp[s_3]->shr.sh_type != SHT_REL) || scp[s_2] != scp[s_3]->info);
				s_3 += 1 )
			{
				;
			}
			if ( sections > s_3 )
			{
				fprintf(
					stderr,
					"Error: section '%s' needs allocation and has relocation data but not in program segment\n",
					scp[s_2]->name);
				scp[s_3] = 0;
				error += 1;
			}
			else
			{
				fprintf(stderr, "Warning: section '%s' needs allocation but not in program segment\n", scp[s_2]->name);
				scp[s_2] = 0;
			}
		}
	}
	free(scp);
	if ( !error )
	{
		update_modinfo(elf);
		update_mdebug(elf);
		update_programheader(elf);
	}
	return error;
}

static CreateSymbolConf *is_reserve_symbol(Srx_gen_table *tp, const char *name)
{
	CreateSymbolConf *csyms;

	if ( !name )
	{
		return 0;
	}
	for ( csyms = tp->create_symbols; csyms->name; csyms += 1 )
	{
		if ( !strcmp(csyms->name, name) )
		{
			return csyms;
		}
	}
	return 0;
}

static int check_undef_symboles_an_reloc(elf_section *relsect)
{
	elf_syment **symp;
	elf_rel *rp;
	int undefcount;
	unsigned int entrise;
	unsigned int i;

	entrise = relsect->shr.sh_size / relsect->shr.sh_entsize;
	rp = (elf_rel *)relsect->data;
	symp = (elf_syment **)relsect->link->data;
	undefcount = 0;
	for ( i = 0; i < entrise; i += 1 )
	{
		if ( rp->symptr && *symp != rp->symptr )
		{
			if ( rp->symptr->sym.st_shndx )
			{
				if (
					rp->symptr->sym.st_shndx > 0xFEFF && rp->symptr->sym.st_shndx != SHN_ABS
					&& rp->symptr->sym.st_shndx != SHN_RADDR )
				{
					if ( rp->symptr->sym.st_shndx == SHN_COMMON )
					{
						fprintf(stderr, "  unallocated variable `%s'\n", rp->symptr->name);
					}
					else
					{
						fprintf(stderr, "  `%s' unknown symbol type %x\n", rp->symptr->name, rp->symptr->sym.st_shndx);
					}
					undefcount += 1;
				}
			}
			else if ( rp->symptr->bind != STB_WEAK )
			{
				fprintf(stderr, "  undefined reference to `%s'\n", rp->symptr->name);
				undefcount += 1;
			}
		}
		rp += 1;
	}
	return undefcount;
}

static int check_undef_symboles(elf_file *elf)
{
	int err;
	int s;

	if ( !elf->scp )
	{
		return 0;
	}
	err = 0;
	for ( s = 1; s < elf->ehp->e_shnum; s += 1 )
	{
		if ( elf->scp[s]->shr.sh_type == SHT_REL )
		{
			err += check_undef_symboles_an_reloc(elf->scp[s]);
		}
	}
	return err;
}

#if 0
// clang-format off
static const char * const SymbolType[] =
{
#define X(d) #d,
	XEACH_SymbolType_enum()
#undef X
};
// clang-format on
#endif
static int create_reserved_symbols(elf_file *elf)
{
	int csyms_;
	unsigned int sh_size;
	unsigned int sh_addr;
	elf_section *scp;
	CreateSymbolConf *csyms;
	Srx_gen_table *tp;

	tp = (Srx_gen_table *)elf->optdata;
	csyms_ = 0;
	if ( !search_section(elf, SHT_SYMTAB) )
	{
		return 1;
	}
	for ( csyms = tp->create_symbols; csyms->name; csyms += 1 )
	{
		elf_syment *sym;

		sym = search_global_symbol(csyms->name, elf);
		if ( !sym
			&& (csyms->shindex > 0xFEFF
			 || (csyms->segment && csyms->segment->scp)
			 || (!csyms->segment && csyms->sectname && search_section_by_name(elf, csyms->sectname))) )
		{
			sym = add_symbol(elf, csyms->name, csyms->bind, 0, 0, 0, 0);
		}
		if ( sym )
		{
			sh_size = 0;
			sh_addr = 0;
			scp = 0;
			if ( csyms->segment )
			{
				sh_addr = csyms->segment->addr;
				sh_size = csyms->segment->size;
				if ( csyms->shindex <= 0xFEFF )
				{
					scp = *csyms->segment->scp;
				}
			}
			else if ( csyms->sectname != NULL )
			{
				scp = search_section_by_name(elf, csyms->sectname);
				if ( scp )
				{
					sh_addr = scp->shr.sh_addr;
					sh_size = scp->shr.sh_size;
				}
			}
			if ( csyms->segment || scp )
			{
				// FIXME: disable this check because it trips on _gp, _end symbols
#if 0
				if ( sym->sym.st_shndx )
				{
					fprintf(stderr, "Unexcepted Symbol \"%s\":%s \n", sym->name, SymbolType[sym->type]);
					csyms_ += 1;
				}
				else
#endif
				{
					sym->bind = csyms->bind;
					if ( !sym->type )
					{
						sym->type = csyms->type;
					}
					sym->sym.st_info = ((csyms->bind & 0xFF) << 4) + (csyms->type & 0xF);
					if ( csyms->shindex > 0xFEFF )
					{
						sym->sym.st_shndx = csyms->shindex;
						sym->shptr = 0;
						switch ( csyms->seflag )
						{
							case 0:
								sym->sym.st_value = sh_addr;
								break;
							case 1:
								sym->sym.st_value = sh_size + sh_addr;
								break;
							case 2:
								sym->sym.st_value = sh_addr + 0x7FF0;
								break;
							default:
								break;
						}
					}
					else
					{
						sym->sym.st_shndx = 1;
						sym->shptr = scp;
						switch ( csyms->seflag )
						{
							case 0:
								sym->sym.st_value = 0;
								break;
							case 1:
								sym->sym.st_value = sh_size;
								break;
							default:
								break;
						}
					}
				}
				continue;
			}
			if ( csyms->bind == STB_WEAK && (sym->bind == STB_GLOBAL || sym->bind == STB_WEAK) && !sym->sym.st_shndx )
			{
				if ( !sym->type )
				{
					sym->type = csyms->type;
				}
				sym->bind = csyms->bind;
				sym->sym.st_info = ((csyms->bind & 0xFF) << 4) + (csyms->type & 0xF);
			}
		}
	}
	return csyms_;
}

static void symbol_value_update(elf_file *elf)
{
	unsigned int entrise;
	unsigned int i;
	elf_syment **syp;
	elf_section *scp;
	int target;

	target = ((Srx_gen_table *)(elf->optdata))->target;
	if ( elf->ehp->e_type != ET_REL )
	{
		return;
	}
	switch ( target )
	{
		case SRX_TARGET_IOP:
			elf->ehp->e_type = ET_SCE_IOPRELEXEC;
			break;
		case SRX_TARGET_EE:
			elf->ehp->e_type = ET_SCE_EERELEXEC2;
			break;
		default:
			break;
	}
	scp = search_section(elf, SHT_SYMTAB);
	if ( scp == NULL )
	{
		return;
	}
	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	syp = (elf_syment **)scp->data;
	for ( i = 1; i < entrise; i += 1 )
	{
		if ( syp[i]->sym.st_shndx )
		{
			if ( syp[i]->sym.st_shndx <= 0xFEFF )
			{
				syp[i]->sym.st_value += syp[i]->shptr->shr.sh_addr;
			}
		}
	}
}

static void rebuild_relocation(elf_file *elf, unsigned int gpvalue)
{
	int s;

	if ( elf->scp == NULL )
	{
		return;
	}
	for ( s = 1; s < elf->ehp->e_shnum; s += 1 )
	{
		if ( elf->scp[s]->shr.sh_type == SHT_REL )
		{
			rebuild_an_relocation(elf->scp[s], gpvalue, ((Srx_gen_table *)(elf->optdata))->target);
		}
	}
}

static int check_irx12(elf_file *elf, int cause_irx1)
{
	int s;

	if ( elf->ehp->e_type != ET_SCE_IOPRELEXEC )
	{
		return 0;
	}
	for ( s = 1; s < elf->ehp->e_shnum; s += 1 )
	{
		if ( elf->scp[s]->shr.sh_type == SHT_REL && relocation_is_version2(elf->scp[s]) )
		{
			if ( cause_irx1 )
			{
				fprintf(stderr, "R_MIPS_LO16 without R_MIPS_HI16\n");
				return 1;
			}
			elf->ehp->e_type = ET_SCE_IOPRELEXEC2;
		}
	}
	return 0;
}

static void setup_module_info(elf_file *elf, elf_section *modsect, const char *modulesymbol)
{
	unsigned int *section_data;
	int i;
	char *buf;
	const char *name;
	unsigned int woff;
	size_t buflen;
	const unsigned int *modnamep;
	unsigned int *modidatap;
	unsigned int modiaddr;
	const elf_syment *syp;

	syp = search_global_symbol(modulesymbol, elf);
	if ( is_defined_symbol(syp) == 0 )
	{
		return;
	}
	modiaddr = get_symbol_value(syp, elf);
	modidatap = get_section_data(elf, modiaddr);
	section_data = get_section_data(elf, *modidatap);
	woff = (uintptr_t)section_data & 3;
	modnamep = (unsigned int *)((char *)section_data - woff);
	buflen = (strlen((const char *)section_data - woff + 4) + 15) & 0xFFFFFFFC;
	buf = (char *)malloc(buflen);
	memcpy(buf, modnamep, buflen);
	swapmemory(buf, "l", buflen >> 2);
	name = &buf[woff];
	switch ( modsect->shr.sh_type )
	{
		case SHT_SCE_IOPMOD:
		{
			Elf32_IopMod *iopmodp_1;
			Elf32_IopMod *iopmodp_2;

			iopmodp_1 = (Elf32_IopMod *)modsect->data;
			iopmodp_1->moduleinfo = modiaddr;
			iopmodp_1->moduleversion = *((uint16_t *)modidatap + 2);
			// HACK FIXME: Don't place the name in the header
			name = "";
			iopmodp_2 = (Elf32_IopMod *)realloc(iopmodp_1, strlen(name) + sizeof(Elf32_IopMod));
			strcpy(iopmodp_2->modulename, name);
			modsect->data = (uint8_t *)iopmodp_2;
			modsect->shr.sh_size = iopmod_size(iopmodp_2);
			break;
		}
		case SHT_SCE_EEMOD:
		{
			Elf32_EeMod *eemodp_1;
			Elf32_EeMod *eemodp_2;
			eemodp_1 = (Elf32_EeMod *)modsect->data;
			eemodp_1->moduleinfo = modiaddr;
			eemodp_1->moduleversion = *((uint16_t *)modidatap + 2);
			eemodp_2 = (Elf32_EeMod *)realloc(eemodp_1, strlen(name) + sizeof(Elf32_EeMod));
			strcpy(eemodp_2->modulename, name);
			modsect->data = (uint8_t *)eemodp_2;
			modsect->shr.sh_size = eemod_size(eemodp_2);
			break;
		}
		default:
			break;
	}
	free(buf);
	if ( elf->php == NULL )
	{
		return;
	}
	for ( i = 0; i < elf->ehp->e_phnum; i += 1 )
	{
		if ( elf->php[i].scp && (modsect == *elf->php[i].scp) )
		{
			elf->php[i].phdr.p_filesz = modsect->shr.sh_size;
		}
	}
}

static void rebuild_an_relocation(elf_section *relsect, unsigned int gpvalue, int target)
{
	elf_rel *newtab;
	void *daddr_2;
	void *daddr_3;
	uint32_t data32_1;
	int data32_2;
	uint32_t datah;
	uint32_t data_1;
	uint32_t data_2;
	uint32_t data_33;
	uint16_t data_4;
	unsigned int data_5;
	unsigned int data_6;
	uint32_t data_7;
	uint32_t step;
	elf_syment **symp;
	elf_rel *rp;
	int rmflag;
	unsigned int entrise;
	unsigned int j_1;
	unsigned int j_2;
	unsigned int j_3;
	unsigned int i_1;

	entrise = relsect->shr.sh_size / relsect->shr.sh_entsize;
	rp = (elf_rel *)relsect->data;
	symp = (elf_syment **)relsect->link->data;
	rmflag = 0;
	for ( i_1 = 0; i_1 < entrise; )
	{
		int v4;
		uint32_t symvalue;
		void *daddr_1;
		unsigned int next;

		if ( relsect->info->shr.sh_size <= rp->rel.r_offset )
		{
			fprintf(
				stderr,
				"Panic !! relocation #%u offset=0x%x overflow (section size=0x%x)\n",
				i_1,
				rp->rel.r_offset,
				relsect->info->shr.sh_size);
			exit(1);
		}
		next = 1;
		daddr_1 = (void *)&relsect->info->data[rp->rel.r_offset];
		symvalue = 0;
		if ( rp->symptr )
		{
			if ( rp->symptr->sym.st_shndx )
			{
				symvalue = rp->symptr->sym.st_value;
			}
		}
		v4 = 0;
		if (
			!rp->symptr || (rp->symptr->sym.st_shndx && rp->symptr->sym.st_shndx <= 0xFEFF)
			|| rp->symptr->sym.st_shndx == SHN_RADDR )
		{
			v4 = 1;
		}
		switch ( rp->type )
		{
			case R_MIPS_NONE:
				rmflag = 1;
				break;
			case R_MIPS_16:
				data_1 = symvalue + (int16_t)*(uint32_t *)daddr_1;
				if ( (uint16_t)(data_1 >> 16) && (uint16_t)(data_1 >> 16) != 0xFFFF )
				{
					fprintf(stderr, "REFHALF data overflow\n");
					exit(1);
				}
				*(uint32_t *)daddr_1 &= 0xFFFF0000;
				*(uint32_t *)daddr_1 |= (uint16_t)data_1;
				if ( !v4 )
				{
					rp->type = R_MIPS_NONE;
					rmflag = 1;
				}
				break;
			case R_MIPS_32:
				*(uint32_t *)daddr_1 += symvalue;
				if ( !v4 )
				{
					rp->type = R_MIPS_NONE;
					rmflag = 1;
				}
				break;
			case R_MIPS_26:
				data_2 = *(uint32_t *)daddr_1;
				if ( rp->symptr && rp->symptr->bind != STB_LOCAL )
				{
					data_33 = data_2 << 6 >> 4;
				}
				else
				{
					data_33 = ((relsect->info->shr.sh_addr + rp->rel.r_offset) & 0xF0000000) | (4 * (data_2 & 0x3FFFFFF));
				}
				*(uint32_t *)daddr_1 &= 0xFC000000;
				*(uint32_t *)daddr_1 |= (16 * (symvalue + data_33)) >> 6;
				if ( !v4 )
				{
					rp->type = R_MIPS_NONE;
					rmflag = 1;
				}
				break;
			case R_MIPS_HI16:
				datah = *(uint32_t *)daddr_1 << 16;
				for ( j_1 = i_1 + 1; j_1 < entrise && rp[next].type == R_MIPS_HI16; j_1 += 1 )
				{
					if ( rp->symptr && rp[next].symptr != rp->symptr )
					{
						fprintf(stderr, "R_MIPS_HI16 without R_MIPS_LO16\n");
						exit(1);
					}
					if ( relsect->info->shr.sh_size <= rp[next].rel.r_offset )
					{
						fprintf(
							stderr,
							"Panic !! relocation #%u offset=0x%x overflow (section size=0x%x)\n",
							i_1 + next,
							rp[next].rel.r_offset,
							relsect->info->shr.sh_size);
						exit(1);
					}
					daddr_1 = (void *)&relsect->info->data[rp[next].rel.r_offset];
					if ( datah != *(uint32_t *)daddr_1 << 16 )
					{
						fprintf(stderr, "R_MIPS_HI16s not same offsets\n");
						exit(1);
					}
					next += 1;
				}
				if ( j_1 == entrise + 1 || rp[next].type != R_MIPS_LO16 || (rp->symptr && rp[next].symptr != rp->symptr) )
				{
					fprintf(stderr, "R_MIPS_HI16 without R_MIPS_LO16\n");
					exit(1);
				}
				data32_1 = symvalue + (int16_t)*(uint32_t *)&relsect->info->data[rp[next].rel.r_offset] + datah;
				if ( next == 1 )
				{
					*(uint32_t *)daddr_1 &= 0xFFFF0000;
					*(uint32_t *)daddr_1 |= (uint16_t)(((data32_1 >> 15) + 1) >> 1);
					if ( !v4 )
					{
						rp->type = R_MIPS_NONE;
						rmflag = 1;
					}
				}
				else if ( v4 )
				{
					for ( j_2 = 0; j_2 < next; j_2 += 1 )
					{
						daddr_2 = (void *)&relsect->info->data[rp[j_2].rel.r_offset];
						*(uint32_t *)daddr_2 &= 0xFFFF0000;
						if ( j_2 < next - 1 )
						{
							step = rp[j_2 + 1].rel.r_offset - rp[j_2].rel.r_offset;
							if ( step >> 18 && step >> 18 != 0x3FFF )
							{
								fprintf(stderr, "R_MIPS_HI16s too long distance\n");
								exit(1);
							}
							*(uint32_t *)daddr_2 |= (uint16_t)(step >> 2);
						}
						rp[j_2].type = R_MIPS_NONE;
						if ( rp[j_2].symptr )
						{
							rp[j_2].symptr->refcount -= 1;
							rp[j_2].symptr = *symp;
						}
					}
					rp->type = R_MIPSSCE_MHI16;
					rp->rel.r_offset += relsect->info->shr.sh_addr;
					rp[1].type = R_MIPSSCE_ADDEND;
					rp[1].rel.r_offset = data32_1;
					rmflag = 1;
					rp += next;
					i_1 += next;
					next = 0;
				}
				else
				{
					data32_2 = (uint16_t)(((data32_1 >> 15) + 1) >> 1);
					for ( j_3 = 0; j_3 < next; j_3 += 1 )
					{
						daddr_3 = (void *)&relsect->info->data[rp[j_3].rel.r_offset];
						*(uint32_t *)daddr_3 &= 0xFFFF0000;
						*(uint32_t *)daddr_3 |= data32_2;
						rp[j_3].type = R_MIPS_NONE;
					}
					rmflag = 1;
				}
				break;
			case R_MIPS_LO16:
				data_4 = symvalue + *(uint32_t *)daddr_1;
				*(uint32_t *)daddr_1 &= 0xFFFF0000;
				*(uint32_t *)daddr_1 |= data_4;
				if ( !v4 )
				{
					rp->type = R_MIPS_NONE;
					rmflag = 1;
				}
				break;
			case R_MIPS_GPREL16:
				data_5 = (int16_t)*(uint32_t *)daddr_1;
				if ( rp->symptr )
				{
					if ( rp->symptr->type == STT_SECTION )
					{
						data_5 += ((Sect_org_data *)(rp->symptr->shptr->optdata))->org_gp_value + symvalue
										- ((Sect_org_data *)(rp->symptr->shptr->optdata))->org_addr - gpvalue;
					}
					else if ( rp->symptr->bind == STB_GLOBAL || (rp->symptr->bind == STB_WEAK && rp->symptr->sym.st_shndx) )
					{
						data_5 += symvalue - gpvalue;
					}
					else if ( rp->symptr->bind != STB_WEAK || rp->symptr->sym.st_shndx )
					{
						fprintf(stderr, "R_MIPS_GPREL16 unknown case abort\n");
						exit(1);
					}
				}
				else
				{
					fprintf(stderr, "R_MIPS_GPREL16 no symtab\n");
					exit(1);
				}
				if ( (uint16_t)(data_5 >> 16) && (uint16_t)(data_5 >> 16) != 0xFFFF )
				{
					fprintf(stderr, "R_MIPS_GPREL16 data overflow\n");
					exit(1);
				}
				*(uint32_t *)daddr_1 &= 0xFFFF0000;
				*(uint32_t *)daddr_1 |= (uint16_t)data_5;
				rp->type = R_MIPS_NONE;
				rmflag = 1;
				break;
			case R_MIPS_LITERAL:
				if ( !rp->symptr || rp->symptr->type != STT_SECTION )
				{
					fprintf(stderr, "R_MIPS_LITERAL unknown case abort\n");
					exit(1);
				}
				data_6 = ((Sect_org_data *)(rp->symptr->shptr->optdata))->org_gp_value + symvalue
							 - ((Sect_org_data *)(rp->symptr->shptr->optdata))->org_addr - gpvalue + (int16_t)*(uint32_t *)daddr_1;
				if ( (uint16_t)(data_6 >> 16) && (uint16_t)(data_6 >> 16) != 0xFFFF )
				{
					fprintf(stderr, "R_MIPS_LITERAL data overflow\n");
					exit(1);
				}
				*(uint32_t *)daddr_1 &= 0xFFFF0000;
				*(uint32_t *)daddr_1 |= (uint16_t)data_6;
				rp->type = R_MIPS_NONE;
				rmflag = 1;
				break;
			case R_MIPS_DVP_27_S4:
				if ( target != SRX_TARGET_EE )
				{
					fprintf(stderr, "R_MIPS_DVP_27_S4 can use only for EE.\n");
					exit(1);
				}
				data_7 = symvalue + (*(uint32_t *)daddr_1 & 0x7FFFFFF0);
				*(uint32_t *)daddr_1 &= 0x8000000F;
				*(uint32_t *)daddr_1 |= data_7 & 0x7FFFFFF0;
				if ( !v4 )
				{
					rp->type = R_MIPS_NONE;
					rmflag = 1;
				}
				break;
			case R_MIPS_REL32:
			case R_MIPS_GOT16:
			case R_MIPS_PC16:
			case R_MIPS_CALL16:
			case R_MIPS_GPREL32:
			case R_MIPS_GOTHI16:
			case R_MIPS_GOTLO16:
			case R_MIPS_CALLHI16:
			case R_MIPS_CALLLO16:
				fprintf(stderr, "unacceptable relocation type: 0x%x\n", rp->type);
				exit(1);
				return;
			default:
				fprintf(stderr, "unknown relocation type: 0x%x\n", rp->type);
				exit(1);
				return;
		}
		for ( ; next > 0; next -= 1 )
		{
			rp->rel.r_offset += relsect->info->shr.sh_addr;
			if ( rp->symptr )
			{
				rp->symptr->refcount -= 1;
				rp->symptr = *symp;
			}
			i_1 += 1;
			rp += 1;
		}
	}
	if ( rmflag > 0 )
	{
		elf_rel *s;
		elf_rel *d;
		unsigned int newentrise;
		unsigned int i_2;

		newtab = (elf_rel *)calloc(entrise, sizeof(elf_rel));
		d = newtab;
		s = (elf_rel *)relsect->data;
		newentrise = 0;
		for ( i_2 = 0; i_2 < entrise; i_2 += 1 )
		{
			if ( s->type )
			{
				memcpy(d, s, sizeof(elf_rel));
				d += 1;
				newentrise += 1;
			}
			s += 1;
		}
		free(relsect->data);
		relsect->data = (uint8_t *)newtab;
		relsect->shr.sh_size = relsect->shr.sh_entsize * newentrise;
	}
}

static size_t iopmod_size(const Elf32_IopMod *modinfo)
{
	return strlen(modinfo->modulename) + (sizeof(Elf32_IopMod) - 1);
}

static size_t eemod_size(const Elf32_EeMod *modinfo)
{
	return strlen(modinfo->modulename) + (sizeof(Elf32_EeMod) - 1);
}

int relocation_is_version2(elf_section *relsect)
{
	elf_rel *rp;
	unsigned int entrise;
	unsigned int i;

	entrise = relsect->shr.sh_size / relsect->shr.sh_entsize;
	rp = (elf_rel *)relsect->data;
	for ( i = 0; i < entrise; i += 1 )
	{
		switch ( rp->type )
		{
			case R_MIPS_LO16:
			case R_MIPSSCE_MHI16:
			case R_MIPSSCE_ADDEND:
				return 1;
			case R_MIPS_HI16:
				if ( i == entrise + 1 || rp[1].type != R_MIPS_LO16 || (rp->symptr && rp[1].symptr != rp->symptr) )
				{
					return 1;
				}
				rp += 1;
				i += 1;
				break;
			default:
				break;
		}
		rp += 1;
	}
	return 0;
}

void dump_srx_gen_table(Srx_gen_table *tp)
{
	const char *v1;
	int scnfp_;
	int v8;
	int scp_;
	int scp_a;
	int nsegment;
	int b;
	int i;
	CreateSymbolConf *csyms;
	PheaderInfo *phip;
	SectConf *sctp;
	const char ***scnfpp;
	SegConf *scnfp;
	elf_section **scp;
	char segsig[32];
	const char **strp;

	if ( tp == NULL )
	{
		return;
	}
	switch ( tp->target )
	{
		case SRX_TARGET_IOP:
			v1 = "IOP";
			break;
		case SRX_TARGET_EE:
			v1 = "EE";
			break;
		default:
			v1 = "??";
			break;
	}
	printf("===============\nTarget is %s(%d)\n", v1, tp->target);
	printf("Segment list\n");
	for ( v8 = 0, scnfp = tp->segment_list; scnfp->name; v8 += 1, scnfp += 1 )
	{
		printf("  %2d:segment %s\n", v8, scnfp->name);
		printf(
			"      addr,size=0x%x,0x%x  bitid,nsect= 0x%x,%d\n      ", scnfp->addr, scnfp->size, scnfp->bitid, scnfp->nsect);
		if ( scnfp->sect_name_patterns )
		{
			for ( strp = scnfp->sect_name_patterns; *strp; strp += 1 )
			{
				printf("%s ", *strp);
			}
			printf("\n");
		}
		if ( scnfp->empty_section )
		{
			printf("        Auto add section: %s\n", scnfp->empty_section->name);
		}
		if ( scnfp->scp )
		{
			for ( scp = scnfp->scp; *scp; scp += 1 )
			{
				printf("        %p: %s\n", *scp, (*scp)->name);
			}
		}
		segsig[v8] = *scnfp->name;
	}
	printf("\nProgram header order\n");
	for ( scp_ = 0, phip = tp->program_header_order; phip->sw; scp_ += 1, phip += 1 )
	{
		switch ( phip->sw )
		{
			case SRX_PH_TYPE_MOD:
				printf("  %2d: section %s\n", scp_, phip->d.section_name);
				break;
			case SRX_PH_TYPE_TEXT:
				printf("  %2d: Segments ", scp_);
				for ( scnfpp = (const char ***)phip->d.section_name; *scnfpp; scnfpp += 1 )
				{
					printf("%s ", **scnfpp);
				}
				printf("\n");
				break;
			default:
				break;
		}
	}
	printf("\nRemove section list\n");
	for ( scp_a = 0, strp = tp->removesection_list; *strp; scp_a += 1, strp += 1 )
	{
		printf("  %2d: %s\n", scp_a, *strp);
	}
	printf("\nSection table order\n");
	for ( nsegment = 0, strp = tp->section_table_order; *strp; nsegment += 1, strp += 1 )
	{
		printf("  %2d: %s\n", nsegment, *strp);
	}
	printf("\nFile layout order\n");
	for ( b = 0, strp = tp->file_layout_order; *strp; b += 1, strp += 1 )
	{
		printf("  %2d: %s\n", b, *strp);
	}
	printf("\nmemory layout order\n");
	for ( i = 0, sctp = tp->section_list; sctp->sect_name_pattern; i += 1, sctp += 1 )
	{
		printf("  %2d: [", i);
		for ( scnfp_ = 0; scnfp_ < v8; scnfp_ += 1 )
		{
			printf("%c", ((sctp->flag & (1 << scnfp_)) != 0) ? (unsigned char)(segsig[scnfp_]) : 46);
		}
		printf("] %s", sctp->sect_name_pattern);
		if ( sctp->secttype )
		{
			printf("\t: Auto create type=%x flag=%x", sctp->secttype, sctp->sectflag);
		}
		printf("\n");
	}
	printf("\nReserved symbols\n");
	for ( csyms = tp->create_symbols; csyms->name; csyms += 1 )
	{
		printf(
			"   %-8s: bind=%d, type=%d, shindex=0x%04x, seflag=%d, seg=%s, sect=%s\n",
			csyms->name,
			csyms->bind,
			csyms->type,
			csyms->shindex,
			csyms->seflag,
			csyms->segment ? csyms->segment->name : "-",
			csyms->sectname ?: "-");
	}
	printf("\n\n");
}
