/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <errno.h>
#include <irx_imports.h>
#include <loadcore.h>

#define MODNAME "erom_file_driver"
IRX_ID(MODNAME, 1, 1);
// Based on the module from DVD Player 3.11.

typedef struct erom_dentry_
{
	u32 m_filename_hash;
	u32 m_fileoffset_hash;
	u32 m_filesize_hash;
	u32 m_next_fileoffset_hash;
	u32 m_xordata_size_hash;
} erom_dentry_t;

typedef struct erom_info_
{
	const void *m_erom_start;
	const erom_dentry_t *m_erom_dentry_start;
} erom_info_t;

typedef struct erom_fdpriv_
{
	int m_unused0;
	int m_cur_offset;
	int m_magic8;
	void *m_file_offset;
	u32 m_file_size;
	void *m_xordata_offset;
	u32 m_xordata_size;
} erom_fdpriv_t;

static int erom_nulldev(void);
static int erom_op_close(iop_file_t *f);
static int erom_op_write(iop_file_t *f, void *ptr, int size);
static int erom_op_lseek(iop_file_t *f, int pos, int mode);
static int erom_op_open(iop_file_t *f, const char *name, int mode);
static int erom_op_read(iop_file_t *f, void *ptr, int size);
static int get_val_from_hash0(u32 obfval);
static int get_val_from_hash1(u32 obfval);
static erom_info_t *get_erom_info(const u32 *erom_start, const u32 *erom_end, erom_info_t *info);
static const erom_dentry_t *get_direntry_by_name(erom_info_t *info, const char *name);

static iop_device_ops_t erom_devops = {
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	erom_op_open,
	erom_op_close,
	erom_op_read,
	erom_op_write,
	erom_op_lseek,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
	(void *)erom_nulldev,
};

static iop_device_t erom_dev = {
	"erom",
	IOP_DT_FS,
	1u,
	"",
	&erom_devops,
};

static erom_fdpriv_t erom_fdpriv;
static erom_info_t erom_info;
static const erom_dentry_t *erom_dentry;

static int erom_nulldev(void)
{
	return 0;
}

static int erom_op_close(iop_file_t *f)
{
	(void)f;
	if ( !erom_dentry )
		return -EBADF;
	erom_dentry = NULL;
	return 0;
}

static int erom_op_write(iop_file_t *f, void *ptr, int size)
{
	(void)f;
	(void)ptr;
	(void)size;

	return -EIO;
}

static int erom_op_lseek(iop_file_t *f, int pos, int mode)
{
	erom_fdpriv_t *privdata;
	int offs_relative;
	u32 pos_plus_curpos;

	privdata = (erom_fdpriv_t *)f->privdata;
	switch ( mode )
	{
		case FIO_SEEK_SET:
			offs_relative = 0;
			break;
		case FIO_SEEK_CUR:
			offs_relative = privdata->m_cur_offset;
			break;
		case FIO_SEEK_END:
			offs_relative = privdata->m_file_size;
			break;
		default:
			return -EINVAL;
	}
	pos_plus_curpos = pos + offs_relative;
	privdata->m_cur_offset = (privdata->m_file_size >= pos_plus_curpos) ? pos_plus_curpos : privdata->m_file_size;
	return privdata->m_cur_offset;
}

int _start(int ac, char **av)
{
	int dev1_address;
	int dev1_delay;
	int has_erom;

	(void)ac;
	(void)av;

	dev1_address = *((vu32 *)0xBF801400);
	dev1_delay = *((vu32 *)0xBF80100C);
	*((vu32 *)0xBF801400) = 0xBE000000;
	*((vu32 *)0xBF80100C) = 0x18344F;
	has_erom = get_erom_info((u32 *)0xBE080000, (u32 *)0xBE080400, &erom_info) != 0;
	if ( !has_erom )
	{
		*((vu32 *)0xBF80100C) = 0x18244F;
		has_erom = get_erom_info((u32 *)0xBE080000, (u32 *)0xBE080400, &erom_info) != 0;
	}
	if ( !has_erom )
	{
		*((vu32 *)0xBF80100C) = dev1_delay;
		*((vu32 *)0xBF801400) = dev1_address;
		return MODULE_NO_RESIDENT_END;
	}
	DelDrv(erom_dev.name);
	return (AddDrv(&erom_dev) < 0) ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END;
}

