/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Common definitions for the libcdvd RPC on the EE and IOP
 */

#ifndef __LIBCDVD_RPC_H__
#define __LIBCDVD_RPC_H__

#include <tamtypes.h>
#include <libcdvd-common.h>

/* S-command parameters */

struct cdvdScmdParam
{
    u16 cmdNum;
    u16 inBuffSize;
    u8 inBuff[16];
};

struct cdvdDecSetParam
{
    u8 arg1;
    u8 arg2;
    u8 shift;
    u8 pad;
};

struct cdvdReadWriteNvmParam
{
    u32 address;
    u16 value;
    u16 pad;
};

/* N-command parameters */

struct cdvdNcmdParam
{
    u16 cmdNum;
    u16 inBuffSize;
    u8 inBuff[16];
};

struct cdvdReadKeyParam
{
    u32 arg1;
    u32 arg2;
    u32 command;
};

/* SIF RPC packet definitions */

typedef struct cdvdfsv_rpc1_inpacket_
{
    int m_mode;
} cdvdfsv_rpc1_inpacket_t;

typedef struct cdvdfsv_rpc1_outpacket_
{
    int m_retres;
    int m_cdvdfsv_ver;
    int m_cdvdman_ver;
    int m_debug_mode;
} cdvdfsv_rpc1_outpacket_t;

typedef struct cdvdfsv_rpc2_inpacket_
{
    int m_mode;
} cdvdfsv_rpc2_inpacket_t;

typedef struct cdvdfsv_rpc2_outpacket_
{
    int m_retres;
} cdvdfsv_rpc2_outpacket_t;

typedef struct cdvdfsv_unaligned_data_outpacket_
{
    u32 m_b1len;
    u32 m_b2len;
    u32 m_b1dst;
    u32 m_b2dst;
    u8 m_pbuf1[64];
    u8 m_pbuf2[64];
} cdvdfsv_unaligned_data_outpacket_t;

typedef struct cdvdfsv_rpc4_sz12c_inpacket_
{
    sceCdlFILE m_fp;
    int m_file_attributes;
    char m_path[256];
    uiptr m_eedest;
    int m_layer;
} cdvdfsv_rpc4_sz12c_inpacket_t;

typedef struct cdvdfsv_rpc4_sz128_inpacket_
{
    sceCdlFILE m_fp;
    int m_file_attributes;
    char m_path[256];
    uiptr m_eedest;
} cdvdfsv_rpc4_sz128_inpacket_t;

typedef struct cdvdfsv_rpc4_sz124_inpacket_
{
    sceCdlFILE m_fp;
    char m_path[256];
    uiptr m_eedest;
} cdvdfsv_rpc4_sz124_inpacket_t;

typedef union cdvdfsv_rpc4_inpacket_
{
    cdvdfsv_rpc4_sz12c_inpacket_t m_pkt_sz12c;
    cdvdfsv_rpc4_sz128_inpacket_t m_pkt_sz128;
    cdvdfsv_rpc4_sz124_inpacket_t m_pkt_sz124;
} cdvdfsv_rpc4_inpacket_t;

typedef struct cdvdfsv_rpc4_outpacket_
{
    int m_retres;
    int m_padding[3];
} cdvdfsv_rpc4_outpacket_t;

typedef struct cdvdfsv_rpc3_05_inpacket_
{
    int m_param;
} cdvdfsv_rpc3_05_inpacket_t;

typedef struct cdvdfsv_rpc3_0B_inpacket_
{
    u8 m_cmdNum;
    u8 m_gap1;
    u16 m_inBuffSize;
    u8 m_inBuff[16];
} cdvdfsv_rpc3_0B_inpacket_t;

typedef struct cdvdfsv_rpc3_15_inpacket_
{
    int m_mode;
} cdvdfsv_rpc3_15_inpacket_t;

typedef struct cdvdfsv_rpc3_22_inpacket_
{
    int m_media;
    char m_char4;
} cdvdfsv_rpc3_22_inpacket_t;

typedef struct cdvdfsv_rpc3_23_inpacket_
{
    int m_priority;
} cdvdfsv_rpc3_23_inpacket_t;

typedef struct cdvdfsv_rpc3_25_inpacket_
{
    int m_param;
    int m_timeout;
} cdvdfsv_rpc3_25_inpacket_t;

typedef union cdvdfsv_rpc3_inpacket_
{
    cdvdfsv_rpc3_05_inpacket_t m_pkt_05;
    cdvdfsv_rpc3_0B_inpacket_t m_pkt_0B;
    cdvdfsv_rpc3_15_inpacket_t m_pkt_15;
    cdvdfsv_rpc3_22_inpacket_t m_pkt_22;
    cdvdfsv_rpc3_23_inpacket_t m_pkt_23;
    cdvdfsv_rpc3_25_inpacket_t m_pkt_25;
} cdvdfsv_rpc3_inpacket_t;

typedef struct cdvdfsv_rpc3_01_outpacket_
{
    int m_retres;
    sceCdCLOCK m_clock;
} cdvdfsv_rpc3_01_outpacket_t;

typedef struct cdvdfsv_rpc3_05_outpacket_
{
    int m_retres;
    u32 m_traychk;
} cdvdfsv_rpc3_05_outpacket_t;

typedef struct cdvdfsv_rpc3_06_outpacket_
{
    int m_retres;
    u32 m_result;
    u8 m_buffer[8];
} cdvdfsv_rpc3_06_outpacket_t;

typedef struct cdvdfsv_rpc3_0B_outpacket_
{
    u8 m_outbuf[16];
} cdvdfsv_rpc3_0B_outpacket_t;

