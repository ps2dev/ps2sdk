/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <irx_imports.h>
#include <ctype.h>
#include <kerr.h>
#include <usbd_macro.h>

#define MODNAME "INET_AN986_driver"

#ifdef AN986_UEPCB
IRX_ID(MODNAME, 1, 80);
#else
IRX_ID(MODNAME, 1, 75);
// Based on the module from SCE SDK 2.5.3
#endif

struct an986_priv
{
	int m_magic_cur;
	int m_is_pegasus2;
	int m_thid;
	int m_efid;
	int m_cfgval;
	int m_ef_wait_retval;
	int m_done_related;
	int m_ctrl_pipe;
	int m_bulk_in_pipe;
	int m_bulk_out_pipe;
	int m_int_in_pipe;
	u8 m_usb_xfer_buf[0x10C];
	sceInetDevOps_t m_devops;
	u8 m_hwaddr_tmp[8];
	int m_val_for_inet_stop;
	int m_start_stop_flag;
	int m_val_for_inet_start;
	int m_timer_active;
	iop_sys_clock_t m_sysclk;
	int m_link_status;
	int m_nego_status;
	int m_val_for_alarm_cb;
	u16 m_usb_ctrl_buf[2];
	int m_cnt_for_bulk_xfer;
	int m_rx_packets;
	int m_tx_packets;
	int m_rx_bytes;
	int m_tx_bytes;
	int m_rx_errors;
	int m_tx_errors;
	int m_rx_dropped;
	int m_tx_dropped;
	int m_multicast;
	int m_collisions;
	int m_err_rx_length;
	int m_err_rx_over;
	int m_err_rx_crc;
	int m_err_rx_frame;
	int m_err_rx_fifo;
	int m_err_rx_missed;
	int m_err_tx_aborted;
	int m_err_tx_carrier;
	int m_err_tx_fifo;
	int m_err_tx_heartbeat;
	int m_err_tx_window;
};

struct an986_devinfo
{
	int m_chip;
	int m_vendor_id;
	const char *m_vendor_name;
	int m_product_id;
	const char *m_product_name;
};

struct an986_idata
{
	// Unofficial: move to bss
	sceUsbdLddOps m_an986_ldd;
	// Unofficial: move to bss
	int m_thpri;
	// Unofficial: move to bss
	int m_thstack;
	// Unofficial: move to bss
	int m_magic_count;
	// Unofficial: move to bss
	int m_verbose;
	int m_resident_flag;
	int m_load_mode;
};

#define VERBOSE_PRINTF(...)                                                                                            \
	{                                                                                                                    \
		if ( g_an986_idata.m_verbose )                                                                                     \
		{                                                                                                                  \
			printf(__VA_ARGS__);                                                                                             \
		}                                                                                                                  \
	}

static void bulk_xfer(struct an986_priv *priv);

extern struct irx_export_table _exp_an986;
static struct an986_devinfo g_an986_devinfo_custom;
static const struct an986_devinfo g_an986_devinfo[] = {
#ifdef AN986_UEPCB
	{'P', 0x07a6, "ADMtek", 0x8513, "PegasusIII"},
	{'P', 0x0b9a, "namco", 0x0500, "System246 UE PCB"},
#else
	{'k', 0x03e8, "AOX", 0x0008, "101"},
	{'p', 0x0411, "Melco", 0x0001, "LUA-TX"},
	{'p', 0x0411, "Melco", 0x0005, "LUA-TX"},
	{'P', 0x0411, "Melco", 0x0009, "LUA2-TX"},
	{'-', 0x0411, "Melco", 0x0012, "LUA-KTX"},
	{'-', 0x0423, "CATC", 0x000a, "NetMate"},
	{'-', 0x0423, "CATC", 0x000c, "NetMate2"},
	{'k', 0x04bb, "I-O Data", 0x0901, "ET/T"},
	{'p', 0x04bb, "I-O Data", 0x0904, "ET/TX"},
	{'k', 0x0506, "3Com", 0x03e8, "3C19250"},
	{'k', 0x0557, "ATEN", 0x2002, "UC-10T"},
	{'k', 0x0557, "ATEN", 0x4000, "DSB-650"},
	{'k', 0x0565, "Peracom", 0x0002, "Enet"},
	{'k', 0x0565, "Peracom", 0x0005, "Enet2"},
	{'k', 0x056e, "Elecom", 0x4000, "LD-USB/T"},
	{'p', 0x056e, "Elecom", 0x4002, "LD-USB/TX"},
	{'P', 0x056e, "Elecom", 0x4005, "LD-USBL/TX"},
	{'k', 0x05e9, "KLSI", 0x0008, "KL5KUSB101B"},
	{'p', 0x05e9, "KLSI", 0x0009, "Pegasus"},
	{'k', 0x066b, "Linksys", 0x2202, "USB10T"},
	{'p', 0x066b, "Linksys", 0x2203, "USB100TX"},
	{'p', 0x066b, "Linksys", 0x2204, "USB100TX"},
	{'p', 0x066b, "Linksys", 0x2206, "USB"},
	{'P', 0x066b, "Linksys", 0x400b, "USB100TX B"},
	{'k', 0x06e1, "ADS", 0x0008, "USBS-10B"},
	{'k', 0x0707, "SMC", 0x0100, "2202"},
	{'p', 0x0707, "SMC", 0x0200, "2202"},
	{'p', 0x07a6, "ADMtek", 0x0986, "Pegasus"},
	{'P', 0x07a6, "ADMtek", 0x8511, "PegasusII"},
	{'k', 0x07aa, "Corega", 0x0001, "USB-T"},
	{'p', 0x07aa, "Corega", 0x0004, "USB-TX"},
	{'P', 0x07aa, "Corega", 0x000d, "USB-TXS"},
	{'p', 0x07b8, "D-Link", 0xabc1, "DU-E10"},
	{'k', 0x07b8, "D-Link", 0x4000, "DU-E10"},
	{'p', 0x07b8, "D-Link", 0x4002, "DU-E100"},
	{'P', 0x07b8, "D-Link", 0x4102, "DU-E100 B1"},
	{'p', 0x083a, "Accton", 0x1046, "USB10/100"},
	{'k', 0x0846, "NetGear", 0x1001, "EA101"},
	{'p', 0x08dd, "Billionton", 0x0986, "USB100N"},
	{'p', 0x08dd, "Billionton", 0x0987, "USBLP-100"},
	{'p', 0x08dd, "Billionton", 0x0988, "USBEL-100"},
	{'P', 0x08dd, "Billionton", 0x8511, "USBE-100"},
	{'k', 0x13d2, "Shark", 0x0400, "Pocket"},
	{'-', 0x1485, "PSION DACOM", 0x0002, "Gold Port"},
	{'p', 0x15e8, "SOHOware", 0x9100, "NUB100"},
	{'k', 0x1645, "Entrega", 0x0005, "E45"},
	{'k', 0x2001, "D-Link", 0x4000, "DSB-650C"},
	{'p', 0x2001, "D-Link", 0x4001, "DSB-650TX"},
	{'p', 0x2001, "D-Link", 0x4002, "DSB-650TX"},
	{'p', 0x2001, "D-Link", 0x4003, "DSB-650TX-PNA"},
	{'P', 0x2001, "D-Link", 0x400b, "DSB-650TX B1"},
	{'p', 0x2001, "D-Link", 0xabc1, "DSB-650"},
#endif
};
#ifndef AN986_UEPCB
static const char *g_version_ptr = "Version 1.75.0";
#endif
// Unofficial: move into structure
static struct an986_idata g_an986_idata;

