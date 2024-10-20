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

#define MODNAME "subfile_driver"
IRX_ID(MODNAME, 2, 1);
// Based on the module from PBPX-95216

static int subfile_op_nulldev(void);
static int subfile_op_open(iop_file_t *f, const char *name, int mode);
static int subfile_op_close(iop_file_t *f);
static int subfile_op_read(iop_file_t *f, void *ptr, int size);
static int subfile_op_lseek(iop_file_t *f, int pos, int mode);

typedef struct subfile_priv_fd_
{
	int m_fd;
	int m_baseoffset;
	int m_totalsize;
	int m_curpos;
} subfile_priv_fd_t;

static iop_device_ops_t subfile_devops = {
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	subfile_op_open,
	subfile_op_close,
	subfile_op_read,
	(void *)subfile_op_nulldev,
	subfile_op_lseek,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
	(void *)subfile_op_nulldev,
};
static iop_device_t subfile_dev = {
	"subfile",
	IOP_DT_FS,
	1u,
	"SubFile",
	&subfile_devops,
};
static subfile_priv_fd_t subfile_info[8];
static void *tmpbuf;

int _start(int ac, char *av[])
{
	int i;
	int state;

	(void)ac;
	(void)av;
	memset(subfile_info, 0, sizeof(subfile_info));
	for ( i = 0; i < (int)(sizeof(subfile_info) / sizeof(subfile_info[0])); i += 1 )
	{
		subfile_info[i].m_fd = -1;
	}
	CpuSuspendIntr(&state);
	tmpbuf = AllocSysMemory(ALLOC_LAST, 0x4000, NULL);
	CpuResumeIntr(state);
	if ( tmpbuf == NULL )
		return MODULE_NO_RESIDENT_END;
	return (AddDrv(&subfile_dev) < 0) ? MODULE_NO_RESIDENT_END : MODULE_RESIDENT_END;
}

static int subfile_op_open(iop_file_t *f, const char *name, int mode)
{
	int cur_baseoffset;
	int cur_totalsize;
	int i;
	int arrind;
	char namechr_int;
	int cur_fd;
	char curfilename[128];
	int state;

	(void)mode;
	cur_baseoffset = 0;
	cur_totalsize = 0;
	curfilename[0] = '\x00';
	arrind = 0;
	for ( i = 0; name[i]; ++i )
	{
		switch ( arrind )
		{
			case 0:
			{
				curfilename[i] = (name[i] != ',') ? name[i] : '\x00';
				if ( curfilename[i] == '\x00' )
				{
					arrind += 1;
				}
				break;
			}
			case 1:
			case 2:
			default:
			{
				if ( name[i] == ',' )
				{
					arrind += 1;
					break;
				}
				if ( name[i] < '0' )
					return -ENOENT;
				if ( name[i] > '9' )
				{
					if ( name[i] < 'A' || name[i] > 'F' )
						return -ENOENT;
					namechr_int = name[i] - '7';
				}
				else
				{
					namechr_int = name[i] - '0';
				}
				if ( arrind == 1 )
					cur_baseoffset = 16 * cur_baseoffset + namechr_int;
				else
					cur_totalsize = 16 * cur_totalsize + namechr_int;
				break;
			}
		}
	}
	cur_fd = open(curfilename, FIO_O_RDONLY);
	if ( cur_fd < 0 )
	{
		return cur_fd;
	}
	CpuSuspendIntr(&state);
	for ( i = 0; i < (int)(sizeof(subfile_info) / sizeof(subfile_info[0])) && subfile_info[i].m_fd >= 0; i += 1 )
	{
	}
	if ( i >= (int)(sizeof(subfile_info) / sizeof(subfile_info[0])) )
	{
		CpuResumeIntr(state);
		close(cur_fd);
		return -ENOMEM;
	}
	subfile_info[i].m_fd = cur_fd;
	subfile_info[i].m_baseoffset = cur_baseoffset;
	subfile_info[i].m_totalsize = cur_totalsize;
	subfile_info[i].m_curpos = 0;
	CpuResumeIntr(state);
	f->privdata = &subfile_info[i];
	return 0;
}

static int subfile_op_nulldev(void)
{
	return 0;
}

