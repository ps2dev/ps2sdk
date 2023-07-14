/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <mcman.h>
#include "mcman-internal.h"

static int mcman_curdirmaxent;
static int mcman_curdirlength;
static char mcman_curdirpath[1024];
static const char *mcman_curdirname;
static sceMcStDateTime mcman_fsmodtime;

//--------------------------------------------------------------
int mcman_format2(int port, int slot)
{
	register int r, i, size, ifc_index, indirect_offset, allocatable_clusters_per_card;
	register int ifc_length, fat_length, fat_entry, alloc_offset;
	register int j = 0, z = 0;
	MCDevInfo *mcdi = &mcman_devinfos[port][slot];
	McCacheEntry *mce;

	DPRINTF("mcman_format2 port%d, slot%d cardform %d\n", port, slot, mcdi->cardform);

	if (mcdi->cardform < 0) {
		for (i = 0; i < 32; i++)
			mcdi->bad_block_list[i] = -1;
		mcdi->rootdir_cluster = 0;
		mcdi->rootdir_cluster2 = 0;
		goto lbl1;
	}

	if (mcdi->cardform > 0) {
		if (((mcdi->version[0] - 48) >= 2) || ((mcdi->version[2] - 48) >= 2))
			goto lbl1;
	}

	r = mcman_reportBadBlocks(port, slot);
	if ((r != sceMcResSucceed) && (r != sceMcResNoFormat))
		return sceMcResChangedCard;

lbl1:
	// set superblock magic & version
	memset((void *)&mcdi->magic, 0, sizeof (mcdi->magic) + sizeof (mcdi->version));
	strcpy(mcdi->magic, SUPERBLOCK_MAGIC);
	strcat(mcdi->magic, SUPERBLOCK_VERSION);

	//size = 8192 / mcdi->cluster_size; // get blocksize in cluster a better way must be found
	size = mcdi->blocksize;
	mcdi->cardform = -1;

	mcman_wr_port = -1;
	mcman_wr_slot = -1;
	mcman_wr_block = -1;
	mcman_wr_flag3 = -10;

	//if (size <= 0)
	//	size = 1;
	if (mcdi->blocksize <= 0)
		size = 8;

	// clear first 8 clusters
	for (i = 0; i < size; i++) {
		r = mcman_writecluster(port, slot, i, 1);
		if (r == 0)
			return sceMcResNoFormat;

		if (r != 1)
			return -40;
	}

	fat_length = (((mcdi->clusters_per_card << 2) - 1) / mcdi->cluster_size) + 1; // get length of fat in clusters
	ifc_length = (((fat_length << 2) - 1) / mcdi->cluster_size) + 1; // get number of needed ifc clusters

	if (!(ifc_length <= 32)) {
		ifc_length = 32;
		fat_length = mcdi->FATentries_per_cluster << 5;
	}

	// clear ifc clusters
	if (ifc_length > 0) {
		j = 0;
		do {
			if ((u32)i >= mcdi->clusters_per_card)
				return sceMcResNoFormat;

			do {
				if (mcman_writecluster(port, slot, i, 1) != 0)
					break;

				i++;
			} while ((u32)i < mcdi->clusters_per_card);

			if ((u32)i >= mcdi->clusters_per_card)
				return sceMcResNoFormat;

			mcdi->ifc_list[j] = i;
			j++;
			i++;
		} while (j < ifc_length);
	}

	// read ifc clusters to mc cache and clear fat clusters
	if (fat_length > 0) {
		j = 0;

		do {
			ifc_index = j / mcdi->FATentries_per_cluster;
			indirect_offset = j % mcdi->FATentries_per_cluster;

			if (indirect_offset == 0) {
				if (McReadCluster(port, slot, mcdi->ifc_list[ifc_index], &mce) != sceMcResSucceed)
					return -42;
				mce->wr_flag = 1;
			}

			if ((u32)i >= mcdi->clusters_per_card)
				return sceMcResNoFormat;

			do {
				r = mcman_writecluster(port, slot, i, 1);
				if (r == 1)
					break;

				if (r < 0)
					return -43;

				i++;
			} while ((u32)i < mcdi->clusters_per_card);

			if ((u32)i >= mcdi->clusters_per_card)
				return sceMcResNoFormat;

			j++;
			McFatCluster *fc = (McFatCluster *)mce->cl_data;
			fc->entry[indirect_offset] = i;
			i++;

		} while (j < fat_length);
	}
	alloc_offset = i;

	mcdi->backup_block1 = 0;
	mcdi->backup_block2 = 0;

	// clear backup blocks
	for (i = (mcdi->clusters_per_card / mcdi->clusters_per_block) - 1; i > 0; i--) {

		r = mcman_writecluster(port, slot, mcdi->clusters_per_block * i, 1);
		if (r < 0)
			return -44;

		if ((r != 0) && (mcdi->backup_block1 == 0))
			mcdi->backup_block1 = i;
		else if ((r != 0) && (mcdi->backup_block2 == 0)) {
			mcdi->backup_block2 = i;
			break;
		}
	}

	// set backup block2 to erased state
	if (mcman_eraseblock(port, slot, mcdi->backup_block2, NULL, NULL) != sceMcResSucceed)
		return -45;

	u32 hi, lo, temp;

	long_multiply(mcdi->clusters_per_card, 0x10624dd3, &hi, &lo);
	temp = (hi >> 6) - (mcdi->clusters_per_card >> 31);
	allocatable_clusters_per_card = (((((temp << 5) - temp) << 2) + temp) << 3) + 1;
	j = alloc_offset;

	// checking for bad allocated clusters and building FAT
	if ((u32)j < i * mcdi->clusters_per_block) {
		z = 0;
		do { // quick check for bad clusters
			r = mcman_writecluster(port, slot, j, 0);
			if (r == 1) {
				if (z == 0) {
					mcdi->alloc_offset = j;
					mcdi->rootdir_cluster = 0;
					fat_entry = 0xffffffff; // marking rootdir end
				}
				else
					fat_entry = ~0x80000000;	// marking free cluster
				z++;
				if (z == allocatable_clusters_per_card)
					mcdi->max_allocatable_clusters = (j - mcdi->alloc_offset) + 1;
			}
			else {
				if (r != 0)
					return -45;
				fat_entry = 0xfffffffd; // marking bad cluster
			}

			if (McSetFATentry(port, slot, j - mcdi->alloc_offset, fat_entry) != sceMcResSucceed)
				return -46;

			j++;
		} while ((u32)j < (i * mcdi->clusters_per_block));
	}

	mcdi->alloc_end = (i * mcdi->clusters_per_block) - mcdi->alloc_offset;

	if (mcdi->max_allocatable_clusters == 0)
		mcdi->max_allocatable_clusters = i * mcdi->clusters_per_block;

	if ((u32)z < mcdi->clusters_per_block)
		return sceMcResNoFormat;

	// read superblock to mc cache
	for (i = 0; (unsigned int)i < sizeof (MCDevInfo); i += MCMAN_CLUSTERSIZE) {
		if (i < 0)
			size = i + (MCMAN_CLUSTERSIZE - 1);
		else
			size = i;

		if (McReadCluster(port, slot, size >> 10, &mce) != sceMcResSucceed)
			return -48;

		size = MCMAN_CLUSTERSIZE;
		mce->wr_flag = 1;

		if ((sizeof (MCDevInfo) - i) <= 1024)
			size = sizeof (MCDevInfo) - i;

		memcpy((void *)mce->cl_data, (void *)(mcdi + i), size);
	}

	mcdi->unknown1 = 0;
	mcdi->unknown2 = 0;
	mcdi->unknown5 = -1;
	mcdi->rootdir_cluster2 = mcdi->rootdir_cluster;

	// Create root dir
	if (McCreateDirentry(port, slot, 0, 0, 0, NULL) != sceMcResSucceed)
		return -49;

	// finally flush cache to memcard
	r = McFlushCache(port, slot);
	if (r != sceMcResSucceed)
		return r;

	mcdi->cardform = 1;

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_dread2(int fd, MC_IO_DRE_T *dirent)
{
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	McFsEntry *fse;

	DPRINTF("mcman_dread2 fd %d\n", fd);

	if (fh->position >= fh->filesize)
		return sceMcResSucceed;

	do {
		register int r;

		r = McReadDirEntry(fh->port, fh->slot, fh->freeclink, fh->position, &fse);
		if (r != sceMcResSucceed)
			return r;

		if (fse->mode & sceMcFileAttrExists)
			break;

		fh->position++;
	} while (fh->position < fh->filesize);

	if (fh->position >= fh->filesize)
		return sceMcResSucceed;

	fh->position++;
	mcman_wmemset((void *)dirent, sizeof (MC_IO_DRE_T), 0);
	strcpy(dirent->name, fse->name);
	*(u8 *)&dirent->name[32] = 0;

	if (fse->mode & sceMcFileAttrReadable)
		dirent->stat.mode |= MC_IO_S_RD;
	if (fse->mode & sceMcFileAttrWriteable)
		dirent->stat.mode |= MC_IO_S_WR;
	if (fse->mode & sceMcFileAttrExecutable)
		dirent->stat.mode |= MC_IO_S_EX;
#if !MCMAN_ENABLE_EXTENDED_DEV_OPS
	if (fse->mode & sceMcFileAttrPS1)
		dirent->stat.mode |= sceMcFileAttrPS1;
	if (fse->mode & sceMcFileAttrPDAExec)
		dirent->stat.mode |= sceMcFileAttrPDAExec;
	if (fse->mode & sceMcFileAttrDupProhibit)
		dirent->stat.mode |= sceMcFileAttrDupProhibit;
#endif
	if (fse->mode & sceMcFileAttrSubdir)
		dirent->stat.mode |= MC_IO_S_DR;
	else
		dirent->stat.mode |= MC_IO_S_FL;

	dirent->stat.attr = fse->attr;
	dirent->stat.size = fse->length;
	memcpy(dirent->stat.ctime, &fse->created, sizeof(sceMcStDateTime));
	memcpy(dirent->stat.mtime, &fse->modified, sizeof(sceMcStDateTime));

	return 1;
}

//--------------------------------------------------------------
int mcman_getstat2(int port, int slot, const char *filename, MC_IO_STA_T *stat)
{
	register int r;
	McFsEntry *fse;

	DPRINTF("mcman_getstat2 port%d slot%d filename %s\n", port, slot, filename);

	r = mcman_cachedirentry(port, slot, filename, NULL, &fse, 1);
	if (r != sceMcResSucceed)
		return r;

	mcman_wmemset((void *)stat, sizeof (MC_IO_STA_T), 0);

	if (fse->mode & sceMcFileAttrReadable)
		stat->mode |= MC_IO_S_RD;
	if (fse->mode & sceMcFileAttrWriteable)
		stat->mode |= MC_IO_S_WR;
	if (fse->mode & sceMcFileAttrExecutable)
		stat->mode |= MC_IO_S_EX;
#if !MCMAN_ENABLE_EXTENDED_DEV_OPS
	if (fse->mode & sceMcFileAttrPS1)
		stat->mode |= sceMcFileAttrPS1;
	if (fse->mode & sceMcFileAttrPDAExec)
		stat->mode |= sceMcFileAttrPDAExec;
	if (fse->mode & sceMcFileAttrDupProhibit)
		stat->mode |= sceMcFileAttrDupProhibit;
#endif
	if (fse->mode & sceMcFileAttrSubdir)
		stat->mode |= MC_IO_S_DR;
	else
		stat->mode |= MC_IO_S_FL;

	stat->attr = fse->attr;

	if (!(fse->mode & sceMcFileAttrSubdir))
		stat->size = fse->length;

	memcpy(stat->ctime, &fse->created, sizeof(sceMcStDateTime));
	memcpy(stat->mtime, &fse->modified, sizeof(sceMcStDateTime));

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_setinfo2(int port, int slot, const char *filename, sceMcTblGetDir *info, int flags)
{
	register int r, fmode;
	McFsEntry *fse;
	McCacheDir dirInfo;
	McFsEntry mfe;

	DPRINTF("mcman_setinfo2 port%d slot%d filename %s flags %x\n", port, slot, filename, flags);

	r = mcman_cachedirentry(port, slot, filename, &dirInfo, &fse, 1); //dirInfo=sp218 fse=sp228
	if (r != sceMcResSucceed)
		return r;

	if ((flags & sceMcFileAttrFile) != 0)	{
		u8 *pfsentry, *pfseend, *mfee;

		if ((!strcmp(".", info->EntryName)) || (!strcmp("..", info->EntryName)))
			return sceMcResNoEntry;

		if (info->EntryName[0] == 0)
			return sceMcResNoEntry;

		r = mcman_chrpos(info->EntryName, '/');
		if (r >= 0)
			return sceMcResNoEntry;

		if (dirInfo.fsindex < 2)
			return sceMcResNoEntry;

		r = McReadDirEntry(port, slot, dirInfo.cluster, 0, &fse);
		if (r != sceMcResSucceed)
			return r;

		r = McReadDirEntry(port, slot, fse->cluster, fse->dir_entry, &fse);
		if (r != sceMcResSucceed)
			return r;

		pfsentry = (u8 *)fse;
		mfee = (u8 *)&mfe;
		pfseend = (u8 *)(pfsentry + sizeof (McFsEntry));

		do {
			*((u32 *)mfee  ) = *((u32 *)pfsentry  );
			*((u32 *)mfee+1) = *((u32 *)pfsentry+1);
			*((u32 *)mfee+2) = *((u32 *)pfsentry+2);
			*((u32 *)mfee+3) = *((u32 *)pfsentry+3);
			pfsentry += 16;
			mfee += 16;
		} while (pfsentry < pfseend);

		r = mcman_getdirinfo(port, slot, &mfe, info->EntryName, NULL, 1);
		if (r != 1) {
			if (r < 2) {
				if (r == 0)
					return sceMcResNoEntry;
				return r;
			}
			else {
				if (r != 2)
					return r;
				return sceMcResDeniedPermit;
			}
		}
	}

	r = McReadDirEntry(port, slot, dirInfo.cluster, dirInfo.fsindex, &fse);
	if (r != sceMcResSucceed)
		return r;

	Mc1stCacheEntSetWrFlagOff();

	//Special fix clause for file managers (like uLaunchELF)
	//This allows writing most entries that can be read by mcGetDir
	//and without usual restrictions. This flags value should cause no conflict
	//as Sony code never uses it, and the value is changed after its use here.
	if(flags == 0xFEED){
		fse->mode = info->AttrFile;
		//fse->unused = info->Reserve1;
		//fse->length = info->FileSizeByte;
		flags = sceMcFileAttrReadable|sceMcFileAttrWriteable;
		//The changed flags value allows more entries to be copied below
	}

	if ((flags & sceMcFileAttrDupProhibit) != 0)
		fse->attr = info->Reserve2;

	if ((flags & sceMcFileAttrExecutable) != 0) {
		fmode = 0xffff7fcf;
		if (!PS1CardFlag)
			fmode = 0x180f;
		fse->mode = (fse->mode & ~fmode) | (info->AttrFile & fmode);
	}

	if ((flags & sceMcFileCreateFile) != 0)	{
		fmode = 0x380f;
		if (!PS1CardFlag)
			fmode = 0x180f;
		fse->mode = (fse->mode & ~fmode) | (info->AttrFile & fmode);
	}

	if ((flags & sceMcFileAttrReadable) != 0)
		fse->created = info->_Create;

	if ((flags & sceMcFileAttrWriteable) != 0)
		fse->modified = info->_Modify;

	if ((flags & sceMcFileAttrFile) != 0) {
		strncpy(fse->name, info->EntryName, 32);
		fse->name[31] = 0;
	}

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_read2(int fd, void *buffer, int nbyte)
{
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	register MCDevInfo *mcdi = &mcman_devinfos[fh->port][fh->slot];
	McCacheEntry *mce;

	DPRINTF("mcman_read2 fd %d buf %x size %d\n", fd, (int)buffer, nbyte);

	if (fh->position < fh->filesize) {
		register int temp, rpos;

		temp = fh->filesize - fh->position;
		if (nbyte > temp)
			nbyte = temp;

		rpos = 0;
		if (nbyte > 0) {

			do {
				register int r, size, offset;

				offset = fh->position % mcdi->cluster_size;  // file pointer offset % cluster size
				temp = mcdi->cluster_size - offset;
				if (temp < nbyte)
					size = temp;
				else
					size = nbyte;

				r = mcman_fatRseek(fd);

				if (r <= 0)
					return r;

				r = McReadCluster(fh->port, fh->slot, r, &mce);
				if (r != sceMcResSucceed)
					return r;

				memcpy((void *)((u8 *)buffer + rpos), (void *)((u8 *)(mce->cl_data) + offset), size);

				rpos += size;
				mce->rd_flag = 1;
				nbyte -= size;
				fh->position += size;

			} while (nbyte);
		}
		return rpos;
	}

	return 0;
}

//--------------------------------------------------------------
int mcman_write2(int fd, void *buffer, int nbyte)
{
	register int r, wpos;
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	register MCDevInfo *mcdi = &mcman_devinfos[fh->port][fh->slot];
	McCacheEntry *mce;

	if (nbyte) {
		if (fh->unknown2 == 0) {
			fh->unknown2 = 1;
			r = mcman_close2(fd);
			if (r != sceMcResSucceed)
				return r;
			r = McFlushCache(fh->port, fh->slot);
			if (r != sceMcResSucceed)
				return r;
		}
	}

	wpos = 0;
	if (nbyte) {
		do {
			register int r2, size, offset;

			r = mcman_fatRseek(fd);
			if (r == sceMcResFullDevice) {

				r2 = mcman_fatWseek(fd);
				if (r2 == r)
					return sceMcResFullDevice;

				if (r2 != sceMcResSucceed)
					return r2;

				r = mcman_fatRseek(fd);
			}
			else {
				if (r < 0)
					return r;
			}

			r = McReadCluster(fh->port, fh->slot, r, &mce);
			if (r != sceMcResSucceed)
				return r;

			mce->rd_flag = 1;

			offset = fh->position % mcdi->cluster_size;  // file pointer offset % cluster size
			r2 = mcdi->cluster_size - offset;
			if (r2 < nbyte)
				size = r2;
			else
				size = nbyte;

			memcpy((void *)((u8 *)(mce->cl_data) + offset), (void *)((u8 *)buffer + wpos), size);

			mce->wr_flag = 1;

			r = fh->position + size;
			fh->position += size;

			if ((u32)r < fh->filesize)
				r = fh->filesize ;

			fh->filesize = r;

			nbyte -= size;
			wpos += size;

		} while (nbyte);
	}

	r = mcman_close2(fd);
	if (r != sceMcResSucceed)
		return r;

	return wpos;
}

//--------------------------------------------------------------
int mcman_close2(int fd)
{
	register int r, fmode;
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	McFsEntry *fse1, *fse2;

	DPRINTF("mcman_close2 fd %d\n", fd);

	r = McReadDirEntry(fh->port, fh->slot, fh->field_20, fh->field_24, &fse1);
	if (r != sceMcResSucceed)
		return -31;

	if (fh->unknown2 == 0) {
		fmode = fse1->mode | sceMcFileAttrClosed;
	}
	else {
		fmode = fse1->mode & 0xff7f;
	}
	fse1->mode = fmode;

	mcman_getmcrtime(&fse1->modified);

	fse1->cluster = fh->freeclink;
	fse1->length = fh->filesize;

	Mc1stCacheEntSetWrFlagOff();

	mcman_fsmodtime = fse1->modified;

	r = McReadDirEntry(fh->port, fh->slot, fh->field_28, fh->field_2C, &fse2);
	if (r != sceMcResSucceed)
		return r;

	fse2->modified = mcman_fsmodtime;

	Mc1stCacheEntSetWrFlagOff();

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_open2(int port, int slot, const char *filename, int flags)
{
	register int fd, i, r, rdflag, wrflag, pos, mcfree;
	register MC_FHANDLE *fh;
	register MCDevInfo *mcdi;
	McCacheDir cacheDir;
	McFsEntry *fse1, *fse2;
	McCacheEntry *mce;
	u8 *pfsentry, *pcache, *pfseend;
	const char *p;
	int fat_entry;

	DPRINTF("mcman_open2 port%d slot%d name %s flags %x\n", port, slot, filename, flags);

	if ((flags & sceMcFileCreateFile) != 0)
		flags |= sceMcFileAttrWriteable; // s5

	//if (!mcman_checkpath(filename))
	//	return sceMcResNoEntry;
	if (filename[0] == 0)
		return sceMcResNoEntry;

	fd = 0;
	do {
		fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
		if (fh->status == 0)
			break;
	} while (++fd < MAX_FDHANDLES);

	if (fd == MAX_FDHANDLES)
		return sceMcResUpLimitHandle;

	fh = (MC_FHANDLE *)&mcman_fdhandles[fd]; // s2

	mcman_wmemset((void *)fh, sizeof (MC_FHANDLE), 0);

	mcdi = (MCDevInfo *)&mcman_devinfos[port][slot]; // s3

	if ((flags & (sceMcFileCreateFile | sceMcFileCreateDir)) == 0)
		cacheDir.maxent = -1; //sp20
	else
		cacheDir.maxent = 0;  //sp20

	//fse1 = sp28
	//sp18 = cacheDir

	fse1 = NULL;
	r = mcman_cachedirentry(port, slot, filename, &cacheDir, &fse1, 1);
	if (r < 0)
		return r;

	if (fse1) {
		pfsentry = (u8 *)fse1;
		pcache = (u8 *)&mcman_dircache[1];
		pfseend = (u8 *)(pfsentry + sizeof(McFsEntry));

		do {
			*((u32 *)pcache  ) = *((u32 *)pfsentry  );
			*((u32 *)pcache+1) = *((u32 *)pfsentry+1);
			*((u32 *)pcache+2) = *((u32 *)pfsentry+2);
			*((u32 *)pcache+3) = *((u32 *)pfsentry+3);
			pfsentry += 16;
			pcache += 16;
		} while (pfsentry < pfseend);
	}

	if ((flags == 0) && ((fse1->mode & sceMcFileAttrExists) == 0))
		r = 1;

	if (r == 2)
		return sceMcResNoEntry;

	if (r == 3)
		return sceMcResDeniedPermit;

	if ((r == 0) && ((flags & sceMcFileCreateDir) != 0))
		return sceMcResNoEntry;

	if ((r == 1) && ((flags & (sceMcFileCreateFile | sceMcFileCreateDir)) == 0))
		return sceMcResNoEntry;

	rdflag = flags & sceMcFileAttrReadable;
	wrflag = flags & sceMcFileAttrWriteable;
	fh->freeclink = -1;
	fh->clink = -1;
	fh->clust_offset = 0;
	fh->filesize = 0;
	fh->position = 0;
	fh->port = port;
	fh->slot = slot;
	fh->unknown2 = 0;
	fh->rdflag = rdflag;
	fh->wrflag = wrflag;
	fh->unknown1 = 0;
	fh->field_20 = cacheDir.cluster;
	fh->field_24 = cacheDir.fsindex;

	// fse2 = sp2c

	if (r == 0) {

		if ((wrflag != 0) && ((mcman_dircache[1].mode & sceMcFileAttrWriteable) == 0))
			return sceMcResDeniedPermit;

		if (!PS1CardFlag) {
			if ((flags & sceMcFileAttrReadable) != 0) {
				if ((mcman_dircache[1].mode & sceMcFileAttrReadable) == 0)
					return sceMcResDeniedPermit;
			}
		}
		r = McReadDirEntry(port, slot, cacheDir.cluster, 0, &fse2);
		if (r != sceMcResSucceed)
			return r;

		fh->field_28 = fse2->cluster;
		fh->field_2C = fse2->dir_entry;

		if ((mcman_dircache[1].mode & sceMcFileAttrSubdir) != 0) {
			if ((mcman_dircache[1].mode & sceMcFileAttrReadable) == 0)
				return sceMcResDeniedPermit;

			fh->freeclink = mcman_dircache[1].cluster;
			fh->rdflag = 0;
			fh->wrflag = 0;
			fh->unknown1 = 0;
			fh->drdflag = 1;
			fh->status = 1;
			fh->filesize = mcman_dircache[1].length;
			fh->clink = fh->freeclink;

			return fd;
		}

		if ((flags & sceMcFileAttrWriteable) != 0) {
			i = 0;
			do {
				register MC_FHANDLE *fh2;

				fh2 = (MC_FHANDLE *)&mcman_fdhandles[i];

				if ((fh2->status == 0) || (fh2->port != port) || (fh2->slot != slot) \
					|| (fh2->field_20 != (u32)(cacheDir.cluster)) || (fh2->field_24 != (u32)(cacheDir.fsindex)))
					continue;

				if (fh2->wrflag != 0)
					return sceMcResDeniedPermit;

			} while (++i < MAX_FDHANDLES);
		}

		if ((flags & sceMcFileCreateFile) != 0) {
			r = McSetDirEntryState(port, slot, cacheDir.cluster, cacheDir.fsindex, 0);
			McFlushCache(port, slot);

			if (r != sceMcResSucceed)
				return -43;

			if (cacheDir.fsindex < cacheDir.maxent)
				cacheDir.maxent = cacheDir.fsindex;
		}
		else {
			fh->freeclink = mcman_dircache[1].cluster;
			fh->filesize = mcman_dircache[1].length;
			fh->clink = fh->freeclink;

			if (fh->rdflag != 0)
				fh->rdflag = (*((u8 *)&mcman_dircache[1].mode)) & sceMcFileAttrReadable;
			else
				fh->rdflag = 0;

			if (fh->wrflag != 0)
				fh->wrflag = (mcman_dircache[1].mode >> 1) & sceMcFileAttrReadable;
			else
				fh->wrflag = 0;

			fh->status = 1;

			return fd;
		}
	}
	else {
		fh->field_28 = cacheDir.cluster;
		fh->field_2C = cacheDir.fsindex;
	}

	r = McReadDirEntry(port, slot, fh->field_28, fh->field_2C, &fse1);
	if (r != sceMcResSucceed)
		return r;

	pfsentry = (u8 *)fse1;
	pcache = (u8 *)&mcman_dircache[2];
	pfseend = (u8 *)(pfsentry + sizeof(McFsEntry));

	do {
		*((u32 *)pcache  ) = *((u32 *)pfsentry  );
		*((u32 *)pcache+1) = *((u32 *)pfsentry+1);
		*((u32 *)pcache+2) = *((u32 *)pfsentry+2);
		*((u32 *)pcache+3) = *((u32 *)pfsentry+3);
		pfsentry += 16;
		pcache += 16;
	} while (pfsentry < pfseend);

	i = -1;
	if (mcman_dircache[2].length == (u32)(cacheDir.maxent)) {
		register int fsindex, fsoffset;

		fsindex = mcman_dircache[2].length / (mcdi->cluster_size >> 9); //v1
		fsoffset = mcman_dircache[2].length % (mcdi->cluster_size >> 9); //v0

		if (fsoffset == 0) {
			register int fat_index;

			fat_index = mcman_dircache[2].cluster;
			i = fsindex;

			if ((mcman_dircache[2].cluster == 0) && (i >= 2)) {
				if (mcman_getFATindex(port, slot, i - 1) >= 0) {
					fat_index = mcman_getFATindex(port, slot, i - 1); // s3
					i = 1;
				}
			}
			i--;

			if (i != -1) {

				do {
					r = McGetFATentry(port, slot, fat_index, &fat_entry);
					if (r != sceMcResSucceed)
						return r;

					if (fat_entry >= -1) {
						r = mcman_findfree2(port, slot, 1);
						if (r < 0)
							return r;

						fat_entry = r;
						mce = mcman_get1stcacheEntp(); // s4

						fat_entry |= 0x80000000;

						r = McSetFATentry(port, slot, fat_index, fat_entry);
						if (r != sceMcResSucceed)
							return r;

						mcman_addcacheentry(mce);
					}
					i--;
					fat_index = fat_entry & ~0x80000000;

				} while (i != -1);
			}
		}

		r = McFlushCache(port, slot);
		if (r != sceMcResSucceed)
			return r;

		i = -1;

		mcman_dircache[2].length++;
	}

	do {
		p = filename + i + 1;
		pos = i + 1;
		r = mcman_chrpos(p, '/');
		if (r < 0)
			break;
		i = pos + r;
	} while (1);

	p = filename + pos;

	mcfree = 0;

	if ((flags & sceMcFileCreateDir) != 0) {
		r = mcman_findfree2(port, slot, 1); // r = s3
		if (r < 0)
			return r;
		mcfree = r;
	}

	mce = mcman_get1stcacheEntp(); // mce = s4

	mcman_getmcrtime(&mcman_dircache[2].modified);

	r = McReadDirEntry(port, slot, mcman_dircache[2].cluster, cacheDir.maxent, &fse2);
	if (r != sceMcResSucceed)
		return r;

	mcman_wmemset((void *)fse2, sizeof (McFsEntry), 0);

	strncpy(fse2->name, p, 32);

	fse2->created = mcman_dircache[2].modified;
	fse2->modified = mcman_dircache[2].modified;

	Mc1stCacheEntSetWrFlagOff();

	mcman_addcacheentry(mce);

	if (!PS1CardFlag)
		flags &= 0xffffdfff;

	if ((flags & sceMcFileCreateDir) != 0) {

		fse2->mode = ((flags & sceMcFileAttrHidden) | sceMcFileAttrReadable | sceMcFileAttrWriteable \
				| sceMcFileAttrExecutable | sceMcFileAttrSubdir | sceMcFile0400 | sceMcFileAttrExists) // 0x8427
					| (flags & (sceMcFileAttrPS1 | sceMcFileAttrPDAExec));
		fse2->length = 2;
		fse2->cluster = mcfree;

		r = McCreateDirentry(port, slot, mcman_dircache[2].cluster, cacheDir.maxent, mcfree, (sceMcStDateTime *)&fse2->created);
		if (r != sceMcResSucceed)
			return -46;

		r = McReadDirEntry(port, slot, fh->field_28, fh->field_2C, &fse1);
		if (r != sceMcResSucceed)
			return r;

		pfsentry = (u8 *)fse1;
		pcache = (u8 *)&mcman_dircache[2];
		pfseend = (u8 *)(pfsentry + sizeof(McFsEntry));

		do {
			*((u32 *)pfsentry  ) = *((u32 *)pcache  );
			*((u32 *)pfsentry+1) = *((u32 *)pcache+1);
			*((u32 *)pfsentry+2) = *((u32 *)pcache+2);
			*((u32 *)pfsentry+3) = *((u32 *)pcache+3);
			pfsentry += 16;
			pcache += 16;
		} while (pfsentry < pfseend);

		Mc1stCacheEntSetWrFlagOff();

		r = McFlushCache(port, slot);
		if (r != sceMcResSucceed)
			return r;

		return sceMcResSucceed;
	}
	else {
		fse2->mode = ((flags & sceMcFileAttrHidden) | sceMcFileAttrReadable | sceMcFileAttrWriteable \
				| sceMcFileAttrExecutable | sceMcFileAttrFile | sceMcFile0400 | sceMcFileAttrExists) // 0x8417
					| (flags & (sceMcFileAttrPS1 | sceMcFileAttrPDAExec));

		fse2->cluster = -1;
		fh->field_20 = mcman_dircache[2].cluster;
		fh->status = 1;
		fh->field_24 = cacheDir.maxent;

		r = McReadDirEntry(port, slot, fh->field_28, fh->field_2C, &fse1);
		if (r != sceMcResSucceed)
			return r;

		pfsentry = (u8 *)fse1;
		pcache = (u8 *)&mcman_dircache[2];
		pfseend = (u8 *)(pfsentry + sizeof(McFsEntry));

		do {
			*((u32 *)pfsentry  ) = *((u32 *)pcache  );
			*((u32 *)pfsentry+1) = *((u32 *)pcache+1);
			*((u32 *)pfsentry+2) = *((u32 *)pcache+2);
			*((u32 *)pfsentry+3) = *((u32 *)pcache+3);
			pfsentry += 16;
			pcache += 16;
		} while (pfsentry < pfseend);

		Mc1stCacheEntSetWrFlagOff();

		r = McFlushCache(port, slot);
		if (r != sceMcResSucceed)
			return r;
	}

	return fd;
}

//--------------------------------------------------------------
int mcman_chdir(int port, int slot, const char *newdir, char *currentdir)
{
	register int r, len, len2, cluster;
	register MCDevInfo *mcdi = &mcman_devinfos[port][slot];
	McCacheDir cacheDir;
	McFsEntry *fse;

	DPRINTF("mcman_chdir port%d slot%d newdir %s\n", port, slot, newdir);

	//if (!mcman_checkpath(newdir))
	//	return sceMcResNoEntry;

	cacheDir.maxent = -1;

	r = mcman_cachedirentry(port, slot, newdir, &cacheDir, &fse, 1);
	if (r < 0)
		return r;

	if (((u32)(r - 1)) < 2)
		return sceMcResNoEntry;

	mcdi->rootdir_cluster2 = cacheDir.cluster;
	mcdi->unknown1 = cacheDir.fsindex;

	cluster = cacheDir.cluster;
	if (!strcmp(fse->name, "..")) {
		r = McReadDirEntry(port, slot, cluster, 0, &fse);
		if (r != sceMcResSucceed)
			return r;
	}

	if (!strcmp(fse->name, ".")) {
		mcdi->rootdir_cluster2 = fse->cluster;
		mcdi->unknown1 = fse->dir_entry;

		cluster = fse->cluster;
		r = McReadDirEntry(port, slot, cluster, fse->dir_entry, &fse);
		if (r != sceMcResSucceed)
			return r;
	}

	currentdir[0] = 0;

lbl1:
	if (strcmp(fse->name, ".")) {

		if (strlen(fse->name) < 32)
			len = strlen(fse->name);
		else
			len = 32;

		if (strlen(currentdir)) {
			len2 = strlen(currentdir);
			if (len2 >= 0) {
				do {
					currentdir[1 + len2 + len] = currentdir[len2];
				} while (--len2 >= 0);
			}
			currentdir[len] = '/';
			strncpy(currentdir, fse->name, len);
		}
		else {
			strncpy(currentdir, fse->name, 32);
			currentdir[32] = 0;
		}

		r = McReadDirEntry(port, slot, cluster, 0, &fse);
		if (r != sceMcResSucceed)
			return r;

		r = McReadDirEntry(port, slot, fse->cluster, fse->dir_entry, &fse);

		if (r == sceMcResSucceed)
			goto lbl1;
	}
	else {
		len = strlen(currentdir);

		if (len >= 0) {
			do {
				currentdir[1 + len] = currentdir[len];
			} while (--len >= 0);
		}
		currentdir[0] = '/';

		r = sceMcResSucceed;
	}

	return r;
}

//--------------------------------------------------------------
int mcman_getdir2(int port, int slot, const char *dirname, int flags, int maxent, sceMcTblGetDir *info)
{
	register int r, nument;
	register MCDevInfo *mcdi = &mcman_devinfos[port][slot];
	McFsEntry *fse;
	char *p;

	DPRINTF("mcman_getdir2 port%d slot%d dir=%s flags=%d maxent=%d\n", port, slot, dirname, flags, maxent);

	nument = 0;
	flags &= 0xffff;

	if (!flags) {
		register int pos;

		p = mcman_curdirpath;
		strncpy(p, dirname, 1023);
		mcman_curdirpath[1023] = 0;

		pos = -1; 	// s1
		p++; 		//s0
		do {
			r = mcman_chrpos((void *)&p[pos], '/');
			if (r < 0)
				break;
			pos += 1 + r;
		} while (1);

		if (pos <= 0) {
			if (pos == 0)
				*p = 0;
			else
				p[-1] = 0;
		}
		else {
			mcman_curdirpath[pos] = 0;
		}

		mcman_curdirname = &dirname[pos] + 1;

		r = mcman_cachedirentry(port, slot, mcman_curdirpath, NULL, &fse, 1);
		if (r > 0)
			return sceMcResNoEntry;
		if (r < 0)
			return r;

		if (!(fse->mode & sceMcFileAttrSubdir)) {
			mcman_curdircluster = -1;
			return sceMcResNoEntry;
		}

		mcman_curdircluster = fse->cluster;
		mcman_curdirlength = fse->length;

		if ((fse->cluster == mcdi->rootdir_cluster) && (fse->dir_entry == 0))
			mcman_curdirmaxent = 2;
		else
			mcman_curdirmaxent = 0;
	}
	else {
		if (mcman_curdircluster < 0)
			return sceMcResNoEntry;
	}

	if (maxent != 0) {
		do {
			if (mcman_curdirmaxent >= mcman_curdirlength)
				break;

			r = McReadDirEntry(port, slot, mcman_curdircluster, mcman_curdirmaxent, &fse);
			if (r != sceMcResSucceed)
				return r;

			mcman_curdirmaxent++;

			if (!(fse->mode & sceMcFileAttrExists))
				continue;
			if ((fse->mode & sceMcFileAttrHidden) && (!PS1CardFlag))
				continue;
			if (!mcman_checkdirpath(fse->name, mcman_curdirname))
				continue;

			mcman_wmemset((void *)info, sizeof (sceMcTblGetDir), 0);

			if (mcman_curdirmaxent == 2) {

				r = McReadDirEntry(port, slot, mcman_curdircluster, 0, &fse);
				if (r != sceMcResSucceed)
					return r;

				r = McReadDirEntry(port, slot, fse->cluster, 0, &fse);
				if (r != sceMcResSucceed)
					return r;

				r = McReadDirEntry(port, slot, fse->cluster, fse->dir_entry, &fse);
				if (r != sceMcResSucceed)
					return r;

				info->EntryName[0] = '.';
				info->EntryName[1] = '.';
				info->EntryName[2] = '\0';
			}
			else if (mcman_curdirmaxent == 1) {

				r = McReadDirEntry(port, slot, mcman_curdircluster, 0, &fse);
				if (r != sceMcResSucceed)
					return r;

				r = McReadDirEntry(port, slot, fse->cluster, fse->dir_entry, &fse);
				if (r != sceMcResSucceed)
					return r;

				info->EntryName[0] = '.';
				info->EntryName[1] = '\0';
			}
			else {
				strncpy(info->EntryName, fse->name, 32);
			}

			info->AttrFile = fse->mode;
			info->Reserve1 = fse->unused;

			info->_Create = fse->created;
			info->_Modify = fse->modified;

			if (!(fse->mode & sceMcFileAttrSubdir))
				info->FileSizeByte = fse->length;

			nument++;
			maxent--;
			info++;

		} while (maxent);
	}

	return nument;
}

//--------------------------------------------------------------
int mcman_delete2(int port, int slot, const char *filename, int flags)
{
	register int r, i;
	McCacheDir cacheDir;
	McFsEntry *fse1, *fse2;

	DPRINTF("mcman_delete2 port%d slot%d filename %s flags %x\n",  port, slot, filename, flags);

	//if (!mcman_checkpath(filename))
	//	return sceMcResNoEntry;

	r = mcman_cachedirentry(port, slot, filename, &cacheDir, &fse1, ((u32)(flags < 1)) ? 1 : 0);
	if (r > 0)
		return sceMcResNoEntry;
	if (r < 0)
		return r;

	if (!flags) {
		if (!(fse1->mode & sceMcFileAttrExists))
			return sceMcResNoEntry;
	}
	else {
		if (fse1->mode & sceMcFileAttrExists)
			return sceMcResNoEntry;
	}

	if ((!PS1CardFlag) && (!flags)) {
		if (!(fse1->mode & sceMcFileAttrWriteable))
			return sceMcResDeniedPermit;
	}

	if ((!fse1->cluster) && (!fse1->dir_entry))
		return sceMcResNoEntry;

	i = 2;
	if ((!flags) && (fse1->mode & sceMcFileAttrSubdir) && ((u32)i < fse1->length)) {

		do {
			r = McReadDirEntry(port, slot, fse1->cluster, i, &fse2);
			if (r != sceMcResSucceed)
				return r;

			if (fse2->mode & sceMcFileAttrExists)
				return sceMcResNotEmpty;

		} while ((u32)(++i) < fse1->length);
	}

	r = McSetDirEntryState(port, slot, cacheDir.cluster, cacheDir.fsindex, flags);
	if (r != sceMcResSucceed)
		return r;

	r = McFlushCache(port, slot);
	if (r != sceMcResSucceed)
		return r;

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_unformat2(int port, int slot)
{
	register int r, i, j, z, l, pageword_cnt, page, blocks_on_card, erase_byte, err_cnt;
	register u32 erase_value;
	register MCDevInfo *mcdi = &mcman_devinfos[port][slot];

	DPRINTF("mcman_unformat2 port%d slot%d\n", port, slot);

	pageword_cnt = mcdi->pagesize >> 2;
	blocks_on_card = mcdi->clusters_per_card / mcdi->clusters_per_block; //sp18

	erase_value = 0xffffffff; //s6
	if (!(mcdi->cardflags & CF_ERASE_ZEROES))
		erase_value = 0x00000000;

	for (i = 0; i < pageword_cnt; i++)
		mcman_pagebuf.word[i] = erase_value;

	for (i = 0; i < 128; i++)
		*((u32 *)&mcman_eccdata + i) = erase_value;

	i = 1;
	if (i < blocks_on_card) {
		erase_byte = erase_value & 0xff;	// sp20
		do {
			page = i * mcdi->blocksize;
			if (mcdi->cardform > 0) {
				j = 0;
				for (j = 0; j < 16; j++) {
					if (mcdi->bad_block_list[j] <= 0) {
						j = 16;
						goto lbl1;
					}
					if (mcdi->bad_block_list[j] == i)
						goto lbl1;
				}
			}
			else {
				err_cnt = 0;
				j = -1;
				z = 0;
				if (mcdi->blocksize > 0) {
					do {
						r = McReadPage(port, slot, page + z, &mcman_pagebuf);
						if (r == sceMcResNoFormat) {
							j = -2;
							break;
						}
						if (r != sceMcResSucceed)
							return -42;

						if ((mcdi->cardflags & CF_USE_ECC) == 0)	{
							for (l = 0; l < mcdi->pagesize; l++) {
								if (mcman_pagebuf.byte[l] != erase_byte)
									err_cnt++;
								if ((u32)err_cnt >= (mcdi->clusters_per_block << 6)) {
									j = 16;
									break;
								}
							}
							if (j != -1)
								break;
						}
					} while (++z < mcdi->blocksize);
				}
			}

			if (((mcdi->cardflags & CF_USE_ECC) != 0) && (j == -1))
				j = 16;
lbl1:
			if (j == 16) {
				r = mcman_eraseblock(port, slot, i, NULL, NULL);
				if (r != sceMcResSucceed)
					return -43;
			}
			else {
				for (l = 0; l < pageword_cnt; l++)
					mcman_pagebuf.word[l] = erase_value;

				if (mcdi->blocksize > 0) {
					z = 0;
					do {
						r = McWritePage(port, slot, page + z, &mcman_pagebuf, mcman_eccdata);
						if (r != sceMcResSucceed)
							return -44;
					} while (++z < mcdi->blocksize);
				}
			}
		} while (++i < blocks_on_card);
	}

	r = mcman_eraseblock(port, slot, 0, NULL, NULL);
	if (r != sceMcResSucceed)
		return -45;

	return sceMcResSucceed;
}

//--------------------------------------------------------------
