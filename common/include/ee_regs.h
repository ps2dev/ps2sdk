#ifndef _EE_REGS_H
#define _EE_REGS_H

#include "tamtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define A_EE_SBUS_REG_BASE (0x1000F200)
#define A_EE_PGIF_REG_BASE (0x1000F300)

#define A_EE_T0_COUNT (0x10000000)
#define A_EE_T0_MODE (0x10000010)
#define A_EE_T0_COMP (0x10000020)
#define A_EE_T0_HOLD (0x10000030)
#define A_EE_T1_COUNT (0x10000800)
#define A_EE_T1_MODE (0x10000810)
#define A_EE_T1_COMP (0x10000820)
#define A_EE_T1_HOLD (0x10000830)
#define A_EE_T2_COUNT (0x10001000)
#define A_EE_T2_MODE (0x10001010)
#define A_EE_T2_COMP (0x10001020)
#define A_EE_T3_COUNT (0x10001800)
#define A_EE_T3_MODE (0x10001810)
#define A_EE_T3_COMP (0x10001820)
#define A_EE_IPU_CMD (0x10002000)
#define A_EE_IPU_CTRL (0x10002010)
#define A_EE_IPU_BP (0x10002020)
#define A_EE_IPU_TOP (0x10002030)
#define A_EE_GIF_CTRL (0x10003000)
#define A_EE_GIF_MODE (0x10003010)
#define A_EE_GIF_STAT (0x10003020)
#define A_EE_GIF_TAG0 (0x10003040)
#define A_EE_GIF_TAG1 (0x10003050)
#define A_EE_GIF_TAG2 (0x10003060)
#define A_EE_GIF_TAG3 (0x10003070)
#define A_EE_GIF_CNT (0x10003080)
#define A_EE_GIF_P3CNT (0x10003090)
#define A_EE_GIF_P3TAG (0x100030a0)
#define A_EE_VIF0_STAT (0x10003800)
#define A_EE_VIF0_FBRST (0x10003810)
#define A_EE_VIF0_ERR (0x10003820)
#define A_EE_VIF0_MARK (0x10003830)
#define A_EE_VIF0_CYCLE (0x10003840)
#define A_EE_VIF0_MODE (0x10003850)
#define A_EE_VIF0_NUM (0x10003860)
#define A_EE_VIF0_MASK (0x10003870)
#define A_EE_VIF0_CODE (0x10003880)
#define A_EE_VIF0_ITOPS (0x10003890)
#define A_EE_VIF0_ITOP (0x100038d0)
#define A_EE_VIF0_R0 (0x10003900)
#define A_EE_VIF0_R1 (0x10003910)
#define A_EE_VIF0_R2 (0x10003920)
#define A_EE_VIF0_R3 (0x10003930)
#define A_EE_VIF0_C0 (0x10003940)
#define A_EE_VIF0_C1 (0x10003950)
#define A_EE_VIF0_C2 (0x10003960)
#define A_EE_VIF0_C3 (0x10003970)
#define A_EE_VIF1_STAT (0x10003c00)
#define A_EE_VIF1_FBRST (0x10003c10)
#define A_EE_VIF1_ERR (0x10003c20)
#define A_EE_VIF1_MARK (0x10003c30)
#define A_EE_VIF1_CYCLE (0x10003c40)
#define A_EE_VIF1_MODE (0x10003c50)
#define A_EE_VIF1_NUM (0x10003c60)
#define A_EE_VIF1_MASK (0x10003c70)
#define A_EE_VIF1_CODE (0x10003c80)
#define A_EE_VIF1_ITOPS (0x10003c90)
#define A_EE_VIF1_BASE (0x10003ca0)
#define A_EE_VIF1_OFST (0x10003cb0)
#define A_EE_VIF1_TOPS (0x10003cc0)
#define A_EE_VIF1_ITOP (0x10003cd0)
#define A_EE_VIF1_TOP (0x10003ce0)
#define A_EE_VIF1_R0 (0x10003d00)
#define A_EE_VIF1_R1 (0x10003d10)
#define A_EE_VIF1_R2 (0x10003d20)
#define A_EE_VIF1_R3 (0x10003d30)
#define A_EE_VIF1_C0 (0x10003d40)
#define A_EE_VIF1_C1 (0x10003d50)
#define A_EE_VIF1_C2 (0x10003d60)
#define A_EE_VIF1_C3 (0x10003d70)
#define A_EE_VIF0_FIFO (0x10004000)
#define A_EE_VIF1_FIFO (0x10005000)
#define A_EE_GIF_FIFO (0x10006000)
#define A_EE_IPU_out_FIFO (0x10007000)
#define A_EE_IPU_in_FIFO (0x10007010)
#define A_EE_D0_CHCR (0x10008000)
#define A_EE_D0_MADR (0x10008010)
#define A_EE_D0_QWC (0x10008020)
#define A_EE_D0_TADR (0x10008030)
#define A_EE_D0_ASR0 (0x10008040)
#define A_EE_D0_ASR1 (0x10008050)
#define A_EE_D1_CHCR (0x10009000)
#define A_EE_D1_MADR (0x10009010)
#define A_EE_D1_QWC (0x10009020)
#define A_EE_D1_TADR (0x10009030)
#define A_EE_D1_ASR0 (0x10009040)
#define A_EE_D1_ASR1 (0x10009050)
#define A_EE_D2_CHCR (0x1000a000)
#define A_EE_D2_MADR (0x1000a010)
#define A_EE_D2_QWC (0x1000a020)
#define A_EE_D2_TADR (0x1000a030)
#define A_EE_D2_ASR0 (0x1000a040)
#define A_EE_D2_ASR1 (0x1000a050)
#define A_EE_D3_CHCR (0x1000b000)
#define A_EE_D3_MADR (0x1000b010)
#define A_EE_D3_QWC (0x1000b020)
#define A_EE_D4_CHCR (0x1000b400)
#define A_EE_D4_MADR (0x1000b410)
#define A_EE_D4_QWC (0x1000b420)
#define A_EE_D4_TADR (0x1000b430)
#define A_EE_D5_CHCR (0x1000c000)
#define A_EE_D5_MADR (0x1000C010)
#define A_EE_D5_QWC (0x1000C020)
#define A_EE_D6_CHCR (0x1000C400)
#define A_EE_D6_MADR (0x1000C410)
#define A_EE_D6_QWC (0x1000C420)
#define A_EE_D6_TADR (0x1000C430)
#define A_EE_D7_CHCR (0xB000C800)
#define A_EE_D7_MADR (0xB000C810)
#define A_EE_D7_QWC (0xB000C820)
#define A_EE_D8_CHCR (0x1000D000)
#define A_EE_D8_MADR (0x1000D010)
#define A_EE_D8_QWC (0x1000D020)
#define A_EE_D8_SADR (0x1000D080)
#define A_EE_D9_CHCR (0x1000D400)
#define A_EE_D9_MADR (0x1000D410)
#define A_EE_D9_QWC (0x1000D420)
#define A_EE_D9_TADR (0x1000D430)
#define A_EE_D9_SADR (0x1000D480)
#define A_EE_D_CTRL (0x1000e000)
#define A_EE_D_STAT (0x1000e010)
#define A_EE_D_PCR (0x1000e020)
#define A_EE_D_SQWC (0x1000e030)
#define A_EE_D_RBSR (0x1000e040)
#define A_EE_D_RBOR (0x1000e050)
#define A_EE_D_STADR (0x1000e060)
#define A_EE_I_STAT (0x1000f000)
#define A_EE_I_MASK (0x1000f010)
#define A_EE_SIO_LCR (0x1000F100)
#define A_EE_SIO_LSR (0x1000F110)
#define A_EE_SIO_IER (0x1000F120)
#define A_EE_SIO_ISR (0x1000F130)
#define A_EE_SIO_FCR (0x1000F140)
#define A_EE_SIO_BRC (0x1000F150)
#define A_EE_SIO_REG60 (0x1000F160)
#define A_EE_SIO_REG70 (0x1000F170)
#define A_EE_SIO_TXFIFO (0x1000F180)
#define A_EE_SIO_REG90 (0x1000F190)
#define A_EE_SIO_REGA0 (0x1000F1A0)
#define A_EE_SIO_REGB0 (0x1000F1B0)
#define A_EE_SIO_RXFIFO (0x1000F1C0)
#define A_EE_SBUS_MADDR (0x1000F200)
#define A_EE_SBUS_SADDR (0x1000F210)
#define A_EE_SBUS_MSFLAG (0x1000F220)
#define A_EE_SBUS_SMFLAG (0x1000F230)
#define A_EE_SBUS_REG40 (0x1000F240)
#define A_EE_SBUS_REG50 (0x1000F250)
#define A_EE_SBUS_REG60 (0x1000F260)
#define A_EE_SBUS_REG70 (0x1000F270)
#define A_EE_SBUS_REG80 (0x1000F280)
#define A_EE_SBUS_REG90 (0x1000F290)
#define A_EE_SBUS_REGA0 (0x1000F2A0)
#define A_EE_SBUS_REGB0 (0x1000F2B0)
#define A_EE_SBUS_REGC0 (0x1000F2C0)
#define A_EE_SBUS_REGD0 (0x1000F2D0)
#define A_EE_SBUS_REGE0 (0x1000F2E0)
#define A_EE_SBUS_REGF0 (0x1000F2F0)
#define A_EE_PGIF_GPU_STAT (0x1000F300)
#define A_EE_PGIF_REG10 (0x1000F310)
#define A_EE_PGIF_REG20 (0x1000F320)
#define A_EE_PGIF_REG30 (0x1000F330)
#define A_EE_PGIF_REG40 (0x1000F340)
#define A_EE_PGIF_REG50 (0x1000F350)
#define A_EE_PGIF_REG60 (0x1000F360)
#define A_EE_PGIF_REG70 (0x1000F370)
#define A_EE_PGIF_CFIFO_STAT (0x1000F380)
#define A_EE_PGIF_REG90 (0x1000F390)
#define A_EE_PGIF_REGA0 (0x1000F3A0)
#define A_EE_PGIF_REGB0 (0x1000F3B0)
#define A_EE_PGIF_CFIFO_DATA (0x1000F3C0)
#define A_EE_PGIF_REGD0 (0x1000F3D0)
#define A_EE_PGIF_REGE0 (0x1000F3E0)
#define A_EE_PGIF_REGF0 (0x1000F3F0)
#define A_EE_D_ENABLER (0x1000f520)
#define A_EE_D_ENABLEW (0x1000f590)
#define A_EE_GS_PMODE (0x12000000)
#define A_EE_GS_SMODE1 (0x12000010)
#define A_EE_GS_SMODE2 (0x12000020)
#define A_EE_GS_SRFSH (0x12000030)
#define A_EE_GS_SYNCH1 (0x12000040)
#define A_EE_GS_SYNCH2 (0x12000050)
#define A_EE_GS_SYNCV (0x12000060)
#define A_EE_GS_DISPFB1 (0x12000070)
#define A_EE_GS_DISPLAY1 (0x12000080)
#define A_EE_GS_DISPFB2 (0x12000090)
#define A_EE_GS_DISPLAY2 (0x120000a0)
#define A_EE_GS_EXTBUF (0x120000b0)
#define A_EE_GS_EXTDATA (0x120000c0)
#define A_EE_GS_EXTWRITE (0x120000d0)
#define A_EE_GS_BGCOLOR (0x120000e0)
#define A_EE_GS_CSR (0x12001000)
#define A_EE_GS_IMR (0x12001010)
#define A_EE_GS_BUSDIR (0x12001040)

