/*
  _____     ___ ____
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2002, David Ryan ( oobles@hotmail.com )
                           (C)2002, Nicholas Van Veen (nickvv@xtra.co.nz)
  ------------------------------------------------------------------------
  ps2http.c                HTTP CLIENT FILE SYSTEM DRIVER.


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

#include "types.h"
#include "sysclib.h"
#include "iomanX.h"
#include "stdio.h"

#include "ps2ip.h"

// How many handles do we keep open.
#define HCOUNT 32

typedef struct
{
	int fd;
	int sockFd;
	int fileSize;
	int filePos;
	int used;
} handle;

// These are the list of socket file handles.
static handle handles[ HCOUNT ];

// FileIO structure.
static iop_device_t driver;

// Function array for fileio structure.
static iop_device_ops_t functions;


// example of basic HTTP 1.0 protocol request.
char strTest[] = "GET /blah HTTP/1.0\n\n";

char HTTPGET[] = "GET ";
char HTTPGETEND[] = " HTTP/1.0\r\n";
char HTTPUSERAGENT[] = "User-Agent: PS2IP HTTP Client\r\n";
char HTTPENDHEADER[] = "\r\n";

#define DOMAINHOST_SIZE 200
char HTTPHOST[] = "Host: ";
static char domainHost[ DOMAINHOST_SIZE ];

#define CONTENTLENGTH "Content-Length:"
//
// This function will parse the Content-Length header line and return the file size
//
int parseContentLength(char *mimeBuffer)
{
	char *line;

	line = strstr(mimeBuffer, CONTENTLENGTH );
	line += strlen( CONTENTLENGTH );

	// Advance past any whitepace characters
	while((*line == ' ') || (*line == '\t')) line++;

	//printf( "strtol on %s\n", line );

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

		if ( rc <= 0 ) 
		{
			//printf( "recv failed in readLine %i\n", rc );
			return rc;
		}

		// If its a linefeed ignore it.
		if ( *ptr == '\r' ) continue;

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


#define CONNECT_BUFFER_SIZE 500
//
// This is the main HTTP client connect work.  Makes the connection
// and handles the protocol and reads the return headers.  Needs
// to leave the stream at the start of the real data.
//
int httpConnect( struct sockaddr_in * server, char * url, handle *pHandle )
{
	int sockHandle;
	int peerHandle;
	int rc;
	char mimeBuffer[CONNECT_BUFFER_SIZE];

	sockHandle = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( sockHandle < 0 )
	{
		printf( "SOCKET FAILED\n" );
		return -1;
	}

	peerHandle = connect( sockHandle, (struct sockaddr *) server, sizeof(*server));
	if ( peerHandle < 0 )
	{
		printf( "CONNECT FAILED %i\n", peerHandle );
		return -1;
	}

	strcpy( mimeBuffer, HTTPGET );
	strcat( mimeBuffer, url );
	strcat( mimeBuffer, HTTPGETEND );
	
	strcat( mimeBuffer, domainHost );
	strcat( mimeBuffer, HTTPUSERAGENT );
	strcat( mimeBuffer, HTTPENDHEADER );

	rc = send( peerHandle, mimeBuffer, strlen( mimeBuffer), 0 );
	if ( rc < 0 )
	{
		printf( "failed to send request %i\n", rc );
		disconnect( peerHandle );
		return rc;
	}
	// Needs more error checking here.....

	// We now need to read the header information
	while ( 1 )
	{
		// read a line from the header information.
		rc = readLine( peerHandle, mimeBuffer, CONNECT_BUFFER_SIZE ); 


		//printf( "lineread = '%s'\n", mimeBuffer );

		if ( rc < 0 )
		{
			printf( "readLine failed %d\n", rc );
			disconnect( peerHandle );
			return rc;
		}

		// End of headers is a blank line.  exit. 
		if ( rc == 0 )
		{
			printf( "readLine returned 0\n" );
			break;
		}

		if ( (rc == 1) && (mimeBuffer[0] == '\r') ) break;

	  
		// First line of header, contains status code. 
		// Check for an error code
		if( !strncmp( mimeBuffer, "HTTP/1.", 7 ) ) 
		{
			//printf( "check errorcode %s\n", mimeBuffer );
			if( (rc = isErrorHeader(mimeBuffer) ) ) 
			{
				//printf("status code = %d!\n", rc);
				disconnect( peerHandle );
				return -rc;
			}
		}

		if ( strstr(mimeBuffer, CONTENTLENGTH ) ) 
		{
			//printf( "check content-len: %s\n", mimeBuffer );
			pHandle->fileSize = parseContentLength(mimeBuffer);
		}
	} 

	// We've sent the request, and read the headers.  SockHandle is
	// now at the start of the main data read for a file io read.
	return peerHandle;
}



//
// Before we can connect we need to parse the server address from
// the url provided.  This currently only deals with IP addresses
// eg 192.168.0.1  (we do not yet have dns resolution.  later?)
// This function will return a filename string for use in GET
// requests, and fill the structure pointed to by *server with the
// correct values.
//
char *resolveAddress( struct sockaddr_in *server, char * url )
{
	unsigned char w,x,y,z;
	short port;
	char *char_ptr;
	char ip[128];
	int i = 0;
        unsigned char iptype = 0;

	// eg url= //192.168.0.1/fred.elf  (the http: is already stripped)

	// NOTE: Need more error checking in parsing code

	// URL must start with double forward slashes.
	if ( url[0] != '/' || url[1] != '/' )
	{
	   return NULL;
	}

        // See if we have a domain name or ip address.
        // iptype can only have numbers 0-9 and . in the host.

        iptype = 1;
        char_ptr = url + 2;
        while (*char_ptr != '/' && *char_ptr != 0 )
        {
           if ( ( *char_ptr >= '0' && *char_ptr <= '9' ) || *char_ptr == '.' )
           {
                char_ptr++;
           }
 	   else
	   {
           	iptype = 0;
           	break;
	   }
        }


	// Copy host addy from url into seperate buffer
	char_ptr = url + 2; // advance past initial '/' characters
	while(*char_ptr != '/')
	{
		ip[i] = *char_ptr;
		i++;
		char_ptr++;
	}

        // null terminate the string.
	ip[i]=0;

	if ( iptype == 1 )
	{
		// turn '.' characters in ip string into null characters
		for(i = 0; i < 16; i++)
			if(ip[i] == '.') ip[i] = '\0';

		i = 0;

		// Extract individual ip number octets from string
		w = (int)strtol(&ip[i],NULL, 10);
		i += (strlen(&ip[i]) + 1);

		x = (int)strtol(&ip[i],NULL, 10);
		i += (strlen(&ip[i]) + 1);

		y = (int)strtol(&ip[i],NULL, 10);
		i += (strlen(&ip[i]) + 1);

		z = (int)strtol(&ip[i],NULL, 10);
		i += (strlen(&ip[i]) + 1);

		//	printf("IP = %d.%d.%d.%d\n", w, x, y, z);

		port = 80;

		// Success.  We resolved the address.
		IP4_ADDR( ((struct ip_addr*)&(server->sin_addr)) ,w,x,y,z );
		server->sin_port = htons( port );

	}
	else
	{
		// resolve the host name.
		//if ( gethostbyname( ip, &server  ) != 0 )
		//  return NULL;

		// While the DNS resolver doesn't work, you must
		// provide the host name when the http driver is run.
		return NULL;

	}

	char_ptr = url + 2;
	while(*char_ptr != '/') char_ptr++;

	return char_ptr;
}

//
// Any calls we don't implement calls dummy.
// 
int dummy()
{
	printf("PS2HTTP: dummy function called\n");
	return -5;
}

//
// Initialise clears our list of socket handles that
// keeps track of open connections.
//
void fd_initialize( iop_device_t *driver)
{

	printf("PS2HTTP: initializing '%s' file driver.\n", driver->name );
  
	// Clear the file handles. 
	memset(&handles, 0, sizeof(handles));

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
int fd_open( iop_file_t *f, char *name, int mode)
{
	int peerHandle = 0;
	struct sockaddr_in server;
	char *getName;
	int handle;

	//printf( "HTTP: fd_open %i %s\n", f->num, name );

	// First find an open Handle.
	for(handle = 0; handle < HCOUNT; handle++)
		if(handles[handle].used == 0) break;

	// No free handles. exit.
	if(handle >= HCOUNT) return -1;

	// Reserve this for our use.
	// We don't have mutex semaphores protecting.. so quicker we
	// stop another thread stealing our it the better.
	handles[handle].used = 1;

	// Store kernel file handle
	handles[handle].fd = f->unit;

	handles[handle].fileSize = 0;
	handles[handle].filePos = 0;

	// Check valid IP address and URL
	getName = resolveAddress( &server, name );
	if ( getName == NULL ) 
	{
		// free up the handle and return error.
		handles[handle].used = 0;
		return -1;
	}

	// Now we connect and initiate the transfer by sending a 
	// request header to the server, and receiving the response header
	peerHandle = httpConnect( &server, getName, &handles[handle] );   
	if ( peerHandle < 0 )
	{
		// free up the handle and return error.
		handles[handle].used = 0;
		return peerHandle;   
	}

	// http connect returns valid socket.  Save in handle list.
	handles[handle].sockFd = peerHandle;

	// return success.  We got it all ready. :)
	return f->unit;
}



//
// Read is simple.  It simply needs to find the socket no
// based on the file handle and call recv.
//
int fd_read( iop_file_t *f, char * buffer, int size )
{
	int bytesRead = 0;
	int handle;
	int left = size;
	int totalRead = 0;

	//printf( "HTTP: fd_read %i\n", size );

	// First find correct handle
	for(handle = 0; handle < HCOUNT; handle++)
		if(handles[handle].fd == f->unit ) break;

	if(handle >= HCOUNT) return -1;

	// Read until: there is an error, we've read "size" bytes or the remote 
	//             side has closed the connection.
	do {

		bytesRead = recv( handles[handle].sockFd, buffer + totalRead, left, 0 ); 

		//printf("bytesRead = %d\n", bytesRead);

		if(bytesRead <= 0) break;

		left -= bytesRead;
		totalRead += bytesRead;

	} while(left);

	// Check for EOF condition.  
	if ( bytesRead == 0  && totalRead == 0 )
	{
		//printf( "HTTP: fd_read ret -1\n" );
		return -1;
	}

	// Check for Error condition.
	if ( bytesRead < 0 && totalRead == 0 )
	{
		//printf( "HTTP: fd_read ret %i\n", bytesRead );
		return bytesRead;
	}

	//printf( "HTTP: fd_read ret %i %i\n", totalRead, (int) buffer[0] );
	return totalRead; 
}


//
// Close finds the correct handle and
// calls disconnect.
//
int fd_close( iop_file_t *f )
{
	int handle;

	//printf( "HTTP: fd_close\n" );

	for(handle = 0; handle < HCOUNT; handle++)
		if(handles[handle].fd == f->unit) break;

	if(handle >= HCOUNT) return -1;

	disconnect(handles[handle].sockFd);

	handles[handle].used = 0;

	return 0;
}

//
// lseek is only supported in order to get the size of a file. Allough the function
// does modify the filePos member, this does not have any effect on the read position
// of the file in the current implimentation.
//
int fd_lseek( iop_file_t *f, unsigned long offset, int whence)
{
	int handle;

	//printf( "HTTP: fd_lseek %i, %i\n", offset, whence );

	// First find correct handle
	for(handle = 0; handle < HCOUNT; handle++)
		if(handles[handle].fd == f->unit ) break;

	if(handle >= HCOUNT) return -1;

	//printf( "HTTP: fd_lseek %i, %i, %i, %i\n",f->num, handle, offset, whence );

	switch(whence)
	{
	case SEEK_SET:
		handle[handles].filePos = offset;
		break;

	case SEEK_CUR:
		handle[handles].filePos += offset;
		break;

	case SEEK_END:
		handle[handles].filePos = handle[handles].fileSize + offset;
		break;

	default:
		return -1;
	}

	//printf( "filePos %i\n", handle[handles].filePos );

	return handle[handles].filePos;
}


//
// Main..  registers the File driver.
//
int _start( int argc, char **argv)
{
	printf("PS2HTTP: Module Loaded\n");


	// Argv[1] is the IP address to resolve to for all
 	// non-ip addresses.

	if ( argc >= 2 )
	{
		strcpy( domainHost, HTTPHOST );
		strcat( domainHost, argv[2] );
		strcat( domainHost, HTTPENDHEADER );
	}
	else
	{
		domainHost[0] = '\0';
	}

 
	driver.name = "http";
	driver.type = 16;
	driver.version = 1;
	driver.desc = "http client file driver";
	driver.ops = &functions;

	functions.init = fd_initialize;
	functions.deinit = dummy;
	functions.format = dummy;
	functions.open = fd_open;
	functions.close = fd_close;
	functions.read = fd_read;
	functions.write = dummy;
	functions.lseek = fd_lseek;
	functions.ioctl = dummy;
	functions.remove = dummy;
	functions.mkdir = dummy;
	functions.rmdir = dummy;
	functions.dopen = dummy;
	functions.dclose = dummy;
	functions.dread = dummy;
	functions.getstat = dummy;
	functions.chstat = dummy;
 
	DelDrv( "http");
	AddDrv( &driver);

	return 0;
}

