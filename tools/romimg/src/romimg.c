/*
    roming.c	- ROM image file manager.
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "platform.h"
#include "romimg.h"
#include "SonyRX.h"

#define BUFCHK(X) (X[0] == '\0') ? "" : X
#define IMAGE_COMMENT_BASESIZE 31

struct ROMImgStat
{
	void *image;
	unsigned int size;
	void *ROMFS_start;
};

struct RomDirFileFd
{
	unsigned int FileOffset; /* The offset into the image at which the file exists at. */
	unsigned int ExtInfoOffset;
	unsigned int size;
	unsigned int offset;
	unsigned short int ExtInfoEntrySize;
};

static int GetRomDirExtInfoOffset(const struct ROMImgStat *ImageStat, struct RomDirFileFd *fd)
{
	const struct RomDirEntry *RomDirEntry;
	int result;
	unsigned int offset;
	RomDirEntry = (const struct RomDirEntry *)ImageStat->ROMFS_start; // (struct RomDirEntry *)
	result = ENOENT;
	offset = 0;
	while (RomDirEntry->name[0] != '\0') {
		if (strncmp("EXTINFO", RomDirEntry->name, sizeof(RomDirEntry->name)) == 0) {
			DPRINTF("Found 'EXTINFO' at 0x%x\n", offset);
			fd->ExtInfoOffset = offset;
			result = 0;
			break;
		}

		offset += (RomDirEntry->size + 0xF) & ~0xF;
		RomDirEntry++;
	}

	return result;
}

static int OpenRomDirFile(const struct ROMImgStat *ImageStat, const char *file, struct RomDirFileFd *fd)
{
	const struct RomDirEntry *RomDirEntry;
	int result;

	memset(fd, 0, sizeof(struct RomDirFileFd));

	RomDirEntry = (const struct RomDirEntry *)ImageStat->ROMFS_start;
	result = ENOENT;
	if (GetRomDirExtInfoOffset(ImageStat, fd) == 0) {
		unsigned int offset = 0;
		while (RomDirEntry->name[0] != '\0') {
			if (strncmp(file, RomDirEntry->name, sizeof(RomDirEntry->name)) == 0) {
				fd->FileOffset = offset;
				fd->size = RomDirEntry->size;
				fd->ExtInfoEntrySize = RomDirEntry->ExtInfoEntrySize;
				result = 0;
				break;
			}

			fd->ExtInfoOffset += RomDirEntry->ExtInfoEntrySize;
			offset += (RomDirEntry->size + 0xF) & ~0xF;
			RomDirEntry++;
		}
	} else
		result = -1;

	return result;
}

static int GetExtInfoStat(const struct ROMImgStat *ImageStat, struct RomDirFileFd *fd, unsigned char type, void **buffer, unsigned int nbytes)
{
	int result;
	unsigned int offset, BytesToCopy;

	result = ENOENT;
	offset = 0;
	while (offset < fd->ExtInfoEntrySize) {
		struct ExtInfoFieldEntry *ExtInfoEntry = (struct ExtInfoFieldEntry *)((RMIMG_PTRCAST)ImageStat->image + fd->ExtInfoOffset);

		if (ExtInfoEntry->type == EXTINFO_FIELD_TYPE_DATE || ExtInfoEntry->type == EXTINFO_FIELD_TYPE_COMMENT) {
			if (type == ExtInfoEntry->type) {
				if (nbytes >= ExtInfoEntry->ExtLength) {
					BytesToCopy = ExtInfoEntry->ExtLength;
					result = 0;
				} else {
					if (*buffer != NULL) {
						BytesToCopy = nbytes;
					} else {
						*buffer = malloc(ExtInfoEntry->ExtLength);
						BytesToCopy = ExtInfoEntry->ExtLength;
					}
					result = ExtInfoEntry->ExtLength;
				}

				memcpy(*buffer, &((const unsigned char *)ImageStat->image)[fd->ExtInfoOffset + offset + sizeof(struct ExtInfoFieldEntry)], BytesToCopy);
				break;
			}

			offset += (sizeof(struct ExtInfoFieldEntry) + ExtInfoEntry->ExtLength);
		} else if (ExtInfoEntry->type == EXTINFO_FIELD_TYPE_VERSION) {
			if (type == ExtInfoEntry->type) {
				if (nbytes >= sizeof(ExtInfoEntry->value)) {
					result = 0;
					memcpy(buffer, &ExtInfoEntry->value, sizeof(ExtInfoEntry->value));
					break;
				} else {
					result = ENOMEM;
					break;
				}
			}

			offset += sizeof(struct ExtInfoFieldEntry);
		} else if (ExtInfoEntry->type == EXTINFO_FIELD_TYPE_NULL) {
			result = 0;
			break;
		} else {
			ERROR("Unknown EXTINFO entry type: 0x%02x @ ROM offset: 0x%x, EXTINFO offset: %u\n", ExtInfoEntry->type, fd->ExtInfoOffset + offset, offset);
			result = EIO;
			break;
		}
	}

	return result;
}

