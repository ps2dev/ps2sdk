/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "acuart_internal.h"

static struct uart_softc Uartc;

static int uart_eve_alloc()
{
	int ret;
	iop_event_t param;

	param.attr = 2;
	param.bits = 0;
	param.option = 0;
	ret = CreateEventFlag(&param);
	if ( ret <= 0 )
	{
		printf("acuart:eve_alloc: error %d\n", ret);
	}
	return ret;
}

static int uart_xmit(struct uart_buf *xmit, acUartReg uartreg)
{
	int head;
	int tail;
	acUint8 *buf;
	int count;

	head = xmit->ub_head;
	tail = xmit->ub_tail;
	if ( head == tail )
		return 0;
	buf = xmit->ub_buf;
	count = 16;
	if ( head < tail )
	{
		signed int size;
		const acUint8 *src;
		int size_v6;

		size = xmit->ub_size - tail;
		src = &buf[tail];
		if ( size > 16 )
		{
			size = 16;
			tail += 16;
		}
		else
		{
			tail = 0;
		}
		count = 16 - size;
		for ( size_v6 = size - 1; size_v6 >= 0; --size_v6 )
		{
			*uartreg = *src++;
		}
	}
	if ( count )
	{
		int size_v8;

		size_v8 = head - tail;
		if ( tail < head )
		{
			const acUint8 *src_v9;
			int size_v10;

			src_v9 = &buf[tail];
			if ( count < size_v8 )
			{
				size_v8 = count;
				tail += count;
			}
			else
			{
				tail = head;
			}
			count -= size_v8;
			for ( size_v10 = size_v8 - 1; size_v10 >= 0; --size_v10 )
			{
				*uartreg = *src_v9++;
			}
		}
	}
	xmit->ub_tail = tail;
	return 2 * (count < 16);
}

int acUartWrite(void *buf, int count)
{
	const acUint8 *src;
	int eve;
	int v6;
	int head;
	int tail;
	acUint8 *ub_buf;
	int bufsize;
	int size;
	acSpl state;

	if ( !buf || count < 0 )
		return -22;
	if ( !count )
		return 0;
	src = (acUint8 *)buf;
	if ( !Uartc.eve )
		return -6;
	eve = Uartc.eve;
	CpuSuspendIntr(&state);
	v6 = count;
	head = Uartc.xmit.ub_head;
	tail = Uartc.xmit.ub_tail;
	CpuResumeIntr(state);
	ub_buf = Uartc.xmit.ub_buf;
	bufsize = Uartc.xmit.ub_size;
	if ( head >= tail )
	{
		int xlen;
		acUint8 *dst;
		int xlen_v9;

		xlen = Uartc.xmit.ub_size - head;
		dst = &Uartc.xmit.ub_buf[head];
		if ( count < (signed int)(Uartc.xmit.ub_size - head) )
		{
			xlen = count;
			head += count;
		}
		else
		{
			head = 0;
		}
		v6 = count - xlen;
		for ( xlen_v9 = xlen - 1; xlen_v9 >= 0; ++dst )
		{
			--xlen_v9;
			*dst = *src++;
		}
	}
	if ( v6 && head < tail )
	{
		int xlen_v11;
		acUint8 *dst_v12;
		int xlen_v13;

		xlen_v11 = tail - head;
		dst_v12 = &ub_buf[head];
		if ( v6 < tail - head )
		{
			xlen_v11 = v6;
			head += v6;
		}
		else
		{
			head = tail;
		}
		v6 -= xlen_v11;
		for ( xlen_v13 = xlen_v11 - 1; xlen_v13 >= 0; ++dst_v12 )
		{
			--xlen_v13;
			*dst_v12 = *src++;
		}
	}
	if ( head == tail )
	{
		if ( --head < 0 )
			head = bufsize - 1;
		++v6;
	}
	else
	{
		SetEventFlag(eve, 2u);
	}
	Uartc.xmit.ub_head = head;
	size = count - v6;
	if ( size )
	{
		if ( (*((volatile acUint16 *)0xB241800A) /* originally 8 bit access */ & 0x20) != 0 )
		{
			uart_xmit(&Uartc.xmit, (acUartReg)0xB2418000);
			*((volatile acUint16 *)0xB2418002) = *((volatile acUint16 *)0xB2418002) | 2;
		}
	}
	return size;
}

