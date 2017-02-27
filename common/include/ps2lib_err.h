/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Error codes shared across ps2lib.
 */

#ifndef PS2LIB_ERR_H
#define PS2LIB_ERR_H

/* Some of these error codes are taken from libc, since a few of them
   overlap with errors returned by the IOP kernel.  Some others have been
   taken from [RO]man's PS2 BIOS reversing project.  The rest were arbitrarily
   created.

   Feel free to add new error codes where appropriate - just make sure you use
   them!  */

/* Guide to prefixes:

   LIB - ps2lib library-specific errors
   IOP - IOP kernel module/subsystem
   LF  - IOP executable file loader
   SIF - libkernel SIF library
*/

enum _ps2lib_errors {

	/** Generic (unmapped) error. */
	E_LIB_ERROR		= 1,	

	/* Erorrs shared with libc. */
	/** libc: File not found. */
	E_LIB_FILE_NOT_FOUND	= 2,
	/** libc: I/O error. */
	E_LIB_IO_ERROR		= 5,
	/** libc: No more memory. */
	E_LIB_OUT_OF_MEMORY	= 12,
	/** libc: Math arg out of domain of func */
	E_LIB_MATH_DOMAIN	= 33,
	/** libc: Math result not representable */
	E_LIB_MATH_RANGE	= 34,

	/* Errors returned by the IOP kernel and system modules.  */
	/** IOP is in exception context. */	
	E_IOP_INTR_CONTEXT	= 100,
	/** inter IRX dependancy error. */
	E_IOP_DEPENDANCY	= 200,
	/** Invalid IRX module. */
	E_LF_NOT_IRX		= 201,
	/** Unable to open executable file. */
	E_LF_FILE_NOT_FOUND	= 203,
	/** Error while accessing file. */
	E_LF_FILE_IO_ERROR	= 204,
	/** IOP is out of memory. */
	E_IOP_NO_MEMORY		= 400,

	/* Library-specific (API) errors.  */
	/** Unable to initialize library. */
	E_LIB_API_INIT		= 0xd601,
	/** Couldn't create semaphore. */
	E_LIB_SEMA_CREATE	= 0xd602,
	/** Couldn't create thread. */
	E_LIB_THREAD_CREATE	= 0xd603,
	/** Couldn't execute thread. */
	E_LIB_THREAD_START	= 0xd604,
	/** Unsupported/unimplemented function. */
	E_LIB_UNSUPPORTED	= 0xd605,
	/** Invalid argument. */
	E_LIB_INVALID_ARG	= 0xd606,

	/* SIF library */
	/** Can't allocate SIF packet. */	
	E_SIF_PKT_ALLOC		= 0xd610,
	/** Can't send SIF packet. */
	E_SIF_PKT_SEND		= 0xd611,
	/** Couldn't bind to server. */
	E_SIF_RPC_BIND		= 0xd612,
	/** Couldn't execute RPC call. */
	E_SIF_RPC_CALL		= 0xd613,


};
#endif /* PS2LIB_ERR_H */