static int ef_wait_wrap(struct an986_priv *priv, u32 efbits)
{
	int efret;
	u32 efres;

	efret = WaitEventFlag(priv->m_efid, efbits, WEF_OR | WEF_CLEAR, &efres);
	if ( efret )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("WaitEventFlag (%d)", efret);
		VERBOSE_PRINTF("\n");
		return -1;
	}
	return priv->m_ef_wait_retval;
}

static void ef_set_wrap(struct an986_priv *priv, int wait_retval, u32 efbits)
{
	int efret;

	priv->m_ef_wait_retval = wait_retval;
	efret = SetEventFlag(priv->m_efid, efbits);
	if ( efret )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("SetEventFlag (%d)", efret);
		VERBOSE_PRINTF("\n");
	}
}

static void an986_done(int efbits, int doneval, void *userdata)
{
	struct an986_priv *priv;

	priv = (struct an986_priv *)userdata;
	if ( efbits )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("%s: -> 0x%x\n", "an986_done", efbits);
		VERBOSE_PRINTF("\n");
	}
	priv->m_done_related = doneval;
	ef_set_wrap(priv, efbits, 4);
}

static int control_in_xfer(struct an986_priv *priv, int xferoffs, int xferlen)
{
	int xferret;

	xferret = sceUsbdControlTransfer(
		priv->m_ctrl_pipe,
		USB_DIR_IN | 0x40,
		0xF0,
		0,
		xferoffs,
		(xferlen < 2) ? 2 : xferlen,
		&priv->m_usb_xfer_buf[xferoffs],
		an986_done,
		priv);
	if ( xferret )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("sceUsbdControlTransfer -> 0x%x", xferret);
		VERBOSE_PRINTF("\n");
		return -1;
	}
	return ef_wait_wrap(priv, 4);
}

static int control_out_xfer(struct an986_priv *priv, int xferoffs, int xferlen)
{
	int xferret;

	xferret = sceUsbdControlTransfer(
		priv->m_ctrl_pipe,
		USB_DIR_OUT | 0x40,
		0xF1,
		0,
		xferoffs,
		(xferlen < 2) ? 2 : xferlen,
		&priv->m_usb_xfer_buf[xferoffs],
		an986_done,
		priv);
	if ( xferret )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("sceUsbdControlTransfer -> 0x%x", xferret);
		VERBOSE_PRINTF("\n");
		return -1;
	}
	return ef_wait_wrap(priv, 4);
}

static int control_inout_xfer(struct an986_priv *priv, char linkval, char xval, u16 *outptr)
{
	int retres;

	// PHY address
	priv->m_usb_xfer_buf[0x25] = linkval & 0x1F;
	priv->m_usb_xfer_buf[0x26] = 0;
	priv->m_usb_xfer_buf[0x27] = 0;
	// PHY access control
	priv->m_usb_xfer_buf[0x28] = (xval & 0x1F) | 0x40;
	retres = control_out_xfer(priv, 0x25, 4);
	if ( retres )
		return retres;
	while ( 1 )
	{
		retres = control_in_xfer(priv, 0x28, 1);
		if ( retres )
			return retres;
		if ( !!(priv->m_usb_xfer_buf[0x28] & 0x80) )
			break;
		DelayThread(10000);
	}
	retres = control_in_xfer(priv, 0x25, 4);
	if ( retres )
		return retres;
	*outptr = priv->m_usb_xfer_buf[0x26] | (priv->m_usb_xfer_buf[0x27] << 8);
	return 0;
}

static void an986_rx_done(int aresult, int acount, void *userdata)
{
	struct an986_priv *priv;
	sceInetPkt_t *pkt;
	u8 rp_cur;

	pkt = (sceInetPkt_t *)userdata;
	priv = (struct an986_priv *)pkt->m_reserved1;
	if ( aresult )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("%s: -> 0x%x\n", "an986_rx_done", aresult);
		VERBOSE_PRINTF("\n");
	}
	priv->m_rx_packets += 1;
	pkt->m_reserved1 = NULL;
	if ( priv->m_start_stop_flag || priv->m_val_for_inet_stop )
		sceInetFreePkt(&priv->m_devops, pkt);
	else if ( acount < 68 )
	{
		priv->m_rx_errors += 1;
		sceInetFreePkt(&priv->m_devops, pkt);
	}
	else
	{
#ifdef AN986_UEPCB
		int rp_xe;

		rp_xe = (pkt->rp[1] << 8) + pkt->rp[0];
		rp_cur = pkt->rp[rp_xe - 2];
#else
		rp_cur = pkt->rp[acount - 2];
#endif
		priv->m_multicast += !!(rp_cur & 1);
		priv->m_err_rx_length += !!(rp_cur & 2);
		priv->m_err_rx_length += !!(rp_cur & 4);
		priv->m_err_rx_crc += !!(rp_cur & 8);
		priv->m_err_rx_frame += !!(rp_cur & 0x10);
		if ( (rp_cur & 0x1E) )
		{
			priv->m_rx_errors += 1;
			sceInetFreePkt(&priv->m_devops, pkt);
		}
		else
		{
#ifdef AN986_UEPCB
			priv->m_rx_bytes += rp_xe - 8;
			pkt->wp += rp_xe - 6;
			pkt->rp += 2;
#else
			priv->m_rx_bytes += acount - 8;
			pkt->wp += acount - 8;
#endif
			sceInetPktEnQ(&priv->m_devops.rcvq, pkt);
			SetEventFlag(priv->m_devops.evfid, sceInetDevEFP_Recv);
		}
	}
	bulk_xfer(priv);
	priv->m_val_for_alarm_cb = 10;
}

static void bulk_xfer(struct an986_priv *priv)
{
	sceInetPkt_t *pkt;
	int xferret;
	int state;

#ifdef AN986_UEPCB
	pkt = sceInetAllocPkt(&priv->m_devops, sizeof(sceInetPkt_t) + 1500 + 2);
#else
	pkt = sceInetAllocPkt(&priv->m_devops, sizeof(sceInetPkt_t) + 1500);
#endif
	if ( !pkt )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
#ifdef AN986_UEPCB
		VERBOSE_PRINTF("sceInetAllocPkt(%d) - no space", (int)(sizeof(sceInetPkt_t) + 1500 + 2));
#else
		VERBOSE_PRINTF("sceInetAllocPkt(%d) - no space", (int)(sizeof(sceInetPkt_t) + 1500));
#endif
		VERBOSE_PRINTF("\n");
		CpuSuspendIntr(&state);
		priv->m_cnt_for_bulk_xfer += 1;
		CpuResumeIntr(state);
		return;
	}
	pkt->m_reserved1 = (void *)priv;
