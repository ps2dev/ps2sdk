/*
 * sys/stat.h - File attributes and directory entries.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef SYS_STAT_H
#define SYS_STAT_H

// Flags for chstat 'statmask'
#define FIO_CST_MODE	0x0001
#define FIO_CST_ATTR	0x0002
#define FIO_CST_SIZE	0x0004
#define FIO_CST_CT		0x0008
#define FIO_CST_AT		0x0010
#define FIO_CST_MT		0x0020
#define FIO_CST_PRVT	0x0040

// File mode flags
#define FIO_S_IFMT		0xF000		// Format mask
#define FIO_S_IFLNK		0x4000		// Symbolic link
#define FIO_S_IFREG		0x2000		// Regular file
#define FIO_S_IFDIR		0x1000		// Directory

// Access rights
#define FIO_S_ISUID		0x0800		// SUID
#define FIO_S_ISGID		0x0400		// SGID
#define FIO_S_ISVTX		0x0200		// Sticky bit

#define FIO_S_IRWXU		0x01C0		// User access rights mask
#define FIO_S_IRUSR		0x0100		// read
#define FIO_S_IWUSR		0x0080		// write
#define FIO_S_IXUSR		0x0040		// execute

#define FIO_S_IRWXG		0x0038		// Group access rights mask
#define FIO_S_IRGRP		0x0020		// read
#define FIO_S_IWGRP		0x0010		// write
#define FIO_S_IXGRP		0x0008		// execute

#define FIO_S_IRWXO		0x0007		// Others access rights mask
#define FIO_S_IROTH		0x0004		// read
#define FIO_S_IWOTH		0x0002		// write
#define FIO_S_IXOTH		0x0001		// execute

// File mode checking macros
#define FIO_S_ISLNK(m)	(((m) & FIO_S_IFMT) == FIO_S_IFLNK)
#define FIO_S_ISREG(m)	(((m) & FIO_S_IFMT) == FIO_S_IFREG)
#define FIO_S_ISDIR(m)	(((m) & FIO_S_IFMT) == FIO_S_IFDIR)

/* File attributes that are retrieved using the getstat and dread calls, and
   set using chstat.  */

/* The following structures are only supported by iomanX.  */

typedef struct {
/*00*/	unsigned int	mode;
/*04*/	unsigned int	attr;
/*08*/	unsigned int	size;
/*0c*/	unsigned char	ctime[8];
/*14*/	unsigned char	atime[8];
/*1c*/	unsigned char	mtime[8];
/*24*/	unsigned int	hisize;
/*28*/	unsigned int	private_0;		/* Number of subs (main) / subpart number (sub) */
/*2c*/	unsigned int	private_1;
/*30*/	unsigned int	private_2;
/*34*/	unsigned int	private_3;
/*38*/	unsigned int	private_4;
/*3c*/	unsigned int	private_5;		/* Sector start.  */
} iox_stat_t;

typedef struct {
	iox_stat_t	stat;
	char		name[256];
	unsigned int	unknown;
} iox_dirent_t;

/* The following defines are only supported by ioman.  */

// File mode flags (for mode in io_stat_t)
#define FIO_SO_IFMT		0x0038		// Format mask
#define FIO_SO_IFLNK		0x0008		// Symbolic link
#define FIO_SO_IFREG		0x0010		// Regular file
#define FIO_SO_IFDIR		0x0020		// Directory

#define FIO_SO_IROTH		0x0004		// read
#define FIO_SO_IWOTH		0x0002		// write
#define FIO_SO_IXOTH		0x0001		// execute

// File mode checking macros
#define FIO_SO_ISLNK(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFLNK)
#define FIO_SO_ISREG(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFREG)
#define FIO_SO_ISDIR(m)	(((m) & FIO_SO_IFMT) == FIO_SO_IFDIR)

/* The following structures are only supported by ioman.  */

typedef struct {
	unsigned int mode;
	unsigned int attr;
	unsigned int size;
	unsigned char ctime[8];
	unsigned char atime[8];
	unsigned char mtime[8];
	unsigned int hisize;
} io_stat_t;

typedef struct {
	io_stat_t stat;
	char name[256];
	unsigned int unknown;
} io_dirent_t;

#endif /* SYS_STAT_H */
