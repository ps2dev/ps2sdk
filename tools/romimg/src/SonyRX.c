/*
    SonyRX.c	- Contains functions for handling Sony Relocatable eXecutable files (e.g. IRX and ERX files).
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ELF.h"
#include "SonyRX.h"

int IsSonyRXModule(const char *path)
{
	FILE *file;
	elf_header_t header;
	elf_shdr_t SectionHeader;
	int result;

	result = 0;
	if ((file = fopen(path, "rb")) != NULL) {
		if (fread(&header, 1, sizeof(elf_header_t), file) != 0) {
            if (*(uint32_t *)header.ident == ELF_MAGIC && (header.type == ELF_TYPE_ERX2 || header.type == ELF_TYPE_IRX)) {
                unsigned int i;
                for (i = 0; i < header.shnum; i++) {
                    fseek(file, header.shoff + i * header.shentsize, SEEK_SET);
                    if (fread(&SectionHeader, 1, sizeof(elf_shdr_t), file) != 0) {
                        if ((SectionHeader.type == (SHT_LOPROC | SHT_LOPROC_EEMOD_TAB)) || (SectionHeader.type == (SHT_LOPROC | SHT_LOPROC_IOPMOD_TAB))) {
                            result = 1;
                            break;
                        }
                    }
                }
            }
        }

		fclose(file);
	}

	return result;
}

int GetSonyRXModInfo(const char *path, char *description, unsigned int MaxLength, unsigned short int *version)
{
	FILE *file;
	int result;
	elf_header_t header;
	elf_shdr_t SectionHeader;

	result = ENOENT;
	if ((file = fopen(path, "rb")) != NULL) {
		if (fread(&header, 1, sizeof(elf_header_t), file) != 0) {
            if (*(uint32_t *)header.ident == ELF_MAGIC && (header.type == ELF_TYPE_ERX2 || header.type == ELF_TYPE_IRX)) {
                unsigned int i;
                for (i = 0; i < header.shnum; i++) {
                    fseek(file, header.shoff + i * header.shentsize, SEEK_SET);
                    if (fread(&SectionHeader, 1, sizeof(elf_shdr_t), file) != 0) {

                        if (SectionHeader.type == (SHT_LOPROC | SHT_LOPROC_EEMOD_TAB) || SectionHeader.type == (SHT_LOPROC | SHT_LOPROC_IOPMOD_TAB)) {
                            void *buffer;
                            if ((buffer = malloc(SectionHeader.size)) != NULL) {
                                fseek(file, SectionHeader.offset, SEEK_SET);
                                if (fread(buffer, 1, SectionHeader.size, file) != 0) {

                                    if (SectionHeader.type == (SHT_LOPROC | SHT_LOPROC_IOPMOD_TAB)) {
                                        *version = ((iopmod_t *)buffer)->version;
                                        strncpy(description, ((iopmod_t *)buffer)->modname, MaxLength - 1);
                                        description[MaxLength - 1] = '\0';
                                    } else if (SectionHeader.type == (SHT_LOPROC | SHT_LOPROC_EEMOD_TAB)) {
                                        *version = ((eemod_t *)buffer)->version;
                                        strncpy(description, ((eemod_t *)buffer)->modname, MaxLength - 1);
                                        description[MaxLength - 1] = '\0';
                                    }
                                }

                                result = 0;

                                free(buffer);
                            } else
                                result = ENOMEM;
                            break;
                        }
                    }
                }
            } else
                result = EINVAL;
        }

		fclose(file);
	} else
		result = ENOENT;

	return result;
}
