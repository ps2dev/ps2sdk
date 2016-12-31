/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: libmc-common.h  $
# Common definitions for libmc on the EE and IOP
*/

#ifndef _LIBMC_COMMON_H_
#define _LIBMC_COMMON_H_

typedef struct _sceMcStDateTime {
	u8  Resv2;
	u8  Sec;
	u8  Min;
	u8  Hour;
	u8  Day;
	u8  Month;
	u16 Year;
} sceMcStDateTime;

/* MCMAN public structures */
typedef struct {			// size = 128
	int mode;			// 0
	int length;			// 4
	s16 linked_block;		// 8
	char  name[20];			// 10
	u8  field_1e;			// 30
	u8  field_1f;			// 31
	sceMcStDateTime created;	// 32
	int field_28;			// 40
	u16 field_2c;			// 44
	u16 field_2e;			// 46
	sceMcStDateTime modified;	// 48
	int field_38;			// 56
	u8  unused2[65];		// 60
	u8  field_7d;			// 125
	u8  field_7e;			// 126
	u8  edc;			// 127
} McFsEntryPS1;

typedef struct {			// size = 512
	u16 mode;			// 0
	u16 unused;			// 2
	u32 length;			// 4
	sceMcStDateTime created;	// 8
	u32 cluster;			// 16
	u32 dir_entry;			// 20
	sceMcStDateTime modified;	// 24
	u32 attr;			// 32
	u32 unused2[7];			// 36
	char  name[32];			// 64
	u8  unused3[416];		// 96
} McFsEntry;

// file descriptor related mc command
// used by: McInit, McClose, McSeek, McRead, McWrite, McGetinfo, McFormat, McFlush, McUnformat
typedef struct {	// size = 48
	int fd;		// 0
	int port;	// 4
	int slot;	// 8
	int size;	// 12
	int offset;	// 16
	int origin;	// 20
	void *buffer;	// 24
	void *param;	// 28
	u8 data[16];	// 32
} mcDescParam_t;

// endParamenter struct
// used by: McRead, McGetInfo, McReadPage
typedef struct {		// size = 64
	union {
		s32	size1; 	// 0
		s32	type;
	};
	union {
		s32	size2;	// 4
		s32	free;
	};
	void	*dest1;		// 8
	void	*dest2;		// 12
	u8	src1[16];	// 16
	u8	src2[16];	// 32
	u8	unused[16];	// 48
} mcEndParam_t;

// endParamenter2 struct
// used by: McRead2, McGetInfo2
typedef struct {			// size = 192
	union {
		s32	size1; 		// 0
		s32	type;
	};
	union {
		s32	size2;		// 4
		s32	free;
	};
	void	*dest1;			// 8
	void	*dest2;			// 12
	u8	src1[64];	 	// 16
	u8	src2[64];		// 80
	union {
		s32	formatted;	// 144
		u8	unused[48];
	};
} mcEndParam2_t;

typedef struct {
	s32 result;
	s32 mcserv_version;
	s32 mcman_version;
} mcRpcStat_t;

#endif // _LIBMC_COMMON_H_
