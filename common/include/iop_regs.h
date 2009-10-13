#ifndef _IOP_REGS_H
#define _IOP_REGS_H

#include <tamtypes.h>

#define A_IOP_SIF0_HANDLER ((vu32 *) (0x000003C0))
#define A_IOP_SIF1_HANDLER ((vu32 *) (0x000003D0))
#define A_IOP_SIF2_HANDLER ((vu32 *) (0x000003E0))

#define A_IOP_IRQ_CTRL 0xBF801450
#define A_IOP_REG_1454 0xBF801454

// unknown, related to CD? 32-bit
#define A_PS1_1F801018 0x1F801018

// unknown, related to CD? 32-bit
#define A_PS1_1F801020 0x1F801020


#define A_PS1_SIO0_DATA 0x1F801040
#define A_PS1_SIO0_STAT 0x1F801044
#define A_PS1_SIO0_MODE 0x1F801048
#define A_PS1_SIO0_CTRL 0x1F80104A
#define A_PS1_SIO0_BAUD 0x1F80104E

#define A_PS1_SIO1_DATA 0x1F801050
#define A_PS1_SIO1_STAT 0x1F801054
#define A_PS1_SIO1_MODE 0x1F801058
#define A_PS1_SIO1_CTRL 0x1F80105A
#define A_PS1_SIO1_BAUD 0x1F80105E

#define A_PS1_RAM_SIZE  0x1F801060

#define A_IOP_I_STAT 0xBF801070
#define A_IOP_I_MASK 0xBF801074

#define A_IOP_IREG_1078 0xBF801078
#define A_IOP_IREG_107C 0xBF80107C

#define A_IOP_D0_MADR     0xBF801080
#define A_IOP_D0_BCR      0xBF801084
#define A_IOP_D0_CHCR     0xBF801088

#define A_IOP_D1_MADR     0xBF801090
#define A_IOP_D1_BCR      0xBF801094
#define A_IOP_D1_CHCR     0xBF801098

#define A_IOP_D2_MADR     0xBF8010A0
#define A_IOP_D2_BCR      0xBF8010A4
#define A_IOP_D2_CHCR     0xBF8010A8

#define A_IOP_D3_MADR     0xBF8010B0
#define A_IOP_D3_BCR      0xBF8010B4
#define A_IOP_D3_CHCR     0xBF8010B8

#define A_IOP_D4_MADR     0xBF8010C0
#define A_IOP_D4_BCR      0xBF8010C4
#define A_IOP_D4_CHCR     0xBF8010C8
#define A_IOP_D4_TADR     0xBF8010CC

#define A_IOP_D5_MADR     0xBF8010D0
#define A_IOP_D5_BCR      0xBF8010D4
#define A_IOP_D5_CHCR     0xBF8010D8

#define A_IOP_D6_MADR     0xBF8010E0
#define A_IOP_D6_BCR      0xBF8010E4
#define A_IOP_D6_CHCR     0xBF8010E8

#define A_IOP_SIF_1450    0xBF801450
#define A_IOP_SIF_1454    0xBF801454

#define A_IOP_BF80146E  0xBF80146E
#define A_IOP_BF801470  0xBF801470
#define A_IOP_BF801472  0xBF801472

#define A_IOP_D7_MADR     0xBF801500
#define A_IOP_D7_BCR      0xBF801504
#define A_IOP_D7_CHCR     0xBF801508

#define A_IOP_D8_MADR     0xBF801510
#define A_IOP_D8_BCR      0xBF801514
#define A_IOP_D8_CHCR     0xBF801518

#define A_IOP_D9_MADR     0xBF801520
#define A_IOP_D9_BCR      0xBF801524
#define A_IOP_D9_CHCR     0xBF801528
#define A_IOP_D9_TADR     0xBF80152C

#define A_IOP_D10_MADR    0xBF801530
#define A_IOP_D10_BCR     0xBF801534
#define A_IOP_D10_CHCR    0xBF801538

#define A_IOP_D11_MADR    0xBF801540
#define A_IOP_D11_BCR     0xBF801544
#define A_IOP_D11_CHCR    0xBF801548