#ifdef AN986_UEPCB
	xferret = sceUsbdBulkTransfer(priv->m_bulk_in_pipe, pkt->wp, sizeof(sceInetPkt_t) + 1500, an986_rx_done, pkt);
#else
	pkt->rp += 2;
	pkt->wp += 2;
	xferret = sceUsbdBulkTransfer(priv->m_bulk_in_pipe, pkt->wp, sizeof(sceInetPkt_t) + 1500 - 2, an986_rx_done, pkt);
#endif
	if ( xferret )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("sceUsbdBulkTransfer -> 0x%x\n", xferret);
		VERBOSE_PRINTF("\n");
#ifdef AN986_UEPCB
		printf("sceUsbdBulkTransfer -> 0x%x\n", xferret);
#endif
		sceInetFreePkt(&priv->m_devops, pkt);
	}
}

static void an986_tx_done(int aresult, int acount, void *userdata)
{
	struct an986_priv *priv;
	sceInetPkt_t *pkt;

	(void)acount;
	pkt = (sceInetPkt_t *)userdata;
	priv = (struct an986_priv *)pkt->m_reserved1;
	if ( aresult )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("%s: -> 0x%x\n", "an986_tx_done", aresult);
		VERBOSE_PRINTF("\n");
	}
	pkt->m_reserved1 = NULL;
	sceInetFreePkt(&priv->m_devops, pkt);
}

static unsigned int alarm_cb(void *userdata)
{
	struct an986_priv *priv;

	priv = (struct an986_priv *)userdata;
	priv->m_val_for_alarm_cb -= !!((int)(priv->m_val_for_alarm_cb) > 0);
	return priv->m_sysclk.lo;
}

static int an986_inet_start(void *userdata, int unused)
{
	struct an986_priv *priv;

	(void)unused;
	priv = (struct an986_priv *)userdata;
	priv->m_start_stop_flag = 0;
	if ( priv->m_val_for_inet_start )
		SetEventFlag(priv->m_devops.evfid, sceInetDevEFP_StartDone);
	else
		ef_set_wrap(priv, 0, 2);
	return 0;
}

static int an986_inet_stop(void *userdata, int unused)
{
	struct an986_priv *priv;

	(void)unused;
	priv = (struct an986_priv *)userdata;
	priv->m_start_stop_flag = 1;
	if ( !priv->m_val_for_inet_stop )
		return 0;
	TerminateThread(priv->m_thid);
	DeleteThread(priv->m_thid);
	DeleteEventFlag(priv->m_efid);
	if ( priv->m_timer_active )
		CancelAlarm(alarm_cb, priv);
	sceInetUnregisterNetDevice(&priv->m_devops);
	sceInetFreeMem(&priv->m_devops, priv);
	return 0;
}

static int an986_inet_xmit(void *userdata, int unused)
{
	int xferret;
	sceInetPkt_t *pkt;
	u32 pktsz;
	struct an986_priv *priv;
	int dropped;

	(void)unused;
	priv = (struct an986_priv *)userdata;
	dropped = 0;
	xferret = -1;
	pkt = sceInetPktDeQ(&priv->m_devops.sndq);
	dropped = !pkt;
	dropped = !!(!dropped && (priv->m_start_stop_flag || priv->m_val_for_inet_stop || !priv->m_link_status));
	if ( !dropped )
	{
		pktsz = pkt->wp - pkt->rp;
		dropped = !!(pktsz - 60 >= 1455);
	}
	if ( !dropped )
	{
		pkt->rp -= 2;
		dropped = !!((uiptr)(pkt->rp) & 3);
	}
	if ( !dropped )
	{
		*((u16 *)(pkt->rp)) = pktsz;
		priv->m_tx_packets += 1;
		priv->m_tx_bytes += pktsz;
		pktsz += 2 + !(((u8)pktsz) & 0x3F);
		pkt->m_reserved1 = (void *)priv;
		while ( 1 )
		{
			xferret = sceUsbdBulkTransfer(priv->m_bulk_out_pipe, pkt->rp + 2, pktsz, an986_tx_done, pkt);
			if ( !xferret )
				break;
			if ( xferret != USB_RC_IOREQ )
			{
				VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
				VERBOSE_PRINTF("sceUsbdBulkTransfer -> 0x%x", xferret);
				VERBOSE_PRINTF("\n");
				dropped = 1;
				break;
			}
			DelayThread(10000);
		}
	}
	if ( dropped )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("dropped");
		VERBOSE_PRINTF("\n");
		priv->m_tx_dropped += 1;
		sceInetFreePkt(&priv->m_devops, pkt);
	}
	priv->m_val_for_alarm_cb = 10;
	return xferret;
}

static int inet_81040000_multicast_list_handler(struct an986_priv *priv, u8 *ptr, int len)
{
	// Multicast address
	// Unofficial: use memset
	memset(&priv->m_usb_xfer_buf[0x08], 0, 8);
	if ( len >= 0 )
	{
		if ( len != 6 * (len / 6) )
			return -512;
		if ( ptr )
		{
			int k;
			for ( k = 0; k < (len / 6); k += 1 )
			{
				if ( !!(*ptr & 1) )
				{
					unsigned int valcr2;
					int i;
					unsigned int xcurval;

					valcr2 = 0xFFFFFFFF;
					for ( i = 0; i < 6; i += 1 )
					{
						int j;

						for ( j = 0; j < 8; j += 1 )
						{
							xcurval = valcr2 >> 1;
							xcurval ^= !!(((u8)valcr2 ^ (u8)(*ptr >> j)) & 1) ? 0xEDB88320 : 0;
							valcr2 = xcurval;
						}
						ptr += 1;
					}
					priv->m_usb_xfer_buf[((u8)(xcurval & 0x3F) >> 3) + 8] |= 1 << (xcurval & 7);
				}
			}
		}
	}
	else
	{
		if ( ptr )
			return -512;
		memset(&priv->m_usb_xfer_buf[0x08], 0xFF, 8);
	}
	return control_out_xfer(priv, 0x08, 8);
}

