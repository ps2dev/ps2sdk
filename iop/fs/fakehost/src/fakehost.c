/*
 * fakehost - IOP filesystem driver v1.0
 *
 * Copyright (c) 2004 adresd <adresd_ps2dev@yahoo.com>
 *
 * This redirects an iomanx device (eg a pfs mount point) the ioman device called 'host' 
 * it also installs a naplink RPC driver.
 * This basically sets up ready to run programs as though they were run from naplink or
 * pukklink, regarding host and naplink printf.
 *
 * Licensed under the AFL v2.0. See the file LICENSE included with this
 * distribution for licensing terms.
 */

//#define DEBUG_T

#include "types.h"
#include "stdio.h"
#include "sysclib.h"
#include "thbase.h"
#include "intrman.h"
#include "sysmem.h"
#include "sifman.h"
#include "sifcmd.h"

#include "iomanX.h"
#include "ioman_mod.h"

#define TRUE	1
#define FALSE	0

/** \defgroup fakehost fakehost - host: to hdd driver */ 

#define MODNAME "fakehost"

// This is the filesystem to replace
#define FS_REPNAME "host"

IRX_ID(MODNAME, 1, 1);

// Host base location
static char base[100];

// FileIO structure.
static iop_io_device_t driver;

// Function array for fileio structure.
static iop_io_device_ops_t functions;

static int fd_global;

extern int ttyMount(void);
extern int naplinkRpcInit(void);

/*! \brief Make a full pathname string, adding base on.
 *  \ingroup fakehost 
 *
 *  \param buffer buffer to hold result.
 *  \param name   name to add base to.
 *  \return Pointer to buffer.
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

/*! \brief Store filedescriptor and get client filedescriptor.
 *  \ingroup fakehost 
 *
 *  \param fd  filedescriptor to store.
 *  \param f   io_file pointer to store fd in .
 *  \return client filedescriptor.
 *
 */
int fd_save( int fd, iop_io_file_t *f )
{
	f->unit = ++fd_global;
	f->privdata = (void *) fd;
	return f->unit;
}

/*! \brief Get real filedescriptor.
 *  \ingroup fakehost 
 *
 *  \param f   io_file pointer to get fd from.
 *  \return real filedescriptor.
 *
 */
int realfd( iop_io_file_t *f )
{
	return (int) f->privdata;
}

/*! \brief Dummy function, for where needed.
 *  \ingroup fakehost 
 */
int dummy()
{
#ifdef DEBUG_T
	printf("fakehost: dummy function called\n");
#endif
	return -5;
}

/*! \brief Initialise fs driver.
 *  \ingroup fakehost 
 *
 *  \param driver  io_device pointer to device
 *  \return Status (0=successful).
 *
 */
int fd_initialize( iop_io_device_t *driver)
{
	printf("fakehost: initializing '%s' file driver.\n", driver->name );
	return 0;
}

/*! \brief Handle open request.
 *  \ingroup fakehost 
 *
 *  \param f     Pointer to io_device structure.
 *  \param name  pathname.
 *  \param mode  open mode.
 *  \return Status (as for fileio open).
 *
 */
int fd_open( iop_io_file_t *f, const char *name, int mode)
{
	char nameBuffer[ 250 ];
	int fd;

#ifdef DEBUG_T
	printf( "fakehost: open %i %s %s\n", f->unit, name ,base);
#endif
	fd = open( fd_name( nameBuffer, name), mode, 0 );
	if ( fd < 0 ) return fd;

	return fd_save( fd, f );
}

/*! \brief Handle close request.
 *  \ingroup fakehost 
 *
 *  \param f     Pointer to io_device structure.
 *  \return Status (as for fileio close).
 *
 */
int fd_close( iop_io_file_t *f )
{
	return close( realfd(f) );
}

/*! \brief Handle read request.
 *  \ingroup fakehost 
 *
 *  \param f       Pointer to io_device structure.
 *  \param buffer  Pointer to read buffer.
 *  \param size    Size of buffer.
 *  \return Status (as for fileio read).
 *
 */
int fd_read( iop_io_file_t *f, char * buffer, int size )
{
	return read( realfd(f), buffer, size ); 
}

/*! \brief Handle write request.
 *  \ingroup fakehost 
 *
 *  \param f       Pointer to io_device structure.
 *  \param buffer  Pointer to read buffer.
 *  \param size    Size of buffer.
 *  \return Status (as for fileio write).
 *
 */
int fd_write( iop_io_file_t *fd, void *buffer, int size )
{
	return write( realfd(fd), buffer, size );
}

/*! \brief Handle lseek request.
 *  \ingroup fakehost 
 *
 *  \param f       Pointer to io_device structure.
 *  \param offset  Offset for seek.
 *  \param whence  Base for seek.
 *  \return Status (as for fileio lseek).
 *
 */
int fd_lseek( iop_io_file_t *fd, unsigned long offset, int whence)
{
	return lseek( realfd(fd), offset, whence );
}

/*! \brief Entry point for IRX.
 *  \ingroup fakehost 
 *
 *  if argc != 2 , quit, as it needs parameter
 *  if argc == 2 , use arv[1] as basename.
 *
 *  \param argc Number of arguments.
 *  \param argv Pointer to array of arguments.
 *  \return Module Status on Exit.
 *
 *  This initialises the fakehost driver, setting up the 'host:' fs driver
 *  basename redirection, and naplink compatible rpc hander.
 *
 *  return values:
 *    MODULE_RESIDENT_END if loaded and registered as library.
 *    MODULE_NO_RESIDENT_END if just exiting normally.
 */
int _start( int argc, char **argv )
{
	printf( "fakehost: Copyright (c) 2004 adresd\n" );

	if ( argc != 2 )
	{
		printf( "HOST requires based argument\n" );
		return -1;
	}
      else
      {
		strncpy( base, argv[1] ,sizeof(base));

		// Copy the base location.
		strncpy( base, argv[1] ,sizeof(base));
      	printf("redirecting '%s:' to '%s'\n",FS_REPNAME,base);
		fd_global = 1;

		driver.name = FS_REPNAME;
		driver.type = 16;
		driver.version = 1;
		driver.desc = "host redirection driver";
		driver.ops = &functions;

		functions.io_init = fd_initialize;
		functions.io_deinit = dummy;
		functions.io_format = dummy;
		functions.io_open = fd_open;
		functions.io_close = fd_close;
		functions.io_read = fd_read;
		functions.io_write = dummy;
		functions.io_lseek = fd_lseek;
		functions.io_ioctl = dummy;
		functions.io_remove = dummy;
		functions.io_mkdir = dummy;
		functions.io_rmdir = dummy;
		functions.io_dopen =  dummy;
		functions.io_dclose = dummy;
		functions.io_dread = dummy;
		functions.io_getstat = dummy;
		functions.io_chstat = dummy;

		printf( "HOST final step, bye\n" );

		// Install naplink RPC handler
		naplinkRpcInit();
	
      	// now install the fileio driver
		io_DelDrv( FS_REPNAME );
		io_AddDrv( &driver );
	}
	return 0;
}


