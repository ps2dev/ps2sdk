struct SecrSifDownloadHeaderParams
{
    s32 port, slot;
    u8 buffer[0x400];
    SecrBitTable_t BitTable;
    s32 size;
    s32 result;
};

struct SecrSifDownloadBlockParams
{
    u8 buffer[0x400];
    s32 size;
    s32 result;
};

struct SecrSifDownloadGetKbitParams
{
    s32 port, slot;
    u8 kbit[16];
    s32 result;
};

struct SecrSifDownloadGetKcParams
{
    s32 port, slot;
    u8 kc[16];
    s32 result;
};

struct SecrSifDownloadGetIcvps2Params
{
    u8 icvps2[8];
    s32 result;
};

struct SecrSifDiskBootHeaderParams
{
    u8 buffer[0x400];
    SecrBitTable_t BitTable;
    s32 size;
    s32 result;
};

struct SecrSifDiskBootBlockParams
{
    u8 source[0x400];
    u8 destination[0x400];
    s32 size;
    s32 result;
};

#define SECRSIF_DOWNLOAD_HEADER     0x80000A01
#define SECRSIF_DOWNLOAD_BLOCK      0x80000A02
#define SECRSIF_DOWNLOAD_GET_KBIT   0x80000A03
#define SECRSIF_DOWNLOAD_GET_KC     0x80000A04
#define SECRSIF_DOWNLOAD_GET_ICVPS2 0x80000A05
#define SECRSIF_DISK_BOOT_HEADER    0x80000A06
#define SECRSIF_DISK_BOOT_BLOCK     0x80000A07