int CreateBlankROMImg(const char *filename, ROMIMG *ROMImg)
{
	unsigned int CommentLength;
	char LocalhostName[32] = "\0", cwd[MAX_PATH] = "\0", UserName[32] = "\0";
	struct FileEntry *ResetFile;
	struct ExtInfoFieldEntry *ExtInfoEntry;

	memset(ROMImg, 0, sizeof(ROMIMG));

	ROMImg->date = GetSystemDate();
#if defined(_WIN32) || defined(WIN32)
	GetUsername(UserName, sizeof(UserName));
#else
    getlogin_r(UserName, sizeof(UserName));
#endif
	GetLocalhostName(LocalhostName, sizeof(LocalhostName));
	GetCurrentWorkingDirectory(cwd, sizeof(cwd));
	/* Comment format: YYYYMMDD-XXXYYY,conffile,<filename>,<user>@<localhost>/<image path> */
	CommentLength = IMAGE_COMMENT_BASESIZE + strlen(filename) + sizeof(LocalhostName) + sizeof(UserName) + MAX_PATH;
	ROMImg->comment = (char *)calloc(0, CommentLength+1);
    if (!ROMImg->comment) return ENOMEM;
	snprintf(ROMImg->comment, CommentLength, "%08x,conffile,%s,%s@%s/%s", ROMImg->date, filename, BUFCHK(UserName), BUFCHK(LocalhostName), cwd);

	// Create a blank RESET file.
	ROMImg->NumFiles = 1;
	ResetFile = ROMImg->files = (struct FileEntry *)malloc(sizeof(struct FileEntry));

	memset(ResetFile->RomDir.name, 0, sizeof(ResetFile->RomDir.name));
	strcpy(ResetFile->RomDir.name, "RESET");
	ResetFile->RomDir.ExtInfoEntrySize = sizeof(ROMImg->date) + sizeof(struct ExtInfoFieldEntry);
	ResetFile->RomDir.size = 0;
	ResetFile->FileData = NULL;
	ResetFile->ExtInfoData = (unsigned char *)malloc(ResetFile->RomDir.ExtInfoEntrySize);
	ExtInfoEntry = (struct ExtInfoFieldEntry *)ResetFile->ExtInfoData;
	ExtInfoEntry->value = 0;
	ExtInfoEntry->ExtLength = sizeof(ROMImg->date);
	ExtInfoEntry->type = EXTINFO_FIELD_TYPE_DATE;
	memcpy((void *)((RMIMG_PTRCAST)ResetFile->ExtInfoData + sizeof(struct ExtInfoFieldEntry)), &ROMImg->date, ExtInfoEntry->ExtLength);

	return 0;
}

