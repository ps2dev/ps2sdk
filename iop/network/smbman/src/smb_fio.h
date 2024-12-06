/*
  Copyright 2009-2010, jimmikaelkael
  Licenced under Academic Free License version 3.0
*/

#ifndef __SMB_FIO_H__
#define __SMB_FIO_H__

// smb driver ops functions prototypes
extern int smb_initdev(void);
extern int smb_init(iop_device_t *iop_dev);
extern int smb_deinit(iop_device_t *dev);
extern int smb_open(iop_file_t *f, const char *filename, int flags, int mode);
extern int smb_close(iop_file_t *f);
extern int smb_lseek(iop_file_t *f, int pos, int where);
extern int smb_read(iop_file_t *f, void *buf, int size);
extern int smb_write(iop_file_t *f, void *buf, int size);
extern int smb_remove(iop_file_t *f, const char *filename);
extern int smb_mkdir(iop_file_t *f, const char *dirname, int mode);
extern int smb_rmdir(iop_file_t *f, const char *dirname);
extern int smb_dopen(iop_file_t *f, const char *dirname);
extern int smb_dclose(iop_file_t *f);
extern int smb_dread(iop_file_t *f, iox_dirent_t *dirent);
extern int smb_getstat(iop_file_t *f, const char *filename, iox_stat_t *stat);
extern int smb_rename(iop_file_t *f, const char *oldname, const char *newname);
extern int smb_chdir(iop_file_t *f, const char *dirname);
extern s64 smb_lseek64(iop_file_t *f, s64 pos, int where);
extern int smb_devctl(iop_file_t *f, const char *devname, int cmd, void *arg, unsigned int arglen, void *bufp, unsigned int buflen);

#endif
