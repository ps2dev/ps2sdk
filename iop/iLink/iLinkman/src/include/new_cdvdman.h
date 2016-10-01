/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: cdvdman.h 1224 2005-10-14 16:37:42Z eeug $
# Definitions and imports for cdvdman
*/

#ifndef IOP_CDVDMAN_H
#define IOP_CDVDMAN_H

#include <types.h>
#include <irx.h>

/* file open modes */
#define SCE_CdCACHE 0x10000000
#define SCE_CdSTREAM 0x40000000

#define CdSecS2048 0
#define CdSecS2328 1
#define CdSecS2340 2

#define CdSpinMax 0
#define CdSpinNom 1
#define CdSpinStm 0

#define CdMmodeCd 1
#define CdMmodeDvd 2

typedef struct
{
    unsigned char stat;
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char padding; /* This is really just a padding field. */
    unsigned char day;
    unsigned char month;
    unsigned char year;
} cd_clock_t;

typedef struct
{
    unsigned long int lsn;
    unsigned long int size;
    char name[16];
    unsigned char date[8];

    unsigned long int flag; /* Bits 0-7: ISO9660 file type flag, Bits 8-31 are reserved */
} adv_cd_file_t;

typedef struct
{
    unsigned long int lsn;
    unsigned long int size;
    char name[16];
    unsigned char date[8];
} cd_file_t;

typedef struct
{
    unsigned char minute;
    unsigned char second;
    unsigned char sector;
    unsigned char track;
} cd_location_t;

typedef struct
{
    unsigned char trycount;
    unsigned char spindlctrl;
    unsigned char datapattern;
    unsigned char pad;
} cd_read_mode_t;

/* SP193: Added structure for chain reading */
typedef struct
{
    unsigned long int sectorLoc;  /* sector location to start reading from */
    unsigned long int numSectors; /* number of sectors to read */
    unsigned long int buffer;     /* buffer address to read to ( bit0: 0=EE, 1=IOP ) */
                                  /* (EE addresses must be on 64byte alignment) */
} CdvdChain_t;

#define cdvdman_IMPORTS_start DECLARE_IMPORT_TABLE(cdvdman, 1, 1)
#define cdvdman_IMPORTS_end END_IMPORT_TABLE

/* SP193: Added export #3 */
void cdvdman_termcall(int arg);
#define I_cdvdman_termcall DECLARE_IMPORT(3, cdvdman_termcall)

int sceCdInit(int iniI_mode);
#define I_sceCdInit DECLARE_IMPORT(4, sceCdInit)
int sceCdStandby(void);
#define I_sceCdStandby DECLARE_IMPORT(5, sceCdStandby)
int sceCdRead(unsigned long int lsn, unsigned long int sectors, void *buf, cd_read_mode_t *mode);
#define I_sceCdRead DECLARE_IMPORT(6, sceCdRead)
int sceCdSeek(unsigned long int lsn);
#define I_sceCdSeek DECLARE_IMPORT(7, sceCdSeek)
int sceCdGetError(void);
#define I_sceCdGetError DECLARE_IMPORT(8, sceCdGetError)
int sceCdGetToc(unsigned char *toc);
#define I_sceCdGetToc DECLARE_IMPORT(9, sceCdGetToc)
int sceCdSearchFile(cd_file_t *fp, const char *name);
#define I_sceCdSearchFile DECLARE_IMPORT(10, sceCdSearchFile)
int sceCdSync(int mode);
#define I_sceCdSync DECLARE_IMPORT(11, sceCdSync)
int sceCdGetDiskType(void);
#define I_sceCdGetDiskType DECLARE_IMPORT(12, sceCdGetDiskType)
int sceCdDiskReady(int mode);
#define I_sceCdDiskReady DECLARE_IMPORT(13, sceCdDiskReady)
int sceCdTrayReq(int mode, unsigned long int *traycnt);
#define I_sceCdTrayReq DECLARE_IMPORT(14, sceCdTrayReq)
int sceCdStop(void);
#define I_sceCdStop DECLARE_IMPORT(15, sceCdStop)
int sceCdPosToInt(cd_location_t *p);
#define I_sceCdPosToInt DECLARE_IMPORT(16, sceCdPosToInt)
cd_location_t *sceCdIntToPos(int i, cd_location_t *p);
#define I_sceCdIntToPos DECLARE_IMPORT(17, sceCdIntToPos)
int sceCdGetToc2(char *toc, int param); /* SP193: Changed "int,int" to "char *toc, int param", and sceCdReadPfi to sceCdGetToc2. */
#define I_sceCdGetToc2 DECLARE_IMPORT(19, sceCdGetToc2)
int sceCdReadDVDV(unsigned long int lsn, unsigned long int sectors, void *buf, cd_read_mode_t *mode); /* SP193: Corrected "int"s to "unsigned long int"s in prototype */
#define I_sceCdReadDVDV DECLARE_IMPORT(20, sceCdReadDVDV)

