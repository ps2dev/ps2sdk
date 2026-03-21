
#include <errno.h>
#include <stdio.h>
#include <dmacman.h>
#include <dev9.h>
#include <intrman.h>
#include <loadcore.h>
#include <modload.h>
#include <stdio.h>
#include <sysclib.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#include <irx.h>

#include <smapregs.h>
#include <speedregs.h>

#include "main.h"

#include "xfer.h"
#include "ipstack.h"

static int SmapDmaTransfer(volatile u8 *smap_regbase, void *buffer, unsigned int size, int direction)
{
    unsigned int NumBlocks;
    int result;

    (void)smap_regbase;

    /*  Non-Sony: the original block size was (32*4 = 128) bytes.
        However, that resulted in slightly lower performance due to the IOP needing to copy more data.    */
    if ((NumBlocks = size >> 6) > 0) {
        if (SpdDmaTransfer(1, buffer, NumBlocks << 16 | 0x10, direction) >= 0) {
            result = NumBlocks << 6;
        } else
            result = 0;
    } else
        result = 0;

    return result;
}

static inline void CopyFromFIFO(volatile u8 *smap_regbase, void *buffer, unsigned int length, u16 RxBdPtr)
{
    int i, result;

    if (buffer == NULL) {
        return;
    }

    SMAP_REG16(SMAP_R_RXFIFO_RD_PTR) = RxBdPtr;

    result = SmapDmaTransfer(smap_regbase, buffer, length, DMAC_TO_MEM);

    for (i = result; (unsigned int)i < length; i += 4) {
        ((u32 *)buffer)[i / 4] = SMAP_REG32(SMAP_R_RXFIFO_DATA);
    }
}

static inline void CopyToFIFO(volatile u8 *smap_regbase, const void *buffer, unsigned int length)
{
    int i, result;

    if (buffer == NULL) {
        return;
    }

    result = SmapDmaTransfer(smap_regbase, (void *)buffer, length, DMAC_FROM_MEM);

    for (i = result; (unsigned int)i < length; i += 4) {
        SMAP_REG32(SMAP_R_TXFIFO_DATA) = ((u32 *)buffer)[i / 4];
    }
}

int HandleRxIntr(struct SmapDriverData *SmapDrivPrivData)
{
    USE_SMAP_RX_BD;
    int NumPacketsReceived, i;
    volatile smap_bd_t *PktBdPtr;
    // cppcheck-suppress constVariablePointer
    volatile u8 *smap_regbase;
    u16 ctrl_stat, length, pointer, LengthRounded;

    smap_regbase = SmapDrivPrivData->smap_regbase;

    NumPacketsReceived = 0;

    /*  Non-Sony: Workaround for the hardware BUG whereby the Rx FIFO of the MAL becomes unresponsive or loses frames when under load.
        Check that there are frames to process, before accessing the BD registers. */
    while (SMAP_REG8(SMAP_R_RXFIFO_FRAME_CNT) > 0) {
        PktBdPtr  = &rx_bd[SmapDrivPrivData->RxBDIndex % SMAP_BD_MAX_ENTRY];
        ctrl_stat = PktBdPtr->ctrl_stat;
        if (!(ctrl_stat & SMAP_BD_RX_EMPTY)) {
            length        = PktBdPtr->length;
            LengthRounded = (length + 3) & ~3;
            pointer       = PktBdPtr->pointer;

            if (ctrl_stat & (SMAP_BD_RX_INRANGE | SMAP_BD_RX_OUTRANGE | SMAP_BD_RX_FRMTOOLONG | SMAP_BD_RX_BADFCS | SMAP_BD_RX_ALIGNERR | SMAP_BD_RX_SHORTEVNT | SMAP_BD_RX_RUNTFRM | SMAP_BD_RX_OVERRUN)) {
                for (i = 0; i < 16; i++)
                    if ((ctrl_stat >> i) & 1) {
                        SmapDrivPrivData->RuntimeStats.RxErrorCount++;
#ifdef BUILDING_SMAP_NETDEV
                        SmapDrivPrivData->RuntimeStats_NetDev.m_RxErrorVarious[i] += 1;
#endif
                    }

                SmapDrivPrivData->RuntimeStats.RxDroppedFrameCount++;
#ifdef BUILDING_SMAP_NETDEV
                SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Errors += 1;
#endif

                if (ctrl_stat & SMAP_BD_RX_OVERRUN) {
                    SmapDrivPrivData->RuntimeStats.RxFrameOverrunCount++;
#ifdef BUILDING_SMAP_NETDEV
                    SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Over_Er += 1;
#endif
                }
                if (ctrl_stat & (SMAP_BD_RX_INRANGE | SMAP_BD_RX_OUTRANGE | SMAP_BD_RX_FRMTOOLONG | SMAP_BD_RX_SHORTEVNT | SMAP_BD_RX_RUNTFRM)) {
                    SmapDrivPrivData->RuntimeStats.RxFrameBadLengthCount++;
#ifdef BUILDING_SMAP_NETDEV
                    SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Length_Er += 1;
#endif
                }
                if (ctrl_stat & SMAP_BD_RX_BADFCS) {
                    SmapDrivPrivData->RuntimeStats.RxFrameBadFCSCount++;
#ifdef BUILDING_SMAP_NETDEV
                    SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Crc_Er += 1;
#endif
                }
                if (ctrl_stat & SMAP_BD_RX_ALIGNERR) {
                    SmapDrivPrivData->RuntimeStats.RxFrameBadAlignmentCount++;
#ifdef BUILDING_SMAP_NETDEV
                    SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Frame_Er += 1;
#endif
                }

                // Original did this whenever a frame is dropped.
                SMAP_REG16(SMAP_R_RXFIFO_RD_PTR) = pointer + LengthRounded;
            } else {
                void *pbuf, *payload;

                if ((pbuf = SMapCommonStackAllocRxPacket(SmapDrivPrivData, LengthRounded, &payload)) != NULL) {
                    CopyFromFIFO(SmapDrivPrivData->smap_regbase, payload, length, pointer);
#ifdef BUILDING_SMAP_NETDEV
                    SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Packets += 1;
                    SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Bytes += length;
                    if ((*((u8 *)payload) & 1) != 0) {
                        SmapDrivPrivData->RuntimeStats_NetDev.m_Multicast += 1;
                        if (*(u32 *)payload == (u32)-1 && *((u16 *)payload + 2) == 0xFFFF) {
                            SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Broadcast_Packets += 1;
                            SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Broadcast_Bytes += length;
                        } else {
                            SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Multicast_Packets += 1;
                            SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Multicast_Bytes += length;
                        }
                    }
#endif
                    SMapStackEnQRxPacket(SmapDrivPrivData, pbuf, length);
                    NumPacketsReceived++;
                } else {
                    SmapDrivPrivData->RuntimeStats.RxAllocFail++;
#ifdef BUILDING_SMAP_NETDEV
                    SmapDrivPrivData->RuntimeStats_NetDev.m_Rx_Dropped += 1;
#endif
                    // Original did this whenever a frame is dropped.
                    SMAP_REG16(SMAP_R_RXFIFO_RD_PTR) = pointer + LengthRounded;
                }
            }

            SMAP_REG8(SMAP_R_RXFIFO_FRAME_DEC) = 0;
            PktBdPtr->ctrl_stat                = SMAP_BD_RX_EMPTY;
            SmapDrivPrivData->RxBDIndex++;
        } else
            break;
    }

#ifdef BUILDING_SMAP_NETDEV
    if (NumPacketsReceived) {
        SetEventFlag(SmapDrivPrivData->m_devops.evfid, sceInetDevEFP_Recv);
    }
#endif

    return NumPacketsReceived;
}

