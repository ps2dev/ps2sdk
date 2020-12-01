/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/** 
 * @file Structs and enums used in packet2 library
 * @defgroup packet2_types Types
 * Structures and enums used in libpacket2.
 * @ingroup packet2
 * @{
 */

#ifndef __PACKET2_DATA_TYPES_H__
#define __PACKET2_DATA_TYPES_H__

#include <tamtypes.h>

/**
 * Two modes are commonly used with the DMA Controller (DMAC): normal mode and 
 * chain mode. In normal mode, transfers are issued one at a time. In chain mode, a 
 * series of transfers are issued simultaneously by initiating a transfer that contains 
 * "DMA Tags", which in turn contains instructions for further transfers. You will 
 * probably use chain mode for the majority of your transfers. 
 */
enum Packet2Mode
{
    P2_MODE_NORMAL = 0,
    P2_MODE_CHAIN = 1,
};

/** Types of memory mapping. */
enum Packet2Type
{
    /** Normal. */
    P2_TYPE_NORMAL = 0x00000000,
    /** Uncached. */
    P2_TYPE_UNCACHED = 0x20000000,
    /** Uncached accelerated. */
    P2_TYPE_UNCACHED_ACCL = 0x30000000,
    /** Scratchpad memory. */
    P2_TYPE_SPRAM = 0x70000000
};

/** DMA tag types */
enum DmaTagType
{
    /** Transfers the QWC qword from the ADDR field, clears the Dn_CHCR.STR to 0, and ends transfer. */
    P2_DMA_TAG_REFE = 0,
    /** Transfers the QWC qword following the tag and reads the succeeding qword as the next tag. */
    P2_DMA_TAG_CNT = 1,
    /** Transfers the QWC qword following the tag and reads the qword of the ADDR field as the next tag. */
    P2_DMA_TAG_NEXT = 2,
    /** Transfers the QWC qword from the ADDR field and reads the qword following the tag as the next tag.  */
    P2_DMA_TAG_REF = 3,
    /** 
     * Transfers the QWC qword from the ADDR field while controlling stalls 
     * and reads the qword following the tag as the next tag.
     * @note Effective only on the VIF1, GIF, and SIF1 channels. 
     */
    P2_DMA_TAG_REFS = 4,
    /** 
     * Transfers the QWC qword following the tag, pushes the next field into 
     * the Dn_ASR register, and reads the qword of the ADDR field as the next tag. 
     * @note Effective only on the VIF0, VIF1, and GIF channels. 
     * Addresses can be pushed up to 2 levels 
     */
    P2_DMA_TAG_CALL = 5,
    /** 
     * Transfers the QWC qword following the tag and reads the 
     * qword of the field popped from the Dn_ASR register as the 
     * next tag. 
     * Transfers the QWC qword following the tag, clears the 
     * Dn_CHCR.STR to 0, and ends transfer when there is no 
     * pushed address. 
     * @note Effective only on the VIF0, VIF1, and GIF channels. 
     */
    P2_DMA_TAG_RET = 6,
    /** Transfers the QWC qword following the tag, clears the Dn_CHCR.STR to 0, and ends transfer. */
    P2_DMA_TAG_END = 7
};

/** Destination chain tag. */
typedef struct
{
    /** Quadword count. */
    u64 QWC : 16;
    u64 PAD : 10;
    /** 
     * Priority Control Enable. 
     * 00 Nothing performed 
     * 01 Reserved 
     * 10 Priority setting disabled (D_PCR.PCE = 0) 
     * 11 Priority setting enabled (D_PCR.PCE = 1) 
     */
    u64 PCE : 2;
    /** Tag ID. (look at DmaTagType) */
    u64 ID : 3;
    /** 
     * Interrupt Request. 
     * 0 No interrupt request 
     * 1 Interrupt request at end of packet transfer 
     */
    u64 IRQ : 1;
    /** 
     * Address. 
     * Address of packet or next tag instruction 
     * (With qword alignment, lower 4 bits become 0.) 
     */
    u64 ADDR : 31;
    /** 
     * Memory/SPR Selection. 
     * 0 Memory address 
     * 1 SPR address 
     */
    u64 SPR : 1;
    u32 OPT1;
    u32 OPT2;
} dma_tag_t __attribute__((aligned(16)));

/** 
 * Unpack modes. 
 * V2 = 2 dimensions, S = Single = 1 dimension 
 * _32 = 32 bits of data length.  
 */
