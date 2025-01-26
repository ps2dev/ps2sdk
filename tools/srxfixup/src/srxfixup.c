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

static const char *conffile = NULL;
static const char *ofile = NULL;
static const char *rfile = NULL;
static const char *ffile = NULL;
static unsigned int startaddr;
static const char *entrysym = NULL;
static unsigned int verbose = 0;
static unsigned int dumpflag = 0;
static unsigned int dispmod_flag = 0;
static int irx1_flag = 0;
static int br_conv = 0;
static int print_config = 0;
// clang-format off
static const Opttable opttable[] =
{
	{ "-v", ARG_HAVEARG_NONE, 'f', &verbose },
	{ "-d", ARG_HAVEARG_NONE, 'f', &dumpflag },
	{ "-r", ARG_HAVEARG_REQUIRED, 's', &rfile },
	{ "-o", ARG_HAVEARG_REQUIRED, 's', &ofile },
	{ "-c", ARG_HAVEARG_REQUIRED, 's', &conffile },
	{ "-f", ARG_HAVEARG_REQUIRED, 's', &ffile },
	{ "-t", ARG_HAVEARG_REQUIRED, 'h', &startaddr },
	{ "-e", ARG_HAVEARG_REQUIRED, 's', &entrysym },
	{ "-m", ARG_HAVEARG_NONE, 'f', &dispmod_flag },
	{ "--irx1", ARG_HAVEARG_NONE, 'f', &irx1_flag },
	{ "--rb", ARG_HAVEARG_NONE, 'f', &br_conv },
	{ "--relative-branch", ARG_HAVEARG_NONE, 'f', &br_conv },
	{ "--print-internal-config", ARG_HAVEARG_NONE, 'f', &print_config },
	{ NULL, 0, '\0', NULL },
};
static const Opttable stripopttable[] =
{
	{ "-v", ARG_HAVEARG_NONE, 'f', &verbose },
	{ "-d", ARG_HAVEARG_NONE, 'f', &dumpflag },
	{ "-o", ARG_HAVEARG_REQUIRED, 's', &ofile },
	{ "-c", ARG_HAVEARG_REQUIRED, 's', &conffile },
	{ "-e", ARG_HAVEARG_REQUIRED, 's', &entrysym },
	{ "-m", ARG_HAVEARG_NONE, 'f', &dispmod_flag },
	{ "--irx1", ARG_HAVEARG_NONE, 'f', &irx1_flag },
	{ "--rb", ARG_HAVEARG_NONE, 'f', &br_conv },
	{ "--relative-branch", ARG_HAVEARG_NONE, 'f', &br_conv },
	{ "--print-internal-config", ARG_HAVEARG_NONE, 'f', &print_config },
	{ NULL, 0, '\0', NULL },
};
// clang-format on

static void display_module_info(elf_file *elf);
static void convert_relative_branch_an_section(elf_section *relsect);
static void convert_relative_branch(elf_file *elf);

void usage(const char *myname)
{
	printf(
		"IOP/EE relocatable object converter\n"
		"%s\n"
		"usage: %s [options] <elf_input_file>\n",
		myname,
		myname);
	printf("  options:\n"
				 "    -v\n"
				 "    -m\n"
				 "    --irx1\n"
				 "    -o <elf_relocatable_nosymbol_output_file>\n"
				 "    -r <elf_relocatable_output_file>\n"
				 "    -e <entry_point_symbol>\n"
				 "    --relative-branch  or  --rb\n");
	if ( verbose )
	{
		printf("    -t <.text start address>\n"
					 "    -f <elf_fixedaddress_output_file>\n"
					 "    -d<hex_flag>\n"
					 "        hex_flag bits:\n"
					 "           bit0: dump section table\n"
					 "           bit1: dump relocation record\n"
					 "           bit2: dump symbol table\n"
					 "           bit3: disassemble program code\n"
					 "           bit4: dump .data/.rodata/.sdata... sections by byte\n"
					 "           bit5: dump .data/.rodata/.sdata... sections by half word\n"
					 "           bit6: dump .data/.rodata/.sdata... sections by word\n"
					 "           bit7: dump .data/.rodata/.sdata... sections by word with relocation data\n"
					 "           bit8: dump file layout\n"
					 "           bit9: dump .mdebug section\n");
	}
	if ( verbose > 1 )
	{
		printf("           bit12: dump srx genaration table\n"
					 "    -c <config_file>\n"
					 "    --print-internal-config\n");
	}
}

void stripusage(const char *myname)
{
	printf(
		"%s\n"
		"usage: %s [options] <elf_file>\n",
		myname,
		myname);
	printf("  options:\n"
				 "    -v\n"
				 "    -m\n"
				 "    -o <elf_relocatable_nosymbol_output_file>\n"
				 "    --relative-branch  or  --rb\n");
}

