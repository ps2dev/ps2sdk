#include <stdio.h>
#include <ioman.h>
#include <iox_stat.h>
#include <sysclib.h>

#include "cdfs_iop.h"

// 16 sectors worth of toc entry
#define MAX_FILES_PER_FOLDER 256
#define MAX_FILES_OPENED 16
#define MAX_FOLDERS_OPENED 16
#define MAX_BITS_READ 16384

#define UNIT_NAME "cdfs"

struct fdtable
{
    iop_file_t *fd;
    int fileSize;
    int LBA;
    int filePos;
};

struct fodtable
{
    iop_file_t *fd;
    int files;
    int filesIndex;
    struct TocEntry entries[MAX_FILES_PER_FOLDER];
};

// File Descriptors
static struct fdtable fd_table[MAX_FILES_OPENED];
static int fd_used[MAX_FILES_OPENED];

// Folder Descriptors
static struct fodtable fod_table[MAX_FOLDERS_OPENED];
static int fod_used[MAX_FOLDERS_OPENED];

// global variables
static int lastsector;
static int last_bk = 0;

/***********************************************
*                                              *
*             DRIVER FUNCTIONS                 *
*                                              *
***********************************************/

static int fio_init(iop_device_t *driver)
{
    printf("CDFS Filesystem v2\n");
    printf("Re-edited by fjtrujy\n");
    printf("Original implementation\n");
    printf("by A.Lee (aka Hiryu) & Nicholas Van Veen (aka Sjeep)\n");
    printf("CDFS: Initializing '%s' file driver.\n", driver->name);

    cdfs_start();
    return 0;
}

static int fio_deinit(iop_device_t *f)
{
#if defined(DEBUG)
    printf("CDFS: fio_deinit called.\n");
    printf("      kernel_fd.. %p\n", f);
#endif
    return cdfs_finish();
}

static int fio_open(iop_file_t *f, const char *name, int mode)
{
    int j;
    static struct TocEntry tocEntry;

#ifdef DEBUG
    printf("CDFS: fio_open called.\n");
    printf("      kernel_fd.. %p\n", f);
    printf("      name....... %s %x\n", name, (int)name);
    printf("      mode....... %d\n\n", mode);
#endif

    // check if the file exists
    if (!cdfs_findfile(name, &tocEntry)) {
        printf("***** FILE %s CAN NOT FOUND ******\n\n", name);
        return -1;
    }

    if (mode != O_RDONLY) {
        printf("mode is different than O_RDONLY, expected %i, received %i\n\n", O_RDONLY, mode);
        return -2;
    }   

#ifdef DEBUG
    printf("CDFS: fio_open TocEntry info\n");
    printf("      TocEntry....... %p\n", &tocEntry);
    printf("      fileLBA........ %i\n", tocEntry.fileLBA);
    printf("      fileSize....... %i\n", tocEntry.fileSize);
    printf("      fileProperties. %i\n", tocEntry.fileProperties);
    printf("      dateStamp...... %s\n", tocEntry.dateStamp);
    printf("      filename....... %s\n", tocEntry.filename);
#endif

    // set up a new file descriptor
    for (j = 0; j < MAX_FILES_OPENED; j++) {
        if (fd_used[j] == 0)
            break;
    }

    if (j >= MAX_FILES_OPENED) {
        printf("File descriptor overflow!!\n\n");
        return -3;
    }

    fd_used[j] = 1;
    fd_table[j].fd = f;
    fd_table[j].fileSize = tocEntry.fileSize;
    fd_table[j].LBA = tocEntry.fileLBA;
    fd_table[j].filePos = 0;

    f->privdata = (void *)j;

    return j;
}

static int fio_close(iop_file_t *f)
{
    int i;

#ifdef DEBUG
    printf("CDFS: fio_close called.\n");
    printf("      kernel fd.. %p\n\n", f);
#endif

    i = (int)f->privdata;

    if (i >= MAX_FILES_OPENED) {
        printf("fio_close: ERROR: File does not appear to be open!\n");
        return -1;
    }

    fd_used[i] = 0;

    return 0;
}

