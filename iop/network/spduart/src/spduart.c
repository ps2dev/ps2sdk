/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <ctype.h>
#include <irx_imports.h>
#include <speedregs.h>

#define MODNAME "INET_SPEED_UART_driver"

// Based off of the module from SDK 3.1.0
IRX_ID(MODNAME, 0x2, 0x1A);

extern struct irx_export_table _exp_spduart;

// 16550 compatible UART interface.
// Reference: https://www.freebsd.org/doc/en_US.ISO8859-1/articles/serial-uart/index.html
typedef struct uart16550_regs_
{
	u8 r_wrthr_rdrbr; /**< write=transmit holding register (THR) / read=receive buffer register (RBR) */
	u8 pad01;
	u8 r_wrfcr_rdiir; /**< write=fifo control register (FCR) / read=interrupt identification register */
	u8 pad03;
	u8 r_lcr; /**< line control register (LCR) */
	u8 pad05;
	u8 r_mcr; /**< modem control register (MCR) */
	u8 pad07;
	u8 r_lsr; /**< line status register (LSR) */
	u8 pad09;
	u8 r_msr; /**< modem status register (MSR) */
	u8 pad0B;
	u8 r_scr; /**< scratch register (SCR) */
	u8 pad0D;
	u8 pad0E;
	u8 pad0F;
	u8 r_cr1; /**< control register 1 (CR1) */
	u8 pad11;
	u8 r_cr2; /**< control register 2 (CR2) */
	u8 pad13;
} uart16550_regs_t;

typedef struct spduart_buffer_struct_
{
	int m_offset1;
	int m_offset2;
	int m_cur_xfer_len;
	void *m_ptr;
} spduart_buffer_struct_t;

typedef struct spduart_internals_
{
	int spduart_thread;
	int spduart_ef;
	uart16550_regs_t *dev9_uart_reg_area;
	int spduart_thpri_;
	int spduart_thstack_;
	int spduart_overrun_error_count;
	int spduart_parity_error_count;
	int spduart_framing_error_count;
	int spduart_break_interrupt_count;
	int spduart_buffer_overflow_count;
	int spduart_rx_count;
	int spduart_tx_count;
	char spduart_fcr_cached;
	char spduart_msr_cached;
	char spduart_signal_pin;
	char spduart_mcr_cached;
	int spduart_baud_;
	int spduart_trig_;
	iop_sys_clock_t spduart_clock_1e4;
	iop_sys_clock_t spduart_clock_1e6;
	int spduart_alarm_1e4_flag;
	int spduart_alarm_1e6_flag;
	int spduart_start_flag;
	int spduart_stop_flag;
	int shutdown_flag;
	int spduart_data_set_ready_flag;
	int spduart_nogci_;
	char spduart_gci_[2];
	// cppcheck-suppress unusedStructMember
	char unused6a;
	// cppcheck-suppress unusedStructMember
	char unused6b;
	spduart_buffer_struct_t recv_buf_struct;
	spduart_buffer_struct_t send_buf_struct;
	sceModemOps_t spduart_modem_ops;
	char spduart_dial_[1024];
	char buf1data[1024];
	char buf2data[1024];
} spduart_internals_t;

// The modem used in SCPH-10281 network adapter is Conexant SMARTSCM/56 CX88168-11
// Datasheet: https://archive.org/download/ps2_modem_ref/DSA-140092.pdf
// AT command reference: https://archive.org/download/ps2_modem_ref/IML56_modem_AT_commands.pdf

// The following is formatted in Motorola s37 format (aka S-Record, SRECORD, S-Rec, SREC)
static const char *modem_firmware_s37[54] = {
	"S3100000A000B20269A9008DBC044C0EE101\r",
	"S31500009DA560606060606060606B606B606060606092\r",
	"S31500009DB56B60606B60606060606060606060606082\r",
	"S31500009DC56060606060606B606B60606B6060606067\r",
	"S31500009DD5606060606060606B6B6060606B60606B4C\r",
	"S31500009DE560606B606060606060606060606B606052\r",
	"S31500009EB000000600000000004E0003000000000045\r",
	"S31500009EC09300002A000000000000000000000000CF\r",
	"S31500009ED00000000000006E006100006E000000003F\r",
	"S31500009EE0000000000000004B7100000074000098A4\r",
	"S31500009EF000009B000000000000000000005E000063\r",
	"S30600009F00005A\r",
	"S31500009F019A284C5F86A9608DA79DADA102D019A99B\r",
	"S31500009F11048DEC02A973CD0D01B0038D0D01A9418C\r",
	"S31500009F21CD110190038D110160C2016001FF4C1931\r",
	"S31500009F31AD6487D014D2105D01A9328D1D87A900A9\r",
	"S31500009F418D1E8748C2105D0168604CC686A9603FB8\r",
	"S31500009F516B02A96B8DE49DA96B8DA79D604C008159\r",
	"S31500009F61A21820445A0DEB02A2184CF5584C6986EA\r",
	"S31500009F714C449EAE59048A2902F0168A29FD8D5950\r",
	"S31500009F8104A9528D1C06A9807B09208D1806A9A05B\r",
	"S31200009F917B60D2083906604CEF864C548682\r",
	"S31500009DF9AD4B878540AD4C878541A541C900D00447\r",
	"S31500009E09A540C996B033AD43878540AD44878541A2\r",
	"S31500009E1938A540E98C8540A541E9008541A540ED75\r",
	"S31500009E2952878540A541ED53878541B26444207820\r",
	"S30F00009E3969A5408D439E4C708660BB\r",
	"S31500009E447F4C04D21060016087FFAD9602C971F0A1\r",
	"S31500009E542C206067A540E9588540A541E902854163\r",
	"S31500009E64900BAD588789018D58874C4575AD960280\r",
	"S31500009E74C971F009ADA89E8DA79E4CAE77ADA89E7C\r",
	"S31500009E8438EDA79E9004C902B00160A900854085FB\r",
	"S31500009E9441A5408D5887A5418D598707FF20DD7759\r",
	"S30800009EA44C1361F5\r",
	"S31500008654A950CD0286B0038D028660E2750102043C\r",
	"S3150000866497FF27FF60A203A9004C9BE4AD478785CB\r",
	"S3150000867440AD48878541B26444207869AD439E8500\r",
	"S3150000868442208B69A541C907D004A540C9089012A8\r",
	"S31500008694A9758D3F87A9778D4087B200AEB248B1E0\r",
	"S315000086A4B759601FFF1BAFFF09A7FFC2027501B2CE\r",
	"S315000086B414FEE25D01100AC6FED00617FFD202754B\r",
	"S315000086C40160AD5702C93CD021BF6B1EB20747B249\r",
	"S315000086D45046B20A45B2B9442096E3B20747B250AF\r",
	"S315000086E446B20B45B2B9442096E360E21106100681\r",
	"S30C000086F47F5C032095026084\r",
	"S3150000810020A786E25D0110374F63342073E7AD96F2\r",
	"S3150000811002C971D00FF24E01100AAD0404C960B055\r",
	"S31500008120034CF960AD3F87C9B1F00FAD4D87C9006B\r",
	"S31500008130F00DE2600110094C7E9EA9008D4D87600E\r",
	"S3150000814020F99DADEC02C903D0F5AD3F87C9DBD060\r",
	"S31500008150EEE25E0101E920C466A5406D45878540D3\r",
	"S31500008160A5416D46878541B26444207869A5408D56\r",
	"S31500008170A89E38E92890C8A54038E93290060FFF36\r",
	"S30C00008180BE4C719E4C4C9EA3\r"};

