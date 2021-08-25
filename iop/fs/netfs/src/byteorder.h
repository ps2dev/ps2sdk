/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Lame header.
 */

#ifndef BYTEORDER_H
#define BYTEORDER_H

#include <stdint.h>

#ifdef BIG_ENDIAN
inline uint64_t ntohll(x) { return x; }
inline uint32_t ntohl(x) { return x; }
inline uint16_t ntohs(x) { return x; }
#else
// LITTLE_ENDIAN
#ifndef htonll
inline uint64_t
htonll(uint64_t x)
{
    return ((x & 0x00000000000000ffull) << 56) |
           ((x & 0x000000000000ff00ull) << 40) |
           ((x & 0x0000000000ff0000ull) << 24) |
           ((x & 0x00000000ff000000ull) << 8) |
           ((x & 0x000000ff00000000ull) >> 8) |
           ((x & 0x0000ff0000000000ull) >> 24) |
           ((x & 0x00ff000000000000ull) >> 40) |
           ((x & 0xff00000000000000ull) >> 56);
}
#endif

#ifndef htonl
inline uint32_t
htonl(uint32_t x)
{
    return ((x & 0x000000ff) << 24) |
           ((x & 0x0000ff00) << 8) |
           ((x & 0x00ff0000) >> 8) |
           ((x & 0xff000000) >> 24);
}
#endif

#ifndef htons
inline uint16_t
htons(uint16_t x)
{
    return ((x & 0x00ff) << 8) |
           ((x & 0xff00) >> 8);
}
#endif

#endif

#endif /* BYTEORDER_H */
