#ifndef __GIF_REGISTERS_H__
#define __GIF_REGISTERS_H__

// GIF Registers

#define GIF_REG_CTRL	*(volatile u32 *)0x10003000	// Control Register
#define GIF_REG_MODE	*(volatile u32 *)0x10003010	// Mode Setting Register
#define GIF_REG_STAT	*(volatile u32 *)0x10003020	// Status Register
#define GIF_REG_TAG0	*(volatile u32 *)0x10003040	// GIFtag Save Register
#define GIF_REG_TAG1	*(volatile u32 *)0x10003050	// GIFtag Save Register
#define GIF_REG_TAG2	*(volatile u32 *)0x10003060	// GIFtag Save Register
#define GIF_REG_TAG3	*(volatile u32 *)0x10003070	// GIFtag Save Register
#define GIF_REG_CNT		*(volatile u32 *)0x10003080	// Count Register
#define GIF_REG_P3CNT	*(volatile u32 *)0x10003090	// PATH3 Count Register
#define GIF_REG_P3TAG	*(volatile u32 *)0x100030A0	// PATH3 Tag Register

#define GIF_SET_CTRL(RST,PSE) \
	(u32)((RST) & 0x00000001) <<  0 | (u32)((PSE) & 0x00000001) <<  3

#define GIF_SET_MODE(M3R,IMT) \
	(u32)((M3R) & 0x00000001) <<  0 | (u32)((IMT) & 0x00000001) <<  2

typedef struct {
	u32 m3r:1;
	u32 m3p:1;
	u32 imt:1;
	u32 pse:1;
	u32 ip3:1;
	u32 p3q:1;
	u32 p2q:1;
	u32 p1q:1;
	u32 oph:1;
	u32 apath:2;
	u32 dir:1;
	u32 pad0:11;
	u32 fqc:5;
	u32 pad1:3;
} __attribute__((packed)) GIFSTAT;

typedef struct {
	u32 nloop:15;
	u32 eop:1;
	u32 tag:16;
} __attribute__((packed)) GIFTAG0;

typedef struct {
	u32 tag:14;
	u32 pre:1;
	u32 prim:11;
	u32 flg:2;
	u32 nreg:4;
} __attribute__((packed)) GIFTAG1;

typedef struct {
	u32 loregs;
} GIFTAG2;

typedef struct {
	u32 hiregs;
} GIFTAG3;

typedef struct {
	u32 loopcnt:15;
	u32 pad0:1;
	u32 regcnt:4;
	u32 vuaddr:10;
	u32 pad1:2;
} __attribute__((packed)) GIFCNT;

typedef struct {
	u32 p3cnt:15;
	u32 pad0:17;
} __attribute__((packed)) GIFP3CNT;

typedef struct {
	u32 loopcnt:15;
	u32 eop:1;
	u32 pad0:16;
} __attribute__((packed)) GIFP3TAG;

#endif /*__GIF_REGISTERS_H__*/
