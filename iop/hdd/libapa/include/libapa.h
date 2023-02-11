/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __LIBAPA_H__
#define __LIBAPA_H__

#include <types.h>
#include <hdd-ioctl.h>

// Sectors for this and that ;)
#define APA_SECTOR_MBR 0
#ifdef APA_SUPPORT_GPT
#define APA_SECTOR_SECTOR_ERROR 34 // 6 use for last sector that had a error...
#else
// TODO: In DVRP firmware, the following value is 0x1000 when 48-bit
#define APA_SECTOR_SECTOR_ERROR 6 // 6 use for last sector that had a error...
#endif
#define APA_SECTOR_PART_ERROR   APA_SECTOR_SECTOR_ERROR + 1       // 7 use for last partition that had a error...
#define APA_SECTOR_APAL         APA_SECTOR_PART_ERROR + 1         // 8
#define APA_SECTOR_APAL_HEADERS APA_SECTOR_APAL + 2               // 10-262
#define APA_SECTOR_MIN_OSDSTART APA_SECTOR_APAL_HEADERS + 252 + 1 // OSD program cannot overlap the journal

// APA Partition
#define APA_MAGIC       0x00415041 // 'APA\0'
#define APA_MBR_VERSION 2

#define APA_MODVER ((APA_MODVER_MAJOR << 8) | APA_MODVER_MINOR)

typedef struct
{
    u8 unused;
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 month;
    u16 year;
} apa_ps2time_t;

//
// MAIN APA defines/struct
//
typedef struct
{
    u32 start;  // Sector address
    u32 length; // Sector count
} apa_sub_t;

typedef struct
{
    u32 checksum;
    u32 magic; // APA_MAGIC
    u32 next;
    u32 prev;
    char id[APA_IDMAX];
    char rpwd[APA_PASSMAX];
    char fpwd[APA_PASSMAX];
    u32 start;
    u32 length;
    u16 type;
    u16 flags;
    u32 nsub;
    apa_ps2time_t created;
    u32 main;
    u32 number;
    u32 modver;
    u32 pading1[7];
    char pading2[128];
    struct
    {
        char magic[32];
        u32 version;
        u32 nsector;
        apa_ps2time_t created;
        u32 osdStart;
        u32 osdSize;
#ifdef APA_SUPPORT_GPT
        char pading3[128];
        struct
        {
            u32 UniqueMbrSignature;
            u16 Unknown;
            struct
            {
                u8 BootIndicator;
                u8 StartHead;
                u8 StartSector;
                u8 StartTrack;
                u8 OSIndicator;
                u8 EndHead;
                u8 EndSector;
                u8 EndTrack;
                u32 StartingLBA;
                u32 SizeInLBA;
            } partition_record1;
            u8 partition_record234[48];
            u16 Signature;
        } protective_mbr;
#else
        char pading3[200];
#endif
    } mbr;
    apa_sub_t subs[APA_MAXSUB];
} apa_header_t;

#define APA_CACHE_FLAG_DIRTY 0x01
typedef struct sapa_cache
{
    struct sapa_cache *next;
    struct sapa_cache *tail;
    u16 flags;
    u16 nused;
    s32 device;
    u32 sector;
    union
    {
        apa_header_t *header;
        u32 *error_lba;
    };
} apa_cache_t;

typedef struct
{
    char id[APA_IDMAX];
    char fpwd[APA_PASSMAX];
    char rpwd[APA_PASSMAX];
    u32 size;
    u16 type;
    u16 flags;
    u32 main;
    u32 number;
} apa_params_t;

void apaSaveError(s32 device, void *buffer, u32 lba, u32 err_lba);
void apaSetPartErrorSector(s32 device, u32 lba);
int apaGetPartErrorSector(s32 device, u32 lba, u32 *lba_out);
int apaGetPartErrorName(s32 device, char *name);

apa_cache_t *apaFillHeader(s32 device, const apa_params_t *params, u32 start, u32 next, u32 prev, u32 length, int *err);
apa_cache_t *apaInsertPartition(s32 device, const apa_params_t *params, u32 sector, int *err);
apa_cache_t *apaFindPartition(s32 device, const char *id, int *err);
void apaAddEmptyBlock(apa_header_t *header, u32 *emptyBlocks);
apa_cache_t *apaRemovePartition(s32 device, u32 start, u32 next, u32 prev, u32 length);
void apaMakeEmpty(apa_cache_t *clink);
apa_cache_t *apaDeleteFixPrev(apa_cache_t *clink1, int *err);
apa_cache_t *apaDeleteFixNext(apa_cache_t *clink, int *err);
int apaDelete(apa_cache_t *clink);
int apaCheckSum(apa_header_t *header, int fullcheck);
int apaReadHeader(s32 device, apa_header_t *header, u32 lba);
int apaWriteHeader(s32 device, apa_header_t *header, u32 lba);
int apaGetFormat(s32 device, int *format);
u32 apaGetPartitionMax(u32 totalLBA);
apa_cache_t *apaGetNextHeader(apa_cache_t *clink, int *err);

///////////////////////////////////////////////////////////////////////////////

int apaCacheInit(u32 size);
void apaCacheLink(apa_cache_t *clink, apa_cache_t *cnew);
apa_cache_t *apaCacheUnLink(apa_cache_t *clink);
int apaCacheTransfer(apa_cache_t *clink, int type);
void apaCacheFlushDirty(apa_cache_t *clink);
int apaCacheFlushAllDirty(s32 device);
apa_cache_t *apaCacheGetHeader(s32 device, u32 sector, u32 mode, int *err);
void apaCacheFree(apa_cache_t *clink);
apa_cache_t *apaCacheAlloc(void);

///////////////////////////////////////////////////////////////////////////////

#define APAL_MAGIC 0x4150414C // 'APAL'
typedef struct
{
    u32 magic; // APAL_MAGIC
    s32 num;
    u32 sectors[126];
} apa_journal_t;

#define journalCheckSum(header) apaCheckSum((apa_header_t *)header, 1)
int apaJournalReset(s32 device);
int apaJournalFlush(s32 device);
int apaJournalWrite(apa_cache_t *clink);
int apaJournalRestore(s32 device);

///////////////////////////////////////////////////////////////////////////////

void *apaAllocMem(int size);
void apaFreeMem(void *ptr);
int apaGetTime(apa_ps2time_t *tm);
int apaGetIlinkID(u8 *idbuf);

///////////////////////////////////////////////////////////////////////////////
int apaPassCmp(const char *password1, const char *password2);
void apaEncryptPassword(const char *id, char *password_out, const char *password_in);

///////////////////////////////////////////////////////////////////////////////
typedef struct
{
    u32 totalLBA;
    u32 partitionMaxSize;
    int format;
    int status;
} apa_device_t;

int apaGetFreeSectors(s32 device, u32 *free, apa_device_t *deviceinfo);

#endif /* __LIBAPA_H__ */
