/*
    ROMFS format:
        ROMDIR section
        EXTINFO section
        File data section
    All files will have an entry in all three sections. The ROMDIR section is terminated by an entry that consists of zeros.

    Required file entries (In this order):
        RESET	(0-byte)
        ROMDIR	(The size of the whole ROMDIR section)
        EXTINFO	(The size of the whole EXTINFO section)

    The EXTINFO section (Extended Information section) contains more information on the file (E.g. date and version numbers) and comments on the file.

    The EXTINFO section is also a file! (In fact, all sections are files)

    All sections and files are (must be?) aligned to 16-byte boundaries, and all records within each section must be aligned to 4-byte boundaries.
*/

#ifndef __ROMING_H__
#define __ROMING_H__

#include "dprintf.h"
#if defined(_WIN32) || defined(WIN32)
#define RMIMG_PTRCAST unsigned int
#else
#define RMIMG_PTRCAST unsigned char *
#endif
struct RomDirEntry
{
	char name[10];
	unsigned short int ExtInfoEntrySize;
	unsigned int size;
};

/* Each ROMDIR entry can have any combination of EXTINFO fields. */
struct ExtInfoFieldEntry
{
	unsigned short int value; /* Only applicable for the version field type. */
	unsigned char ExtLength;  /* The length of data appended to the end of this entry. */
	unsigned char type;
};

enum ExtInfoFieldTypes {
	EXTINFO_FIELD_TYPE_DATE = 1,
	EXTINFO_FIELD_TYPE_VERSION,
	EXTINFO_FIELD_TYPE_COMMENT,
	EXTINFO_FIELD_TYPE_NULL = 0x7F
};

struct FileEntry
{
	struct RomDirEntry RomDir;
	unsigned char *ExtInfoData;
	void *FileData;
};

typedef struct ImageStructure
{
	unsigned int NumFiles;
	unsigned int date;
	char *comment;
	struct FileEntry *files;  // Stores the records of all files, including RESET. Only those entries that are automatically generated like ROMDIR and EXTINFO will be omitted.
} ROMIMG;

/* Function prototypes */
extern int CreateBlankROMImg(const char *filename, ROMIMG *ROMImg);
extern int WriteROMImg(const char *file, const ROMIMG *ROMImg);
extern int LoadROMImg(ROMIMG *ROMImg, const char *path);
extern void UnloadROMImg(ROMIMG *ROMImg);
extern int AddFile(ROMIMG *ROMImg, const char *path, int upperconv);
extern int DeleteFile(ROMIMG *ROMImg, const char *filename);
extern int ExtractFile(const ROMIMG *ROMImg, const char *filename, const char *FileToExtract);
extern int IsFileExists(const ROMIMG *ROMImg, const char *filename);

#endif /* __ROMING_H__ */
