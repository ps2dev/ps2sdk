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
#include <stdio.h>
#include <string.h>

#include "ps2ip.h"


void serverThread();

int main()
{
    // char args[LF_ARG_MAX];
    // int argsLen;

	SifInitRpc(0);

	SifLoadModule("host:ps2ips.irx", 0, NULL);

	if(ps2ip_init() < 0) {
		printf("ERROR: ps2ip_init falied!\n");
		SleepThread();
	}

	serverThread();
	return 0;
}

char buffer[100];


int HandleClient( int cs )
{
   int rcvSize,sntSize;
   // fd_set rd_set;

   rcvSize = recv( cs, buffer, 100, 0);
   if ( rcvSize <= 0 )
   {
      printf( "PS2ECHO: recv returned %i\n", rcvSize );
      return -1;
   }
   sntSize = send( cs, buffer, rcvSize, 0 );

   return 0;
}


void serverThread()
{
   int sh;
   int cs;
   struct sockaddr_in echoServAddr;
   struct sockaddr_in echoClntAddr;
   int clntLen;
   int rc;
   fd_set active_rd_set;
   fd_set rd_set;

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
 
   FD_ZERO(&active_rd_set);
   FD_SET(sh, &active_rd_set);
   while(1)
   {
	   int i;
      clntLen = sizeof( echoClntAddr );
	  rd_set = active_rd_set;
	  if(select(FD_SETSIZE, &rd_set, NULL, NULL, NULL) < 0)
	  {
		  printf("PS2ECHO: Select failed.\n");
		  SleepThread();
	  }

	  for(i = 0; i < FD_SETSIZE; i++)
	  {
		  if(FD_ISSET(i, &rd_set))
		  {
			  if(i == sh)
			  {
				  cs = accept( sh, (struct sockaddr *)&echoClntAddr, &clntLen );
				  if ( cs < 0 )
				  {
					 printf( "PS2ECHO: accept failed.\n" );
					 SleepThread();
				  }
				  FD_SET(cs, &active_rd_set);
				  printf( "PS2ECHO: accept returned %i.\n", cs );
			  }
			  else
			  {
				  if(HandleClient( i ) < 0)
				  {
					  FD_CLR(i, &active_rd_set);
					  disconnect(i);
				  }
			  }
		  }
	  }
   } 

	SleepThread();
}