// Timer 0 counter value
#define R_EE_T0_COUNT ((vu32 *) A_EE_T0_COUNT)
// Timer 0 mode/status
#define R_EE_T0_MODE ((vu32 *) A_EE_T0_MODE)
// Timer 0 compare value
#define R_EE_T0_COMP ((vu32 *) A_EE_T0_COMP)
// Timer 0 hold value
#define R_EE_T0_HOLD ((vu32 *) A_EE_T0_HOLD)
// Timer 1 counter value
#define R_EE_T1_COUNT ((vu32 *) A_EE_T1_COUNT)
// Timer 1 mode/status
#define R_EE_T1_MODE ((vu32 *) A_EE_T1_MODE)
// Timer 1 compare value
#define R_EE_T1_COMP ((vu32 *) A_EE_T1_COMP)
// Timer 1 hold value
#define R_EE_T1_HOLD ((vu32 *) A_EE_T1_HOLD)
// Timer 2 counter value
#define R_EE_T2_COUNT ((vu32 *) A_EE_T2_COUNT)
// Timer 2 mode/status
#define R_EE_T2_MODE ((vu32 *) A_EE_T2_MODE)
// Timer 2 compare value
#define R_EE_T2_COMP ((vu32 *) A_EE_T2_COMP)
// Timer 3 counter value
#define R_EE_T3_COUNT ((vu32 *) A_EE_T3_COUNT)
// Timer 3 mode/status
#define R_EE_T3_MODE ((vu32 *) A_EE_T3_MODE)
// Timer 3 compare value
#define R_EE_T3_COMP ((vu32 *) A_EE_T3_COMP)
// IPU command
#define R_EE_IPU_CMD ((vu64 *) A_EE_IPU_CMD)
// IPU control
#define R_EE_IPU_CTRL ((vu32 *) A_EE_IPU_CTRL)
// IPU input FIFO control
#define R_EE_IPU_BP ((vu32 *) A_EE_IPU_BP)
// First data of bit stream
#define R_EE_IPU_TOP ((vu64 *) A_EE_IPU_TOP)
// GIF control
#define R_EE_GIF_CTRL ((vu32 *) A_EE_GIF_CTRL)
// GIF mode setting
#define R_EE_GIF_MODE ((vu32 *) A_EE_GIF_MODE)
// GIF status
#define R_EE_GIF_STAT ((vu32 *) A_EE_GIF_STAT)
// GIFtag ((vu32 *) bits 31-0) immediately before
#define R_EE_GIF_TAG0 ((vu32 *) A_EE_GIF_TAG0)
// GIFtag ((vu32 *) bits 63-32) immediately before
#define R_EE_GIF_TAG1 ((vu32 *) A_EE_GIF_TAG1)
// GIFtag ((vu32 *) bits 95-64) immediately before
#define R_EE_GIF_TAG2 ((vu32 *) A_EE_GIF_TAG2)
// GIFtag ((vu32 *) bits 127-96) immediately before
#define R_EE_GIF_TAG3 ((vu32 *) A_EE_GIF_TAG3)
// Transfer status counter
#define R_EE_GIF_CNT ((vu32 *) A_EE_GIF_CNT)
// PATH3 transfer status counter
#define R_EE_GIF_P3CNT ((vu32 *) A_EE_GIF_P3CNT)
// PATH3 tag value
#define R_EE_GIF_P3TAG ((vu32 *) A_EE_GIF_P3TAG)
// Status
#define R_EE_VIF0_STAT ((vu32 *) A_EE_VIF0_STAT)
// Operation control
#define R_EE_VIF0_FBRST ((vu32 *) A_EE_VIF0_FBRST)
// Error status
#define R_EE_VIF0_ERR ((vu32 *) A_EE_VIF0_ERR)
// Mark value
#define R_EE_VIF0_MARK ((vu32 *) A_EE_VIF0_MARK)
// Data write cycle
#define R_EE_VIF0_CYCLE ((vu32 *) A_EE_VIF0_CYCLE)
// ADD mode
#define R_EE_VIF0_MODE ((vu32 *) A_EE_VIF0_MODE)
// Amount of non-transferred data
#define R_EE_VIF0_NUM ((vu32 *) A_EE_VIF0_NUM)
// Write mask pattern
#define R_EE_VIF0_MASK ((vu32 *) A_EE_VIF0_MASK)
// Last processed VIFcode
#define R_EE_VIF0_CODE ((vu32 *) A_EE_VIF0_CODE)
// Next ITOP value
#define R_EE_VIF0_ITOPS ((vu32 *) A_EE_VIF0_ITOPS)
// ITOP value
#define R_EE_VIF0_ITOP ((vu32 *) A_EE_VIF0_ITOP)
// Filling data R0 ((vu32 *) Row register)
#define R_EE_VIF0_R0 ((vu32 *) A_EE_VIF0_R0)
// Filling data R1 ((vu32 *) Row register)
#define R_EE_VIF0_R1 ((vu32 *) A_EE_VIF0_R1)
// Filling data R2 ((vu32 *) Row register)
#define R_EE_VIF0_R2 ((vu32 *) A_EE_VIF0_R2)
// Filling data R3 ((vu32 *) Row register)
#define R_EE_VIF0_R3 ((vu32 *) A_EE_VIF0_R3)
// Filling data C0 ((vu32 *) Col register)
#define R_EE_VIF0_C0 ((vu32 *) A_EE_VIF0_C0)
// Filling data C1 ((vu32 *) Col register)
#define R_EE_VIF0_C1 ((vu32 *) A_EE_VIF0_C1)
// Filling data C2 ((vu32 *) Col register)
#define R_EE_VIF0_C2 ((vu32 *) A_EE_VIF0_C2)
// Filling data C3 ((vu32 *) Col register)
#define R_EE_VIF0_C3 ((vu32 *) A_EE_VIF0_C3)
// Status
#define R_EE_VIF1_STAT ((vu32 *) A_EE_VIF1_STAT)
// Operation control
#define R_EE_VIF1_FBRST ((vu32 *) A_EE_VIF1_FBRST)
// Error status
#define R_EE_VIF1_ERR ((vu32 *) A_EE_VIF1_ERR)
// Mark value
#define R_EE_VIF1_MARK ((vu32 *) A_EE_VIF1_MARK)
// Data write cycle
#define R_EE_VIF1_CYCLE ((vu32 *) A_EE_VIF1_CYCLE)
// ADD mode
#define R_EE_VIF1_MODE ((vu32 *) A_EE_VIF1_MODE)
// Amount of non-transferred data
#define R_EE_VIF1_NUM ((vu32 *) A_EE_VIF1_NUM)
// Write mask pattern
#define R_EE_VIF1_MASK ((vu32 *) A_EE_VIF1_MASK)
// Last processed VIFcode
#define R_EE_VIF1_CODE ((vu32 *) A_EE_VIF1_CODE)
// Next ITOP value
#define R_EE_VIF1_ITOPS ((vu32 *) A_EE_VIF1_ITOPS)
// Base address of double buffer
#define R_EE_VIF1_BASE ((vu32 *) A_EE_VIF1_BASE)
// Offset of double buffer
#define R_EE_VIF1_OFST ((vu32 *) A_EE_VIF1_OFST)
// Next TOP value/data write address
#define R_EE_VIF1_TOPS ((vu32 *) A_EE_VIF1_TOPS)
// ITOP value
#define R_EE_VIF1_ITOP ((vu32 *) A_EE_VIF1_ITOP)
// TOP value
#define R_EE_VIF1_TOP ((vu32 *) A_EE_VIF1_TOP)
// Filling data R0 ((vu32 *) Row register)
#define R_EE_VIF1_R0 ((vu32 *) A_EE_VIF1_R0)
// Filling data R1 ((vu32 *) Row register)
#define R_EE_VIF1_R1 ((vu32 *) A_EE_VIF1_R1)
// Filling data R2 ((vu32 *) Row register)
#define R_EE_VIF1_R2 ((vu32 *) A_EE_VIF1_R2)
// Filling data R3 ((vu32 *) Row register)
#define R_EE_VIF1_R3 ((vu32 *) A_EE_VIF1_R3)
// Filling data C0 ((vu32 *) Col register)
#define R_EE_VIF1_C0 ((vu32 *) A_EE_VIF1_C0)
// Filling data C1 ((vu32 *) Col register)
#define R_EE_VIF1_C1 ((vu32 *) A_EE_VIF1_C1)
// Filling data C2 ((vu32 *) Col register)
#define R_EE_VIF1_C2 ((vu32 *) A_EE_VIF1_C2)
// Filling data C3 ((vu32 *) Col register)
#define R_EE_VIF1_C3 ((vu32 *) A_EE_VIF1_C3)
// VIF0 FIFO ((vu32 *) write)
#define R_EE_VIF0_FIFO ((vu32 *) A_EE_VIF0_FIFO)
// VIF1 FIFO ((vu32 *) read/write)
#define R_EE_VIF1_FIFO ((vu32 *) A_EE_VIF1_FIFO)
// GIF FIFO ((vu32 *) write)
#define R_EE_GIF_FIFO ((vu32 *) A_EE_GIF_FIFO)
// IPU FIFO ((vu32 *) read)
#define R_EE_IPU_out_FIFO ((vu32 *) A_EE_IPU_out_FIFO)
// IPU FIFO ((vu32 *) write)
#define R_EE_IPU_in_FIFO ((vu32 *) A_EE_IPU_in_FIFO)
// Ch0 channel control
#define R_EE_D0_CHCR ((vu32 *) A_EE_D0_CHCR)
// Ch0 memory address
#define R_EE_D0_MADR ((vu32 *) A_EE_D0_MADR)
// Ch0 quad word count
#define R_EE_D0_QWC ((vu32 *) A_EE_D0_QWC)
// Ch0 tag address
#define R_EE_D0_TADR ((vu32 *) A_EE_D0_TADR)
// Ch0 address stack 0
#define R_EE_D0_ASR0 ((vu32 *) A_EE_D0_ASR0)
// Ch0 address stack 1
#define R_EE_D0_ASR1 ((vu32 *) A_EE_D0_ASR1)
// Ch1 channel control
#define R_EE_D1_CHCR ((vu32 *) A_EE_D1_CHCR)
// Ch1 memory address
#define R_EE_D1_MADR ((vu32 *) A_EE_D1_MADR)
// Ch1 quad word count
#define R_EE_D1_QWC ((vu32 *) A_EE_D1_QWC)
// Ch1 tag address
#define R_EE_D1_TADR ((vu32 *) A_EE_D1_TADR)
// Ch1 address stack 0
#define R_EE_D1_ASR0 ((vu32 *) A_EE_D1_ASR0)
// Ch1 address stack 1
#define R_EE_D1_ASR1 ((vu32 *) A_EE_D1_ASR1)
// Ch2 channel control
#define R_EE_D2_CHCR ((vu32 *) A_EE_D2_CHCR)
// Ch2 memory address
#define R_EE_D2_MADR ((vu32 *) A_EE_D2_MADR)
// Ch2 quad word count
#define R_EE_D2_QWC ((vu32 *) A_EE_D2_QWC)
// Ch2 tag address
#define R_EE_D2_TADR ((vu32 *) A_EE_D2_TADR)
// Ch2 address stack 0
#define R_EE_D2_ASR0 ((vu32 *) A_EE_D2_ASR0)
// Ch2 address stack 1
#define R_EE_D2_ASR1 ((vu32 *) A_EE_D2_ASR1)
// Ch3 channel control
#define R_EE_D3_CHCR ((vu32 *) A_EE_D3_CHCR)
// Ch3 memory address
#define R_EE_D3_MADR ((vu32 *) A_EE_D3_MADR)
// Ch3 quad word count
#define R_EE_D3_QWC ((vu32 *) A_EE_D3_QWC)
// Ch4 chnnel control
#define R_EE_D4_CHCR ((vu32 *) A_EE_D4_CHCR)
// Ch4 memory address
#define R_EE_D4_MADR ((vu32 *) A_EE_D4_MADR)
// Ch4 quad word count
#define R_EE_D4_QWC ((vu32 *) A_EE_D4_QWC)
// Ch4 tag address
#define R_EE_D4_TADR ((vu32 *) A_EE_D4_TADR)
// Ch5 channel control
#define R_EE_D5_CHCR ((vu32 *) A_EE_D5_CHCR)
// Ch5 memory address
#define R_EE_D5_MADR ((vu32 *) A_EE_D5_MADR)
// Ch5 quad word count
#define R_EE_D5_QWC ((vu32 *) A_EE_D5_QWC)
// Ch6 channel control
#define R_EE_D6_CHCR ((vu32 *) A_EE_D6_CHCR)
// Ch6 memory address
#define R_EE_D6_MADR ((vu32 *) A_EE_D6_MADR)
// Ch6 quad word count
#define R_EE_D6_QWC ((vu32 *) A_EE_D6_QWC)
// Ch6 tag address
#define R_EE_D6_TADR ((vu32 *) A_EE_D6_TADR)
// Ch7 channel control
#define R_EE_D7_CHCR ((vu32 *) A_EE_D7_CHCR)
// Ch7 memory address
#define R_EE_D7_MADR ((vu32 *) A_EE_D7_MADR)
// Ch7 quad word count
#define R_EE_D7_QWC ((vu32 *) A_EE_D7_QWC)
// Ch8 channel control
#define R_EE_D8_CHCR ((vu32 *) A_EE_D8_CHCR)
// Ch8 memory address
#define R_EE_D8_MADR ((vu32 *) A_EE_D8_MADR)
// Ch8 quad word count
#define R_EE_D8_QWC ((vu32 *) A_EE_D8_QWC)
// Ch8 SPR address
#define R_EE_D8_SADR ((vu32 *) A_EE_D8_SADR)
// Ch9 channel control
#define R_EE_D9_CHCR ((vu32 *) A_EE_D9_CHCR)
// Ch9 memory address
#define R_EE_D9_MADR ((vu32 *) A_EE_D9_MADR)
// Ch9 quad word count
#define R_EE_D9_QWC ((vu32 *) A_EE_D9_QWC)
// Ch9 tag address
#define R_EE_D9_TADR ((vu32 *) A_EE_D9_TADR)
// Ch9 SPR address
#define R_EE_D9_SADR ((vu32 *) A_EE_D9_SADR)
// DMAC control
#define R_EE_D_CTRL ((vu32 *) A_EE_D_CTRL)
// DMAC status
#define R_EE_D_STAT ((vu32 *) A_EE_D_STAT)
// DMAC priority control
#define R_EE_D_PCR ((vu32 *) A_EE_D_PCR)
// DMAC skip quad word
#define R_EE_D_SQWC ((vu32 *) A_EE_D_SQWC)
// DMAC ring buffer size
#define R_EE_D_RBSR ((vu32 *) A_EE_D_RBSR)
// DMAC ring buffer offset
#define R_EE_D_RBOR ((vu32 *) A_EE_D_RBOR)
// DMA stall address
#define R_EE_D_STADR ((vu32 *) A_EE_D_STADR)
// Interrupt status
#define R_EE_I_STAT ((vu32 *) A_EE_I_STAT)
// Interrupt mask
#define R_EE_I_MASK ((vu32 *) A_EE_I_MASK)


