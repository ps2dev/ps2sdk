#ifndef __CC_H__
#define __CC_H__

#include <errno.h>
#include <stddef.h>

#define BYTE_ORDER LITTLE_ENDIAN

typedef unsigned char		u8_t;
typedef signed char		s8_t;
typedef unsigned short int	u16_t;
typedef signed short		s16_t;
typedef unsigned int		u32_t;
typedef signed int		s32_t;

typedef u32_t			mem_ptr_t;

/* Define (sn)printf formatters for these lwIP types */
#define U8_F "hu"
#define S8_F "hd"
#define X8_F "hx"
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"
#define SZT_F "uz"

#define PACK_STRUCT_FIELD(x) x __attribute((packed))
#define PACK_STRUCT_STRUCT __attribute((packed))
/* Used for struct fields of u8_t,
 * where some compilers warn that packing is not necessary */
#define PACK_STRUCT_FLD_8(x) x
/* Used for struct fields of that are packed structs themself,
 * where some compilers warn that packing is not necessary */
#define PACK_STRUCT_FLD_S(x) x
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

#ifdef DEBUG
#define LWIP_PLATFORM_DIAG(args) printf args
#define LWIP_PLATFORM_ASSERT(args) printf args
#else
#define LWIP_PLATFORM_DIAG(args)
#define LWIP_PLATFORM_ASSERT(args)
#endif

#define atoi(x) strtol(x, NULL, 10)

#define LWIP_NO_STDINT_H	1	//stdint.h does not exist.
#define LWIP_NO_INTTYPES_H	1	//inttypes.h does not exist.

#define lwip_htons(x) PP_HTONS(x)
#define lwip_htonl(x) PP_HTONL(x)

#endif /* __CC_H__ */