#define A_IOP_D12_MADR    0xBF801550
#define A_IOP_D12_BCR     0xBF801554
#define A_IOP_D12_CHCR    0xBF801558

#define A_IOP_DMAC_1560   0xBF801560
#define A_IOP_DMAC_1564   0xBF801564
#define A_IOP_DMAC_1568   0xBF801568

#define A_IOP_DPCR        0xBF8010F0
#define A_IOP_DPCR2       0xBF801570
#define A_IOP_DPCR3       0xBF8015F0

#define A_IOP_DICR        0xBF8010F4
#define A_IOP_DICR2       0xBF801574
#define A_IOP_DICR3       0xBF80157C

#define A_IOP_DMAC_1578   0xBF801578

#define A_PS1_CD_REG0  0x1F801800
#define A_PS1_CD_REG1  0x1F801801
#define A_PS1_CD_REG2  0x1F801802
#define A_PS1_CD_REG3  0x1F801803

#define A_IOP_GPU_DATA  0x1F801810
#define A_IOP_GPU_CTRL  0x1F801814

// $1f801c00-$1f801dff
#define A_IOP_SPU1_BASE 0x1F801C00

#define A_IOP_UNK_2070 0xBF802070

#define A_IOP_BF803200  0xBF803200
#define A_IOP_BF803204  0xBF803204
#define A_IOP_BF803218  0xBF803218
#define A_IOP_BF808400  0xBF808400
#define A_IOP_BF808414  0xBF808414
#define A_IOP_BF80844C  0xBF80844C
#define A_IOP_BF808420  0xBF808420
#define A_IOP_BF808428  0xBF808428
#define A_IOP_BF808430  0xBF808430
#define A_IOP_BF80847C  0xBF80847C

#define A_IOP_SBUS_REG_BASE 0xBD000000

// Register pointer definitions

// IOP/EE IRQ control?
#define R_IOP_IRQ_CTRL ((vu32 *) (A_IOP_IRQ_CTRL))

// ??? related to IOP/EE communication?
#define R_IOP_REG_1454 ((vu32 *) (A_IOP_REG_1454))

// PS1 SIO0(pad/card slots) All 16-bit?
#define R_PS1_SIO0_DATA ((vu16 *) (A_PS1_SIO0_DATA))
#define R_PS1_SIO0_STAT ((vu16 *) (A_PS1_SIO0_STAT))
#define R_PS1_SIO0_MODE ((vu16 *) (A_PS1_SIO0_MODE))
#define R_PS1_SIO0_CTRL ((vu16 *) (A_PS1_SIO0_CTRL))
#define R_PS1_SIO0_BAUD ((vu16 *) (A_PS1_SIO0_BAUD))

// PS1 SIO1(serial port) All 16-bit?
// note: these are just duplicated from SIO0 infos!
#define R_PS1_SIO1_DATA ((vu16 *) (A_PS1_SIO1_DATA))
#define R_PS1_SIO1_STAT ((vu16 *) (A_PS1_SIO1_STAT))
#define R_PS1_SIO1_MODE ((vu16 *) (A_PS1_SIO1_MODE))
#define R_PS1_SIO1_CTRL ((vu16 *) (A_PS1_SIO1_CTRL))
#define R_PS1_SIO1_BAUD ((vu16 *) (A_PS1_SIO1_BAUD))

// ??
#define R_PS1_RAM_SIZE ((vu32 *) (A_PS1_RAM_SIZE))

#define R_IOP_I_STAT ((vu32 *) (A_IOP_I_STAT))
#define R_IOP_I_MASK ((vu32 *) (A_IOP_I_MASK))

// unknown functions, likely interrupt/dma related.
#define R_IOP_IREG_1078 ((vu32 *) (A_IOP_IREG_1078))
#define R_IOP_IREG_107C ((vu32 *) (A_IOP_IREG_107C))

// IOP DMAC Registers
#define R_IOP_D0_MADR ((vu32 *) (A_IOP_D0_MADR))
#define R_IOP_D0_BCR ((vu32 *) (A_IOP_D0_BCR))
#define R_IOP_D0_CHCR ((vu32 *) (A_IOP_D0_CHCR))

