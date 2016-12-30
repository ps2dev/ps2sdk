#define XFER_BLOCK_SIZE 65024	/* 65536-512. It seems like my Oxford 934 is capable of maximum transfers of less than 65536 bytes.
						NOTE: I reduced the maximum block size here by 512, since transfers are done in (smallest) groups of 512 bytes!
				*/

#define READ_TRANSACTION	0
#define WRITE_TRANSACTION	1

/* Some macros */
#define CDB_PAGE_SIZE(v)		((v) << 16)	/* Not in use. */
#define CDB_PAGE_TABLE_PRESENT(v)	((v) << 19)	/* Not in use. */
#define CDB_MAX_PAYLOAD(v)		((v) << 20)
#define CDB_SPEED(v)			((v) << 24)
#define CDB_DIRECTION(v)		((v) << 27)
#define CDB_DATA_SIZE(v)		((v))

typedef struct _inquiry_data {
    u8 peripheral_device_type;  // 00h - Direct access (Floppy), 1Fh none (no FDD connected)
    u8 removable_media; // 80h - removeable
    u8 iso_ecma_ansi;
    u8 repsonse_data_format;
    u8 additional_length;
    u8 res[3];
    u8 vendor[8];
    u8 product[16];
    u8 revision[4];
} inquiry_data;

typedef struct _sense_data {
    u8 error_code;
    u8 res1;
    u8 sense_key;
    u8 information[4];
    u8 add_sense_len;
    u8 res3[4];
    u8 add_sense_code;
    u8 add_sense_qual;
    u8 res4[4];
} sense_data;

typedef struct _read_capacity_data {
    u32 last_lba;	/* Big endian data. */
    u32 block_length;	/* Big endian data. */
} read_capacity_data;

/* Function prototypes. */
int scsiReadSector(struct SBP2Device *dev, unsigned long int lba, void *buffer, int sectorCount);
int scsiWriteSector(struct SBP2Device *dev, unsigned long int lba, void* buffer, int sectorCount);
void releaseSBP2Device(struct SBP2Device *dev);
int ConfigureSBP2Device(struct SBP2Device *dev);
