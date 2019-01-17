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
 * global error codes and string values
 */

#ifndef __ERRNO_H__
#define __ERRNO_H__

/** Not super-user */
#define	EPERM 1		
/** No such file or directory */
#define	ENOENT 2	
/** No such process */
#define	ESRCH 3		
/** Interrupted system call */
#define	EINTR 4		
/** I/O error */
#define	EIO 5		
/** No such device or address */
#define	ENXIO 6		
/** Arg list too long */
#define	E2BIG 7		
/** Exec format error */
#define	ENOEXEC 8	
/** Bad file number */
#define	EBADF 9		
/** No children */
#define	ECHILD 10	
/** No more processes */
#define	EAGAIN 11	
/** Not enough core */
#define	ENOMEM 12	
/** Permission denied */
#define	EACCES 13	
/** Bad address */
#define	EFAULT 14	
/** Block device required */
#define	ENOTBLK 15	
/** Mount device busy */
#define	EBUSY 16	
/** File exists */
#define	EEXIST 17	
/** Cross-device link */
#define	EXDEV 18	
/** No such device */
#define	ENODEV 19	
/** Not a directory */
#define	ENOTDIR 20	
/** Is a directory */
#define	EISDIR 21	
/** Invalid argument */
#define	EINVAL 22	
/** Too many open files in system */
#define	ENFILE 23	
/** Too many open files */
#define	EMFILE 24	
/** Not a typewriter */
#define	ENOTTY 25	
/** Text file busy */
#define	ETXTBSY 26	
/** File too large */
#define	EFBIG 27	
/** No space left on device */
#define	ENOSPC 28	
/** Illegal seek */
#define	ESPIPE 29	
/** Read only file system */
#define	EROFS 30	
/** Too many links */
#define	EMLINK 31	
/** Broken pipe */
#define	EPIPE 32	
/** Math arg out of domain of func */
#define	EDOM 33		
/** Math result not representable */
#define	ERANGE 34	
/** No message of desired type */
#define	ENOMSG 35	
/** Identifier removed */
#define	EIDRM 36	
/** Channel number out of range */
#define	ECHRNG 37	
/** Level 2 not synchronized */
#define	EL2NSYNC 38	
/** Level 3 halted */
#define	EL3HLT 39	
/** Level 3 reset */
#define	EL3RST 40	
/** Operation would block */
#define	EWOULDBLOCK  EAGAIN  
/** Link number out of range */
#define	ELNRNG 41	
/** Protocol driver not attached */
#define	EUNATCH 42	
/** No CSI structure available */
#define	ENOCSI 43	
/** Level 2 halted */
#define	EL2HLT 44	
/** Deadlock condition */
#define	EDEADLK 45	
/** No record locks available */
#define	ENOLCK 46	
/** Invalid exchange */
#define EBADE 50	
/** Invalid request descriptor */
#define EBADR 51	
/** Exchange full */
#define EXFULL 52	
/** No anode */
#define ENOANO 53	
/** Invalid request code */
#define EBADRQC 54	
/** Invalid slot */
#define EBADSLT 55	
/** File locking deadlock error */
#define EDEADLOCK 56	
/** Bad font file fmt */
#define EBFONT 57	
/** Device not a stream */
#define ENOSTR 60	
/** No data (for no delay io) */
#define ENODATA 61	
/** Timer expired */
#define ETIME 62	
/** Out of streams resources */
#define ENOSR 63	
/** Machine is not on the network */
#define ENONET 64	
/** Package not installed */
#define ENOPKG 65	
/** The object is remote */
#define EREMOTE 66	
/** The link has been severed */
#define ENOLINK 67	
/** Advertise error */
#define EADV 68		
/** Srmount error */
#define ESRMNT 69	
/** Communication error on send */
#define	ECOMM 70	
/** Protocol error */
#define EPROTO 71	
/** Multihop attempted */
#define	EMULTIHOP 74	
/** Inode is remote (not really error) */
#define	ELBIN 75	
/** Cross mount point (not really error) */
#define	EDOTDOT 76	
/** Trying to read unreadable message */
#define EBADMSG 77	
/** Inappropriate file type or format */
#define EFTYPE 79	
/** Given log. name not unique */
#define ENOTUNIQ 80	
/** f.d. invalid for this operation */
#define EBADFD 81	
/** Remote address changed */
#define EREMCHG 82	
/** Can't access a needed shared lib */
#define ELIBACC 83	
/** Accessing a corrupted shared lib */
#define ELIBBAD 84	
/** .lib section in a.out corrupted */
#define ELIBSCN 85	
/** Attempting to link in too many libs */
#define ELIBMAX 86	
/** Attempting to exec a shared library */
#define ELIBEXEC 87	
/** Function not implemented */
#define ENOSYS 88	
/** No more files */
#define ENMFILE 89      
/** Directory not empty */
#define ENOTEMPTY 90	
/** File or path name too long */
#define ENAMETOOLONG 91	
/** Too many symbolic links */
#define ELOOP 92	
/** Operation not supported on transport endpoint */
#define EOPNOTSUPP 95	
/** Protocol family not supported */
#define EPFNOSUPPORT 96 
/** Connection reset by peer */
#define ECONNRESET 104  
/** No buffer space available */
#define ENOBUFS 105	
/** Address family not supported by protocol family */
#define EAFNOSUPPORT 106 
/** Protocol wrong type for socket */
#define EPROTOTYPE 107	
/** Socket operation on non-socket */
#define ENOTSOCK 108	
/** Protocol not available */
#define ENOPROTOOPT 109	
/** Can't send after socket shutdown */
#define ESHUTDOWN 110	
/** Connection refused */
#define ECONNREFUSED 111	
/** Address already in use */
#define EADDRINUSE 112		
/** Connection aborted */
#define ECONNABORTED 113	
/** Network is unreachable */
#define ENETUNREACH 114		
/** Network interface is not configured */
#define ENETDOWN 115		
/** Connection timed out */
#define ETIMEDOUT 116		
/** Host is down */
#define EHOSTDOWN 117		
/** Host is unreachable */
#define EHOSTUNREACH 118	
/** Connection already in progress */
#define EINPROGRESS 119		
/** Socket already connected */
#define EALREADY 120		
/** Destination address required */
#define EDESTADDRREQ 121	
/** Message too long */
#define EMSGSIZE 122		
/** Unknown protocol */
#define EPROTONOSUPPORT 123	
/** Socket type not supported */
#define ESOCKTNOSUPPORT 124	
/** Address not available */
#define EADDRNOTAVAIL 125	
#define ENETRESET 126
/** Socket is already connected */
#define EISCONN 127		
/** Socket is not connected */
#define ENOTCONN 128		
#define ETOOMANYREFS 129
#define EPROCLIM 130
#define EUSERS 131
#define EDQUOT 132
#define ESTALE 133
/** Not supported */
#define ENOTSUP 134		
/** No medium (in tape drive) */
#define ENOMEDIUM 135   
/** No such host or network path */
#define ENOSHARE 136    
/** Filename exists with different case */
#define ECASECLASH 137  
#define EILSEQ 138
/** Value too large for defined data type */
#define EOVERFLOW 139	