static int fio_read(iop_file_t *f, void *buffer, int size)
{
    int i;

    int start_sector;
    int off_sector;
    int num_sectors;

    int read = 0;
    static char local_buffer[9 * 2048];


#ifdef DEBUG
    printf("CDFS: fio_read called\n\n");
    printf("      kernel_fd... %p\n", f);
    printf("      buffer...... 0x%X\n", (int)buffer);
    printf("      size........ %d\n\n", size);
#endif

    i = (int)f->privdata;

    if (i >= MAX_FILES_OPENED) {
        printf("fio_read: ERROR: File does not appear to be open!\n");
        return -1;
    }

    // A few sanity checks
    if (fd_table[i].filePos > fd_table[i].fileSize) {
        // We cant start reading from past the beginning of the file
        return 0;  // File exists but we couldnt read anything from it
    }

    if ((fd_table[i].filePos + size) > fd_table[i].fileSize)
        size = fd_table[i].fileSize - fd_table[i].filePos;

    if (size <= 0)
        return 0;

    if (size > MAX_BITS_READ)
        size = MAX_BITS_READ;

    // Now work out where we want to start reading from
    start_sector = fd_table[i].LBA + (fd_table[i].filePos >> 11);
    off_sector = (fd_table[i].filePos & 0x7FF);

    num_sectors = (off_sector + size);
    num_sectors = (num_sectors >> 11) + ((num_sectors & 2047) != 0);

#ifdef DEBUG
    printf("fio_read: read sectors %d to %d\n", start_sector, start_sector + num_sectors);
#endif

    // Skip a Sector for equal (use the last sector in buffer)
    if (start_sector == lastsector) {
        read = 1;
        if (last_bk > 0)
            memcpy(local_buffer, local_buffer + 2048 * (last_bk), 2048);
        last_bk = 0;
    }

    lastsector = start_sector + num_sectors - 1;
    // Read the data (we only ever get 16KB max request at once)

    if (read == 0 || (read == 1 && num_sectors > 1)) {
        if (!cdfs_readSect(start_sector + read, num_sectors - read, local_buffer + ((read) << 11))) {
#ifdef DEBUG
            printf("Couldn't Read from file for some reason\n");
#endif
        }

        last_bk = num_sectors - 1;
    }

    memcpy(buffer, local_buffer + off_sector, size);
    fd_table[i].filePos += size;

    return (size);
}

static int fio_write(iop_file_t *f, void *buffer, int size)
{
    if (size == 0)
        return 0;
    else {
        printf("CDFS: dummy fio_write function called, this is not a re-writer xD");
        return -1;
    }
}

static int fio_lseek(iop_file_t *f, int offset, int whence)
{
    int i;

#ifdef DEBUG
    printf("CDFS: fio_lseek called.\n");
    printf("      kernel_fd... %p\n", f);
    printf("      offset...... %d\n", offset);
    printf("      whence...... %d\n\n", whence);
#endif

    i = (int) f->privdata;

    if (i >= 16) {
#ifdef DEBUG
        printf("fio_lseek: ERROR: File does not appear to be open!\n");
#endif

        return -1;
    }

    switch (whence) {
        case SEEK_SET:
            fd_table[i].filePos = offset;
            break;

        case SEEK_CUR:
            fd_table[i].filePos += offset;
            break;

        case SEEK_END:
            fd_table[i].filePos = fd_table[i].fileSize + offset;
            break;

        default:
            return -1;
    }

    if (fd_table[i].filePos < 0)
        fd_table[i].filePos = 0;

    if (fd_table[i].filePos > fd_table[i].fileSize)
        fd_table[i].filePos = fd_table[i].fileSize;

    return fd_table[i].filePos;
}

static int fio_openDir(iop_file_t *f, const char *path) {
   int j;

#ifdef DEBUG
    printf("CDFS: fio_openDir called.\n");
    printf("      kernel_fd.. %p\n", f);
    printf("      name....... %s\n", f->device->name);
    printf("      mode....... %d\n\n", f->mode);
    printf("      path....... %s\n\n", path);
#endif

    // set up a new file descriptor
    for (j = 0; j < MAX_FOLDERS_OPENED; j++) {
        if (fod_used[j] == 0)
            break;
    }

    if (j >= MAX_FOLDERS_OPENED)
        return -3;

    fod_table[j].files = cdfs_getDir(path, NULL, CDFS_GET_FILES_AND_DIRS, fod_table[j].entries, MAX_FILES_PER_FOLDER);
    if (fod_table[j].files < 0) {
        printf("The path doesn't exist\n\n");
        return -2;
    }

    fod_table[j].filesIndex = 0;
    fod_table[j].fd = f;
    fod_used[j] = 1;

#ifdef DEBUG
    printf("ITEMS %i\n\n", fod_table[j].files);
    int index = 0;
    for (index=0; index < fod_table[j].files; index++) {
        struct TocEntry tocEntry = fod_table[j].entries[index];
        
        printf("CDFS: fio_openDir index=%d TocEntry info\n", index);
        printf("      TocEntry....... %p\n", &tocEntry);
        printf("      fileLBA........ %i\n", tocEntry.fileLBA);
        printf("      fileSize....... %i\n", tocEntry.fileSize);
        printf("      fileProperties. %i\n", tocEntry.fileProperties);
        printf("      dateStamp....... %s\n", tocEntry.dateStamp);
        printf("      filename....... %s\n", tocEntry.filename);
    }
#endif
   
    f->privdata = (void *)j;

    return j;
}