static int erom_op_open(iop_file_t *f, const char *name, int mode)
{
	int state;

	(void)mode;
	if ( f->unit || !erom_info.m_erom_start )
		return -ENXIO;
	while ( *name == '@' || *name == '&' )
	{
		erom_fdpriv.m_magic8 ^= 1u;
		name += 1;
	}
	CpuSuspendIntr(&state);
	if ( erom_dentry != NULL )
	{
		CpuResumeIntr(state);
		return -ENOMEM;
	}
	erom_dentry = get_direntry_by_name(&erom_info, name);
	CpuResumeIntr(state);
	if ( erom_dentry == NULL )
	{
		return -ENOENT;
	}
	erom_fdpriv.m_unused0 = 0;
	erom_fdpriv.m_cur_offset = 0;
	erom_fdpriv.m_file_offset = (u8 *)erom_info.m_erom_start + get_val_from_hash0(erom_dentry->m_fileoffset_hash);
	erom_fdpriv.m_xordata_size = 4 * get_val_from_hash0(erom_dentry->m_xordata_size_hash);
	erom_fdpriv.m_xordata_offset = (u8 *)erom_fdpriv.m_file_offset - erom_fdpriv.m_xordata_size;
	erom_fdpriv.m_file_size = get_val_from_hash1(erom_dentry->m_filesize_hash) & 0xFFFFFF;
	f->privdata = &erom_fdpriv;
	return 0;
}

static int erom_op_read(iop_file_t *f, void *ptr, int size)
{
	int size_tmp;
	erom_fdpriv_t *privdata;
	int filesize_tmp;
	int m_cur_offset;
	int m_file_size;
	int m_xordata_size;
	u8 *dstptr;

	dstptr = (u8 *)ptr;
	privdata = (erom_fdpriv_t *)f->privdata;
	if ( size < 0 )
		return -EINVAL;
	m_cur_offset = privdata->m_cur_offset;
	m_file_size = privdata->m_file_size;
	size_tmp = (m_file_size - m_cur_offset < size) ? m_file_size - m_cur_offset : size;
	if ( size_tmp <= 0 )
	{
		return size_tmp;
	}
	m_xordata_size = privdata->m_xordata_size;
	filesize_tmp = m_xordata_size - m_cur_offset;
	if ( filesize_tmp > 0 )
	{
		u8 *srcptr;
		u8 *srcptr_end;

		if ( filesize_tmp > size_tmp )
			filesize_tmp = size_tmp;
		memcpy(dstptr, (u8 *)privdata->m_xordata_offset + m_cur_offset, filesize_tmp);
		srcptr = (u8 *)privdata->m_file_offset + privdata->m_cur_offset;
		srcptr_end = srcptr + filesize_tmp;
		if ( ((uiptr)dstptr & 3) == ((uiptr)srcptr & 3) )
		{
			while ( ((uiptr)srcptr & 3) != 0 )
			{
				*dstptr ^= *srcptr;
				dstptr += 1;
				srcptr += 1;
			}
			while ( (uiptr)srcptr < ((uiptr)srcptr_end & (~3)) )
			{
				*(u32 *)dstptr ^= *(u32 *)srcptr;
				srcptr += 4;
				dstptr += 4;
			}
		}
		while ( srcptr < srcptr_end )
		{
			*dstptr ^= *(u8 *)srcptr;
			dstptr += 1;
			srcptr += 1;
		}
		size_tmp -= filesize_tmp;
		privdata->m_cur_offset += filesize_tmp;
	}
	if ( size_tmp > 0 )
	{
		memcpy(dstptr, (u8 *)privdata->m_file_offset + privdata->m_cur_offset, size_tmp);
		privdata->m_cur_offset += size_tmp;
	}
	return size_tmp + filesize_tmp;
}

