/*
 * ps2_fio.h - Lame header.
 *
 * Copyright (C) 2003 Tord Lindstrom (pukko@home.se)
 * Copyright (C) 2004 adresd <adresd_ps2dev@yahoo.com>
 *
 * Licensed under the AFL v2.0. See the file LICENSE included with this
 * distribution for licensing terms.
 */

#ifndef BYTEORDER_H
#define BYTEORDER_H

#ifdef BIG_ENDIAN
inline unsigned int   ntohl(x) { return x; }
inline unsigned short ntohs(x) { return x; }
#else
// LITTLE_ENDIAN
#ifndef htonl
inline unsigned int
htonl(unsigned int x)
{
    return ((x & 0xff) << 24 ) |
        ((x & 0xff00) << 8 ) |
        ((x & 0xff0000) >> 8 ) |
        ((x & 0xff000000) >> 24 );
}
#endif

#ifndef htons
inline unsigned short 
htons(unsigned short x)
{
    return ((x & 0xff) << 8 ) | ((x & 0xff00) >> 8 );
}
#endif

#endif

#endif /* BYTEORDER_H */
