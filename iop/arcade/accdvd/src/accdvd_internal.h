/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACCDVD_INTERNAL_H
#define _ACCDVD_INTERNAL_H

#include <accdvd.h>
#include <irx_imports.h>

typedef acUint32 acd_lsn_t;

struct acd;

typedef void (*acd_done_t)(struct acd *acd, void *arg, int ret);

struct acd
{
	acAtapiData c_atapi;
	acd_done_t c_done;
	void *c_arg;
	acInt32 c_thid;
	acInt32 c_tmout;
};

typedef void (*cdc_done_t)(int eveid);

struct cdc_read_stru
{
	acUint8 *buf;
	acInt32 size;
	acInt32 bsize;
	acInt32 pos;
	acInt32 bank;
	acd_lsn_t lsn;
	cdc_xfer_t xfer;
	acInt16 spindle;
	acUint16 maxspeed;
};

struct cdc_stream_stru
{
	acUint8 *buf;
	acInt32 size;
	acInt32 bsize;
	acInt32 head;
	acInt32 tail;
	acd_lsn_t lsn;
	acUint32 reqlsn;
	acInt32 flag;
};

struct cdc_softc
{
	acInt32 lockid;
	acInt32 syncid;
	acInt32 error;
	acCdvdsifId fno;
	acUint8 *buf;
	cdc_done_t done;
	acUint32 cdsize;
	acUint16 tray;
	acUint16 stat;
	struct cdc_read_stru rd;
	struct cdc_stream_stru st;
	struct acd acd;
};

struct iso9660_desc
{
	// cppcheck-suppress unusedStructMember
	unsigned char type[1];
	// cppcheck-suppress unusedStructMember
	unsigned char id[5];
	// cppcheck-suppress unusedStructMember
	unsigned char version[1];
	// cppcheck-suppress unusedStructMember
	unsigned char unused1[1];
	// cppcheck-suppress unusedStructMember
	unsigned char system_id[32];
	// cppcheck-suppress unusedStructMember
	unsigned char volume_id[32];
	// cppcheck-suppress unusedStructMember
	unsigned char unused2[8];
	// cppcheck-suppress unusedStructMember
	unsigned char volume_space_size[8];
	// cppcheck-suppress unusedStructMember
	unsigned char unused3[32];
	// cppcheck-suppress unusedStructMember
	unsigned char volume_set_size[4];
	// cppcheck-suppress unusedStructMember
	unsigned char volume_sequence_number[4];
	// cppcheck-suppress unusedStructMember
	unsigned char logical_block_size[4];
	// cppcheck-suppress unusedStructMember
	unsigned char path_table_size[8];
	// cppcheck-suppress unusedStructMember
	unsigned char type_l_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char opt_type_l_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char type_m_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char opt_type_m_path_table[4];
	// cppcheck-suppress unusedStructMember
	unsigned char root_directory_record[34];
	// cppcheck-suppress unusedStructMember
	unsigned char volume_set_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char publisher_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char preparer_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char application_id[128];
	// cppcheck-suppress unusedStructMember
	unsigned char copyright_file_id[37];
	// cppcheck-suppress unusedStructMember
	unsigned char abstract_file_id[37];
	// cppcheck-suppress unusedStructMember
	unsigned char bibliographic_file_id[37];
	// cppcheck-suppress unusedStructMember
	unsigned char creation_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char modification_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char expiration_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char effective_date[17];
	// cppcheck-suppress unusedStructMember
	unsigned char file_structure_version[1];
	// cppcheck-suppress unusedStructMember
	unsigned char unused4[1];
	// cppcheck-suppress unusedStructMember
	unsigned char application_data[512];
	// cppcheck-suppress unusedStructMember
	unsigned char unused5[653];
};

struct iso9660_dirent
{
	unsigned char length[1];
	unsigned char ext_attr_length[1];
	unsigned char extent[8];
	unsigned char size[8];
	unsigned char date[7];
	unsigned char flags[1];
	unsigned char file_unit_size[1];
	unsigned char interleave[1];
	unsigned char volume_sequence_number[4];
	unsigned char name_len[1];
	unsigned char name[];
};

struct iso9660_path
{
	unsigned char name_len[2];
	unsigned char extent[4];
	unsigned char parent[2];
	unsigned char name[];
};

struct cdfs_ptable
{
	struct iso9660_path *path;
	acUint32 size;
};

struct cdfs_softc
{
	acInt32 semid;
	acUint32 all;
	struct cdfs_ptable *ptable;
	acInt32 ptnum;
	acUint32 rootlsn;
	acUint32 rootsize;
	acInt32 rootidx;
	struct iso9660_dirent *dcache;
	acUint32 dclsn;
	acUint32 dcsize;
	struct iso9660_path *pcache;
	acUint32 pcsize;
	// cppcheck-suppress unusedStructMember
	acInt32 padding[4];
	acUint8 buf[2048];
};

struct atapi_mode_h
{
	acUint8 h_len[2];
	acUint8 h_mtype;
	// cppcheck-suppress unusedStructMember
	acUint8 h_nblocks;
	// cppcheck-suppress unusedStructMember
	acUint8 h_padding[4];
};

struct atapi_mode
{
	// cppcheck-suppress unusedStructMember
	acUint8 d_pgcode;
	// cppcheck-suppress unusedStructMember
	acUint8 d_pglen;
};

