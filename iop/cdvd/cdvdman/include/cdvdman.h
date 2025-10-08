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
 * Definitions and imports for cdvdman
 */

#ifndef __CDVDMAN_H__
#define __CDVDMAN_H__

#include <types.h>
#include <irx.h>

#include <libcdvd-common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_CDVD_MODULE
#define PRINTF(...) \
	{ \
		printf(__VA_ARGS__); \
	}
#define KPRINTF(...) \
	{ \
		Kprintf(__VA_ARGS__); \
	}
#define VERBOSE_PRINTF(level, ...) \
	{ \
		if ( g_verbose_level >= (level) ) \
		{ \
			printf(__VA_ARGS__);\
		} \
	}
#define VERBOSE_KPRINTF(level, ...) \
	{ \
		if ( g_verbose_level >= (level) ) \
		{ \
			Kprintf(__VA_ARGS__);\
		} \
	}
#endif

typedef struct cdvdman_dma3_parameter_
{
	u16 m_dma3_blkwords;
	u16 m_dma3_blkcount;
	void *m_dma3_maddress;
	int (*m_dma3_callback)(void);
	u16 m_dma3_csectors;
	u16 m_cdvdreg_howto;
	u32 m_dma3_msectors;
} cdvdman_dma3_parameter_t;

typedef struct cdvdman_internal_struct_
{
	char m_cdvdman_command;
	char m_last_error;
	char m_unused_002;
	char m_ncmd_intr_count;
	int m_wait_flag;
	int m_thread_id;
	int m_stream_flag;
	int m_read2_flag;
	int m_cdvdman_lsn;
	int m_cdvdman_rbuffer;
	int m_cdvdman_nsec;
	int m_cdvdman_csec;
	int m_cdvdman_rsec;
	int m_cdvdman_pattern;
	sceCdRMode m_cdvdman_cdrmode;
	int m_recover_status;
	int m_dvd_flag;
	int m_read_lsn;
	void *m_read_buf;
	int m_read_sectors;
	sceCdRMode m_read_mode;
	int m_read_chunk_reprocial_32;
	int m_dintrlsn;
	void *m_read_callback;
	int m_read_chunk;
	char m_scmd_flag;
	char m_scmd;
	char m_sdlen;
	char m_rdlen;
	char m_scmd_sd[16];
	char m_scmd_rd[16];
	int m_sync_error;
	int m_last_read_timeout;
	int m_power_flag;
	u32 m_layer_1_lsn;
	char m_use_toc;
	char m_opo_or_para;
	char m_current_dvd;
	char m_dual_layer_emulation;
	u32 m_current_dvd_lsn;
	int m_check_version;
	int m_dec_shift;
	int m_dec_state;
	int m_no_dec_flag;
	cdvdman_dma3_parameter_t m_dma3_param;
	int m_cdvdman_dma3sec;
	int m_drive_interupt_request;
	u16 m_dec_mode_set;
	u16 m_dec_mode_last_set;
	int m_waf_set_test;
	// OSD add
	int m_cd_mode_ps2_atapi;
	void (*m_cd_atapi_intr_callback)(int writer_drive);
	int (*m_chgsys_callback)(void *userdata, int writer_drive);
	int m_chgsys_callback_next_disktype;
	int m_unusedosd1;
	int m_field_0DC;
	int m_var_sc_ffffffdb;
	// OSD add end
	int m_interupt_read_state;
	// OSD add
	int m_atapi_disk_ejected;
	// OSD add end
	int m_cd_inited;
	// OSD add
	int m_medium_removal_state;
	int m_chgsys_callback_next_disktype_last;
	int m_chgsys_writer_drive_shell_is_open;
	int m_unusedosd2[6];
	// OSD add end
	int m_tray_is_open;
	int m_break_cdvdfsv_readchain;
	int m_unused[10];
} cdvdman_internal_struct_t;

//IOP-only libcdvd function prototypes.
extern int sceCdCheckCmd(void);
extern int sceCdNop(void);
extern void *sceGetFsvRbuf(void);
extern int sceCdstm0Cb(void (*p)(int));
extern int sceCdstm1Cb(void (*p)(int));
extern int sceCdSC(int code, int *param);
/*	Within all CDVDMAN modules, sceCdReadClock and sceCdRC both exist. In the old one, both have exactly the same code.
	In the newer ones, sceCdReadClock would automatically file off the most significant bit within the month field,
	while sceCdRC continued having its original behaviour of not changing the data.	*/
