/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * File attributes and directory entries.
 */

#ifndef __SYS_IOX_STAT_H__
#define __SYS_IOX_STAT_H__

#include <sys/time.h>

#ifdef _EE
#ifndef NEWLIB_PORT_AWARE
#error "Using fio/fileXio functions directly in the newlib port will lead to problems."
#error "Use posix function calls instead."
#endif
#endif

// Flags for chstat 'statmask'
#define FIO_CST_MODE	0x0001
#define FIO_CST_ATTR	0x0002
#define FIO_CST_SIZE	0x0004
#define FIO_CST_CT	0x0008
#define FIO_CST_AT	0x0010
#define FIO_CST_MT	0x0020
#define FIO_CST_PRVT	0x0040

// File mode flags
/** Format mask */
#define FIO_S_IFMT		0xF000
/** Symbolic link */
#define FIO_S_IFLNK		0x4000
/** Regular file */
#define FIO_S_IFREG		0x2000
/** Directory */
#define FIO_S_IFDIR		0x1000

// Access rights
/** SUID */
#define FIO_S_ISUID		0x0800
/** SGID */
#define FIO_S_ISGID		0x0400
/** Sticky bit */
#define FIO_S_ISVTX		0x0200

/** User access rights mask */
#define FIO_S_IRWXU		0x01C0
/** read */
#define FIO_S_IRUSR		0x0100
/** write */
#define FIO_S_IWUSR		0x0080
/** execute */
#define FIO_S_IXUSR		0x0040

/** Group access rights mask */
#define FIO_S_IRWXG		0x0038
/** read */
#define FIO_S_IRGRP		0x0020
/** write */
#define FIO_S_IWGRP		0x0010
/** execute */
#define FIO_S_IXGRP		0x0008

/** Others access rights mask */
#define FIO_S_IRWXO		0x0007
/** read */
#define FIO_S_IROTH		0x0004
/** write */
#define FIO_S_IWOTH		0x0002
/** execute */
#define FIO_S_IXOTH		0x0001

// File mode checking macros
#define FIO_S_ISLNK(m)	(((m) & FIO_S_IFMT) == FIO_S_IFLNK)
#define FIO_S_ISREG(m)	(((m) & FIO_S_IFMT) == FIO_S_IFREG)
#define FIO_S_ISDIR(m)	(((m) & FIO_S_IFMT) == FIO_S_IFDIR)

/* File attributes that are retrieved using the getstat and dread calls, and
   set using chstat.  */

/* The following structures are only supported by iomanX.  */

typedef struct {
	unsigned int	mode;
	unsigned int	attr;
	unsigned int	size;
	unsigned char	ctime[8];
	unsigned char	atime[8];
	unsigned char	mtime[8];
	unsigned int	hisize;
	/** Number of subs (main) / subpart number (sub) */
	unsigned int	private_0;
	unsigned int	private_1;
	unsigned int	private_2;
	unsigned int	private_3;
	unsigned int	private_4;
	/** Sector start.  */
	unsigned int	private_5;
} iox_stat_t;

typedef struct {
	iox_stat_t	stat;
	char		name[256];
	unsigned int	unknown;
} iox_dirent_t;

/* The following defines are only supported by ioman.  */

// File mode flags (for mode in io_stat_t)
/** Format mask */
#define FIO_SO_IFMT		0x0038
/** Symbolic link */
#define FIO_SO_IFLNK		0x0008
/** Regular file */
#define FIO_SO_IFREG		0x0010
/** Directory */
#define FIO_SO_IFDIR		0x0020

/** read */
#define FIO_SO_IROTH		0x0004
/** write */
#define FIO_SO_IWOTH		0x0002
/** execute */
#define FIO_SO_IXOTH		0x0001

// File mode checking macros
#define FIO_SO_ISLNK(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFLNK)
#define FIO_SO_ISREG(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFREG)
#define FIO_SO_ISDIR(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFDIR)

#endif /* __SYS_IOX_STAT_H__ */
