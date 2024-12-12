/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP filesystem driver v1.0
 * This redirects an iomanx device (eg a pfs mount point) the ioman device called 'host'
 * it also installs a naplink RPC driver.
 * This basically sets up ready to run programs as though they were run from naplink or
 * pukklink, regarding host and naplink printf.
 */

//#define DEBUG

#include "types.h"
#include "loadcore.h"
#include "stdio.h"
#include "sysclib.h"
#include <errno.h>
#include "thbase.h"
#include "intrman.h"
#include "sysmem.h"
#include "sifman.h"
#include "sifcmd.h"

#include "iomanX.h"
#include "ioman_mod.h"

#define TRUE	1
#define FALSE	0

/** @defgroup fakehost fakehost - host: to hdd driver */

#define MODNAME "fakehost"

// This is the filesystem to replace
#define FS_REPNAME "host"

IRX_ID(MODNAME, 1, 1);

#define M_PRINTF(format, args...) \
    printf(MODNAME ": " format, ##args)

#ifdef DEBUG
#define M_DEBUG M_PRINTF
#else
#define M_DEBUG(format, args...)
#endif

// Host base location
static char base[100];

static int fd_global;

extern int ttyMount(void);
extern int naplinkRpcInit(void);

/** Make a full pathname string, adding base on.
 * @ingroup fakehost
 *
 * @param buffer buffer to hold result.
 * @param name   name to add base to.
 * @return Pointer to buffer.
 *
 *  This function takes the name param and adds it to the basename
 *  for replacement, returning the destination full pathname.
 */
char * fd_name( char * buffer, const char * name )
{
	strcpy( buffer, base );
	strcat( buffer, name );
	return buffer;
}

/** Store filedescriptor and get client filedescriptor.
 * @ingroup fakehost
 *
 * @param fd  filedescriptor to store.
 * @param f   io_file pointer to store fd in .
 * @return client filedescriptor.
 */
int fd_save( int fd, iop_io_file_t *f )
{
	f->unit = ++fd_global;
	f->privdata = (void *) fd;
	return f->unit;
}

/** Get real filedescriptor.
 * @ingroup fakehost
 *
 * @param f   io_file pointer to get fd from.
 * @return real filedescriptor.
 */
int realfd( iop_io_file_t *f )
{
	return (int) f->privdata;
}

/** Handle open request.
 * @ingroup fakehost
 *
 * @param f     Pointer to io_device structure.
 * @param name  pathname.
 * @param mode  open mode.
 * @return Status (as for fileio open).
 */
int fd_open( iop_io_file_t *f, const char *name, int mode)
{
	char nameBuffer[ 250 ];
	int fd;

	M_DEBUG("open %i %s %s\n", f->unit, name ,base);
	fd = iomanX_open( fd_name( nameBuffer, name), mode, 0 );
	if ( fd < 0 ) return fd;

	return fd_save( fd, f );
}

/** Handle close request.
 * @ingroup fakehost
 *
 * @param f     Pointer to io_device structure.
 * @return Status (as for fileio close).
 */
int fd_close( iop_io_file_t *f )
{
	return iomanX_close( realfd(f) );
}

/** Handle read request.
 * @ingroup fakehost
 *
 * @param f       Pointer to io_device structure.
 * @param buffer  Pointer to read buffer.
 * @param size    Size of buffer.
 * @return Status (as for fileio read).
 */
int fd_read( iop_io_file_t *f, void * buffer, int size )
{
	return iomanX_read( realfd(f), buffer, size );
}

/** Handle write request.
 * @ingroup fakehost
 *
 * @param f       Pointer to io_device structure.
 * @param buffer  Pointer to read buffer.
 * @param size    Size of buffer.
 * @return Status (as for fileio write).
 */
int fd_write( iop_io_file_t *fd, void *buffer, int size )
{
	return iomanX_write( realfd(fd), buffer, size );
}

/** Handle lseek request.
 * @ingroup fakehost
 *
 * @param f       Pointer to io_device structure.
 * @param offset  Offset for seek.
 * @param whence  Base for seek.
 * @return Status (as for fileio lseek).
 */
int fd_lseek( iop_io_file_t *fd, int offset, int whence)
{
	return iomanX_lseek( realfd(fd), offset, whence );
}

IOMAN_RETURN_VALUE_IMPL(0);
IOMAN_RETURN_VALUE_IMPL(EIO);

// Function array for fileio structure.
static iop_io_device_ops_t functions = {
	IOMAN_RETURN_VALUE(0), // init
	IOMAN_RETURN_VALUE(0), // deinit
	IOMAN_RETURN_VALUE(EIO), // format
	&fd_open, // open
	&fd_close, // close
	&fd_read, // read
	IOMAN_RETURN_VALUE(EIO), // write
	&fd_lseek, // lseek
	IOMAN_RETURN_VALUE(EIO), // ioctl
	IOMAN_RETURN_VALUE(EIO), // remove
	IOMAN_RETURN_VALUE(EIO), // mkdir
	IOMAN_RETURN_VALUE(EIO), // rmdir
	IOMAN_RETURN_VALUE(EIO), // dopen
	IOMAN_RETURN_VALUE(EIO), // dclose
	IOMAN_RETURN_VALUE(EIO), // dread
	IOMAN_RETURN_VALUE(EIO), // getstat
	IOMAN_RETURN_VALUE(EIO), // chstat
};

// FileIO structure.
static iop_io_device_t driver = {
	FS_REPNAME,
	16,
	1,
	"host redirection driver",
	&functions,
};

/** Entry point for IRX.
 * @ingroup fakehost
 *
 * if argc != 2 , quit, as it needs parameter
 * if argc == 2 , use arv[1] as basename.
 *
 * @param argc Number of arguments.
 * @param argv Pointer to array of arguments.
 * @return Module Status on Exit.
 *
 * This initialises the fakehost driver, setting up the 'host:' fs driver
 * basename redirection, and naplink compatible rpc hander.
 *
 * return values:
 *   MODULE_RESIDENT_END if loaded and registered as library.
 *   MODULE_NO_RESIDENT_END if just exiting normally.
 */
int _start( int argc, char *argv[] )
{
	M_PRINTF( "Copyright (c) 2004 adresd\n" );

	if ( argc != 2 )
	{
		M_PRINTF( "HOST requires based argument\n" );
		return MODULE_NO_RESIDENT_END;
	}
      else
      {
		// Copy the base location.
		strncpy( base, argv[1] ,sizeof(base) - 1);
		base[sizeof(base) - 1] = '\0';

      	M_PRINTF( "redirecting '%s:' to '%s'\n",FS_REPNAME,base);
		fd_global = 1;

		M_PRINTF( "HOST final step, bye\n" );

		// Install naplink RPC handler
		naplinkRpcInit();

      	// now install the fileio driver
		io_DelDrv( FS_REPNAME );
		io_AddDrv( &driver );
	}
	return MODULE_RESIDENT_END;
}