typedef struct cdvdfsv_rpc3_15_outpacket_
{
    int m_retres;
    u32 m_status;
} cdvdfsv_rpc3_15_outpacket_t;

typedef struct cdvdfsv_rpc3_1A_outpacket_
{
    int m_retres;
    u32 m_status;
    char m_buffer[16];
} cdvdfsv_rpc3_1A_outpacket_t;

typedef struct cdvdfsv_rpc3_21_outpacket_
{
    int m_retres;
    u32 m_result;
} cdvdfsv_rpc3_21_outpacket_t;

typedef struct cdvdfsv_rpc3_24_outpacket_
{
    int m_retres;
    u64 m_guid;
} cdvdfsv_rpc3_24_outpacket_t;

typedef struct cdvdfsv_rpc3_26_outpacket_
{
    int m_retres;
    unsigned int m_id;
} cdvdfsv_rpc3_26_outpacket_t;

typedef struct cdvdfsv_rpc3_27_outpacket_
{
    int m_retres;
    int m_on_dual;
    unsigned int m_layer1_start;
} cdvdfsv_rpc3_27_outpacket_t;

typedef union cdvdfsv_rpc3_outpacket_
{
    int m_retres;
    cdvdfsv_rpc3_01_outpacket_t m_pkt_01;
    cdvdfsv_rpc3_05_outpacket_t m_pkt_05;
    cdvdfsv_rpc3_06_outpacket_t m_pkt_06;
    cdvdfsv_rpc3_0B_outpacket_t m_pkt_0B;
    cdvdfsv_rpc3_15_outpacket_t m_pkt_15;
    cdvdfsv_rpc3_1A_outpacket_t m_pkt_1A;
    cdvdfsv_rpc3_21_outpacket_t m_pkt_21;
    cdvdfsv_rpc3_24_outpacket_t m_pkt_24;
    cdvdfsv_rpc3_26_outpacket_t m_pkt_26;
    cdvdfsv_rpc3_27_outpacket_t m_pkt_27;
} cdvdfsv_rpc3_outpacket_t;

typedef struct cdvdfsv_rpc5_01_inpacket_
{
    u32 m_lbn;
    u32 m_sectors;
    uiptr m_paddr;
    sceCdRMode m_rmodeee;
    uiptr m_eeremaindest;
    uiptr m_eedest;
    u32 m_decval;
} cdvdfsv_rpc5_01_inpacket_t;

typedef struct cdvdfsv_rpc5_02_inpacket_
{
    u32 m_lbn;
    u32 m_sectors;
    uiptr m_buf;
    sceCdRMode m_mode;
    uiptr m_eeremaindest;
    uiptr m_eedest;
} cdvdfsv_rpc5_02_inpacket_t;

typedef struct cdvdfsv_rpc5_04_inpacket_
{
    uiptr m_eedest;
} cdvdfsv_rpc5_04_inpacket_t;

typedef struct cdvdfsv_rpc5_05_inpacket_
{
    u32 m_lbn;
} cdvdfsv_rpc5_05_inpacket_t;

typedef struct cdvdfsv_rpc5_0C_inpacket_
{
    u8 m_cmdNum;
    u8 m_gap1;
    u16 m_inBuffSize;
    u8 m_inBuff[16];
} cdvdfsv_rpc5_0C_inpacket_t;

typedef struct cdvdfsv_rpc5_0D_inpacket_
{
    u32 m_lbn;
    u32 m_sectors;
    void *m_buf;
    sceCdRMode m_mode;
    u8 m_unused[4];
    uiptr m_eedest;
} cdvdfsv_rpc5_0D_inpacket_t;

typedef struct cdvdfsv_rpc5_0F_inpacket_
{
    sceCdRChain m_readChain[65];
    sceCdRMode m_mode;
    uiptr m_eedest;
} cdvdfsv_rpc5_0F_inpacket_t;

typedef union cdvdfsv_rpc5_inpacket_
{
    cdvdfsv_rpc5_01_inpacket_t m_pkt_01;
    cdvdfsv_rpc5_02_inpacket_t m_pkt_02;
    cdvdfsv_rpc5_04_inpacket_t m_pkt_04;
    cdvdfsv_rpc5_05_inpacket_t m_pkt_05;
    cdvdfsv_rpc5_0C_inpacket_t m_pkt_0C;
    cdvdfsv_rpc5_0D_inpacket_t m_pkt_0D;
    cdvdfsv_rpc5_0F_inpacket_t m_pkt_0F;
} cdvdfsv_rpc5_inpacket_t;

typedef struct cdvdfsv_rpc5_04_outpacket_
{
    int m_retres;
    int m_isdvd;
} cdvdfsv_rpc5_04_outpacket_t;

typedef struct cdvdfsv_rpc5_11_outpacket_
{
    int m_retres;
    u8 m_diskid[5];
} cdvdfsv_rpc5_11_outpacket_t;

typedef struct cdvdfsv_rpc5_17_outpacket_
{
    int m_retres;
    u32 m_status;
} cdvdfsv_rpc5_17_outpacket_t;

typedef union cdvdfsv_rpc5_outpacket_
{
    int m_retres;
    cdvdfsv_rpc5_04_outpacket_t m_pkt_04;
    cdvdfsv_rpc5_11_outpacket_t m_pkt_11;
    cdvdfsv_rpc5_17_outpacket_t m_pkt_17;
} cdvdfsv_rpc5_outpacket_t;

#endif /* _LIBCDVD_RPC_H_ */