/* SP193: Added export #21 and #22 */
int sceCdCheckCmd(void);
#define I_sceCdCheckCmd DECLARE_IMPORT(21, sceCdCheckCmd)
int sceCdRI(unsigned char *buf, int *stat);
#define I_sceCdRI DECLARE_IMPORT(22, sceCdRI)

int sceCdReadClock(cd_clock_t *rtc);
#define I_sceCdReadClock DECLARE_IMPORT(24, sceCdReadClock)
int sceCdStatus(void);
#define I_sceCdStatus DECLARE_IMPORT(28, sceCdStatus)
int sceCdApplySCmd(unsigned char cmd, void *in, unsigned long int in_size, void *out);
#define I_sceCdApplySCmd DECLARE_IMPORT(29, sceCdApplySCmd)

/* SP193: Added exports #30 - #36 */
int sceCdSetHDMode(int arg);
#define I_sceCdSetHDMode DECLARE_IMPORT(30, sceCdSetHDMode)
int sceCdReadKey(int arg1, int arg2, unsigned long int lsn, char *key);
#define I_sceCdReadKey DECLARE_IMPORT(35, sceCdReadKey)
int sceCdDecSet(unsigned int arg1, unsigned int arg2, unsigned int shift);
#define I_sceCdDecSet DECLARE_IMPORT(36, sceCdDecSet)

void *sceCdCallback(void (*func)()); /* SP193: Corrected return value from int to void* */
#define I_sceCdCallback DECLARE_IMPORT(37, sceCdCallback)
int sceCdPause(void);
#define I_sceCdPause DECLARE_IMPORT(38, sceCdPause)
int sceCdBreak(void);
#define I_sceCdBreak DECLARE_IMPORT(39, sceCdBreak)
int sceCdReadCdda(unsigned long int lsn, unsigned long int sectors, void *buf, cd_read_mode_t *mode);
#define I_sceCdReadCdda DECLARE_IMPORT(40, sceCdReadCdda)
int sceCdReadConsoleID(u64 *id, unsigned long int *stat); /* SP193: Changed "unsigned long int *res, int *stat" to "u64 *id, unsigned long int *stat" */
#define I_sceCdReadConsoleID DECLARE_IMPORT(41, sceCdReadConsoleID)

/* SP193: Added export #43 */
int sceCdMV(unsigned long int *mv, unsigned long int *stat);
#define I_sceCdMV DECLARE_IMPORT(43, sceCdMV)

unsigned long int sceCdGetReadPos(void);
#define I_sceCdGetReadPos DECLARE_IMPORT(44, sceCdGetReadPos)

/* SP193: Added exports #45 - #74 */
int sceCdCtrlADoutsceCdCtrlADout(int param, int *stat);
#define I_sceCdCtrlADout DECLARE_IMPORT(45, sceCdCtrlADout)
int sceCdNop(void);
#define I_sceCdNop DECLARE_IMPORT(46, sceCdNop)
void *sceGetFsvRbuf(void);
#define I_sceGetFsvRbuf DECLARE_IMPORT(47, sceGetFsvRbuf)
int sceCdstm0Cb(void (*p)(int));
#define I_sceCdstm0Cb DECLARE_IMPORT(48, sceCdstm0Cb)
int sceCdstm1Cb(void (*p)(int));
#define I_sceCdstm1Cb DECLARE_IMPORT(49, sceCdstm1Cb)
int sceCdSC(int code, int *param);
#define I_sceCdSC DECLARE_IMPORT(50, sceCdSC)
int sceCdRC(cd_clock_t *rtc); /* SP193: Note! Clone of sceCdReadClock() */
#define I_sceCdRC DECLARE_IMPORT(51, sceCdRC)
int sceCdForbidDVDP(unsigned long int *stat);
#define I_sceCdForbidDVDP DECLARE_IMPORT(52, sceCdForbidDVDP)
int sceCdReadSUBQ(char *buf, unsigned long int *stat);
#define I_sceCdReadSUBQ DECLARE_IMPORT(53, sceCdReadSUBQ)
int sceCdApplyNCmd(int ncmd, void *ndata, int ndlen);
#define I_sceCdApplyNCmd DECLARE_IMPORT(54, sceCdApplyNCmd)
int sceCdStInit(unsigned long int bufmax, unsigned long int bankmax, void *iop_bufaddr);
#define I_sceCdStInit DECLARE_IMPORT(56, sceCdStInit)
int sceCdStRead(unsigned long int size, void *buf, unsigned long int mode, unsigned long int *err);
#define I_sceCdStRead DECLARE_IMPORT(57, sceCdStRead)
int sceCdStSeek(unsigned long int lsn);
#define I_sceCdStSeek DECLARE_IMPORT(58, sceCdStSeek)
int sceCdStStart(unsigned long int lsn, cd_read_mode_t *mode);
#define I_sceCdStStart DECLARE_IMPORT(59, sceCdStStart)
int sceCdStStat(void);
#define I_sceCdStStat DECLARE_IMPORT(60, sceCdStStat)
int sceCdStStop(void);
#define I_sceCdStStop DECLARE_IMPORT(61, sceCdStStop)
int sceCdRead0(unsigned long int lsn, unsigned long int sectors, void *buf, cd_read_mode_t *mode, int csec, void *cb);
#define I_sceCdRead0 DECLARE_IMPORT(62, sceCdRead0)
int sceCdRV(unsigned long int lsn, unsigned long int sectors, void *buf, cd_read_mode_t *mode, int csec, void *cb);
#define I_sceCdRV DECLARE_IMPORT(63, sceCdRV)
int sceCdRM(char *m, unsigned long int *stat);
#define I_sceCdRM DECLARE_IMPORT(64, sceCdRM)
int sceCdReadChain(CdvdChain_t *read_tag, cd_read_mode_t *mode);
#define I_sceCdReadChain DECLARE_IMPORT(66, sceCdReadChain)
int sceCdStPause(void);
#define I_sceCdStPause DECLARE_IMPORT(67, sceCdStPause)
int sceCdStResume(void);
#define I_sceCdStResume DECLARE_IMPORT(68, sceCdStResume)
int sceCdForbidRead(unsigned long int *);
#define I_sceCdForbidRead DECLARE_IMPORT(69, sceCdForbidRead)
int sceCdBootCertify(unsigned char *);
#define I_sceCdBootCertify DECLARE_IMPORT(70, sceCdBootCertify)
int sceCdSpinCtrlIOP(int speed);
#define I_sceCdSpinCtrlIOP DECLARE_IMPORT(71, sceCdSpinCtrlIOP)
int sceCdBlueLEDCtl(int code, int *result);
#define I_sceCdBlueLEDCtl DECLARE_IMPORT(72, sceCdBlueLEDCtl)
int sceCdCancelPOffRdy(int *stat);
#define I_sceCdCancelPOffRdy DECLARE_IMPORT(73, sceCdCancelPOffRdy)
int sceCdPowerOff(int *stat);
#define I_sceCdPowerOff DECLARE_IMPORT(74, sceCdPowerOff)