int acUartRead(void *buf, int count)
{
	acUint8 *dst;
	int eve;
	int head;
	int tail;
	acUint8 *ub_buf;
	int v9;
	acSpl state;

	if ( !buf || count < 0 )
		return -22;
	if ( count == 0 )
	{
		return 0;
	}
	dst = (acUint8 *)buf;
	if ( Uartc.eve == 0 )
	{
		return -6;
	}
	eve = Uartc.eve;
	CpuSuspendIntr(&state);
	head = Uartc.recv.ub_head;
	tail = Uartc.recv.ub_tail;
	CpuResumeIntr(state);
	if ( head == tail )
	{
		return 0;
	}
	ub_buf = Uartc.recv.ub_buf;
	v9 = count;
	if ( head < tail )
	{
		int bufsize;
		const acUint8 *src;
		int bufsize_v9;

		bufsize = Uartc.recv.ub_size - tail;
		src = &Uartc.recv.ub_buf[tail];
		if ( count < (signed int)(Uartc.recv.ub_size - tail) )
		{
			bufsize = count;
			tail += count;
		}
		else
		{
			tail = 0;
		}
		v9 = count - bufsize;
		for ( bufsize_v9 = bufsize - 1; bufsize_v9 >= 0; ++dst )
		{
			--bufsize_v9;
			*dst = *src++;
		}
	}
	if ( v9 && tail < head )
	{
		int bufsize_v11;
		const acUint8 *src_v12;
		int bufsize_v13;

		bufsize_v11 = head - tail;
		src_v12 = &ub_buf[tail];
		if ( v9 < head - tail )
		{
			bufsize_v11 = v9;
			tail += v9;
		}
		else
		{
			tail = head;
		}
		v9 -= bufsize_v11;
		for ( bufsize_v13 = bufsize_v11 - 1; bufsize_v13 >= 0; ++dst )
		{
			--bufsize_v13;
			*dst = *src_v12++;
		}
	}
	Uartc.recv.ub_tail = tail;
	if ( head != tail )
		SetEventFlag(eve, 1u);
	return count - v9;
}

static unsigned int uart_timedout(void *arg)
{
	iReleaseWaitThread((int)arg);
	return 0;
}

int acUartWait(acUartFlag rw, int usec)
{
	int thid;
	int v7;
	int ret;
	iop_sys_clock_t tmout;
	u32 result_v7;

	if ( rw == 0 )
	{
		return -22;
	}
	if ( Uartc.eve == 0 )
	{
		return -6;
	}
	thid = GetThreadId();
	if ( usec > 0 )
	{
		USec2SysClock(usec, &tmout);
		SetAlarm(&tmout, uart_timedout, (void *)thid);
	}
	result_v7 = 0;
	v7 = (usec > 0 ? WaitEventFlag : PollEventFlag)(Uartc.eve, rw, 17, &result_v7);
	if ( v7 == -418 )
	{
		ret = -116;
	}
	else if ( v7 >= -417 )
	{
		ret = -5;
		if ( !v7 )
			ret = 0;
	}
	else
	{
		ret = -5;
		if ( v7 == -425 )
			ret = -6;
	}
	CancelAlarm(uart_timedout, (void *)thid);
	if ( !ret )
	{
		int ret_v5;

		ret_v5 = result_v7 & rw;
		result_v7 &= ~rw;
		if ( result_v7 )
			SetEventFlag(Uartc.eve, result_v7);
		return ret_v5;
	}
	return ret;
}

