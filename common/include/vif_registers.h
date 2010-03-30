#ifndef __VIF_REGISTERS_H__
#define __VIF_REGISTERS_H__

#include <tamtypes.h>

#define VU0_MEM0_START			0x11000000
#define VU0_MICROMEM0_START	0x11004000

#define VU1_MEM1_START			0x11008000
#define VU1_MICROMEM1_START	0x1100C000

#define VIF0_FIFO				0x10004000
#define VIF1_FIFO				0x10005000

#define VIF0_STAT		*(volatile u32 *)0x10003800
#define VIF0_FBRST		*(volatile u32 *)0x10003810
#define VIF0_ERR		*(volatile u32 *)0x10003820
#define VIF0_MARK		*(volatile u32 *)0x10003830
#define VIF0_CYCLE		*(volatile u32 *)0x10003840
#define VIF0_MODE		*(volatile u32 *)0x10003850
#define VIF0_NUM		*(volatile u32 *)0x10003860
#define VIF0_MASK		*(volatile u32 *)0x10003870
#define VIF0_CODE		*(volatile u32 *)0x10003880
#define VIF0_ITOPS		*(volatile u32 *)0x10003890
#define VIF0_ITOP		*(volatile u32 *)0x100038d0
#define VIF0_TOP		*(volatile u32 *)0x100038e0
#define VIF0_R0			*(volatile u32 *)0x10003900
#define VIF0_R1			*(volatile u32 *)0x10003910
#define VIF0_R2			*(volatile u32 *)0x10003920
#define VIF0_R3			*(volatile u32 *)0x10003930
#define VIF0_C0			*(volatile u32 *)0x10003940
#define VIF0_C1			*(volatile u32 *)0x10003950
#define VIF0_C2			*(volatile u32 *)0x10003960
#define VIF0_C3			*(volatile u32 *)0x10003970

#define VIF1_STAT		*(volatile u32 *)0x10003c00
#define VIF1_FBRST		*(volatile u32 *)0x10003c10
#define VIF1_ERR		*(volatile u32 *)0x10003c20
#define VIF1_MARK		*(volatile u32 *)0x10003c30
#define VIF1_CYCLE		*(volatile u32 *)0x10003c40
#define VIF1_MODE		*(volatile u32 *)0x10003c50
#define VIF1_NUM		*(volatile u32 *)0x10003c60
#define VIF1_MASK		*(volatile u32 *)0x10003c70
#define VIF1_CODE		*(volatile u32 *)0x10003c80
#define VIF1_ITOPS		*(volatile u32 *)0x10003c90
#define VIF1_BASE		*(volatile u32 *)0x10003ca0
#define VIF1_OFST		*(volatile u32 *)0x10003cb0
#define VIF1_TOPS		*(volatile u32 *)0x10003cc0
#define VIF1_ITOP		*(volatile u32 *)0x10003cd0
#define VIF1_TOP		*(volatile u32 *)0x10003ce0
#define VIF1_R0			*(volatile u32 *)0x10003d00
#define VIF1_R1			*(volatile u32 *)0x10003d10
#define VIF1_R2			*(volatile u32 *)0x10003d20
#define VIF1_R3			*(volatile u32 *)0x10003d30
#define VIF1_C0			*(volatile u32 *)0x10003d40
#define VIF1_C1			*(volatile u32 *)0x10003d50
#define VIF1_C2			*(volatile u32 *)0x10003d60
#define VIF1_C3			*(volatile u32 *)0x10003d70

#define VIF_SET_FBRST(RST,FBK,STP,STC) \
	(u32)((RST) & 0x00000001) << 0 | (u32)((FBK) & 0x00000001) << 1 | \
	(u32)((STP) & 0x00000001) << 2 | (u32)((STC) & 0x00000001) << 3

#define VIF_SET_ERR(MII,ME0,ME1) \
	(u32)((MII) & 0x00000001) << 0 | (u32)((ME0) & 0x00000001) << 1 | \
	(u32)((ME1) & 0x00000001) << 2

#define VIF_SET_MARK(MARK) \
	(u32)((MARK) & 0x0000FFFF) << 0

typedef struct {
	u32 vps:2;
	u32 vew:1;
	u32 vgw:1; //vif1
	u32 pad0:2;
	u32 mrk:1;
	u32 dbf:1; //vif1
	u32 vss:1;
	u32 vfs:1;
	u32 vis:1;
	u32 irq:1;
	u32 er0:1;
	u32 er1:1;
	u32 pad1:9;
	u32 fdr:1; //vif1
	u32 fqc:5;
	u32 pad2:3;
} __attribute__((packed)) VIFSTAT;

typedef struct {
	u32 cl:8;
	u32 wl:8;
	u32 pad0:16;
} __attribute__((packed)) VIFCYCLE;

typedef struct {
	u32 mode:2;
	u32 pad0:30;
} __attribute__((packed)) VIFMODE;

typedef struct {
	u32 m0:2;
	u32 m1:2;
	u32 m2:2;
	u32 m3:2;
	u32 m4:2;
	u32 m5:2;
	u32 m6:2;
	u32 m7:2;
	u32 m8:2;
	u32 m9:2;
	u32 m10:2;
	u32 m11:2;
	u32 m12:2;
	u32 m13:2;
	u32 m14:2;
	u32 m15:2;
} __attribute__((packed)) VIFMASK;

typedef struct {
	u32 imdt:16;
	u32 num:8;
	u32 cmd:8;
} __attribute__((packed)) VIFCODE;

typedef struct {
	u32 itops:10;
	u32 pad0:22;
} __attribute__((packed)) VIFITOPS;

typedef struct {
	u32 base:10;
	u32 pad0:22;
} __attribute__((packed)) VIF1BASE; //vif1

typedef struct {
	u32 offset:10;
	u32 pad0:22;
} __attribute__((packed)) VIF1OFST; //vif1

typedef struct {
	u32 tops:10;
	u32 pad0:22;
} __attribute__((packed)) VIF1TOPS; //vif1

typedef struct {
	u32 itop:10;
	u32 pad0:22;
} __attribute__((packed)) VIFITOP;

typedef struct {
	u32 top:10;
	u32 pad0:22;
} __attribute__((packed)) VIFTOP;

typedef struct {
	u32 row;
} VIFR;

typedef struct {
	u32 column;
} VIFC;

#endif /*__VIF_REGISTERS_H__*/
