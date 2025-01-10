#ifndef _CDFS_IOP_H
#define _CDFS_IOP_H

#define MODNAME "cdfs_driver"
#ifdef DEBUG
#define DPRINTF(args...)	printf(MODNAME ": "args)
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
    u8 padding[2]; // Padding to make the structure 4 byte aligned
} __attribute__((packed));

enum Cdvd_Changed_Index {
    CHANGED_TOC = 0,
    CHANGED_FIO = 1,
    CHANGED_MAX
};

extern int cdfs_prepare(void);
extern int cdfs_start(void);
extern int cdfs_finish(void);
extern int cdfs_findfile(const char *fname, struct TocEntry *tocEntry);
extern int cdfs_readSect(u32 lsn, u32 sectors, u8 *buf);
extern int cdfs_getDir(const char *pathname, struct TocEntry tocEntry[], unsigned int req_entries);
extern int cdfs_checkDiskChanged(enum Cdvd_Changed_Index index);

#endif  // _CDFS_H