#define R_IOP_D1_MADR ((vu32 *) (A_IOP_D1_MADR))
#define R_IOP_D1_BCR ((vu32 *) (A_IOP_D1_BCR))
#define R_IOP_D1_CHCR ((vu32 *) (A_IOP_D1_CHCR))

#define R_IOP_D2_MADR ((vu32 *) (A_IOP_D2_MADR))
#define R_IOP_D2_BCR ((vu32 *) (A_IOP_D2_BCR))
#define R_IOP_D2_BCR_BS ((vu16 *) (A_IOP_D2_BCR + 0x00))
#define R_IOP_D2_BCR_BC ((vu16 *) (A_IOP_D2_BCR + 0x02))
#define R_IOP_D2_CHCR ((vu32 *) (A_IOP_D2_CHCR))

#define R_IOP_D3_MADR ((vu32 *) (A_IOP_D3_MADR))
#define R_IOP_D3_BCR ((vu32 *) (A_IOP_D3_BCR))
#define R_IOP_D3_CHCR ((vu32 *) (A_IOP_D3_CHCR))

#define R_IOP_D4_MADR ((vu32 *) (A_IOP_D4_MADR))
#define R_IOP_D4_BCR ((vu32 *) (A_IOP_D4_BCR))
#define R_IOP_D4_CHCR ((vu32 *) (A_IOP_D4_CHCR))
#define R_IOP_D4_TADR ((vu32 *) (A_IOP_D4_TADR))

#define R_IOP_D5_MADR ((vu32 *) (A_IOP_D5_MADR))
#define R_IOP_D5_BCR ((vu32 *) (A_IOP_D5_BCR))
#define R_IOP_D5_CHCR ((vu32 *) (A_IOP_D5_CHCR))

#define R_IOP_D6_MADR ((vu32 *) (A_IOP_D6_MADR))
#define R_IOP_D6_BCR ((vu32 *) (A_IOP_D6_BCR))
#define R_IOP_D6_CHCR ((vu32 *) (A_IOP_D6_CHCR))

#define R_IOP_D7_MADR ((vu32 *) (A_IOP_D7_MADR))
#define R_IOP_D7_BCR ((vu32 *) (A_IOP_D7_BCR))
#define R_IOP_D7_CHCR ((vu32 *) (A_IOP_D7_CHCR))

#define R_IOP_D8_MADR ((vu32 *) (A_IOP_D8_MADR))
#define R_IOP_D8_BCR ((vu32 *) (A_IOP_D8_BCR))
#define R_IOP_D8_CHCR ((vu32 *) (A_IOP_D8_CHCR))

#define R_IOP_D9_MADR ((vu32 *) (A_IOP_D9_MADR))
#define R_IOP_D9_BCR ((vu32 *) (A_IOP_D9_BCR))
#define R_IOP_D9_CHCR ((vu32 *) (A_IOP_D9_CHCR))
#define R_IOP_D9_TADR ((vu32 *) (A_IOP_D9_TADR))

#define R_IOP_D10_MADR ((vu32 *) (A_IOP_D10_MADR))
#define R_IOP_D10_BCR ((vu32 *) (A_IOP_D10_BCR))
#define R_IOP_D10_CHCR ((vu32 *) (A_IOP_D10_CHCR))

#define R_IOP_D11_MADR ((vu32 *) (A_IOP_D11_MADR))
#define R_IOP_D11_BCR ((vu32 *) (A_IOP_D11_BCR))
#define R_IOP_D11_CHCR ((vu32 *) (A_IOP_D11_CHCR))

#define R_IOP_D12_MADR ((vu32 *) (A_IOP_D12_MADR))
#define R_IOP_D12_BCR ((vu32 *) (A_IOP_D12_BCR))
#define R_IOP_D12_CHCR ((vu32 *) (A_IOP_D12_CHCR))

// These are some type of extended DMA control/address
// Reg 1560 is for SIF0(CH9), 1564 is for SIF1(CH10) and 1568 is for "SPU"(CH4) though "SPU" seems odd, perhaps SIF2??
#define R_IOP_DMAC_1560 ((vu32 *) (A_IOP_DMAC_1560))
#define R_IOP_DMAC_1564 ((vu32 *) (A_IOP_DMAC_1564))
#define R_IOP_DMAC_1568 ((vu32 *) (A_IOP_DMAC_1568))

