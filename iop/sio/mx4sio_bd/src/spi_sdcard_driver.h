#ifndef SPI_SDCARD_DRIVER_H
#define SPI_SDCARD_DRIVER_H

#include <bdm.h>
#include <stdint.h>

#define READ_TOKEN_TIMEOUT     4000
#define SECTOR_SIZE            512

#define CMD_ABORT_NO_READ_TOKEN 0x1
#define CMD_ABORT_CRC16_INVALID 0x2
#define CMD_ABORT_WRITE_ERROR   0x3
#define CMD_ABORT_UKNOWN_ERROR  0x7

typedef enum card_type_e {
    CARD_TYPE_MMC    = 0x00,
    CARD_TYPE_SDV1   = 0x01,
    CARD_TYPE_SDV2   = 0x02,
    CARD_TYPE_SDV2HC = 0x04
} card_type_t;

typedef enum spisd_result_s {
    SPISD_RESULT_OK = 0,
    SPISD_RESULT_ERROR,
    SPISD_RESULT_NO_CARD,
    SPISD_RESULT_TIMEOUT,
} spisd_result_t;

/*
 * START: structures copied from wisi's implementation
 */
struct t_csdVer1
{
    u8 reserved1          : 6;
    u8 csd_structure      : 2;
    u8 taac               : 8;
    u8 nsac               : 8;
    u8 tran_speed         : 8;

    u16 cccHi             : 8;
    u16 read_bl_len       : 4;
    u16 cccLo             : 4;

    u32 c_sizeHi          : 2;
    u8 read_bl_partial    : 1;
    u8 write_blk_misalign : 1;
    u8 read_blk_misalign  : 1;
    u8 dsr_imp            : 1;
    u8 reserved2          : 2;

    u32 c_sizeMd          : 8;

    u32 c_sizeLo          : 2;
    u8 vdd_r_curr_max     : 3;
    u8 vdd_r_curr_min     : 3;

    u8 c_size_multHi      : 2;
    u8 vdd_w_curr_max     : 3;
    u8 vdd_w_curr_min     : 3;

    u8 c_size_multLo      : 1;
    // XXX: The folowing are not fixed to correct endianness!
    u8 erase_blk_en       : 1;
    u8 sector_size_high   : 6;
    u8 sector_size_low    : 1;
    u8 wp_grp_size        : 7;
    u8 ep_grp_enable      : 1;
    u8 reserved5          : 2;
    u8 r2w_factor         : 3;
    u8 write_bl_len_high  : 2;
    u8 write_bl_len_low   : 2;
    u8 write_bl_partial   : 1;
    u8 reserved6          : 5;
    u8 file_format_group  : 1;
    u8 copy               : 1;
    u8 perm_write_protect : 1;
    u8 tmp_write_protect  : 1;
    u8 file_format        : 2;
    u8 reserved7          : 2;
    u8 crc                : 7;
    u8 fixedTo1           : 1;
} __attribute__((packed));

struct t_csdVer2
{
    u8 reserved1          : 6;
    u8 csd_structure      : 2;
    u8 taac               : 8;
    u8 nsac               : 8;
    u8 tran_speed         : 8;

    u16 cccHi             : 8;
    u16 read_bl_len       : 4;
    u16 cccLo             : 4;

    // Not fixed!:
    u8 read_bl_partial    : 1;
    u8 write_blk_misalign : 1;
    u8 read_blk_misalign  : 1;
    u8 dsr_imp            : 1;
    u8 reserved2          : 4;

    // fixed:
    u32 c_sizeHi          : 6;
    u8 reserved3          : 2; // I give up - this is far too unaligned...
    // 79:58
    u32 c_sizeMd          : 8;
    u32 c_sizeLo          : 8; // 16 22;

    // Not fixed!:
    u32 reserved4         : 1;
    u8 erase_blk_en       : 1;
    u8 sector_size_high   : 6;
    u8 sector_size_low    : 1;
    u8 wp_grp_size        : 7;
    u8 ep_grp_enable      : 1;
    u8 reserved5          : 2;
    u8 r2w_factor         : 3;
    u8 write_bl_len_high  : 2;
    u8 write_bl_len_low   : 2;
    u8 write_bl_partial   : 1;
    u8 reserved6          : 5;
    u8 file_format_group  : 1;
    u8 copy               : 1;
    u8 perm_write_protect : 1;
    u8 tmp_write_protect  : 1;
    u8 file_format        : 2;
    u8 reserved7          : 2;
    u8 crc                : 7;
    u8 fixedTo1           : 1;
} __attribute__((packed));
/*
 * END: structures copied from wisi's implementation
 */

typedef struct spisd_cid_t
{                           /* Card Identification Data */
    uint8_t ManufacturerID; /* ManufacturerID */
    uint16_t OEM_AppliID;   /* OEM/Application ID */
    uint32_t ProdName1;     /* Product Name part1 */
    uint8_t ProdName2;      /* Product Name part2*/
    uint8_t ProdRev;        /* Product Revision */
    uint32_t ProdSN;        /* Product Serial Number */
    uint8_t Reserved1;      /* Reserved1 */
    uint16_t ManufactDate;  /* Manufacturing Date */
    uint8_t CID_CRC;        /* CID CRC */
}spisd_cid_t;

/* global struct for everything sdcard */
typedef struct spisd_t
{
    uint8_t csd[16];
    spisd_cid_t cid;
    card_type_t card_type;
    uint8_t initialized;
    uint8_t used;
}spisd_t;

/* globals */
extern spisd_t sdcard;
extern struct block_device bd;

uint16_t spisd_read_status_register();

/* init */
int spisd_init_card();   /* bring card up from identification mode to data-transfer mode */
int spisd_get_card_info(); /* get card info and reattach to bdm if card capacity changed */
int spisd_recover();

/* BDM functions */
int spisd_read(struct block_device *bd, uint64_t sector, void *buffer, uint16_t count);
int spisd_write(struct block_device *bd, uint64_t sector, const void *buffer, uint16_t count);
void spisd_flush(struct block_device *bd);
int spisd_stop(struct block_device *bd);


#endif