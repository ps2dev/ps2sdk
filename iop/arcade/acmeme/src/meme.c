/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acmeme_internal.h"

static int meme_op_xfer(struct meme_softc *memec, struct ac_memsif_reply *rpl, const void *arg, int size);
static int meme_op_init(struct meme_softc *memec, struct ac_memsif_reply *rpl, const void *arg, int size);

static meme_ops_t ops_48[3] = {NULL, &meme_op_init, &meme_op_xfer};
static struct meme_softc Memec;

static int meme_jv_read(acMemAddr addr, void *buf, int size)
{
	return acJvRead(addr, buf, size);
}

static int meme_jv_write(acMemAddr addr, void *buf, int size)
{
	return acJvWrite(addr, buf, size);
}

static void meme_ram_done(acRamT ram, struct meme_ram *arg, int result)
{
	(void)ram;
	if ( arg )
	{
		int thid;

		thid = arg->mr_thid;
		arg->mr_thid = 0;
		arg->mr_result = result;
		if ( thid )
			WakeupThread(thid);
	}
}

static int meme_ram_read(acMemAddr addr, void *buf, int size)
{
	acRamT ret;
	int ret_v1;
	int thid;
	struct meme_ram mram_data;

	mram_data.mr_thid = GetThreadId();
	mram_data.mr_result = 0;
	ret = acRamSetup(&mram_data.mr_ram, (acRamDone)meme_ram_done, &mram_data, 1000000);
	ret_v1 = acRamRead(ret, addr, buf, size);
	if ( ret_v1 < 0 )
	{
		return ret_v1;
	}
	thid = GetThreadId();
	while ( thid == mram_data.mr_thid )
		SleepThread();
	return mram_data.mr_result;
}

static int meme_ram_write(acMemAddr addr, void *buf, int size)
{
	int ret_v1;
	int thid;
	struct meme_ram mram_data;

	mram_data.mr_thid = GetThreadId();
	mram_data.mr_result = 0;
	ret_v1 = acRamWrite(acRamSetup(&mram_data.mr_ram, (acRamDone)meme_ram_done, &mram_data, 1000000), addr, buf, size);
	if ( ret_v1 < 0 )
	{
		return ret_v1;
	}
	thid = GetThreadId();
	while ( thid == mram_data.mr_thid )
		SleepThread();
	return mram_data.mr_result;
}

int meme_sram_read(acMemAddr addr, void *buf, int size)
{
	return acSramRead(addr, buf, size);
}

int meme_sram_write(acMemAddr addr, void *buf, int size)
{
	return acSramWrite(addr, buf, size);
}

static int meme_xfer_eetodev(struct meme_softc *memec, acMemVecT mvec, meme_xfer_t xfer)
{
	int v5;
	int mv_size;
	void *buf;
	acMemAddr dst;
	acMemAddr src;
	int v11;
	acMemData mem_data;

	v5 = 0;
	mv_size = mvec->mv_size;
	buf = memec->buf;
	dst = mvec->mv_dst & 0xFFFFFFE;
	src = mvec->mv_src & 0xFFFFFFE;
	acMemSetup(&mem_data, buf, memec->size);
	for ( ; mv_size > 0; dst += v11 )
	{
		int v10;

		v10 = acMemReceive(&mem_data, src, mv_size);
		v5 = v10;
		v11 = v10;
		if ( v10 <= 0 )
			break;
		v5 = xfer(dst, buf, v10);
		src += v11;
		if ( v5 <= 0 )
			break;
		mv_size -= v11;
	}
	mvec->mv_result = mvec->mv_size - mv_size;
	return v5;
}