static u32 get_string_hash(const char *name)
{
	int ret;
	int i;
	char nametmp[6];

	ret = 0;
	strncpy(nametmp, name, sizeof(nametmp));
	for ( i = 0; i < (int)(sizeof(nametmp)); i += 1 )
	{
		int tmpval1;
		int tmpval2;

		tmpval1 = (u8)nametmp[i];
		tmpval2 = tmpval1 - 64;
		if ( (u32)(tmpval1 - 65) >= 0xD )
		{
			tmpval2 = 14;
			if ( tmpval1 )
			{
				tmpval2 = (u8)tmpval1 - 63;
				if ( (u8)tmpval1 < 0x4Eu )
				{
					tmpval2 = 28;
					if ( (u8)tmpval1 != 32 )
						tmpval2 = (u8)tmpval1 - 19;
				}
			}
		}
		ret = 40 * ret + tmpval2;
	}
	return ret;
}

static int get_val_from_hash0(u32 obfval)
{
	int ret;
	int i;

	ret = 0;
	for ( i = 0; i < 32; i += 4 )
	{
		u32 tmpval1;
		int tmpval2;

		tmpval1 = obfval % 0x12;
		tmpval2 = (tmpval1 - 1) << i;
		if ( tmpval1 - 1 >= 0xA )
			tmpval2 = (tmpval1 - 2) << i;
		ret |= tmpval2;
		obfval /= 0x12u;
	}
	return ret;
}

static int get_val_from_hash1(u32 obfval)
{
	int ret;
	int i;

	ret = 0;
	for ( i = 0; i < 28; i += 4 )
	{
		u32 tmpval1;

		tmpval1 = obfval % 0x13 + 9;
		if ( obfval % 0x13 - 1 >= 6 )
			tmpval1 = obfval % 0x13 - 8;
		ret |= tmpval1 << i;
		obfval /= 0x13u;
	}
	return ret;
}

static erom_info_t *get_erom_info(const u32 *erom_start, const u32 *erom_end, erom_info_t *info)
{
	int bufcount;
	u32 *buf;
	char c;
	int i;
	char tmpname[6];

	bufcount = erom_end - erom_start;
	memcpy(tmpname, " @ A B", 6);
	info->m_erom_start = 0;
	buf = (u32 *)AllocSysMemory(ALLOC_FIRST, bufcount * sizeof(u32), NULL);
	if ( buf == NULL )
		return NULL;
	memcpy(buf, erom_start, bufcount * sizeof(u32));
	for ( c = 'C'; (u8)c < '['; c += 1 )
	{
		u32 string_hash;

		tmpname[1] = tmpname[3];
		tmpname[3] = tmpname[5];
		tmpname[5] = c;
		string_hash = get_string_hash(tmpname);
		for ( i = 0; (i < bufcount) && (buf[i] != string_hash); i += 1 )
		{
		}
		if ( i < bufcount )
		{
			info->m_erom_start = erom_start;
			info->m_erom_dentry_start = (const erom_dentry_t *)&erom_start[i];
			FreeSysMemory(buf);
			return info;
		}
	}
	FreeSysMemory(buf);
	return NULL;
}

const erom_dentry_t *get_direntry_by_name(erom_info_t *info, const char *name)
{
	const erom_dentry_t *res;
	u32 string_hash;

	res = info->m_erom_dentry_start;
	string_hash = get_string_hash(name);
	while ( res && res->m_filename_hash != string_hash )
	{
		int val_from_hash0;

		val_from_hash0 = get_val_from_hash0(res->m_next_fileoffset_hash);
		res = val_from_hash0 ? (const erom_dentry_t *)((u8 *)info->m_erom_start + val_from_hash0) : NULL;
	}
	return res;
}
