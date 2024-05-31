/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accdvd_internal.h"

static struct cdfs_softc Cdfsc;

static void cdfs_reset(struct cdfs_softc *cdfsc)
{
	cdfsc->all = 0;
	cdfsc->ptable = 0;
	cdfsc->ptnum = 0;
	cdfsc->rootlsn = 0;
	cdfsc->rootsize = 0;
	cdfsc->rootidx = -1;
	cdfsc->dcache = 0;
	cdfsc->dclsn = 0;
	cdfsc->dcsize = 0;
	cdfsc->pcache = 0;
	cdfsc->pcsize = 0;
}

static int cdfs_path_init(struct cdfs_ptable *ptable, struct iso9660_path *path, unsigned int size)
{
	int count;

	for ( count = 0; size; ++count )
	{
		unsigned int v4;

		v4 = ((path->name_len[0] | (path->name_len[1] << 8)) + 1) & 0xFFFFFFFE;
		if ( ptable )
		{
			ptable->path = path;
			ptable->size = 0;
			++ptable;
		}
		size = size - 8 - v4;
		path = (struct iso9660_path *)((char *)path + v4 + 8);
	}
	return count;
}

static void cdfs_ready_done(struct acd *acd, const struct cdfs_softc *arg, int ret)
{
	int flg;

	flg = 0;
	if ( ret >= -255 )
	{
		flg = 1;
	}
	else
	{
		int asc;
		int delay;

		asc = (acUint16) - (ret & 0xFFFF);
		if ( -ret >> 16 == 6 )
		{
			delay = 0;
			if ( asc == 10496 )
				delay = acd_delay();
		}
		else
		{
			delay = -1;
			if ( asc == 1025 )
				delay = 1000000;
		}
		if ( delay < 0 )
			flg = 1;
		else
		{
			if ( delay > 0 )
				DelayThread(delay);
			if ( acd_ready(acd) < 0 )
			{
				flg = 1;
			}
		}
	}
	if ( flg )
	{
		if ( arg->semid > 0 )
			SignalSema(arg->semid);
	}
}

static void cdfs_detach(struct cdfs_softc *cdfsc)
{
	void *ptr;
	void *ptr_v1;
	void *ptr_v2;

	ptr = cdfsc->ptable;
	if ( ptr )
		FreeSysMemory(ptr);
	ptr_v1 = cdfsc->pcache;
	if ( ptr_v1 )
		FreeSysMemory(ptr_v1);
	ptr_v2 = cdfsc->dcache;
	if ( ptr_v2 )
		FreeSysMemory(ptr_v2);
	cdfs_reset(cdfsc);
}

