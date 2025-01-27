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
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_in_range(unsigned int top, unsigned int size, unsigned int pos);
static void read_symtab(elf_file *elf, int sctindex, FILE *fp);
static void read_rel(elf_file *elf, int sctindex, FILE *fp);
static elf_mips_symbolic_data *read_mips_symbolic(FILE *fp);
static void renumber_a_symtab(elf_section *scp);
static void renumber_symtab(elf_file *elf);
static void write_symtab(elf_file *elf, int sctindex, FILE *fp);
static void write_rel(elf_file *elf, int sctindex, FILE *fp);
static void write_mips_symbolic(elf_mips_symbolic_data *sycb, unsigned int basepos, FILE *fp);
static void reorder_an_symtab(elf_file *elf, elf_section *scp);
static size_t search_string_table(const char *tbltop, size_t endindex, const char *str);
static void rebuild_a_symbol_name_strings(elf_section *scp);
static int comp_Elf_file_slot(const void *a1, const void *a2);

elf_file *read_elf(const char *filename)
{
	uint32_t ident;
	elf_file *elf;
	FILE *fp;

	fp = fopen(filename, "rb");
	if ( !fp )
	{
		fprintf(stderr, "\"%s\" can't open (errno=%d)\n", filename, errno);
		return 0;
	}
	elf = (elf_file *)calloc(1, sizeof(elf_file));
	elf->ehp = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
	if ( fread(elf->ehp, sizeof(Elf32_Ehdr), 1, fp) != 1 )
	{
		fprintf(stderr, "%s: Could not read ELF header (errno=%d)\n", filename, errno);
		exit(1);
	}
	swapmemory(elf->ehp, "ccccccccccccccccsslllllssssss", 1);
	ident = *(uint32_t *)elf->ehp->e_ident;
	swapmemory(&ident, "l", 1);
	if ( ident != 0x464C457F )
	{
		fprintf(stderr, "%s: not elf format\n", filename);
		free(elf->ehp);
		free(elf);
		fclose(fp);
		return 0;
	}
	if ( elf->ehp->e_ident[4] != ELFCLASS32 || elf->ehp->e_ident[5] != ELFDATA2LSB || elf->ehp->e_ident[6] != EV_CURRENT )
	{
		fprintf(stderr, "%s: Not 32-bit object or Not little endian or Invalid Elf version\n", filename);
		free(elf->ehp);
		free(elf);
		fclose(fp);
		return 0;
	}
	if ( elf->ehp->e_machine != EM_MIPS )
	{
		fprintf(stderr, "%s: not EM_MIPS\n", filename);
		free(elf->ehp);
		free(elf);
		fclose(fp);
		return 0;
	}
	if ( elf->ehp->e_phentsize && elf->ehp->e_phentsize != sizeof(Elf32_Phdr) )
	{
		fprintf(stderr, "%s: Unknown program header size\n", filename);
		free(elf->ehp);
		free(elf);
		fclose(fp);
		return 0;
	}
	if ( elf->ehp->e_shentsize && elf->ehp->e_shentsize != sizeof(Elf32_Shdr) )
	{
		fprintf(stderr, "%s: Unknown sectoin header size\n", filename);
		free(elf->ehp);
		free(elf);
		fclose(fp);
		return 0;
	}
	if ( elf->ehp->e_phnum && elf->ehp->e_phentsize )
	{
		signed int count_1;
		unsigned int pos_1;
		int i_1;

		count_1 = elf->ehp->e_phnum;
		pos_1 = elf->ehp->e_phoff;
		elf->php = (elf_proghead *)calloc(count_1, sizeof(elf_proghead));
		fseek(fp, pos_1, SEEK_SET);
		for ( i_1 = 0; count_1 > i_1; i_1 += 1 )
		{
			if ( fread(&elf->php[i_1], sizeof(Elf32_Phdr), 1, fp) != 1 )
			{
				fprintf(stderr, "%s: Could not read ELF program header (errno=%d)\n", filename, errno);
				exit(1);
			}
			swapmemory(&elf->php[i_1], "llllllll", 1);
		}
	}
	if ( elf->ehp->e_shnum && elf->ehp->e_shentsize )
	{
		signed int count_2;
		unsigned int pos_2;
		int i_2;
		int i_3;
		int i_4;

		pos_2 = elf->ehp->e_shoff;
		count_2 = elf->ehp->e_shnum;
		elf->scp = (elf_section **)calloc(count_2 + 1, sizeof(elf_section *));
		fseek(fp, pos_2, SEEK_SET);
		for ( i_2 = 0; count_2 > i_2; i_2 += 1 )
		{
			elf->scp[i_2] = (elf_section *)calloc(1, sizeof(elf_section));
			if ( fread(elf->scp[i_2], sizeof(Elf32_Shdr), 1, fp) != 1 )
			{
				fprintf(stderr, "%s: Could not read ELF section header (errno=%d)\n", filename, errno);
				exit(1);
			}
			swapmemory(elf->scp[i_2], "llllllllll", 1);
		}
		for ( i_3 = 0; count_2 > i_3; i_3 += 1 )
		{
			switch ( elf->scp[i_3]->shr.sh_type )
			{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
				case SHT_RELA:
				case SHT_REL:
					elf->scp[i_3]->info = elf->scp[elf->scp[i_3]->shr.sh_info];
				case SHT_SYMTAB:
				case SHT_HASH:
				case SHT_DYNAMIC:
				case SHT_DYNSYM:
					elf->scp[i_3]->link = elf->scp[elf->scp[i_3]->shr.sh_link];
					break;
#pragma GCC diagnostic pop
				default:
					break;
			}
			if ( i_3 == elf->ehp->e_shstrndx )
			{
				elf->shstrptr = elf->scp[i_3];
			}
		}
		for ( i_4 = 0;; i_4 += 1 )
		{
			unsigned int size;
			unsigned int pos_3;

			if ( count_2 <= i_4 )
			{
				int i_5;
				int i_6;
				int i_7;
				int i_8;

				for ( i_5 = 0; count_2 > i_5; i_5 += 1 )
				{
					unsigned int pos_4;

					pos_4 = elf->scp[i_5]->shr.sh_offset;
					switch ( elf->scp[i_5]->shr.sh_type )
					{
						case SHT_SYMTAB:
						case SHT_DYNSYM:
							if ( pos_4 != 0 && elf->scp[i_5]->shr.sh_size )
							{
								fseek(fp, pos_4, SEEK_SET);
								read_symtab(elf, i_5, fp);
							}
							break;
						default:
							break;
					}
				}
				for ( i_6 = 0; count_2 > i_6; i_6 += 1 )
				{
					unsigned int pos_5;

					pos_5 = elf->scp[i_6]->shr.sh_offset;
					if ( elf->scp[i_6]->shr.sh_type == SHT_REL && pos_5 != 0 && elf->scp[i_6]->shr.sh_size )
					{
						fseek(fp, pos_5, SEEK_SET);
						read_rel(elf, i_6, fp);
					}
				}
				for ( i_7 = 0; count_2 > i_7; i_7 += 1 )
				{
					elf->scp[i_7]->name =
						strdup((elf->shstrptr != NULL) ? ((char *)&elf->shstrptr->data[elf->scp[i_7]->shr.sh_name]) : "");
				}
				for ( i_8 = 0; i_8 < elf->ehp->e_phnum; i_8 += 1 )
				{
					int d;
					int s;

					switch ( elf->php[i_8].phdr.p_type )
					{
						case PT_LOAD:
							elf->php[i_8].scp = (elf_section **)calloc(count_2, sizeof(elf_section *));
							d = 0;
							for ( s = 1; s < count_2; s += 1 )
							{
								unsigned int p_filesz;
								unsigned int p_offset;

								p_offset = elf->php[i_8].phdr.p_offset;
								p_filesz = elf->php[i_8].phdr.p_filesz;
								switch ( elf->scp[s]->shr.sh_type )
								{
									case SHT_PROGBITS:
										if (
											is_in_range(p_offset, p_filesz, elf->scp[s]->shr.sh_offset)
											|| (!elf->scp[s]->shr.sh_size && elf->scp[s]->shr.sh_offset == p_filesz + p_offset) )
										{
											elf->php[i_8].scp[d] = elf->scp[s];
											d += 1;
										}
										break;
									case SHT_NOBITS:
										if (
											is_in_range(elf->php[i_8].phdr.p_vaddr, elf->php[i_8].phdr.p_memsz, elf->scp[s]->shr.sh_addr)
											|| (!elf->scp[s]->shr.sh_size && elf->scp[s]->shr.sh_offset == p_filesz + p_offset) )
										{
											elf->php[i_8].scp[d] = elf->scp[s];
											d += 1;
										}
										break;
									default:
										break;
								}
							}
							break;
						case PT_MIPS_REGINFO:
							elf->php[i_8].scp = (elf_section **)calloc(2, sizeof(elf_section *));
							*elf->php[i_8].scp = search_section(elf, SHT_MIPS_REGINFO);
							break;
						case PT_SCE_IOPMOD:
							elf->php[i_8].scp = (elf_section **)calloc(2, sizeof(elf_section *));
							*elf->php[i_8].scp = search_section(elf, SHT_SCE_IOPMOD);
							break;
						case PT_SCE_EEMOD:
							elf->php[i_8].scp = (elf_section **)calloc(2, sizeof(elf_section *));
							*elf->php[i_8].scp = search_section(elf, SHT_SCE_EEMOD);
							break;
						default:
							break;
					}
				}
				// Workaround for malformed file: make distance between sh_addr agree with sh_offset
				switch ( elf->ehp->e_type )
				{
					case ET_SCE_IOPRELEXEC:
					case ET_SCE_IOPRELEXEC2:
					case ET_SCE_EERELEXEC:
					case ET_SCE_EERELEXEC2:
						for ( i_8 = 0; i_8 < elf->ehp->e_phnum; i_8 += 1 )
						{
							int s;
							elf_section **scp;

							switch ( elf->php[i_8].phdr.p_type )
							{
								case PT_LOAD:
									scp = elf->php[i_8].scp;
									for ( s = 1; scp[s]; s += 1 )
									{
										if ( scp[s]->shr.sh_type != SHT_NOBITS )
										{
											unsigned int addrdiff;
											unsigned int offsetdiff;

											addrdiff = scp[s]->shr.sh_addr - scp[s - 1]->shr.sh_addr;
											offsetdiff = scp[s]->shr.sh_offset - scp[s - 1]->shr.sh_offset;
											if ( addrdiff != offsetdiff )
											{
												scp[s]->shr.sh_addr = scp[s - 1]->shr.sh_addr + offsetdiff;
											}
										}
									}
									break;
								default:
									break;
							}
						}
						break;
					default:
						break;
				}
				return elf;
			}
			pos_3 = elf->scp[i_4]->shr.sh_offset;
			size = elf->scp[i_4]->shr.sh_size;
			if ( pos_3 != 0 && size != 0 )
			{
				fseek(fp, pos_3, SEEK_SET);
				switch ( elf->scp[i_4]->shr.sh_type )
				{
					case SHT_PROGBITS:
					case SHT_RELA:
					case SHT_HASH:
					case SHT_DYNAMIC:
					case SHT_MIPS_REGINFO:
						elf->scp[i_4]->data = (uint8_t *)malloc(size);
						if ( fread(elf->scp[i_4]->data, size, 1, fp) != 1 )
						{
							fprintf(stderr, "%s: Could not read ELF section contents, (errno=%d)\n", filename, errno);
							exit(1);
						}
						swapmemory(elf->scp[i_4]->data, "l", size >> 2);
						break;
					case SHT_SYMTAB:
					case SHT_NOBITS:
					case SHT_DYNSYM:
						break;
					case SHT_MIPS_DEBUG:
						elf->scp[i_4]->data = (uint8_t *)read_mips_symbolic(fp);
						break;
					case SHT_SCE_IOPMOD:
						elf->scp[i_4]->data = (uint8_t *)malloc(size);
						if ( fread(elf->scp[i_4]->data, size, 1, fp) != 1 )
						{
							fprintf(stderr, "%s: Could not read ELF section contents (errno=%d)\n", filename, errno);
							exit(1);
						}
						swapmemory(elf->scp[i_4]->data, "lllllls", 1);
						break;
					case SHT_SCE_EEMOD:
						elf->scp[i_4]->data = (uint8_t *)malloc(size);
						if ( fread(elf->scp[i_4]->data, size, 1, fp) != 1 )
						{
							fprintf(stderr, "%s: Could not read ELF section contents (errno=%d)\n", filename, errno);
							exit(1);
						}
						swapmemory(elf->scp[i_4]->data, "lllllllllls", 1);
						break;
					default:
						elf->scp[i_4]->data = (uint8_t *)malloc(size);
						if ( fread(elf->scp[i_4]->data, size, 1, fp) != 1 )
						{
							fprintf(stderr, "%s: Could not read ELF section contents (errno=%d)\n", filename, errno);
							exit(1);
						}
						break;
				}
			}
		}
	}
	fclose(fp);
	return elf;
}