static int meme_xfer_ioptodev(struct meme_softc *memec, acMemVecT mvec, meme_xfer_t xfer)
{
	int ret;
	signed int mv_size;
	acMemAddr dst;
	acUint8 *src;
	int v9;

	(void)memec;
	ret = 0;
	mv_size = mvec->mv_size;
	dst = mvec->mv_dst & 0xFFFFFFE;
	for ( src = (acUint8 *)(mvec->mv_src & 0xFFFFFFE); mv_size > 0; src += v9 )
	{
		v9 = 0x20000;
		if ( mv_size <= 0x20000 )
			v9 = mv_size;
		ret = xfer(dst, src, v9);
		dst += v9;
		if ( ret <= 0 )
			break;
		mv_size -= v9;
	}
	mvec->mv_result = mvec->mv_size - mv_size;
	return ret;
}

static int meme_op_xfer(struct meme_softc *memec, struct ac_memsif_reply *rpl, const void *arg, int size)
{
	int ret;
	int v5;
	acMemVecT mvec;
	acMemAddr v8;
	meme_xfer_t xfer_src;
	acMemAddr v10;
	meme_xfer_t xfer_dst;
	int v12;
	acMemAddr mv_dst;
	int v14;
	acUint8 *dst;
	signed int mv_size;
	acMemAddr src;
	int ret_v14;
	acMemAddr dst_v15;
	acMemAddr src_v16;
	int pos;
	int bsize;
	signed int v23;
	int xlen;
	acMemT v25;
	acMemT size_v22;
	int v27;
	acMemT size_v24;
	int v29;
	int reply;
	acInt32 v31;
	acMemData mem_data;
	acMemData mem_data_v29[2];
	acMemVecT addr;
	int v36;
	int v37;
	int v40;
	acUint8 *buf;
	struct ac_memsif_xfer *argt;

	(void)size;
	argt = (struct ac_memsif_xfer *)arg;
	addr = argt->mvec;
	v36 = argt->item;
	acMemSetup(&mem_data, memec->mvec, 256);
	rpl->error = 0;
	ret = 0;
	v37 = v36;
	v40 = 0xFFFFFFE;
	v5 = v37;
	while ( v37 > 0 )
	{
		int v6;
		int buf_vectors;
		int pos_v35;

		v6 = acMemReceive(&mem_data, (acMemEEaddr)addr, 16 * v5);
		ret = v6;
		if ( v6 < 0 )
			break;
		pos_v35 = 0;
		buf_vectors = v6 >> 4;
		while ( pos_v35 < buf_vectors )
		{
			mvec = &memec->mvec[pos_v35];
			v8 = memec->mvec[pos_v35].mv_src & 0xF0000000;
			if ( v8 == 1342177280 )
			{
				xfer_src = meme_sram_read;
			}
			else if ( v8 > 0x50000000 )
			{
				xfer_src = 0;
				if ( v8 == 0x60000000 )
					xfer_src = meme_jv_read;
			}
			else
			{
				xfer_src = 0;
				if ( v8 == 0x40000000 )
					xfer_src = meme_ram_read;
			}
			v10 = memec->mvec[pos_v35].mv_dst & 0xF0000000;
			if ( v10 == 0x50000000 )
			{
				xfer_dst = meme_sram_write;
			}
			else if ( v10 > 0x50000000 )
			{
				xfer_dst = 0;
				if ( v10 == 0x60000000 )
					xfer_dst = meme_jv_write;
			}
			else
			{
				xfer_dst = 0;
				if ( v10 == 0x40000000 )
					xfer_dst = meme_ram_write;
			}
			ret = 0;
			if ( xfer_src )
			{
				v12 = -134;
				if ( xfer_dst )
				{
					mvec->mv_result = -134;
					ret = v12;
				}
			}
			else
			{
				v12 = -134;
				if ( !xfer_dst )
				{
					mvec->mv_result = -134;
					ret = v12;
				}
			}
			if ( ret >= 0 )
			{
				if ( !memec->mvec[pos_v35].mv_size )
				{
					mvec->mv_result = 0;
				}
				else
				{
					if ( xfer_src )
					{
						mv_dst = memec->mvec[pos_v35].mv_dst;
						v14 = 0;
						if ( (mv_dst & 1) != 0 )
						{
							dst = (acUint8 *)(mv_dst & v40);
							mv_size = memec->mvec[pos_v35].mv_size;
							for ( src = memec->mvec[pos_v35].mv_src & v40; mv_size > 0; src += ret_v14 )
							{
								ret_v14 = 0x20000;
								if ( mv_size <= 0x20000 )
									ret_v14 = mv_size;
								v14 = xfer_src(src, dst, ret_v14);
								dst += ret_v14;
								if ( v14 <= 0 )
									break;
								mv_size -= ret_v14;
							}
							ret = v14;
							mvec->mv_result = mvec->mv_size - mv_size;
						}
						else
						{
							dst_v15 = mv_dst & v40;
							src_v16 = memec->mvec[pos_v35].mv_src & v40;
							pos = 0;
							buf = (acUint8 *)memec->buf;
							bsize = (memec->size / 2) & 0xFFFFFFFC;
							acMemSetup(mem_data_v29, buf, bsize);
							acMemSetup(&mem_data_v29[1], &buf[bsize], bsize);
							v23 = mvec->mv_size;
							for ( ret = 0; v23 > 0; dst_v15 += v27 )
							{
								xlen = v23;
								if ( bsize < v23 )
									xlen = bsize;
								ret = xfer_src(src_v16, &buf[pos], xlen);
								if ( ret <= 0 )
									break;
								v25 = mem_data_v29;
								if ( !pos )
									v25 = &mem_data_v29[1];
								ret = acMemWait(v25, 100, 110);
								if ( ret < 0 )
									break;
								size_v22 = mem_data_v29;
								if ( pos )
									size_v22 = &mem_data_v29[1];
								v27 = acMemSend(size_v22, dst_v15, xlen, 10);
								ret = v27;
								if ( v27 <= 0 )
									break;
								pos ^= bsize;
								src_v16 += v27;
								v23 -= v27;
							}
							size_v24 = mem_data_v29;
							if ( !pos )
								size_v24 = &mem_data_v29[1];
							acMemWait(size_v24, 100, 110);
							mvec->mv_result = mvec->mv_size - v23;
						}
					}
					else
					{
						if ( (memec->mvec[pos_v35].mv_src & 1) != 0 )
						{
							ret = meme_xfer_ioptodev(memec, &memec->mvec[pos_v35], xfer_dst);
						}
						else
						{
							v12 = meme_xfer_eetodev(memec, &memec->mvec[pos_v35], xfer_dst);
							ret = v12;
						}
					}
				}
			}
			if ( ret < 0 )
				break;
			pos_v35 += 1;
		}
		if ( pos_v35 > 0 )
		{
			v29 = 16 * pos_v35;
			reply = acMemSend(&mem_data, (acMemEEaddr)addr, 16 * pos_v35, 10);
			if ( reply < 0 )
			{
				if ( ret < 0 )
					break;
				ret = reply;
			}
			else
			{
				addr = (acMemVecT)((char *)addr + v29);
				v37 -= pos_v35;
				acMemWait(&mem_data, 100, 110);
			}
		}
		if ( ret < 0 )
			break;
		v5 = v37;
	}
	v31 = ret;
	if ( v31 > 0 )
		v31 = 0;
	rpl->error = v31;
	return v36 - v37;
}