static int cdfs_attach(struct cdfs_softc *cdfsc, struct acd *acd, int remount)
{
	int semid;
	int active;
	int active_v3;
	int active_v6;
	int ret;
	struct iso9660_path *pcache_v17;
	struct cdfs_ptable *ptable_v19;

	semid = cdfsc->semid;
	if ( semid <= 0 )
		active = -9;
	else
		active = WaitSema(semid);
	if ( active >= 0 )
	{
		acd_setup((struct acd *)cdfsc->buf, (acd_done_t)cdfs_ready_done, cdfsc, -5000000);
		if ( acd_ready((struct acd *)cdfsc->buf) < 0 )
		{
			if ( cdfsc->semid > 0 )
				SignalSema(cdfsc->semid);
		}
	}
	active_v3 = -9;
	if ( cdfsc->semid > 0 )
		active_v3 = WaitSema(cdfsc->semid);
	if ( active_v3 < 0 )
	{
		return -6;
	}
	active_v6 = cdfsc->all;
	if ( remount )
	{
		if ( active_v6 != 0 )
		{
			cdfs_detach(cdfsc);
			active_v6 = 0;
		}
	}
	ret = -16;
	if ( active_v6 == 0 )
	{
		acUint8 *buf;
		acd_lsn_t v17;

		buf = cdfsc->buf;
		v17 = 16;
		while ( 1 )
		{
			int active_v12;

			FlushDcache();
			active_v12 = acd_read(acd, v17, buf, 1);
			ret = active_v12;
			if ( active_v12 < 0 )
			{
				printf("cdfs:super:read: error %d\n", active_v12);
				break;
			}
			if ( !strncmp((const char *)buf + 1, "CD001", 5u) )
			{
				int active_v13;

				active_v13 = *buf;
				if ( active_v13 == 255 )
				{
					printf("cdfs:%u:invalid descriptor\n", (unsigned int)v17);
					ret = -6;
					break;
				}
				if ( active_v13 == 1 )
				{
					cdfsc->all = *((acUint32 *)buf + 20);
					cdfsc->rootlsn = *(acUint32 *)(buf + 158);
					cdfsc->rootsize = *(acUint32 *)(buf + 166);
					ret = 0;
					break;
				}
			}
			++v17;
			ret = -6;
			if ( v17 >= 0x64 )
				break;
		}
		if ( !ret )
		{
			int ret_v14;
			acd_lsn_t lsn;
			int ret_v16;

			ret_v14 = *(acUint32 *)&cdfsc->buf[132];
			lsn = *(acUint32 *)&cdfsc->buf[140];
			cdfsc->pcsize = ret_v14;
			ret_v16 = (unsigned int)(ret_v14 + 2047) >> 11;
			FlushDcache();
			pcache_v17 = (struct iso9660_path *)AllocSysMemory(0, ret_v16 << 11, 0);
			cdfsc->pcache = pcache_v17;
			if ( pcache_v17 == 0 )
			{
				ret = -12;
			}
			else
			{
				ret = acd_read(acd, lsn, pcache_v17, ret_v16);
				if ( ret >= 0 )
				{
					int active_v18;

					active_v18 = cdfs_path_init(0, cdfsc->pcache, cdfsc->pcsize);
					cdfsc->ptnum = active_v18;
					if (
						(8 * active_v18 == 0)
						|| (ptable_v19 = (struct cdfs_ptable *)AllocSysMemory(0, 8 * active_v18, 0), (cdfsc->ptable = ptable_v19) != 0) )
					{
						int n;
						int i;
						struct cdfs_ptable *ptable;
						int active_v24;

						n = cdfs_path_init(cdfsc->ptable, cdfsc->pcache, cdfsc->pcsize);
						i = 0;
						ptable = cdfsc->ptable;
						active_v24 = 0;
						while ( i < n )
						{
							if ( cdfsc->rootlsn == *(acUint32 *)ptable[active_v24].path->extent )
							{
								ptable[active_v24].size = cdfsc->rootsize;
								cdfsc->rootidx = i;
								break;
							}
							i++;
							active_v24 = i;
						}
						ret = 0;
					}
					else
					{
						void *ptr;

						printf("accdvd:cdfs:path_read: malloc failed\n");
						ptr = cdfsc->pcache;
						if ( ptr )
							FreeSysMemory(ptr);
						cdfsc->pcache = 0;
						ret = -12;
					}
				}
			}
		}
	}
	if ( cdfsc->semid > 0 )
	{
		SignalSema(cdfsc->semid);
	}
	return ret;
}

int cdfs_mount()
{
	struct acd v1;

	acd_setup(&v1, 0, 0, 5000000);
	return cdfs_attach(&Cdfsc, &v1, 0);
}

int cdfs_umount()
{
	int ret;

	if ( Cdfsc.semid <= 0 )
		return -9;
	ret = WaitSema(Cdfsc.semid);
	if ( ret < 0 )
	{
		return ret;
	}
	cdfs_detach(&Cdfsc);
	if ( Cdfsc.semid > 0 )
	{
		SignalSema(Cdfsc.semid);
	}
	return 0;
}

int cdfs_recover(int ret)
{
	int v1;
	struct acd acd_data;

	v1 = -ret;
	while ( 1 )
	{
		int v3;

		if ( !(v1 >= 255 && (v1 >> 16 == 6 || (v1 & 0xFFFF) == 1025 || (v1 & 0xFFFF) == 10240)) )
			return -5;
		v3 = cdfs_attach(&Cdfsc, &acd_data, 1);
		v1 = -v3;
		if ( v3 >= 0 )
			return 1;
	}
}