// EE SIO Line Control Register
#define R_EE_SIO_LCR ((vu32 *) A_EE_SIO_LCR)
// EE SIO Line Status Register
#define R_EE_SIO_LSR ((vu32 *) A_EE_SIO_LSR)
// EE SIO Interrupt Enable Register
#define R_EE_SIO_IER ((vu32 *) A_EE_SIO_IER)
// EE SIO Interrupt Status Register
#define R_EE_SIO_ISR ((vu32 *) A_EE_SIO_ISR)
// EE SIO FIFO Control Register
#define R_EE_SIO_FCR ((vu32 *) A_EE_SIO_FCR)
// EE SIO Baud Rate Control Register
#define R_EE_SIO_BRC ((vu32 *) A_EE_SIO_BRC)
// Unknown SIO Register 0x60
#define R_EE_SIO_REG60 ((vu8 *) A_EE_SIO_REG60)
// Unknown SIO Register 0x70
#define R_EE_SIO_REG70 ((vu8 *) A_EE_SIO_REG70)
// Transfer Holding Register
#define R_EE_SIO_TXFIFO ((vu8 *) A_EE_SIO_TXFIFO)
// Unknown SIO Register 0x90
#define R_EE_SIO_REG90 ((vu8 *) A_EE_SIO_REG90)
// Unknown SIO Register 0xA0
#define R_EE_SIO_REGA0 ((vu8 *) A_EE_SIO_REGA0)
// Unknown SIO Register 0xB0
#define R_EE_SIO_REGB0 ((vu8 *) A_EE_SIO_REGB0)
// Recieve Buffer Register
#define R_EE_SIO_RXFIFO ((vu8 *) A_EE_SIO_RXFIFO)

