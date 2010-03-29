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
# HTTP CLIENT FILE SYSTEM DRIVER.
*/

/*

  The HTTP file io driver is a read only driver that slots into the PS2
  IO subsystem and provides access to HTTP.

  For each open request a file handle is allocated and any read request
  is directed to the socket.  After close has been called the file
  handle slot is free'd for the next request.

  No header information normally returned from a HTTP request is returned.
  The client must know the content of the data stream and how to deal with it.

  lseek is currently only supported in order to get the size of a file. With
  the current implimentation, it is not possible to seek to different positions
  in a file.

  With the current implimentation, hostnames may only be specified as an IP
  address. This will change once we get a DNS resolver up & running with lwip.
*/

#include <types.h>
#include <irx.h>
#include <stdio.h>
#include <thbase.h>
#include <sysclib.h>
#include <ioman_mod.h>
#include <sysmem.h>

#include "ps2ip.h"
#include "dns.h"

//#define DEBUG

#ifdef DEBUG
#define DBG_printf      printf
#else
#define DBG_printf(args...) do { } while(0)
#endif

typedef struct
{
	int sockFd;
	int fileSize;
	int filePos;
} t_fioPrivData;


// example of basic HTTP 1.0 protocol request.
char strTest[] = "GET /blah HTTP/1.0\n\n";

char HTTPGET[] = "GET ";
char HTTPHOST[] = "Host: ";
char HTTPGETEND[] = " HTTP/1.0\r\n";
char HTTPUSERAGENT[] = "User-Agent: PS2IP HTTP Client\r\n";
char HTTPENDHEADER[] = "\r\n";

//
// This function will parse the Content-Length header line and return the file size
//
int parseContentLength(char *mimeBuffer)
{
	char *line;

	line = strstr(mimeBuffer, "CONTENT-LENGTH:");
	line += strlen("CONTENT-LENGTH:");

	// Advance past any whitepace characters
	while((*line == ' ') || (*line == '\t')) line++;

	return (int)strtol(line,NULL, 10);
}

//
// This function will parse the initial response header line and return 0 for a "200 OK",
// or return the error code in the event of an error (such as 404 - not found)
//
int isErrorHeader(char *mimeBuffer)
{
	char *line;
	int i;
	int code;

	line = strstr(mimeBuffer, "HTTP/1.");
	line += strlen("HTTP/1.");

	// Advance past minor protocol version number
	line++;

	// Advance past any whitespace characters
	while((*line == ' ') || (*line == '\t')) line++;

	// Terminate string after status code
	for(i = 0; ((line[i] != ' ') && (line[i] != '\t')); i++);
	line[i] = '\0';

	code = (int)strtol(line,NULL, 10);
	if( code == 200 )
		return 0;
	else
		return code;
}

//
// When a request has been sent, we can expect mime headers to be
// before the data.  We need to read exactly to the end of the headers
// and no more data.  This readline reads a single char at a time.
//
int readLine( int socket, char * buffer, int size )
{
	char * ptr = buffer;
	int count = 0;
	int rc;

	// Keep reading until we fill the buffer.

	while ( count < size )
	{
		rc = recv( socket, ptr, 1, 0 );

		if ( rc <= 0 ) return rc;

		if ( (*ptr == '\n') ) break;

		// increment after check for cr.  Don't want to count the cr.
		count++;
		ptr++;
	}

	// Terminate string
	*ptr = '\0';

	// return how many bytes read.
	return count;
}