static spduart_internals_t spduart_internals;

#define sceInetPrintf(...) printf(__VA_ARGS__)

static int module_start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi);
static int module_stop(int ac, char *av[], void *startaddr, ModuleInfo_t *mi);

int _start(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi)
{
	if ( argc >= 0 )
		return module_start(argc, argv, startaddr, mi);
	else
		return module_stop(argc, argv, startaddr, mi);
}

static void spduart_dump_msr(u8 msr)
{
	sceInetPrintf(
		"spduart: MSR=%02x (DCTS=%d DDSR=%d TERI=%d DDCD=%d\n",
		msr,
		msr & 1,
		(msr >> 1) & 1,
		(msr >> 2) & 1,
		(msr >> 3) & 1);
	sceInetPrintf(
		"spduart:          CTS=%d  DSR=%d   RI=%d  DCD=%d)\n", (msr >> 4) & 1, (msr >> 5) & 1, (msr >> 6) & 1, msr >> 7);
}

static unsigned int alarm_cb_1e4(struct spduart_internals_ *priv)
{
	priv->spduart_modem_ops.rcv_len = priv->recv_buf_struct.m_cur_xfer_len;
	iSetEventFlag(priv->spduart_modem_ops.evfid, 0x100u);
	return 0;
}

static u32 alarm_cb_1e6(struct spduart_internals_ *priv)
{
	iSetEventFlag(priv->spduart_ef, 4u);
	return priv->spduart_clock_1e6.lo;
}

static int spduart_send(struct spduart_internals_ *priv)
{
	int m_cur_xfer_len;
	int v4;
	u8 v7;
	int state;

	CpuSuspendIntr(&state);
	priv->spduart_fcr_cached &= ~2;
	m_cur_xfer_len = priv->send_buf_struct.m_cur_xfer_len;
	v4 = 0;
	if ( m_cur_xfer_len > 0 )
	{
		if ( m_cur_xfer_len > 16 )
			m_cur_xfer_len = 16;
		if ( (priv->spduart_msr_cached & 0x10) != 0 )
		{
			if ( (spduart_internals.dev9_uart_reg_area->r_msr & 0x20) != 0 )
			{
				int v6;

				v6 = m_cur_xfer_len - 1;
				do
				{
					v7 = *((u8 *)priv->send_buf_struct.m_ptr + (priv->send_buf_struct.m_offset2 & 0x3FF));
					++priv->send_buf_struct.m_offset2;
					priv->send_buf_struct.m_cur_xfer_len -= 1;
					--v6;
					spduart_internals.dev9_uart_reg_area->r_wrthr_rdrbr = v7;
					++v4;
				} while ( v6 > 0 );
				if ( priv->send_buf_struct.m_cur_xfer_len > 0 )
				{
					priv->spduart_fcr_cached |= 2;
				}
			}
			else
			{
				priv->spduart_fcr_cached |= 2;
			}
			priv->spduart_tx_count += v4;
		}
	}
	spduart_internals.dev9_uart_reg_area->r_wrfcr_rdiir = priv->spduart_fcr_cached;
	CpuResumeIntr(state);
	return v4;
}

