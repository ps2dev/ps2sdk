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
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <sifrpc.h>
#include <loadfile.h>
#include "ps2ip.h"

t_ip_info ip_info;

// Change these to fit your network configuration
#define IPADDR "192.168.0.10"
#define NETMASK "255.255.255.0"
#define GWADDR "192.168.0.1"

// Change this path to reflect your setup
#define HOSTPATH "host:"


void serverThread();

int main()
{
    char args[LF_ARG_MAX];
    int argsLen;

	SifInitRpc(0);

	// You may also need to change the paths to reflect your setup
	SifLoadModule(HOSTPATH "../../iop/bin/ps2ip.irx", 0, NULL);

    memset(args, 0, LF_ARG_MAX);
    argsLen = 0;
    // Make smap argument list. Each argument is a null-separated string.
    // Argument length is the total length of the list, including null chars
    strcpy(args, IPADDR);
    argsLen += strlen(IPADDR) + 1;
    strcpy(&args[argsLen], NETMASK);
    argsLen += strlen(NETMASK) + 1;
    strcpy(&args[argsLen], GWADDR);
    argsLen += strlen(GWADDR) + 1;

	SifLoadModule(HOSTPATH "../../../ps2eth/bin/ps2smap.irx", argsLen, args);
	SifLoadModule(HOSTPATH "../../iop/bin/ps2echo.irx", 0,NULL );
	SifLoadModule(HOSTPATH "../../iop/bin/ps2ips.irx", 0, NULL);

	if(ps2ip_init() < 0) {
		printf("ERROR: ps2ip_init falied!\n");
		SleepThread();
	}

	serverThread();
}

char buffer[100];


void HandleClient( int cs )
{
   int rcvSize,sntSize;

   rcvSize = recv( cs, buffer, 100, 0);
   if ( rcvSize <= 0 )
   {
      printf( "PS2ECHO: recv returned %i\n", rcvSize );
      return;
   }
   while( rcvSize > 0 )
   {
      sntSize = send( cs, buffer, rcvSize, 0 );
      if ( sntSize != rcvSize )
      {
         printf( "PS2ECHO: send != recv\n" );
         return;
      }

      rcvSize = recv( cs, buffer, 100, 0 );
      if ( rcvSize <= 0 )
      {
         printf( "PS2ECHO: recv returned %i\n", rcvSize );
      }
   }
}


void serverThread()
{
   int sh;
   int cs;
   struct sockaddr_in echoServAddr;
   struct sockaddr_in echoClntAddr;
   int clntLen;
   int rc;

   printf( "PS2ECHO: Server Thread Started.\n" );


   sh = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   if ( sh < 0 )
   {
      printf( "PS2ECHO: Socket failed to create.\n" );   
      SleepThread();
   }

   printf( "PS2ECHO: Got socket.. %i\n" , sh );


   memset( &echoServAddr, 0 , sizeof(echoServAddr ));
   echoServAddr.sin_family = AF_INET;
   echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   echoServAddr.sin_port = htons(12);

   rc = bind( sh, (struct sockaddr *) &echoServAddr, sizeof( echoServAddr) );
   if ( rc < 0 )
   {
      printf( "PS2ECHO: Socket failed to bind.\n" );
      SleepThread();
   } 

   printf( "PS2ECHO: bind returned %i\n",rc );


   rc = listen( sh, 2 );
   if ( rc < 0 )
   {
      printf( "PS2ECHO: listen failed.\n" );
      SleepThread();
   }

   printf(  "PS2ECHO: listen returned %i\n", rc );
 
   while(1)
   {
      clntLen = sizeof( echoClntAddr );
      cs = accept( sh, (struct sockaddr *)&echoClntAddr, &clntLen );
      if ( cs < 0 )
      {
         printf( "PS2ECHO: accept failed.\n" );
         SleepThread();
      }

      printf( "PS2ECHO: accept returned %i.\n", cs );

      HandleClient( cs );

	  disconnect(cs);
	  disconnect(sh);
   } 

	SleepThread();
}