int sceCdMmode(int mode);
#define I_sceCdMmode DECLARE_IMPORT(75, sceCdMmode)

/* SP193: Added exports 79 - 125 */
int sceCdReadFull(unsigned long int lsn, unsigned long int sectors, void *buf, cd_read_mode_t *mode);
#define I_sceCdReadFull DECLARE_IMPORT(76, sceCdReadFull)
int sceCdStSeekF(unsigned long int lsn);
#define I_sceCdStSeekF DECLARE_IMPORT(77, sceCdStSeekF)
void *sceCdPOffCallback(void (*func)(void *), void *addr);
#define I_sceCdPOffCallback DECLARE_IMPORT(78, sceCdPOffCallback)
int sceCdReadDiskID(unsigned int *id); /* SP193: Note! DNAS IOPRP images only! */
#define I_sceCdReadDiskID DECLARE_IMPORT(79, sceCdReadDiskID)
int sceCdReadGUID(u64 *guid); /* SP193: Note! DNAS IOPRP images only! */
#define I_sceCdReadGUID DECLARE_IMPORT(80, sceCdReadGUID)
int sceCdSetTimeout(int param, int timeout);
#define I_sceCdSetTimeout DECLARE_IMPORT(81, sceCdSetTimeout)
int sceCdReadModelID(unsigned long int *id); /* SP193: Note! DNAS IOPRP images only! */
#define I_sceCdReadModelID DECLARE_IMPORT(82, sceCdReadModelID)
int sceCdReadDvdDualInfo(int *on_dual, unsigned long int *layer1_start);
#define I_sceCdReadDvdDualInfo DECLARE_IMPORT(83, sceCdReadDvdDualInfo)
int sceCdLayerSearchFile(adv_cd_file_t *fp, const char *path, int layer);
#define I_sceCdLayerSearchFile DECLARE_IMPORT(84, sceCdLayerSearchFile)
int sceCdStatus2(void);
#define I_sceCdStatus2 DECLARE_IMPORT(90, sceCdStatus2)
int sceCdApplySCmd2(unsigned char cmd, void *wdata, unsigned long int wdlen, void *rdata);
#define I_sceCdApplySCmd2 DECLARE_IMPORT(112, sceCdApplySCmd2)
int sceCdReadIOPm(unsigned long int lsn, unsigned long int sectors, void *buf, cd_read_mode_t *mode);
#define I_sceCdReadIOPm DECLARE_IMPORT(114, sceCdReadIOPm)
int sceCdRcBypassCtl(int param, int *stat);
#define I_sceCdRcBypassCtl DECLARE_IMPORT(115, sceCdRcBypassCtl)
int cdvdman_125(int cmd, void *wdata, int wdlen, void *rdata); /* SP193: Note! Unsure what this really does... */
#define I_cdvdman_125 DECLARE_IMPORT(125, cdvdman_125)

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
#define CdReadCdda sceCdReadCdda
#define CdGetReadPos sceCdGetReadPos
#define CdMmode sceCdMmode

#endif /* IOP_CDVDMAN_H */
