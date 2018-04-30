#ifndef SIO2REGS_H
#define SIO2REGS_H


//SIO2 registers:

#define SIO2_REL_BASE 0x1F808000
#define SIO2_SEG 0xA0000000 //Uncached
#define SIO2_BASE (SIO2_REL_BASE + SIO2_SEG)

#define SIO2_REG_BASE (SIO2_BASE + 0x200)
#define SIO2_REG_PORT0_CTRL1 (SIO2_BASE + 0x240)
#define SIO2_REG_PORT0_CTRL2 (SIO2_BASE + 0x244)
#define SIO2_REG_DATA_OUT (SIO2_BASE + 0x260)
#define SIO2_REG_DATA_IN (SIO2_BASE + 0x264)
#define SIO2_REG_CTRL (SIO2_BASE + 0x268)
#define SIO2_REG_STAT6C (SIO2_BASE + 0x26C)
#define SIO2_REG_STAT70 (SIO2_BASE + 0x270)
#define SIO2_REG_STAT74 (SIO2_BASE + 0x274)
#define SIO2_REG_UNKN78 (SIO2_BASE + 0x278)
#define SIO2_REG_UNKN7C (SIO2_BASE + 0x27C)
#define SIO2_REG_STAT (SIO2_BASE + 0x280)

#define SIO2_A_QUEUE (SIO2_BASE + 0x200)
#define SIO2_A_PORT0_CTRL1 (SIO2_BASE + 0x240)
#define SIO2_A_PORT0_CTRL2 (SIO2_BASE + 0x244)
#define SIO2_A_DATA_OUT (SIO2_BASE + 0x260)
#define SIO2_A_DATA_IN (SIO2_BASE + 0x264)
#define SIO2_A_CTRL (SIO2_BASE + 0x268)
#define SIO2_A_STAT6C (SIO2_BASE + 0x26C)
#define SIO2_A_CONN_STAT70 (SIO2_BASE + 0x270)
#define SIO2_A_FIFO_STAT74 (SIO2_BASE + 0x274)
#define SIO2_A_TX_FIFO_PT (SIO2_BASE + 0x278)
#define SIO2_A_RX_FIFO_PT (SIO2_BASE + 0x27C)
#define SIO2_A_INTR_STAT (SIO2_BASE + 0x280)

//Bit-field definitions:

#define PCTRL0_ATT_LOW_PER(x) (((x) << 0) & 0xFF)
#define PCTRL0_ATT_MIN_HIGH_PER(x) (((x) << 8) & 0xFF00)
#define PCTRL0_BAUD0_DIV(x) (((x) << 16) & 0xFF0000)
#define PCTRL0_BAUD1_DIV(x) (((x) << 24) & 0xFF000000)

#define PCTRL1_ACK_TIMEOUT_PER(x) (((x) << 0) & 0xFFFF)
#define PCTRL1_INTER_BYTE_PER(x) (((x) << 16) & 0xFF0000)
#define PCTRL1_UNK24(x) (((x)&1) << 24)
#define PCTRL1_IF_MODE_SPI_DIFF(x) (((x)&1) << 25)

#define TR_CTRL_PORT_NR(x) (((x)&0x3) << 0)
#define TR_CTRL_PAUSE(x) (((x)&1) << 2)
#define TR_CTRL_UNK03(x) (((x)&1) << 3)
//Each of the folowing is 0 for PIO transfer in the given direction. to select DMA, set to 1.
#define TR_CTRL_TX_MODE_PIO_DMA(x) (((x)&1) << 4)
#define TR_CTRL_RX_MODE_PIO_DMA(x) (((x)&1) << 5)
/*
normal	special
0			0				no transfer done(???)
1			0				normal transfer - usually used
0			1				"special" transfer (not known to have been ever used by anything, no known difference from normal).
1			1				no transfer takes place
*/
#define TR_CTRL_NORMAL_TR(x) (((x)&1) << 6)
#define TR_CTRL_SPECIAL_TR(x) (((x)&1) << 7)
//In bytes 0 - 0x100:
#define TR_CTRL_TX_DATA_SZ(x) (((x)&0x1FF) << 8)
#define TR_CTRL_UNK17(x) (((x)&1) << 17)
#define TR_CTRL_RX_DATA_SZ(x) (((x)&0x1FF) << 18)
#define TR_CTRL_UNK27(x) (((x)&1) << 27)
//28 and 29 can't be set
#define TR_CTRL_UNK28(x) (((x)&1) << 28)
#define TR_CTRL_UNK29(x) (((x)&1) << 29)
//selects between baud rate divisors 0 and 1
#define TR_CTRL_BAUD_DIV(x) (((x)&1) << 30)
#define TR_CTRL_WAIT_ACK_FOREVER(x) (((x)&1) << 31)