extern int sceCdRC(sceCdCLOCK *clock);
extern int sceCdRead0(u32 lsn, u32 sectors, void *buffer, sceCdRMode *mode, int csec, void *callback);

/** Reads DVD video.
 * SUPPORTED IN NEWER CDVDMAN MODULES WITHIN DVD PLAYER IOPRP ONLY
 *
 * @return 1 on success, 0 on failure.
 */
extern int sceCdRV(u32 lsn, u32 sectors, void *buf, sceCdRMode *mode, int arg5, void *cb);

/** send an s-command by function number
 * 
 * @param cmdNum command number
 * @param inBuff input buffer  (can be null)
 * @param inBuffSize size of input buffer  (>= 16 bytes)
 * @param outBuff output buffer (can be null)
 * @return 1 on success, 0 on failure.
 */
extern int sceCdApplySCmd2(u8 cmdNum, const void* inBuff, unsigned long int inBuffSize, void *outBuff);

/** send an s-command by function number
 * Unofficial name.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN NEWER IOPRP ONLY
 *
 * @param cmdNum command number
 * @param inBuff input buffer  (can be null)
 * @param inBuffSize size of input buffer  (>= 16 bytes)
 * @param outBuff output buffer (can be null)
 * @return 1 on success, 0 on failure.
 */
extern int sceCdApplySCmd3(u8 cmdNum, const void* inBuff, unsigned long int inBuffSize, void *outBuff);

/** Controls spindle speed? Not sure what it really does.
 * SUPPORTED IN XCDVDMAN ONLY
 *
 * @param speed Speed mode.
 * @return 1 on success, 0 on failure.
 */
extern int sceCdSpinCtrlIOP(u32 speed);

/** Set the eject callback when in ATAPI mode.
 * Unofficial name.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN DNAS IOPRP ONLY
 *
 * @param cb The pointer to the callback
 * @param userdata The pointer to the userdata that will be passed to the callback
 * @return The old callback value
 */
extern void *sceCdSetAtapiEjectCallback(int (*cb)(int reason, void *userdata), void *userdata);

//DNAS functions

/** Reads the Disk ID.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN DNAS IOPRP ONLY
 *
 * @param id integer where the Disk ID is stored.
 * @return 1 on success, 0 on failure.
 */
extern int sceCdReadDiskID(unsigned int *id);

/** Deobfuscate using unique key.
 * Unofficial name.
 * SUPPORTED IN NEWER CDVDMAN MODULES INCLUDED WITHIN DNAS IOPRP ONLY
 *
 * @param buffer Output buffer
 * @param shiftval The amount to rotate left shift
 * @param xorval The value to XOR the buffer against
 * @param status Command status
 * @return 1 on success, 0 on failure.
 */
extern int sceCdDeobfuscateUsingUniqueKey(u8 *buffer, unsigned int shiftval, int xorval, u32 *status);

#define cdvdman_IMPORTS_start DECLARE_IMPORT_TABLE(cdvdman, 1, 1)
#define cdvdman_IMPORTS_end END_IMPORT_TABLE