int HandleTxReqs(struct SmapDriverData *SmapDrivPrivData)
{
    int result;
    void *data;
    void *pbuf;
    USE_SMAP_TX_BD;
    volatile smap_bd_t *BD_ptr;
    u16 BD_data_ptr;
    unsigned int SizeRounded;
    volatile u8 *smap_regbase;

    result       = 0;
    smap_regbase = SmapDrivPrivData->smap_regbase;
    while (1) {
        int length;

        data = NULL;
        if ((length = SMAPCommonTxPacketNext(SmapDrivPrivData, &data, &pbuf)) < 1) {
            return result;
        }
        SmapDrivPrivData->packetToSend = pbuf;

        if (SmapDrivPrivData->NumPacketsInTx >= SMAP_BD_MAX_ENTRY) {
            return result; // Queue full
        }
        SizeRounded = (length + 3) & ~3;

        if (SmapDrivPrivData->TxBufferSpaceAvailable < SizeRounded) {
            return result; // Out of FIFO space
        }

#ifdef BUILDING_SMAP_NETDEV
        if ((*((u8 *)data) & 1) != 0) {
            if (*(u32 *)data == (u32)-1 && *((u16 *)data + 2) == 0xFFFF) {
                SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Broadcast_Packets += 1;
                SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Broadcast_Bytes += length;
            } else {
                SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Multicast_Packets += 1;
                SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Multicast_Bytes += length;
            }
        }
#endif

        BD_data_ptr = SMAP_REG16(SMAP_R_TXFIFO_WR_PTR) + SMAP_TX_BASE;
        BD_ptr      = &tx_bd[SmapDrivPrivData->TxBDIndex % SMAP_BD_MAX_ENTRY];

        CopyToFIFO(smap_regbase, data, length);

        result++;
        BD_ptr->length                     = length;
        BD_ptr->pointer                    = BD_data_ptr;
        SMAP_REG8(SMAP_R_TXFIFO_FRAME_INC) = 0;
        BD_ptr->ctrl_stat                  = SMAP_BD_TX_READY | SMAP_BD_TX_GENFCS | SMAP_BD_TX_GENPAD;
        SmapDrivPrivData->TxBDIndex++;
        SmapDrivPrivData->NumPacketsInTx++;
        SmapDrivPrivData->TxBufferSpaceAvailable -= SizeRounded;

#ifdef BUILDING_SMAP_NETDEV
        SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Bytes += length;
        SmapDrivPrivData->RuntimeStats_NetDev.m_Tx_Packets += 1;
#endif

        SmapDrivPrivData->packetToSend = NULL;
        SMAPCommonTxPacketDeQ(SmapDrivPrivData, &data, &pbuf);
    }
}
