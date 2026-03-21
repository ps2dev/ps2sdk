
#include "main.h"
#include "ipstack.h"

#ifdef BUILDING_SMAP_NETDEV
#include <thevent.h>
#endif

int SMAPCommonTxPacketNext(struct SmapDriverData *SmapDrivPrivData, void **data, void **pbuf)
{
#ifdef BUILDING_SMAP_MODULAR
    {
        const SmapModularHookTable_t *HookTableAt0;

        HookTableAt0 = SmapDrivPrivData->HookTable[0];
        if (HookTableAt0 != NULL && HookTableAt0->TxPacketNext != NULL) {
            int res;

            res = HookTableAt0->TxPacketNext(data);
            if (res > 0) {
                return res;
            }
        }
    }
#endif
#if defined(BUILDING_SMAP_NETMAN)
    (void)SmapDrivPrivData;
    (void)pbuf;
    return NetManTxPacketNext(data);
#elif defined(BUILDING_SMAP_PS2IP)
    (void)SmapDrivPrivData;
    (void)pbuf;
    return SMapTxPacketNext(data);
#elif defined(BUILDING_SMAP_NETDEV)
    while (1) {
        sceInetPkt_t *pkt;
        u8 *rp;
        unsigned int length;
        pkt = sceInetPktDeQ(&SmapDrivPrivData->m_devops.sndq);
        if (pkt == NULL) {
            return 0;
        }
        rp     = pkt->rp;
        length = pkt->wp - rp;
        if (length == 0 || ((uiptr)rp & 3) != 0) {
            sceInetPrintf("smap: dropped\n");
            SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Dropped += 1;

#ifdef BUILDING_SMAP_NETDEV
            SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Bytes += length;
            SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Packets += 1;
#endif

            sceInetFreePkt(&SmapDrivPrivData->m_devops, pkt);
            continue;
        }
        *pbuf = pkt;
        *data = rp;
        return length;
    }
#else
    (void)SmapDrivPrivData;
    (void)data;
    (void)pbuf;
    return 0;
#endif
}

void SMAPCommonTxPacketDeQ(struct SmapDriverData *SmapDrivPrivData, void **data, void **pbuf)
{
#ifdef BUILDING_SMAP_MODULAR
    {
        const SmapModularHookTable_t *HookTableAt0;

        HookTableAt0 = SmapDrivPrivData->HookTable[0];
        if (HookTableAt0 != NULL && HookTableAt0->TxPacketDeQ != NULL) {
            if (HookTableAt0->TxPacketDeQ(data)) {
                return;
            }
        }
    }
#else
    (void)data;
#endif
#if defined(BUILDING_SMAP_NETMAN)
    (void)SmapDrivPrivData;
    (void)pbuf;
    NetManTxPacketDeQ();
#elif defined(BUILDING_SMAP_PS2IP)
    (void)SmapDrivPrivData;
    (void)pbuf;
    SMapTxPacketDeQ();
#elif defined(BUILDING_SMAP_NETDEV)
    sceInetFreePkt(&SmapDrivPrivData->m_devops, *pbuf);
#else
    (void)SmapDrivPrivData;
    (void)pbuf;
#endif
}

void SMapCommonLinkStateDown(struct SmapDriverData *SmapDrivPrivData)
{
#ifdef BUILDING_SMAP_MODULAR
    {
        const SmapModularHookTable_t *HookTableAt0;

        HookTableAt0 = SmapDrivPrivData->HookTable[0];
        if (HookTableAt0 != NULL && HookTableAt0->LinkStateDown != NULL) {
            if (HookTableAt0->LinkStateDown()) {
                return;
            }
        }
    }
#endif
#if defined(BUILDING_SMAP_NETMAN)
    NetManToggleNetIFLinkState(SmapDrivPrivData->NetIFID, NETMAN_NETIF_ETH_LINK_STATE_DOWN);
#elif defined(BUILDING_SMAP_PS2IP)
    (void)SmapDrivPrivData;

    PS2IPLinkStateDown();
#else
    (void)SmapDrivPrivData;
#endif
}

