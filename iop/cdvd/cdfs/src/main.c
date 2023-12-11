#include <stdio.h>
#include <ioman.h>
#include <iox_stat.h>
#include <loadcore.h>
#include <sysclib.h>

#include "cdfs_iop.h"

// 16 sectors worth of toc entry
#define MAX_FILES_PER_FOLDER 256
#define MAX_FILES_OPENED 16
#define MAX_FOLDERS_OPENED 16
#define MAX_BYTES_READ 16384

#define DRIVER_UNIT_NAME "cdfs"
#define DRIVER_UNIT_VERSION 2
#define VERSION_STRINGIFY(x) #x


IRX_ID(MODNAME, 1, 1);

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
static int lastsector = -1;
static int last_bk = 0;

/***********************************************
*                                              *
*             DRIVER FUNCTIONS                 *
*                                              *
***********************************************/

static int fio_init(iop_device_t *driver)
{
    printf("%s\n", driver->desc);
    printf("Re-edited by fjtrujy\n");
    printf("Original implementation\n");
    printf("by A.Lee (aka Hiryu) & Nicholas Van Veen (aka Sjeep)\n");
    printf("CDFS: Initializing '%s' file driver.\n", driver->name);

    cdfs_start();
    return 0;
}

static int fio_deinit(iop_device_t *f)
{
    (void)f;

    DPRINTF("CDFS: fio_deinit called.\n");
    DPRINTF("      kernel_fd.. %p\n", f);
    return cdfs_finish();
}

static int fio_open(iop_file_t *f, const char *name, int mode)
{
    int j;
    static struct TocEntry tocEntry;

    DPRINTF("CDFS: fio_open called.\n");
    DPRINTF("      kernel_fd.. %p\n", f);
    DPRINTF("      name....... %s %x\n", name, (int)name);
    DPRINTF("      mode....... %d\n\n", mode);

    // Invalidate last sector cache if disk changed
    if (cdfs_checkDiskChanged(CHANGED_FIO)) {
        lastsector = -1;
        last_bk = 0;
    }

    // check if the file exists
    if (!cdfs_findfile(name, &tocEntry)) {
        printf("***** FILE %s CAN NOT FOUND ******\n\n", name);
        return -1;
    }

    if (mode != O_RDONLY) {
        printf("mode is different than O_RDONLY, expected %i, received %i\n\n", O_RDONLY, mode);
        return -2;
    }   

    DPRINTF("CDFS: fio_open TocEntry info\n");
    DPRINTF("      TocEntry....... %p\n", &tocEntry);
    DPRINTF("      fileLBA........ %i\n", tocEntry.fileLBA);
    DPRINTF("      fileSize....... %i\n", tocEntry.fileSize);
    DPRINTF("      fileProperties. %i\n", tocEntry.fileProperties);
    DPRINTF("      dateStamp...... %s\n", tocEntry.dateStamp);
    DPRINTF("      filename....... %s\n", tocEntry.filename);

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

    DPRINTF("CDFS: fio_close called.\n");
    DPRINTF("      kernel fd.. %p\n\n", f);

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
    static u8 local_buffer[9 * 2048];

    DPRINTF("CDFS: fio_read called\n\n");
    DPRINTF("      kernel_fd... %p\n", f);
    DPRINTF("      buffer...... 0x%X\n", (int)buffer);
    DPRINTF("      size........ %d\n\n", size);

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

    if (size > MAX_BYTES_READ)
        size = MAX_BYTES_READ;

    // Now work out where we want to start reading from
    start_sector = fd_table[i].LBA + (fd_table[i].filePos >> 11);
    off_sector = (fd_table[i].filePos & 0x7FF);

    num_sectors = (off_sector + size);
    num_sectors = (num_sectors >> 11) + ((num_sectors & 2047) != 0);

    DPRINTF("fio_read: read sectors %d to %d\n", start_sector, start_sector + num_sectors);

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
            DPRINTF("Couldn't Read from file for some reason\n");
        }
        
        last_bk = num_sectors - 1;
    }

    memcpy(buffer, local_buffer + off_sector, size);
    fd_table[i].filePos += size;

    return (size);
}