static int meme_op_init(struct meme_softc *memec, struct ac_memsif_reply *rpl, const void *arg, int size)
{
	acInt32 v4;
	void *buf_v3;
	void *oldbuf;
	const struct ac_memsif_init *argt;

	(void)rpl;
	(void)size;
	argt = (const struct ac_memsif_init *)arg;
	v4 = argt->size;
	if ( argt->start == 0 )
	{
		void *buf_v1;

		buf_v1 = memec->buf;
		memec->buf = 0;
		if ( buf_v1 )
			FreeSysMemory(buf_v1);
		return 0;
	}
	if ( v4 == 0 )
	{
		if ( memec->buf == 0 )
			return -6;
		return memec->size;
	}
	buf_v3 = AllocSysMemory(0, argt->size, 0);
	if ( buf_v3 == 0 )
	{
		return -12;
	}
	oldbuf = memec->buf;
	memec->buf = buf_v3;
	if ( oldbuf )
		FreeSysMemory(oldbuf);
	memec->size = v4;
	return v4;
}

static void *meme_request(unsigned int fno, struct meme_softc *data, int size)
{
	int v5;
	meme_ops_t op;

	data->status = 2;
	data->pkt.rpl.error = 0;
	if ( size != 16 )
	{
		v5 = -122;
	}
	else
	{
		if ( fno >= 3 || (op = ops_48[fno]) == 0 )
			v5 = -88;
		else
			v5 = op(data, &data->pkt.rpl, data, 16);
	}
	if ( v5 < 0 )
	{
		data->pkt.rpl.error = -v5;
	}
	data->pkt.rpl.result = v5;
	data->status = 1;
	return data;
}

