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
# PS2 DNS resolver
*/
 
#ifndef _DNS_H
#define	_DNS_H

#include "types.h"
#include "irx.h"

#include "ps2ip.h"


#define DNS_ERROR_HOST_NOT_FOUND	-1
#define	DNS_ERROR_CONNECT			-2
#define	DNS_ERROR_PARSE				-3


#define dns_IMPORTS_start	DECLARE_IMPORT_TABLE(dns, 1, 0)
#define dns_IMPORTS_end		END_IMPORT_TABLE

int gethostbyname(char *name, struct in_addr *ip);
#define I_gethostbyname DECLARE_IMPORT(4, gethostbyname)
void dnsSetNameserverAddress(struct in_addr *addr);
#define I_dnsSetNameserverAddress DECLARE_IMPORT(5, dnsSetNameserverAddress)


#endif
