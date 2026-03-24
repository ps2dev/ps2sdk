
#include <irx_imports.h>
#include <netman.h>
#include <netdev.h>

static sceInetDevOps_t *g_ops;

static int NetDevAdaptorStart(void)
{
	if (!g_ops)
	{
		return 0;
	}
	g_ops->start(g_ops->priv, 0);
	return 0;
}

static void NetDevAdaptorStop(void)
{
	if (!g_ops)
	{
		return;
	}
	g_ops->stop(g_ops->priv, 0);
}

static void NetDevAdaptorXmit(void)
{
	if (!g_ops)
	{
		return;
	}
	g_ops->xmit(g_ops->priv, 0);
}

static int NetDevAdaptorIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length)
{
	int result;
	int ret2;
	int mode;

	(void)args_len;
	(void)length;

	if (!g_ops)
	{
		return 0;
	}

	result = 0;
	ret2 = 0;
	switch (command) {
		case NETMAN_NETIF_IOCTL_ETH_GET_MAC:
			memcpy(output, g_ops->hw_addr, (length > sizeof(g_ops->hw_addr)) ? sizeof(g_ops->hw_addr) : length);
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_LINK_MODE:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_NEGO_MODE, &ret2, sizeof(ret2));
			if (ret2 & 0x08)
				result = NETMAN_NETIF_ETH_LINK_MODE_100M_FDX; /* 100Base-TX FDX */
			if (ret2 & 0x04)
				result = NETMAN_NETIF_ETH_LINK_MODE_100M_HDX; /* 100Base-TX HDX */
			if (ret2 & 0x02)
				result = NETMAN_NETIF_ETH_LINK_MODE_10M_FDX; /* 10Base-TX FDX */
			if (ret2 & 0x01)
				result = NETMAN_NETIF_ETH_LINK_MODE_10M_HDX; /* 10Base-TX HDX */
			if (!(ret2 & 0x40))
				result |= NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE;
			break;
		case NETMAN_NETIF_IOCTL_GET_LINK_STATUS:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_LINK_STATUS, &ret2, sizeof(ret2));
			result = (ret2 > 0) ? NETMAN_NETIF_ETH_LINK_STATE_UP : NETMAN_NETIF_ETH_LINK_STATE_DOWN;
			break;
		case NETMAN_NETIF_IOCTL_GET_TX_DROPPED_COUNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_ERRORS, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_GET_RX_DROPPED_COUNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_ERRORS, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_RX_EOVERRUN_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_OVER_ER, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADLEN_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_LENGTH_ER, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADFCS_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_CRC_ER, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_RX_EBADALIGN_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_FRAME_ER, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_TX_ELOSSCR_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_CARRIER_ER, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_TX_EEDEFER_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_WINDOW_ER, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_TX_ECOLL_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_COLLISIONS, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_TX_EUNDERRUN_CNT:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_FIFO_ER, &result, sizeof(result));
			break;
		case NETMAN_NETIF_IOCTL_ETH_SET_LINK_MODE:
			int baseMode;

			mode = *(int *)args;
			baseMode = mode & (~NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE);
			ret2 = sceInetNDNEGO_AUTO | sceInetNDNEGO_TX_FD | sceInetNDNEGO_TX | sceInetNDNEGO_10_FD | sceInetNDNEGO_10;

			if (baseMode != NETMAN_NETIF_ETH_LINK_MODE_AUTO) {
				switch (baseMode) {
					case NETMAN_NETIF_ETH_LINK_MODE_10M_HDX:
						ret2 = sceInetNDNEGO_10;
						break;
					case NETMAN_NETIF_ETH_LINK_MODE_10M_FDX:
						ret2 = sceInetNDNEGO_10_FD;
						break;
					case NETMAN_NETIF_ETH_LINK_MODE_100M_HDX:
						ret2 = sceInetNDNEGO_TX;
						break;
					case NETMAN_NETIF_ETH_LINK_MODE_100M_FDX:
						ret2 = sceInetNDNEGO_TX_FD;
						break;
					default:
						break;
				}
			}
			if (!(mode & NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE))
				ret2 |= sceInetNDNEGO_PAUSE;
			g_ops->control(g_ops->priv, sceInetNDCC_SET_NEGO_MODE, &ret2, sizeof(ret2));
			break;
		case NETMAN_NETIF_IOCTL_ETH_GET_STATUS:
			g_ops->control(g_ops->priv, sceInetNDCC_GET_NEGO_MODE, &ret2, sizeof(ret2));
			if (ret2 & 0x08)
				result = NETMAN_NETIF_ETH_LINK_MODE_100M_FDX; /* 100Base-TX FDX */
			if (ret2 & 0x04)
				result = NETMAN_NETIF_ETH_LINK_MODE_100M_HDX; /* 100Base-TX HDX */
			if (ret2 & 0x02)
				result = NETMAN_NETIF_ETH_LINK_MODE_10M_FDX; /* 10Base-TX FDX */
			if (ret2 & 0x01)
				result = NETMAN_NETIF_ETH_LINK_MODE_10M_HDX; /* 10Base-TX HDX */
			if (!(ret2 & 0x40))
				result |= NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE;
			((struct NetManEthStatus *)output)->LinkMode = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_LINK_STATUS, &ret2, sizeof(ret2));
			result = (ret2 > 0) ? NETMAN_NETIF_ETH_LINK_STATE_UP : NETMAN_NETIF_ETH_LINK_STATE_DOWN;
			((struct NetManEthStatus *)output)->LinkStatus = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_ERRORS, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.RxDroppedFrameCount = result;
			// TODO: RxErrorCount not exposed through netdev ctrl
			((struct NetManEthStatus *)output)->stats.RxErrorCount = 0;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_OVER_ER, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.RxFrameOverrunCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_LENGTH_ER, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.RxFrameBadLengthCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_CRC_ER, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.RxFrameBadFCSCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_FRAME_ER, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.RxFrameBadAlignmentCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_ERRORS, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.TxDroppedFrameCount = result;
			// TODO: TxErrorCount not exposed through netdev ctrl
			((struct NetManEthStatus *)output)->stats.TxErrorCount = 0;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_CARRIER_ER, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.TxFrameLOSSCRCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_WINDOW_ER, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.TxFrameEDEFERCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_COLLISIONS, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.TxFrameCollisionCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_TX_FIFO_ER, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.TxFrameUnderrunCount = result;
			g_ops->control(g_ops->priv, sceInetNDCC_GET_RX_DROPPED, &result, sizeof(result));
			((struct NetManEthStatus *)output)->stats.RxAllocFail = result;
			result = 0;
			break;
		default:
			result = -1;
	}

	return result;
}

