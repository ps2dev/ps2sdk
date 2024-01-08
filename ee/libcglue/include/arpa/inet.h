/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2SDK_ARPA_INET_H__
#define __PS2SDK_ARPA_INET_H__

#include <sys/cdefs.h>
#include <tcpip.h>

#ifndef LIBCGLUE_ARPA_INET_NO_ALIASES
#define inet_addr(cp) libcglue_inet_addr(cp)
#define inet_ntoa(addr) libcglue_inet_ntoa((const ip4_addr_t*)&(addr))
#define inet_ntoa_r(addr, buf, buflen) libcglue_inet_ntoa_r((const ip4_addr_t*)&(addr), buf, buflen)
#define inet_aton(cp, addr) libcglue_inet_aton(cp, (ip4_addr_t*)addr)
#endif

__BEGIN_DECLS
u32 libcglue_inet_addr(const char *cp);
char *libcglue_inet_ntoa(const ip4_addr_t *addr);
char *libcglue_inet_ntoa_r(const ip4_addr_t *addr, char *buf, int buflen);
int libcglue_inet_aton(const char *cp, ip4_addr_t *addr);
__END_DECLS

#endif