//8268 SIO2 (main) CTRL register.
#define CTRL_TR_START(x) (((x)&1) << 0)
#define CTRL_TR_RESUME_PAUSED(x) (((x)&1) << 1)
#define CTRL_RESET_STATE(x) (((x)&1) << 2)
#define CTRL_RESET_FIFOS(x) (((x)&1) << 3)
#define CTRL_USE_ACK_WAIT_TIMEOUT(x) (((x)&1) << 4)
#define CTRL_NO_MISSING_ACK_ERR(x) (((x)&1) << 5)
//bit6 unknonw r/o, 0
#define CTRL_UNK06(x) (((x)&1) << 6)
//bit7 unknonw r/w, usually set to 1 by software
#define CTRL_UNK07(x) (((x)&1) << 7)
#define CTRL_ERROR_INTR_EN(x) (((x)&1) << 8)
#define CTRL_TR_COMP_INTR_EN(x) (((x)&1) << 9)
//bits 10, 11 unknonw r/w, usually set to 0 by software
#define CTRL_UNK10(x) (((x)&1) << 10)
#define CTRL_UNK11(x) (((x)&1) << 11)
//Unknown r/o, 0:
#define CTRL_UNK2912(x) (((x)&0x3FFFF) << 12)
#define CTRL_PS1_MODE_EN(x) (((x)&1) << 30)
#define CTRL_SLAVE_MODE_EN(x) (((x)&1) << 31)

//826C SIO2 (main) STAT register. (read-only)
#define STAT_UNK0300(x) (((x) >> 0) & 0xF)
//Needs more data through DMA to complete transfer:
#define STAT_NEED_DMA_DATA_TX(x) (((x) >> 4) & 1)
#define STAT_NEED_DMA_DATA_RX(x) (((x) >> 5) & 1)
#define STAT_UNK0706(x) (((x) >> 6) & 3)
//Queue slot number that is going to be processed next (0-15):
#define STAT_QUEUE_SLOT_PROC(x) (((x) >> 8) & 0xF)
//Kept clear only while tranfer is running, and set while idle. 1 can also signify transfer completion:
#define STAT_TR_IDLE(x) (((x) >> 12) & 1)
//FIFO underflow / overflow errors: (I am not sure if they really correspond to the TX and RX FIFOs)
#define STAT_ERR_TX_FIFO_UOFLOW(x) (((x) >> 13) & 1)
#define STAT_ERR_RX_FIFO_UOFLOW(x) (((x) >> 14) & 1)
#define STAT_ERR_ACK_MISSING(x) (((x) >> 15) & 1)
//Flags used to determine the transfers of which queue slots triggered erroirs. Slot = 0 - 15:
#define STAT_ERR_QUEUE_SLOT(x, slot) (((x) >> (slot + 16)) & 1)

//8270 SIO2 Device connected detection status register. (read-only)
//For ports 0-3. Only SAS0 and SAS1 are actually connected to the physical port and only on early PS2 models (~ SCPH-30000).
#define CONN_STAT_CDC0(x) (((x) >> 0) & 1)
#define CONN_STAT_CDC1(x) (((x) >> 1) & 1)
#define CONN_STAT_CDC2(x) (((x) >> 2) & 1)
#define CONN_STAT_CDC3(x) (((x) >> 3) & 1)
#define CONN_STAT_SAS0(x) (((x) >> 4) & 1)
#define CONN_STAT_SAS1(x) (((x) >> 5) & 1)
#define CONN_STAT_SAS2(x) (((x) >> 6) & 1)
#define CONN_STAT_SAS3(x) (((x) >> 7) & 1)