static int spduart_dev9_intr_cb(int flag)
{
	int v1;
	u32 v2;
	u8 r_msr;
	int m_offset1;
	char r_scr;
	u32 v18;

	(void)flag;

	v1 = 0;
	v2 = 0;
	while ( 1 )
	{
		switch ( spduart_internals.dev9_uart_reg_area->r_lcr & 0xF )
		{
			case 0:
				r_scr = spduart_internals.dev9_uart_reg_area->r_scr;
				if ( spduart_internals.spduart_start_flag )
				{
					spduart_internals.spduart_msr_cached = spduart_internals.dev9_uart_reg_area->r_scr;
					continue;
				}
				if ( (spduart_internals.dev9_uart_reg_area->r_scr & 1) != 0 )
				{
					if ( (spduart_internals.dev9_uart_reg_area->r_scr & 0x10) != 0 )
					{
						v2 |= 4u;
					}
				}
				if ( r_scr & 2 )
				{
					if ( (spduart_internals.dev9_uart_reg_area->r_scr & 0x20) == 0 )
					{
						spduart_internals.shutdown_flag = 1;
						iSetEventFlag(spduart_internals.spduart_modem_ops.evfid, 2);
					}
					else
					{
						if ( !spduart_internals.spduart_data_set_ready_flag )
						{
							v2 |= 4u;
							spduart_internals.spduart_data_set_ready_flag = 1;
							iSetEventFlag(spduart_internals.spduart_modem_ops.evfid, 513);
						}
					}
				}
				if ( r_scr & 4 )
				{
					iSetEventFlag(spduart_internals.spduart_modem_ops.evfid, 0x40u);
				}
				if ( (r_scr & 8) && ((spduart_internals.spduart_msr_cached ^ (u8)r_scr) & 0x80) != 0 )
				{
					v18 = 32;
					if ( (r_scr & 0x80) != 0 )
						v18 = 16;
					iSetEventFlag(spduart_internals.spduart_modem_ops.evfid, v18);
				}
				spduart_internals.spduart_msr_cached = r_scr;
				break;
			case 1:
				if ( v1 > 0 )
				{
					v2 |= 8u;
					spduart_internals.spduart_rx_count += v1;
					if ( spduart_internals.recv_buf_struct.m_cur_xfer_len < 257 )
					{
						if ( (spduart_internals.spduart_signal_pin & 2) == 0 )
						{
							spduart_internals.spduart_signal_pin |= 2u;
							spduart_internals.dev9_uart_reg_area->r_lsr = spduart_internals.spduart_signal_pin;
						}
					}
					if ( spduart_internals.recv_buf_struct.m_cur_xfer_len >= 768 )
					{
						if ( (spduart_internals.spduart_signal_pin & 2) != 0 )
						{
							spduart_internals.spduart_signal_pin &= ~2u;
							spduart_internals.dev9_uart_reg_area->r_lsr = spduart_internals.spduart_signal_pin;
						}
					}
				}
				if ( v2 )
					iSetEventFlag(spduart_internals.spduart_ef, v2);
				return 0;
			case 2:
				spduart_internals.spduart_fcr_cached &= ~2u;
				spduart_internals.dev9_uart_reg_area->r_wrfcr_rdiir = spduart_internals.spduart_fcr_cached;
				v2 |= 4u;
				continue;
			case 4:
			case 0xC:
				while ( (spduart_internals.dev9_uart_reg_area->r_msr & 1) != 0 && !spduart_internals.spduart_start_flag )
				{
					if ( spduart_internals.recv_buf_struct.m_cur_xfer_len >= 1024 )
					{
						++spduart_internals.spduart_buffer_overflow_count;
					}
					else
					{
						++v1;
						m_offset1 = spduart_internals.recv_buf_struct.m_offset1;
						*((u8 *)spduart_internals.recv_buf_struct.m_ptr + (spduart_internals.recv_buf_struct.m_offset1 & 0x3FF)) =
							spduart_internals.dev9_uart_reg_area->r_wrthr_rdrbr;
						spduart_internals.recv_buf_struct.m_offset1 = m_offset1 + 1;
						++spduart_internals.recv_buf_struct.m_cur_xfer_len;
					}
				}
				continue;
			case 6:
				r_msr = spduart_internals.dev9_uart_reg_area->r_msr;
				if ( r_msr & 2 )
				{
					++spduart_internals.spduart_overrun_error_count;
				}
				if ( r_msr & 4 )
				{
					++spduart_internals.spduart_parity_error_count;
				}
				if ( r_msr & 8 )
				{
					++spduart_internals.spduart_framing_error_count;
				}
				if ( r_msr & 0x10 )
				{
					++spduart_internals.spduart_break_interrupt_count;
				}
				continue;
			default:
				continue;
		}
	}
}

static int spduart_set_baud(int baud)
{
	switch ( baud )
	{
		case 115200:
			return 1;
		case 57600:
			return 2;
		case 38400:
			return 3;
		case 28800:
			return 4;
		case 19200:
			return 6;
		case 9600:
			return 12;
		default:
			return 0;
	}
}

static void spduart_do_init_dev9(struct spduart_internals_ *priv)
{
	int v2;

	spduart_internals.dev9_uart_reg_area->r_cr1 = 27;
	spduart_internals.dev9_uart_reg_area->r_cr2 = 4;
	DelayThread(10000);
	spduart_internals.dev9_uart_reg_area->r_wrfcr_rdiir = 0;
	priv->spduart_mcr_cached = -125;
	spduart_internals.dev9_uart_reg_area->r_mcr = priv->spduart_mcr_cached;
	v2 = spduart_set_baud(priv->spduart_baud_);
	spduart_internals.dev9_uart_reg_area->r_wrthr_rdrbr = v2;
	spduart_internals.dev9_uart_reg_area->r_wrfcr_rdiir = (v2 >> 8) & 0xFF;
	priv->spduart_mcr_cached = 3;
	spduart_internals.dev9_uart_reg_area->r_mcr = priv->spduart_mcr_cached;
	priv->spduart_signal_pin = 3;
	spduart_internals.dev9_uart_reg_area->r_lsr = priv->spduart_signal_pin;
	switch ( priv->spduart_trig_ )
	{
		case 1:
		{
			spduart_internals.dev9_uart_reg_area->r_lcr = 1;
			break;
		}
		case 4:
		{
			spduart_internals.dev9_uart_reg_area->r_lcr = 65;
			break;
		}
		case 8:
		default:
		{
			spduart_internals.dev9_uart_reg_area->r_lcr = -127;
			break;
		}
		case 14:
		{
			spduart_internals.dev9_uart_reg_area->r_lcr = -63;
			break;
		}
	}
}

static void spduart_deinit()
{
	spduart_internals.dev9_uart_reg_area->r_cr2 = -124;
}

static void fail_sleep(const char *fmt)
{
	sceInetPrintf(fmt);
	while ( 1 )
		SleepThread();
}

static void sleep_on_shutdown(const struct spduart_internals_ *priv)
{
	if ( priv->shutdown_flag )
	{
		fail_sleep("spduart: plug-out\n");
	}
}

