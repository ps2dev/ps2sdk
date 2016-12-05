#ifndef __CC_H__
#define __CC_H__

#include <errno.h>

#define BYTE_ORDER LITTLE_ENDIAN

typedef unsigned char		u8_t;
typedef signed char		s8_t;
typedef unsigned short int	u16_t;
typedef signed short		s16_t;
typedef unsigned int		u32_t;
typedef signed int		s32_t;

typedef u32_t mem_ptr_t;

/* Define (sn)printf formatters for these lwIP types */
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"

#define PACK_STRUCT_FIELD(x) x __attribute((packed))
#define PACK_STRUCT_STRUCT __attribute((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#define atoi(x) strtol(x, NULL, 10)

#endif /* __CC_H__ */