static int fio_closeDir(iop_file_t *fd) 
{
    int i;

#ifdef DEBUG
    printf("CDFS: fio_closeDir called.\n");
    printf("      kernel_fd.. %p\n", fd);
#endif

    i = (int)fd->privdata;

    if (i >= MAX_FOLDERS_OPENED) {
        printf("fio_close: ERROR: File does not appear to be open!\n");
        return -1;
    }

    fod_used[i] = 0;
    return 0;
}

static int fio_dread(iop_file_t *fd, io_dirent_t *dirent)
{
    int i;
    int filesIndex;
    struct TocEntry entry;
#ifdef DEBUG
    printf("CDFS: fio_dread called.\n");
    printf("      kernel_fd.. %p\n", fd);
    printf("      mode....... %p\n\n", dirent);
#endif
    i = (int)fd->privdata;

    if (i >= MAX_FOLDERS_OPENED) {
        printf("fio_dread: ERROR: Folder does not appear to be open!\n\n");
        return -1;
    }

    filesIndex = fod_table[i].filesIndex;
    if (filesIndex >= fod_table[i].files) {
        printf("fio_dread: No more items pending to read!\n\n");
        return -1;
    }

    entry = fod_table[i].entries[filesIndex];
#ifdef DEBUG
    printf("fio_dread: fod_table index=%i, fileIndex=%i\n\n", i, filesIndex);
    printf("fio_dread: entries=%i\n\n", fod_table[i].files);
    printf("fio_dread: reading entry\n\n");
    printf("      entry.. %p\n", entry);
    printf("      filesize....... %i\n\n", entry.fileSize);
    printf("      filename....... %s\n\n", entry.filename);
    printf("      fileproperties.. %i\n\n", entry.fileProperties);
#endif
    dirent->stat.mode = (entry.fileProperties == CDFS_FILEPROPERTY_DIR) ? FIO_SO_IFDIR : FIO_SO_IFREG;
    dirent->stat.attr = entry.fileProperties;
    dirent->stat.size = entry.fileSize;
    memcpy(dirent->stat.ctime, entry.dateStamp, 8);
    memcpy(dirent->stat.atime, entry.dateStamp, 8);
    memcpy(dirent->stat.mtime, entry.dateStamp, 8);
    strncpy(dirent->name, entry.filename, 128);
    dirent->unknown = 0;
    
    fod_table[i].filesIndex++;
    return fod_table[i].filesIndex;
}

static int fio_getstat(iop_file_t *fd, const char *name, io_stat_t *stat) 
{
    struct TocEntry entry;
    int ret = -1;
#ifdef DEBUG
    printf("CDFS: fio_getstat called.\n");
    printf("      kernel_fd.. %p\n", fd);
    printf("      name....... %s\n\n", name);
#endif
    ret = cdfs_findfile(name, &entry);
#ifdef DEBUG
    printf("      entry.. %p\n", entry);
    printf("      filesize....... %i\n\n", entry.fileSize);
    printf("      filename....... %s\n\n", entry.filename);
    printf("      fileproperties.. %i\n\n", entry.fileProperties);
#endif
    stat->mode = (entry.fileProperties == CDFS_FILEPROPERTY_DIR) ? FIO_SO_IFDIR : FIO_SO_IFREG;
    stat->attr = entry.fileProperties;
    stat->size = entry.fileSize;
    memcpy(stat->ctime, entry.dateStamp, 8);
    memcpy(stat->atime, entry.dateStamp, 8);
    memcpy(stat->mtime, entry.dateStamp, 8);

    return ret;
}

static int cdfs_dummy() {
    printf("CDFS: dummy function called\n\n");
    return -5;
}

static iop_device_ops_t fio_ops = {
    fio_init,
    fio_deinit,
    cdfs_dummy,
    fio_open,
    fio_close,
    fio_read,
    fio_write,
    fio_lseek,
    cdfs_dummy,
    cdfs_dummy,
    cdfs_dummy,
    cdfs_dummy,
    fio_openDir,
    fio_closeDir,
    fio_dread,
    fio_getstat,
    cdfs_dummy
};

int _start(int argc, char **argv)
{
    static iop_device_t fio_driver;

    // Prepare cache and read mode
    cdfs_prepare();

    // setup the fio_driver structure
    fio_driver.name = UNIT_NAME;
    fio_driver.type = IOP_DT_FS;
    fio_driver.version = 2;
    fio_driver.desc = "CDFS Filedriver";
    fio_driver.ops = &fio_ops;

    DelDrv(UNIT_NAME);
    if(AddDrv(&fio_driver) != 0) { return(-1); }

    return(0);
}
