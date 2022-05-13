#ifndef __CC_H__
#define __CC_H__

#include <errno.h>
#include <stdlib.h>
#include <stddef.h>

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
#include <stdio.h>
#define LWIP_PLATFORM_DIAG(args) printf args
#define LWIP_PLATFORM_ASSERT(args) printf args
#else
#define LWIP_PLATFORM_DIAG(args)
#define LWIP_PLATFORM_ASSERT(args)
#endif

#define lwip_htons(x) PP_HTONS(x)
#define lwip_htonl(x) PP_HTONL(x)

// I think there is an issue in the lwip code that doesn't import stdlib properly
#define LWIP_RAND() ((u32_t)rand())

#endif /* __CC_H__ */