int sceInetRegisterNetDevice(sceInetDevOps_t *ops)
{
	struct NetManNetIF device;
	if (g_ops != NULL)
	{
		return -1;
	}
	strcpy(device.name, ops->module_name);
	device.init = &NetDevAdaptorStart;
	device.deinit = &NetDevAdaptorStop;
	device.xmit = &NetDevAdaptorXmit;
	device.ioctl = &NetDevAdaptorIoctl;
	if (NetManRegisterNetIF(&device) >= 0)
	{
		g_ops = ops;
	}
	return (g_ops != NULL) ? 0 : -1;
}

int sceInetUnregisterNetDevice(sceInetDevOps_t *ops)
{
	struct NetManNetIF device;
	if (g_ops != ops)
	{
		return -1;
	}
	strcpy(device.name, ops->module_name);
	NetManUnregisterNetIF(device.name);
	return 0;
}

void *sceInetAllocMem(sceInetDevOps_t *ops, int siz)
{
	// TODO: Currently stubbed
	return NULL;
}

void sceInetFreeMem(sceInetDevOps_t *ops, void *ptr)
{
	// TODO: Currently stubbed
}

void sceInetPktEnQ(sceInetPktQ_t *que, sceInetPkt_t *pkt)
{
	NetManNetProtStackEnQRxPacket(pkt->m_reserved1);
}

sceInetPkt_t *sceInetPktDeQ(sceInetPktQ_t *que)
{
	static sceInetPkt_t pkt;
	void *data;
	int len;
	memset(&pkt, 0, sizeof(pkt));

	data = NULL;
	len = NetManTxPacketNext(&data);
	pkt.rp = data;
	pkt.wp = (void *)(((u8 *)data) + len);
	return len ? &pkt : NULL;
}

unsigned int sceInetRand(void)
{
	// TODO: Currently stubbed
	return 0;
}

int sceInetPrintf(const char *fmt, ...)
{
	// TODO: Currently stubbed
	return 0;
}

sceInetPkt_t *sceInetAllocPkt(sceInetDevOps_t *ops, int siz)
{
	static sceInetPkt_t pkt;
	void *payload;
	void *pbuf;

	(void)ops;

	memset(&pkt, 0, sizeof(pkt));
	pbuf = NetManNetProtStackAllocRxPacket(siz, &payload);
	pkt.wp = payload;
	pkt.m_reserved1 = pbuf;
	return pbuf ? &pkt : NULL;
}

void sceInetFreePkt(sceInetDevOps_t *ops, sceInetPkt_t *pkt)
{
	(void)ops;
	(void)pkt;

	NetManTxPacketDeQ();
}
