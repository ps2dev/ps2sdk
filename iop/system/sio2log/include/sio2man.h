/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
*/

#ifndef SIO2MAN_H
#define SIO2MAN_H

struct _sio2_dma_arg { // size 12
	void	*addr;
	int	size;
	int	count; 
};

typedef struct {
	u32	stat6c;

	u32	port_ctrl1[4];
	u32	port_ctrl2[4]; 

	u32	stat70;

	u32	regdata[16]; 

	u32	stat74; 

	u32	in_size; 
	u32	out_size; 
	u8	*in;
	u8	*out; 

	struct _sio2_dma_arg in_dma; 
	struct _sio2_dma_arg out_dma;
} sio2_transfer_data_t;

/* 04 */ void sio2_ctrl_set(u32 val);
/* 05 */ u32  sio2_ctrl_get(void);
/* 06 */ u32  sio2_stat6c_get(void);
/* 07 */ void sio2_portN_ctrl1_set(int N, u32 val);
/* 08 */ u32  sio2_portN_ctrl1_get(int N);
/* 09 */ void sio2_portN_ctrl2_set(int N, u32 val);
/* 10 */ u32  sio2_portN_ctrl2_get(int N);
/* 11 */ u32  sio2_stat70_get(void);
/* 12 */ void sio2_regN_set(int N, u32 val);
/* 13 */ u32  sio2_regN_get(int N);
/* 14 */ u32  sio2_stat74_get(void);
/* 15 */ void sio2_unkn78_set(u32 val);
/* 16 */ u32  sio2_unkn78_get(void);
/* 17 */ void sio2_unkn7c_set(u32 val);
/* 18 */ u32  sio2_unkn7c_get(void);
/* 19 */ void sio2_data_out(u8 val);
/* 20 */ u8   sio2_data_in(void);
/* 21 */ void sio2_stat_set(u32 val);
/* 22 */ u32  sio2_stat_get(void);

#endif /* SIO2MAN_H */