int WriteROMImg(const char *file, const ROMIMG *ROMImg)
{
	int result;
	unsigned char *extinfo;
	struct RomDirEntry ROMDIR_romdir, EXTINFO_romdir, NULL_romdir;
	unsigned int i, TotalExtInfoSize, ExtInfoOffset, CommentLengthRounded;

	result = 0;
	ExtInfoOffset = 0;
	CommentLengthRounded = (strlen(ROMImg->comment) + 1 + 3) & ~3;

	for (i = 0, TotalExtInfoSize = 0; i < ROMImg->NumFiles; i++) {
		TotalExtInfoSize += ROMImg->files[i].RomDir.ExtInfoEntrySize;
		if (ROMImg->files[i].RomDir.ExtInfoEntrySize % 4 != 0) {
			WARNING("ASSERT ROMImg->files[%u].RomDir.ExtInfoEntrySize%%4==0\n", i);
			abort();
		}
	}
	TotalExtInfoSize += CommentLengthRounded + 2 * sizeof(struct ExtInfoFieldEntry);
	if ((extinfo = (unsigned char *)malloc(TotalExtInfoSize)) != NULL) {
		memset(&NULL_romdir, 0, sizeof(NULL_romdir));
		memset(&ROMDIR_romdir, 0, sizeof(ROMDIR_romdir));
		memset(&EXTINFO_romdir, 0, sizeof(EXTINFO_romdir));

		// Copy the EXTINFO data for RESET over to the EXTINFO buffer.
		memcpy(&extinfo[ExtInfoOffset], ROMImg->files[0].ExtInfoData, ROMImg->files[0].RomDir.ExtInfoEntrySize);
		ExtInfoOffset += ROMImg->files[0].RomDir.ExtInfoEntrySize;

		// Generate the content for the ROMDIR and EXTINFO entries.
		strcpy(ROMDIR_romdir.name, "ROMDIR");
		ROMDIR_romdir.size = (ROMImg->NumFiles + 3) * sizeof(struct RomDirEntry);  // Number of files (Including RESET) + one ROMDIR and one EXTINFO entries... and one NULL entry.
		ROMDIR_romdir.ExtInfoEntrySize = sizeof(struct ExtInfoFieldEntry) + CommentLengthRounded;
		struct ExtInfoFieldEntry *ExtInfoEntry = (struct ExtInfoFieldEntry *)(extinfo + ExtInfoOffset);
		ExtInfoEntry->value = 0;
		ExtInfoEntry->ExtLength = CommentLengthRounded;
		ExtInfoEntry->type = EXTINFO_FIELD_TYPE_COMMENT;
		memcpy(&extinfo[ExtInfoOffset + sizeof(struct ExtInfoFieldEntry)], ROMImg->comment, ExtInfoEntry->ExtLength);
		ExtInfoOffset += sizeof(struct ExtInfoFieldEntry) + ExtInfoEntry->ExtLength;

		strcpy(EXTINFO_romdir.name, "EXTINFO");
		EXTINFO_romdir.size = TotalExtInfoSize;
		EXTINFO_romdir.ExtInfoEntrySize = 0;

		for (i = 1; i < ROMImg->NumFiles; i++) {
			if (ExtInfoOffset % 4 != 0) {
				WARNING("ASSERT: ExtInfoOffset[%u]%%4==0: %u\n", i, ExtInfoOffset);
				abort();
			}
			memcpy(&extinfo[ExtInfoOffset], ROMImg->files[i].ExtInfoData, ROMImg->files[i].RomDir.ExtInfoEntrySize);
			ExtInfoOffset += ROMImg->files[i].RomDir.ExtInfoEntrySize;
		}

		FILE *OutputFile;
		if ((OutputFile = fopen(file, "wb")) != NULL) {
			int FileAlignMargin;
			// Write the content of RESET (The bootstrap program, if it exists).
			fwrite(ROMImg->files[0].FileData, 1, ROMImg->files[0].RomDir.size, OutputFile);  // It will be aligned to 16 byte units in size.

			// Write out the ROMDIR entries.
			fwrite(&ROMImg->files[0].RomDir, sizeof(struct RomDirEntry), 1, OutputFile);
			fwrite(&ROMDIR_romdir, sizeof(struct RomDirEntry), 1, OutputFile);
			fwrite(&EXTINFO_romdir, sizeof(struct RomDirEntry), 1, OutputFile);
			for (i = 1; i < ROMImg->NumFiles; i++) {
				fwrite(&ROMImg->files[i].RomDir, sizeof(struct RomDirEntry), 1, OutputFile);
			}
			fwrite(&NULL_romdir, sizeof(struct RomDirEntry), 1, OutputFile);

			// Write out the EXTINFO section.
			fwrite(extinfo, 1, TotalExtInfoSize, OutputFile);
			if ((FileAlignMargin = (ftell(OutputFile) % 16)) > 0)
				fseek(OutputFile, 16 - FileAlignMargin, SEEK_CUR);

			// Write out the file data, excluding the content of RESET, which was done above.
			for (i = 1; i < ROMImg->NumFiles; i++) {
				fwrite(ROMImg->files[i].FileData, 1, ROMImg->files[i].RomDir.size, OutputFile);
				if ((FileAlignMargin = (ftell(OutputFile) % 16)) > 0)
					fseek(OutputFile, 16 - FileAlignMargin, SEEK_CUR);
			}

			fclose(OutputFile);
		}

	} else
		result = ENOMEM;

	free(extinfo);

	return result;
}