enum UnpackMode
{
    P2_UNPACK_S_32 = 0x00,
    P2_UNPACK_S_16 = 0x01,
    P2_UNPACK_S_8 = 0x02,
    P2_UNPACK_V2_32 = 0x04,
    P2_UNPACK_V2_16 = 0x05,
    P2_UNPACK_V2_8 = 0x06,
    P2_UNPACK_V3_32 = 0x08,
    P2_UNPACK_V3_16 = 0x09,
    P2_UNPACK_V3_8 = 0x0A,
    P2_UNPACK_V4_32 = 0x0C,
    P2_UNPACK_V4_16 = 0x0D,
    P2_UNPACK_V4_8 = 0x0E,
    P2_UNPACK_V4_5 = 0x0F,
};

/** VIF opcodes. */
enum VIFOpcode
{
    /** 
     * No operation. 
     * No operation is performed. 
     * NOP is used to adjust the data alignment in the VIF packet. 
     */
    P2_VIF_NOP = 0,
    /** 
     * Sets write recycle. 
     * STCYCL writes the value of the IMMEDIATE field to the VIFn_CYCLE register. 
     */
    P2_VIF_STCYCL = 1,
    /** 
     * Sets the double buffer offset. 
     * OFFSET writes the lower 10 bits of the IMMEDIATE field to the VIF1_OFST register. 
     * At the same time, the DBF flag of the VIF1_STAT register is cleared to 0, and the VIF1_BASE register 
     * value is set to the VIF1_TOPS. That is, the pointer for the double buffer points to the BASE. 
     */
    P2_VIF_OFFSET = 2,
    /** 
     * Sets the base address of the double buffer. 
     * BASE writes the lower 10 bits of the IMMEDIATE field to the VIF1_BASE register. 
     * These bits become the base address of the double buffers. 
     */
    P2_VIF_BASE = 3,
    /** 
     * Sets the data pointer. 
     * ITOP writes the lower 10 bits of the IMMEDIATE field to the VIFn_ITOPS register. 
     * This value is read from the VU by the XITOP instruction. 
     */
    P2_VIF_ITOP = 4,
    /** 
     * Sets the addition decompression mode. 
     * STMOD writes the lower 2 bits of the IMMEDIATE field to the VIFn_MODE register. 
     * This becomes the addition decompression mode setting. 
     */
    P2_VIF_STMOD = 5,
    /** 
     * Sets the PATH3 mask. 
     * MSKPATH3 enables/disables transfer processing via the GIF PATH3. 
     * The setting of this register applies to the next data block or later. 
     */
    P2_VIF_MSKPATH3 = 6,
    /** 
     * Sets the MARK value. 
     * MARK writes the value of the IMMEDIATE field to the VIFn_MARK register. 
     * By properly setting the MARK value, it becomes possible to use this value for 
     * synchronization with the EE Core and debugging. 
     */
    P2_VIF_MARK = 7,
    /** 
     * Waits for end of the microprogram. 
     * FLUSHE waits for the state in which the microprogram in VU0/VU1 has been ended. 
     */
    P2_VIF_FLUSHE = 16,
    /** 
     * Waits for end of the microprogram. 
     * FLUSH waits for the state in which transfers to the GIF from PATH1 and PATH2 have been ended after 
     * end of microprogram in VU1. FLUSH does not wait for the end of the transfer via PATH1 by 
     * the XGKICK instruction with the E bit. 
     */
    P2_VIF_FLUSH = 17,
    /** 
     * Waits for end of the microprogram. 
     * FLUSHA waits for the state in which there is no transfer request from PATH3 after the end of micro 
     * program in VU1 and end of transfer to the GIF from PATH1 and PATH2. 
     */
    P2_VIF_FLUSHA = 18,
    /** 
     * Activates the microprogram. 
     * MSCAL waits for the end of the microprogram under execution and activates the micro program with the 
     * value of the IMMEDIATE field as the starting address. 
     */
    P2_VIF_MSCAL = 20,
    /** 
     * Activates the microprogram. 
     * MSCNT waits for the end of the microprogram under execution and executes the next microprogram from 
     * the address following the most recently ended one in the PC (program counter). 
     */
    P2_VIF_MSCNT = 23,
    /** 
     * Activates the microprogram. 
     * MSCALF waits for the end of both the microprogram and the GIF(PATH1/PATH2) transfer and executes 
     * the microprogram with the value of the IMMEDIATE field as the starting address. 
     */
    P2_VIF_MSCALF = 21,
    /** 
     * Sets the data mask pattern. 
     * STMASK stores the next 1 word of data in the VIFn_MASK register. 
     * This data becomes the mask pattern of the write data. 
     */
    P2_VIF_STMASK = 32,
    /** 
     * Sets the filling data. 
     * STROW stores the following 4 words of data in the VIFn_R0 - VIFn_R3 registers. 
     * These are used as filling data when decompressed by the VIFcode UNPACK. 
     */
    P2_VIF_STROW = 48,
    /** 
     * Sets the filling data. 
     * STCOL stores the following 4 words of data in the VIFn_C0 - VIFn_C3 registers. 
     * These are used as filling data when decompressed by the VIFcode UNPACK. 
     */
    P2_VIF_STCOL = 49,
    /** 
     * Transfers the microprogram. 
     * MPG waits for the end of the microprogram and transfers the following NUM pieces of 64-bit data. 
     * (microinstruction code) to the MicroMem address shown by the IMMEDIATE field. 
     */
    P2_VIF_MPG = 74,
    /** 
     * Transfers data to the GIF(GS). 
     * DIRECT transfers the following IMMEDIATE pieces of 128-bit data to the GS via GIF PATH2. It is 
     * necessary to put the appropriate GIFtag to the data. 
     */
    P2_VIF_DIRECT = 80,
    /** 
     * Transfers data to the GIF(GS). 
     * DIRECTHL transfers the following IMMEDIATE pieces of 128-bit data to the GS via GIF PATH2. 
     * The appropriate GIFtag must be attached to the data. 
     * Interruption of PATH3 IMAGE mode data does not occur during the data transfer via PATH2 by 
     * DIRECTHL. Moreover, when the IMAGE mode data is being transferred via PATH3, DIRECTHL stalls 
     * until the end of the transfer. 
     */
    P2_VIF_DIRECTHL = 81
};

