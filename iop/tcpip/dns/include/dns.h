/*
 * dns.h - PS2 DNS resolver
 *
 * Copyright (c) 2004 Nicholas Van Veen <nickvv@xtra.co.nz>
 *
 * See the file LICENSE included with this distribution for licensing terms.
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