static int an986_inet_control(void *userdata, int code, void *ptr, int len)
{
	int retres;
	const int *src_int_ptr;
	int priority;
	struct an986_priv *priv;

	priv = (struct an986_priv *)userdata;
	retres = -512;
	src_int_ptr = NULL;
	switch ( code )
	{
		case sceInetNDCC_GET_THPRI:
			retres = g_an986_idata.m_thpri;
			break;
		case sceInetNDCC_GET_IF_TYPE:
			retres = sceInetNDIFT_ETHERNET;
			break;
		case sceInetNDCC_GET_RX_PACKETS:
			src_int_ptr = &priv->m_rx_packets;
			break;
		case sceInetNDCC_GET_TX_PACKETS:
			src_int_ptr = &priv->m_tx_packets;
			break;
		case sceInetNDCC_GET_RX_BYTES:
			src_int_ptr = &priv->m_rx_bytes;
			break;
		case sceInetNDCC_GET_TX_BYTES:
			src_int_ptr = &priv->m_tx_bytes;
			break;
		case sceInetNDCC_GET_RX_ERRORS:
			src_int_ptr = &priv->m_rx_errors;
			break;
		case sceInetNDCC_GET_TX_ERRORS:
			src_int_ptr = &priv->m_tx_errors;
			break;
		case sceInetNDCC_GET_RX_DROPPED:
			src_int_ptr = &priv->m_rx_dropped;
			break;
		case sceInetNDCC_GET_TX_DROPPED:
			src_int_ptr = &priv->m_tx_dropped;
			break;
		case sceInetNDCC_GET_MULTICAST:
			src_int_ptr = &priv->m_multicast;
			break;
		case sceInetNDCC_GET_COLLISIONS:
			src_int_ptr = &priv->m_collisions;
			break;
		case sceInetNDCC_GET_RX_LENGTH_ER:
			src_int_ptr = &priv->m_err_rx_length;
			break;
		case sceInetNDCC_GET_RX_OVER_ER:
			src_int_ptr = &priv->m_err_rx_over;
			break;
		case sceInetNDCC_GET_RX_CRC_ER:
			src_int_ptr = &priv->m_err_rx_crc;
			break;
		case sceInetNDCC_GET_RX_FRAME_ER:
			src_int_ptr = &priv->m_err_rx_frame;
			break;
		case sceInetNDCC_GET_RX_FIFO_ER:
			src_int_ptr = &priv->m_err_rx_fifo;
			break;
		case sceInetNDCC_GET_RX_MISSED_ER:
			src_int_ptr = &priv->m_err_rx_missed;
			break;
		case sceInetNDCC_GET_TX_ABORTED_ER:
			src_int_ptr = &priv->m_err_tx_aborted;
			break;
		case sceInetNDCC_GET_TX_CARRIER_ER:
			src_int_ptr = &priv->m_err_tx_carrier;
			break;
		case sceInetNDCC_GET_TX_FIFO_ER:
			src_int_ptr = &priv->m_err_tx_fifo;
			break;
		case sceInetNDCC_GET_TX_HEARTBEAT_ER:
			src_int_ptr = &priv->m_err_tx_heartbeat;
			break;
		case sceInetNDCC_GET_TX_WINDOW_ER:
			src_int_ptr = &priv->m_err_tx_window;
			break;
		case sceInetNDCC_GET_NEGO_STATUS:
			retres = (priv->m_link_status > 0) ? priv->m_nego_status : 0;
			break;
		case sceInetNDCC_GET_LINK_STATUS:
			retres = priv->m_link_status;
			break;
		case sceInetNDCC_SET_THPRI:
			if ( !ptr )
				break;
			if ( len != sizeof(priority) )
				break;
			// Unofficial: use memcpy
			memcpy(&priority, ptr, sizeof(priority));
			retres = KE_ILLEGAL_PRIORITY;
			if ( (unsigned int)(priority - 9) >= 0x73 )
				break;
			g_an986_idata.m_thpri = priority;
			retres = ChangeThreadPriority(priv->m_thid, priority);
			break;
		case sceInetNDCC_SET_MULTICAST_LIST:
			retres = inet_81040000_multicast_list_handler(priv, ptr, len);
			break;
	}
	if ( src_int_ptr && ptr && len == sizeof(*src_int_ptr) )
	{
		// Unofficial: use memcpy
		memcpy(ptr, src_int_ptr, sizeof(*src_int_ptr));
		retres = 0;
	}
	return retres;
}

static void inet_thread_proc(void *userdata)
{
	int xferret;
	int regres;
	int k;
	int j;
	int i;
	int l;
	u16 outval_1;
	u16 outval_2;
	u16 outval_3;
	int state;
	struct an986_priv *priv;

	priv = (struct an986_priv *)userdata;
	if ( ef_wait_wrap(priv, 1) )
		return;
	xferret = sceUsbdSetConfiguration(priv->m_ctrl_pipe, priv->m_cfgval, an986_done, priv);
	if ( xferret )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("sceUsbdSetConfiguration -> 0x%x", xferret);
		VERBOSE_PRINTF("\n");
		return;
	}
	if ( ef_wait_wrap(priv, 4) || control_in_xfer(priv, 0x10, 6) )
		return;
#ifdef AN986_UEPCB
	// Ethernet_control_2 load EEPROM start
	priv->m_usb_xfer_buf[0x02] = (1 << 5);
	priv->m_usb_xfer_buf[0x03] = 0;
	if ( control_out_xfer(priv, 0x02, 2) )
		return;
	priv->m_usb_xfer_buf[0x02] = 0;
	if ( control_out_xfer(priv, 0x02, 2) )
		return;
	// AN986_UEPCB note: An unsed variable is set to 0xB2500010
#endif
	for ( i = 0; i < 3; i += 1 )
	{
		// EEPROM offset
		priv->m_usb_xfer_buf[0x20] = i;
		priv->m_usb_xfer_buf[0x21] = 0;
		priv->m_usb_xfer_buf[0x22] = 0;
		// EEPROM access control
		priv->m_usb_xfer_buf[0x23] = 2;
		if ( control_out_xfer(priv, 0x20, 4) )
			return;
		while ( 1 )
		{
			if ( control_in_xfer(priv, 0x23, 1) )
				return;
			if ( !!(priv->m_usb_xfer_buf[0x23] & 4) )
				break;
			DelayThread(10000);
		}
		if ( control_in_xfer(priv, 0x21, 3) )
			return;
		priv->m_hwaddr_tmp[(i * 2) + 0] = priv->m_usb_xfer_buf[0x21];
		priv->m_hwaddr_tmp[(i * 2) + 1] = priv->m_usb_xfer_buf[0x22];
#ifdef AN986_UEPCB
		printf("%d %x %x\n", i, priv->m_usb_xfer_buf[0x21], priv->m_usb_xfer_buf[0x22]);
#endif
	}
	// Unofficial: use memcpy
	memcpy(&priv->m_usb_xfer_buf[0x10], priv->m_hwaddr_tmp, 6);
	if ( control_out_xfer(priv, 0x10, 6) )
		return;
	// Unofficial: use memcpy
	memcpy(priv->m_devops.hw_addr, priv->m_hwaddr_tmp, 6);
	priv->m_link_status = -1;
	regres = sceInetRegisterNetDevice(&priv->m_devops);
	if ( regres < 0 )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("sceInetRegisterNetDevice -> %d", regres);
		VERBOSE_PRINTF("\n");
		return;
	}
	if ( ef_wait_wrap(priv, 2) )
		return;
	// GPIO
	priv->m_usb_xfer_buf[0x7E] = 36;
	priv->m_usb_xfer_buf[0x7F] = 6;
	if ( control_out_xfer(priv, 0x7E, 2) )
		return;
	// GPIO
	priv->m_usb_xfer_buf[0x7E] = 38;
	priv->m_usb_xfer_buf[0x7F] = 4;
	if ( control_out_xfer(priv, 0x7E, 2) )
		return;
