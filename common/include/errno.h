/*
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2003, adresd (adresd_ps2dev@yahoo.com)
  ------------------------------------------------------------------------
  errno.h
			global error codes and string values
			gives return values for all things
*/

#ifndef _ERRNO_H
#define _ERRNO_H
 

#define	EPERM 1		/* Not super-user */
#define	ENOENT 2	/* No such file or directory */
#define	ESRCH 3		/* No such process */
#define	EINTR 4		/* Interrupted system call */
#define	EIO 5		/* I/O error */
#define	ENXIO 6		/* No such device or address */
#define	E2BIG 7		/* Arg list too long */
#define	ENOEXEC 8	/* Exec format error */
#define	EBADF 9		/* Bad file number */
#define	ECHILD 10	/* No children */
#define	EAGAIN 11	/* No more processes */
#define	ENOMEM 12	/* Not enough core */
#define	EACCES 13	/* Permission denied */
#define	EFAULT 14	/* Bad address */
#define	ENOTBLK 15	/* Block device required */
#define	EBUSY 16	/* Mount device busy */
#define	EEXIST 17	/* File exists */
#define	EXDEV 18	/* Cross-device link */
#define	ENODEV 19	/* No such device */
#define	ENOTDIR 20	/* Not a directory */
#define	EISDIR 21	/* Is a directory */
#define	EINVAL 22	/* Invalid argument */
#define	ENFILE 23	/* Too many open files in system */
#define	EMFILE 24	/* Too many open files */
#define	ENOTTY 25	/* Not a typewriter */
#define	ETXTBSY 26	/* Text file busy */
#define	EFBIG 27	/* File too large */
#define	ENOSPC 28	/* No space left on device */
#define	ESPIPE 29	/* Illegal seek */
#define	EROFS 30	/* Read only file system */
#define	EMLINK 31	/* Too many links */
#define	EPIPE 32	/* Broken pipe */
#define	EDOM 33		/* Math arg out of domain of func */
#define	ERANGE 34	/* Math result not representable */
#define	ENOMSG 35	/* No message of desired type */
#define	EIDRM 36	/* Identifier removed */
#define	ECHRNG 37	/* Channel number out of range */
#define	EL2NSYNC 38	/* Level 2 not synchronized */
#define	EL3HLT 39	/* Level 3 halted */
#define	EL3RST 40	/* Level 3 reset */
#define	ELNRNG 41	/* Link number out of range */
#define	EUNATCH 42	/* Protocol driver not attached */
#define	ENOCSI 43	/* No CSI structure available */
#define	EL2HLT 44	/* Level 2 halted */
#define	EDEADLK 45	/* Deadlock condition */
#define	ENOLCK 46	/* No record locks available */
#define EBADE 50	/* Invalid exchange */
#define EBADR 51	/* Invalid request descriptor */
#define EXFULL 52	/* Exchange full */
#define ENOANO 53	/* No anode */
#define EBADRQC 54	/* Invalid request code */
#define EBADSLT 55	/* Invalid slot */
#define EDEADLOCK 56	/* File locking deadlock error */
#define EBFONT 57	/* Bad font file fmt */
#define ENOSTR 60	/* Device not a stream */
#define ENODATA 61	/* No data (for no delay io) */
#define ETIME 62	/* Timer expired */
#define ENOSR 63	/* Out of streams resources */
#define ENONET 64	/* Machine is not on the network */
#define ENOPKG 65	/* Package not installed */
#define EREMOTE 66	/* The object is remote */
#define ENOLINK 67	/* The link has been severed */
#define EADV 68		/* Advertise error */
#define ESRMNT 69	/* Srmount error */
#define	ECOMM 70	/* Communication error on send */
#define EPROTO 71	/* Protocol error */
#define	EMULTIHOP 74	/* Multihop attempted */
#define	ELBIN 75	/* Inode is remote (not really error) */
#define	EDOTDOT 76	/* Cross mount point (not really error) */
#define EBADMSG 77	/* Trying to read unreadable message */
#define EFTYPE 79	/* Inappropriate file type or format */
#define ENOTUNIQ 80	/* Given log. name not unique */
#define EBADFD 81	/* f.d. invalid for this operation */
#define EREMCHG 82	/* Remote address changed */
#define ELIBACC 83	/* Can't access a needed shared lib */
#define ELIBBAD 84	/* Accessing a corrupted shared lib */
#define ELIBSCN 85	/* .lib section in a.out corrupted */
#define ELIBMAX 86	/* Attempting to link in too many libs */
#define ELIBEXEC 87	/* Attempting to exec a shared library */
#define ENOSYS 88	/* Function not implemented */
#define ENMFILE 89      /* No more files */
#define ENOTEMPTY 90	/* Directory not empty */
#define ENAMETOOLONG 91	/* File or path name too long */
#define ELOOP 92	/* Too many symbolic links */
#define EOPNOTSUPP 95	/* Operation not supported on transport endpoint */
#define EPFNOSUPPORT 96 /* Protocol family not supported */
#define ECONNRESET 104  /* Connection reset by peer */
#define ENOBUFS 105	/* No buffer space available */
#define EAFNOSUPPORT 106 /* Address family not supported by protocol family */
#define EPROTOTYPE 107	/* Protocol wrong type for socket */
#define ENOTSOCK 108	/* Socket operation on non-socket */
#define ENOPROTOOPT 109	/* Protocol not available */
#define ESHUTDOWN 110	/* Can't send after socket shutdown */
#define ECONNREFUSED 111	/* Connection refused */
#define EADDRINUSE 112		/* Address already in use */
#define ECONNABORTED 113	/* Connection aborted */
#define ENETUNREACH 114		/* Network is unreachable */
#define ENETDOWN 115		/* Network interface is not configured */
#define ETIMEDOUT 116		/* Connection timed out */
#define EHOSTDOWN 117		/* Host is down */
#define EHOSTUNREACH 118	/* Host is unreachable */
#define EINPROGRESS 119		/* Connection already in progress */
#define EALREADY 120		/* Socket already connected */
#define EDESTADDRREQ 121	/* Destination address required */
#define EMSGSIZE 122		/* Message too long */
#define EPROTONOSUPPORT 123	/* Unknown protocol */
#define ESOCKTNOSUPPORT 124	/* Socket type not supported */
#define EADDRNOTAVAIL 125	/* Address not available */
#define ENETRESET 126
#define EISCONN 127		/* Socket is already connected */
#define ENOTCONN 128		/* Socket is not connected */
#define ETOOMANYREFS 129
#define EPROCLIM 130
#define EUSERS 131
#define EDQUOT 132
#define ESTALE 133
#define ENOTSUP 134		/* Not supported */
#define ENOMEDIUM 135   /* No medium (in tape drive) */
#define ENOSHARE 136    /* No such host or network path */
#define ECASECLASH 137  /* Filename exists with different case */
#define EILSEQ 138
#define EOVERFLOW 139	/* Value too large for defined data type */

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

#endif // _ERRNO_H_
