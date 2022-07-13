#include <stdio.h>
#include <libcdvd-common.h>
#include <sysclib.h>
#include <sysmem.h>

#include "cdfs_iop.h"

#define TRUE 1
#define FALSE 0

#define MAX_DIR_CACHE_SECTORS 32

struct DirTocEntry
{
    short length;
    unsigned int fileLBA;
    unsigned int fileLBA_bigend;
    unsigned int fileSize;
    unsigned int fileSize_bigend;
    unsigned char dateStamp[6];
    unsigned char reserved1;
    unsigned char fileProperties;
    unsigned char reserved2[6];
    unsigned char filenameLength;
    char filename[128];
} __attribute__((packed));  // This is the internal format on the CD
// a file with a single character filename will have a 34byte toc entry
// (max 60 entries per sector)6

// TocEntry structure contains only the important stuff needed for export
//

struct CacheInfoDir
{
    char pathname[1024];  // The pathname of the cached directory
    unsigned int valid;   // TRUE if cache data is valid, FALSE if not

    unsigned int path_depth;  // The path depth of the cached directory (0 = root)

    unsigned int sector_start;  // The start sector (LBA) of the cached directory
    unsigned int sector_num;    // The total size of the directory (in sectors)
    unsigned int cache_offset;  // The offset from sector_start of the cached area
    unsigned int cache_size;    // The size of the cached directory area (in sectors)

    u8 *cache;  // The actual cached data
};

struct RootDirTocHeader
{
    u16 length;
    u32 tocLBA;
    u32 tocLBA_bigend;
    u32 tocSize;
    u32 tocSize_bigend;
    u8 dateStamp[8];
    u8 reserved[6];
    u8 reserved2;
    u8 reserved3;
} __attribute__((packed));

struct ASCIIDate
{
    char year[4];
    char month[2];
    char day[2];
    char hours[2];
    char minutes[2];
    char seconds[2];
    char hundreths[2];
    char terminator[1];
} __attribute__((packed));

struct CDVolDesc
{
    u8 filesystemType;  // 0x01 = ISO9660, 0x02 = Joliet, 0xFF = NULL
    u8 volID[5];        // "CD001"
    u8 reserved2;
    u8 reserved3;
    u8 sysIdName[32];
    u8 volName[32];  // The ISO9660 Volume Name
    u8 reserved5[8];
    u32 volSize;     // Volume Size
    u32 volSizeBig;  // Volume Size Big-Endian
    u8 reserved6[32];
    u32 unknown1;
    u32 unknown1_bigend;
    u16 volDescSize;
    u16 volDescSize_bigend;
    u32 unknown3;
    u32 unknown3_bigend;
    u32 priDirTableLBA;  // LBA of Primary Dir Table
    u32 reserved7;
    u32 secDirTableLBA;  // LBA of Secondary Dir Table
    u32 reserved8;
    struct RootDirTocHeader rootToc;
    u8 volSetName[128];
    u8 publisherName[128];
    u8 preparerName[128];
    u8 applicationName[128];
    u8 copyrightFileName[37];
    u8 abstractFileName[37];
    u8 bibliographyFileName[37];
    struct ASCIIDate creationDate;
    struct ASCIIDate modificationDate;
    struct ASCIIDate effectiveDate;
    struct ASCIIDate expirationDate;
    u8 reserved10;
    u8 reserved11[1166];
} __attribute__((packed));

enum Cache_getMode {
    CACHE_START = 0,
    CACHE_NEXT = 1
};

enum PathMatch {
    NOT_MATCH = 0,
    MATCH,
    SUBDIR
};

static struct CacheInfoDir cacheInfoDir;
static struct CDVolDesc cdVolDesc;
static sceCdRMode cdReadMode;
static u8 dvdvBuffer[2064];

static int cdvdChangedMagic = 0;
static int cdvdChangedMagicLast[CHANGED_MAX] = {-1};

/***********************************************
*                                              *
*            PRIVATE FUNCTIONS                 *
*                                              *
***********************************************/

// Used in findfile
static int strcasecmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && tolower(*s1) == tolower(*s2)) {
        s1++;
        s2++;
    }

    return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}

/***********************************************
* Determines if there is a valid disc inserted *
***********************************************/
static int isValidDisc(void) {
    int result;

    switch (sceCdGetDiskType()) {
        case SCECdPSCD:
        case SCECdPSCDDA:
        case SCECdPS2CD:
        case SCECdPS2CDDA:
        case SCECdPS2DVD:
        case SCECdDVDV:
            result = 1;
            break;
        default:
            result = 0;
    }

    return result;
}

