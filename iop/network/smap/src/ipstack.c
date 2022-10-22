
#include "main.h"
#include "ipstack.h"

int SMAPCommonTxPacketNext(struct SmapDriverData *SmapDrivPrivData, void **data)
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
    return NetManTxPacketNext(data);
#elif defined(BUILDING_SMAP_PS2IP)
    return SMapTxPacketNext(data);
#else
    return 0;
#endif
}

void SMAPCommonTxPacketDeQ(struct SmapDriverData *SmapDrivPrivData, void **data)
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
    NetManTxPacketDeQ();
#elif defined(BUILDING_SMAP_PS2IP)
    SMapTxPacketDeQ();
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
    pbuf = NetManNetProtStackAllocRxPacket(LengthRounded, payload);
#elif defined(BUILDING_SMAP_PS2IP)
    struct pbuf *pbuf_struct;

    pbuf_struct = pbuf_alloc(PBUF_RAW, LengthRounded, PBUF_POOL);
    pbuf        = (void *)pbuf_struct;

    if (pbuf_struct != NULL && payload != NULL) {
        *payload = pbuf_struct->payload;
    }
#else
    (void)LengthRounded;
    if (payload != NULL) {
        *payload = NULL;
    }
    pbuf = NULL;
#endif
    return pbuf;
}

void SMapStackEnQRxPacket(struct SmapDriverData *SmapDrivPrivData, void *pbuf)
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
    NetManNetProtStackEnQRxPacket(pbuf);
#elif defined(BUILDING_SMAP_PS2IP)
    SMapLowLevelInput(pbuf);
#else
    (void)pbuf;
#endif
}