// SBUS Unknown Reg 0x00
#define R_EE_SBUS_MADDR ((vu32 *) A_EE_SBUS_REG00)
// SBUS Unknown Reg 0x10
#define R_EE_SBUS_SADDR ((vu32 *) A_EE_SBUS_REG10)
// SBUS Main -> Sub Flag
#define R_EE_SBUS_MSFLAG ((vu32 *) A_EE_SBUS_MSFLAG)
// SBUS Sub -> Main Flag
#define R_EE_SBUS_SMFLAG ((vu32 *) A_EE_SBUS_SMFLAG)
// SBUS Unknown Reg 0x40
#define R_EE_SBUS_REG40 ((vu32 *) A_EE_SBUS_REG40)
// SBUS Unknown Reg 0x50
#define R_EE_SBUS_REG50 ((vu32 *) A_EE_SBUS_REG50)
// SBUS Unknown Reg 0x60
#define R_EE_SBUS_REG60 ((vu32 *) A_EE_SBUS_REG60)
// SBUS Unknown Reg 0x70
#define R_EE_SBUS_REG70 ((vu32 *) A_EE_SBUS_REG70)
// SBUS Unknown Reg 0x80
#define R_EE_SBUS_REG80 ((vu32 *) A_EE_SBUS_REG80)
// SBUS Unknown Reg 0x90
#define R_EE_SBUS_REG90 ((vu32 *) A_EE_SBUS_REG90)
// SBUS Unknown Reg 0xA0
#define R_EE_SBUS_REGA0 ((vu32 *) A_EE_SBUS_REGA0)
// SBUS Unknown Reg 0xB0
#define R_EE_SBUS_REGB0 ((vu32 *) A_EE_SBUS_REGB0)
// SBUS Unknown Reg 0xC0
#define R_EE_SBUS_REGC0 ((vu32 *) A_EE_SBUS_REGC0)
// SBUS Unknown Reg 0xD0
#define R_EE_SBUS_REGD0 ((vu32 *) A_EE_SBUS_REGD0)
// SBUS Unknown Reg 0xE0
#define R_EE_SBUS_REGE0 ((vu32 *) A_EE_SBUS_REGE0)
// SBUS Unknown Reg 0xF0
#define R_EE_SBUS_REGF0 ((vu32 *) A_EE_SBUS_REGF0)