int main(int argc, char **argv)
{
	Srx_gen_table *srxgen_1;
	const char *defaultconf;
	elf_file *elf;
	const char *myname_1;
	const char *myname_2;
	const char *source;

	myname_1 = strrchr(*argv, '/');
	if ( !myname_1 )
	{
		myname_1 = strrchr(*argv, '\\');
	}
	if ( myname_1 )
	{
		myname_2 = myname_1 + 1;
	}
	else
	{
		myname_2 = *argv;
	}
	if ( (strncmp(myname_2, "ee", 2) != 0) && (strncmp(myname_2, "EE", 2) != 0) )
	{
		defaultconf = iop_defaultconf;
	}
	else
	{
		defaultconf = ee_defaultconf;
	}
	if ( strlen(*argv) > 5 && !strcmp(&(*argv)[strlen(*argv) - 5], "strip") )
	{
		int argca;

		argca = analize_arguments(stripopttable, argc, argv);
		if ( argca != 2 )
		{
			Srx_gen_table *srxgen_2;

			srxgen_2 = read_conf(defaultconf, conffile, print_config);
			if ( !srxgen_2 )
			{
				exit(1);
			}
			if ( (dumpflag & 0x1000) != 0 )
			{
				dump_srx_gen_table(srxgen_2);
				exit(0);
			}
		}
		if ( argca <= 1 )
		{
			stripusage(*argv);
			exit(1);
		}
		if ( argca > 2 )
		{
			fprintf(stderr, "Too many input file\n");
			stripusage(*argv);
			exit(1);
		}
		if ( !ofile )
		{
			ofile = argv[1];
		}
	}
	else
	{
		int argcb;

		argcb = analize_arguments(opttable, argc, argv);
		if ( argcb != 2 )
		{
			Srx_gen_table *srxgen_3;

			srxgen_3 = read_conf(defaultconf, conffile, print_config);
			if ( !srxgen_3 )
			{
				exit(1);
			}
			if ( (dumpflag & 0x1000) != 0 )
			{
				dump_srx_gen_table(srxgen_3);
				exit(0);
			}
		}
		if ( argcb <= 1 )
		{
			usage(*argv);
			exit(1);
		}
		if ( argcb > 2 )
		{
			fprintf(stderr, "Too many input file\n");
			usage(*argv);
			exit(1);
		}
	}
	source = argv[1];
	elf = read_elf(source);
	if ( !elf )
	{
		exit(1);
	}
	if (
		((elf->ehp->e_flags & EF_MIPS_MACH) == EF_MIPS_MACH_5900)
		&& ((elf->ehp->e_flags & EF_MIPS_ARCH) == EF_MIPS_ARCH_3) )
	{
		srxgen_1 = read_conf(ee_defaultconf, conffile, print_config);
	}
	else
	{
		srxgen_1 = read_conf(iop_defaultconf, conffile, print_config);
	}
	if ( !srxgen_1 )
	{
		exit(1);
	}
	if ( (dumpflag & 0x1000) != 0 )
	{
		dump_srx_gen_table(srxgen_1);
		exit(0);
	}
	if ( (dumpflag & 0xFFF) != 0 )
	{
		print_elf(elf, dumpflag & 0xFFF);
		exit(0);
	}
	elf->optdata = (void *)srxgen_1;
	switch ( elf->ehp->e_type )
	{
		case ET_REL:
			if ( convert_rel2srx(elf, entrysym, (rfile || ofile || ffile) ? 1 : 0, irx1_flag) )
			{
				exit(1);
			}
			break;
		case ET_SCE_IOPRELEXEC:
		case ET_SCE_IOPRELEXEC2:
		case ET_SCE_EERELEXEC2:
		case ET_EXEC:
			break;
		default:
			fprintf(stderr, "Error: '%s' is unsupport Type Elf file(type=%x)\n", source, elf->ehp->e_type);
			exit(1);
			break;
	}
	if ( dispmod_flag )
	{
		display_module_info(elf);
	}
	switch ( elf->ehp->e_type )
	{
		case ET_SCE_IOPRELEXEC:
		case ET_SCE_IOPRELEXEC2:
		case ET_SCE_EERELEXEC2:
			if ( br_conv )
			{
				convert_relative_branch(elf);
			}
			if ( rfile )
			{
				if ( layout_srx_file(elf) )
				{
					exit(1);
				}
				write_elf(elf, rfile);
			}
			if ( ofile )
			{
				strip_elf(elf);
				if ( layout_srx_file(elf) )
				{
					exit(1);
				}
				write_elf(elf, ofile);
			}
			break;
		default:
			if ( rfile || ofile )
			{
				fprintf(stderr, "Error: Cannot generate IRX/ERX file.  '%s' file type is ET_EXEC.\n", source);
				exit(1);
			}
			break;
	}
	if ( ffile || startaddr != (unsigned int)(-1) )
	{
		switch ( elf->ehp->e_type )
		{
			case ET_SCE_IOPRELEXEC:
			case ET_SCE_IOPRELEXEC2:
			case ET_SCE_EERELEXEC2:
				elf->ehp->e_type = ET_EXEC;
				fixlocation_elf(elf, startaddr);
				break;
			default:
				break;
		}
		if ( ffile )
		{
			if ( layout_srx_file(elf) )
			{
				exit(1);
			}
			write_elf(elf, ffile);
		}
	}
	return 0;
}