static int spduart_send_now(struct spduart_internals_ *priv, const char *in_buf, char *expected_buf)
{
	char *v6;
	int v7;
	char r_scr;
	int v11;
	char v12;
	int v13;
	int v14;

	v6 = expected_buf;
	v7 = 0;
	while ( 1 )
	{
		int v8;
		int v9;

		v8 = 0;
		v9 = 0;
		sleep_on_shutdown(priv);
		if ( *in_buf )
		{
			r_scr = spduart_internals.dev9_uart_reg_area->r_scr;
			priv->spduart_msr_cached = r_scr;
			if ( (r_scr & 0x10) != 0 && (spduart_internals.dev9_uart_reg_area->r_msr & 0x20) != 0 )
			{
				v11 = 15;
				do
				{
					v12 = *in_buf;
					v13 = v11;
					if ( !v12 )
						break;
					++in_buf;
					++v9;
					--v11;
					spduart_internals.dev9_uart_reg_area->r_wrthr_rdrbr = v12;
				} while ( v13 > 0 );
			}
		}
		if ( (spduart_internals.dev9_uart_reg_area->r_msr & 1) != 0 )
		{
			v8 = 1;
			v14 = *v6;
			if ( *v6 )
			{
				++v6;
				if ( v14 != spduart_internals.dev9_uart_reg_area->r_wrthr_rdrbr )
					v6 = expected_buf;
			}
		}
		if ( !*in_buf )
		{
			if ( !*v6 )
				break;
		}
		if ( !v9 && !v8 )
		{
			if ( v7 >= 300 )
				return -1;
			v7 += 1;
			DelayThread(10000);
		}
	}
	return 0;
}

static void spduart_init_hw(struct spduart_internals_ *priv)
{
	int i;
	int v5;
	char *buf1data;

	while ( 1 )
	{
		int v7;
		int v10;
		const char *v11;

		while ( 1 )
		{
			while ( 1 )
			{
				do
				{
					spduart_deinit();
					DelayThread(200000);
					sleep_on_shutdown(priv);
					spduart_do_init_dev9(priv);
					for ( i = 0; i < 300; ++i )
					{
						sleep_on_shutdown(priv);
						DelayThread(10000);
					}
					while ( 1 )
					{
						sleep_on_shutdown(priv);
						priv->spduart_msr_cached = spduart_internals.dev9_uart_reg_area->r_scr;
						if ( (priv->spduart_msr_cached & 0x20) != 0 )
							break;
						DelayThread(100000);
					}
				} while ( spduart_send_now(priv, "AT\r", "OK") < 0 && spduart_send_now(priv, "AT\r", "OK") < 0 );
				if ( spduart_send_now(priv, "ATI3\r", "P2109-V90") < 0 )
					break;
				sceInetPrintf("spduart: P2109-V90 detected\n");
				spduart_send_now(priv, "AT**\r", "Download initiated ..");
				sceInetPrintf("spduart: downloading (9A-40)\n");
				v5 = 0;
				i = 0;
				while ( modem_firmware_s37[i] )
				{
					v5 = spduart_send_now(priv, modem_firmware_s37[i], ".");
					if ( v5 < 0 )
					{
						sceInetPrintf("spduart: failed to download (1)\n");
						break;
					}
					i += 1;
				}
				if ( v5 >= 0 && spduart_send_now(priv, "S70500000000FA\r", "OK") >= 0 )
				{
					sceInetPrintf("spduart: download completed\n");
					break;
				}
				sceInetPrintf("spduart: failed to download (2)\n");
			}
			if ( priv->spduart_nogci_ )
				return;
			if ( !priv->spduart_gci_[0] )
				break;
			sprintf(priv->buf2data, "AT+GCI=%c%c\r", priv->spduart_gci_[0], priv->spduart_gci_[1]);
			sceInetPrintf("spduart: sending \"%S\"\n", priv->buf2data);
			if ( spduart_send_now(priv, priv->buf2data, "OK") >= 0 )
				return;
			sceInetPrintf("spduart: failed to set GCI (1)\n");
		}
		if ( !priv->spduart_dial_[0] )
		{
			fail_sleep("spduart: no dial_cnf\n");
		}
		sceInetPrintf("spduart: scanning GCI setting in \"%s\"\n", priv->spduart_dial_);
		v7 = open(priv->spduart_dial_, 1);
		if ( v7 < 0 )
		{
			fail_sleep("spduart: can't open\n");
		}
		buf1data = priv->buf1data;
		v10 = read(v7, buf1data, 1024);
		if ( v10 < 0 )
		{
			fail_sleep("spduart: read error\n");
		}
		close(v7);
		if ( (unsigned int)v10 >= 0x400 )
		{
			fail_sleep("spduart: file too big\n");
		}
		v11 = &buf1data[v10];
		while ( 1 )
		{
			if ( buf1data >= v11 )
			{
				fail_sleep("spduart: no GCI setting\n");
			}
			while ( buf1data < v11 )
			{
				if ( (isspace(*buf1data)) == 0 )
					break;
				buf1data += 1;
			}
			if ( *buf1data != '#' )
			{
				while ( buf1data < v11 && *buf1data != '+' )
				{
					buf1data += 1;
				}
				if ( (*buf1data == '+') && (strncmp("+GCI=", buf1data, 5) == 0) )
				{
					buf1data += 5;
					if ( ((isxdigit(buf1data[0])) != 0) && ((isxdigit(buf1data[1])) != 0) )
					{
						break;
					}
				}
			}
			while ( buf1data < v11 && *buf1data != '\n' )
			{
				buf1data += 1;
			}
			if ( *buf1data == '\n' )
				buf1data += 1;
		}
		sprintf(priv->buf2data, "AT+GCI=%c%c\r", (u8)*buf1data, (u8)buf1data[1]);
		sceInetPrintf("spduart: sending \"%S\"\n", priv->buf2data);
		if ( spduart_send_now(priv, priv->buf2data, "OK") < 0 )
		{
			sceInetPrintf("spduart: failed to set GCI (2)\n");
			continue;
		}
		break;
	}
}