#ifdef AN986_UEPCB
	// Reserved (undocumented)
	priv->m_usb_xfer_buf[0x83] = 0xFF;
	priv->m_usb_xfer_buf[0x84] = 1;
	if ( control_out_xfer(priv, 0x83, 2) )
		return;
#endif
	if ( priv->m_is_pegasus2 )
	{
#ifdef AN986_UEPCB
		// PHY control
		printf("set reset\n");
		priv->m_usb_xfer_buf[0x7B] = 1;
		if ( control_out_xfer(priv, 0x7B, 1) )
			return;
		priv->m_usb_xfer_buf[0x7B] = 2;
		if ( control_out_xfer(priv, 0x7B, 1) )
		{
			printf("set ng\n");
			return;
		}
		printf("set ok\n");
#else
		// PHY control
		priv->m_usb_xfer_buf[0x7B] = 3;
		if ( control_out_xfer(priv, 0x7B, 1) )
			return;
		priv->m_usb_xfer_buf[0x7B] = 2;
		if ( control_out_xfer(priv, 0x7B, 1) )
			return;
#endif
	}
	// Ethernet_control_1 reset_mac
	priv->m_usb_xfer_buf[0x01] = 8;
	if ( control_out_xfer(priv, 0x01, 1) )
		return;
	while ( 1 )
	{
		if ( control_in_xfer(priv, 0x01, 1) )
			return;
		if ( !(priv->m_usb_xfer_buf[0x01] & 8) )
			break;
		DelayThread(10000);
	}
	k = 0;
	j = 0;
	outval_1 = 0;
	while ( (outval_1 & 0x24) != 0x24 )
	{
		if ( control_inout_xfer(priv, k, 1, &outval_1) )
			return;
		if ( outval_1 == 0xFFFF )
		{
			k += 1;
			if ( k >= 32 )
			{
				VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
				VERBOSE_PRINTF("Valid PHY chip not found");
				VERBOSE_PRINTF("\n");
				return;
			}
		}
		else
		{
			if ( (outval_1 & 0x24) == 0x24 )
				break;
			DelayThread(100000);
			j += 1;
			if ( j >= 30 )
				priv->m_link_status = 0;
		}
	}
	priv->m_link_status = 1;
	printf("%s: Auto-Nego complete and valid link detected (%d,BMSR=%04x)\n", priv->m_devops.interface, k, outval_1);
	if (
		control_inout_xfer(priv, k, 4, priv->m_usb_ctrl_buf) || control_inout_xfer(priv, k, 5, &priv->m_usb_ctrl_buf[1]) )
		return;
	priv->m_usb_xfer_buf[0x01] = 0;
	outval_1 = priv->m_usb_ctrl_buf[0] & priv->m_usb_ctrl_buf[1];
	// Ethernet_control_1 full_duplex
	priv->m_usb_xfer_buf[0x01] |= !!(outval_1 & 0x140) ? 0x20 : 0;
	// Ethernet_control_1 10mode
	priv->m_usb_xfer_buf[0x01] |= !!(outval_1 & 0x180) ? 0x10 : 0;
	if ( control_out_xfer(priv, 0x01, 1) )
		return;
	priv->m_nego_status = (!!(outval_1 & 0x180)) ? ((!!(outval_1 & 0x140)) ? sceInetNDNEGO_TX_FD : sceInetNDNEGO_TX) :
																								 ((!!(outval_1 & 0x140)) ? sceInetNDNEGO_10_FD : sceInetNDNEGO_10);
	printf(
		"%s: %s %s Duplex Mode (ANAR=0x%04x ANLPAR=0x%04x)\n",
		priv->m_devops.interface,
		(!!(outval_1 & 0x180)) ? "100BaseTX" : "10BaseT",
		(!!(outval_1 & 0x140)) ? "Full" : "Half",
		priv->m_usb_ctrl_buf[0],
		priv->m_usb_ctrl_buf[1]);
	if ( control_inout_xfer(priv, k, 2, &outval_2) || control_inout_xfer(priv, k, 3, &outval_3) )
		return;
	printf(
		"%s: PHY OUI=0x%06x MODEL=0x%02x REV=0x%x (0x%04x,0x%04x)\n",
		priv->m_devops.interface,
		(outval_2 << 6) | (outval_3 >> 10),
		(outval_3 >> 4) & 0x1F,
		outval_3 & 0xF,
		outval_2,
		outval_3);
#ifdef AN986_UEPCB
	// Reserved (undocumented)
	priv->m_usb_xfer_buf[0x80] = 0xE5;
	priv->m_usb_xfer_buf[0x81] = 2;
	if ( control_out_xfer(priv, 0x80, 2) )
		return;
#endif
	// Ethernet_control_0
	priv->m_usb_xfer_buf[0x00] = 0xC9;
	if ( control_out_xfer(priv, 0x00, 1) )
		return;
	if ( priv->m_is_pegasus2 )
	{
		// GPIOs
		priv->m_usb_xfer_buf[0x7C] = 0x34;
		priv->m_usb_xfer_buf[0x7E] = 0x26;
		priv->m_usb_xfer_buf[0x7F] = 0x30;
		if ( control_out_xfer(priv, 0x7C, 4) )
			return;
	}
	for ( i = 0; i < 8; i += 1 )
		bulk_xfer(priv);
#ifdef AN986_UEPCB
	printf("rxstart1\n");