// Copy a TOC Entry from the CD native format to our tidier format
static void copyToTocEntry(struct TocEntry *tocEntry, struct DirTocEntry *internalTocEntry) {
    int filenamelen;

    DPRINTF("copyToTocEntry: from=%p, to=%p\n\n", tocEntry, internalTocEntry);
    DPRINTF("copyToTocEntry: DirTocEntry=%p\n", internalTocEntry);
    DPRINTF("      length......... %hd\n", internalTocEntry->length);
    DPRINTF("      fileLBA........ %u\n", internalTocEntry->fileLBA);
    DPRINTF("      fileLBA_bigend. %u\n", internalTocEntry->fileLBA_bigend);
    DPRINTF("      fileSize....... %u\n", internalTocEntry->fileSize);
    DPRINTF("      fileSize_bigend %u\n", internalTocEntry->fileSize_bigend);
    DPRINTF("      dateStamp...... %s\n", internalTocEntry->dateStamp);
    DPRINTF("      reserved1...... %u\n", internalTocEntry->reserved1);
    DPRINTF("      fileProperties. %u\n", internalTocEntry->fileProperties);
    DPRINTF("      reserved2...... %s\n", internalTocEntry->reserved2);
    DPRINTF("      filenameLength. %u\n", internalTocEntry->filenameLength);
    DPRINTF("      filename....... %s\n", internalTocEntry->filename);

    tocEntry->fileSize = internalTocEntry->fileSize;
    tocEntry->fileLBA = internalTocEntry->fileLBA;
    tocEntry->fileProperties = internalTocEntry->fileProperties;

    if (cdVolDesc.filesystemType == 2) {
        int i;

        // This is a Joliet Filesystem, so use Unicode to ISO string copy
        filenamelen = internalTocEntry->filenameLength / 2;

        for (i = 0; i < filenamelen; i++)
            tocEntry->filename[i] = internalTocEntry->filename[(i << 1) + 1];
    } else {
        filenamelen = internalTocEntry->filenameLength;

        // use normal string copy
        strncpy(tocEntry->filename, internalTocEntry->filename, 128);
    }

    tocEntry->filename[filenamelen] = 0;

    if (!(tocEntry->fileProperties & 0x02)) {
        // strip the ;1 from the filename (if it's there)
        strtok(tocEntry->filename, ";");
    }

    DPRINTF("copyToTocEntry: tocEntry=%p\n\n", tocEntry);
    DPRINTF("      fileLBA........ %u\n\n", tocEntry->fileLBA);
    DPRINTF("      fileSize....... %u\n\n", tocEntry->fileSize);
    DPRINTF("      filename....... %s\n\n", tocEntry->filename);
    DPRINTF("      fileProperties. %u\n\n", tocEntry->fileProperties);
}


// findPath should use the current directory cache to start it's search (in this case the root)
// and should change cacheInfoDir.pathname, to the path of the dir it finds
// it should also cache the first chunk of directory sectors,
// and fill the contents of the other elements of cacheInfoDir appropriately

