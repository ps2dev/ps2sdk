#ifndef __CC_H__
#define __CC_H__

#include <errno.h>

#define BYTE_ORDER LITTLE_ENDIAN

typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   long    u32_t;
typedef signed     long    s32_t;

typedef u32_t mem_ptr_t;

/* #define HAVE_BITIFIELDS */


#define PACK_STRUCT_FIELD(x) x __attribute((packed))
#define PACK_STRUCT_STRUCT __attribute((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END 

#define LWIP_PLATFORM_DIAG(___x) printf(___x)

// todo: this should be fatal!
#define LWIP_PLATFORM_ASSERT(___x) printf(___x)

#define U16_F %ud
#define S16_F %d
#define X16_F %04X

#define U32_F %ud
#define S32_F %d
#define X32_F %08X

#define SZT_F %d

#endif /* __CC_H__ */