//
// This is the main HTTP client connect work.  Makes the connection
// and handles the protocol and reads the return headers.  Needs
// to leave the stream at the start of the real data.
//
int httpConnect( struct sockaddr_in * server, char *hostAddr, const char * url, t_fioPrivData *pHandle )
{
	int sockHandle;
	int peerHandle;
	int rc;
	char mimeBuffer[100];

#ifdef DEBUG
	printf( "create socket\n" );
#endif

	if((sockHandle = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP )) < 0)
	{
		printf( "HTTP: SOCKET FAILED\n" );
		return -1;
	}

	DBG_printf( "connect\n" );

	rc = connect( sockHandle, (struct sockaddr *) server, sizeof(*server));
	if ( rc < 0 )
	{
		printf( "HTTP: CONNECT FAILED %i\n", sockHandle );
		return -1;
	}
	peerHandle = sockHandle;

	DBG_printf( "send\n" );

	// Needs more error checking here.....
	rc = send( peerHandle, HTTPGET,  sizeof( HTTPGET ) - 1, 0 );
	rc = send( peerHandle, (void*) url, strlen( url ), 0 );
	rc = send( peerHandle, HTTPGETEND, sizeof( HTTPGETEND ) - 1, 0 );

	rc = send( peerHandle, HTTPHOST,  sizeof( HTTPHOST ) - 1, 0 );
	rc = send( peerHandle, hostAddr, strlen( url ), 0 );
	rc = send( peerHandle, HTTPENDHEADER, sizeof( HTTPENDHEADER ) - 1, 0 ); // "\r\n"

	rc = send( peerHandle, HTTPUSERAGENT, sizeof( HTTPUSERAGENT ) - 1, 0 );
	rc = send( peerHandle, HTTPENDHEADER, sizeof( HTTPENDHEADER ) - 1, 0 );

	// We now need to read the header information
	while ( 1 )
	{
		int i;

		// read a line from the header information.
		rc = readLine( peerHandle, mimeBuffer, 100 );

		DBG_printf(">> %s", mimeBuffer);

		if ( rc < 0 ) return rc;

		// End of headers is a blank line.  exit.
		if ( rc == 0 ) break;
		if ( (rc == 1) && (mimeBuffer[0] == '\r') ) break;

		// Convert mimeBuffer to upper case, so we can do string comps
		for(i = 0; i < strlen(mimeBuffer); i++)
			mimeBuffer[i] = toupper(mimeBuffer[i]);

		if(strstr(mimeBuffer, "HTTP/1.")) // First line of header, contains status code. Check for an error code
			if((rc = isErrorHeader(mimeBuffer))) {
				printf("HTTP: status code = %d!\n", rc);
				return -rc;
			}

		if(strstr(mimeBuffer, "CONTENT-LENGTH:"))
		{
			pHandle->fileSize = parseContentLength(mimeBuffer);
			DBG_printf("fileSize = %d\n", pHandle->fileSize);
		}
	}

	// We've sent the request, and read the headers.  SockHandle is
	// now at the start of the main data read for a file io read.
	return peerHandle;
}

char *strnchr(char *str, char ch, int max) {
    int i;

    for(i = 0; (i < max) && (str[i] != '\0'); i++)
        if(str[i] == ch)
            return(&str[i]);

    return(NULL);
}

//
// Before we can connect we need to parse the server address and optional
// port from the url provided.  the format of "url" as passed to this function
// is "//192.168.0.1:8080/blah.elf" where "192.168.0.1" can be either an IP
// or a domain name and ":8080" is the optional port to connect to, default
// port is 80.
//
// This function will return a filename string for use in GET
// requests, and fill the structure pointed to by *server with the
// correct values.
//
const char *resolveAddress( struct sockaddr_in *server, const char * url, char *hostAddr )
{
	unsigned char w,x,y,z;
	const char *char_ptr;
	char addr[128];
	char port[6] = "80"; // default port of 80(HTTP)
	int i = 0, rv;
	int isDomain = 0;

	// eg url= //192.168.0.1/fred.elf  (the http: is already stripped)

	// NOTE: Need more error checking in parsing code

	// URL must start with double forward slashes.
	while(url[0] == '/') {
		url++;
	}

	for(i = 0; ((url[i] != '\0') && (url[i] != '/')) && (i < 127); i++)
	    if((((addr[i] = url[i]) < '0') || (url[i] > '9')) && (url[i] != '.')) {

	        if(url[i] == ':') {// allow specification of port in URL like http://www.server.net:8080/
	            for(w = 0; ((w + i + 1) < 127) && (w < 5) && (url[w + i + 1] != '/') && (url[w + i + 1] != '\0'); w++)
	                port[w] = url[w + i + 1];
	            port[w] = '\0';

	            DBG_printf("HTTP: using port %s for connection\n", port);
	            break;
	            }
            else // it's a domain name if a non-numeric char is contained in the "server" part of the URL.
	            isDomain = 1;
	        }
    addr[i] = '\0'; // overwrite last char copied(should be '/', '\0' or ':') with a '\0'
    strcpy(hostAddr, addr);

    if(isDomain) {
		// resolve the host name.
		rv = gethostbyname(addr, &server->sin_addr);
		if(rv != 0) {
		    printf("HTTP: failed to resolve domain '%s'\n", addr);
			return NULL;
			}
        }
    else {
		// turn '.' characters in ip string into null characters
		for(i = 0, w = 0; i < 16; i++)
			if(addr[i] == '.') { addr[i] = '\0'; w++; }

        if(w != 3) { // w is used as a simple error check here
            printf("HTTP: invalid IP address '%s'\n", hostAddr);
            return(NULL);
            }

		i = 0;

		// Extract individual ip number octets from string
		w = (int)strtol(&addr[i],NULL, 10);
		i += (strlen(&addr[i]) + 1);

		x = (int)strtol(&addr[i],NULL, 10);
		i += (strlen(&addr[i]) + 1);

		y = (int)strtol(&addr[i],NULL, 10);
		i += (strlen(&addr[i]) + 1);

		z = (int)strtol(&addr[i],NULL, 10);
		i += (strlen(&addr[i]) + 1);

		IP4_ADDR( (struct ip_addr *)&(server->sin_addr) ,w,x,y,z );
	}

    i = (int) strtol(port, NULL, 10); // set the port
	server->sin_port = htons(i);

	server->sin_family = AF_INET;

#if 1
	char_ptr = url;
	while(*char_ptr != '/') char_ptr++;

	return char_ptr;
#else
	return url;
#endif
}

