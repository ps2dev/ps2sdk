/*
 * sys/fcntl.h - File control definitions.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef SYS_FCNTL_H
#define SYS_FCNTL_H

#define O_RDONLY	0x0001
#define O_WRONLY	0x0002
#define O_RDWR		0x0003
#define O_DIROPEN	0x0008	// Internal use for dopen
#define O_NBLOCK	0x0010
#define O_APPEND	0x0100
#define O_CREAT		0x0200
#define O_TRUNC		0x0400
#define	O_EXCL		0x0800
#define O_NOWAIT	0x8000

// Access flags for filesystem mount
#define FIO_MT_RDWR			0x00
#define	FIO_MT_RDONLY		0x01

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#endif /* SYS_FCNTL_H */