static int fio_write(iop_file_t *f, void *buffer, int size)
{
    (void)f;
    (void)buffer;

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

    DPRINTF("CDFS: fio_lseek called.\n");
    DPRINTF("      kernel_fd... %p\n", f);
    DPRINTF("      offset...... %d\n", offset);
    DPRINTF("      whence...... %d\n\n", whence);

    i = (int) f->privdata;

    if (i >= 16) {
        DPRINTF("fio_lseek: ERROR: File does not appear to be open!\n");
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

    DPRINTF("CDFS: fio_openDir called.\n");
    DPRINTF("      kernel_fd.. %p\n", f);
    DPRINTF("      name....... %s\n", f->device->name);
    DPRINTF("      mode....... %d\n\n", f->mode);
    DPRINTF("      path....... %s\n\n", path);

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

    DPRINTF("ITEMS %i\n\n", fod_table[j].files);
#ifdef DEBUG
    int index = 0;
    for (index=0; index < fod_table[j].files; index++) {
        struct TocEntry tocEntry = fod_table[j].entries[index];
        
        DPRINTF("CDFS: fio_openDir index=%d TocEntry info\n", index);
        DPRINTF("      TocEntry....... %p\n", &tocEntry);
        DPRINTF("      fileLBA........ %i\n", tocEntry.fileLBA);
        DPRINTF("      fileSize....... %i\n", tocEntry.fileSize);
        DPRINTF("      fileProperties. %i\n", tocEntry.fileProperties);
        DPRINTF("      dateStamp....... %s\n", tocEntry.dateStamp);
        DPRINTF("      filename....... %s\n", tocEntry.filename);
    }
#endif
   
    f->privdata = (void *)j;

    return j;
}

static int fio_closeDir(iop_file_t *fd) 
{
    int i;

    DPRINTF("CDFS: fio_closeDir called.\n");
    DPRINTF("      kernel_fd.. %p\n", fd);

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

    DPRINTF("CDFS: fio_dread called.\n");
    DPRINTF("      kernel_fd.. %p\n", fd);
    DPRINTF("      mode....... %p\n\n", dirent);

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

    DPRINTF("fio_dread: fod_table index=%i, fileIndex=%i\n\n", i, filesIndex);
    DPRINTF("fio_dread: entries=%i\n\n", fod_table[i].files);
    DPRINTF("fio_dread: reading entry\n\n");
    DPRINTF("      entry.. %p\n", &entry);
    DPRINTF("      filesize....... %i\n\n", entry.fileSize);
    DPRINTF("      filename....... %s\n\n", entry.filename);
    DPRINTF("      fileproperties.. %i\n\n", entry.fileProperties);

    dirent->stat.mode = (entry.fileProperties == CDFS_FILEPROPERTY_DIR) ? FIO_SO_IFDIR : FIO_SO_IFREG;
    dirent->stat.attr = entry.fileProperties;
    dirent->stat.size = entry.fileSize;
    memcpy(dirent->stat.ctime, entry.dateStamp, 8);
    memcpy(dirent->stat.atime, entry.dateStamp, 8);
    memcpy(dirent->stat.mtime, entry.dateStamp, 8);
    strncpy(dirent->name, entry.filename, 128);
    
    fod_table[i].filesIndex++;
    return fod_table[i].filesIndex;
}

static int fio_getstat(iop_file_t *fd, const char *name, io_stat_t *stat) 
{
    struct TocEntry entry;
    int ret = -1;

    (void)fd;

    DPRINTF("CDFS: fio_getstat called.\n");
    DPRINTF("      kernel_fd.. %p\n", fd);
    DPRINTF("      name....... %s\n\n", name);

    ret = cdfs_findfile(name, &entry);

    DPRINTF("      entry.. %p\n", &entry);
    DPRINTF("      filesize....... %i\n\n", entry.fileSize);
    DPRINTF("      filename....... %s\n\n", entry.filename);
    DPRINTF("      fileproperties.. %i\n\n", entry.fileProperties);

    stat->mode = (entry.fileProperties == CDFS_FILEPROPERTY_DIR) ? FIO_SO_IFDIR : FIO_SO_IFREG;
    stat->attr = entry.fileProperties;
    stat->size = entry.fileSize;
    memcpy(stat->ctime, entry.dateStamp, 8);
    memcpy(stat->atime, entry.dateStamp, 8);
    memcpy(stat->mtime, entry.dateStamp, 8);

    return ret;
}

static int cdfs_dummy() {
    DPRINTF("CDFS: dummy function called\n\n");
    return -5;
}

static iop_device_ops_t fio_ops = {
    &fio_init,
    &fio_deinit,
    (void *)&cdfs_dummy,
    &fio_open,
    &fio_close,
    &fio_read,
    &fio_write,
    &fio_lseek,
    (void *)&cdfs_dummy,
    (void *)&cdfs_dummy,
    (void *)&cdfs_dummy,
    (void *)&cdfs_dummy,
    &fio_openDir,
    &fio_closeDir,
    &fio_dread,
    &fio_getstat,
    (void *)&cdfs_dummy,
};

static iop_device_t fio_driver = {
    DRIVER_UNIT_NAME,
    IOP_DT_FS,
    DRIVER_UNIT_VERSION,
    DRIVER_UNIT_NAME " Filedriver v" VERSION_STRINGIFY(DRIVER_UNIT_VERSION),
    &fio_ops,
};

int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // Prepare cache and read mode
    cdfs_prepare();

    DelDrv(fio_driver.name);
    if(AddDrv(&fio_driver) != 0) { return MODULE_NO_RESIDENT_END; }

    return MODULE_RESIDENT_END;
}
