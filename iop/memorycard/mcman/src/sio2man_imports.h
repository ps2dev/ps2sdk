#ifndef __SIO2MAN_IMPORTS_H__
#define __SIO2MAN_IMPORTS_H__

#include <types.h>
#include <irx.h>
#include <sio2man.h>

#define SIO2MAN 	0
#define XSIO2MAN 	1
#define XSIO2MAN_V2 	2

// sio2man exports
/* 24 */ void (*psio2_mc_transfer_init)(void);
/* 25 */ int  (*psio2_transfer)(sio2_transfer_data_t *sio2data);

// xsio2man exports
/* 26 */ void (*psio2_transfer_reset)(void);
/* 55/57 */ int  (*psio2_mtap_change_slot)(s32 *arg);

#endif