#define I_sceCdInit DECLARE_IMPORT(4, sceCdInit)
#define I_sceCdStandby DECLARE_IMPORT(5, sceCdStandby)
#define I_sceCdRead DECLARE_IMPORT(6, sceCdRead)
#define I_sceCdSeek DECLARE_IMPORT(7, sceCdSeek)
#define I_sceCdGetError DECLARE_IMPORT(8, sceCdGetError)
#define I_sceCdGetToc DECLARE_IMPORT(9, sceCdGetToc)
#define I_sceCdSearchFile DECLARE_IMPORT(10, sceCdSearchFile)
#define I_sceCdSync DECLARE_IMPORT(11, sceCdSync)
#define I_sceCdGetDiskType DECLARE_IMPORT(12, sceCdGetDiskType)
#define I_sceCdDiskReady DECLARE_IMPORT(13, sceCdDiskReady)
#define I_sceCdTrayReq DECLARE_IMPORT(14, sceCdTrayReq)
#define I_sceCdStop DECLARE_IMPORT(15, sceCdStop)
#define I_sceCdPosToInt DECLARE_IMPORT(16, sceCdPosToInt)
#define I_sceCdIntToPos DECLARE_IMPORT(17, sceCdIntToPos)
#define I_sceCdGetToc2 DECLARE_IMPORT(19, sceCdGetToc2)
#define I_sceCdReadDVDV DECLARE_IMPORT(20, sceCdReadDVDV)
#define I_sceCdCheckCmd DECLARE_IMPORT(21, sceCdCheckCmd)
#define I_sceCdRI DECLARE_IMPORT(22, sceCdRI)
#define I_sceCdWI DECLARE_IMPORT(23, sceCdWI)
#define I_sceCdReadClock DECLARE_IMPORT(24, sceCdReadClock)
#define I_sceCdWriteClock DECLARE_IMPORT(25, sceCdWriteClock)
#define I_sceCdReadNVM DECLARE_IMPORT(26, sceCdReadNVM)
#define I_sceCdWriteNVM DECLARE_IMPORT(27, sceCdWriteNVM)
#define I_sceCdStatus DECLARE_IMPORT(28, sceCdStatus)
#define I_sceCdApplySCmd DECLARE_IMPORT(29, sceCdApplySCmd)
#define I_sceCdSetHDMode DECLARE_IMPORT(30, sceCdSetHDMode)
#define I_sceCdOpenConfig DECLARE_IMPORT(31, sceCdOpenConfig)
#define I_sceCdCloseConfig DECLARE_IMPORT(32, sceCdCloseConfig)
#define I_sceCdReadConfig DECLARE_IMPORT(33, sceCdReadConfig)
#define I_sceCdWriteConfig DECLARE_IMPORT(34, sceCdWriteConfig)
#define I_sceCdReadKey DECLARE_IMPORT(35, sceCdReadKey)
#define I_sceCdDecSet DECLARE_IMPORT(36, sceCdDecSet)
#define I_sceCdCallback DECLARE_IMPORT(37, sceCdCallback)
#define I_sceCdPause DECLARE_IMPORT(38, sceCdPause)
#define I_sceCdBreak DECLARE_IMPORT(39, sceCdBreak)
#define I_sceCdReadCDDA DECLARE_IMPORT(40, sceCdReadCDDA)
#define I_sceCdReadConsoleID DECLARE_IMPORT(41, sceCdReadConsoleID)
#define I_sceCdWriteConsoleID DECLARE_IMPORT(42, sceCdWriteConsoleID)
#define I_sceCdMV DECLARE_IMPORT(43, sceCdMV)
#define I_sceCdGetReadPos DECLARE_IMPORT(44, sceCdGetReadPos)
#define I_sceCdCtrlADout DECLARE_IMPORT(45, sceCdCtrlADout)
#define I_sceCdNop DECLARE_IMPORT(46, sceCdNop)
#define I_sceGetFsvRbuf DECLARE_IMPORT(47, sceGetFsvRbuf)
#define I_sceCdstm0Cb DECLARE_IMPORT(48, sceCdstm0Cb)
#define I_sceCdstm1Cb DECLARE_IMPORT(49, sceCdstm1Cb)
#define I_sceCdSC DECLARE_IMPORT(50, sceCdSC)
#define I_sceCdRC DECLARE_IMPORT(51, sceCdRC)
#define I_sceCdForbidDVDP DECLARE_IMPORT(52, sceCdForbidDVDP)
#define I_sceCdReadSUBQ DECLARE_IMPORT(53, sceCdReadSUBQ)
#define I_sceCdApplyNCmd DECLARE_IMPORT(54, sceCdApplyNCmd)
#define I_sceCdAutoAdjustCtrl DECLARE_IMPORT(55, sceCdAutoAdjustCtrl)
#define I_sceCdStInit DECLARE_IMPORT(56, sceCdStInit)
#define I_sceCdStRead DECLARE_IMPORT(57, sceCdStRead)
#define I_sceCdStSeek DECLARE_IMPORT(58, sceCdStSeek)
#define I_sceCdStStart DECLARE_IMPORT(59, sceCdStStart)
#define I_sceCdStStat DECLARE_IMPORT(60, sceCdStStat)
#define I_sceCdStStop DECLARE_IMPORT(61, sceCdStStop)
#define I_sceCdRead0 DECLARE_IMPORT(62, sceCdRead0)
#define I_sceCdRV DECLARE_IMPORT(63, sceCdRV)
#define I_sceCdRM DECLARE_IMPORT(64, sceCdRM)
#define I_sceCdWM DECLARE_IMPORT(65, sceCdWM)
#define I_sceCdReadChain DECLARE_IMPORT(66, sceCdReadChain)
#define I_sceCdStPause DECLARE_IMPORT(67, sceCdStPause)
#define I_sceCdStResume DECLARE_IMPORT(68, sceCdStResume)
#define I_sceCdForbidRead DECLARE_IMPORT(69, sceCdForbidRead)
#define I_sceCdBootCertify DECLARE_IMPORT(70, sceCdBootCertify)
#define I_sceCdSpinCtrlIOP DECLARE_IMPORT(71, sceCdSpinCtrlIOP)
#define I_sceCdBlueLEDCtl DECLARE_IMPORT(72, sceCdBlueLEDCtl)
#define I_sceCdCancelPOffRdy DECLARE_IMPORT(73, sceCdCancelPOffRdy)
#define I_sceCdPowerOff DECLARE_IMPORT(74, sceCdPowerOff)
#define I_sceCdMmode DECLARE_IMPORT(75, sceCdMmode)

