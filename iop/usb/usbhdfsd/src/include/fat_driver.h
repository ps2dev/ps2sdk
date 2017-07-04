#ifndef _FAT_DRIVER_H
#define _FAT_DRIVER_H 1

#define DIR_CHAIN_SIZE 32

#define FAT_MAX_NAME 256

//attributes (bits:5-Archive 4-Directory 3-Volume Label 2-System 1-Hidden 0-Read Only)
#define FAT_ATTR_READONLY     0x01
#define FAT_ATTR_HIDDEN       0x02
#define FAT_ATTR_SYSTEM       0x04
#define FAT_ATTR_VOLUME_LABEL 0x08
#define FAT_ATTR_DIRECTORY    0x10
#define FAT_ATTR_ARCHIVE      0x20

typedef struct _fat_dir_list {
	unsigned int  direntryCluster; //the directory cluster requested by getFirstDirentry
	int direntryIndex; //index of the directory children
} fat_dir_list;

typedef struct _fat_dir_chain_record {
	unsigned int cluster;
	unsigned int index;
} fat_dir_chain_record;

typedef struct _fat_dir {
	unsigned char attr;		//attributes (bits:5-Archive 4-Directory 3-Volume Label 2-System 1-Hidden 0-Read Only)
	char name[FAT_MAX_NAME];
	unsigned char cdate[4];	//D:M:Yl:Yh
	unsigned char ctime[3]; //H:M:S
	unsigned char adate[4];	//D:M:Yl:Yh
	unsigned char atime[3]; //H:M:S
	unsigned char mdate[4];	//D:M:Yl:Yh
	unsigned char mtime[3]; //H:M:S
	unsigned int size;		//file size, 0 for directory
	unsigned int parentDirCluster;	//The cluster number of the parent directory.
	unsigned int startCluster;
	//Stuff here are used for caching and might not be filled.
	unsigned int lastCluster;
	fat_dir_chain_record chain[DIR_CHAIN_SIZE];  //cluser/offset cache - for seeking purpose
} fat_dir;

int strEqual(const char *s1, const char* s2);

int fat_mount(mass_dev* dev, unsigned int start, unsigned int count);
void fat_forceUnmount(mass_dev* dev);
void fat_setFatDirChain(fat_driver* fatd, fat_dir* fatDir);
int fat_readFile(fat_driver* fatd, fat_dir* fatDir, unsigned int filePos, unsigned char* buffer, unsigned int size);
int fat_getFirstDirentry(fat_driver* fatd, const char* dirName, fat_dir_list* fatdlist, fat_dir *fatDir_host, fat_dir* fatDir);
int fat_getNextDirentry(fat_driver* fatd, fat_dir_list* fatdlist, fat_dir* fatDir);

fat_driver * fat_getData(int device);
int      fat_getFileStartCluster(fat_driver* fatd, const char* fname, unsigned int* startCluster, fat_dir* fatDir);
int      fat_getClusterChain(fat_driver* fatd, unsigned int cluster, unsigned int* buf, unsigned int bufSize, int startFlag);

#endif