static void spduart_thread_proc(struct spduart_internals_ *priv)
{
	int i;
	int m_cur_xfer_len;
	int v10;
	u32 v12;

	spduart_init_hw(priv);
	priv->spduart_fcr_cached = 13;
	spduart_internals.dev9_uart_reg_area->r_wrfcr_rdiir = priv->spduart_fcr_cached;
	while ( !WaitEventFlag(priv->spduart_ef, 0xFu, 17, &v12) )
	{
		if ( (v12 & 2) != 0 && !priv->spduart_start_flag )
		{
			SpdIntrDisable(4096);
			if ( priv->spduart_alarm_1e4_flag )
			{
				CancelAlarm((unsigned int (*)(void *))alarm_cb_1e4, priv);
				priv->spduart_alarm_1e4_flag = 0;
			}
			if ( priv->spduart_alarm_1e6_flag )
			{
				CancelAlarm((unsigned int (*)(void *))alarm_cb_1e6, priv);
				priv->spduart_alarm_1e6_flag = 0;
			}
			priv->spduart_signal_pin = 0;
			spduart_internals.dev9_uart_reg_area->r_lsr = 0;
			spduart_internals.dev9_uart_reg_area->r_lcr = -121;
			priv->recv_buf_struct.m_cur_xfer_len = 0;
			priv->recv_buf_struct.m_offset1 = 0;
			priv->recv_buf_struct.m_offset2 = 0;
			priv->send_buf_struct.m_cur_xfer_len = 0;
			priv->send_buf_struct.m_offset1 = 0;
			priv->send_buf_struct.m_offset2 = 0;
			// cppcheck-suppress redundantAssignment
			spduart_internals.dev9_uart_reg_area->r_lcr = -127;
			priv->spduart_start_flag = 1;
			priv->spduart_stop_flag = 0;
			if ( !priv->shutdown_flag )
			{
				priv->spduart_mcr_cached = 67;
				spduart_internals.dev9_uart_reg_area->r_mcr = priv->spduart_mcr_cached;
				for ( i = 0; i < 20; i += 1 )
				{
					if ( priv->shutdown_flag )
						break;
					if ( (priv->spduart_msr_cached & 0x80) == 0 )
					{
						priv->spduart_mcr_cached = 3;
						spduart_internals.dev9_uart_reg_area->r_mcr = 3;
						for ( i = 0; i < 300; i += 1 )
						{
							if ( priv->shutdown_flag )
								break;
							DelayThread(10000);
						}
						break;
					}
					DelayThread(100000);
				}
			}
		}
		if ( !priv->spduart_start_flag )
		{
			if ( (v12 & 4) != 0 )
			{
				if ( priv->spduart_data_set_ready_flag )
				{
					if ( spduart_send(priv) > 0 )
					{
						m_cur_xfer_len = priv->send_buf_struct.m_cur_xfer_len;
						if ( m_cur_xfer_len >= 257 )
						{
							priv->spduart_modem_ops.snd_len = 1024 - m_cur_xfer_len;
							SetEventFlag(priv->spduart_modem_ops.evfid, 0x200u);
						}
					}
				}
			}
			if ( (v12 & 8) != 0 )
			{
				if ( priv->spduart_alarm_1e4_flag )
				{
					CancelAlarm((unsigned int (*)(void *))alarm_cb_1e4, priv);
					priv->spduart_alarm_1e4_flag = 0;
				}
				v10 = priv->recv_buf_struct.m_cur_xfer_len;
				if ( v10 >= 257 )
				{
					priv->spduart_modem_ops.rcv_len = v10;
					SetEventFlag(priv->spduart_modem_ops.evfid, 0x100u);
				}
				else
				{
					SetAlarm(&priv->spduart_clock_1e4, (unsigned int (*)(void *))alarm_cb_1e4, priv);
					priv->spduart_alarm_1e4_flag = 1;
				}
			}
		}
		else if ( (v12 & 1) != 0 )
		{
			priv->shutdown_flag = 0;
			priv->spduart_data_set_ready_flag = 0;
			priv->spduart_modem_ops.snd_len = 1024;
			priv->spduart_signal_pin = 3;
			spduart_internals.dev9_uart_reg_area->r_lsr = 3;
			priv->spduart_msr_cached = spduart_internals.dev9_uart_reg_area->r_scr;
			spduart_dump_msr(priv->spduart_msr_cached);
			if ( (priv->spduart_msr_cached & 0x20) != 0 )
			{
				SetEventFlag(priv->spduart_modem_ops.evfid, 0x201u);
				priv->spduart_data_set_ready_flag = 1;
			}
			if ( (priv->spduart_msr_cached & 0x80) != 0 )
			{
				SetEventFlag(priv->spduart_modem_ops.evfid, 0x10u);
			}
			priv->spduart_start_flag = 0;
			SetAlarm(&priv->spduart_clock_1e6, (unsigned int (*)(void *))alarm_cb_1e6, priv);
			priv->spduart_alarm_1e6_flag = 1;
			SpdIntrEnable(4096);
		}
	}
}

static int spduart_op_start(struct spduart_internals_ *priv, int flags)
{
	(void)flags;

	priv->spduart_stop_flag = 0;
	SetEventFlag(priv->spduart_ef, 1u);
	return 0;
}

static int spduart_op_stop(struct spduart_internals_ *priv, int flags)
{
	(void)flags;

	priv->spduart_stop_flag = 1;
	SetEventFlag(priv->spduart_ef, 2u);
	return 0;
}

