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

static int mcman_PS1curcluster;
static char mcman_PS1curdir[64];

//--------------------------------------------------------------
int mcman_format1(int port, int slot)
{
	register int r, i;
	register MCDevInfo *mcdi = &mcman_devinfos[port][slot];

	DPRINTF("mcman_format1 port%d slot%d\n", port, slot);

	mcman_invhandles(port, slot);

	mcdi->cardform = -1;

	for (i = 0; i < 56; i++) {
		r = mcman_PS1pagetest(port, slot, i);
		if (r == 0)
			return sceMcResNoFormat;
		if (r != 1)
			return -41;
	}

	memset(&mcman_PS1PDApagebuf, 0, 128);

	mcman_PS1PDApagebuf.word[0] = 0xa0;
	mcman_PS1PDApagebuf.half[2] = 0xffff;

	mcman_PS1PDApagebuf.byte[127] = mcman_calcEDC(&mcman_PS1PDApagebuf, 127);

	for (i = 1; i < 15; i++) {
		r = McWritePS1PDACard(port, slot, i, &mcman_PS1PDApagebuf);
		if (r != sceMcResSucceed)
			return -42;
	}

	memset(&mcman_PS1PDApagebuf, 0, 128);

	mcman_PS1PDApagebuf.word[0] = 0xffffffff;

	for (i = 0; i < 20; i++) {
		mcman_PS1PDApagebuf.word[0] = mcdi->bad_block_list[i];
		mcman_PS1PDApagebuf.byte[127] = mcman_calcEDC(&mcman_PS1PDApagebuf, 127);

		r = McWritePS1PDACard(port, slot, i + 16, &mcman_PS1PDApagebuf);
		if (r != sceMcResSucceed)
			return -43;
	}

	mcman_wmemset(&mcman_PS1PDApagebuf, 128, 0);

	mcman_PS1PDApagebuf.byte[0] = 0x4d;
	mcman_PS1PDApagebuf.byte[1] = 0x43;
	mcman_PS1PDApagebuf.byte[127] = 0x0e;

	r = McWritePS1PDACard(port, slot, 0, &mcman_PS1PDApagebuf);
	if (r != sceMcResSucceed)
		return -44;

	mcdi->cardform = 1;

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_open1(int port, int slot, const char *filename, int flags)
{
	register int r, i, fd = 0, cluster, temp;
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	McFsEntryPS1 *fse; //sp18
	McCacheEntry *mce;
	const char *p = filename;

	DPRINTF("mcman_open1 port%d slot%d filename %s flags %x\n", port, slot, filename, flags);

	if ((flags & sceMcFileCreateFile) != 0)
		flags |= sceMcFileAttrWriteable;

	for (fd = 0; fd < MAX_FDHANDLES; fd++) {
		fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
		if (fh->status == 0)
			break;
	}

	if (fd == MAX_FDHANDLES)
		return -7;

	mcman_wmemset((void *)fh, sizeof (MC_FHANDLE), 0);

	fh->port = port;
	fh->slot = slot;

	if (*p == '/')
		p++;

	if ((*p == 0) || ((*p == '.') && (p[1] == 0))) {
		fh->filesize = 15;
		fh->drdflag = 1;
		fh->status = 1;
		return fd;
	}

	r = mcman_getPS1direntry(port, slot, p, &fse, 1);
	if ((fse->mode & 0xf0) == 0xa0)
		r = -4;

	fh->freeclink = r;

	if (r >= 0) {
		if (fse->field_7d == 1)
			fh->filesize = fse->field_38;
		else
			fh->filesize = fse->length;
	}
	else
		fh->filesize = 0;

	fh->rdflag = flags & sceMcFileAttrReadable;
	fh->wrflag = flags & sceMcFileAttrWriteable;
	fh->unknown1 = 0;

	if ((flags & sceMcFileCreateFile) == 0) {
		if (r < 0)
			return sceMcResNoEntry;

		fh->status = 1;
		return fd;
	}

	for (i = 0; i < MAX_FDHANDLES; i++) {
		if ((mcman_fdhandles[i].status != 0) && \
			(mcman_fdhandles[i].port == port) && (mcman_fdhandles[i].slot == slot) && \
			 	(mcman_fdhandles[i].freeclink == (u32)r) && (fh->wrflag != 0))
			return sceMcResDeniedPermit;
	}

	if (r >= 0) {
		r = mcman_clearPS1direntry(port, slot, r, 0);
		if (r != sceMcResSucceed)
			return r;
	}

	cluster = -1;
	for (i = 0; i < 15; i++) {
		r = mcman_readdirentryPS1(port, slot, i, &fse);
		if (r != sceMcResSucceed)
			return r;

		if ((fse->mode & 0xf0) == 0xa0) {
			if (cluster < 0)
				cluster = i;

			if (fse->mode != 0xa0) {

				mcman_wmemset((void *)fse, sizeof(McFsEntryPS1), 0);
				fse->mode = 0xa0;
				fse->linked_block = -1;
				fse->edc = mcman_calcEDC((void *)fse, 127);

				mce = mcman_get1stcacheEntp();

#if 0
				// This condition is always false because "i" starts at 0 and increments.
				if ((i + 1) < 0)
				{
					temp = i + 8;
				}
				else
#endif
				{
					temp = i + 1;
				}

				temp &= ~0x00000007;
				temp = (i + 1) - temp;
#if 0
				// This condition is always false due to the preceding set of assignments on "temp" variable.
				if (temp < 0)
					temp = 0;
#endif

				mce->wr_flag |= 1 << temp;
			}
		}
	}

	r = McFlushCache(port, slot);
	if (r != sceMcResSucceed)
		return r;

	if (cluster < 0)
		return sceMcResFullDevice;

	r = mcman_readdirentryPS1(port, slot, cluster, &fse);
	if (r != sceMcResSucceed)
		return r;

	mcman_wmemset((void *)fse, sizeof(McFsEntryPS1), 0);

	fse->mode = 0x51;
	fse->length = 0;
	fse->linked_block = -1;

	strncpy(fse->name, p, 20);

	if ((flags & sceMcFileAttrPDAExec) != 0)
		fse->field_7e = 1;

	fse->edc = mcman_calcEDC((void *)fse, 127);
	fh->freeclink = cluster;

	mce = mcman_get1stcacheEntp();

#if 0
	// This condition is always false due to the preceding check on "cluster" variable.
	if ((cluster + 1) < 0)
	{
		temp = cluster + 8;
	}
	else
#endif
	{
		temp = cluster + 1;
	}

	temp &= ~0x00000007;
	temp = (cluster + 1) - temp;
	if (temp < 0)
		temp = 0;

	mce->wr_flag |= 1 << temp;

	r = McFlushCache(port, slot);
	if (r != sceMcResSucceed)
		return r;

	fh->unknown2 = 1;
	fh->status = 1;

	return fd;
}

//--------------------------------------------------------------
int mcman_read1(int fd, void *buffer, int nbyte)
{
	register int rpos;
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	McCacheEntry *mce;
	u8 *p = (u8 *)buffer;

	if (fh->position >= fh->filesize)
		return 0;

	if ((u32)nbyte >= (fh->filesize - fh->position))
		nbyte = fh->filesize - fh->position;

	rpos = 0;
	if (nbyte) {
		do {
			register int r, size, temp, offset, maxsize;

			if ((int)(fh->position) < 0)
				temp = fh->position + 0x3ff;
			else
				temp = fh->position;

			offset = (fh->position - (temp & ~0x000003ff));
			maxsize = MCMAN_CLUSTERSIZE - offset;
			if (maxsize < nbyte)
				size = maxsize;
			else
				size = nbyte;

			r = mcman_fatRseekPS1(fd);
			if (r < 0)
				return r;

			r = mcman_readclusterPS1(fh->port, fh->slot, r, &mce);
			if (r != sceMcResSucceed)
				return r;

			memcpy(&p[rpos], (void *)(mce->cl_data + offset), size);

			rpos += size;
			nbyte -= size;
			fh->position += size;

		} while (nbyte);
	}

	return rpos;
}

//--------------------------------------------------------------
int mcman_write1(int fd, void *buffer, int nbyte)
{
	register int r, wpos;
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	McCacheEntry *mce;
	u8 *p = (u8 *)buffer;

	if (nbyte) {
		if (fh->unknown2 == 0) {
			fh->unknown2 = 1;
			r = mcman_close1(fd);
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
			register int size, temp, offset, maxsize;

			r = mcman_fatRseekPS1(fd);

			if (r == sceMcResFullDevice) {
				r = mcman_fatWseekPS1(fd);
				if (r == sceMcResFullDevice)
					return r;
				if (r!= sceMcResSucceed)
					return r;

				r = mcman_fatRseekPS1(fd);
			}
			else {
				if (r < 0)
					return r;
			}

			r = mcman_readclusterPS1(fh->port, fh->slot, r, &mce);
			if (r != sceMcResSucceed)
				return r;

			if ((int)(fh->position) < 0)
				temp = fh->position + 0x3ff;
			else
				temp = fh->position;

			offset = fh->position - (temp & ~0x000003ff);
			maxsize = MCMAN_CLUSTERSIZE - offset;
			if (maxsize < nbyte)
				size = maxsize;
			else
				size = nbyte;

			memcpy((void *)(mce->cl_data + offset), &p[wpos], size);

			mce->wr_flag = -1;
			fh->position += size;
			if (fh->position >= fh->filesize)
				fh->filesize = fh->position;

			nbyte -= size;
			wpos += size;

		} while (nbyte);

	}

	mcman_close1(fd);

	return wpos;
}

//--------------------------------------------------------------
int mcman_dread1(int fd, MC_IO_DRE_T *dirent)
{
	register MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	McFsEntryPS1 *fse;

	if (fh->position < fh->filesize) {
		do {
			register int r;

			r = mcman_readdirentryPS1(fh->port, fh->slot, fh->position, &fse);
			if (r != sceMcResSucceed)
				return r;

			if (fse->mode == 0x51)
				break;

		} while (++fh->position < fh->filesize);
	}

	if (fh->position >= fh->filesize)
		return 0;

	fh->position++;
	mcman_wmemset((void *)dirent, sizeof(MC_IO_DRE_T), 0);

	strncpy(dirent->name, fse->name, 20);
	dirent->name[20] = 0;

	dirent->stat.mode = MC_IO_S_RD | MC_IO_S_WR | MC_IO_S_EX | MC_IO_S_FL;

#if !MCMAN_ENABLE_EXTENDED_DEV_OPS
	dirent->stat.mode |= sceMcFileAttrDupProhibit | sceMcFileAttrPS1;

	if (fse->field_7e == 1)
		dirent->stat.mode |= sceMcFileAttrPDAExec;
#endif

	if (fse->field_7d == 1) {
		memcpy(dirent->stat.ctime, &fse->created, sizeof(sceMcStDateTime));
		memcpy(dirent->stat.mtime, &fse->modified, sizeof(sceMcStDateTime));
		dirent->stat.size = fse->field_38;
		dirent->stat.attr = fse->field_28;
	}
	else {
		dirent->stat.size = fse->length;
	}

	return 1;
}

//--------------------------------------------------------------
int mcman_getstat1(int port, int slot, const char *filename, MC_IO_STA_T *stat)
{
	register int r;
	McFsEntryPS1 *fse;

	DPRINTF("mcman_getstat1 port%d slot%d filename %s\n", port, slot, filename);

	r = mcman_getPS1direntry(port, slot, filename, &fse, 1);

	if ((fse->mode & 0xf0) == 0xa0)
		r = -4;

	if (r < 0)
		return sceMcResNoEntry;

	mcman_wmemset(stat, sizeof(MC_IO_STA_T), 0);

	stat->mode = MC_IO_S_RD | MC_IO_S_WR | MC_IO_S_EX | MC_IO_S_FL;

#if !MCMAN_ENABLE_EXTENDED_DEV_OPS
	stat->mode |= sceMcFileAttrDupProhibit;
#endif

	if (fse->field_7d == 1) {

#if !MCMAN_ENABLE_EXTENDED_DEV_OPS
		if ((fse->field_2c & sceMcFileAttrClosed) != 0)
			stat->mode |= sceMcFileAttrClosed;
#endif

		memcpy(stat->ctime, &fse->created, sizeof(sceMcStDateTime));
		memcpy(stat->mtime, &fse->modified, sizeof(sceMcStDateTime));

		stat->size = fse->field_38;
		stat->attr = fse->field_28;

		return sceMcResSucceed;
	}

	stat->size = fse->length;

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_setinfo1(int port, int slot, const char *filename, sceMcTblGetDir *info, int flags)
{
	register int r, temp, ret;
#ifdef BUILDING_XMCMAN
	McFsEntryPS1 *fse1;
#endif
	McFsEntryPS1 *fse2;
	McCacheEntry *mce;

	DPRINTF("mcman_setinfo1 port%d slot%d filename %s flags %x\n", port, slot, filename, flags);

	ret = 0;
#ifdef BUILDING_XMCMAN
	if ((flags & sceMcFileAttrFile) != 0) {
		r = mcman_getPS1direntry(port, slot, info->EntryName, &fse1, 1);
		if (r < 0) {
			if (r != sceMcResNoEntry) {
				ret = r;
			}
			else {
				if ((!strcmp(".", info->EntryName)) || (!strcmp("..", info->EntryName)) || (info->EntryName[0] == 0))
					ret = sceMcResNoEntry;
			}
		}
	}
#endif

	r = mcman_getPS1direntry(port, slot, filename, &fse2, 1);

	if ((fse2->mode & 0xf0) == 0)
		r = sceMcResNoEntry;

	if (r < 0)
		return r;

	if (ret != sceMcResSucceed)
		return sceMcResNoEntry;

	mce = mcman_get1stcacheEntp();

	temp = r + 1;
#if 0
	// This condition is always false due to the preceding check on "r" variable.
	if ((r + 1) < 0)
		temp = r + 8;
#endif

	temp &= ~0x00000007;
	temp = (r + 1) - temp;
	if (temp < 0)
		temp = 0;

	mce->wr_flag |= 1 << temp;

#ifdef BUILDING_XMCMAN
	fse2->field_7d = 0;
	fse2->field_2c = 0;
	flags &= -12;
#else
	if(fse2->field_7d != 1) {
		fse2->field_7d = 1;
		fse2->field_38 = fse2->length;
	}
#endif

	if ((flags & sceMcFileAttrExecutable) != 0) {
		if ((info->AttrFile & sceMcFileAttrPDAExec) != 0)
			fse2->field_7e = 1;
		else
			fse2->field_7e = 0;
	}

	//Special fix clause for file managers (like uLaunchELF)
	//This allows writing most entries that can be read by mcGetDir
	//and without usual restrictions. This flags value should cause no conflict
	//as Sony code never uses it, and the value is changed after its use here.
	//This is primarily needed for PS2 MC backups, but must be implemented here
	//too since the 'flags' argument may be used indiscriminately for both
	if(flags == 0xFEED){
		flags = sceMcFileAttrReadable|sceMcFileAttrWriteable;
		//The changed flags value allows more entries to be copied below
	}

	if ((flags & sceMcFileAttrDupProhibit) != 0)
		fse2->field_28 = info->Reserve2;

	if ((flags & sceMcFileAttrReadable) != 0)
		fse2->created = info->_Create;

	if ((flags & sceMcFileAttrWriteable) != 0)
		fse2->modified = info->_Modify;

	if ((flags & sceMcFileAttrFile) != 0)
		strncpy(fse2->name, info->EntryName, 20);

	fse2->field_1e = 0;

	r = McFlushCache(port, slot);

	return r;
}

//--------------------------------------------------------------
int mcman_getdir1(int port, int slot, const char *dirname, int flags, int maxent, sceMcTblGetDir *info)
{
	register int r, i;
	McFsEntryPS1 *fse;

	DPRINTF("mcman_getdir1 port%d slot%d dirname %s maxent %d flags %x\n", port, slot, dirname, maxent, flags);

	flags &= 0xffff;

	i = 0;

	if (!flags) {
		const char *p;

		mcman_PS1curcluster = 0;
		p = dirname;
		do {
			r = mcman_chrpos(p, '/');
			if (r < 0)
				break;
			p += mcman_chrpos(p, '/') + 1;
		} while (1);

		strncpy(mcman_PS1curdir, p, 63);

	}

	if (maxent) {
		do {
			if (mcman_PS1curcluster >= 15)
				break;

			r = mcman_readdirentryPS1(port, slot, mcman_PS1curcluster, &fse);
			if (r != sceMcResSucceed)
				return r;

			mcman_PS1curcluster++;

			if (fse->mode != 0x51)
				continue;

			if (!mcman_checkdirpath(fse->name, mcman_PS1curdir))
				continue;

			memset((void *)info, 0, sizeof (sceMcTblGetDir));

			info->AttrFile = 0x9417;

			if (fse->field_7e == 1)
				info->AttrFile = 0x9c17; //MC_ATTR_PDAEXEC set !!!

			if (fse->field_7d == 1) {
				if ((fse->field_2c & sceMcFileAttrClosed) != 0) {
					info->AttrFile |= sceMcFileAttrClosed;
				}
				info->Reserve1 = fse->field_2e;
				info->_Create = fse->created;
				info->_Modify = fse->modified;
				info->FileSizeByte = fse->field_38;
				info->Reserve2 = fse->field_28;
			}
			else {
				info->FileSizeByte = fse->length;
			}

			strncpy(info->EntryName, fse->name, 20);
			info->EntryName[20] = 0;

			i++;
			info++;
			maxent--;
		} while (maxent);
	}
	return i;
}

//--------------------------------------------------------------
int mcman_delete1(int port, int slot, const char *filename, int flags)
{
	register int r;
	McFsEntryPS1 *fse;

	DPRINTF("mcman_delete1 port%d slot%d filename %s flags %x\n", port, slot, filename, flags);

	r = mcman_getPS1direntry(port, slot, filename, &fse, ((u32)flags < 1) ? 1 : 0);
	if (r < 0)
		return r;

	r = mcman_clearPS1direntry(port, slot, r, flags);

	return r;
}

//--------------------------------------------------------------
int mcman_close1(int fd)
{
	register int r, temp;
	MC_FHANDLE *fh = (MC_FHANDLE *)&mcman_fdhandles[fd];
	McFsEntryPS1 *fse;
	McCacheEntry *mce;

	DPRINTF("mcman_close1 fd %d\n", fd);

	r = mcman_readdirentryPS1(fh->port, fh->slot, fh->freeclink, &fse);
#ifdef BUILDING_XMCMAN
	if (r != sceMcResSucceed)
		return -31;
#endif

	mce = mcman_get1stcacheEntp();

	if ((int)(fh->freeclink + 1) < 0)
		temp = fh->freeclink + 8;
	else
		temp = fh->freeclink + 1;

	temp &= ~0x00000007;
	temp = (fh->freeclink + 1) - temp;
	if (temp < 0)
		temp = 0;

	mce->wr_flag |= 1 << temp;

	if (fh->filesize == 0) {
		fse->length = 0x2000;
	}
	else {
		if ((int)(fh->filesize - 1) < 0)
			temp = (fh->filesize + 8190) >> 13;
		else
			temp = (fh->filesize - 1) >> 13;

		temp++;
		fse->length = temp << 13;
	}

#ifdef BUILDING_XMCMAN
	fse->field_7d = 0; // <--- To preserve for XMCMAN
	fse->field_2c = 0; //
	fse->field_38 = 0; //
	memset((void *)&fse->created, 0, 8);  //
	memset((void *)&fse->modified, 0, 8); //
#else
	// MCMAN does as following
	fse->field_7d = 1;
	fse->field_38 = fh->filesize;
	mcman_getmcrtime(&fse->modified);
#endif

	fse->edc = mcman_calcEDC((void *)fse, 127);

	r = McFlushCache(fh->port, fh->slot);
	if (r != sceMcResSucceed)
		return r;

	return sceMcResSucceed;
}

//--------------------------------------------------------------
int mcman_unformat1(int port, int slot)
{
	register int r, i;
	u32 *p;

	DPRINTF("mcman_unformat1 port%d slot%d\n", port, slot);

	p = mcman_PS1PDApagebuf.word;
	for (i = 0; i < 32; i++)
		*p++ = 0;

	for (i = 0; i < 1024; i++) {
		r = McWritePS1PDACard(port, slot, i, &mcman_PS1PDApagebuf);
		if (r != sceMcResSucceed)
			return -41;
	}

	return r;
}

//--------------------------------------------------------------