static int findPath(char *pathname) {
    char *dirname;
    char *seperator;


    struct TocEntry localTocEntry;

    dirname = strtok(pathname, "\\/");
    DPRINTF("findPath: trying to find directory %s\n\n", pathname);

    if (!isValidDisc())
        return FALSE;

    sceCdDiskReady(0);

    while (dirname != NULL) {
        int dir_entry;
        int found_dir;

        struct DirTocEntry *tocEntryPointer;

        found_dir = FALSE;

        tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;

        // Always skip the first entry (self-refencing entry)
        tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length);

        dir_entry = 0;

        for (; tocEntryPointer < (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048)); tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length)) {
            // If we have a null toc entry, then we've either reached the end of the dir, or have reached a sector boundary
            if (tocEntryPointer->length == 0) {
                DPRINTF("Got a null pointer entry, so either reached end of dir, or end of sector\n\n");
                tocEntryPointer = (struct DirTocEntry *)(cacheInfoDir.cache + (((((u8 *)tocEntryPointer - cacheInfoDir.cache) / 2048) + 1) * 2048));
            }

            if (tocEntryPointer >= (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048))) {
                // If we've gone past the end of the cache
                // then check if there are more sectors to load into the cache

                if ((cacheInfoDir.cache_offset + cacheInfoDir.cache_size) < cacheInfoDir.sector_num) {
                    // If there are more sectors to load, then load them
                    cacheInfoDir.cache_offset += cacheInfoDir.cache_size;
                    cacheInfoDir.cache_size = cacheInfoDir.sector_num - cacheInfoDir.cache_offset;

                    if (cacheInfoDir.cache_size > MAX_DIR_CACHE_SECTORS)
                        cacheInfoDir.cache_size = MAX_DIR_CACHE_SECTORS;

                    if (!cdfs_readSect(cacheInfoDir.sector_start + cacheInfoDir.cache_offset, cacheInfoDir.cache_size, cacheInfoDir.cache)) {
                        DPRINTF("Couldn't Read from CD !\n\n");
                        cacheInfoDir.valid = FALSE;  // should we completely invalidate just because we couldnt read time?
                        return FALSE;
                    }

                    tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;
                } else {
                    cacheInfoDir.valid = FALSE;
                    return FALSE;
                }
            }

            // If the toc Entry is a directory ...
            if (tocEntryPointer->fileProperties & 0x02) {
                // Convert to our format (inc ascii name), for the check
                copyToTocEntry(&localTocEntry, tocEntryPointer);

                // If it's the link to the parent directory, then give it the name ".."
                if (dir_entry == 0) {
                    if (cacheInfoDir.path_depth != 0) {
                        DPRINTF("First directory entry in dir, so name it '..'\n\n");
                        strcpy(localTocEntry.filename, "..");
                    }
                }

                // Check if this is the directory that we are looking for
                if (strcasecmp(dirname, localTocEntry.filename) == 0) {
                    DPRINTF("Found the matching sub-directory\n\n");
                    found_dir = TRUE;

                    if (dir_entry == 0) {
                        // We've matched with the parent directory
                        // so truncate the pathname by one level

                        if (cacheInfoDir.path_depth > 0)
                            cacheInfoDir.path_depth--;

                        if (cacheInfoDir.path_depth == 0) {
                            // If at root then just clear the path to root
                            // (simpler than finding the colon seperator etc)
                            cacheInfoDir.pathname[0] = 0;
                        } else {
                            seperator = strrchr(cacheInfoDir.pathname, '/');

                            if (seperator != NULL)
                                *seperator = 0;
                        }
                    } else {
                        // otherwise append a seperator, and the matched directory
                        // to the pathname
                        strcat(cacheInfoDir.pathname, "/");
                        DPRINTF("Adding '%s' to cached pathname - path depth = %u\n\n", dirname, cacheInfoDir.path_depth);
                        strcat(cacheInfoDir.pathname, dirname);
                        cacheInfoDir.path_depth++;
                    }

                    // Exit out of the search loop
                    // (and find the next sub-directory, if there is one)
                    break;
                } else {
                    DPRINTF("Found a directory, but it doesn't match\n\n");
                }
            }

            dir_entry++;

        }  // end of cache block search loop


        // if we've reached here, without finding the directory, then it's not there
        if (!found_dir) {
            cacheInfoDir.valid = FALSE;
            return FALSE;
        }

        // find name of next dir
        dirname = strtok(NULL, "\\/");

        cacheInfoDir.sector_start = localTocEntry.fileLBA;
        cacheInfoDir.sector_num = (localTocEntry.fileSize >> 11) + ((cdVolDesc.rootToc.tocSize & 2047) != 0);

        // Cache the start of the found directory
        // (used in searching if this isn't the last dir,
        // or used by whatever requested the cache in the first place if it is the last dir)
        cacheInfoDir.cache_offset = 0;
        cacheInfoDir.cache_size = cacheInfoDir.sector_num;

        if (cacheInfoDir.cache_size > MAX_DIR_CACHE_SECTORS)
            cacheInfoDir.cache_size = MAX_DIR_CACHE_SECTORS;

        if (!cdfs_readSect(cacheInfoDir.sector_start + cacheInfoDir.cache_offset, cacheInfoDir.cache_size, cacheInfoDir.cache)) {
            DPRINTF("Couldn't Read from CD, trying to read %u sectors, starting at sector %u !\n\n",
                   cacheInfoDir.cache_size, cacheInfoDir.sector_start + cacheInfoDir.cache_offset);
            cacheInfoDir.valid = FALSE;  // should we completely invalidate just because we couldnt read time?
            return FALSE;
        }
    }

// If we've got here then we found the requested directory
    DPRINTF("findPath found the path\n\n");
    cacheInfoDir.valid = TRUE;
    return TRUE;
}

static int cdfs_getVolumeDescriptor(void) {
    // Read until we find the last valid Volume Descriptor
    int volDescSector;
    static struct CDVolDesc localVolDesc;
    DPRINTF("cdfs_getVolumeDescriptor called\n\n");

    for (volDescSector = 16; volDescSector < 20; volDescSector++) {
        cdfs_readSect(volDescSector, 1, (u8*)&localVolDesc);

        // If this is still a volume Descriptor
        if (memcmp(localVolDesc.volID, "CD001", 5) == 0) {
            if ((localVolDesc.filesystemType == 1) ||
                (localVolDesc.filesystemType == 2)) {
                memcpy(&cdVolDesc, &localVolDesc, sizeof(struct CDVolDesc));
            }
        } else
            break;
    }

#ifdef DEBUG
    switch (cdVolDesc.filesystemType) {
        case 1:
            DPRINTF("CD FileSystem is ISO9660\n\n");
            break;
        case 2:
            DPRINTF("CD FileSystem is Joliet\n\n");
            break;
        default:
            DPRINTF("CD FileSystem is unknown type\n\n");
            break;
    }
#endif
    //	sceCdStop();
    return TRUE;
}

static enum PathMatch comparePath(const char *path) {
    int length;
    int i;

    length = strlen(cacheInfoDir.pathname);