int cdfs_lookup(struct cdfs_dirent *result, const char *path, int pathlen)
{
	const char *v9;
	char *dirent;
	int len;
	int idx;
	int namlen;
	struct iso9660_dirent *name_v8;
	int namlen_v9;
	int idx_v10;
	int high;
	int low;
	int ret;
	int v21;
	struct iso9660_path *path_v16;
	int v23;
	int ret_v18;
	int len_v19;
	unsigned char *s2;
	int idx_v21;
	struct iso9660_dirent *dirent_v22;
	int len_v23;
	struct iso9660_path *path_v24;
	unsigned int dcsize;
	unsigned char *pt;
	unsigned int v33;
	unsigned int lsn;
	signed int size_v29;
	int ret_v30;
	int ret_v31;
	struct iso9660_dirent *dirent_v32;
	int idx_v33;
	int v40;
	unsigned int n;
	struct iso9660_path *path_v36;
	void *dirent_v37;
	struct iso9660_dirent *dirent_v38;
	int n_v39;
	int i_v40;
	int rest;
	int v48;
	const acUint8 *len_v43;
	int idx_v44;
	unsigned char *s2_v45;
	int v52;
	int v53;
	char *path_v48;
	int v55;
	int v57;
	unsigned int t0;
	size_t d_namlen;
	struct acd acd_data;
	struct acd *acd;

	acd_setup(&acd_data, 0, 0, 5000000);
	while ( 1 )
	{
		int v6;
		int v8;

		v6 = -9;
		if ( Cdfsc.semid > 0 )
			v6 = WaitSema(Cdfsc.semid);
		if ( v6 < 0 )
			return -6;
		if ( Cdfsc.all )
		{
			int ret_v57;

			acd = &acd_data;
			ret_v57 = -2;
			v9 = path + 1;
			if ( pathlen <= 0 || *path == 92 )
			{
				len = pathlen - 1;
				idx = Cdfsc.rootidx;
				while ( 1 )
				{
					namlen = len;
					name_v8 = (struct iso9660_dirent *)v9;
					while ( len > 0 )
					{
						if ( *v9 == 92 )
							break;
						--len;
						++v9;
					}
					if ( len <= 0 )
					{
						path_v24 = (struct iso9660_path *)(8 * idx);
						dirent = 0;
						if ( namlen )
						{
							dcsize = Cdfsc.dcsize;
							pt = &path_v24->name_len[(unsigned int)Cdfsc.ptable];
							v33 = *(acUint32 *)&path_v24->extent[(unsigned int)Cdfsc.ptable + 2];
							lsn = *(acUint32 *)(*(acUint32 *)pt + 2);
							FlushDcache();
							size_v29 = 0;
							if ( lsn != Cdfsc.dclsn )
							{
								ret_v30 = acd_read(acd, lsn, Cdfsc.buf, 1);
								if ( ret_v30 < 0 )
								{
									Cdfsc.dclsn = 0;
								}
								else
								{
									ret_v31 = v33 - 1;
									if ( !v33 )
									{
										dirent_v32 = (struct iso9660_dirent *)Cdfsc.buf;
										idx_v33 = 2048;
										v33 = 0;
										while ( idx_v33 )
										{
											v40 = dirent_v32->length[0];
											if ( !dirent_v32->length[0] )
												break;
											if ( dirent_v32->name_len[0] == 1 && !dirent_v32->name[0] )
											{
												v33 = *(acUint32 *)dirent_v32->size;
												break;
											}
											idx_v33 -= v40;
											dirent_v32 = (struct iso9660_dirent *)((char *)dirent_v32 + v40);
										}
										*((acUint32 *)pt + 1) = v33;
										ret_v31 = v33 - 1;
									}
									if ( !v33 )
									{
										ret_v30 = 0;
									}
									else
									{
										n = (unsigned int)ret_v31 >> 11;
										if ( dcsize >= (unsigned int)ret_v31 >> 11 )
										{
											dirent_v37 = Cdfsc.dcache;
										}
										else
										{
											if ( Cdfsc.dcache )
												FreeSysMemory(Cdfsc.dcache);
											path_v36 = (struct iso9660_path *)AllocSysMemory(0, n << 11, 0);
											dirent_v37 = path_v36;
											Cdfsc.dcache = (struct iso9660_dirent *)path_v36;
											if ( path_v36 )
												Cdfsc.dcsize = n;
											else
												Cdfsc.dcsize = 0;
											if ( !path_v36 )
												lsn = 0;
										}
										ret_v30 = 0;
										if ( lsn )
										{
											if ( n )
											{
												ret_v30 = v33;
												if ( acd_read(acd, lsn + 1, dirent_v37, n) < 0 )
												{
													lsn = 0;
													ret_v30 = 0;
												}
											}
											else
											{
												ret_v30 = v33;
											}
										}
										Cdfsc.dclsn = lsn;
									}
								}
								size_v29 = ret_v30;
							}
							dirent_v38 = (struct iso9660_dirent *)Cdfsc.buf;
							if ( size_v29 >= 0 )
							{
								n_v39 = 1;
								for ( i_v40 = 1; i_v40 >= 0; --i_v40 )
								{
									while ( 1 )
									{
										--n_v39;
										rest = 2048;
										if ( n_v39 < 0 )
											break;
										if ( size_v29 < 2048 )
											rest = size_v29;
										size_v29 -= rest;
										while ( rest > 0 )
										{
											v48 = dirent_v38->length[0];
											if ( !v48 )
											{
												break;
											}
											if ( namlen == dirent_v38->name_len[0] )
											{
												len_v43 = (acUint8 *)name_v8;
												idx_v44 = namlen - 1;
												s2_v45 = dirent_v38->name;
												while ( idx_v44 >= 0 )
												{
													v52 = *len_v43++;
													v53 = *s2_v45 - v52;
													if ( *s2_v45++ != v52 )
														break;
													idx_v44--;
												}
												if ( idx_v44 < 0 )
												{
													v53 = 0;
												}
												rest -= v48;
												if ( !v53 )
												{
													ret_v57 = 0;
													path_v48 = (char *)dirent_v38;
													dirent = path_v48;
												}
											}
											else
											{
												rest -= dirent_v38->length[0];
											}
											if ( dirent )
											{
												break;
											}
											dirent_v38 = (struct iso9660_dirent *)((char *)dirent_v38 + v48);
										}
										if ( dirent )
										{
											break;
										}
										dirent_v38 = (struct iso9660_dirent *)((char *)dirent_v38 + rest);
									}
									if ( dirent )
									{
										break;
									}
									dirent_v38 = Cdfsc.dcache;
									v55 = size_v29 + 2047;
									if ( size_v29 + 2047 < 0 )
										v55 = size_v29 + 4094;
									n_v39 = v55 >> 11;
								}
							}
							else
							{
								ret_v57 = size_v29;
							}
						}
						if ( dirent )
						{
							result->d_lsn = *(acUint32 *)(dirent + 2);
							result->d_size = *(acUint32 *)(dirent + 10);
							result->d_vol = (dirent[28] & 0xFF) | ((dirent[29] & 0xFF) << 8);
							v57 = dirent[32];
							result->d_ftype = dirent[25];
							result->d_namlen = v57;
							t0 = v57 & 0xFF;
							result->d_name[t0] = 0;
							d_namlen = result->d_namlen;
							result->d_time.t_mon = dirent[19];
							result->d_time.t_day = dirent[20];
							result->d_time.t_hour = dirent[21];
							result->d_time.t_padding = 0;
							result->d_time.t_min = dirent[22];
							result->d_time.t_sec = dirent[23];
							result->d_time.t_year = (dirent[18] & 0xFF) + 1900;
							memcpy(result->d_name, dirent + 33, d_namlen);
							ret_v57 = 0;
						}
						break;
					}
					namlen_v9 = namlen - len;
					idx_v10 = idx;
					if ( namlen_v9 && (namlen_v9 != 1 || name_v8->length[0] != 46) )
					{
						if ( namlen_v9 == 2 && name_v8->length[0] == 46 && name_v8->ext_attr_length[0] == 46 )
						{
							idx = (Cdfsc.ptable[idx].path->parent[0] | (Cdfsc.ptable[idx].path->parent[1] << 8)) - 1;
						}
						else
						{
							high = Cdfsc.ptnum;
							low = idx;
							v21 = high;
							for ( ;; )
							{
								ret = low + high;
								if ( low >= v21 )
								{
									idx = -1;
									break;
								}
								v21 = ret / 2;
								path_v16 = Cdfsc.ptable[ret / 2].path;
								v23 = path_v16->parent[1] << 8;
								ret_v18 = (path_v16->parent[0] | v23) - 1 - idx_v10;
								if ( (path_v16->parent[0] | v23) - 1 == idx_v10 )
								{
									len_v19 = path_v16->name_len[0] | (path_v16->name_len[1] << 8);
									s2 = path_v16->name;
									if ( namlen_v9 < len_v19 )
										len_v19 = namlen_v9;
									idx_v21 = len_v19 - 1;
									dirent_v22 = name_v8;
									while ( idx_v21 >= 0 )
									{
										len_v23 = dirent_v22->length[0];
										dirent_v22 = (struct iso9660_dirent *)((char *)dirent_v22 + 1);
										ret_v18 = *s2 - len_v23;
										if ( *s2++ != len_v23 )
											break;
										idx_v21--;
									}
									if ( idx_v21 < 0 )
									{
										ret_v18 = 0;
									}
								}
								if ( ret_v18 < 0 )
								{
									low = v21 + 1;
									v21 = high;
								}
								else
								{
									high = v21;
									if ( ret_v18 <= 0 )
									{
										idx = v21;
										break;
									}
								}
							}
						}
					}
					++v9;
					if ( idx < 0 )
						break;
					--len;
				}
			}
			if ( Cdfsc.semid > 0 )
				SignalSema(Cdfsc.semid);
			return ret_v57;
		}
		if ( Cdfsc.semid > 0 )
			SignalSema(Cdfsc.semid);
		v8 = cdfs_attach(&Cdfsc, &acd_data, 1);
		if ( v8 < 0 )
			return v8;
	}
}