static size_t spduart_op_recv(struct spduart_internals_ *priv, void *ptr, int len)
{
	int m_cur_xfer_len;
	size_t v7;
	int v8;
	size_t v9;
	int state;

	m_cur_xfer_len = len;
	v7 = 0;
	if ( priv->recv_buf_struct.m_cur_xfer_len < len )
		m_cur_xfer_len = priv->recv_buf_struct.m_cur_xfer_len;
	v8 = priv->recv_buf_struct.m_offset2 & 0x3FF;
	v9 = 1024 - v8;
	if ( m_cur_xfer_len < 1024 - v8 )
		v9 = m_cur_xfer_len;
	if ( v9 > 0 )
	{
		size_t v10;

		v7 = v9;
		bcopy((char *)priv->recv_buf_struct.m_ptr + v8, ptr, v9);
		v10 = m_cur_xfer_len - v9;
		if ( v10 > 0 )
		{
			bcopy(priv->recv_buf_struct.m_ptr, (char *)ptr + v9, v10);
			v7 += v10;
		}
		CpuSuspendIntr(&state);
		priv->recv_buf_struct.m_cur_xfer_len -= v7;
		priv->recv_buf_struct.m_offset2 += v7;
		priv->spduart_modem_ops.rcv_len = priv->recv_buf_struct.m_cur_xfer_len;
		if ( priv->recv_buf_struct.m_cur_xfer_len < 257 )
		{
			if ( (priv->spduart_signal_pin & 2) == 0 )
			{
				priv->spduart_signal_pin |= 2;
				spduart_internals.dev9_uart_reg_area->r_lsr = priv->spduart_signal_pin;
			}
		}
		CpuResumeIntr(state);
	}
	return v7;
}

static int spduart_op_send(struct spduart_internals_ *priv, void *ptr, int len)
{
	int v4;
	int v7;
	int v8;
	size_t v9;
	int state;

	v4 = len;
	v7 = 0;
	if ( 1024 - priv->send_buf_struct.m_cur_xfer_len < len )
		v4 = 1024 - priv->send_buf_struct.m_cur_xfer_len;
	v8 = priv->send_buf_struct.m_offset1 & 0x3FF;
	v9 = 1024 - v8;
	if ( (unsigned int)v4 < v9 )
		v9 = v4;
	if ( v9 > 0 )
	{
		size_t v10;

		v7 = v9;
		bcopy(ptr, (char *)priv->send_buf_struct.m_ptr + v8, v9);
		v10 = v4 - v9;
		if ( v10 > 0 )
		{
			bcopy((char *)ptr + v9, priv->send_buf_struct.m_ptr, v10);
			v7 += v10;
		}
		CpuSuspendIntr(&state);
		priv->send_buf_struct.m_cur_xfer_len += v7;
		priv->send_buf_struct.m_offset1 += v7;
		priv->spduart_modem_ops.snd_len = 1024 - priv->send_buf_struct.m_cur_xfer_len;
		CpuResumeIntr(state);
	}
	if ( v7 > 0 )
		SetEventFlag(priv->spduart_ef, 4u);
	return v7;
}

static int spduart_op_control(struct spduart_internals_ *priv, int code, void *ptr, int len)
{
	int v22;
	int state;
	int priority;
	unsigned int v25;
	int v26;

	switch ( code )
	{
		case 0xC0000110:
		case 0xC0000111:
		case 0xC0000200:
			break;
		default:
		{
			if ( len != 4 )
				return -512;
		}
	}
	switch ( code )
	{
		case 0xC0000000:
		{
			bcopy(&priv->spduart_thpri_, ptr, 4);
			return 0;
		}
		case 0xC0000100:
		{
			v22 = 1;
			bcopy(&v22, ptr, 4);
			return 0;
		}
		case 0xC0000110:
		{
			CpuSuspendIntr(&state);
			priv->recv_buf_struct.m_offset1 = 0;
			priv->recv_buf_struct.m_offset2 = 0;
			priv->recv_buf_struct.m_cur_xfer_len = 0;
			priv->spduart_modem_ops.rcv_len = 0;
			CpuResumeIntr(state);
			return 0;
		}
		case 0xC0000111:
		{
			CpuSuspendIntr(&state);
			priv->send_buf_struct.m_offset1 = 0;
			priv->send_buf_struct.m_offset2 = 0;
			priv->send_buf_struct.m_cur_xfer_len = 0;
			priv->spduart_modem_ops.snd_len = 1024;
			CpuResumeIntr(state);
			return 0;
		}
		case 0xC0000200:
		{
			int v13;

			v13 = strlen(priv->spduart_dial_) + 1;
			if ( len < v13 )
			{
				return -512;
			}
			bcopy(priv->spduart_dial_, ptr, v13);
			return 0;
		}
		case 0xC0010000:
		{
			bcopy(&priv->spduart_rx_count, ptr, 4);
			return 0;
		}
		case 0xC0010001:
		{
			bcopy(&priv->spduart_tx_count, ptr, 4);
			return 0;
		}
		case 0xC0010002:
		{
			bcopy(&priv->spduart_overrun_error_count, ptr, 4);
			return 0;
		}
		case 0xC0010003:
		{
			bcopy(&priv->spduart_parity_error_count, ptr, 4);
			return 0;
		}
		case 0xC0010004:
		{
			bcopy(&priv->spduart_framing_error_count, ptr, 4);
			return 0;
		}
		case 0xC0010005:
		{
			bcopy(&priv->spduart_buffer_overflow_count, ptr, 4);
			return 0;
		}
		case 0xC0020000:
		{
			v25 = ((priv->spduart_mcr_cached & 3) << 28) | ((priv->spduart_mcr_cached & 0x18) << 27) | priv->spduart_baud_
					| 0x2000000;
			if ( (priv->spduart_mcr_cached & 4) != 0 )
				v25 |= 0xC000000;
			else
				v25 |= 0x4000000;
			bcopy((int *)&v25, ptr, 4);
			return 0;
		}
		case 0xC0030000:
		{
			v26 = (16 * (priv->spduart_signal_pin & 3)) | ((u8)priv->spduart_msr_cached >> 4);
			bcopy(&v26, ptr, 4);
			return 0;
		}
		case 0xC1000000:
		{
			int v6;

			bcopy(ptr, &priority, 4);
			v6 = 0;
			if ( priv->spduart_thread > 0 )
			{
				v6 = ChangeThreadPriority(priv->spduart_thread, priority);
				if ( !v6 )
					priv->spduart_thpri_ = priority;
			}
			if ( priv->spduart_thread <= 0 || v6 == -413 )
			{
				if ( (unsigned int)(priority - 9) < 0x73 )
					priv->spduart_thpri_ = priority;
			}
			return v6;
		}
		case 0xC1020000:
		{
			int v17;
			unsigned int v18;

			bcopy(ptr, &v25, 4);
			v17 = spduart_set_baud(v25 & 0x3FFFFF);
			if ( !v17 )
			{
				return -512;
			}
			if ( ((v25 >> 26) & 1) == 0 || (v25 & 0x1800000) != 0 )
				return -512;
			v18 = v25 >> 30;
			if ( v18 == 1 || (v25 & 0x2000000) == 0 )
			{
				return -512;
			}
			priv->spduart_mcr_cached =
				v18 | ((int)((v25 >> 26) & 3) >> 1) | ((v25 >> 28) & 3) | (priv->spduart_mcr_cached & 0x40) | 0x80;
			spduart_internals.dev9_uart_reg_area->r_mcr = priv->spduart_mcr_cached;
			spduart_internals.dev9_uart_reg_area->r_wrthr_rdrbr = v17;
			spduart_internals.dev9_uart_reg_area->r_wrfcr_rdiir = (v17 >> 8) & 0xFF;
			priv->spduart_mcr_cached &= ~0x80;
			// cppcheck-suppress redundantAssignment
			spduart_internals.dev9_uart_reg_area->r_mcr = priv->spduart_mcr_cached;
			return 0;
		}
		case 0xC1030000:
		{
			bcopy(ptr, &v26, 4);
			if ( (v26 & 0xFFFFFFCF) != 0 )
			{
				return -512;
			}
			priv->spduart_signal_pin &= ~3;
			priv->spduart_signal_pin |= ((unsigned int)v26 >> 4);
			spduart_internals.dev9_uart_reg_area->r_lsr = priv->spduart_signal_pin;
			return 0;
		}
		default:
			return -513;
	}
}

