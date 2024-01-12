// "License": Public Domain
// I, Mathias Panzenbck, place this file hereby into the public domain. Use it at your own risk for whatever you like.
// In case there are jurisdictions that don't support putting things in the public domain you can also consider it to
// be "dual licensed" under the BSD, MIT and Apache licenses, if you want to. This code is trivial anyway. Consider it
// an example on how to get the endian conversion functions on different platforms.

#ifndef PORTABLE_ENDIAN_H__
#define PORTABLE_ENDIAN_H__

#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__) && !defined(_XBOX) 
#define __WINDOWS__
#endif

#if defined(ESP_PLATFORM)

// These 4 #defines may be needed with older esp-idf environments
//#       define _LITTLE_ENDIAN LITTLE_ENDIAN
//#       define __bswap16     __bswap_16
//#       define __bswap32     __bswap_32
//#       define __bswap64     __bswap_64

#	include <endian.h>

#elif defined(PICO_PLATFORM)

#   include <machine/endian.h>
#   include "lwip/def.h"

#   define be16toh(x) PP_NTOHS(x)
#   define htobe16(x) PP_HTONS(x)
#   define htole16(x) (x)
#   define le16toh(x) (x)

#   define be32toh(x) PP_NTOHL(x)
#   define htobe32(x) PP_HTONL(x)
#   define htole32(x) (x)
#   define le32toh(x) (x)

#   define htobe64(x) be64toh(x)
#   define htole64(x) (x)
#   define le64toh(x) (x)

#elif defined(PS2_IOP_PLATFORM)

#	include <tcpip.h>

#       define _LITTLE_ENDIAN LITTLE_ENDIAN

#   define be16toh(x) PP_NTOHS(x)
#   define htobe16(x) PP_HTONS(x)
#   define htole16(x) (x)
#   define le16toh(x) (x)

#   define be32toh(x) PP_NTOHL(x)
#   define htobe32(x) PP_HTONL(x)
#   define htole32(x) (x)
#   define le32toh(x) (x)

#   define htobe64(x) be64toh(x)
#   define htole64(x) (x)
#   define le64toh(x) (x)

#elif defined(PS2_EE_PLATFORM)

#       ifndef _LITTLE_ENDIAN
#       define _LITTLE_ENDIAN LITTLE_ENDIAN
#       endif
#	include <machine/endian.h>
#	include <tcpip.h>

#   define be16toh(x) PP_NTOHS(x)
#   define htobe16(x) PP_HTONS(x)
#   define htole16(x) (x)
#   define le16toh(x) (x)

#   define be32toh(x) PP_NTOHL(x)
#   define htobe32(x) PP_HTONL(x)
#   define htole32(x) (x)
#   define le32toh(x) (x)

#   define htobe64(x) be64toh(x)
#   define htole64(x) (x)
#   define le64toh(x) (x)

#elif defined(DC_KOS_PLATFORM)

#	include <machine/endian.h>

#   define be16toh(x) __builtin_bswap16(x)
#   define htobe16(x) __builtin_bswap16(x)
#   define htole16(x) (x)
#   define le16toh(x) (x)

#   define be32toh(x) __builtin_bswap32(x)
#   define htobe32(x) __builtin_bswap32(x)
#   define htole32(x) (x)
#   define le32toh(x) (x)

#   define be64toh(x) __builtin_bswap64(x)
#   define htobe64(x) __builtin_bswap64(x)
#   define htole64(x) (x)
#   define le64toh(x) (x)

#elif defined(__linux__) || defined(__CYGWIN__)

#	include <endian.h>

#elif defined(__APPLE__)

#	include <libkern/OSByteOrder.h>

#	define htobe16(x) OSSwapHostToBigInt16(x)
#	define htole16(x) OSSwapHostToLittleInt16(x)
#	define be16toh(x) OSSwapBigToHostInt16(x)
#	define le16toh(x) OSSwapLittleToHostInt16(x)

#	define htobe32(x) OSSwapHostToBigInt32(x)
#	define htole32(x) OSSwapHostToLittleInt32(x)
#	define be32toh(x) OSSwapBigToHostInt32(x)
#	define le32toh(x) OSSwapLittleToHostInt32(x)