int cdfs_read(struct cdfs_file *file, void *buf, int size)
{
	int ret;
	unsigned int cpos;
	unsigned int f_size;
	unsigned int off;
	unsigned int npos;
	unsigned char *data;
	int rest;
	unsigned int cpos_v14;
	unsigned int npos_v15;
	unsigned int npos_v16;
	unsigned int lsn_v17;
	int len_v18;
	unsigned char *data_v33;
	int rest_v34;
	unsigned int cpos_v35;
	unsigned int npos_v36;
	int len_v37;
	unsigned int lsn_v38;
	int rest_v42;
	struct acd acd_data;

	if ( !Cdfsc.semid )
		return -6;
	acd_setup(&acd_data, 0, 0, 5000000);
	cpos = file->f_pos;
	f_size = file->f_size;
	off = cpos & 0x7FF;
	npos = cpos - (off - 2048);
	if ( (cpos & 0x7FF) == 0 )
	{
		ret = 0;
	}
	else
	{
		signed int len;
		unsigned int lsn;

		if ( cpos + size < f_size )
		{
			f_size = cpos + size;
		}
		len = 2048 - (cpos & 0x7FF);
		if ( f_size < npos )
		{
			npos = f_size;
			len = f_size - cpos;
		}
		lsn = file->f_lsn + (cpos >> 11);
		if ( len <= 0 )
		{
			ret = len;
		}
		else
		{
			int ret_v8;

			ret_v8 = -9;
			if ( Cdfsc.semid > 0 )
				ret_v8 = WaitSema(Cdfsc.semid);
			ret = -6;
			if ( ret_v8 >= 0 )
			{
				int ret_v10;

				ret_v10 = -11;
				if ( Cdfsc.all )
				{
					if ( lsn )
					{
						if ( Cdfsc.dclsn == lsn )
						{
							ret_v10 = 1;
						}
					}
					if ( ret_v10 <= 0 )
					{
						ret_v10 = acd_read(&acd_data, lsn, Cdfsc.buf, 1);
					}
				}
				if ( ret_v10 <= 0 )
				{
					len = ret_v10;
					lsn = 0;
				}
				else
				{
					memcpy(buf, &Cdfsc.buf[off], len);
					file->f_pos = npos;
				}
				Cdfsc.dclsn = lsn;
				if ( Cdfsc.semid > 0 )
					SignalSema(Cdfsc.semid);
				ret = len;
			}
		}
	}
	data = (unsigned char *)buf + ret;
	if ( ret < 0 )
		return ret;
	rest = size - ret;
	cpos_v14 = file->f_pos;
	npos_v15 = cpos_v14 + rest;
	if ( file->f_size < cpos_v14 + rest )
		npos_v15 = file->f_size;
	npos_v16 = npos_v15 - (npos_v15 & 0x7FF);
	lsn_v17 = file->f_lsn + (cpos_v14 >> 11);
	len_v18 = npos_v16 - cpos_v14;
	if ( (int)(npos_v16 - cpos_v14) < 2048 )
	{
		len_v18 = 0;
		ret = len_v18;
	}
	else
	{
		unsigned char *ptr;
		ptr = data;
		if ( ((uiptr)data & 3) == 0 )
		{
			int ret_v32;

			ret_v32 = acd_read(&acd_data, file->f_lsn + (cpos_v14 >> 11), data, len_v18 / 2048);
			if ( ret_v32 <= 0 )
				len_v18 = ret_v32;
			else
				file->f_pos = npos_v16;
			ret = len_v18;
		}
		else
		{
			signed int rest_v20;
			int flg;

			flg = 0;
			rest_v20 = npos_v16 - cpos_v14;
			while ( 1 )
			{
				int ret_v21;
				int ret_v22;

				if ( rest_v20 <= 0 )
				{
					flg = 1;
					break;
				}
				ret_v21 = -9;
				if ( Cdfsc.semid > 0 )
					ret_v21 = WaitSema(Cdfsc.semid);
				ret = -6;
				if ( ret_v21 < 0 )
					break;
				ret_v22 = -11;
				if ( Cdfsc.all )
				{
					if ( lsn_v17 )
					{
						if ( Cdfsc.dclsn == lsn_v17 )
						{
							ret_v22 = 1;
						}
					}
					if ( ret_v22 <= 0 )
					{
						ret_v22 = acd_read(&acd_data, lsn_v17, Cdfsc.buf, 1);
					}
				}
				if ( ret_v22 <= 0 )
				{
					Cdfsc.dclsn = 0;
					len_v18 = ret_v22;
					flg = 1;
					break;
				}
				memcpy(ptr, &Cdfsc, sizeof(Cdfsc));
				ptr += 2048;
				rest_v20 -= 2048;
				Cdfsc.dclsn = lsn_v17++;
				if ( Cdfsc.semid > 0 )
					SignalSema(Cdfsc.semid);
			}
			if ( flg )
			{
				ret = len_v18;
				if ( !rest_v20 )
					file->f_pos = npos_v16;
			}
		}
	}
	data_v33 = &data[ret];
	if ( ret < 0 )
		return ret;
	rest_v34 = rest - ret;
	cpos_v35 = file->f_pos;
	npos_v36 = cpos_v35 + rest_v34;
	if ( file->f_size < cpos_v35 + rest_v34 )
		npos_v36 = file->f_size;
	len_v37 = npos_v36 - cpos_v35;
	lsn_v38 = file->f_lsn + (cpos_v35 >> 11);
	ret = len_v37;
	if ( (int)(npos_v36 - cpos_v35) > 0 )
	{
		int ret_v39;

		ret_v39 = -9;
		if ( Cdfsc.semid > 0 )
			ret_v39 = WaitSema(Cdfsc.semid);
		if ( ret_v39 < 0 )
		{
			ret = -6;
		}
		else
		{
			int ret_v40;

			ret_v40 = -11;
			if ( Cdfsc.all )
			{
				if ( lsn_v38 )
				{
					if ( Cdfsc.dclsn == lsn_v38 )
					{
						ret_v40 = 1;
					}
				}
				if ( ret_v40 <= 0 )
				{
					ret_v40 = acd_read(&acd_data, lsn_v38, Cdfsc.buf, 1);
				}
			}
			if ( ret_v40 <= 0 )
			{
				len_v37 = ret_v40;
				lsn_v38 = 0;
			}
			else
			{
				memcpy(data_v33, Cdfsc.buf, len_v37);
				file->f_pos = npos_v36;
			}
			Cdfsc.dclsn = lsn_v38;
			if ( Cdfsc.semid > 0 )
				SignalSema(Cdfsc.semid);
			ret = len_v37;
		}
	}
	rest_v42 = rest_v34 - ret;
	if ( ret >= 0 )
		return size - rest_v42;
	return ret;
}