#endif
	priv->m_val_for_inet_start = 1;
	if ( !priv->m_start_stop_flag )
		SetEventFlag(priv->m_devops.evfid, sceInetDevEFP_StartDone);
	priv->m_val_for_alarm_cb = 10;
	USec2SysClock(1000000, &priv->m_sysclk);
	SetAlarm(&priv->m_sysclk, alarm_cb, priv);
	l = 0;
	priv->m_timer_active = 1;
	while ( 1 )
	{
		while ( priv->m_val_for_alarm_cb > 0 )
		{
			control_in_xfer(priv, 0x2B, 5);
			// transmit_status_1
			priv->m_collisions += !!(priv->m_usb_xfer_buf[0x2B] & 0x60);
			priv->m_err_tx_carrier += !!(priv->m_usb_xfer_buf[0x2B] & 0xC);
			priv->m_tx_errors += !!(priv->m_usb_xfer_buf[0x2B] & 0x6C);
			// receive_status
			priv->m_err_rx_over += !!(priv->m_usb_xfer_buf[0x2D] & 1);
			priv->m_rx_errors += !!(priv->m_usb_xfer_buf[0x2D] & 1);
			// receive lost packet low
			priv->m_err_rx_missed += priv->m_usb_xfer_buf[0x2F];
			priv->m_rx_errors += priv->m_usb_xfer_buf[0x2F];
			DelayThread(100000);
			l += 1;
			if ( l >= 11 )
			{
				l = 0;
				if ( priv->m_cnt_for_bulk_xfer > 0 )
				{
					CpuSuspendIntr(&state);
					priv->m_cnt_for_bulk_xfer -= 1;
					CpuResumeIntr(state);
					bulk_xfer(priv);
#ifdef AN986_UEPCB
					printf("rxstart2\n");
#endif
				}
			}
		}
		if ( control_inout_xfer(priv, k, 1, &outval_1) )
			return;
		if ( !(outval_1 & 4) )
		{
			priv->m_link_status = 0;
			while ( (outval_1 & 0x24) != 0x24 )
			{
				if ( control_inout_xfer(priv, k, 1, &outval_1) )
					return;
				if ( (outval_1 & 0x24) == 0x24 )
					break;
				DelayThread(100000);
			}
			priv->m_link_status = 1;
		}
		priv->m_val_for_alarm_cb = 10;
	}
}

static struct an986_priv *do_allocate_mem_for_inet(const char *vendor_name, const char *device_name, int is_pegasus2)
{
	struct an986_priv *priv;
	int err;
	iop_event_t efparam;
	iop_thread_t thparam;

	err = 0;
	priv = (struct an986_priv *)sceInetAllocMem(NULL, sizeof(struct an986_priv));
	if ( !priv )
	{
		// Unofficial: don't reference null priv->m_devops.interface
		VERBOSE_PRINTF("%s: ", "an986");
		VERBOSE_PRINTF("sceInetAllocMem(%d) -> no space or not ready", (int)sizeof(struct an986_priv));
		VERBOSE_PRINTF("\n");
		return priv;
	}
	// Unofficial: use memset
	memset(priv, 0, sizeof(struct an986_priv));
	priv->m_is_pegasus2 = is_pegasus2;
	priv->m_magic_cur = g_an986_idata.m_magic_count;
	sprintf(priv->m_devops.interface, "an986,%d", priv->m_magic_cur);
	g_an986_idata.m_magic_count += 1;
	priv->m_devops.module_name = "an986";
	priv->m_devops.prot_ver = sceInetDevProtVer;
	priv->m_devops.flags = sceInetDevF_Multicast | sceInetDevF_ARP;
	priv->m_devops.start = an986_inet_start;
	priv->m_devops.stop = an986_inet_stop;
	priv->m_devops.xmit = an986_inet_xmit;
	priv->m_devops.control = an986_inet_control;
	priv->m_devops.vendor_name = (char *)vendor_name;
	priv->m_devops.device_name = (char *)device_name;
	priv->m_devops.impl_ver = 0;
	priv->m_devops.priv = priv;
	priv->m_devops.mtu = 1500;
	memset(&efparam, 0, sizeof(efparam));
	priv->m_efid = CreateEventFlag(&efparam);
	if ( priv->m_efid <= 0 )
	{
		VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
		VERBOSE_PRINTF("CreateEventFlag -> %d", priv->m_efid);
		VERBOSE_PRINTF("\n");
		err = 1;
	}
	if ( !err )
	{
		thparam.attr = TH_C;
		thparam.thread = inet_thread_proc;
		thparam.option = 0;
		thparam.priority = g_an986_idata.m_thpri;
		thparam.stacksize = g_an986_idata.m_thstack;
		priv->m_thid = CreateThread(&thparam);
		if ( priv->m_thid <= 0 )
		{
			VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
			VERBOSE_PRINTF("CreateThread -> %d", priv->m_thid);
			VERBOSE_PRINTF("\n");
			err = 1;
		}
	}
	if ( !err )
	{
		err = StartThread(priv->m_thid, priv);
		if ( err )
		{
			VERBOSE_PRINTF("%s: ", priv->m_devops.interface);
			VERBOSE_PRINTF("StartThread -> %d", err);
			VERBOSE_PRINTF("\n");
			err = 1;
		}
	}
	if ( err )
	{
		if ( priv->m_thid > 0 )
			DeleteThread(priv->m_thid);
		if ( priv->m_efid > 0 )
			DeleteEventFlag(priv->m_efid);
		sceInetFreeMem(&priv->m_devops, priv);
		priv = NULL;
	}
	return priv;
}

static const struct an986_devinfo *do_check_static_descriptor(int is_probe, u16 id_vendor, u16 id_product)
{
	const struct an986_devinfo *cur_devinfo;

#ifdef AN986_UEPCB
	printf("andev %d %x %x\n", is_probe, id_vendor, id_product);
#endif
	if ( is_probe )
		VERBOSE_PRINTF("an986: idVendor=0x%04x idProduct=0x%04x\n", id_vendor, id_product);
	cur_devinfo = NULL;
	if ( id_vendor == g_an986_devinfo_custom.m_vendor_id && id_product == g_an986_devinfo_custom.m_product_id )
		cur_devinfo = &g_an986_devinfo_custom;
	if ( !cur_devinfo )
	{
		unsigned int i;

		// Unofficial: avoid out of bounds read when device not found
		for ( i = 0; i < (sizeof(g_an986_devinfo) / sizeof(g_an986_devinfo[0])); i += 1 )
		{
			if ( id_vendor == g_an986_devinfo[i].m_vendor_id && id_product == g_an986_devinfo[i].m_product_id )
			{
				cur_devinfo = &g_an986_devinfo[i];
				break;
			}
		}
	}
	if ( cur_devinfo )
	{
		if ( is_probe )
			VERBOSE_PRINTF("an986: %s, %s", cur_devinfo->m_vendor_name, cur_devinfo->m_product_name);
		switch ( cur_devinfo->m_chip )
		{
			case 'p':
				if ( is_probe )
					VERBOSE_PRINTF(" [pegasus] -> supported\n");
				return cur_devinfo;
			case 'P':
				if ( is_probe )
					VERBOSE_PRINTF(" [pegasusII] -> supported\n");
				return cur_devinfo;
			case 'k':
				if ( is_probe )
					VERBOSE_PRINTF(" [klsi] -> unsupported\n");
				return NULL;
			default:
				break;
		}
	}
	if ( is_probe )
		VERBOSE_PRINTF(" [unknown] -> unsupported\n");
	return NULL;
}