//
// Any calls we don't implement calls dummy.
//
int httpDummy()
{
	printf("PS2HTTP: dummy function called\n");
	return -5;
}

int httpInitialize(iop_io_device_t *driver)
{
	printf("PS2HTTP: filesystem driver initialized\n");

	return 0;
}

//
// Open has the most work to do in the file driver.  It must:
//
//  1. Find a free file Handle.
//  2. Check we have a valid IP address and URL.
//  3. Try and connect to the remote server.
//  4. Send a GET request to the server
//  5. Parse the GET response header from the server
//
int httpOpen(iop_io_file_t *f, const char *name, int mode)
{
	int peerHandle = 0;
	struct sockaddr_in server;
	const char *getName;
	t_fioPrivData *privData;
	char hostAddr[100];

#ifdef DEBUG
	printf("httpOpen(-, %s, %d)\n", name, mode);
#endif

	if((privData = AllocSysMemory(ALLOC_FIRST, sizeof(t_fioPrivData), NULL)) == NULL)
		return -1;

	f->privdata = privData;

	privData->fileSize = 0;
	privData->filePos = 0;

	memset(&server, 0, sizeof(server));
	// Check valid IP address and URL
	if((getName = resolveAddress( &server, name, hostAddr )) == NULL)
	{
		FreeSysMemory(privData);
		return -2;
	}

	// Now we connect and initiate the transfer by sending a
	// request header to the server, and receiving the response header
	if((peerHandle = httpConnect( &server, hostAddr, getName, privData )) < 0)
	{
        printf("HTTP: failed to connect to '%s'!\n", hostAddr);
		FreeSysMemory(privData);
		return peerHandle;
	}

	// http connect returns valid socket.  Save in handle list.
	privData->sockFd = peerHandle;

	// return success.  We got it all ready. :)
	return 0;
}


//
// Read is simple.  It simply needs to find the socket no
// based on the file handle and call recv.
//
int httpRead(iop_io_file_t *f, void *buffer, int size)
{
	int bytesRead = 0;
	t_fioPrivData *privData = (t_fioPrivData *)f->privdata;
	int left = size;
	int totalRead = 0;

#ifdef DEBUG
	printf("httpRead(-, 0x%X, %d)\n", (int)buffer, size);
#endif

	// Read until: there is an error, we've read "size" bytes or the remote
	//             side has closed the connection.
	do {

		bytesRead = recv( privData->sockFd, buffer + totalRead, left, 0 );

#ifdef DEBUG
//		printf("bytesRead = %d\n", bytesRead);
#endif

		if(bytesRead <= 0) break;

		left -= bytesRead;
		totalRead += bytesRead;

	} while(left);

	return totalRead;
}


//
// Close finds the correct handle and
// calls disconnect.
//
int httpClose(iop_io_file_t *f)
{
	t_fioPrivData *privData = (t_fioPrivData *)f->privdata;

#ifdef DEBUG
	printf("httpClose(-)\n");
#endif

	lwip_close(privData->sockFd);
	FreeSysMemory(privData);

	return 0;
}

//
// lseek is only supported in order to get the size of a file. Allough the function
// does modify the filePos member, this does not have any effect on the read position
// of the file in the current implimentation.
//
int httpLseek(iop_io_file_t *f, unsigned long offset, int mode)
{
	t_fioPrivData *privData = (t_fioPrivData *)f->privdata;

#ifdef DEBUG
	printf("httpLseek(-, %d, %d)\n", (int)offset, mode);
#endif

	switch(mode)
	{
		case SEEK_SET:
			privData->filePos = offset;
			break;

		case SEEK_CUR:
			privData->filePos += offset;
			break;

		case SEEK_END:
			privData->filePos = privData->fileSize + offset;
			break;

		default:
			return -1;
	}

	return privData->filePos;
}


iop_io_device_ops_t ps2httpOps = {
	httpInitialize, httpDummy, httpDummy, httpOpen, httpClose, httpRead, httpDummy, httpLseek,
	httpDummy, httpDummy, httpDummy, httpDummy, httpDummy, httpDummy, httpDummy, httpDummy,
	httpDummy
};

iop_io_device_t ps2httpDev = {
	"http",
	IOP_DT_FS,
	1,
	"HTTP client file driver",
	&ps2httpOps
};


//
// Main..  registers the File driver.
//
int _start( int argc, char **argv)
{
	printf("PS2HTTP: Module Loaded\n");

	printf("PS2HTTP: Adding 'http' driver into io system\n");
	io_DelDrv( "http");
	io_AddDrv(&ps2httpDev);

	return 0;
}