    for (i = 0; i < length; i++) {
        // check if character matches
        if (path[i] != cacheInfoDir.pathname[i]) {
            // if not, then is it just because of different path seperator ?
            if ((path[i] == '/') || (path[i] == '\\')) {
                if ((cacheInfoDir.pathname[i] == '/') || (cacheInfoDir.pathname[i] == '\\')) {
                    continue;
                }
            }

            // if the characters don't match for any other reason then report a failure
            return NOT_MATCH;
        }
    }

    // Reached the end of the Cached pathname
    // if requested path is same length, then report exact match
    if (path[length] == 0)
        return MATCH;

    // if requested path is longer, and next char is a dir seperator
    // then report sub-dir match
    if ((path[length] == '/') || (path[length] == '\\'))
        return SUBDIR;
    else
        return NOT_MATCH;
}

// Check if a TOC Entry matches our extension list
static int compareTocEntry(char *filename, const char *extensions) {
    static char ext_list[129];

    char *token;


    strncpy(ext_list, extensions, 128);
    ext_list[128] = 0;

    token = strtok(ext_list, " ,");
    while (token != NULL) {
        char *ext_point;

        // if 'token' matches extension of 'filename'
        // then return a match
        ext_point = strrchr(filename, '.');

        if (strcasecmp(ext_point, token) == 0)
            return TRUE;

        /* Get next token: */
        token = strtok(NULL, " ,");
    }

    // If not match found then return FALSE
    return FALSE;
}

// Find, and cache, the requested directory, for use by GetDir or  (and thus open)
// provide an optional offset variable, for use when caching dirs of greater than 500 files