// PGIF GPU Status
#define R_EE_PGIF_GPU_STAT ((vu32 *) A_EE_PGIF_GPU_STAT)
// PGIF Unknown Reg 0x10
#define R_EE_PGIF_REG10 ((vu32 *) A_EE_PGIF_REG10)
// PGIF Unknown Reg 0x20
#define R_EE_PGIF_REG20 ((vu32 *) A_EE_PGIF_REG20)
// PGIF Unknown Reg 0x30
#define R_EE_PGIF_REG30 ((vu32 *) A_EE_PGIF_REG30)
// PGIF Unknown Reg 0x40
#define R_EE_PGIF_REG40 ((vu32 *) A_EE_PGIF_REG40)
// PGIF Unknown Reg 0x50
#define R_EE_PGIF_REG50 ((vu32 *) A_EE_PGIF_REG50)
// PGIF Unknown Reg 0x60
#define R_EE_PGIF_REG60 ((vu32 *) A_EE_PGIF_REG60)
// PGIF Unknown Reg 0x70
#define R_EE_PGIF_REG70 ((vu32 *) A_EE_PGIF_REG70)
// PGIF GPU Command FIFO Status
#define R_EE_PGIF_CFIFO_STAT ((vu32 *) A_EE_PGIF_CFIFO_STAT)
// PGIF Unknown Reg 0x90
#define R_EE_PGIF_REG90 ((vu32 *) A_EE_PGIF_REG90)
// PGIF Unknown Reg 0xA0
#define R_EE_PGIF_REGA0 ((vu32 *) A_EE_PGIF_REGA0)
// PGIF Unknown Reg 0xB0
#define R_EE_PGIF_REGB0 ((vu32 *) A_EE_PGIF_REGB0)
// PGIF GPU Command FIFO Data
#define R_EE_PGIF_CFIFO_DATA ((vu32 *) A_EE_PGIF_CFIFO_DATA)
// PGIF Unknown Reg 0xD0
#define R_EE_PGIF_REGD0 ((vu32 *) A_EE_PGIF_REGD0)
// PGIF Unknown Reg 0xE0
#define R_EE_PGIF_REGE0 ((vu32 *) A_EE_PGIF_REGE0)
// PGIF Unknown Reg 0xF0
#define R_EE_PGIF_REGF0 ((vu32 *) A_EE_PGIF_REGF0)
// Acquisition of DMA suspend status
#define R_EE_D_ENABLER ((vu32 *) A_EE_D_ENABLER)
// DMA suspend control
#define R_EE_D_ENABLEW ((vu32 *) A_EE_D_ENABLEW)

