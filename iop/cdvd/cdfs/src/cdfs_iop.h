#ifndef _CDFS_IOP_H
#define _CDFS_IOP_H

#ifdef DEBUG
#define DPRINTF(args...)	printf(args)
#else
#define DPRINTF(args...)	do { } while(0)
#endif

#define CDFS_FILEPROPERTY_DIR 0x02

struct TocEntry {
    u32 fileLBA;
    u32 fileSize;
    u8 fileProperties;
    unsigned char dateStamp[8];
    char filename[128 + 1];
} __attribute__((packed));

enum CDFS_getMode {
    CDFS_GET_FILES_ONLY = 1,
    CDFS_GET_DIRS_ONLY = 2,
    CDFS_GET_FILES_AND_DIRS = 3
};

enum Cdvd_Changed_Index {
    CHANGED_TOC = 0,
    CHANGED_FIO = 1,
    CHANGED_MAX
};

int cdfs_prepare(void);
int cdfs_start(void);
int cdfs_finish(void);
int cdfs_findfile(const char *fname, struct TocEntry *tocEntry);
int cdfs_readSect(u32 lsn, u32 sectors, u8 *buf);
int cdfs_getDir(const char *pathname, const char *extensions, enum CDFS_getMode getMode, struct TocEntry tocEntry[], unsigned int req_entries);
int cdfs_checkDiskChanged(enum Cdvd_Changed_Index index);

#endif  // _CDFS_H
