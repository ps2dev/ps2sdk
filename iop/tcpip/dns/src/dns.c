/*
 * dns.c - PS2 DNS resolver
 *
 * Copyright (c) 2004 Nicholas Van Veen <nickvv@xtra.co.nz>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "types.h"
#include "stdio.h"
#include "loadcore.h"
#include "sysclib.h"
#include "sysmem.h"

#include "ps2ip.h"
#include "dns.h"
#include "dns_types.h"


//#define DEBUG


IRX_ID("DNS Resolver", 1, 0);
extern struct irx_export_table	_exp_dns;


typedef struct t_dns_cache
{
	struct t_dns_cache *next;
	char *hostName;
	struct in_addr ipAddr;
	int rv;
} t_dnsCache;


t_dnsCache cacheHead;
struct sockaddr_in nameServerAddr;
u8 packetBuffer[2048]; // should be plenty!
int currentId = 1;


void dnsResolveInit();
int dnsPrepareQueryPacket(u8 *packetBuf, char *hostName); // returns size of packet
int dnsParseResponsePacket(u8 *packetBuf, int pktSize, struct in_addr *ip);
void dnsCacheAdd(t_dnsCache *entry);
t_dnsCache *dnsCacheFind(char *hostName);
int getLabelLength(u8 *buffer);
int getResourceRecordLength(u8 *buffer);

static void *malloc(int size)
{
	return AllocSysMemory(ALLOC_FIRST, size, NULL);
}

static void free(void *ptr)
{
	FreeSysMemory(ptr);
}

static int strcasecmp(char *s1, char *s2)
{
	while (*s1 != '\0' && tolower(*s1) == tolower(*s2))
    {
      s1++;
      s2++;
    }

	return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}

int _start(int argc, char *argv[])
{
	cacheHead.next = NULL;
	cacheHead.hostName = "";
	cacheHead.ipAddr.s_addr = 0;
	cacheHead.rv = 0;

	memset(&nameServerAddr, 0, sizeof(struct sockaddr_in));
	nameServerAddr.sin_family = AF_INET;
	nameServerAddr.sin_port = htons(NAMESERVER_PORT);

	if(argc == 2)
	{
		printf("DNS: Setting nameserver IP to: %s\n", argv[1]);
		nameServerAddr.sin_addr.s_addr = inet_addr(argv[1]);
	}

	RegisterLibraryEntries(&_exp_dns);

	printf("DNS: Module started.\n");

	return MODULE_RESIDENT_END;
}

void dnsSetNameserverAddress(struct in_addr *addr)
{
	memcpy(&nameServerAddr.sin_addr, addr, sizeof(struct in_addr));
}

int gethostbyname(char *name, struct in_addr *ip)
{
	t_dnsCache *cached = dnsCacheFind(name);
	int pktSize, sockFd, pos = 0, rv;
	t_dnsCache *newNode;

	memset(ip, 0, sizeof(struct in_addr));

	// Is the host already in the cache?
	if(cached != NULL)
	{
#ifdef DEBUG
		printf("host (%s) found in cache!\n", name);
#endif
		ip->s_addr = cached->ipAddr.s_addr;
		return cached->rv;
	}

	// prepare the query packet
	pktSize = dnsPrepareQueryPacket(&packetBuffer[2], name);

	// connect to dns server via TCP (UDP is standard, but TCP is easier :))
	sockFd = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd < 0)
		return DNS_ERROR_CONNECT;

	if(lwip_connect(sockFd, (struct sockaddr *)&nameServerAddr, sizeof(struct sockaddr)) < 0)
	{
		lwip_close(sockFd);
		return DNS_ERROR_CONNECT;
	}

#ifdef DEBUG
	printf("connected to nameserver!\n");
#endif

	// Send our packet - first send packet size
	*(u16 *)packetBuffer = htons(pktSize);
	pktSize += 2;

	while(pktSize)
	{
		int sent = lwip_send(sockFd, &packetBuffer[pos], pktSize, 0);
		
		if(sent < 0)
		{
			lwip_close(sockFd);
			return DNS_ERROR_CONNECT;
		}

		pos += sent;
		pktSize -= sent;
	}
	
#ifdef DEBUG
	printf("packet sent!\n");
#endif

	rv = lwip_recv(sockFd, &packetBuffer[pktSize], sizeof(packetBuffer), 0);
	if(rv < 0)
	{
			lwip_close(sockFd);
			return DNS_ERROR_CONNECT;
	}

	pktSize = ntohs(*(u16 *)packetBuffer);

	// should do proper loop to get all data if the entire packet didnt make it
	// through the first time.. maybe later :)
	lwip_close(sockFd);
	if(pktSize != (rv - 2))
			return DNS_ERROR_CONNECT;

#ifdef DEBUG
	printf("received response! len = %d\n", pktSize);
#endif

	rv = dnsParseResponsePacket(&packetBuffer[2], pktSize, ip);
#ifdef DEBUG
	{
		u8 *ptr = (u8 *)ip;
		printf("resolved ip: %d.%d.%d.%d\n", ptr[0], ptr[1], ptr[2], ptr[3]);
	}
#endif

	newNode = (t_dnsCache *)malloc(sizeof(t_dnsCache));
	if(!newNode)
		return rv;

	newNode->ipAddr.s_addr = ip->s_addr;
	newNode->rv = rv;
	newNode->hostName = malloc(strlen(name) + 1);
	if(!newNode->hostName)
	{
		free(newNode);
		return rv;
	}

	strcpy(newNode->hostName, name);
	dnsCacheAdd(newNode);

	return rv;
}

int dnsPrepareQueryPacket(u8 *packetBuf, char *hostName)
{
	t_dnsMessageHeader *hdr = (t_dnsMessageHeader *)packetBuf;
	int currLen, pktSize = 0, left = strlen(hostName), i;

	memset(hdr, 0, sizeof(t_dnsMessageHeader));
	hdr->id = currentId++;
	hdr->flags = 0x100; // standard query, recursion desired
	hdr->QDCOUNT = 1;

	// convert header to network byte order
	for(i = 0; i < sizeof(t_dnsMessageHeader); i += 2)
		*(u16 *)&packetBuf[i] = htons(*(u16 *)&packetBuf[i]);

	pktSize += sizeof(t_dnsMessageHeader);
	packetBuf += sizeof(t_dnsMessageHeader);

	// Copy over QNAME
	while(left > 0)
	{
		for(currLen = 0; (hostName[currLen] != '.') && (hostName[currLen] != '\0'); currLen++);
		*(packetBuf++) = currLen;
		memcpy(packetBuf, hostName, currLen);

		hostName += currLen + 1;
		packetBuf += currLen;
		pktSize += currLen + 1;
		left -= currLen + 1;
	}

	// terminate QNAME
	*(packetBuf++) = '\0';
	pktSize++;

	// Set type = TYPE_A, class = CLASS_IN (the cheat way :P)
	*(packetBuf++) = 0;
	*(packetBuf++) = 1;
	*(packetBuf++) = 0;
	*(packetBuf++) = 1;
	pktSize += 4;

	return pktSize;
}

int getLabelLength(u8 *buffer)
{
	int len = 0;

	// if "compressed" label, simply skip 2 bytes
	if(buffer[len] == 0xC0)
		len += 2;
	else
	{
		// otherwise skip past null terminated string
		while(buffer[len] != '\0')
			len++;

		len++;
	}

	return len;
}

int getResourceRecordLength(u8 *buffer)
{
	int len = 0;
	u32 rdLen;

	len += getLabelLength(buffer);
	// skip to RDLENGTH
	len += 8;
	memcpy(&rdLen, &buffer[len], 2);
	rdLen = ntohs(rdLen);
	len += 2 + rdLen;
		
	return len;
}

int dnsParseResponsePacket(u8 *packetBuf, int pktSize, struct in_addr *ip)
{
	t_dnsMessageHeader *hdr = (t_dnsMessageHeader *)packetBuf;
	t_dnsResourceRecordHostAddr rrBody;
	u8 retCode;
	int pos, i;

	// convert header to host byte order and make sure everything is in order
	for(i = 0; i < sizeof(t_dnsMessageHeader); i += 2)
		*(u16 *)&packetBuf[i] = ntohs(*(u16 *)&packetBuf[i]);

	// make sure this is a response
	if(!(hdr->flags & 0x8000))
		return DNS_ERROR_PARSE;

	// make sure message isnt truncated
	if(hdr->flags & 0x200)
		return DNS_ERROR_PARSE;

	retCode = hdr->flags & 0x0F;
	if(retCode == RCODE_nameError)
		return DNS_ERROR_HOST_NOT_FOUND;
	else if(retCode != RCODE_noError)
		return DNS_ERROR_PARSE;

	// skip past original query to resource records
	pos = sizeof(t_dnsMessageHeader);
	while(packetBuf[pos] != '\0')
		pos++;
	pos += 5;

	// go through all resource records looking for host address type
	while(pos < pktSize)
	{
		memcpy(&rrBody, &packetBuf[pos + getLabelLength(&packetBuf[pos])], sizeof(t_dnsResourceRecordHostAddr));

		// keep searching till we find the right resource record (if any)
		if(	(ntohs(rrBody.type) != TYPE_A) ||
			(ntohs(rrBody.class) != CLASS_IN) ||
			(ntohs(rrBody.rdlength) != 4))
		{
			pos += getResourceRecordLength(&packetBuf[pos]);
			continue;
		}

		memcpy(ip, rrBody.rdata, 4);
		return 0;
	}

	return DNS_ERROR_PARSE;
}

void dnsCacheAdd(t_dnsCache *entry)
{
	t_dnsCache *last = &cacheHead;
	
	while(last->next != NULL)
		last = last->next;

	last->next = entry;
	entry->next = NULL;
}

t_dnsCache *dnsCacheFind(char *hostName)
{
	t_dnsCache *current;

	for(current = cacheHead.next; current != NULL; current = current->next)
	{
		if(!strcasecmp(current->hostName, hostName))
			return current;
	}

	return NULL;
}