int LoadROMImg(ROMIMG *ROMImg, const char *path)
{
	FILE *InputFile;
	int result;
	unsigned int ExtInfoOffset, DataStartOffset, ScanLimit;
	struct RomDirFileFd RomDirFileFd;
	struct ROMImgStat ImageStat;
	struct FileEntry *file;
	void *ptr;
	memset(ROMImg, 0, sizeof(ROMIMG));

	if ((InputFile = fopen(path, "rb")) != NULL) {
		fseek(InputFile, 0, SEEK_END);
		ImageStat.size = ftell(InputFile);
		rewind(InputFile);

		if ((ImageStat.image = malloc(ImageStat.size)) != NULL) {
			if (fread(ImageStat.image, 1, ImageStat.size, InputFile) == ImageStat.size) {
				DataStartOffset = 0;

				// Scan for the start of the image, which is after the end of the bootstrap program. All regular IOPRP images don't have one (The image begins immediately at the start of the file).
				unsigned int i;
				for (i = 0, result = -EIO, ScanLimit = 0x40000 < ImageStat.size ? 0x40000 : ImageStat.size; i < ScanLimit; i++) {

					if (((const char *)ImageStat.image)[i] == 'R' &&
					    ((const char *)ImageStat.image)[i + 1] == 'E' &&
					    ((const char *)ImageStat.image)[i + 2] == 'S' &&
					    ((const char *)ImageStat.image)[i + 3] == 'E' &&
					    ((const char *)ImageStat.image)[i + 4] == 'T') {
						result = 0;
						DataStartOffset = i;
						break;
					}
				}
				if (result == 0) {
					ImageStat.ROMFS_start = (void *)((RMIMG_PTRCAST)ImageStat.image + DataStartOffset);
					if (OpenRomDirFile(&ImageStat, "RESET", &RomDirFileFd) != 0) {
						ERROR(" Unable to locate RESET!\n");
						free(ImageStat.image);
						fclose(InputFile);
						return -1;
					}
					if (RomDirFileFd.size != DataStartOffset) {
						ERROR(" Size of RESET does not match the start of ROMFS!\n");
						free(ImageStat.image);
						fclose(InputFile);
						return -1;
					}
					ptr = &ROMImg->date;
					GetExtInfoStat(&ImageStat, &RomDirFileFd, EXTINFO_FIELD_TYPE_DATE, &ptr, sizeof(ROMImg->date));

					if (OpenRomDirFile(&ImageStat, "ROMDIR", &RomDirFileFd) != 0) {
						ERROR(" Unable to locate ROMDIR!\n");
						free(ImageStat.image);
						fclose(InputFile);
						return -1;
					}

					ROMImg->comment = NULL;
					GetExtInfoStat(&ImageStat, &RomDirFileFd, EXTINFO_FIELD_TYPE_COMMENT, (void **)&ROMImg->comment, 0);

					struct RomDirEntry * RomDir = (struct RomDirEntry *)ImageStat.ROMFS_start;
					unsigned int offset = 0;
					ExtInfoOffset = 0;
					GetRomDirExtInfoOffset(&ImageStat, &RomDirFileFd);
					while (RomDir->name[0] != '\0') {
						if (strncmp(RomDir->name, "ROMDIR", sizeof(RomDir->name)) != 0 && strncmp(RomDir->name, "EXTINFO", sizeof(RomDir->name)) != 0) {
							ROMImg->NumFiles++;

							ROMImg->files = (ROMImg->files == NULL) ? (struct FileEntry *)malloc(sizeof(struct FileEntry)) : (struct FileEntry *)realloc(ROMImg->files, ROMImg->NumFiles * sizeof(struct FileEntry));
							file = &ROMImg->files[ROMImg->NumFiles - 1];

							memcpy(&file->RomDir, RomDir, sizeof(struct RomDirEntry));
							file->ExtInfoData = (unsigned char *)malloc(RomDir->ExtInfoEntrySize);
							memcpy(file->ExtInfoData, (void *)((RMIMG_PTRCAST)ImageStat.image + RomDirFileFd.ExtInfoOffset + ExtInfoOffset), RomDir->ExtInfoEntrySize);
							file->FileData = malloc(RomDir->size);
							memcpy(file->FileData, (void *)((RMIMG_PTRCAST)ImageStat.image + offset), RomDir->size);
						}

						offset += (RomDir->size + 0xF) & ~0xF;
						ExtInfoOffset += RomDir->ExtInfoEntrySize;
						RomDir++;
					}
				} else {
					WARNING("Could not locate RESET file, aborting...\n");
					result = EINVAL;
				}
			} else {
				ERROR("failed to read %u bytes\n", ImageStat.size);
				result = EIO;
			}

			free(ImageStat.image);
		} else {
			ERROR("failed to malloc %u bytes\n", ImageStat.size);
			result = ENOMEM;
		}

		fclose(InputFile);
	} else {
		ERROR("cant open '%s'\n", path);
		result = ENOENT;
	}

	return result;
}

