/*
 * cdvdman.h - Definitions and imports for cdvdman
 *
 * Copyright (c) 2003 Nicholas Van Veen <nickvv@xtra.co.nz>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef IOP_CDVDMAN_H
#define IOP_CDVDMAN_H

#include "types.h"
#include "irx.h"

#define CdSecS2048		0	
#define CdSecS2328		1	
#define CdSecS2340		2	

#define CdSpinMax		0
#define CdSpinNom		1
#define CdSpinStm		0

#define CdMmodeCd		1
#define CdMmodeDvd		2

typedef struct {
	u8 stat;  			
	u8 second; 			
	u8 minute; 			
	u8 hour; 			
	u8 week; 			
	u8 day; 			
	u8 month; 			
	u8 year; 			
} cd_clock_t;

typedef struct {
	u32 lsn; 			
	u32 size; 			
	char name[16]; 		
	u8 date[8]; 		
} cd_file_t;

typedef struct {
	u8 minute; 			
	u8 second; 			
	u8 sector; 			
	u8 track; 			
} cd_location_t;

typedef struct {
	u8 trycount; 		
	u8 spindlctrl; 		
	u8 datapattern; 	
	u8 pad; 			
} cd_read_mode_t;

#define cdvdman_IMPORTS_start DECLARE_IMPORT_TABLE(cdvdman, 1, 1)
#define cdvdman_IMPORTS_end END_IMPORT_TABLE

int sceCdInit(int iniI_mode);
#define I_sceCdInit DECLARE_IMPORT(4 , sceCdInit)
int sceCdStandby(void);
#define I_sceCdStandby DECLARE_IMPORT(5 , sceCdStandby)
int sceCdRead(u32 lsn, u32 sectors, void *buf, cd_read_mode_t *mode);
#define I_sceCdRead DECLARE_IMPORT(6 , sceCdRead)
int sceCdSeek(u32 lsn);
#define I_sceCdSeek DECLARE_IMPORT(7 , sceCdSeek)
int sceCdGetError(void);
#define I_sceCdGetError DECLARE_IMPORT(8 , sceCdGetError)
int sceCdGetToc(u8 *toc);
#define I_sceCdGetToc DECLARE_IMPORT(9 , sceCdGetToc)
int sceCdSearchFile(cd_file_t *fp, const char *name);
#define I_sceCdSearchFile DECLARE_IMPORT(10 , sceCdSearchFile)
int sceCdSync(int mode);
#define I_sceCdSync DECLARE_IMPORT(11 , sceCdSync)
int sceCdGetDiskType(void);
#define I_sceCdGetDiskType DECLARE_IMPORT(12 , sceCdGetDiskType)
int sceCdDiskReady(int mode);
#define I_sceCdDiskReady DECLARE_IMPORT(13 , sceCdDiskReady)
int sceCdTrayReq(int mode, u32 *traycnt);
#define I_sceCdTrayReq DECLARE_IMPORT(14 , sceCdTrayReq)
int sceCdStop(void);
#define I_sceCdStop DECLARE_IMPORT(15 , sceCdStop)
int sceCdPosToInt(cd_location_t *p);
#define I_sceCdPosToInt DECLARE_IMPORT(16 , sceCdPosToInt)
cd_location_t *sceCdIntToPos(int i, cd_location_t *p);
#define I_sceCdIntToPos DECLARE_IMPORT(17 , sceCdIntToPos)
int sceCdReadClock(cd_clock_t *rtc);
#define I_sceCdReadClock DECLARE_IMPORT(24 , sceCdReadClock)
int sceCdStatus(void);
#define I_sceCdStatus DECLARE_IMPORT(28 , sceCdStatus)
int sceCdApplySCmd(u8 cmd, void *in, u32 in_size, void *out);
#define I_sceCdApplySCmd DECLARE_IMPORT(29, sceCdApplySCmd)
int sceCdCallback(void (*func)());
#define I_sceCdCallback DECLARE_IMPORT(37 , sceCdCallback)
int sceCdPause(void);
#define I_sceCdPause DECLARE_IMPORT(38 , sceCdPause)
int sceCdBreak(void);
#define I_sceCdBreak DECLARE_IMPORT(39, sceCdBreak)
u32 sceCdGetReadPos(void);
#define I_sceCdReadConsoleID DECLARE_IMPORT(41, sceCdReadConsoleID)
int sceCdReadConsoleID( u32 *res, int *idBuf );
#define I_sceCdGetReadPos DECLARE_IMPORT(44 , sceCdGetReadPos)
int sceCdMmode(int mode);
#define I_sceCdMmode DECLARE_IMPORT(75 , sceCdMmode)

/* Compatibility names for use with ps2lib.  The use of these names without
   the official name is deprecated, don't add new imports if the name is not
   the same name used in the PS2 BIOS (IOW, leave the 'sce' prefix intact).  */
#define CdInit sceCdInit
#define CdStandby sceCdStandby
#define CdRead sceCdRead
#define CdSeek sceCdSeek
#define CdGetError sceCdGetError
#define CdGetToc sceCdGetToc
#define CdSearchFile sceCdSearchFile
#define CdSync sceCdSync
#define CdGetDiskType sceCdGetDiskType
#define CdDiskReady sceCdDiskReady
#define CdTrayReq sceCdTrayReq
#define CdStop sceCdStop
#define CdPosToInt sceCdPosToInt
#define CdIntToPos sceCdIntToPos
#define CdReadClock sceCdReadClock
#define CdStatus sceCdStatus
#define CdCallback sceCdCallback
#define CdPause sceCdPause
#define CdBreak sceCdBreak
#define CdGetReadPos sceCdGetReadPos
#define CdMmode sceCdMmode

#endif /* IOP_CDVDMAN_H */