static int an986_attach(int devId)
{
	UsbDeviceDescriptor *devdesc;
	const struct an986_devinfo *cur_devinfo;
	UsbConfigDescriptor *cfgdesc;
	UsbInterfaceDescriptor *intfdesc;
	UsbEndpointDescriptor *bulk_in_desc;
	UsbEndpointDescriptor *bulk_out_desc;
	UsbEndpointDescriptor *int_in_desc;
	struct an986_priv *priv;

#ifdef AN986_UEPCB
	printf("an986_attach start\n");
#endif
	VERBOSE_PRINTF("an986_attach,%d: called\n", devId);
	devdesc = (UsbDeviceDescriptor *)sceUsbdScanStaticDescriptor(devId, NULL, USB_DT_DEVICE);
	if ( !devdesc )
		return -1;
	cur_devinfo = do_check_static_descriptor(0, devdesc->idVendor, devdesc->idProduct);
	if ( !cur_devinfo )
		return -1;
	cfgdesc = (UsbConfigDescriptor *)sceUsbdScanStaticDescriptor(devId, devdesc, USB_DT_CONFIG);
	if ( !cfgdesc )
		return -1;
	if ( cfgdesc->bNumInterfaces != 1 )
		return -1;
	intfdesc = (UsbInterfaceDescriptor *)sceUsbdScanStaticDescriptor(devId, cfgdesc, USB_DT_INTERFACE);
	if ( !intfdesc )
		return -1;
	if ( intfdesc->bNumEndpoints != 3 )
		return -1;
	bulk_in_desc = (UsbEndpointDescriptor *)sceUsbdScanStaticDescriptor(devId, intfdesc, USB_DT_ENDPOINT);
	if ( !bulk_in_desc )
		return -1;
	if ( (bulk_in_desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) != USB_DIR_IN )
		return -1;
	if ( (bulk_in_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_BULK )
		return -1;
	bulk_out_desc = (UsbEndpointDescriptor *)sceUsbdScanStaticDescriptor(devId, bulk_in_desc, USB_DT_ENDPOINT);
	if ( !bulk_out_desc )
		return -1;
	if ( (bulk_out_desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) != USB_DIR_OUT )
		return -1;
	if ( (bulk_out_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_BULK )
		return -1;
	int_in_desc = (UsbEndpointDescriptor *)sceUsbdScanStaticDescriptor(devId, bulk_out_desc, USB_DT_ENDPOINT);
	if ( !int_in_desc )
		return -1;
	if ( (int_in_desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) != USB_DIR_IN )
		return -1;
	if ( (int_in_desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT )
		return -1;
	priv = do_allocate_mem_for_inet(cur_devinfo->m_vendor_name, cur_devinfo->m_product_name, cur_devinfo->m_chip == 'P');
	if ( !priv )
		return -1;
	priv->m_ctrl_pipe = sceUsbdOpenPipe(devId, NULL);
	if ( priv->m_ctrl_pipe < 0 )
		return -1;
	priv->m_bulk_in_pipe = sceUsbdOpenPipe(devId, bulk_in_desc);
	if ( priv->m_bulk_in_pipe < 0 )
		return -1;
	priv->m_bulk_out_pipe = sceUsbdOpenPipeAligned(devId, bulk_out_desc);
	if ( priv->m_bulk_out_pipe < 0 )
		return -1;
	priv->m_int_in_pipe = sceUsbdOpenPipe(devId, int_in_desc);
	if ( priv->m_int_in_pipe < 0 )
		return -1;
	sceUsbdSetPrivateData(devId, priv);
	priv->m_devops.bus_type = sceInetBus_USB;
	sceUsbdGetDeviceLocation(devId, priv->m_devops.bus_loc);
	priv->m_cfgval = cfgdesc->bConfigurationValue;
	ef_set_wrap(priv, 0, 1);
	VERBOSE_PRINTF("an986_attach,%d: -> attached\n", devId);
#ifdef AN986_UEPCB
	printf("an986_attach end\n");
#endif
	return 0;
}

static int an986_detach(int devId)
{
	struct an986_priv *priv;

	VERBOSE_PRINTF("an986_detach,%d: -> detached\n", devId);
	priv = (struct an986_priv *)sceUsbdGetPrivateData(devId);
	if ( !priv )
		return -1;
	priv->m_val_for_inet_stop = 1;
	SetEventFlag(priv->m_devops.evfid, sceInetDevEFP_PlugOut);
	return 0;
}

static int an986_probe(int devId)
{
	const UsbDeviceDescriptor *devdesc;

	if ( g_an986_idata.m_verbose )
	{
		UsbStringDescriptor *gendesc;
		int i;
		char strlocbuf[16];

		printf("an986_probe,%d: called", devId);
		if ( sceUsbdGetDeviceLocation(devId, (u8 *)strlocbuf) )
			printf(" dev_id=%d\n", devId);
		else
		{
			printf(" Loc:USB-");
			for ( i = 0; i < 7 && strlocbuf[i]; i += 1 )
				printf("%s%d", i ? "," : "", strlocbuf[i]);
		}
		gendesc = NULL;
		while ( 1 )
		{
			printf("\n");
			gendesc = (UsbStringDescriptor *)sceUsbdScanStaticDescriptor(devId, gendesc, 0);
			if ( !gendesc )
				break;
			for ( i = 0; i < gendesc->bLength; i += 1 )
				printf(" %02x", ((u8 *)gendesc)[i]);
		}
	}
	devdesc = (UsbDeviceDescriptor *)sceUsbdScanStaticDescriptor(devId, NULL, USB_DT_DEVICE);
	if ( !devdesc )
		return 0;
	if ( !do_check_static_descriptor(1, devdesc->idVendor, devdesc->idProduct) )
		return 0;
	g_an986_idata.m_resident_flag = 1;
	// AN986_UEPCB note: add 16 to unused variable
	if ( g_an986_idata.m_load_mode == 't' )
		return 0;
	VERBOSE_PRINTF("an986_probe,%d: -> accepted\n", devId);
	return 1;
}

static int do_print_version(void)
{
#ifdef AN986_UEPCB
	printf("AN986 1.80.0\n");
#else
	printf("AN986 (%s)\n", g_version_ptr);
#endif
	return 1;
}

static int do_print_help(void)
{
	do_print_version();
	printf("Usage: an986 [-verbose] [-list] [thpri=<prio>] [thstack=<stack>] [-p <id>] [-P <id>]\n");
	return 2;
}

static int scan_number(const char *e_arg, unsigned int *n_result)
{
	const char *e_arg_1;
	unsigned int curbasex;
	unsigned int curnum;

	e_arg_1 = e_arg;
	curbasex = 10;
	if ( *e_arg_1 == '0' && e_arg_1[1] )
	{
		e_arg_1 += 1;
		curbasex = 8;
		if ( *e_arg_1 == 'x' )
		{
			e_arg_1 += 1;
			curbasex = 16;
		}
	}
	curnum = 0;
	if ( *e_arg_1 )
	{
		while ( 1 )
		{
			u32 e_arg_1_num;

			e_arg_1_num = (((u8)*e_arg_1)) - '0';
			if ( ((u8)*e_arg_1) - (unsigned int)'0' >= 0xA )
			{
				e_arg_1_num = (((u8)*e_arg_1)) - 'W';
				if ( ((u8)*e_arg_1) - (unsigned int)'a' >= 6 )
					break;
			}
			if ( e_arg_1_num >= curbasex )
				break;
			e_arg_1 += 1;
			curnum = curnum * curbasex + e_arg_1_num;
			if ( !*e_arg_1 )
			{
				*n_result = curnum;
				return 0;
			}
		}
	}
	printf("%s: %s - invalid digit\n", "scan_number", e_arg);
	return -1;
}

static int do_print_list(void)
{
	unsigned int i;

	do_print_version();
	printf("  VID   PID   Vendor          Device          Chip\n");
	printf("------------------------------------------------------\n");
	for ( i = 0; i < (sizeof(g_an986_devinfo) / sizeof(g_an986_devinfo[0])); i += 1 )
	{
		printf("  %04x", g_an986_devinfo[i].m_vendor_id);
		printf("  %04x", g_an986_devinfo[i].m_product_id);
		printf("  %-14s", g_an986_devinfo[i].m_vendor_name);
		printf("  %-14s", g_an986_devinfo[i].m_product_name);
		switch ( g_an986_devinfo[i].m_chip )
		{
			case 'p':
				printf("  Pegasus");
				break;
			case 'P':
				printf("  PegasusII");
				break;
			case 'k':
				printf("  KLSI");
				break;
			default:
				printf("  Unknown");
				break;
		}
		printf("\n");
	}
	return 3;
}

static int an986_init(int ac, char **av)
{
	int i;
	const char *chr_num_ptr;
	unsigned int vidpidtmp;

	g_an986_idata.m_thpri = 40;
	g_an986_idata.m_thstack = 0x4000;
	g_an986_idata.m_magic_count = 0;
	g_an986_idata.m_verbose = 0;
	g_an986_idata.m_load_mode = 'n';
	g_an986_idata.m_resident_flag = 1;
	g_an986_devinfo_custom.m_chip = '-';
	g_an986_devinfo_custom.m_vendor_id = 0x0000;
	g_an986_devinfo_custom.m_vendor_name = "Unknown";
	g_an986_devinfo_custom.m_product_id = 0x0000;
	g_an986_devinfo_custom.m_product_name = "Unknown";
#ifdef AN986_UEPCB
	printf("debug %s\n", av[0]);
#endif
	for ( i = 1; i < ac; i += 1 )
	{
		if ( !strcmp("-help", av[i]) )
			return do_print_help();
		else if ( !strcmp("-version", av[i]) )
			return do_print_version();
		else if ( !strcmp("-verbose", av[i]) )
			g_an986_idata.m_verbose = 1;
		else if ( !strcmp("-list", av[i]) )
			return do_print_list();
		else if ( !strcmp("-p", av[i]) || !strcmp("-P", av[i]) )
		{
			g_an986_devinfo_custom.m_chip = av[i][1];
			i += 1;
			if ( i >= ac || scan_number((char *)av[i], &vidpidtmp) )
				return do_print_help();
			g_an986_devinfo_custom.m_vendor_id = (vidpidtmp >> 16) & 0xFFFF;
			g_an986_devinfo_custom.m_product_id = vidpidtmp & 0xFFFF;
		}
		else if ( !strncmp("thpri=", av[i], 6) )
		{
			chr_num_ptr = &av[i][6];
			if ( !isdigit(*chr_num_ptr) )
				return do_print_help();
			g_an986_idata.m_thpri = strtol(chr_num_ptr, NULL, 10);
			if ( (unsigned int)(g_an986_idata.m_thpri - 9) >= 0x73 )
				return do_print_help();
			while ( *chr_num_ptr && isdigit(*chr_num_ptr) )
				chr_num_ptr += 1;
			if ( *chr_num_ptr )
				return do_print_help();
		}
		else if ( !strncmp("thstack=", av[i], 8) )
		{
			chr_num_ptr = &av[i][8];
			if ( !isdigit(*chr_num_ptr) )
				return do_print_help();
			g_an986_idata.m_thstack = strtol(chr_num_ptr, NULL, 10);
			while ( *chr_num_ptr && isdigit(*chr_num_ptr) )
				chr_num_ptr += 1;
			if ( !strcmp(chr_num_ptr, "KB") )
			{
				g_an986_idata.m_thstack <<= 10;
				chr_num_ptr += 2;
			}
			if ( *chr_num_ptr )
				return do_print_help();
		}
		else if ( !strcmp("AUTOLOAD", av[i]) || !strcmp("lmode=AUTOLOAD", av[i]) )
			g_an986_idata.m_load_mode = 'a';
		else if ( !strcmp("TESTLOAD", av[i]) || !strcmp("lmode=TESTLOAD", av[i]) )
			g_an986_idata.m_load_mode = 't';
		else
			return do_print_help();
	}
	if ( g_an986_idata.m_load_mode != 'n' )
		g_an986_idata.m_resident_flag = 0;
	memset(&g_an986_idata.m_an986_ldd, 0, sizeof(g_an986_idata.m_an986_ldd));
	g_an986_idata.m_an986_ldd.name = "an986";
	g_an986_idata.m_an986_ldd.probe = &an986_probe;
	g_an986_idata.m_an986_ldd.connect = &an986_attach;
	g_an986_idata.m_an986_ldd.disconnect = &an986_detach;
	if ( sceUsbdRegisterLdd(&g_an986_idata.m_an986_ldd) )
		return 4;
	VERBOSE_PRINTF(
		"an986_start: load_mode='%c' resident_flag=%d\n", g_an986_idata.m_load_mode, g_an986_idata.m_resident_flag);
	if ( g_an986_idata.m_load_mode == 't' )
	{
		sceUsbdUnregisterLdd(&g_an986_idata.m_an986_ldd);
		return 5;
	}
	else if ( g_an986_idata.m_resident_flag )
	{
		do_print_version();
		return 0;
	}
	sceUsbdUnregisterLdd(&g_an986_idata.m_an986_ldd);
	return 6;
}

int _start(int ac, char **av)
{
	int retres;

	if ( RegisterLibraryEntries(&_exp_an986) )
	{
		printf("an986: module already loaded\n");
		return MODULE_NO_RESIDENT_END;
	}
	retres = an986_init(ac, av);
	VERBOSE_PRINTF("an986: an986_init() -> 0x%x\n", retres);
	if ( retres )
	{
		ReleaseLibraryEntries(&_exp_an986);
		return (retres << 4) | (g_an986_idata.m_resident_flag ? 4 : 0) | MODULE_NO_RESIDENT_END;
	}
	return MODULE_RESIDENT_END;
}
