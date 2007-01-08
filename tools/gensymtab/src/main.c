/*
	Generate a symbol table encapsulated in a .o file.
	Supports reading global symbols from elfs (.o, .elf), ranlibs (.a) and text files (.l)
	The text files should have one symbol per line, terminated with \n (not \r\n)

	NOTES;
		Doesn't support big-endian systems (sorry powerMac users)
		Requires a system elf.h and ar.h (sorry windows users)
		String handling is pretty evil, needs cleaning up
		Actually, everything wants to be cleaned up a bit :)
*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <ar.h>
#include <string.h>
#include <stdlib.h>
#include <byteswap.h>
#include <stdarg.h>

int harvest_text(const char *filename, int (*call_symbol)(void *user, int type, const char *name), void *user)
{
	FILE *f = fopen(filename, "r");
	if (f==NULL)
	{
		printf("%s: Failed to open (%s)\n", filename, strerror(errno));
		return(-1);
	}

	while(!feof(f))
	{
		char buf[512];
		fgets(buf, sizeof(buf), f);
		if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
		call_symbol(user, STT_OBJECT, buf);
	}

	fclose(f);
	return(0);
}

int harvest_ar(const char *filename, int (*call_symbol)(void *user, int type, const char *name), void *user)
{
	int fdi = open(filename, O_RDONLY);
	if (fdi<0)
	{
		printf("%s: Failed to open (%s)\n", filename, strerror(errno));
		return(-1);
	}

	char armag[SARMAG];
	read(fdi, armag, SARMAG);
	if (memcmp(armag, ARMAG, SARMAG))
	{
		close(fdi);
		printf("%s: not an AR file!\n", filename);
		return(-1);
	}

	struct ar_hdr arhdr;
	read(fdi, &arhdr, sizeof(struct ar_hdr));

	if (arhdr.ar_name[0]!='/' && arhdr.ar_name[0]!='\0')
	{
		close(fdi);
		printf("%s: AR file doesn't seem to have symbol table!\n", filename);
		return(-1);
	}

	char buf[sizeof(arhdr.ar_size)+1];
	memcpy(buf, arhdr.ar_size, sizeof(arhdr.ar_size));
	buf[sizeof(arhdr.ar_size)] = '\0';

	int size = atoi(buf);
	if (size==0)
	{
		close(fdi);
		printf("%s: AR file doesn't seem to have symbol table! (%s)\n", filename, buf);
		return(-1);
	}

	char *data = malloc(size);
	read(fdi, data, size);

	int symbols = bswap_32(*(int*)data);

	int *type = (int*)data+4;
	char *names = data+4+(4*symbols);
	for (int i=0;i<symbols;i++)
	{
		call_symbol(user, STT_OBJECT, names);
		names+=strlen(names)+1;
		type++;
	}

	free(data);


	close(fdi);
	return(0);
}

int harvest_elf(const char *filename, int (*call_symbol)(void *user, int type, const char *name), void *user)
{
	int symcnt=0;
	char *shstrs=NULL, *strs=NULL;
	int fdi = open(filename, O_RDONLY);
	if (fdi<0)
	{
		printf("%s: Failed to open (%s)\n", filename, strerror(errno));
		return(-1);
	}

	Elf32_Ehdr ehdr;
	read(fdi, &ehdr, sizeof(Elf32_Ehdr));
	if ((ehdr.e_ident[EI_MAG0] != ELFMAG0) || 
		(ehdr.e_ident[EI_MAG1] != ELFMAG1) ||
		(ehdr.e_ident[EI_MAG2] != ELFMAG2) ||
		(ehdr.e_ident[EI_MAG3] != ELFMAG3))
	{
		printf("%s: Doesn't seem to be an ELF!\n", filename);
		return(-1);
	}

	lseek(fdi, ehdr.e_shoff+(sizeof(Elf32_Shdr)*ehdr.e_shstrndx), SEEK_SET);
	{
		Elf32_Shdr shdr;
		read(fdi, &shdr, sizeof(Elf32_Shdr));
		shstrs = malloc(shdr.sh_size);
		lseek(fdi, shdr.sh_offset, SEEK_SET);
		read(fdi, shstrs, shdr.sh_size);
	}

	Elf32_Sym *syms = NULL;
	for (int i=0;i<ehdr.e_shnum;i++)
	{
		Elf32_Shdr shdr;

		lseek(fdi, ehdr.e_shoff+(sizeof(Elf32_Shdr)*i), SEEK_SET);

		read(fdi, &shdr, sizeof(Elf32_Shdr));
		if ((!strcmp(&shstrs[shdr.sh_name], ".strtab")) && shdr.sh_type == SHT_STRTAB)
		{
			strs = malloc(shdr.sh_size);
			lseek(fdi, shdr.sh_offset, SEEK_SET);
			read(fdi, strs, shdr.sh_size);
		}

		if ((!strcmp(&shstrs[shdr.sh_name], ".symtab")) && shdr.sh_type == SHT_SYMTAB)
		{
			if (syms!=NULL)
			{
				printf("%s: More than one .symtab sections? using latest.\n", filename);
				free(syms);
			}
			syms = malloc(shdr.sh_size);
			lseek(fdi, shdr.sh_offset, SEEK_SET);
			read(fdi, syms, shdr.sh_size);
			symcnt = shdr.sh_size/shdr.sh_entsize;
		}
	}

	for (int i=0;i<symcnt;i++)
	{
		if (ELF32_ST_BIND(syms[i].st_info) == STB_GLOBAL)
		{
			if (syms[i].st_shndx==STN_UNDEF) // XXX: Maybe needed?
				continue;
			call_symbol(user, ELF32_ST_TYPE(syms[i].st_info), &strs[syms[i].st_name]);
		}
	}


	if (shstrs!=NULL)
		free(shstrs);
	if (strs!=NULL)
		free(strs);
	if (syms!=NULL)
		free(syms);


	close(fdi);
	return(0);
}

typedef struct
{
	int Size;
	char *Buffer;
	char *Cur;
} elf_str;


void elf_str_start(elf_str *s, int size)
{
	s->Size = size;
	s->Buffer = malloc(size);
	s->Buffer[0] = '\0';
	s->Cur = s->Buffer+1;
}

int elf_str_add(elf_str *s, const char *format, ...)
{
	int idx = s->Cur-s->Buffer;
	char buf[512];
	va_list va;
	va_start(va, format);
	vsnprintf(buf, sizeof(buf), format, va);
	va_end(va);

	int len = strlen(buf);

	if (len>(s->Size-(s->Cur-s->Buffer)))
	{
		printf("Failed to add '%s'\n", buf);
		return(0);
	}
	
	strcpy(s->Cur, buf);
	s->Cur[len] = '\0';
	s->Cur += len+1;
	return(idx);
}

int elf_str_size(elf_str *s)
{
	return(s->Cur-s->Buffer);
}


typedef struct symbol_ll
{
	const char *Name;
	int Type;
	struct symbol_ll *Next;
} symbol_ll;

typedef struct
{
	symbol_ll *First;
	int Count;
} symbol;

int symtab_write(const char *filename, const char *name, symbol *s)
{
	int cur = 0;
	FILE *file;
	elf_str shstr;
	elf_str str;
	Elf32_Ehdr ehdr;
	Elf32_Shdr shdr[16];
	Elf32_Sym sym;
	Elf32_Rel rel;

	file = fopen(filename, "wb");
	if (file==NULL)
		return(-1);

	elf_str_start(&shstr, 512);
	elf_str_start(&str, 512*1024);

	cur += sizeof(Elf32_Ehdr);
	ehdr.e_ident[EI_MAG0]       = ELFMAG0;
	ehdr.e_ident[EI_MAG1]       = ELFMAG1;
	ehdr.e_ident[EI_MAG2]       = ELFMAG2;
	ehdr.e_ident[EI_MAG3]       = ELFMAG3;
	ehdr.e_ident[EI_CLASS]      = ELFCLASS32;
	ehdr.e_ident[EI_DATA]       = ELFDATA2LSB;
	ehdr.e_ident[EI_VERSION]    = EV_CURRENT;
	ehdr.e_ident[EI_OSABI]      = ELFOSABI_SYSV;
	ehdr.e_ident[EI_ABIVERSION] = 0;
	ehdr.e_ident[EI_PAD]        = 0;
	ehdr.e_type      = ET_REL;
	ehdr.e_machine   = EM_MIPS;
	ehdr.e_version   = 0x1;
	ehdr.e_entry     = 0x0000;
	ehdr.e_phoff     = 0;
	ehdr.e_shoff     = cur;
	ehdr.e_flags     = 0x20924001; // HRM?
	ehdr.e_ehsize    = sizeof(Elf32_Ehdr);
	ehdr.e_phentsize = sizeof(Elf32_Phdr);
	ehdr.e_phnum     = 0;
	ehdr.e_shentsize = sizeof(Elf32_Shdr);
	ehdr.e_shnum     = 6;
	ehdr.e_shstrndx  = 1;

	cur += ehdr.e_shentsize * ehdr.e_shnum;

	/* Make section names first.... */
	shdr[0].sh_name      = 0;
	shdr[1].sh_name      = elf_str_add(&shstr, ".shstrtab");
	shdr[2].sh_name      = elf_str_add(&shstr, ".symtab");
	shdr[3].sh_name      = elf_str_add(&shstr, ".data");
	shdr[4].sh_name      = elf_str_add(&shstr, ".rel.data");
	shdr[5].sh_name      = elf_str_add(&shstr, ".strtab");

	// NULL section
	shdr[0].sh_type      = SHT_NULL;
	shdr[0].sh_flags     = 0;
	shdr[0].sh_addr      = 0;
	shdr[0].sh_offset    = 0;
	shdr[0].sh_size      = 0;
	shdr[0].sh_link      = 0;
	shdr[0].sh_info      = 0;
	shdr[0].sh_addralign = 0;
	shdr[0].sh_entsize   = 0;

	/* Write out section header string table */
	shdr[1].sh_type      = SHT_STRTAB;
	shdr[1].sh_flags     = 0;
	shdr[1].sh_addr      = 0;
	shdr[1].sh_offset    = cur;
	shdr[1].sh_size      = elf_str_size(&shstr);
	shdr[1].sh_link      = 0;
	shdr[1].sh_info      = 0;
	shdr[1].sh_addralign = 1;
	shdr[1].sh_entsize   = 0;
	cur = shdr[1].sh_offset + shdr[1].sh_size;
	fseek(file, shdr[1].sh_offset, SEEK_SET);
	fwrite(shstr.Buffer, shdr[1].sh_size, 1, file);


	/*
		Write out symbol table
	*/
	shdr[2].sh_type      = SHT_SYMTAB;
	shdr[2].sh_flags     = 0;
	shdr[2].sh_addr      = 0;
	shdr[2].sh_offset    = cur;
	shdr[2].sh_size      = sizeof(Elf32_Sym)*(3+s->Count);
	shdr[2].sh_link      = 5;  // .strtab
	shdr[2].sh_info      = 0;
	shdr[2].sh_addralign = 4;
	shdr[2].sh_entsize   = sizeof(Elf32_Sym);
	cur = shdr[2].sh_offset + shdr[2].sh_size;

	fseek(file, shdr[2].sh_offset, SEEK_SET);
	sym.st_name  = 0;
	sym.st_info  = 0;
	sym.st_other = 0;
	sym.st_shndx = 0;
	sym.st_value = 0;
	sym.st_size  = 0;
	fwrite(&sym, shdr[2].sh_entsize, 1, file);

	sym.st_name  = 0;
	sym.st_info  = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
	sym.st_other = 0;
	sym.st_shndx = 3; // .data
	sym.st_value = 0;
	sym.st_size  = 0;
	fwrite(&sym, shdr[2].sh_entsize, 1, file);

	sym.st_name  = elf_str_add(&str, "_symtab_%s", name);
	sym.st_info  = ELF32_ST_INFO(STB_GLOBAL, STT_OBJECT);
	sym.st_other = 0;
	sym.st_shndx = 3; // .data
	sym.st_value = 0;
	sym.st_size  = 1;
	fwrite(&sym, shdr[2].sh_entsize, 1, file);


	for (symbol_ll *cur=s->First;cur!=NULL;cur=cur->Next)
	{
		sym.st_name  = elf_str_add(&str, "%s", cur->Name);
		sym.st_info  = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
		sym.st_other = 0;
		sym.st_shndx = 0;
		sym.st_value = 0;
		sym.st_size  = 0;
		fwrite(&sym, shdr[2].sh_entsize, 1, file);
	}

	/*
		Write out data area
	*/
	shdr[3].sh_type      = SHT_PROGBITS;
	shdr[3].sh_flags     = SHF_WRITE | SHF_ALLOC;
	shdr[3].sh_addr      = 0;
	shdr[3].sh_offset    = cur;
	shdr[3].sh_link      = 0;
	shdr[3].sh_info      = 0;
	shdr[3].sh_addralign = 16;
	shdr[3].sh_entsize   = 0;

	elf_str prgstr;
	elf_str_start(&prgstr, 512*1024);

	fseek(file, shdr[3].sh_offset, SEEK_SET);
	unsigned int dat = 0;
	for (symbol_ll *cur=s->First;cur!=NULL;cur=cur->Next)
	{
		dat = 0x00000000;
		fwrite(&dat, 4, 1, file);

		dat = (8*(s->Count+1))+elf_str_add(&prgstr, "%s", cur->Name);
		fwrite(&dat, 4, 1, file);
	}
	dat = 0;
	fwrite(&dat, 4, 1, file);
	fwrite(&dat, 4, 1, file);
	fwrite(prgstr.Buffer, elf_str_size(&prgstr), 1, file);

	shdr[3].sh_size  = (8*(s->Count+1))+elf_str_size(&prgstr); 
	cur = shdr[3].sh_offset + shdr[3].sh_size;


	/*
		Write out relocation table
	*/
	shdr[4].sh_type      = SHT_REL;
	shdr[4].sh_flags     = 0;
	shdr[4].sh_addr      = 0;
	shdr[4].sh_offset    = cur;
	shdr[4].sh_size      = sizeof(Elf32_Rel)*(s->Count*2); 
	shdr[4].sh_link      = 2;   // .symtab
	shdr[4].sh_info      = 3;   // .data
	shdr[4].sh_addralign = 16;
	shdr[4].sh_entsize   = sizeof(Elf32_Rel);
	cur = shdr[4].sh_offset + shdr[4].sh_size;

	fseek(file, shdr[4].sh_offset, SEEK_SET);
	for (int i=0;i<s->Count;i++)
	{
		// Symbol pointer
		rel.r_offset = i*8;
		rel.r_info   = ELF32_R_INFO(3+i, R_MIPS_32);
		fwrite(&rel, shdr[4].sh_entsize, 1, file);

		// String offset
		rel.r_offset = (i*8)+4;
		rel.r_info   = ELF32_R_INFO(1, R_MIPS_32);
		fwrite(&rel, shdr[4].sh_entsize, 1, file);
	}

	/*
		Write out string table
	*/
	shdr[5].sh_type      = SHT_STRTAB;
	shdr[5].sh_flags     = 0;
	shdr[5].sh_addr      = 0;
	shdr[5].sh_offset    = cur;
	shdr[5].sh_size      = elf_str_size(&str);
	shdr[5].sh_link      = 0;
	shdr[5].sh_info      = 0;
	shdr[5].sh_addralign = 1;
	shdr[5].sh_entsize   = 0;
	cur = shdr[5].sh_offset + shdr[5].sh_size;

	fseek(file, shdr[5].sh_offset, SEEK_SET);
	fwrite(str.Buffer, shdr[5].sh_size, 1, file);

	/* Write out section headers*/
	fseek(file, ehdr.e_shoff, SEEK_SET);
	fwrite(shdr, ehdr.e_shentsize, ehdr.e_shnum, file);

	/* Write out ELF header */
	fseek(file, 0, SEEK_SET);
	fwrite(&ehdr, ehdr.e_ehsize, 1, file);


	return(0);
}

