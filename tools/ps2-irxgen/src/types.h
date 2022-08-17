/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2022, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

static inline uint32_t lw_le(uint32_t data)
{
	uint8_t *ptr;
	uint32_t val;

	ptr = (uint8_t*) &data;

	val = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);

	return val;
}

static inline uint16_t lh_le(uint16_t data)
{
	uint8_t *ptr;
	uint16_t val;

	ptr = (uint8_t*) &data;

	val = ptr[0] | (ptr[1] << 8);

	return val;
}

static inline uint32_t lw_be(uint32_t data)
{
	uint8_t *ptr;
	uint32_t val;

	ptr = (uint8_t*) &data;

	val = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];

	return val;
}

static inline uint16_t lh_be(uint16_t data)
{
	uint8_t *ptr;
	uint16_t val;

	ptr = (uint8_t*) &data;

	val = (ptr[0] << 16) | ptr[1];

	return val;
}

#define LW_LE(x) (lw_le((x)))
#define LW_BE(x) (lw_be((x)))
#define LH_LE(x) (lh_le((x)))
#define LH_BE(x) (lh_be((x)))

#define LW(x) (LW_LE(x))
#define LH(x) (LH_LE(x))


static inline void sw_le(uint8_t *data, uint32_t val)
{
	uint8_t* ptr = (uint8_t*) data;

	ptr[0] = (uint8_t) (val & 0xFF);
	ptr[1] = (uint8_t) ((val >> 8) & 0xFF);
	ptr[2] = (uint8_t) ((val >> 16) & 0xFF);
	ptr[3] = (uint8_t) ((val >> 24) & 0xFF);
}

static inline void sh_le(uint8_t *data, uint16_t val)
{
	uint8_t *ptr = (uint8_t*) data;

	ptr[0] = (uint8_t) (val & 0xFF);
	ptr[1] = (uint8_t) ((val >> 8) & 0xFF);
}

static inline void sw_be(uint8_t *data, uint32_t val)
{
	uint8_t *ptr = (uint8_t*) data;

	ptr[0] = (uint8_t) ((val >> 24) & 0xFF);
	ptr[1] = (uint8_t) ((val >> 16) & 0xFF);
	ptr[2] = (uint8_t) ((val >> 8) & 0xFF);
	ptr[3] = (uint8_t) (val & 0xFF);
}

static inline void sh_be(uint8_t *data, uint16_t val)
{
	uint8_t* ptr = (uint8_t*) data;

	ptr[0] = (uint8_t) ((val >> 8) & 0xFF);
	ptr[1] = (uint8_t) (val & 0xFF);
}

#define SW_LE(x, v) (sw_le((uint8_t *)(x), (v)))
#define SW_BE(x, v) (sw_be((uint8_t *)(x), (v)))
#define SH_LE(x, v) (sh_le((uint8_t *)(x), (v)))
#define SH_BE(x, v) (sh_be((uint8_t *)(x), (v)))

#define SW(x, v) (SW_LE(x, v))
#define SH(x, v) (SH_LE(x, v))

#endif