// returns TRUE if all TOC entries have been retrieved, or
// returns FALSE if there are more TOC entries to be retrieved
static int cdfs_cacheDir(const char *pathname, enum Cache_getMode getMode) {
    // macke sure that the requested pathname is not directly modified
    static char dirname[1024];

    DPRINTF("Attempting to find, and cache, directory: %s\n", pathname);

    // Invalidate table of contents cache if disk changed
    if (cdfs_checkDiskChanged(CHANGED_TOC)) {
        cacheInfoDir.valid = FALSE;
    }

    // only take any notice of the existing cache, if it's valid
    if (cacheInfoDir.valid) {
        // Check if the requested path is already cached
        //		if (strcasecmp(pathname,cacheInfoDir.pathname)==0)
        if (comparePath(pathname) == MATCH) {
            DPRINTF("CacheDir: The requested path is already cached\n");
            // If so, is the request ot cache the start of the directory, or to resume the next block ?
            if (getMode == CACHE_START) {
                DPRINTF("          and requested cache from start of dir\n");

                if (cacheInfoDir.cache_offset == 0) {
// requested cache of start of the directory, and thats what's already cached
// so sit back and do nothing
                    DPRINTF("          and start of dir is already cached so nothing to do :o)\n");

                    cacheInfoDir.valid = TRUE;
                    return TRUE;
                } else {
// Requested cache of start of the directory, but thats not what's cached
// so re-cache the start of the directory
                    DPRINTF("          but dir isn't cached from start, so re-cache existing dir from start\n");

                    // reset cache data to start of existing directory
                    cacheInfoDir.cache_offset = 0;
                    cacheInfoDir.cache_size = cacheInfoDir.sector_num;

                    if (cacheInfoDir.cache_size > MAX_DIR_CACHE_SECTORS)
                        cacheInfoDir.cache_size = MAX_DIR_CACHE_SECTORS;

                    // Now fill the cache with the specified sectors
                    if (!cdfs_readSect(cacheInfoDir.sector_start + cacheInfoDir.cache_offset, cacheInfoDir.cache_size, cacheInfoDir.cache)) {
                        DPRINTF("Couldn't Read from CD !\n");

                        cacheInfoDir.valid = FALSE;  // should we completely invalidate just because we couldnt read first time?
                        return FALSE;
                    }

                    cacheInfoDir.valid = TRUE;
                    return TRUE;
                }
            } else  { 
                // getMode == CACHE_NEXT 
                // So get the next block of the existing directory
                cacheInfoDir.cache_offset += cacheInfoDir.cache_size;
                cacheInfoDir.cache_size = cacheInfoDir.sector_num - cacheInfoDir.cache_offset;

                if (cacheInfoDir.cache_size > MAX_DIR_CACHE_SECTORS)
                    cacheInfoDir.cache_size = MAX_DIR_CACHE_SECTORS;

                // Now fill the cache with the specified sectors
                if (!cdfs_readSect(cacheInfoDir.sector_start + cacheInfoDir.cache_offset, cacheInfoDir.cache_size, cacheInfoDir.cache)) {
                    DPRINTF("Couldn't Read from CD !\n");

                    cacheInfoDir.valid = FALSE;  // should we completely invalidate just because we couldnt read first time?
                    return FALSE;
                }

                cacheInfoDir.valid = TRUE;
                return TRUE;
            }
        } else  {
            // requested directory is not the cached directory (but cache is still valid)
            DPRINTF("Cache is valid, but cached directory, is not the requested one\n"
                   "so check if the requested directory is a sub-dir of the cached one\n");
            DPRINTF("Requested Path = %s , Cached Path = %s\n", pathname, cacheInfoDir.pathname);

            if (comparePath(pathname) == SUBDIR) {
                int path_len;

// If so then we can start our search for the path, from the currently cached directory
                DPRINTF("Requested dir is a sub-dir of the cached directory,\n"
                       "so start search from current cached dir\n");
                // if the cached chunk, is not the start of the dir,
                // then we will need to re-load it before starting search
                if (cacheInfoDir.cache_offset != 0) {
                    cacheInfoDir.cache_offset = 0;
                    cacheInfoDir.cache_size = cacheInfoDir.sector_num;
                    if (cacheInfoDir.cache_size > MAX_DIR_CACHE_SECTORS)
                        cacheInfoDir.cache_size = MAX_DIR_CACHE_SECTORS;

                    // Now fill the cache with the specified sectors
                    if (!cdfs_readSect(cacheInfoDir.sector_start + cacheInfoDir.cache_offset, cacheInfoDir.cache_size, cacheInfoDir.cache)) {
                        DPRINTF("Couldn't Read from CD !\n");
                        cacheInfoDir.valid = FALSE;  // should we completely invalidate just because we couldnt read time?
                        return FALSE;
                    }
                }

                // start the search, with the path after the current directory
                path_len = strlen(cacheInfoDir.pathname);
                strcpy(dirname, pathname + path_len);

                // findPath should use the current directory cache to start it's search
                // and should change cacheInfoDir.pathname, to the path of the dir it finds
                // it should also cache the first chunk of directory sectors,
                // and fill the contents of the other elements of cacheInfoDir appropriately

                return (findPath(dirname));
            }
        }
    }

// If we've got here, then either the cache was not valid to start with
// or the requested path is not a subdirectory of the currently cached directory
// so lets start again
    DPRINTF("The cache is not valid, or the requested directory is not a sub-dir of the cached one\n\n");

    if (!isValidDisc()) {
        DPRINTF("No supported disc inserted.\n");
        return -1;
    }

    sceCdDiskReady(0);

    // Read the main volume descriptor
    if (!cdfs_getVolumeDescriptor()) {
        DPRINTF("Could not read the CD/DVD Volume Descriptor\n");
        return -1;
    }

    DPRINTF("Read the CD Volume Descriptor\n\n");

    cacheInfoDir.path_depth = 0;
    strcpy(cacheInfoDir.pathname, "");

    // Setup the lba and sector size, for retrieving the root toc
    cacheInfoDir.cache_offset = 0;
    cacheInfoDir.sector_start = cdVolDesc.rootToc.tocLBA;
    cacheInfoDir.sector_num = (cdVolDesc.rootToc.tocSize >> 11) + ((cdVolDesc.rootToc.tocSize & 2047) != 0);
    cacheInfoDir.cache_size = cacheInfoDir.sector_num;

    if (cacheInfoDir.cache_size > MAX_DIR_CACHE_SECTORS)
        cacheInfoDir.cache_size = MAX_DIR_CACHE_SECTORS;

    // Now fill the cache with the specified sectors
    if (!cdfs_readSect(cacheInfoDir.sector_start + cacheInfoDir.cache_offset, cacheInfoDir.cache_size, cacheInfoDir.cache)) {
        DPRINTF("Couldn't Read from CD !\n");
        cacheInfoDir.valid = FALSE;  // should we completely invalidate just because we couldnt read time?
        return FALSE;
    }

    DPRINTF("Read the first block from the root directory\n");
    DPRINTF("Calling findPath\n");
    strcpy(dirname, pathname);

    return (findPath(dirname));
}

static void splitPath(const char *constpath, char *dir, char *fname) {
    // 255 char max path-length is an ISO9660 restriction
    // we must change this for Joliet or relaxed iso restriction support
    static char pathcopy[1024 + 1];
    char *slash;

    strncpy(pathcopy, constpath, 1024);
    slash = strrchr(pathcopy, '/');

    // if the path doesn't contain a '/' then look for a '\'
    if (!slash)
        slash = strrchr(pathcopy, (int)'\\');

    // if a slash was found
    if (slash != NULL) {
        // null terminate the path
        slash[0] = 0;
        // and copy the path into 'dir'
        strncpy(dir, pathcopy, 1024);
        dir[255] = 0;

        // copy the filename into 'fname'
        strncpy(fname, slash + 1, 128);
        fname[128] = 0;
    } else {
        dir[0] = 0;

        strncpy(fname, pathcopy, 128);
        fname[128] = 0;
    }
}

/***********************************************
*                                              *
*              CDFS FUNCTIONS                  *
*                                              *
***********************************************/