#	define htobe64(x) OSSwapHostToBigInt64(x)
#	define htole64(x) OSSwapHostToLittleInt64(x)
#	define be64toh(x) OSSwapBigToHostInt64(x)
#	define le64toh(x) OSSwapLittleToHostInt64(x)

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#elif defined(__OpenBSD__)

#	include <sys/endian.h>

#elif defined(PS4_PLATFORM)

#	include <endian.h>

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)

#	include <sys/endian.h>

#	define be16toh(x) betoh16(x)
#	define le16toh(x) letoh16(x)

#	define be32toh(x) betoh32(x)
#	define le32toh(x) letoh32(x)

#	define be64toh(x) betoh64(x)
#	define le64toh(x) letoh64(x)

#elif defined(PS3_PPU_PLATFORM)

#   define htobe16(x) (x)
#   define htole16(x) __builtin_bswap16(x)
#   define be16toh(x) (x)
#   define le16toh(x) __builtin_bswap16(x)

#   define htobe32(x) (x)
#   define htole32(x) __builtin_bswap32(x)
#   define be32toh(x) (x)
#   define le32toh(x) __builtin_bswap32(x)

#   define htobe64(x) (x)
#   define htole64(x) __builtin_bswap64(x)
#   define be64toh(x) (x)
#   define le64toh(x) __builtin_bswap64(x)

#elif defined(__WINDOWS__) 

# include <windows.h>

# if defined(_MSC_VER)
#   include <stdlib.h>

#   define htobe16(x) _byteswap_ushort(x)
#   define htole16(x) (x)
#   define be16toh(x) _byteswap_ushort(x)
#   define le16toh(x) (x)

#   define htobe32(x) _byteswap_ulong(x)
#   define htole32(x) (x)
#   define be32toh(x) _byteswap_ulong(x)
#   define le32toh(x) (x)

#   define htobe64(x) _byteswap_uint64(x)
#   define htole64(x) (x)
#   define be64toh(x) _byteswap_uint64(x)
#   define le64toh(x) (x)
# elif defined(__GNUC__) || defined(__clang__)

#   define htobe16(x) __builtin_bswap16(x)
#   define htole16(x) (x)
#   define be16toh(x) __builtin_bswap16(x)
#   define le16toh(x) (x)

#   define htobe32(x) __builtin_bswap32(x)
#   define htole32(x) (x)
#   define be32toh(x) __builtin_bswap32(x)
#   define le32toh(x) (x)

#   define htobe64(x) __builtin_bswap64(x)
#   define htole64(x) (x)
#   define be64toh(x) __builtin_bswap64(x)
#   define le64toh(x) (x)

#   else
#     error platform not supported
#   endif

#elif defined(__amigaos4__) || defined(__AMIGA__)

# if defined(__NEWLIB__)
#   include <machine/endian.h>

#   define htobe16(x) (x)
#   define htole16(x) __bswap16(x)
#   define be16toh(x) (x)
#   define le16toh(x) __bswap16(x)

#   define htobe32(x) (x)
#   define htole32(x) __bswap32(x)
#   define be32toh(x) (x)
#   define le32toh(x) __bswap32(x)

#   define htobe64(x) (x)
#   define htole64(x) __bswap64(x)
#   define be64toh(x) (x)
#   define le64toh(x) __bswap64(x)

# elif defined(__GNUC__)

#   define htobe16(x) (x)
#   define htole16(x) __builtin_bswap16(x)
#   define be16toh(x) (x)
#   define le16toh(x) __builtin_bswap16(x)

#   define htobe32(x) (x)
#   define htole32(x) __builtin_bswap32(x)
#   define be32toh(x) (x)
#   define le32toh(x) __builtin_bswap32(x)

#   define htobe64(x) (x)
#   define htole64(x) __builtin_bswap64(x)
#   define be64toh(x) (x)
#   define le64toh(x) __builtin_bswap64(x)

# else
#   error platform not supported
# endif

#elif defined(__AROS__)

# include <endian.h>

# if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR >= 8))

#   define __bswap16(x) __builtin_bswap16(x)
#   define __bswap32(x) __builtin_bswap32(x)
#   define __bswap64(x) __builtin_bswap64(x)

# else

#   define __bswap16(x) ((((uint16_t)(x) & 0xFF00) >> 8) | \
                         (((uint16_t)(x) & 0x00FF) << 8))