static int uart_intr(struct uart_softc *arg)
{
	struct uart_buf *p_recv;
	int v3;
	signed int event;
	acUint32 ub_tail;
	acUint8 *buf;
	signed int bufsize;
	int sr;
	int event_v9;

	p_recv = &arg->recv;
	v3 = 0;
	*((volatile acUint16 *)0xB3100000) = 0;
	event = arg->recv.ub_head;
	ub_tail = arg->recv.ub_tail;
	buf = arg->recv.ub_buf;
	bufsize = arg->recv.ub_size;
	sr = *((volatile acUint16 *)0xB241800A) /* originally 8 bit access */ & 0xFF;
	while ( (sr & 0x1F) != 0 )
	{
		if ( (sr & 0x10) != 0 )
		{
			Kprintf("acuart:intr: BREAK\n");
		}
		else
		{
			if ( sr & 1 )
			{
				buf[event] = *((volatile acUint16 *)0xB2418000);
				++event;
				if ( event >= bufsize )
					event = 0;
				if ( (acUint32)event == ub_tail && (int)++ub_tail >= bufsize )
					ub_tail = 0;
			}
		}
		v3 = 1;
		sr = *((volatile acUint16 *)0xB241800A) /* originally 8 bit access */ & 0xFF;
	}
	p_recv->ub_head = event;
	p_recv->ub_tail = ub_tail;
	event_v9 = v3;
	if ( (*((volatile acUint16 *)0xB241800A) /* originally 8 bit access */ & 0x20) != 0 )
	{
		if ( !uart_xmit(&arg->xmit, (acUartReg)0xB2418000) )
			*((volatile acUint16 *)0xB2418002) = *((volatile acUint16 *)0xB2418002) & 0xFD;
		event_v9 |= 2u;
	}
	if ( event_v9 )
		iSetEventFlag(arg->eve, event_v9);
	return 1;
}

static void uart_attr_set(struct uart_softc *uartc, acUartReg uartreg)
{
	int v3;
	int trigger;
	int v6;
	int v7;
	int state;

	v3 = 36864000 / (signed int)(16 * uartc->speed);
	trigger = uartc->fifo;
	if ( trigger < 14 )
	{
		if ( trigger < 8 )
			v6 = ((trigger >= 4) << 6 & 0xFFFF);
		else
			v6 = 128;
	}
	else
	{
		v6 = 192;
	}
	v7 = 16 * (uartc->loopback != 0);
	uartreg[1] = 0;
	uartreg[2] = 6;
	uartreg[3] = 131;
	*uartreg = (v3 + 1) >> 1;
	uartreg[1] = (v3 + 1) >> 9;
	uartreg[3] = 3;
	uartreg[4] = v7 | 0xF;
	uartreg[2] = v6 | 1;
	CpuSuspendIntr(&state);
	uartc->xmit.ub_tail = 0;
	uartc->xmit.ub_head = uartc->xmit.ub_tail;
	uartc->recv.ub_tail = 0;
	uartc->recv.ub_head = uartc->recv.ub_tail;
	CpuResumeIntr(state);
	uartreg[1] = 1;
}

int acUartSetAttr(const acUartAttrData *attr)
{
	int fifo;
	int fifo_v2;

	if ( attr == 0 )
	{
		return -22;
	}
	if ( Uartc.eve == 0 )
	{
		return -6;
	}
	fifo = attr->ua_fifo;
	Uartc.speed = attr->ua_speed;
	Uartc.loopback = attr->ua_loopback != 0;
	if ( fifo < 14 )
	{
		if ( fifo < 8 )
		{
			fifo_v2 = 4;
			if ( fifo < 4 )
				fifo_v2 = 1;
		}
		else
		{
			fifo_v2 = 8;
		}
	}
	else
	{
		fifo_v2 = 14;
	}
	Uartc.fifo = fifo_v2;
	uart_attr_set(&Uartc, (acUartReg)0xB2418000);
	return 0;
}

int acUartGetAttr(acUartAttrData *attr)
{
	if ( attr == 0 )
	{
		return -22;
	}
	if ( Uartc.eve == 0 )
	{
		return -6;
	}
	attr->ua_speed = Uartc.speed;
	attr->ua_fifo = Uartc.fifo;
	attr->ua_loopback = Uartc.loopback;
	return 0;
}