void SMapCommonLinkStateUp(struct SmapDriverData *SmapDrivPrivData)
{
#ifdef BUILDING_SMAP_MODULAR
    {
        const SmapModularHookTable_t *HookTableAt0;

        HookTableAt0 = SmapDrivPrivData->HookTable[0];
        if (HookTableAt0 != NULL && HookTableAt0->LinkStateUp != NULL) {
            if (HookTableAt0->LinkStateUp()) {
                return;
            }
        }
    }
#endif
#if defined(BUILDING_SMAP_NETMAN)
    NetManToggleNetIFLinkState(SmapDrivPrivData->NetIFID, NETMAN_NETIF_ETH_LINK_STATE_UP);
#elif defined(BUILDING_SMAP_PS2IP)
    (void)SmapDrivPrivData;

    PS2IPLinkStateUp();
#elif defined(BUILDING_SMAP_NETDEV)
    SetEventFlag(SmapDrivPrivData->m_devops.evfid, sceInetDevEFP_StartDone);
#else
    (void)SmapDrivPrivData;
#endif
}

void *SMapCommonStackAllocRxPacket(struct SmapDriverData *SmapDrivPrivData, u16 LengthRounded, void **payload)
{
    void *pbuf;
#ifdef BUILDING_SMAP_MODULAR
    {
        const SmapModularHookTable_t *HookTableAt0;

        HookTableAt0 = SmapDrivPrivData->HookTable[0];
        if (HookTableAt0 != NULL && HookTableAt0->StackAllocRxPacket != NULL) {
            void *res;

            res = HookTableAt0->StackAllocRxPacket(LengthRounded, payload);
            if (res != NULL) {
                return res;
            }
        }
    }
#endif
#if defined(BUILDING_SMAP_NETMAN)
    (void)SmapDrivPrivData;
    pbuf = NetManNetProtStackAllocRxPacket(LengthRounded, payload);
#elif defined(BUILDING_SMAP_PS2IP)
    (void)SmapDrivPrivData;
    struct pbuf *pbuf_struct;

    pbuf_struct = pbuf_alloc(PBUF_RAW, LengthRounded, PBUF_POOL);
    pbuf        = (void *)pbuf_struct;

    if (pbuf_struct != NULL && payload != NULL) {
        *payload = pbuf_struct->payload;
    }
#elif defined(BUILDING_SMAP_NETDEV)
    sceInetPkt_t *pkt;
    pkt = sceInetAllocPkt(&SmapDrivPrivData->m_devops, LengthRounded);
    if (pkt != NULL && payload != NULL) {
        *payload = pkt->wp;
    }
    pbuf = pkt;
#else
    (void)SmapDrivPrivData;
    (void)LengthRounded;
    if (payload != NULL) {
        *payload = NULL;
    }
    pbuf = NULL;
#endif
    return pbuf;
}

void SMapStackEnQRxPacket(struct SmapDriverData *SmapDrivPrivData, void *pbuf, u16 length)
{
#ifdef BUILDING_SMAP_MODULAR
    {
        const SmapModularHookTable_t *HookTableAt0;

        HookTableAt0 = SmapDrivPrivData->HookTable[0];
        if (HookTableAt0 != NULL && HookTableAt0->EnQRxPacket != NULL) {
            if (HookTableAt0->EnQRxPacket(pbuf)) {
                return;
            }
        }
    }
#endif
#if defined(BUILDING_SMAP_NETMAN)
    (void)SmapDrivPrivData;
    (void)length;
    NetManNetProtStackEnQRxPacket(pbuf);
#elif defined(BUILDING_SMAP_PS2IP)
    (void)SmapDrivPrivData;
    (void)length;
    SMapLowLevelInput(pbuf);
#elif defined(BUILDING_SMAP_NETDEV)
    {
        sceInetPkt_t *pkt;

        pkt     = (sceInetPkt_t *)pbuf;
        pkt->wp = &pkt->rp[length];
        sceInetPktEnQ(&SmapDrivPrivData->m_devops.rcvq, pkt);
    }
#else
    (void)SmapDrivPrivData;
    (void)length;
    (void)pbuf;
#endif
}