#define I_sceCdReadFull DECLARE_IMPORT(76, sceCdReadFull)
#define I_sceCdStSeekF DECLARE_IMPORT(77, sceCdStSeekF)
#define I_sceCdPOffCallback DECLARE_IMPORT(78, sceCdPOffCallback)
#define I_sceCdReadDiskID DECLARE_IMPORT(79, sceCdReadDiskID)
#define I_sceCdReadGUID DECLARE_IMPORT(80, sceCdReadGUID)
#define I_sceCdSetTimeout DECLARE_IMPORT(81, sceCdSetTimeout)
#define I_sceCdReadModelID DECLARE_IMPORT(82, sceCdReadModelID)
#define I_sceCdReadDvdDualInfo DECLARE_IMPORT(83, sceCdReadDvdDualInfo)
#define I_sceCdLayerSearchFile DECLARE_IMPORT(84, sceCdLayerSearchFile)
#define I_sceCdStatus2 DECLARE_IMPORT(90, sceCdStatus2)
#define I_sceCdReadWakeUpTime DECLARE_IMPORT(109, sceCdReadWakeUpTime)
#define I_sceCdWriteWakeUpTime DECLARE_IMPORT(110, sceCdWriteWakeUpTime)
#define I_sceCdApplySCmd2 DECLARE_IMPORT(112, sceCdApplySCmd2)
#define I_sceCdRE DECLARE_IMPORT(114, sceCdRE)
#define I_sceCdRcBypassCtl DECLARE_IMPORT(115, sceCdRcBypassCtl)
#define I_sceCdSendSCmd1D DECLARE_IMPORT(116, sceCdSendSCmd1D)
#define I_sceRemote2_7 DECLARE_IMPORT(117, sceRemote2_7)
#define I_sceCdSetLEDsMode DECLARE_IMPORT(120, sceCdSetLEDsMode)
#define I_sceCdApplySCmd3 DECLARE_IMPORT(125, sceCdApplySCmd3)
#define I_sceRemote2_7Get DECLARE_IMPORT(128, sceRemote2_7Get)
#define I_sceCdReadPS1BootParam DECLARE_IMPORT(148, sceCdReadPS1BootParam)
#define I_sceCdSetFanProfile DECLARE_IMPORT(150, sceCdSetFanProfile)
#define I_sceCdChgSys DECLARE_IMPORT(154, sceCdChgSys)
#define I_sceCdNoticeGameStart DECLARE_IMPORT(156, sceCdNoticeGameStart)
#define I_sceCdDeobfuscateUsingUniqueKey DECLARE_IMPORT(161, sceCdDeobfuscateUsingUniqueKey)
#define I_sceCdXLEDCtl DECLARE_IMPORT(163, sceCdXLEDCtl)
#define I_sceCdBuzzerCtl DECLARE_IMPORT(165, sceCdBuzzerCtl)
#define I_sceCdXBSPowerCtl DECLARE_IMPORT(171, sceCdXBSPowerCtl)
#define I_sceCdSetAtapiEjectCallback DECLARE_IMPORT(173, sceCdSetAtapiEjectCallback)
#define I_sceCdSetMediumRemoval DECLARE_IMPORT(175, sceCdSetMediumRemoval)
#define I_sceCdGetMediumRemoval DECLARE_IMPORT(177, sceCdGetMediumRemoval)
#define I_sceCdDoesUniqueKeyExist DECLARE_IMPORT(179, sceCdDoesUniqueKeyExist)
#define I_sceCdXDVRPReset DECLARE_IMPORT(181, sceCdXDVRPReset)
#define I_sceCdGetWakeUpReason DECLARE_IMPORT(183, sceCdGetWakeUpReason)
#define I_sceCdReadRegionParams DECLARE_IMPORT(189, sceCdReadRegionParams)
#define I_sceCdWriteRegionParams DECLARE_IMPORT(191, sceCdWriteRegionParams)

#ifdef __cplusplus
}
#endif

#endif /* __CDVDMAN_H__ */