static int is_in_range(unsigned int top, unsigned int size, unsigned int pos)
{
	if ( pos >= top && pos < size + top )
	{
		return 1;
	}
	return 0;
}

static void read_symtab(elf_file *elf, int sctindex, FILE *fp)
{
	elf_syment **result;
	elf_section *sp_x;
	unsigned int entrise;
	unsigned int i;

	sp_x = elf->scp[sctindex];
	entrise = sp_x->shr.sh_size / sp_x->shr.sh_entsize;
	result = (elf_syment **)calloc(entrise, sizeof(elf_syment *));
	sp_x->data = (uint8_t *)result;
	for ( i = 0; entrise > i; i += 1 )
	{
		result[i] = (elf_syment *)calloc(1, sizeof(elf_syment));
		if ( fread(result[i], sp_x->shr.sh_entsize, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF symbol table (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(result[i], "lllccs", 1);
		result[i]->bind = result[i]->sym.st_info >> 4;
		result[i]->type = result[i]->sym.st_info & 0xF;
		result[i]->name = 0;
		if ( result[i]->sym.st_name )
		{
			result[i]->name = strdup((char *)&sp_x->link->data[result[i]->sym.st_name]);
		}
		if ( result[i]->sym.st_shndx && result[i]->sym.st_shndx <= 0xFEFF )
		{
			result[i]->shptr = elf->scp[result[i]->sym.st_shndx];
		}
		else
		{
			result[i]->shptr = 0;
		}
	}
}

static void read_rel(elf_file *elf, int sctindex, FILE *fp)
{
	elf_syment **symp;
	elf_rel *result;
	elf_section *sp_x;
	unsigned int entrise;
	unsigned int i;

	sp_x = elf->scp[sctindex];
	entrise = sp_x->shr.sh_size / sp_x->shr.sh_entsize;
	result = (elf_rel *)calloc(entrise, sizeof(elf_rel));
	sp_x->data = (uint8_t *)result;
	symp = (elf_syment **)sp_x->link->data;
	for ( i = 0; entrise > i; i += 1 )
	{
		if ( fread(&result[i], sp_x->shr.sh_entsize, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF relocations (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(&result[i], "ll", 1);
		result[i].type = result[i].rel.r_info & 0xFF;
		// Workaround for malformed file: Handle case of missing .symtab/.strtab section
		if ( symp )
		{
			result[i].symptr = symp[result[i].rel.r_info >> 8];
			result[i].symptr->refcount += 1;
		}
	}
}

static elf_mips_symbolic_data *read_mips_symbolic(FILE *fp)
{
	elf_mips_symbolic_data *sycb;

	sycb = (elf_mips_symbolic_data *)malloc(sizeof(elf_mips_symbolic_data));
	if ( fread(sycb, sizeof(hdrr), 1, fp) != 1 )
	{
		fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
		exit(1);
	}
	swapmemory(sycb, "sslllllllllllllllllllllll", 1);
	if ( sycb->head.cbLineOffset > 0 )
	{
		size_t size_1;

		size_1 = sycb->head.cbLine;
		sycb->cbLine_Ptr = (char *)malloc(size_1);
		fseek(fp, sycb->head.cbLineOffset, SEEK_SET);
		if ( fread(sycb->cbLine_Ptr, size_1, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
	}
	if ( sycb->head.cbDnOffset > 0 )
	{
		size_t size_2;

		size_2 = 8L * sycb->head.idnMax;
		sycb->cbDn_Ptr = (char *)malloc(size_2);
		fseek(fp, sycb->head.cbDnOffset, SEEK_SET);
		if ( fread(sycb->cbDn_Ptr, size_2, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbDn_Ptr, "ll", sycb->head.idnMax);
	}
	if ( sycb->head.cbPdOffset > 0 )
	{
		size_t size_3;

		size_3 = 52L * sycb->head.ipdMax;
		sycb->cbPd_Ptr = (char *)malloc(size_3);
		fseek(fp, sycb->head.cbPdOffset, SEEK_SET);
		if ( fread(sycb->cbPd_Ptr, size_3, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbPd_Ptr, "lllllllllsslll", sycb->head.ipdMax);
	}
	if ( sycb->head.cbSymOffset > 0 )
	{
		size_t size_4;

		size_4 = 12L * sycb->head.isymMax;
		sycb->cbSym_Ptr = (char *)malloc(size_4);
		fseek(fp, sycb->head.cbSymOffset, SEEK_SET);
		if ( fread(sycb->cbSym_Ptr, size_4, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbSym_Ptr, "lll", sycb->head.isymMax);
	}
	if ( sycb->head.cbOptOffset > 0 )
	{
		size_t size_5;

		size_5 = 12L * sycb->head.ioptMax;
		sycb->cbOpt_Ptr = (char *)malloc(size_5);
		fseek(fp, sycb->head.cbOptOffset, SEEK_SET);
		if ( fread(sycb->cbOpt_Ptr, size_5, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbOpt_Ptr, "lll", sycb->head.ioptMax);
	}
	if ( sycb->head.cbAuxOffset > 0 )
	{
		size_t size_6;

		size_6 = 4L * sycb->head.iauxMax;
		sycb->cbAux_Ptr = (char *)malloc(size_6);
		fseek(fp, sycb->head.cbAuxOffset, SEEK_SET);
		if ( fread(sycb->cbAux_Ptr, size_6, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbAux_Ptr, "l", sycb->head.iauxMax);
	}
	if ( sycb->head.cbSsOffset > 0 )
	{
		size_t size_7;

		size_7 = sycb->head.issMax;
		sycb->cbSs_Ptr = (char *)malloc(size_7);
		fseek(fp, sycb->head.cbSsOffset, SEEK_SET);
		if ( fread(sycb->cbSs_Ptr, size_7, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
	}
	if ( sycb->head.cbSsExtOffset > 0 )
	{
		size_t size_8;

		size_8 = sycb->head.issExtMax;
		sycb->cbSsExt_Ptr = (char *)malloc(size_8);
		fseek(fp, sycb->head.cbSsExtOffset, SEEK_SET);
		if ( fread(sycb->cbSsExt_Ptr, size_8, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
	}
	if ( sycb->head.cbFdOffset > 0 )
	{
		size_t size_9;

		size_9 = 72L * sycb->head.ifdMax;
		sycb->cbFd_Ptr = (char *)malloc(size_9);
		fseek(fp, sycb->head.cbFdOffset, SEEK_SET);
		if ( fread(sycb->cbFd_Ptr, size_9, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbFd_Ptr, "llllllllllsslllllll", sycb->head.ifdMax);
	}
	if ( sycb->head.cbRfdOffset > 0 )
	{
		size_t size_A;

		size_A = 4L * sycb->head.crfd;
		sycb->cbRfd_Ptr = (char *)malloc(size_A);
		fseek(fp, sycb->head.cbRfdOffset, SEEK_SET);
		if ( fread(sycb->cbRfd_Ptr, size_A, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbRfd_Ptr, "l", sycb->head.crfd);
	}
	if ( sycb->head.cbExtOffset > 0 )
	{
		size_t size_B;

		size_B = 16L * sycb->head.iextMax;
		sycb->cbExt_Ptr = (char *)malloc(size_B);
		fseek(fp, sycb->head.cbExtOffset, SEEK_SET);
		if ( fread(sycb->cbExt_Ptr, size_B, 1, fp) != 1 )
		{
			fprintf(stderr, "elflib: Could not read ELF debug info contents (errno=%d)\n", errno);
			exit(1);
		}
		swapmemory(sycb->cbExt_Ptr, "sslll", sycb->head.iextMax);
	}
	return sycb;
}

int layout_elf_file(elf_file *elf)
{
	Elf_file_slot *order;

	reorder_symtab(elf);
	rebuild_section_name_strings(elf);
	rebuild_symbol_name_strings(elf);
	order = build_file_order_list(elf);
	shrink_file_order_list(order);
	writeback_file_order_list(elf, order);
	free(order);
	return 0;
}

int write_elf(elf_file *elf, const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "wb");
	if ( !fp )
	{
		fprintf(stderr, "\"%s\" can't open (errno=%d)\n", filename, errno);
		return 1;
	}
	renumber_symtab(elf);
	if ( elf->ehp->e_shnum && elf->ehp->e_shentsize )
	{
		int i_1;
		int i_2;

		for ( i_1 = 0; i_1 < elf->ehp->e_shnum; i_1 += 1 )
		{
			elf->scp[i_1]->number = i_1;
		}
		for ( i_2 = 0; i_2 < elf->ehp->e_shnum; i_2 += 1 )
		{
			switch ( elf->scp[i_2]->shr.sh_type )
			{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
				case SHT_RELA:
				case SHT_REL:
					elf->scp[i_2]->shr.sh_info = elf->scp[i_2]->info->number;
				case SHT_SYMTAB:
				case SHT_HASH:
				case SHT_DYNAMIC:
				case SHT_DYNSYM:
					elf->scp[i_2]->shr.sh_link = elf->scp[i_2]->link->number;
					break;
#pragma GCC diagnostic pop
				default:
					break;
			}
			if ( elf->scp[i_2] == elf->shstrptr )
			{
				elf->ehp->e_shstrndx = elf->scp[i_2]->number;
			}
		}
	}
	swapmemory(elf->ehp, "ccccccccccccccccsslllllssssss", 1);
	fwrite(elf->ehp, sizeof(Elf32_Ehdr), 1, fp);
	swapmemory(elf->ehp, "ccccccccccccccccsslllllssssss", 1);
	if ( elf->ehp->e_phnum && elf->ehp->e_phentsize )
	{
		int count_1;
		int i_3;

		count_1 = elf->ehp->e_phnum;
		fseek(fp, elf->ehp->e_phoff, SEEK_SET);
		for ( i_3 = 0; count_1 > i_3; i_3 += 1 )
		{
			swapmemory(&elf->php[i_3], "llllllll", 1);
			fwrite(&elf->php[i_3], sizeof(Elf32_Phdr), 1, fp);
			swapmemory(&elf->php[i_3], "llllllll", 1);
		}
	}
	if ( elf->ehp->e_shnum && elf->ehp->e_shentsize )
	{
		int count_2;
		int i_4;
		int i_5;

		count_2 = elf->ehp->e_shnum;
		fseek(fp, elf->ehp->e_shoff, SEEK_SET);
		for ( i_4 = 0; count_2 > i_4; i_4 += 1 )
		{
			swapmemory(elf->scp[i_4], "llllllllll", 1);
			fwrite(elf->scp[i_4], sizeof(Elf32_Shdr), 1, fp);
			swapmemory(elf->scp[i_4], "llllllllll", 1);
		}
		for ( i_5 = 0; count_2 > i_5; i_5 += 1 )
		{
			unsigned int size;
			unsigned int pos;

			pos = elf->scp[i_5]->shr.sh_offset;
			size = elf->scp[i_5]->shr.sh_size;
			if ( pos != 0 && size != 0 )
			{
				fseek(fp, pos, SEEK_SET);
				switch ( elf->scp[i_5]->shr.sh_type )
				{
					case SHT_PROGBITS:
					case SHT_RELA:
					case SHT_HASH:
					case SHT_DYNAMIC:
					case SHT_MIPS_REGINFO:
						swapmemory(elf->scp[i_5]->data, "l", size >> 2);
						fwrite(elf->scp[i_5]->data, size, 1, fp);
						swapmemory(elf->scp[i_5]->data, "l", size >> 2);
						break;
					case SHT_SYMTAB:
					case SHT_DYNSYM:
						write_symtab(elf, i_5, fp);
						break;
					case SHT_NOBITS:
						break;
					case SHT_REL:
						write_rel(elf, i_5, fp);
						break;
					case SHT_MIPS_DEBUG:
						write_mips_symbolic((elf_mips_symbolic_data *)elf->scp[i_5]->data, pos, fp);
						break;
					case SHT_SCE_IOPMOD:
						swapmemory(elf->scp[i_5]->data, "lllllls", 1);
						fwrite(elf->scp[i_5]->data, size, 1, fp);
						swapmemory(elf->scp[i_5]->data, "lllllls", 1);
						break;
					case SHT_SCE_EEMOD:
						swapmemory(elf->scp[i_5]->data, "lllllllllls", 1);
						fwrite(elf->scp[i_5]->data, size, 1, fp);
						swapmemory(elf->scp[i_5]->data, "lllllllllls", 1);
						break;
					default:
						fwrite(elf->scp[i_5]->data, size, 1, fp);
						break;
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

static void renumber_a_symtab(elf_section *scp)
{
	elf_syment **syp;
	unsigned int entrise;
	unsigned int i;

	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	syp = (elf_syment **)scp->data;
	for ( i = 0; entrise > i; i += 1 )
	{
		syp[i]->number = i;
	}
}

static void renumber_symtab(elf_file *elf)
{
	int sc;

	for ( sc = 1; sc < elf->ehp->e_shnum; sc += 1 )
	{
		switch ( elf->scp[sc]->shr.sh_type )
		{
			case SHT_SYMTAB:
			case SHT_DYNSYM:
				renumber_a_symtab(elf->scp[sc]);
				break;
			default:
				break;
		}
	}
}

static void write_symtab(elf_file *elf, int sctindex, FILE *fp)
{
	Elf32_Sym sym;
	elf_syment **syp;
	elf_section *sp_x;
	unsigned int entrise;
	unsigned int i;

	sp_x = elf->scp[sctindex];
	entrise = sp_x->shr.sh_size / sp_x->shr.sh_entsize;
	syp = (elf_syment **)sp_x->data;
	fseek(fp, sp_x->shr.sh_offset, SEEK_SET);
	for ( i = 0; entrise > i; i += 1 )
	{
		memcpy(&sym, syp[i], sizeof(sym));
		if ( syp[i]->shptr )
		{
			sym.st_shndx = syp[i]->shptr->number;
		}
		swapmemory(&sym, "lllccs", 1);
		fwrite(&sym, sizeof(Elf32_Sym), 1, fp);
	}
}

static void write_rel(elf_file *elf, int sctindex, FILE *fp)
{
	Elf32_Rel rel;
	elf_rel *rp;
	elf_section *sp_x;
	unsigned int entrise;
	unsigned int i;

	sp_x = elf->scp[sctindex];
	entrise = sp_x->shr.sh_size / sp_x->shr.sh_entsize;
	rp = (elf_rel *)sp_x->data;
	fseek(fp, sp_x->shr.sh_offset, SEEK_SET);
	for ( i = 0; entrise > i; i += 1 )
	{
		memcpy(&rel, &rp[i], sizeof(rel));
		if ( rp[i].symptr && rp[i].symptr->number == (unsigned int)(-1) )
		{
			fprintf(stderr, "Internal error !!\n");
			fprintf(stderr, " relocation entry have no symbol\nabort\n");
			exit(1);
		}
		rel.r_info = ((rp[i].symptr ? rp[i].symptr->number : sp_x->info->number) << 8) + (rp[i].type & 0xFF);
		swapmemory(&rel, "ll", 1);
		fwrite(&rel, sizeof(Elf32_Rel), 1, fp);
	}
}

static void write_mips_symbolic(elf_mips_symbolic_data *sycb, unsigned int basepos, FILE *fp)
{
	unsigned int pos;

	pos = basepos + 96;
	if ( sycb->head.cbLineOffset > 0 )
	{
		size_t size_1;

		size_1 = sycb->head.cbLine;
		sycb->head.cbLineOffset = pos;
		pos += size_1;
		fseek(fp, sycb->head.cbLineOffset, SEEK_SET);
		fwrite(sycb->cbLine_Ptr, size_1, 1, fp);
	}
	if ( sycb->head.cbDnOffset > 0 )
	{
		size_t size_2;

		size_2 = 8L * sycb->head.idnMax;
		sycb->head.cbDnOffset = pos;
		pos += size_2;
		fseek(fp, sycb->head.cbDnOffset, SEEK_SET);
		swapmemory(sycb->cbDn_Ptr, "ll", sycb->head.idnMax);
		fwrite(sycb->cbDn_Ptr, size_2, 1, fp);
		swapmemory(sycb->cbDn_Ptr, "ll", sycb->head.idnMax);
	}
	if ( sycb->head.cbPdOffset > 0 )
	{
		size_t size_3;

		size_3 = 52L * sycb->head.ipdMax;
		sycb->head.cbPdOffset = pos;
		pos += size_3;
		fseek(fp, sycb->head.cbPdOffset, SEEK_SET);
		swapmemory(sycb->cbPd_Ptr, "lllllllllsslll", sycb->head.ipdMax);
		fwrite(sycb->cbPd_Ptr, size_3, 1, fp);
		swapmemory(sycb->cbPd_Ptr, "lllllllllsslll", sycb->head.ipdMax);
	}
	if ( sycb->head.cbSymOffset > 0 )
	{
		size_t size_4;

		size_4 = 12L * sycb->head.isymMax;
		sycb->head.cbSymOffset = pos;
		pos += size_4;
		fseek(fp, sycb->head.cbSymOffset, SEEK_SET);
		swapmemory(sycb->cbSym_Ptr, "lll", sycb->head.isymMax);
		fwrite(sycb->cbSym_Ptr, size_4, 1, fp);
		swapmemory(sycb->cbSym_Ptr, "lll", sycb->head.isymMax);
	}
	if ( sycb->head.cbOptOffset > 0 )
	{
		size_t size_5;

		size_5 = 12L * sycb->head.ioptMax;
		pos += size_5;
		fseek(fp, sycb->head.cbOptOffset, SEEK_SET);
		swapmemory(sycb->cbOpt_Ptr, "lll", sycb->head.ioptMax);
		fwrite(sycb->cbOpt_Ptr, size_5, 1, fp);
		swapmemory(sycb->cbOpt_Ptr, "lll", sycb->head.ioptMax);
	}
	if ( sycb->head.cbAuxOffset > 0 )
	{
		size_t size_6;

		size_6 = 4L * sycb->head.iauxMax;
		sycb->head.cbAuxOffset = pos;
		pos += size_6;
		fseek(fp, sycb->head.cbAuxOffset, SEEK_SET);
		swapmemory(sycb->cbAux_Ptr, "l", sycb->head.iauxMax);
		fwrite(sycb->cbAux_Ptr, size_6, 1, fp);
		swapmemory(sycb->cbAux_Ptr, "l", sycb->head.iauxMax);
	}
	if ( sycb->head.cbSsOffset > 0 )
	{
		size_t size_7;

		size_7 = sycb->head.issMax;
		sycb->head.cbSsOffset = pos;
		pos += size_7;
		fseek(fp, sycb->head.cbSsOffset, SEEK_SET);
		fwrite(sycb->cbSs_Ptr, size_7, 1, fp);
	}
	if ( sycb->head.cbSsExtOffset > 0 )
	{
		size_t size_8;

		size_8 = sycb->head.issExtMax;
		sycb->head.cbSsExtOffset = pos;
		pos += size_8;
		fseek(fp, sycb->head.cbSsExtOffset, SEEK_SET);
		fwrite(sycb->cbSsExt_Ptr, size_8, 1, fp);
	}
	if ( sycb->head.cbFdOffset > 0 )
	{
		size_t size_9;

		size_9 = 72L * sycb->head.ifdMax;
		sycb->head.cbFdOffset = pos;
		pos += size_9;
		fseek(fp, sycb->head.cbFdOffset, SEEK_SET);
		swapmemory(sycb->cbFd_Ptr, "llllllllllsslllllll", sycb->head.ifdMax);
		fwrite(sycb->cbFd_Ptr, size_9, 1, fp);
		swapmemory(sycb->cbFd_Ptr, "llllllllllsslllllll", sycb->head.ifdMax);
	}
	if ( sycb->head.cbRfdOffset > 0 )
	{
		size_t size_A;

		size_A = 4L * sycb->head.crfd;
		sycb->head.cbRfdOffset = pos;
		pos += size_A;
		fseek(fp, sycb->head.cbRfdOffset, SEEK_SET);
		swapmemory(sycb->cbRfd_Ptr, "l", sycb->head.crfd);
		fwrite(sycb->cbRfd_Ptr, size_A, 1, fp);
		swapmemory(sycb->cbRfd_Ptr, "l", sycb->head.crfd);
	}
	if ( sycb->head.cbExtOffset > 0 )
	{
		size_t size_B;

		size_B = 16L * sycb->head.iextMax;
		sycb->head.cbExtOffset = pos;
		fseek(fp, sycb->head.cbExtOffset, SEEK_SET);
		swapmemory(sycb->cbExt_Ptr, "sslll", sycb->head.iextMax);
		fwrite(sycb->cbExt_Ptr, size_B, 1, fp);
		swapmemory(sycb->cbExt_Ptr, "sslll", sycb->head.iextMax);
	}
	fseek(fp, basepos, SEEK_SET);
	swapmemory(sycb, "sslllllllllllllllllllllll", 1);
	fwrite(sycb, sizeof(hdrr), 1, fp);
	swapmemory(sycb, "sslllllllllllllllllllllll", 1);
}

void add_section(elf_file *elf, elf_section *scp)
{
	Elf32_Ehdr *ehp;
	int count;

	ehp = elf->ehp;
	ehp->e_shnum += 1;
	count = ehp->e_shnum;
	elf->scp = (elf_section **)realloc(elf->scp, (count + 1) * sizeof(elf_section *));
	elf->scp[count - 1] = scp;
	elf->scp[count] = 0;
	add_symbol(elf, 0, 0, 3, 0, scp, 0);
}

elf_section *remove_section(elf_file *elf, Elf32_Word shtype)
{
	elf_section *rmsec;
	int s;

	rmsec = 0;
	for ( s = 1; s < elf->ehp->e_shnum; s += 1 )
	{
		if ( shtype == elf->scp[s]->shr.sh_type )
		{
			elf->ehp->e_shnum -= 1;
			rmsec = elf->scp[s];
			break;
		}
	}
	for ( ; s < elf->ehp->e_shnum; s += 1 )
	{
		elf->scp[s] = elf->scp[s + 1];
	}
	return rmsec;
}

elf_section *remove_section_by_name(elf_file *elf, const char *secname)
{
	elf_section *rmsec;
	int s;

	rmsec = 0;
	for ( s = 1; s < elf->ehp->e_shnum; s += 1 )
	{
		if ( !strcmp(elf->scp[s]->name, secname) )
		{
			elf->ehp->e_shnum -= 1;
			rmsec = elf->scp[s];
			break;
		}
	}
	for ( ; s < elf->ehp->e_shnum; s += 1 )
	{
		elf->scp[s] = elf->scp[s + 1];
	}
	return rmsec;
}

elf_section *search_section(elf_file *elf, Elf32_Word stype)
{
	int i;

	for ( i = 1; i < elf->ehp->e_shnum; i += 1 )
	{
		if ( stype == elf->scp[i]->shr.sh_type )
		{
			return elf->scp[i];
		}
	}
	return 0;
}

elf_section *search_section_by_name(elf_file *elf, const char *secname)
{
	int i;

	for ( i = 1; i < elf->ehp->e_shnum; i += 1 )
	{
		if ( !strcmp(elf->scp[i]->name, secname) )
		{
			return elf->scp[i];
		}
	}
	return 0;
}

unsigned int *get_section_data(elf_file *elf, unsigned int addr)
{
	int i;

	for ( i = 1; i < elf->ehp->e_shnum; i += 1 )
	{
		if (
			elf->scp[i]->shr.sh_type == SHT_PROGBITS && addr >= elf->scp[i]->shr.sh_addr
			&& addr < elf->scp[i]->shr.sh_size + elf->scp[i]->shr.sh_addr )
		{
			return (unsigned int *)&elf->scp[i]->data[addr - elf->scp[i]->shr.sh_addr];
		}
	}
	return 0;
}

elf_syment *search_global_symbol(const char *name, elf_file *elf)
{
	unsigned int entrise;
	unsigned int i;
	elf_syment **syp;
	elf_section *scp;

	scp = search_section(elf, SHT_SYMTAB);
	if ( !scp )
	{
		return 0;
	}
	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	syp = (elf_syment **)scp->data;
	for ( i = 1; entrise > i; i += 1 )
	{
		if ( syp[i]->name && syp[i]->bind == STB_GLOBAL && !strcmp(syp[i]->name, name) )
		{
			return syp[i];
		}
	}
	return 0;
}

int is_defined_symbol(const elf_syment *sym)
{
	if ( !sym )
	{
		return 0;
	}
	if ( !sym->sym.st_shndx )
	{
		return 0;
	}
	if ( sym->sym.st_shndx <= 0xFEFF )
	{
		return 1;
	}
	return sym->sym.st_shndx == SHN_ABS;
}

elf_syment *add_symbol(elf_file *elf, const char *name, int bind, int type, int value, elf_section *scp, int st_shndx)
{
	unsigned int entrise;
	elf_syment *sym;
	elf_syment **newtab;
	elf_section *symtbl;

	symtbl = search_section(elf, SHT_SYMTAB);
	if ( !symtbl )
	{
		return 0;
	}
	entrise = symtbl->shr.sh_size / symtbl->shr.sh_entsize;
	newtab = (elf_syment **)realloc(symtbl->data, (entrise + 1) * sizeof(elf_syment *));
	sym = (elf_syment *)calloc(1, sizeof(elf_syment));
	newtab[entrise] = sym;
	symtbl->shr.sh_size += symtbl->shr.sh_entsize;
	symtbl->data = (uint8_t *)newtab;
	if ( name )
	{
		sym->name = strdup(name);
	}
	sym->bind = bind;
	sym->type = type;
	sym->sym.st_info = (type & 0xF) + 16 * bind;
	sym->sym.st_value = value;
	sym->shptr = scp;
	if ( scp )
	{
		sym->sym.st_shndx = 1;
	}
	else
	{
		sym->sym.st_shndx = st_shndx;
	}
	return sym;
}

unsigned int get_symbol_value(const elf_syment *sym, const elf_file *elf)
{
	if ( !is_defined_symbol(sym) )
	{
		return 0;
	}
	if ( sym->sym.st_shndx != SHN_ABS && elf->ehp->e_type == ET_REL )
	{
		return sym->shptr->shr.sh_addr + sym->sym.st_value;
	}
	return sym->sym.st_value;
}

static void reorder_an_symtab(elf_file *elf, elf_section *scp)
{
	int sections;
	elf_syment **oldtab;
	elf_syment **newtab;
	unsigned int entrise;
	int sc;
	unsigned int d;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int m;

	sections = elf->ehp->e_shnum;
	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	oldtab = (elf_syment **)malloc(entrise * sizeof(elf_syment *));
	memcpy(oldtab, (elf_syment **)scp->data, entrise * sizeof(elf_syment *));
	newtab = (elf_syment **)calloc(entrise, sizeof(elf_syment *));
	scp->data = (uint8_t *)newtab;
	*newtab = *oldtab;
	d = 1;
	for ( sc = 1; sections > sc; sc += 1 )
	{
		for ( i = 1; entrise > i; i += 1 )
		{
			if (
				oldtab[i] && oldtab[i]->type == STT_SECTION && !oldtab[i]->name && oldtab[i]->shptr
				&& !strcmp(oldtab[i]->shptr->name, elf->scp[sc]->name) )
			{
				newtab[d] = oldtab[i];
				d += 1;
				oldtab[i] = 0;
				break;
			}
		}
	}
	for ( j = 1; entrise > j; j += 1 )
	{
		if ( oldtab[j] && oldtab[j]->type == STT_SECTION && !oldtab[j]->name )
		{
			oldtab[j] = 0;
		}
	}
	for ( k = 1; entrise > k; k += 1 )
	{
		if ( oldtab[k] && !oldtab[k]->bind )
		{
			newtab[d] = oldtab[k];
			d += 1;
			oldtab[k] = 0;
		}
	}
	scp->shr.sh_info = d;
	for ( m = 1; entrise > m; m += 1 )
	{
		if ( oldtab[m] )
		{
			newtab[d] = oldtab[m];
			d += 1;
			oldtab[m] = 0;
		}
	}
	scp->shr.sh_size = scp->shr.sh_entsize * d;
	free(oldtab);
}

void reorder_symtab(elf_file *elf)
{
	int s;

	for ( s = 1; s < elf->ehp->e_shnum; s += 1 )
	{
		switch ( elf->scp[s]->shr.sh_type )
		{
			case SHT_SYMTAB:
			case SHT_DYNSYM:
				reorder_an_symtab(elf, elf->scp[s]);
				break;
			default:
				break;
		}
	}
}

unsigned int adjust_align(unsigned int value, unsigned int align)
{
	return ~(align - 1) & (align + value - 1);
}

void rebuild_section_name_strings(elf_file *elf)
{
	unsigned int offset;
	size_t namesize;
	int i_1;
	int i_2;

	namesize = 1;
	if ( elf->scp == NULL )
	{
		return;
	}
	for ( i_1 = 1; i_1 < elf->ehp->e_shnum; i_1 += 1 )
	{
		namesize += strlen(elf->scp[i_1]->name) + 1;
	}
	if ( elf->shstrptr->data )
	{
		free(elf->shstrptr->data);
	}
	elf->shstrptr->data = (uint8_t *)calloc(1, namesize);
	elf->shstrptr->shr.sh_size = namesize;
	offset = 1;
	for ( i_2 = 1; i_2 < elf->ehp->e_shnum; i_2 += 1 )
	{
		strcpy((char *)&elf->shstrptr->data[offset], elf->scp[i_2]->name);
		elf->scp[i_2]->shr.sh_name = offset;
		offset += strlen(elf->scp[i_2]->name) + 1;
	}
}

static size_t search_string_table(const char *tbltop, size_t endindex, const char *str)
{
	size_t idx;

	for ( idx = 1; idx < endindex; idx += strlen(&tbltop[idx]) + 1 )
	{
		if ( !strcmp(str, &tbltop[idx]) )
		{
			return idx;
		}
	}
	return 0;
}

static void rebuild_a_symbol_name_strings(elf_section *scp)
{
	elf_section *strtab;
	elf_syment **syp;
	size_t offset;
	size_t namesize;
	unsigned int entrise;
	unsigned int i_1;
	unsigned int i_2;

	entrise = scp->shr.sh_size / scp->shr.sh_entsize;
	strtab = scp->link;
	syp = (elf_syment **)scp->data;
	namesize = 1;
	for ( i_1 = 1; i_1 < entrise; i_1 += 1 )
	{
		namesize = (syp[i_1] != NULL && syp[i_1]->name != NULL) ? (strlen(syp[i_1]->name) + namesize + 1) : namesize;
	}
	if ( strtab->data )
	{
		free(strtab->data);
	}
	strtab->data = (uint8_t *)calloc(1, namesize);
	offset = 1;
	for ( i_2 = 1; i_2 < entrise; i_2 += 1 )
	{
		if ( syp[i_2] != NULL && syp[i_2]->name != NULL )
		{
			syp[i_2]->sym.st_name = search_string_table((char *)strtab->data, offset, syp[i_2]->name);
			if ( !syp[i_2]->sym.st_name )
			{
				strcpy((char *)&strtab->data[offset], syp[i_2]->name);
				syp[i_2]->sym.st_name = offset;
				offset += strlen(syp[i_2]->name) + 1;
			}
		}
	}
	strtab->shr.sh_size = offset;
}

void rebuild_symbol_name_strings(elf_file *elf)
{
	int sc;

	if ( elf->scp == NULL )
	{
		return;
	}
	for ( sc = 1; sc < elf->ehp->e_shnum; sc += 1 )
	{
		switch ( elf->scp[sc]->shr.sh_type )
		{
			case SHT_SYMTAB:
			case SHT_DYNSYM:
				rebuild_a_symbol_name_strings(elf->scp[sc]);
				break;
			default:
				break;
		}
	}
}

static int comp_Elf_file_slot(const void *a1, const void *a2)
{
	const Elf_file_slot *p1;
	const Elf_file_slot *p2;

	p1 = a1;
	p2 = a2;

	if ( p1->type == EFS_TYPE_ELF_HEADER && p2->type == EFS_TYPE_ELF_HEADER )
	{
		return 0;
	}
	if ( p1->type == EFS_TYPE_ELF_HEADER )
	{
		return -1;
	}
	if ( p2->type == EFS_TYPE_ELF_HEADER )
	{
		return 1;
	}
	if ( p1->type == EFS_TYPE_PROGRAM_HEADER_TABLE && p2->type == EFS_TYPE_PROGRAM_HEADER_TABLE )
	{
		return 0;
	}
	if ( p1->type == EFS_TYPE_PROGRAM_HEADER_TABLE )
	{
		return -1;
	}
	if ( p2->type == EFS_TYPE_PROGRAM_HEADER_TABLE )
	{
		return 1;
	}
	if ( !p1->type && !p2->type )
	{
		return 0;
	}
	if ( !p1->type )
	{
		return 1;
	}
	if ( !p2->type )
	{
		return -1;
	}
	if ( p1->type == EFS_TYPE_END && p2->type == EFS_TYPE_END )
	{
		return 0;
	}
	if ( p1->type == EFS_TYPE_END )
	{
		return 1;
	}
	if ( p2->type == EFS_TYPE_END )
	{
		return -1;
	}
	if ( p1->type == EFS_TYPE_PROGRAM_HEADER_ENTRY && p2->type == EFS_TYPE_SECTION_HEADER_TABLE )
	{
		return -1;
	}
	if ( p1->type == EFS_TYPE_SECTION_HEADER_TABLE && p2->type == EFS_TYPE_PROGRAM_HEADER_ENTRY )
	{
		return 1;
	}
	if ( p2->offset == p1->offset )
	{
		return 0;
	}
	if ( p2->offset >= p1->offset )
	{
		return -1;
	}
	return 1;
}

Elf_file_slot *build_file_order_list(const elf_file *elf)
{
	int sections;
	elf_section **scp;
	Elf_file_slot *resolt;
	int s_2;
	int d_1;
	size_t d_2;
	int maxent;

	sections = elf->ehp->e_shnum;
	scp = (elf_section **)calloc(sections + 1, sizeof(elf_section *));
	memcpy(scp, elf->scp, sections * sizeof(elf_section *));
	maxent = elf->ehp->e_shnum + 2;
	if ( elf->ehp->e_phnum )
	{
		maxent = elf->ehp->e_phnum + elf->ehp->e_shnum + 3;
	}
	resolt = (Elf_file_slot *)calloc(maxent, sizeof(Elf_file_slot));
	resolt->type = EFS_TYPE_ELF_HEADER;
	resolt->offset = 0;
	resolt->size = sizeof(Elf32_Ehdr);
	resolt->align = 4;
	d_1 = 1;
	if ( elf->ehp->e_phnum )
	{
		int seg;

		resolt[1].type = EFS_TYPE_PROGRAM_HEADER_TABLE;
		resolt[1].offset = resolt->size;
		resolt[1].size = sizeof(Elf32_Phdr) * elf->ehp->e_phnum;
		resolt[1].align = 4;
		for ( seg = 0, d_1 = 2; seg < elf->ehp->e_phnum; seg += 1, d_1 += 1 )
		{
			elf_section **phdscp;

			resolt[d_1].type = EFS_TYPE_PROGRAM_HEADER_ENTRY;
			resolt[d_1].d.php = &elf->php[seg];
			resolt[d_1].offset = elf->php[seg].phdr.p_offset;
			resolt[d_1].size = elf->php[seg].phdr.p_filesz;
			resolt[d_1].align = elf->php[seg].phdr.p_align;
			for ( phdscp = elf->php[seg].scp; *phdscp; phdscp += 1 )
			{
				int s_1;

				for ( s_1 = 0; s_1 < sections; s_1 += 1 )
				{
					if ( *phdscp == scp[s_1] )
					{
						scp[s_1] = 0;
						break;
					}
				}
			}
		}
	}
	resolt[d_1].type = EFS_TYPE_SECTION_HEADER_TABLE;
	resolt[d_1].offset = elf->ehp->e_shoff ?: (Elf32_Off)(-256);
	resolt[d_1].size = sizeof(Elf32_Shdr) * elf->ehp->e_shnum;
	resolt[d_1].align = 4;
	d_2 = d_1 + 1;
	for ( s_2 = 1; s_2 < sections; s_2 += 1 )
	{
		if ( scp[s_2] )
		{
			resolt[d_2].type = EFS_TYPE_SECTION_DATA;
			resolt[d_2].d.scp = scp[s_2];
			resolt[d_2].offset = scp[s_2]->shr.sh_offset;
			resolt[d_2].size = scp[s_2]->shr.sh_size;
			resolt[d_2].align = scp[s_2]->shr.sh_addralign;
			scp[s_2] = 0;
			d_2 += 1;
		}
	}
	resolt[d_2].type = EFS_TYPE_END;
	free(scp);
	qsort(resolt, d_2, sizeof(Elf_file_slot), comp_Elf_file_slot);
	return resolt;
}

void shrink_file_order_list(Elf_file_slot *efs)
{
	unsigned int slot;

	slot = 0;
	for ( ; efs->type != EFS_TYPE_END; efs += 1 )
	{
		unsigned int foffset;

		foffset = adjust_align(slot, efs->align);
		efs->offset = foffset;
		slot = efs->size + foffset;
	}
}

void writeback_file_order_list(elf_file *elf, Elf_file_slot *efs)
{
	elf_section **scp;
	unsigned int segoffset;
	int i;

	for ( ; efs->type != EFS_TYPE_END; efs += 1 )
	{
		switch ( efs->type )
		{
			case EFS_TYPE_PROGRAM_HEADER_TABLE:
				elf->ehp->e_phoff = efs->offset;
				break;
			case EFS_TYPE_PROGRAM_HEADER_ENTRY:
				efs->d.php->phdr.p_offset = efs->offset;
				scp = efs->d.php->scp;
				segoffset = efs->offset;
				(*scp)->shr.sh_offset = efs->offset;
				for ( i = 1; scp[i]; i += 1 )
				{
					if ( scp[i]->shr.sh_type != SHT_NOBITS )
					{
						segoffset = scp[i]->shr.sh_addr + efs->offset - (*scp)->shr.sh_addr;
					}
					scp[i]->shr.sh_offset = segoffset;
					if ( scp[i]->shr.sh_type != SHT_NOBITS )
					{
						segoffset += scp[i]->shr.sh_size;
					}
				}
				break;
			case EFS_TYPE_SECTION_HEADER_TABLE:
				elf->ehp->e_shoff = efs->offset;
				break;
			case EFS_TYPE_SECTION_DATA:
				efs->d.php->phdr.p_filesz = efs->offset;
				break;
			default:
				break;
		}
	}
}

void dump_file_order_list(const elf_file *elf, const Elf_file_slot *efs)
{
	unsigned int offset;
	unsigned int size_tmp;
	unsigned int offset_tmp;
	char tmp[100];
	const char *name;
	elf_section **scp;
	const Elf_file_slot *slot;
	int i;

	offset_tmp = efs->offset;
	printf("\n");
	for ( slot = efs; slot->type != EFS_TYPE_END; slot += 1 )
	{
		unsigned int oldend_1;
		unsigned int size_1;
		unsigned int startpos_1;

		oldend_1 = slot->size;
		startpos_1 = slot->offset;
		if ( oldend_1 == 0 )
		{
			offset = slot->offset;
		}
		else
		{
			offset = oldend_1 + startpos_1 - 1;
		}
		size_1 = offset;
		switch ( slot->type )
		{
			case EFS_TYPE_ELF_HEADER:
				name = "[Elf header]";
				break;
			case EFS_TYPE_PROGRAM_HEADER_TABLE:
				name = "[Proram Header Table]";
				break;
			case EFS_TYPE_PROGRAM_HEADER_ENTRY:
				sprintf(tmp, "[Proram Header entry %d]", (int)(0xCCCCCCCD * ((char *)slot->d.php - (char *)elf->php)) >> 3);
				name = tmp;
				break;
			case EFS_TYPE_SECTION_HEADER_TABLE:
				name = "[Section Header Table]";
				break;
			case EFS_TYPE_SECTION_DATA:
				sprintf(tmp, "%s data", slot->d.scp->name);
				name = tmp;
				break;
			default:
				name = "internal error Unknown EFS type !!!!";
				break;
		}
		if ( startpos_1 > offset_tmp + 1 )
		{
			printf("%36s = %08x-%08x (%06x)\n", "**Blank**", offset_tmp + 1, startpos_1 - 1, startpos_1 - offset_tmp - 1);
		}
		offset_tmp = size_1;
		printf("%36s = %08x-%08x (%06x)\n", name, startpos_1, size_1, oldend_1);
		if ( slot->type == EFS_TYPE_PROGRAM_HEADER_ENTRY )
		{
			scp = slot->d.php->scp;
			(*scp)->shr.sh_offset = slot->offset;
			size_tmp = slot->offset;
			for ( i = 0; scp[i]; i += 1 )
			{
				unsigned int oldend_2;
				unsigned int size_2;
				unsigned int startpos_2;

				if ( scp[i]->shr.sh_type == SHT_NOBITS )
				{
					oldend_2 = 0;
				}
				else
				{
					oldend_2 = scp[i]->shr.sh_size;
				}
				startpos_2 = scp[i]->shr.sh_offset;
				size_2 = (oldend_2 == 0) ? (scp[i]->shr.sh_offset) : (Elf32_Off)(oldend_2 + startpos_2 - 1);
				sprintf(tmp, "(%s)", scp[i]->name);
				name = tmp;
				if ( startpos_2 > size_tmp + 1 )
				{
					printf("%36s | %08x-%08x (%06x)\n", "**Blank**", size_tmp + 1, startpos_2 - 1, startpos_2 - size_tmp - 1);
				}
				size_tmp = size_2;
				printf("%36s | %08x-%08x (%06x)\n", name, startpos_2, size_2, scp[i]->shr.sh_size);
			}
		}
	}
}
