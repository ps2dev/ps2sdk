/*
  _____     ___ ____
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2002, David Ryan ( oobles@hotmail.com )
  ------------------------------------------------------------------------
  ps2http.c                HTTP CLIENT FILE SYSTEM DRIVER.
*/

#include <tamtypes.h>
#include <fileio.h>
#include <stdlib.h>
#include <stdio.h>
#include <kernel.h>
#include <ps2debug.h>

#include "ps2ip.h"

#define BUFFER_SIZE  4024 

char buffer[ BUFFER_SIZE ];


void HandleClient( int cs )
{
   int rcvSize,sntSize;

   rcvSize = recv( cs, buffer, BUFFER_SIZE, 0);
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

      rcvSize = recv( cs, buffer, BUFFER_SIZE, 0 );
      if ( rcvSize <= 0 )
      {
         printf( "PS2ECHO: recv returned %i\n", rcvSize );
      }
   }

   // disconnect the socket.
   disconnect( cs );
}


void serverThread( void * arg )
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
   echoServAddr.sin_port = htons(11);

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
   } 
}


int _start( int argc, char **argv)
{
   struct t_thread t;
   int tid;
   

   printf("IOP-ECHO: Module Loaded\n");

   t.type = TH_C;
   t.unknown = 0;
   t.function = serverThread;
   t.stackSize = 0x800;
   t.priority = 0x1e;
   tid = CreateThread( &t );
   if ( tid >= 0 )
      StartThread( tid, NULL );
   else
      printf( "IOP-ECHO: Server thread failed to start. %i\n", tid );

   return 0;
}

