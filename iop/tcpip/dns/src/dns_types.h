/*
 * dns_types.h - PS2 DNS resolver
 *
 * Copyright (c) 2004 Nicholas Van Veen <nickvv@xtra.co.nz>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef _DNS_TYPES_H
#define _DNS_TYPES_H

typedef struct
{
	u16 id;
	u16 flags;
	u16 QDCOUNT;
	u16 ANCOUNT;
	u16 NSCOUNT;
	u16 ARCOUNT;
} t_dnsMessageHeader;

typedef struct
{
	u16 type;
	u16 class;
	u32 ttl;
	u16 rdlength;
	u8 rdata[4]; // always 4 for A type RR's
} t_dnsResourceRecordHostAddr __attribute__ ((packed));

#define RCODE_noError			0
#define RCODE_formatError		1
#define	RCODE_serverFailure		2
#define	RCODE_nameError			3
#define	RCODE_notImplemented	4
#define RCODE_refused			5

#define	TYPE_A					1	// host address
#define	TYPE_PTR				12	// domain name pointer

#define CLASS_IN				1	// internet class

#define	NAMESERVER_PORT			53

#endif