#define R_IOP_DPCR ((vu32 *) (A_IOP_DPCR))
#define R_IOP_DPCR2 ((vu32 *) (A_IOP_DPCR2))
#define R_IOP_DPCR3 ((vu32 *) (A_IOP_DPCR3))

#define R_IOP_DICR ((vu32 *) (A_IOP_DICR))
#define R_IOP_DICR2 ((vu32 *) (A_IOP_DICR2))
#define R_IOP_DICR3 ((vu32 *) (A_IOP_DICR3))

// ??? Set to 1 by DMACMAN when DMAC is initilized, set to 0 by DMACMAN when shut down.
#define R_IOP_DMAC_1578 ((vu32 *) (A_IOP_DMAC_1578))

// SIF/SBUS

#define R_IOP_SIF_1450 ((vu32 *) (A_IOP_SIF_1450))
#define R_IOP_SIF_1454 ((vu32 *) (A_IOP_SIF_1454))

#define R_PS1_CD_REG0 ((vu8 *) (A_PS1_CD_REG0))
#define R_PS1_CD_REG1 ((vu8 *) (A_PS1_CD_REG1))
#define R_PS1_CD_REG2 ((vu8 *) (A_PS1_CD_REG2))
#define R_PS1_CD_REG3 ((vu8 *) (A_PS1_CD_REG3))

#define R_IOP_GPU_DATA ((vu32 *) (A_IOP_GPU_DATA))
#define R_IOP_GPU_CTRL ((vu32 *) (A_IOP_GPU_CTRL))

#define R_IOP_UNK_2070 ((vu32 *) (A_IOP_UNK_2070))

// IOP SBUS Registers(add PS2_SBUS_xxx to this to get register address)
#define R_IOP_SBUS_REG_BASE ((vu32 *) (A_IOP_SBUS_REG_BASE))

// Accessed by EECONF.IRX:

// accessed as 8-bit
#define R_IOP_BF80146E ((vu8 *) (A_IOP_BF80146E))

// accessed as 16-bit
#define R_IOP_BF801470 ((vu16 *) (A_IOP_BF801470))
#define R_IOP_BF801472 ((vu16 *) (A_IOP_BF801472))

// accessed as 8-bit
#define R_IOP_BF803200 ((vu8 *) (A_IOP_BF803200))
#define R_IOP_BF803204 ((vu8 *) (A_IOP_BF803204))
#define R_IOP_BF803218 ((vu8 *) (A_IOP_BF803218))

// 32-bit??
#define R_IOP_BF808400 ((vu32 *) (A_IOP_BF808400))
#define R_IOP_BF808414 ((vu32 *) (A_IOP_BF808414))
#define R_IOP_BF80844C ((vu32 *) (A_IOP_BF80844C))
#define R_IOP_BF808420 ((vu32 *) (A_IOP_BF808420))
#define R_IOP_BF808428 ((vu32 *) (A_IOP_BF808428))
#define R_IOP_BF808430 ((vu32 *) (A_IOP_BF808430))
#define R_IOP_BF80847C ((vu32 *) (A_IOP_BF80847C))

// Unknown, seems to be related to SIF2?
#define IOP_CHCR_30 (1<<30)

// Transfer(used to kick a transfer and determine if a transfer is in progress).
#define IOP_CHCR_TR (1<<24)

// Linked List(DMA tags), valid for GPU(SIF2?), SPU and SIF0
#define IOP_CHCR_LI (1<<10)

// Continuous transfer, TR is not cleared when transfer has completed.
#define IOP_CHCR_CO (1<<9)

// Unknown..
#define IOP_CHCR_08 (1<<8)

// Direction: 0 = "to RAM", 1 = "from RAM"
#define IOP_CHCR_DR (1<<0)

#define IOP_TO_MEM	0
#define IOP_FROM_MEM	1

#define IOP_I_STAT_VB (1 << 0)
#define IOP_I_STAT_SBUS (1 << 1)

#endif // #ifndef _IOP_REGS_H