static int subfile_op_close(iop_file_t *f)
{
	subfile_priv_fd_t *privdata;

	privdata = (subfile_priv_fd_t *)f->privdata;
	if ( privdata == NULL )
	{
		return -EBADF;
	}
	close(privdata->m_fd);
	privdata->m_fd = -1;
	f->privdata = NULL;
	return 0;
}

static int subfile_op_read(iop_file_t *f, void *ptr, int size)
{
	subfile_priv_fd_t *privdata;
	int size_tmp;
	int i;
	int baseoffs_plus_curpos_size;

	privdata = (subfile_priv_fd_t *)f->privdata;
	if ( privdata == NULL )
	{
		return -EBADF;
	}
	if ( ((uiptr)ptr & 3) != 0 )
	{
		return -EINVAL;
	}
	for ( size_tmp = (privdata->m_totalsize - privdata->m_curpos < size) ? privdata->m_totalsize - privdata->m_curpos :
																																				 size,
				i = 0;
				size_tmp > 0;
				size_tmp -= baseoffs_plus_curpos_size, i += baseoffs_plus_curpos_size )
	{
		int baseoffs_plus_curpos;
		int baseoffs_plus_curpos_chunk;
		u32 baseoffs_plus_curpos_sub;
		int baseoffs_plus_curpos_mask;

		baseoffs_plus_curpos = privdata->m_baseoffset + privdata->m_curpos + i;
		baseoffs_plus_curpos_chunk = baseoffs_plus_curpos & ~0x1FF;
		baseoffs_plus_curpos_sub = privdata->m_baseoffset + privdata->m_totalsize - baseoffs_plus_curpos_chunk;
		baseoffs_plus_curpos_mask = baseoffs_plus_curpos_sub & 0x1FF;
		if ( (u32)(size_tmp + 0x400) < baseoffs_plus_curpos_sub )
		{
			baseoffs_plus_curpos_sub = size_tmp + 0x400;
			baseoffs_plus_curpos_mask = baseoffs_plus_curpos_sub & 0x1FF;
		}
		if ( baseoffs_plus_curpos_mask )
			baseoffs_plus_curpos_sub = (baseoffs_plus_curpos_sub + 0x1FF) & ~0x1FF;
		if ( baseoffs_plus_curpos_sub > 0x4000 )
		{
			baseoffs_plus_curpos_sub = 0x4000;
		}
		baseoffs_plus_curpos_size = (baseoffs_plus_curpos_chunk + baseoffs_plus_curpos_sub) - baseoffs_plus_curpos;
		if ( size_tmp < baseoffs_plus_curpos_size )
			baseoffs_plus_curpos_size = size_tmp;
		lseek(privdata->m_fd, baseoffs_plus_curpos_chunk, FIO_SEEK_SET);
		read(privdata->m_fd, tmpbuf, baseoffs_plus_curpos_sub);
		memcpy((u8 *)ptr + i, (u8 *)tmpbuf + (baseoffs_plus_curpos & 0x1FF), baseoffs_plus_curpos_size);
	}
	privdata->m_curpos += i;
	return i;
}

static int subfile_op_lseek(iop_file_t *f, int pos, int mode)
{
	subfile_priv_fd_t *privdata;
	int offs_relative;
	u32 m_totalsize;
	int pos_plus_curpos;

	privdata = (subfile_priv_fd_t *)f->privdata;
	if ( privdata == NULL )
		return -EBADF;
	switch ( mode )
	{
		case FIO_SEEK_SET:
			offs_relative = 0;
			break;
		case FIO_SEEK_CUR:
			offs_relative = privdata->m_curpos;
			break;
		case FIO_SEEK_END:
			offs_relative = privdata->m_totalsize;
			break;
		default:
			return -EINVAL;
	}
	m_totalsize = privdata->m_totalsize;
	pos_plus_curpos = pos + offs_relative;
	if ( (int)m_totalsize >= pos_plus_curpos )
	{
		m_totalsize = 0;
		if ( pos_plus_curpos >= 0 )
			m_totalsize = pos_plus_curpos;
	}
	privdata->m_curpos = m_totalsize;
	return m_totalsize;
}