int symtab_symbol(void *user, int type, const char *name)
{
	symbol *s = user;

	if (type!=STT_OBJECT && type!=STT_FUNC)
		return(0);

	symbol_ll *sn = malloc(sizeof(symbol_ll));
	sn->Name = strdup(name);
	sn->Next = s->First;
	sn->Type = type;
	s->First = sn;
	s->Count++;
	return(1);
}

int main(int argc, char *argv[])
{
	if (argc<4)
	{
		printf("usage:\n%s <name> output_symtab.o [input elfs]\n", argv[0]);
		return(1);
	}
	symbol syms; 
	syms.First = NULL;
	syms.Count = 0;

	for (int i=3;i<argc;i++)
	{
		int len = strlen(argv[i]);
		if (argv[i][len-2] == '.')
		{
			if (argv[i][len-1] == 'o')
			{
				harvest_elf(argv[i], symtab_symbol, &syms);
				continue;
			}
			else if (argv[i][len-1] == 'a')
			{
				harvest_ar(argv[i], symtab_symbol, &syms);
				continue;
			}
			else if (argv[i][len-1] == 'l')
			{
				harvest_text(argv[i], symtab_symbol, &syms);
				continue;
			}
		}
		printf("%s: Unknown filetype!\n", argv[i]);
	}

	symtab_write(argv[2], argv[1], &syms);


	return(0);
}