int cdfs_prepare(void) {

    // Initialise the directory cache
    strcpy(cacheInfoDir.pathname, "");  // The pathname of the cached directory
    cacheInfoDir.valid = FALSE;         // Cache is not valid
    cacheInfoDir.path_depth = 0;        // 0 = root)
    cacheInfoDir.sector_start = 0;      // The start sector (LBA) of the cached directory
    cacheInfoDir.sector_num = 0;        // The total size of the directory (in sectors)
    cacheInfoDir.cache_offset = 0;      // The offset from sector_start of the cached area
    cacheInfoDir.cache_size = 0;        // The size of the cached directory area (in sectors)

    if (cacheInfoDir.cache == NULL)
        cacheInfoDir.cache = (u8 *)AllocSysMemory(0, MAX_DIR_CACHE_SECTORS * 2048, NULL);

     // setup the cdReadMode structure
    cdReadMode.trycount = 0;
    cdReadMode.spindlctrl = SCECdSpinStm;
    cdReadMode.datapattern = SCECdSecS2048;

    return 0;
}

int cdfs_start(void) {
    return sceCdInit(SCECdINoD);
}

int cdfs_finish(void) {
    if (cacheInfoDir.cache)
        FreeSysMemory(cacheInfoDir.cache);
    
    return 0;
}

int cdfs_findfile(const char *fname, struct TocEntry *tocEntry) {
    static char filename[128 + 1];
    static char pathname[1024 + 1];

    struct DirTocEntry *tocEntryPointer;

    DPRINTF("cdfs_findfile called\n\n");

    splitPath(fname, pathname, filename);
    DPRINTF("Trying to find file: %s in directory: %s\n", filename, pathname);

    // Invalidate table of contents cache if disk changed
    if (cdfs_checkDiskChanged(CHANGED_TOC)) {
        cacheInfoDir.valid = FALSE;
    }

    if ((cacheInfoDir.valid) && (comparePath(pathname) == MATCH)) {
        // the directory is already cached, so check through the currently
        // cached chunk of the directory first

        tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;

        for (; tocEntryPointer < (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048)); tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length)) {
            if (tocEntryPointer->length == 0) {
                DPRINTF("Got a null pointer entry, so either reached end of dir, or end of sector\n");
                tocEntryPointer = (struct DirTocEntry *)(cacheInfoDir.cache + (((((u8 *)tocEntryPointer - cacheInfoDir.cache) / 2048) + 1) * 2048));
            }

            if (tocEntryPointer >= (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048))) {
                // reached the end of the cache block
                break;
            }

            copyToTocEntry(tocEntry, tocEntryPointer);

            if (strcasecmp(tocEntry->filename, filename) == 0) {
               // and it matches !!
               return TRUE;
            }
        }  // end of for loop

        // If that was the only dir block, and we havent found it, then fail
        if (cacheInfoDir.cache_size == cacheInfoDir.sector_num)
            return FALSE;

        // Otherwise there is more dir to check
        if (cacheInfoDir.cache_offset == 0) {
            // If that was the first block then continue with the next block
            if (!cdfs_cacheDir(pathname, CACHE_NEXT))
                return FALSE;
        } else {
            // otherwise (if that wasnt the first block) then start checking from the start
            if (!cdfs_cacheDir(pathname, CACHE_START))
                return FALSE;
        }
    } else {
        DPRINTF("Trying to cache directory\n\n");
        // The wanted directory wasnt already cached, so cache it now
        if (!cdfs_cacheDir(pathname, CACHE_START)) {
            DPRINTF("Failed to cache directory\n\n");
            return FALSE;
        }
    }

    // If we've got here, then we have a block of the directory cached, and want to check
    // from this point, to the end of the dir
    DPRINTF("cache_size = %u\n", cacheInfoDir.cache_size);
    while (cacheInfoDir.cache_size > 0) {
        tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;

        if (cacheInfoDir.cache_offset == 0)
            tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length);

        for (; tocEntryPointer < (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048)); tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length)) {
            if (tocEntryPointer->length == 0) {
                DPRINTF("Got a null pointer entry, so either reached end of dir, or end of sector\n");
                DPRINTF("Offset into cache = %d bytes\n", (int)((u8 *)tocEntryPointer - cacheInfoDir.cache));
                tocEntryPointer = (struct DirTocEntry *)(cacheInfoDir.cache + (((((u8 *)tocEntryPointer - cacheInfoDir.cache) / 2048) + 1) * 2048));
            }

            if (tocEntryPointer >= (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048))) {
                // reached the end of the cache block
                break;
            }

            copyToTocEntry(tocEntry, tocEntryPointer);

            if (strcasecmp(tocEntry->filename, filename) == 0) {
                DPRINTF("Found a matching file\n\n");
                // and it matches !!
                return TRUE;
            }

            DPRINTF("Non-matching file - looking for %s , found %s\n", filename, tocEntry->filename);
        }  // end of for loop

        DPRINTF("Reached end of cache block\n");
        // cache the next block
        cdfs_cacheDir(pathname, CACHE_NEXT);
    }

    // we've run out of dir blocks to cache, and still not found it, so fail
    DPRINTF("cdfs_findfile: could not find file\n");
    return FALSE;
}