struct atapi_mode_error
{
	struct atapi_mode_h me_h;
	// cppcheck-suppress unusedStructMember
	struct atapi_mode me_d;
	// cppcheck-suppress unusedStructMember
	acUint8 me_param;
	acUint8 me_rretry;
	// cppcheck-suppress unusedStructMember
	acUint8 me_unused1[4];
	// cppcheck-suppress unusedStructMember
	acUint8 me_wretry;
	// cppcheck-suppress unusedStructMember
	acUint8 me_unused2[3];
};

struct atapi_mode_drive
{
	struct atapi_mode_h md_h;
	// cppcheck-suppress unusedStructMember
	struct atapi_mode md_d;
	// cppcheck-suppress unusedStructMember
	acUint8 md_unused;
	acUint8 md_timer;
	// cppcheck-suppress unusedStructMember
	acUint8 md_spm[2];
	// cppcheck-suppress unusedStructMember
	acUint8 md_fps[2];
};

struct atapi_mode_capmach
{
	struct atapi_mode_h mc_h;
	// cppcheck-suppress unusedStructMember
	struct atapi_mode mc_d;
	// cppcheck-suppress unusedStructMember
	acUint8 mc_mer;
	// cppcheck-suppress unusedStructMember
	acUint8 mc_padding;
	// cppcheck-suppress unusedStructMember
	acUint8 mc_cap[4];
	acUint8 mc_maxspeed[2];
	// cppcheck-suppress unusedStructMember
	acUint8 mc_novl[2];
	// cppcheck-suppress unusedStructMember
	acUint8 mc_bufsize[2];
	acUint8 mc_speed[2];
	// cppcheck-suppress unusedStructMember
	acUint8 mc_padding2;
	// cppcheck-suppress unusedStructMember
	acUint8 mc_flags2;
	// cppcheck-suppress unusedStructMember
	acUint8 mc_padding3[2];
};

struct atapi_read_capacity
{
	acUint8 lba[4];
	// cppcheck-suppress unusedStructMember
	acUint8 blen[4];
};

struct acd_softc
{
	acInt32 active;
	acUint32 status;
	acUint32 drive;
	acCdvdsifId dma;
	acCdvdsifId medium;
	acInt32 sense;
	acUint32 dmamap;
	acUint32 padding;
	struct atapi_mode_drive timer;
	struct atapi_mode_error retry;
	struct atapi_mode_capmach speed;
};

struct cdfs_time
{
	acUint8 t_padding;
	acUint8 t_sec;
	acUint8 t_min;
	acUint8 t_hour;
	acUint8 t_day;
	acUint8 t_mon;
	acUint16 t_year;
};

struct cdfs_dirent
{
	acUint32 d_lsn;
	acUint32 d_size;
	acUint32 d_vol;
	// cppcheck-suppress unusedStructMember
	acUint32 d_padding;
	acUint8 d_ftype;
	acUint8 d_namlen;
	char d_name[14];
	struct cdfs_time d_time;
};

struct cdfs_file
{
	acUint32 f_lsn;
	acUint32 f_pos;
	acUint32 f_size;
	acUint32 f_padding;
};

struct cdvd_modules
{
	int (*cm_restart)(int argc, char **argv);
	int (*cm_start)(int argc, char **argv);
	int (*cm_status)();
	int (*cm_stop)();
};

struct cdc_module
{
	int (*start)(int argc, char **argv);
	int (*stop)();
	int (*status)();
};

struct acd_ata
{
	acAtaData a_ata;
	acInt32 a_thid;
	acInt32 a_result;
	acUint32 a_padding[2];
};

struct cdi_softc
{
	// cppcheck-suppress unusedStructMember
	acInt32 error;
	cdc_done_t done;
	// cppcheck-suppress unusedStructMember
	acUint32 padding[2];
};

extern struct acd *acd_setup(struct acd *acd, acd_done_t done, void *arg, int tmout);
extern int acd_module_status();
extern int acd_module_start(int argc, char **argv);
extern int acd_module_stop();
extern int acd_module_restart(int argc, char **argv);
extern int cddrv_module_start(int argc, char **argv);
extern int cddrv_module_stop();
extern int cddrv_module_restart(int argc, char **argv);
extern int cddrv_module_status();
extern int cdfs_umount();
extern int cdfs_recover(int ret);
extern int cdfs_lookup(struct cdfs_dirent *result, const char *path, int pathlen);
extern int cdfs_read(struct cdfs_file *file, void *buf, int size);
extern int cdfs_module_status();
extern int cdfs_module_start(int argc, char **argv);
extern int cdfs_module_stop();
extern int cdfs_module_restart(int argc, char **argv);

extern int acd_ready(struct acd *acd);
extern int acd_readcapacity();
extern int acd_delay();
extern int acd_getmedium(struct acd *acd);
extern acCdvdsifId acd_gettray();
extern int acd_getstatus();
extern int acd_readtoc(struct acd *acd, void *buf, int size);
extern int acd_seek(struct acd *acd, acd_lsn_t lsn);
extern int acd_ioctl(struct acd *acd, int cmd);
extern int acd_getspeed(struct acd *acd, int maxspeed);
extern int acd_setspeed(struct acd *acd, int speed);
extern int acd_setretry(struct acd *acd, int rretry);
extern int acd_getretry(struct acd *acd);
extern int acd_read(struct acd *acd, acd_lsn_t lsn, void *buf, int sectors);

#endif