void UnloadROMImg(ROMIMG *ROMImg)
{
	DPRINTF("start...\n");

	if (ROMImg->comment != NULL) {
		free(ROMImg->comment);
	}
	if (ROMImg->files != NULL) {
		unsigned int i;
		for (i = 0; i < ROMImg->NumFiles; i++) {
			if (ROMImg->files[i].ExtInfoData != NULL)
				free(ROMImg->files[i].ExtInfoData);
			if (ROMImg->files[i].FileData != NULL)
				free(ROMImg->files[i].FileData);
		}

		free(ROMImg->files);
	}

	memset(ROMImg, 0, sizeof(ROMIMG));
}

static void *ReallocExtInfoArea(struct FileEntry *file, unsigned short int nbytes)
{
	return (file->ExtInfoData = (file->ExtInfoData == NULL) ? (unsigned char *)malloc(nbytes + sizeof(struct ExtInfoFieldEntry)) : (unsigned char *)realloc(file->ExtInfoData, file->RomDir.ExtInfoEntrySize + nbytes + sizeof(struct ExtInfoFieldEntry)));
}

static int AddExtInfoStat(struct FileEntry *file, unsigned char type, void *data, unsigned char nbytes)
{
	int result;
	struct ExtInfoFieldEntry *ExtInfo;
	switch (type) {
		case EXTINFO_FIELD_TYPE_DATE:
		case EXTINFO_FIELD_TYPE_COMMENT:
			if (type != EXTINFO_FIELD_TYPE_DATE || (type == EXTINFO_FIELD_TYPE_DATE && nbytes == 4)) {
				if (ReallocExtInfoArea(file, nbytes) != NULL) {
					ExtInfo = (struct ExtInfoFieldEntry *)(file->ExtInfoData + file->RomDir.ExtInfoEntrySize);
					ExtInfo->value = 0;
					ExtInfo->ExtLength = (nbytes + 0x3) & ~0x3;
					ExtInfo->type = type;
					memcpy(file->ExtInfoData + file->RomDir.ExtInfoEntrySize + sizeof(struct ExtInfoFieldEntry), data, nbytes);

					file->RomDir.ExtInfoEntrySize += ExtInfo->ExtLength + sizeof(struct ExtInfoFieldEntry);

					result = 0;
				} else
					result = ENOMEM;
			} else
				result = EINVAL;
			break;
		case EXTINFO_FIELD_TYPE_VERSION:
			if (nbytes == 2) {
				if (ReallocExtInfoArea(file, 0) != NULL) {
					ExtInfo = (struct ExtInfoFieldEntry *)(file->ExtInfoData + file->RomDir.ExtInfoEntrySize);
					ExtInfo->value = *(unsigned short int *)data;
					ExtInfo->ExtLength = 0;
					ExtInfo->type = type;

					file->RomDir.ExtInfoEntrySize += sizeof(struct ExtInfoFieldEntry);

					result = 0;
				} else
					result = ENOMEM;
			} else
				result = EINVAL;
			break;
		default:
			result = EINVAL;
	}

	return result;
}