// EE GS Registers

// Various PCRTC modes
#define R_EE_GS_PMODE ((vu64 *) A_EE_GS_PMODE)
// Related to Sync
#define R_EE_GS_SMODE1 ((vu64 *) A_EE_GS_SMODE1)
// Related to Sync
#define R_EE_GS_SMODE2 ((vu64 *) A_EE_GS_SMODE2)
// DRAM refresh
#define R_EE_GS_SRFSH ((vu64 *) A_EE_GS_SRFSH)
// Related to Sync
#define R_EE_GS_SYNCH1 ((vu64 *) A_EE_GS_SYNCH1)
// Related to Sync
#define R_EE_GS_SYNCH2 ((vu64 *) A_EE_GS_SYNCH2)
// Related to Sync/start
#define R_EE_GS_SYNCV ((vu64 *) A_EE_GS_SYNCV)
// Related to display buffer of Rectangular Area 1
#define R_EE_GS_DISPFB1 ((vu64 *) A_EE_GS_DISPFB1)
// Rectangular Area 1 display position etc.
#define R_EE_GS_DISPLAY1 ((vu64 *) A_EE_GS_DISPLAY1)
// Related to display buffer of Rectangular Area 2
#define R_EE_GS_DISPFB2 ((vu64 *) A_EE_GS_DISPFB2)
// Rectangular Area 2 display position etc.
#define R_EE_GS_DISPLAY2 ((vu64 *) A_EE_GS_DISPLAY2)
// Rectangular area write buffer
#define R_EE_GS_EXTBUF ((vu64 *) A_EE_GS_EXTBUF)
// Rectangular area write data
#define R_EE_GS_EXTDATA ((vu64 *) A_EE_GS_EXTDATA)
// Rectangular area write start
#define R_EE_GS_EXTWRITE ((vu64 *) A_EE_GS_EXTWRITE)
// Background color
#define R_EE_GS_BGCOLOR ((vu64 *) A_EE_GS_BGCOLOR)
// Various GS status
#define R_EE_GS_CSR ((vu64 *) A_EE_GS_CSR)
// Interrupt mask
#define R_EE_GS_IMR ((vu64 *) A_EE_GS_IMR)
// Host I/F switching
#define R_EE_GS_BUSDIR ((vu64 *) A_EE_GS_BUSDIR)

#define EE_CHCR_MOD_NORM  (0)
#define EE_CHCR_MOD_CHAIN (1)
#define EE_CHCR_MOD_INTER (2)

#define EE_CHCR_MOD (3 << 2)
#define EE_CHCR_ASP (3 << 4)
#define EE_CHCR_TTE (1 << 6)
#define EE_CHCR_TIE (1 << 7)
#define EE_CHCR_STR (1 << 8)

#define EE_I_STAT_GS    (1 << 0)
#define EE_I_STAT_SBUS  (1 << 1)
#define EE_I_STAT_VSS   (1 << 2)
#define EE_I_STAT_VSE   (1 << 3)

#ifdef __cplusplus
}
#endif

#endif
