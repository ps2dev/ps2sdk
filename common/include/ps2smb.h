/*
  Copyright 2009-2010, jimmikaelkael
  Licenced under Academic Free License version 3.0
*/

/**
 * @file
 * PS2SMB definitions.
 */

#ifndef _PS2SMB_H
#define _PS2SMB_H

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NO_PASSWORD		-1
#define PLAINTEXT_PASSWORD 	 0
#define HASHED_PASSWORD		 1

// DEVCTL commands
#define SMB_DEVCTL_GETPASSWORDHASHES	0xC0DE0001
#define SMB_DEVCTL_LOGON		0xC0DE0002
#define SMB_DEVCTL_LOGOFF		0xC0DE0003
#define SMB_DEVCTL_GETSHARELIST		0xC0DE0004
#define SMB_DEVCTL_OPENSHARE		0xC0DE0005
#define SMB_DEVCTL_CLOSESHARE		0xC0DE0006
#define SMB_DEVCTL_ECHO			0xC0DE0007
#define SMB_DEVCTL_QUERYDISKINFO	0xC0DE0008

// helpers for DEVCTL commands

typedef struct {
	char password[256];
} smbGetPasswordHashes_in_t;

typedef struct {		// size = 32
	u8 LMhash[16];
	u8 NTLMhash[16];
} smbGetPasswordHashes_out_t;

typedef struct {		// size = 536
	char serverIP[16];
	int  serverPort;
	char User[256];
	char Password[256];
	int  PasswordType;	// PLAINTEXT_PASSWORD or HASHED_PASSWORD
} smbLogOn_in_t;

typedef struct {		// size = 8
	void *EE_addr;
	int   maxent;
} smbGetShareList_in_t;

typedef struct {		// size = 520
	char ShareName[256];
	char Password[256];
	int  PasswordType;	// PLAINTEXT_PASSWORD or HASHED_PASSWORD
} smbOpenShare_in_t;

typedef struct {		// size = 260
	char echo[256];
	int  len;
} smbEcho_in_t;

typedef struct {		// size = 16
	int TotalUnits;
	int BlocksPerUnit;
	int BlockSize;
	int FreeUnits;
} smbQueryDiskInfo_out_t;

typedef struct {		// size = 512
	char ShareName[256];
	char ShareComment[256];
} ShareEntry_t;

// Error codes for some DEVCTL operations.
#define	SMB_DEVCTL_LOGON_ERR_CONN	0x1001
#define	SMB_DEVCTL_LOGON_ERR_PROT	0x1002
#define	SMB_DEVCTL_LOGON_ERR_LOGON	0x1003

#ifdef __cplusplus
}
#endif

#endif
