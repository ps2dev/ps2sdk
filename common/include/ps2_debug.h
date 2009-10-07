#ifndef _PS2_DEBUG_H
#define _PS2_DEBUG_H

#include "tamtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_IOP_RegFrame
{
    u32 zero; // 0x00
    u32 at; // 0x04
    u32 v0; // 0x08
    u32 v1; // 0x0C
    u32 a0; // 0x10
    u32 a1; // 0x14
    u32 a2; // 0x18
    u32 a3; // 0x1C
    u32 t0; // 0x20
    u32 t1; // 0x24
    u32 t2; // 0x28
    u32 t3; // 0x2C
    u32 t4; // 0x30
    u32 t5; // 0x34
    u32 t6; // 0x38
    u32 t7; // 0x3C
    u32 s0; // 0x40
    u32 s1; // 0x44
    u32 s2; // 0x48
    u32 s3; // 0x4C
    u32 s4; // 0x50
    u32 s5; // 0x54
    u32 s6; // 0x58
    u32 s7; // 0x5C
    u32 t8; // 0x60
    u32 t9; // 0x64
    u32 k0; // 0x68
    u32 k1; // 0x6C
    u32 gp; // 0x70
    u32 sp; // 0x74
    u32 fp; // 0x78
    u32 ra; // 0x7C
    u32 hi; // 0x80
    u32 lo; // 0x84
    u32 bpc; // 0x88
    u32 bda; // 0x8C
    u32 dcic; // 0x90
    u32 badvaddr; // 0x94
    u32 bdam; // 0x98
    u32 bpcm; // 0x9C
    u32 status; // 0xA0
    u32 cause; // 0xA4
    u32 epc; // 0xA8
} IOP_RegFrame;

typedef struct st_EE_RegFrame
{
    u32 zero[4];
    u32 at[4];
    u32 v0[4];
    u32 v1[4];
    u32 a0[4];
    u32 a1[4];
    u32 a2[4];
    u32 a3[4];
    u32 t0[4];
    u32 t1[4];
    u32 t2[4];
    u32 t3[4];
    u32 t4[4];
    u32 t5[4];
    u32 t6[4];
    u32 t7[4];
    u32 s0[4];
    u32 s1[4];
    u32 s2[4];
    u32 s3[4];
    u32 s4[4];
    u32 s5[4];
    u32 s6[4];
    u32 s7[4];
    u32 t8[4];
    u32 t9[4];
    u32 k0[4];
    u32 k1[4];
    u32 gp[4];
    u32 sp[4];
    u32 fp[4];
    u32 ra[4];

    u32 status;
    u32 cause;
    u32 epc;
    u32 errorepc;
    u32 badvaddr;
    u32 hi;
    u32 hi1;
    u32 lo;
    u32 lo1;
    u32 sa;

    u32 bpc;
    u32 iab;
    u32 iabm;
    u32 dab;
    u32 dabm;
    u32 dvb;
    u32 dvbm;
} EE_RegFrame;

#ifdef __cplusplus
}
#endif

#endif
