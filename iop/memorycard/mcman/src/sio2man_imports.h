#ifndef __SIO2MAN_IMPORTS_H__
#define __SIO2MAN_IMPORTS_H__

#include <types.h>
#include <irx.h>
#include <sio2man.h>

#define SIO2MAN 0
#define XSIO2MAN 1

// sio2man exports
/* 24 */ void (*sio2_mc_transfer_init)(void);
/* 25 */ int (*sio2_transfer)(sio2_transfer_data_t *sio2data);

// xsio2man exports
/* 26 */ void (*sio2_transfer_reset)(void);
/* 55 */ int (*sio2_func1)(void *arg);

#endif