/** VIFCode structure. */
typedef struct
{
    /** The contents of the IMMEDIATE field vary according to the CMD field. */
    u32 immediate : 16;
    /**
     * The NUM field shows the amount of data written to the VU Mem or MicroMem (Data is in 128-bit units 
     * and microinstruction code is in 64-bit units). When NUM=0, it is considered to be 256. 
     * Note that it is not the amount of data in the VIF packet. NUM pieces of data can be written even when the 
     * amount of data in the packet is 0 depending on the CYCLE register setting. 
     */
    u32 num : 8;
    /** 
     * The CMD field directs the operation of the VIF, that is, the decompression 
     * method of the following data.
     */
    u32 cmd : 8;
} vif_code_t;

/** 
 * DMA data packet. 
 * Successor of standard packet. 
 */
typedef struct
{
    /** Maximum number of qwords which was defined in packet2_create(). */
    u16 max_qwords_count;
    /** Type of packet's memory mapping. */
    enum Packet2Type type;
    /** Packet mode. */
    enum Packet2Mode mode;
    /** 
     * Tag transfer enable. 
     * Effective only in chain mode. 
     * If >0 transfer tag is set during packet sending and 
     * add_dma_tag() (so also every open_tag()) will move buffer by DWORD, 
     * so remember to align memory!
     */
    u8 tte;
    /** Start position of the buffer. */
    qword_t *base __attribute__((aligned(64)));
    /** Current position of the buffer. */
    qword_t *next;
    /** 
     * The buffer position where DMA tag was opened. 
     * NULL, if no dma tag is open. 
     */
    dma_tag_t *tag_opened_at;
    /** 
     * The buffer position where DIRECT/UNPACK was opened. 
     * NULL, if no DIRECT/UNPACK is open. 
     */
    vif_code_t *vif_code_opened_at;
} packet2_t;

/** Mask, used in VIF's STMASK opcode. */
typedef union
{
    struct
    {
        unsigned int m0 : 2;
        unsigned int m1 : 2;
        unsigned int m2 : 2;
        unsigned int m3 : 2;
        unsigned int m4 : 2;
        unsigned int m5 : 2;
        unsigned int m6 : 2;
        unsigned int m7 : 2;
        unsigned int m8 : 2;
        unsigned int m9 : 2;
        unsigned int m10 : 2;
        unsigned int m11 : 2;
        unsigned int m12 : 2;
        unsigned int m13 : 2;
        unsigned int m14 : 2;
        unsigned int m15 : 2;
    };
    unsigned int m;
} Mask;

#endif /* __PACKET2_DATA_TYPES_H__ */

/** @} */ // end of packet2_types subgroup