/********************
* Optimised CD Read *
********************/

int cdfs_readSect(u32 lsn, u32 sectors, u8 *buf) {
    u32 i;
    u32 consecutive_sectors = (sectors > 2) ? (sectors * 2048) / 2064 : 0;
    int retry;
    int result = 0;
    cdReadMode.trycount = 32;

    for (retry = 0; retry < 32; retry++) { // 32 retries
        if (retry <= 8)
            cdReadMode.spindlctrl = 1;  // Try fast reads for first 8 tries
        else
            cdReadMode.spindlctrl = 0;  // Then try slow reads

        if (!isValidDisc())
            return FALSE;

        sceCdDiskReady(0);

        if (sceCdGetDiskType() == SCECdDVDV)
        {
            if (consecutive_sectors > 0)
            {
                result = !sceCdReadDVDV(lsn, consecutive_sectors, buf, &cdReadMode);
                if (result == 0)
                {
                    sceCdSync(0);
                    result = sceCdGetError();
                }
                if (result != 0)
                {
                    continue;
                }
            }
            for (i = 0; i < consecutive_sectors; i += 1)
            {
                memmove(buf + (2048 * i), buf + (2064 * i) + 12, 2048);
            }
            for (i = consecutive_sectors; i < sectors; i += 1)
            {
                result = !sceCdReadDVDV(lsn + i, 1, dvdvBuffer, &cdReadMode);
                if (result == 0)
                {
                    sceCdSync(0);
                    result = sceCdGetError();
                }
                if (result != 0)
                {
                    break;
                }
                memcpy(buf + (2048 * i), dvdvBuffer + 12, 2048);
            }
        }
        else
        {
            result = !sceCdRead(lsn, sectors, buf, &cdReadMode);
            if (result == 0)
            {
                sceCdSync(0);
                result = sceCdGetError();
            }
        }

        if (result == 0)
            break;
    }

    cdReadMode.trycount = 32;
    cdReadMode.spindlctrl = 1;

    if (result == 0)
        return TRUE;

    memset(buf, 0, (sectors << 11));

    return FALSE;  // error
}