#   define __bswap32(x) ((((uint32_t)(x) & 0xFF000000) >> 24) | \
                         (((uint32_t)(x) & 0x00FF0000) >>  8) | \
                         (((uint32_t)(x) & 0x0000FF00) <<  8) | \
                         (((uint32_t)(x) & 0x000000FF) << 24))
#   define __bswap64(x) ((((uint64_t)(x) & 0xFF00000000000000) >> 56) | \
                         (((uint64_t)(x) & 0x00FF000000000000) >> 40) | \
                         (((uint64_t)(x) & 0x0000FF0000000000) >> 24) | \
                         (((uint64_t)(x) & 0x000000FF00000000) >>  8) | \
                         (((uint64_t)(x) & 0x00000000FF000000) <<  8) | \
                         (((uint64_t)(x) & 0x0000000000FF0000) << 24) | \
                         (((uint64_t)(x) & 0x000000000000FF00) << 40) | \
                         (((uint64_t)(x) & 0x00000000000000FF) << 56))

# endif

# if _BYTE_ORDER == _LITTLE_ENDIAN

#   define htobe16(x) __bswap16(x)
#   define htole16(x) (x)
#   define be16toh(x) __bswap16(x)
#   define le16toh(x) (x)

#   define htobe32(x) __bswap32(x)
#   define htole32(x) (x)
#   define be32toh(x) __bswap32(x)
#   define le32toh(x) (x)

#   define htobe64(x) __bswap64(x)
#   define htole64(x) (x)
#   define be64toh(x) __bswap64(x)
#   define le64toh(x) (x)

# else

#   define htobe16(x) (x)
#   define htole16(x) __bswap16(x)
#   define be16toh(x) (x)
#   define le16toh(x) __bswap16(x)

#   define htobe32(x) (x)
#   define htole32(x) __bswap32(x)
#   define be32toh(x) (x)
#   define le32toh(x) __bswap32(x)

#   define htobe64(x) (x)
#   define htole64(x) __bswap64(x)
#   define be64toh(x) (x)
#   define le64toh(x) __bswap64(x)

# endif

#elif defined(XBOX_360_PLATFORM)
#   include <xtl.h>

#   include <stdlib.h>

#   define htobe16(x) (x)
#   define htole16(x) _byteswap_ushort(x)
#   define be16toh(x) (x)
#   define le16toh(x) _byteswap_ushort(x)

#   define htobe32(x) (x)
#   define htole32(x) _byteswap_ulong(x)
#   define be32toh(x) (x)
#   define le32toh(x) _byteswap_ulong(x)

#   define htobe64(x) (x)
#   define htole64(x) _byteswap_uint64(x)
#   define be64toh(x) (x)
#   define le64toh(x) _byteswap_uint64(x)

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#elif defined(XBOX_PLATFORM)
#   include <xtl.h>

#   include <stdlib.h>

#   define htobe16(x) (x)
#   define htole16(x) _byteswap_ushort(x)
#   define be16toh(x) (x)
#   define le16toh(x) _byteswap_ushort(x)

#   define htobe32(x) (x)
#   define htole32(x) _byteswap_ulong(x)
#   define be32toh(x) (x)
#   define le32toh(x) _byteswap_ulong(x)

#   define htobe64(x) (x)
#   define htole64(x) _byteswap_uint64(x)
#   define be64toh(x) (x)
#   define le64toh(x) _byteswap_uint64(x)

/* #	define __BYTE_ORDER    BYTE_ORDER */
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
/* #	define __PDP_ENDIAN    PDP_ENDIAN */

# elif defined(__GNUC__) || defined(__clang__)

#   define htobe16(x) __builtin_bswap16(x)
#   define htole16(x) (x)
#   define be16toh(x) __builtin_bswap16(x)
#   define le16toh(x) (x)

#   define htobe32(x) __builtin_bswap32(x)
#   define htole32(x) (x)
#   define be32toh(x) __builtin_bswap32(x)
#   define le32toh(x) (x)

#   define htobe64(x) __builtin_bswap64(x)
#   define htole64(x) (x)
#   define be64toh(x) __builtin_bswap64(x)
#   define le64toh(x) (x)

#else
#  error platform not supported
#endif

#endif