int acUartModuleStatus()
{
	int v0;
	int state;

	CpuSuspendIntr(&state);
	v0 = Uartc.eve != 0;
	CpuResumeIntr(state);
	return v0;
}

static int uart_optarg(const char *str, int default_value)
{
	int result;
	char *next;

	result = strtol(str, &next, 0);
	if ( next == str )
		return default_value;
	return result;
}

int acUartModuleStart(int argc, char **argv)
{
	int index;
	char **v7;
	int v9;
	int uartc;
	int eve;
	acUint8 *buf;
	char *msg;
	int size;

	if ( acUartModuleStatus() != 0 )
	{
		return -16;
	}
	Uartc.loopback = 0;
	if ( !Uartc.speed )
		Uartc.speed = 9600;
	if ( !Uartc.fifo )
		Uartc.fifo = 1;
	if ( !Uartc.xmit.ub_size )
		Uartc.xmit.ub_size = 256;
	index = 1;
	if ( !Uartc.recv.ub_size )
		Uartc.recv.ub_size = 512;
	v7 = argv + 1;
	while ( index < argc )
	{
		const char *opt;

		opt = *v7;
		if ( **v7 == 45 )
		{
			switch ( opt[1] )
			{
				case 'b':
					Uartc.speed = uart_optarg(opt + 2, Uartc.speed);
					break;
				case 'f':
					Uartc.fifo = uart_optarg(opt + 2, Uartc.fifo);
					break;
				case 'l':
					Uartc.loopback = 1;
					break;
				case 'r':
					Uartc.recv.ub_size = uart_optarg(opt + 2, Uartc.recv.ub_size);
					break;
				case 'x':
					Uartc.xmit.ub_size = uart_optarg(opt + 2, Uartc.xmit.ub_size);
					break;
				default:
					break;
			}
		}
		++index;
		++v7;
	}
	Uartc.xmit.ub_buf = 0;
	uartc = Uartc.recv.ub_size + Uartc.xmit.ub_size;
	v9 = uart_eve_alloc();
	eve = v9;
	if ( v9 <= 0 )
	{
		buf = 0;
		msg = "eve_alloc";
		size = v9;
	}
	else
	{
		buf = (acUint8 *)AllocSysMemory(0, uartc, 0);
		if ( buf == 0 )
		{
			msg = "mem_alloc";
			size = -12;
		}
		else
		{
			size = acIntrRegister(AC_INTR_NUM_UART, (acIntrHandler)uart_intr, &Uartc);
			if ( size < 0 )
			{
				msg = "intr_register";
			}
			else
			{
				size = acIntrEnable(AC_INTR_NUM_UART);
				if ( size < 0 )
				{
					acIntrRelease(AC_INTR_NUM_UART);
					msg = "intr_enable";
				}
				else
				{
					Uartc.xmit.ub_buf = buf;
					Uartc.recv.ub_buf = &buf[Uartc.xmit.ub_size];
					uart_attr_set(&Uartc, (acUartReg)0xB2418000);
					Uartc.eve = eve;
					return 0;
				}
			}
		}
	}
	printf("acuart:init:%s: error %d\n", msg, size);
	if ( buf )
		FreeSysMemory(buf);
	if ( eve > 0 )
	{
		DeleteEventFlag(eve);
	}
	return -6;
}

int acUartModuleStop()
{
	if ( !acUartModuleStatus() )
		return 0;
	*((volatile acUint16 *)0xB2418002) = 0;
	*((volatile acUint16 *)0xB2418004) = 7;
	acIntrDisable(AC_INTR_NUM_UART);
	acIntrRelease(AC_INTR_NUM_UART);
	if ( Uartc.xmit.ub_buf )
		FreeSysMemory(Uartc.xmit.ub_buf);
	if ( Uartc.eve > 0 )
	{
		DeleteEventFlag(Uartc.eve);
	}
	return 0;
}

int acUartModuleRestart(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	return -88;
}