static void meme_thread(void *arg)
{
	int thid;
	SifRpcDataQueue_t queue_data;
	SifRpcServerData_t serv_data;
	struct meme_softc *argt;

	argt = (struct meme_softc *)arg;
	thid = GetThreadId();
	sceSifSetRpcQueue(&queue_data, thid);
	sceSifRegisterRpc(&serv_data, 0x76500001, (SifRpcFunc_t)meme_request, argt, 0, 0, &queue_data);
	argt->thid = thid;
	while ( 1 )
	{
		SifRpcServerData_t *s;

		s = sceSifGetNextRequest(&queue_data);
		if ( s )
		{
			sceSifExecRequest(s);
		}
		else
		{
			SleepThread();
			if ( thid != argt->thid )
				ExitThread();
		}
	}
}

static int meme_thread_init(struct meme_softc *memec, int prio)
{
	int th;
	iop_thread_t param;

	param.attr = 0x2000000;
	param.thread = meme_thread;
	param.priority = prio;
	param.stacksize = 4096;
	param.option = 0;
	th = CreateThread(&param);
	if ( th > 0 )
		StartThread(th, memec);
	return th;
}

int acMemeModuleStatus()
{
	return Memec.status;
}

int acMemeModuleStart(int argc, char **argv)
{
	int index;
	int prio;
	char **v8;
	char *opt;
	int v10;
	const char *opt_v7;
	int value;
	int ret;
	int v14;
	char *next;

	if ( acMemeModuleStatus() == 0 )
	{
		return -16;
	}
	index = 1;
	prio = 84;
	v8 = argv + 1;
	while ( index < argc )
	{
		opt = *v8;
		if ( **v8 == 45 )
		{
			v10 = opt[1];
			opt_v7 = opt + 2;
			if ( v10 == 112 )
			{
				value = strtol(opt_v7, &next, 0);
				if ( next != opt_v7 )
					prio = value;
			}
		}
		++index;
		++v8;
	}
	memset(&Memec, 0, sizeof(Memec));
	ret = meme_thread_init(&Memec, prio);
	v14 = -6;
	if ( ret > 0 )
	{
		v14 = 0;
		Memec.thid = ret;
		Memec.buf = 0;
		Memec.size = 0;
		Memec.status = 1;
	}
	return v14;
}

int acMemeModuleStop()
{
	int thid;
	void *buf;

	if ( acMemeModuleStatus() == 0 )
	{
		return 0;
	}
	thid = Memec.thid;
	if ( Memec.thid > 0 )
	{
		int retry;
		int ret;

		Memec.thid = 0;
		WakeupThread(thid);
		DelayThread(1000);
		for ( retry = 9; retry >= 0; --retry )
		{
			ret = DeleteThread(thid);
			if ( !ret )
				break;
			DelayThread(1000000);
		}
		if ( retry < 0 )
			printf("acmeme:thread:term: TIMEDOUT %d\n", ret);
	}
	buf = Memec.buf;
	Memec.size = 0;
	Memec.buf = 0;
	if ( buf )
		FreeSysMemory(buf);
	Memec.status = 0;
	return 0;
}

int acMemeModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	return -88;
}