static void shutdown_cb(void)
{
	spduart_internals.shutdown_flag = 1;
}

static int print_version(void)
{
	printf("SPDUART (%s)%s\n", "Version 2.26.0", "");
	return MODULE_NO_RESIDENT_END;
}

static int print_usage(void)
{
	printf("Usage: spduart [-probe] [-nogci] [baud=<baud>] [trig=<trig>] [gci=<gci>] [thpri=<prio>] [thstack=<stack>] "
				 "[dial=<dial>]\n");
	return MODULE_NO_RESIDENT_END;
}

// cppcheck-suppress unusedFunction
void spduart_2_deinit(int a1)
{
	USE_SPD_REGS;

	if ( !a1 && (SPD_REG16(SPD_R_REV_3) & 8) != 0 )
		spduart_internals.dev9_uart_reg_area->r_cr2 = -124;
}

// cppcheck-suppress constParameter
static int module_start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	int v2;
	int v3;
	int v5;
	char *v7;
	int v11;
	int v14;
	int v16;
	iop_event_t v17;
	iop_thread_t v18;
	USE_SPD_REGS;

	(void)startaddr;

	v2 = 0;
	v3 = 0;
	spduart_internals.spduart_thpri_ = 40;
	spduart_internals.spduart_thstack_ = 0x2000;
	spduart_internals.spduart_dial_[0] = 0;
	spduart_internals.spduart_baud_ = 115200;
	spduart_internals.spduart_trig_ = 8;
	for ( v5 = 1; v5 < ac; v5 += 1 )
	{
		if ( !strcmp("-help", av[v5]) )
		{
			return print_usage();
		}
		else if ( !strcmp("-version", av[v5]) )
		{
			return print_version();
		}
		else if ( !strcmp("-probe", av[v5]) )
		{
			v3 = 1;
			continue;
		}
		else if ( !strcmp("-nogci", av[v5]) )
		{
			spduart_internals.spduart_nogci_ = 1;
			continue;
		}
		else if ( !strncmp("dial=", av[v5], 5) )
		{
			strcpy(spduart_internals.spduart_dial_, av[v5] + 5);
			continue;
		}
		else if ( !strncmp("gci=", av[v5], 4) )
		{
			v7 = (char *)(av[v5] + 4);
			if ( (isxdigit(*v7)) == 0 || (isxdigit(v7[1])) == 0 )
				return print_usage();
			spduart_internals.spduart_gci_[0] = v7[0];
			spduart_internals.spduart_gci_[1] = v7[1];
			continue;
		}
		else if ( !strncmp("thpri=", av[v5], 6) )
		{
			v7 = (char *)(av[v5] + 6);
			if ( (isdigit(*v7)) == 0 )
				return print_usage();
			v11 = strtol(v7, 0, 10);
			if ( (unsigned int)(v11 - 9) >= 0x73 )
				return print_usage();
			spduart_internals.spduart_thpri_ = v11;
		}
		else if ( !strncmp("thstack=", av[v5], 8) )
		{
			v7 = (char *)(av[v5] + 8);
			if ( (isdigit(*v7)) == 0 )
				return print_usage();
			spduart_internals.spduart_thstack_ = strtol(v7, 0, 10);
			while ( *v7 )
			{
				if ( (isdigit(*v7)) == 0 )
					break;
				++v7;
			}
			if ( !strcmp(v7, "KB") )
			{
				spduart_internals.spduart_thstack_ <<= 10;
				continue;
			}
		}
		else if ( !strncmp("baud=", av[v5], 5) )
		{
			v7 = (char *)(av[v5] + 5);
			if ( (isdigit(*v7)) == 0 )
				return print_usage();
			spduart_internals.spduart_baud_ = strtol(v7, 0, 10);
			if ( !spduart_set_baud(spduart_internals.spduart_baud_) )
				return print_usage();
		}
		else if ( !strncmp("trig=", av[v5], 5) )
		{
			v7 = (char *)(av[v5] + 5);
			if ( (isdigit(*v7)) == 0 )
				return print_usage();
			v11 = strtol(v7, 0, 10);
			switch ( v11 )
			{
				case 1:
				case 4:
				case 8:
				case 14:
				{
					spduart_internals.spduart_trig_ = v11;
					break;
				}
				default:
				{
					return print_usage();
				}
			}
		}
		else
		{
			return print_usage();
		}
		while ( *v7 )
		{
			if ( (isdigit(*v7)) == 0 )
				break;
			++v7;
		}
		if ( *v7 )
			return print_usage();
	}
	if ( (SPD_REG16(SPD_R_REV_3) & 8) == 0 )
	{
		sceInetPrintf("spduart: SPEED-UART not found\n");
		return 1;
	}
	sceInetPrintf("spduart: SPEED-UART detected\n");
	if ( v3 )
	{
		return 5;
	}
	if ( RegisterLibraryEntries(&_exp_spduart) )
	{
		sceInetPrintf("spduart: module already loaded\n");
	}
	else
	{
		v14 = 0;
		sceInetPrintf(
			"spduart: thpri=%d baud=%d trig=%d\n",
			spduart_internals.spduart_thpri_,
			spduart_internals.spduart_baud_,
			spduart_internals.spduart_trig_);
		spduart_internals.dev9_uart_reg_area = (uart16550_regs_t *)0xB0000080;
		spduart_internals.recv_buf_struct.m_ptr = spduart_internals.buf1data;
		spduart_internals.send_buf_struct.m_ptr = spduart_internals.buf2data;
		spduart_internals.spduart_start_flag = 1;
		spduart_internals.spduart_stop_flag = 0;
		spduart_do_init_dev9(&spduart_internals);
		for ( v14 = 0; v14 < 30; v14 += 1 )
		{
			spduart_internals.spduart_msr_cached = spduart_internals.dev9_uart_reg_area->r_scr;
			if ( (spduart_internals.spduart_msr_cached & 0x20) != 0 )
			{
				sceInetPrintf("spduart: Modem module detected\n");
				Dev9RegisterPowerOffHandler(1, shutdown_cb);
				memset(&v17, 0, sizeof(v17));
				spduart_internals.spduart_ef = CreateEventFlag(&v17);
				if ( spduart_internals.spduart_ef > 0 )
				{
					v18.attr = 0x2000000;
					v18.thread = (void (*)(void *))spduart_thread_proc;
					v18.option = 0;
					v18.priority = spduart_internals.spduart_thpri_;
					v18.stacksize = spduart_internals.spduart_thstack_;
					v16 = CreateThread(&v18);
					if ( v16 > 0 )
					{
						spduart_internals.spduart_thread = v16;
						if ( !StartThread(v16, &spduart_internals) )
						{
							SpdIntrDisable(4096);
							SpdRegisterIntrHandler(12, (dev9_intr_cb_t)spduart_dev9_intr_cb);
							USec2SysClock(10000u, &spduart_internals.spduart_clock_1e4);
							USec2SysClock(1000000u, &spduart_internals.spduart_clock_1e6);
							bzero(&spduart_internals.spduart_modem_ops, 108);
							spduart_internals.spduart_modem_ops.module_name = "spduart";
							spduart_internals.spduart_modem_ops.vendor_name = "SCE";
							spduart_internals.spduart_modem_ops.device_name = "Modem (Network Adaptor)";
							spduart_internals.spduart_modem_ops.bus_type = 5;
							spduart_internals.spduart_modem_ops.start = (int (*)(void *, int))spduart_op_start;
							spduart_internals.spduart_modem_ops.stop = (int (*)(void *, int))spduart_op_stop;
							spduart_internals.spduart_modem_ops.recv = (int (*)(void *, void *, int))spduart_op_recv;
							spduart_internals.spduart_modem_ops.send = (int (*)(void *, void *, int))spduart_op_send;
							spduart_internals.spduart_modem_ops.prot_ver = 0;
							spduart_internals.spduart_modem_ops.impl_ver = 0;
							spduart_internals.spduart_modem_ops.priv = &spduart_internals;
							spduart_internals.spduart_modem_ops.control = (int (*)(void *, int, void *, int))spduart_op_control;
							if ( sceModemRegisterDevice(&spduart_internals.spduart_modem_ops) >= 0 )
							{
#if 0
								return MODULE_REMOVABLE_END;
#else
								if ( mi && ((mi->newflags & 2) != 0) )
									mi->newflags |= 0x10;
								return MODULE_RESIDENT_END;
#endif
							}
							v2 += 1;
							TerminateThread(spduart_internals.spduart_thread);
						}
						v2 += 1;
						DeleteThread(spduart_internals.spduart_thread);
					}
					v2 += 1;
					DeleteEventFlag(spduart_internals.spduart_ef);
				}
			}
			DelayThread(100000);
		}
		sceInetPrintf("spduart: Modem module NOT detected\n");
		spduart_deinit();
		ReleaseLibraryEntries(&_exp_spduart);
	}
	sceInetPrintf("spduart: not resident end (fail%d)\n", v2);
	return MODULE_NO_RESIDENT_END;
}

static int module_stop(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	(void)ac;
	(void)av;
	(void)startaddr;
	(void)mi;

	if ( !spduart_internals.spduart_start_flag && !spduart_internals.spduart_stop_flag )
	{
		sceInetPrintf("spduart: can't unload (busy)\n");
		return MODULE_REMOVABLE_END;
	}
	sceModemUnregisterDevice(&spduart_internals.spduart_modem_ops);
	TerminateThread(spduart_internals.spduart_thread);
	DeleteThread(spduart_internals.spduart_thread);
	DeleteEventFlag(spduart_internals.spduart_ef);
	if ( spduart_internals.spduart_alarm_1e4_flag )
		CancelAlarm((unsigned int (*)(void *))alarm_cb_1e4, &spduart_internals);
	if ( spduart_internals.spduart_alarm_1e6_flag )
		CancelAlarm((unsigned int (*)(void *))alarm_cb_1e6, &spduart_internals);
	Dev9RegisterPowerOffHandler(1, 0);
	ReleaseLibraryEntries(&_exp_spduart);
	SpdIntrDisable(4096);
	spduart_deinit();
	return MODULE_NO_RESIDENT_END;
}