int cdfs_getDir(const char *pathname, const char *extensions, enum CDFS_getMode getMode, struct TocEntry tocEntry[], unsigned int req_entries) {
    int matched_entries;
    int dir_entry;

    struct TocEntry localTocEntry;
    struct DirTocEntry *tocEntryPointer;

    DPRINTF("cdfs_getDir Request\n\n");

    matched_entries = 0;

    // pre-cache the dir (and get the new pathname - in-case selected "..")
    if (!cdfs_cacheDir(pathname, CACHE_START)) {
        DPRINTF("cdfs_getDir - Call of cdfs_cacheDir failed\n\n");
        return -1;
    }

    DPRINTF("requested directory is %u sectors\n", cacheInfoDir.sector_num);

    if ((getMode == CDFS_GET_DIRS_ONLY) || (getMode == CDFS_GET_FILES_AND_DIRS)) {
        // Cache the start of the requested directory
        if (!cdfs_cacheDir(cacheInfoDir.pathname, CACHE_START)) {
            DPRINTF("cdfs_getDir - Call of cdfs_cacheDir failed\n\n");
            return -1;
        }

        tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;
        // skip the first self-referencing entry
        tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length);

        // skip the parent entry if this is the root
        if (cacheInfoDir.path_depth == 0)
            tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length);

        dir_entry = 0;

        while (1) {
            DPRINTF("cdfs_getDir - inside while-loop\n\n");

            // parse the current cache block
            for (; tocEntryPointer < (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048)); tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length)) {
                if (tocEntryPointer->length == 0) {
                    // if we have a toc entry length of zero,
                    // then we've either reached the end of the sector, or the end of the dir
                    // so point to next sector (if there is one - will be checked by next condition)

                    tocEntryPointer = (struct DirTocEntry *)(cacheInfoDir.cache + (((((u8 *)tocEntryPointer - cacheInfoDir.cache) / 2048) + 1) * 2048));
                }

                if (tocEntryPointer >= (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048))) {
                    // we've reached the end of the current cache block (which may be end of entire dir
                    // so just break the loop
                    break;
                }

                // Check if the current entry is a dir or a file
                if (tocEntryPointer->fileProperties & 0x02) {
                    DPRINTF("We found a dir, and we want all dirs\n\n");
                    copyToTocEntry(&localTocEntry, tocEntryPointer);

                    if (dir_entry == 0) {
                        if (cacheInfoDir.path_depth != 0) {
                            DPRINTF("It's the first directory entry, so name it '..'\n\n");
                            strcpy(localTocEntry.filename, "..");
                        }
                    }

                    // Copy from localTocEntry
                    tocEntry[matched_entries] = localTocEntry;
                    matched_entries++;
                } else { // it must be a file
                    DPRINTF("We found a file, but we dont want files (at least not yet)\n\n");
                }

                dir_entry++;

                if ((unsigned int)matched_entries >= req_entries)  // if we've filled the requested buffer
                    return (matched_entries);        // then just return

            }  // end of the current cache block

            // if there is more dir to load, then load next chunk, else finish
            if ((cacheInfoDir.cache_offset + cacheInfoDir.cache_size) < cacheInfoDir.sector_num) {
                if (!cdfs_cacheDir(cacheInfoDir.pathname, CACHE_NEXT)) {
                    // failed to cache next block (should return TRUE even if
                    // there is no more directory, as long as a CD read didnt fail
                    return -1;
                }
            } else
                break;

            tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;
        }
    }

    // Next do files
    if ((getMode == CDFS_GET_FILES_ONLY) || (getMode == CDFS_GET_FILES_AND_DIRS)) {
        // Cache the start of the requested directory
        if (!cdfs_cacheDir(cacheInfoDir.pathname, CACHE_START)) {
            DPRINTF("cdfs_getDir - Call of cdfs_cacheDir failed\n\n");
            return -1;
        }

        tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;

        // skip the first self-referencing entry
        tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length);

        // skip the parent entry if this is the root
        if (cacheInfoDir.path_depth == 0)
            tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length);

        dir_entry = 0;

        while (1) {
            DPRINTF("cdfs_getDir - inside while-loop\n\n");

            // parse the current cache block
            for (; tocEntryPointer < (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048)); tocEntryPointer = (struct DirTocEntry *)((u8 *)tocEntryPointer + tocEntryPointer->length)) {
                if (tocEntryPointer->length == 0) {
                    // if we have a toc entry length of zero,
                    // then we've either reached the end of the sector, or the end of the dir
                    // so point to next sector (if there is one - will be checked by next condition)

                    tocEntryPointer = (struct DirTocEntry *)(cacheInfoDir.cache + (((((u8 *)tocEntryPointer - cacheInfoDir.cache) / 2048) + 1) * 2048));
                }

                if (tocEntryPointer >= (struct DirTocEntry *)(cacheInfoDir.cache + (cacheInfoDir.cache_size * 2048))) {
                    // we've reached the end of the current cache block (which may be end of entire dir
                    // so just break the loop
                    break;
                }

                // Check if the current entry is a dir or a file
                if (tocEntryPointer->fileProperties & 0x02) {
                    DPRINTF("We don't want files now\n\n");
                } else { // it must be a file
                    copyToTocEntry(&localTocEntry, tocEntryPointer);

                    if (strlen(extensions) > 0) {
                        // check if the file matches the extension list
                        if (compareTocEntry(localTocEntry.filename, extensions)) {
                            DPRINTF("We found a file that matches the requested extension list\n\n");
                            // Copy from localTocEntry
                            tocEntry[matched_entries] = localTocEntry; 
                            matched_entries++;
                        } else {
                            DPRINTF("We found a file, but it didnt match the requested extension list\n\n");
                        }
                    } else { // no extension list to match against
                        DPRINTF("We found a file, and there is not extension list to match against\n\n");

                        // Copy from localTocEntry
                        tocEntry[matched_entries] = localTocEntry; 
                        matched_entries++;
                    }
                }

                dir_entry++;

                if ((unsigned int)matched_entries >= req_entries)  // if we've filled the requested buffer
                    return (matched_entries);        // then just return

            }  // end of the current cache block


            // if there is more dir to load, then load next chunk, else finish
            if ((cacheInfoDir.cache_offset + cacheInfoDir.cache_size) < cacheInfoDir.sector_num) {
                if (!cdfs_cacheDir(cacheInfoDir.pathname, CACHE_NEXT)) {
                    // failed to cache next block (should return TRUE even if
                    // there is no more directory, as long as a CD read didnt fail
                    return -1;
                }
            } else
                break;

            tocEntryPointer = (struct DirTocEntry *)cacheInfoDir.cache;
        }
    }
    // reached the end of the dir, before filling up the requested entries

    return (matched_entries);
}

// This function uses sceCdTrayReq to check if the disc in the disc drive has changed.
// Once sceCdTrayReq is called it will reset the flag inside CDVDMAN.
// Avoid using sceCdTrayReq with SCECdTrayCheck as arguemnt in other code if possible.

int cdfs_checkDiskChanged(enum Cdvd_Changed_Index index) {
    u32 res = 0;
    sceCdTrayReq(SCECdTrayCheck, &res);
    if (res) {
        cdvdChangedMagic += 1;
    }
    if (cdvdChangedMagic != cdvdChangedMagicLast[index]) {
        cdvdChangedMagicLast[index] = cdvdChangedMagic;
        return TRUE;
    }
    return FALSE;
}