int AddFile(ROMIMG *ROMImg, const char *path)
{
	FILE *InputFile;
	int result;
	unsigned int FileDateStamp;
	unsigned short FileVersion;
	if ((InputFile = fopen(path, "rb")) != NULL) {
		const char* fname = strrchr(path, PATHSEP);
		if (fname == NULL) fname = path; else fname++;
		int size;
		fseek(InputFile, 0, SEEK_END);
		size = ftell(InputFile);
		rewind(InputFile);

		struct FileEntry *file;
		// Files cannot exist more than once in an image. The RESET entry is special here because all images will have a RESET entry, but it might be empty (If it has no content, allow the user to add in content).
		if (strcmp(fname, "RESET")) {
			if (!IsFileExists(ROMImg, fname)) {
				ROMImg->NumFiles++;

				if ((ROMImg->files = (ROMImg->files == NULL) ? (struct FileEntry *)malloc(sizeof(struct FileEntry)) : (struct FileEntry *)realloc(ROMImg->files, ROMImg->NumFiles * sizeof(struct FileEntry))) != NULL) {
					file = &ROMImg->files[ROMImg->NumFiles - 1];
					memset(&ROMImg->files[ROMImg->NumFiles - 1], 0, sizeof(struct FileEntry));

					strncpy(file->RomDir.name, fname, sizeof(file->RomDir.name) - 1);
                    file->RomDir.name[sizeof(file->RomDir.name) - 1] = '\0';
					file->RomDir.ExtInfoEntrySize = 0;

					FileDateStamp = GetFileCreationDate(path);
					AddExtInfoStat(&ROMImg->files[ROMImg->NumFiles - 1], EXTINFO_FIELD_TYPE_DATE, &FileDateStamp, 4);

					if (IsSonyRXModule(path)) {
						char ModuleDescription[32];
						if ((result = GetSonyRXModInfo(path, ModuleDescription, sizeof(ModuleDescription), &FileVersion)) == 0) {
							AddExtInfoStat(&ROMImg->files[ROMImg->NumFiles - 1], EXTINFO_FIELD_TYPE_VERSION, &FileVersion, 2);
							AddExtInfoStat(&ROMImg->files[ROMImg->NumFiles - 1], EXTINFO_FIELD_TYPE_COMMENT, ModuleDescription, strlen(ModuleDescription) + 1);
						}
					} else
						result = 0;

					if (result == 0) {
						file->RomDir.size = size;
						file->FileData = malloc(size);
						if (fread(file->FileData, 1, size, InputFile) != size) {
							ERROR("failed to read %d bytes\n", size);
							result = EIO;
						}
					}
				} else
					result = ENOMEM;
			} else
				result = EEXIST;
		} else {
			if (ROMImg->files[0].RomDir.size < 1) {
				file = &ROMImg->files[0];
				file->RomDir.size = size;
				file->FileData = malloc(size);
				if (fread(file->FileData, 1, size, InputFile) != file->RomDir.size) {
					ERROR("failed to read %u bytes\n", file->RomDir.size);
					result = EIO;
				} else
					result = 0;
			} else
				result = EEXIST;
		}

		fclose(InputFile);
	} else
		result = ENOENT;

	return result;
}

int DeleteFile(ROMIMG *ROMImg, const char *filename)
{
	int result;
	struct FileEntry *file;

	// The RESET entry cannot be deleted, but its content will be.
	if (strcmp("RESET", filename)) {
		unsigned int i;
		for (result = ENOENT, i = 0, file = ROMImg->files; i < ROMImg->NumFiles; i++, file++) {
			if (strncmp(file->RomDir.name, filename, sizeof(file->RomDir.name)) == 0) {
				if (file->FileData != NULL)
					free(file->FileData);

				if (file->ExtInfoData != NULL)
					free(file->ExtInfoData);
				for (; i < ROMImg->NumFiles; i++)
					memcpy(&ROMImg->files[i], &ROMImg->files[i + 1], sizeof(struct FileEntry));
				ROMImg->files = (struct FileEntry *)realloc(ROMImg->files, (--ROMImg->NumFiles) * sizeof(struct FileEntry));
				result = 0;
				break;
			}
		}
	} else {
		file = &ROMImg->files[0];
		if (file->RomDir.size > 0) {
			file->FileData = NULL;
			file->RomDir.size = 0;
			result = 0;
		} else
			result = ENOENT;
	}

	return result;
}

int ExtractFile(const ROMIMG *ROMImg, const char *filename, const char *FileToExtract)
{
	int result;
	unsigned int i;
	FILE *OutputFile;
	const struct FileEntry *file;

	for (result = ENOENT, i = 0, file = ROMImg->files; i < ROMImg->NumFiles; i++, file++) {
		if (strncmp(file->RomDir.name, FileToExtract, sizeof(file->RomDir.name)) == 0) {
			if ((OutputFile = fopen(filename, "wb")) != NULL) {
				result = (fwrite(file->FileData, 1, file->RomDir.size, OutputFile) == file->RomDir.size) ? 0 : EIO;
				fclose(OutputFile);
			} else
				result = EIO;
			break;
		}
	}

	return result;
}

int IsFileExists(const ROMIMG *ROMImg, const char *filename)
{
	int result;
	unsigned int i;
	const struct FileEntry *file;

	for (result = 0, i = 0, file = ROMImg->files; i < ROMImg->NumFiles; i++, file++) {
		if (strncmp(file->RomDir.name, filename, sizeof(file->RomDir.name)) == 0) {
			result = 1;
			break;
		}
	}

	return result;
}
