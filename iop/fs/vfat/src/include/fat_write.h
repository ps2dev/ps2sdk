#ifndef _FAT_WRITE_H
#define _FAT_WRITE_H 1

extern int fat_createFile(fat_driver *fatd, const char *fname, char directory, char escapeNotExist, unsigned int *cluster, unsigned int *sfnSector, int *sfnOffset);
extern int fat_deleteFile(fat_driver *fatd, const char *fname, char directory);
extern int fat_truncateFile(fat_driver *fatd, unsigned int cluster, unsigned int sfnSector, int sfnOffset);
extern int fat_renameFile(fat_driver *fatd, fat_dir *fatdir, const char *fname);
extern int fat_writeFile(fat_driver *fatd, fat_dir *fatDir, int *updateClusterIndices, unsigned int filePos, unsigned char *buffer, int size);
extern int fat_updateSfn(fat_driver *fatd, int size, unsigned int sfnSector, int sfnOffset);

extern int fat_allocSector(fat_driver *fatd, unsigned int sector, unsigned char **buf);
extern int fat_writeSector(fat_driver *fatd, unsigned int sector);
extern int fat_flushSectors(fat_driver *fatd);
#endif /* _FAT_WRITE_H */