int cdfs_module_status()
{
	return Cdfsc.semid > 0;
}

int cdfs_module_start(int argc, char **argv)
{
	int ret;
	int v4;
	acInt32 v5;
	iop_sema_t param;

	(void)argc;
	(void)argv;
	ret = cdfs_module_status();
	if ( ret != 0 )
	{
		return ret;
	}
	param.attr = 0;
	param.initial = 1;
	param.max = 1;
	param.option = 0;
	v4 = CreateSema(&param);
	v5 = v4;
	if ( v4 <= 0 )
		printf("accdvd:cdfs:sem_alloc: error %d\n", v4);
	if ( v5 <= 0 )
		return -6;
	Cdfsc.semid = v5;
	cdfs_reset(&Cdfsc);
	return 0;
}

int cdfs_module_stop()
{
	int ret;
	int semid;

	ret = cdfs_module_status();
	if ( ret == 0 )
	{
		return ret;
	}
	semid = Cdfsc.semid;
	if ( Cdfsc.semid )
	{
		if ( Cdfsc.semid > 0 )
			WaitSema(Cdfsc.semid);
		cdfs_detach(&Cdfsc);
		Cdfsc.semid = 0;
		if ( semid > 0 )
			SignalSema(semid);
		DeleteSema(semid);
	}
	return 0;
}

int cdfs_module_restart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}