#ifndef E_USE_NAMES
#define error_to_string(errnum) ("")
#else
char *file_errors[] = {
"",										//	0
"Not super-user",								//	1
"No such file or directory",						//	2
"No such process",							//	3
"Interrupted system call",						//	4
"I/O error",								//	5
"No such device or address",						//	6
"Arg list too long",							//	7
"Exec format error",							//	8
"Bad file number",							//	9
"No children",								//	10
"No more processes",							//	11
"Not enough core",							//	12
"Permission denied",							//	13
"Bad address",								//	14
"Block device required",						//	15
"Mount device busy",							//	16
"File exists",								//	17
"Cross-device link",							//	18
"No such device",								//	19
"Not a directory",							//	20
"Is a directory",								//	21
"Invalid argument",							//	22
"Too many open files in system",					//	23
"Too many open files",							//	24
"Not a typewriter",							//	25
"Text file busy",								//	26
"File too large",								//	27
"No space left on device",						//	28
"Illegal seek",								//	29
"Read only file system",						//	30
"Too many links",								//	31
"Broken pipe",								//	32
"Math arg out of domain of func",					//	33
"Math result not representable",					//	34
"No message of desired type",						//	35
"Identifier removed",							//	36
"Channel number out of range",					//	37
"Level 2 not synchronized",						//	38
"Level 3 halted",								//	39
"Level 3 reset",								//	40
"Link number out of range",						//	41
"Protocol driver not attached",					//	42
"No CSI structure available",						//	43
"Level 2 halted",								//	44
"Deadlock condition",							//	45
"No record locks available",						//	46
"",										//	47
"",										//	48
"",										//	49
"Invalid exchange",							//	50
"Invalid request descriptor",						//	51
"Exchange full",								//	52
"No anode",									//	53
"Invalid request code",							//	54
"Invalid slot",								//	55
"File locking deadlock error",					//	56
"Bad font file fmt",							//	57
"",										//	58
"",										//	59
"Device not a stream",							//	60
"No data (for no delay io)",						//	61
"Timer expired",								//	62
"Out of streams resources",						//	63
"Machine is not on the network",					//	64
"Package not installed",						//	65
"The object is remote",							//	66
"The link has been severed",						//	67
"Advertise error",							//	68
"Srmount error",								//	69
"Communication error on send",					//	70
"Protocol error",								//	71
"",										//	72
"",										//	73
"Multihop attempted",							//	74
"Inode is remote (not really error)",				//	75


"Cross mount point (not really error)",				//	76
"Trying to read unreadable message",				//	77
"",										//	78
"Inappropriate file type or format",				//	79
"Given log. name not unique",						//	80
"f.d. invalid for this operation",					//	81
"Remote address changed",						//	82
"Can't access a needed shared lib",					//	83
"Accessing a corrupted shared lib",					//	84
".lib section in a.out corrupted",					//	85
"Attempting to link in too many libs",				//	86
"Attempting to exec a shared library",				//	87
"Function not implemented",						//	88
"No more files",								//	89
"Directory not empty",							//	90
"File or path name too long",						//	91
"Too many symbolic links",						//	92
"",										//	93
"",										//	94
"Operation not supported on transport endpoint",		//	95
"Protocol family not supported",					//	96
"",										//	97
"",										//	98
"",										//	99
"",										//	100
"",										//	101
"",										//	102
"",										//	103
"Connection reset by peer",						//	104
"No buffer space available",						//	105
"Address family not supported by protocol family",		//	106
"Protocol wrong type for socket",					//	107
"Socket operation on non-socket",					//	108
"Protocol not available",						//	109
"Can't send after socket shutdown",					//	110
"Connection refused",							//	111
"Address already in use",						//	112
"Connection aborted",							//	113
"Network is unreachable",						//	114
"Network interface is not configured",				//	115
"Connection timed out",							//	116
"Host is down",								//	117
"Host is unreachable",							//	118
"Connection already in progress",					//	119
"Socket already connected",						//	120
"Destination address required",					//	121
"Message too long",							//	122
"Unknown protocol",							//	123
"Socket type not supported",						//	124
"Address not available",						//	125
"",										//	126
"Socket is already connected",					//	127
"Socket is not connected",						//	128
"",										//	129
"EPROCLIM",									//	130
"EUSERS",									//	131
"EDQUOT",									//	132
"ESTALE",									//	133
"Not supported",								//	134
"No medium (in tape drive)",						//	135
"No such host or network path",					//	136
"Filename exists with different case",				//	137
"EILSEQ",									//	138
"Value too large for defined data type",				//	139
""};
#define error_to_string(errnum) (file_errors[errnum*-1])
#endif

extern int errno __attribute__((section("data")));

#endif /* __ERRNO_H__ */