static void display_module_info(elf_file *elf)
{
	elf_section *modsect_1;
	elf_section *modsect_2;

	modsect_1 = search_section(elf, SHT_SCE_IOPMOD);
	if ( modsect_1 )
	{
		const Elf32_IopMod *iopmodinfo;

		iopmodinfo = (Elf32_IopMod *)modsect_1->data;
		if ( iopmodinfo->moduleinfo != (Elf32_Word)(-1) )
		{
			printf(
				"name:%s version:%d.%d\n",
				iopmodinfo->modulename,
				(uint8_t)(iopmodinfo->moduleversion >> 24),
				(uint8_t)iopmodinfo->moduleversion);
		}
	}
	modsect_2 = search_section(elf, SHT_SCE_EEMOD);
	if ( modsect_2 )
	{
		const Elf32_EeMod *eemodinfo;

		eemodinfo = (Elf32_EeMod *)modsect_2->data;
		if ( eemodinfo->moduleinfo != (Elf32_Word)(-1) )
		{
			printf(
				"name:%s version:%d.%d\n",
				eemodinfo->modulename,
				(uint8_t)(eemodinfo->moduleversion >> 24),
				(uint8_t)eemodinfo->moduleversion);
		}
	}
}

static void convert_relative_branch_an_section(elf_section *relsect)
{
	elf_syment **symp;
	elf_rel *rp;
	int rmcount;
	unsigned int entrise;
	unsigned int i;

	entrise = relsect->shr.sh_size / relsect->shr.sh_entsize;
	rp = (elf_rel *)relsect->data;
	symp = (elf_syment **)relsect->link->data;
	rmcount = 0;
	for ( i = 0; i < entrise; i += 1 )
	{
		unsigned int type;
		uint8_t *daddr;

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
		daddr = &relsect->info->data[rp->rel.r_offset - relsect->info->shr.sh_addr];
		type = rp->type;
		if ( type )
		{
			if ( type == R_MIPS_26 )
			{
				uint32_t raddr;
				unsigned int data;

				data = *(uint32_t *)daddr;
				if ( rp->symptr && rp->symptr->bind != STB_LOCAL )
				{
					fprintf(stderr, "R_MIPS_26 Unexcepted bind\n");
					exit(1);
				}
				raddr = rp->rel.r_offset + relsect->info->shr.sh_addr;
				if (
					!((((rp->rel.r_offset & 0xF0000000) | (4 * (data & 0x3FFFFFF))) - 4 - raddr) >> 18)
					|| (((rp->rel.r_offset & 0xF0000000) | (4 * (data & 0x3FFFFFF))) - 4 - raddr) >> 18 == 0x3FFF )
				{
					int jaddr;

					jaddr = (uint16_t)((((rp->rel.r_offset & 0xF0000000) | (4 * (data & 0x3FFFFFF))) - 4 - raddr) >> 2);
					if ( data >> 26 == 2 )
					{
						*(uint32_t *)daddr = jaddr | 0x10000000;
						rp->type = R_MIPS_NONE;
						rmcount += 1;
					}
					else if ( data >> 26 == 3 )
					{
						*(uint32_t *)daddr = jaddr | 0x4110000;
						rp->type = R_MIPS_NONE;
						rmcount += 1;
					}
				}
			}
		}
		else
		{
			rmcount += 1;
		}
		rp += 1;
	}
	if ( rmcount > 0 && (entrise - rmcount) > 0 )
	{
		elf_rel *s;
		elf_rel *d;
		elf_rel *newtab;
		unsigned int j;

		newtab = (elf_rel *)calloc(entrise - rmcount, sizeof(elf_rel));
		d = newtab;
		s = (elf_rel *)relsect->data;
		for ( j = 0; j < entrise; j += 1 )
		{
			if ( s->type )
			{
				memcpy(d, s, sizeof(elf_rel));
				d += 1;
			}
			s += 1;
		}
		free(relsect->data);
		relsect->data = (uint8_t *)newtab;
		relsect->shr.sh_size = relsect->shr.sh_entsize * (entrise - rmcount);
	}
}

static void convert_relative_branch(elf_file *elf)
{
	int i;

	if ( elf->scp == NULL )
	{
		return;
	}
	for ( i = 1; i < elf->ehp->e_shnum; i += 1 )
	{
		if ( elf->scp[i]->shr.sh_type == SHT_REL )
		{
			convert_relative_branch_an_section(elf->scp[i]);
		}
	}
}
