/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Lame header.
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