//8274 SIO2 FIFO status register. (read-only)
//values 0 - 0x100 bytes:
#define FSTAT_TX_SZ(x) (((x) >> 0) & 0x1FF)
#define FSTAT_TX_UNK09(x) (((x) >> 9) & 1)
#define FSTAT_TX_OVERFLOW(x) (((x) >> 10) & 1)
#define FSTAT_TX_UNDERFLOW(x) (((x) >> 11) & 1)
#define FSTAT_UNK1512(x) (((x) >> 12) & 0xF)
#define FSTAT_RX_SZ(x) (((x) >> 16) & 0x1FF)
#define FSTAT_RX_UNK09(x) (((x) >> 25) & 1)
#define FSTAT_RX_OVERFLOW(x) (((x) >> 26) & 1)
#define FSTAT_RX_UNDERFLOW(x) (((x) >> 27) & 1)
#define FSTAT_UNK3128(x) (((x) >> 28) & 0xF)

//8278 SIO2_REG_UNKN78	TX FIFO pointers. (read/write)
//#define GET_TXFPNTS_HEAD(x)	(((x)>>0)&0xFF)
//#define SET_TXFPNTS_HEAD(x)	(((x)>>0)&0xFF)
//...
//Better not add the FIFO pointers, as to change only one of them would require more complex macros.
//827C SIO2_REG_UNKN7C	RX FIFO pointers.

//8280	SIO2_REG_STAT	u32 Interrupt status. Writing 1 to an interrupt flag clears it.
#define GET_ISTAT_TR_COMP(x) (((x) >> 0) & 1)
//Any error:
#define GET_ISTAT_TR_ERR(x) (((x) >> 1) & 1)

/* 04 */ inline void sio2_ctrl_set(u32 val) { _sw(val, SIO2_REG_CTRL); }
/* 05 */ inline u32 sio2_ctrl_get() { return _lw(SIO2_REG_CTRL); }
/* 06 */ inline u32 sio2_stat6c_get() { return _lw(SIO2_REG_STAT6C); }
/* 07 */ inline void sio2_portN_ctrl1_set(int N, u32 val) { _sw(val, SIO2_REG_PORT0_CTRL1 + (N * 8)); }
/* 08 */ inline u32 sio2_portN_ctrl1_get(int N) { return _lw(SIO2_REG_PORT0_CTRL1 + (N * 8)); }
/* 09 */ inline void sio2_portN_ctrl2_set(int N, u32 val) { _sw(val, SIO2_REG_PORT0_CTRL2 + (N * 8)); }
/* 10 */ inline u32 sio2_portN_ctrl2_get(int N) { return _lw(SIO2_REG_PORT0_CTRL2 + (N * 8)); }
/* 11 */ inline u32 sio2_stat70_get() { return _lw(SIO2_REG_STAT70); }
/* 12 */ inline void sio2_regN_set(int N, u32 val) { _sw(val, SIO2_REG_BASE + (N * 4)); }
/* 13 */ inline u32 sio2_regN_get(int N) { return _lw(SIO2_REG_BASE + (N * 4)); }
/* 14 */ inline u32 sio2_stat74_get() { return _lw(SIO2_REG_STAT74); }
/* 15 */ inline void sio2_unkn78_set(u32 val) { _sw(val, SIO2_REG_UNKN78); }
/* 16 */ inline u32 sio2_unkn78_get() { return _lw(SIO2_REG_UNKN78); }
/* 17 */ inline void sio2_unkn7c_set(u32 val) { _sw(val, SIO2_REG_UNKN7C); }
/* 18 */ inline u32 sio2_unkn7c_get() { return _lw(SIO2_REG_UNKN7C); }
/* 19 */ inline void sio2_data_out(u8 val) { _sb(val, SIO2_REG_DATA_OUT); }
/* 20 */ inline u8 sio2_data_in() { return _lb(SIO2_REG_DATA_IN); }
/* 21 */ inline void sio2_stat_set(u32 val) { _sw(val, SIO2_REG_STAT); }
/* 22 */ inline u32 sio2_stat_get() { return _lw(SIO2_REG_STAT); }

inline u16 sio2_data_inh() { return _lh(SIO2_REG_DATA_IN); }
inline u32 sio2_data_inw() { return _lw(SIO2_REG_DATA_IN); }

#endif
