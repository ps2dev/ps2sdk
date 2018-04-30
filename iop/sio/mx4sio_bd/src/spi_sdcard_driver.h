#ifndef __SPI_SDCARD_DRIVER_H__
#define __SPI_SDCARD_DRIVER_H__

/* Includes ------------------------------------------------------------------*/
#include "spi_sdcard_driver_config.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum spisd_result_s {
    SPISD_RESULT_OK = 0,
    SPISD_RESULT_ERROR,
    SPISD_RESULT_NO_CARD,
    SPISD_RESULT_TIMEOUT,
} spisd_result_t;

#define SPISD_R1_IDLE_FLAG                  (0x01)
#define SPISD_R1_ERASE_RESET_FLAG           (0x02)
#define SPISD_R1_ILLEGAL_CMD_FLAG           (0x04)
#define SPISD_R1_CMD_CRC_FLAG               (0x08)
#define SPISD_R1_ERASE_SEQ_ERROR_FLAG       (0x10)
#define SPISD_R1_ADDR_ERROR_FLAG            (0x20)
#define SPISD_R1_PARAM_ERROR_FLAG           (0x40)
#define SPISD_R1_ZERO_FLAG                  (0x80)

typedef union sppisd_r1_u {

    uint8_t raw;
    struct {
        uint8_t idle: 1;                    /*<!-- The card is in idle state and running the initializing process */
        uint8_t erase_reset: 1;             /*<!-- An erase sequence was cleared before executing because an out of erase sequence command was received */
        uint8_t illegal_cmd: 1;             /*<!-- An illegal command code was detected */
        uint8_t cmd_crc_err: 1;             /*<!-- The CRC check of the last command failed */
        uint8_t erase_seq_err: 1;           /*<!-- An error in the sequence of erase commands occurred */
        uint8_t addr_err: 1;                /*<!-- A misaligned address that did not match the block length was used in the command */
        uint8_t param_error: 1;             /*<!-- The command�s argument (e.g. address, block length) was outside the allowed range for this card */
        uint8_t zero: 1;                    /*<!-- mustbe zero  */
    } fields;
} spisd_r1_t __attribute__((packed));

typedef struct spisd_r1b_s {
    uint8_t is_ready;                           /*<!-- A zero value indicates card is busy. A non-zero value indicates the card is ready for the next command */
} spisd_r1b_t_ __attribute__((packed));

typedef union spisd_r2_s {

    uint16_t raw;
    struct {

        uint16_t card_locked: 1;            /*<!-- Set when the card is locked by the user. Reset when it is unlocked  */
        uint16_t wp_erase_skip: 1;          /*<!-- This status bit has two functions overloaded. It is set when the host attempts to erase a write-protected sector or makes a sequence or password errors during card lock/unlock operation*/
        uint16_t err: 1;                    /*<!-- A general or an unknown error occurred during the operation */
        uint16_t cc_err: 1;                 /*<!-- Internal card controller error */
        uint16_t card_ecc_failed: 1;        /*<!-- Card internal ECC was applied but failed to correct the data */
        uint16_t wp_viol: 1;                /*<!-- The command tried to write a write-protected block */
        uint16_t erase_param: 1;            /*<!-- An invalid selection for erase, sectors or group */
        uint16_t out_of_range: 1;           /*<!-- */
        uint8_t idle: 1;                    /*<!-- The card is in idle state and running the initializing process */
        uint8_t erase_reset: 1;             /*<!-- An erase sequence was cleared before executing because an out of erase sequence command was received */
        uint8_t illegal_cmd: 1;             /*<!-- An illegal command code was detected */
        uint8_t cmd_crc_err: 1;             /*<!-- The CRC check of the last command failed */
        uint8_t erase_seq_err: 1;           /*<!-- An error in the sequence of erase commands occurred */
        uint8_t addr_err: 1;                /*<!-- A misaligned address that did not match the block length was used in the command */
        uint8_t param_error: 1;             /*<!-- The command�s argument (e.g. address, block length) was outside the allowed range for this card */
        uint8_t zero: 1;                    /*<!-- mustbe zero  */
    } fields;
} spisd_r2_t __attribute__((packed));

typedef struct sppisd_r3_s {

    spisd_r1_t r1;
    uint32_t ocr;
} spisd_r3_t __attribute__((packed));

typedef struct sppisd_r7_s {

    spisd_r1_t r1;
    union {
        uint32_t raw32;
        struct {
            uint32_t cmd_ver: 4;
            uint32_t reserved: 16;
            uint32_t voltage_accept: 4;
            uint32_t ehco_back: 8;
        } fields;
    };
} spisd_r7_t __attribute__((packed));

typedef struct {                    /* Card Specific Data */
    uint8_t  CSDStruct;               /* CSD structure */
    uint8_t  SysSpecVersion;          /* System specification version */
    uint8_t  Reserved1;               /* Reserved */
    uint8_t  TAAC;                    /* Data read access-time 1 */
    uint8_t  NSAC;                    /* Data read access-time 2 in CLK cycles */
    uint8_t  MaxBusClkFrec;           /* Max. bus clock frequency */
    uint16_t CardComdClasses;         /* Card command classes */
    uint8_t  RdBlockLen;              /* Max. read data block length */
    uint8_t  PartBlockRead;           /* Partial blocks for read allowed */
    uint8_t  WrBlockMisalign;         /* Write block misalignment */
    uint8_t  RdBlockMisalign;         /* Read block misalignment */
    uint8_t  DSRImpl;                 /* DSR implemented */
    uint8_t  Reserved2;               /* Reserved */
    uint32_t DeviceSize;              /* Device Size */
    uint8_t  MaxRdCurrentVDDMin;      /* Max. read current @ VDD min */
    uint8_t  MaxRdCurrentVDDMax;      /* Max. read current @ VDD max */
    uint8_t  MaxWrCurrentVDDMin;      /* Max. write current @ VDD min */
    uint8_t  MaxWrCurrentVDDMax;      /* Max. write current @ VDD max */
    uint8_t  DeviceSizeMul;           /* Device size multiplier */
    uint8_t  EraseGrSize;             /* Erase group size */
    uint8_t  EraseGrMul;              /* Erase group size multiplier */
    uint8_t  WrProtectGrSize;         /* Write protect group size */
    uint8_t  WrProtectGrEnable;       /* Write protect group enable */
    uint8_t  ManDeflECC;              /* Manufacturer default ECC */
    uint8_t  WrSpeedFact;             /* Write speed factor */
    uint8_t  MaxWrBlockLen;           /* Max. write data block length */
    uint8_t  WriteBlockPaPartial;     /* Partial blocks for write allowed */
    uint8_t  Reserved3;               /* Reserded */
    uint8_t  ContentProtectAppli;     /* Content protection application */
    uint8_t  FileFormatGrouop;        /* File format group */
    uint8_t  CopyFlag;                /* Copy flag (OTP) */
    uint8_t  PermWrProtect;           /* Permanent write protection */
    uint8_t  TempWrProtect;           /* Temporary write protection */
    uint8_t  FileFormat;              /* File Format */
    uint8_t  ECC;                     /* ECC code */
    uint8_t  CSD_CRC;                 /* CSD CRC */
    uint8_t  Reserved4;               /* always 1*/
} spisd_csd_t;

typedef struct {                    /*Card Identification Data*/
    uint8_t  ManufacturerID;          /* ManufacturerID */
    uint16_t OEM_AppliID;             /* OEM/Application ID */
    uint32_t ProdName1;               /* Product Name part1 */
    uint8_t  ProdName2;               /* Product Name part2*/
    uint8_t  ProdRev;                 /* Product Revision */
    uint32_t ProdSN;                  /* Product Serial Number */
    uint8_t  Reserved1;               /* Reserved1 */
    uint16_t ManufactDate;            /* Manufacturing Date */
    uint8_t  CID_CRC;                 /* CID CRC */
    uint8_t  Reserved2;               /* always 1 */
} spisd_cid_t;

typedef struct {
    spisd_csd_t csd;
    spisd_cid_t cid;
    uint32_t capacity;                /* Card Capacity */
    uint32_t block_size;              /* Card Block Size */
    uint16_t rca;
    uint8_t card_type;
    uint32_t space_total;             /* Total space size in file system */
    uint32_t space_free;              /* Free space size in file system */
} spisd_info_t;

typedef struct spisd_interface_s {
    void (*set_speed)(uint32_t freq);
    void (*select)(void);
    void (*relese)(void);
    bool (*is_present)(void);
    uint8_t (*wr_rd_byte)(uint8_t byte);
    void (*write)(uint8_t const *buffer, uint32_t size);
    void (*read)(uint8_t *buffer, uint32_t size);
} spisd_interface_t;


spisd_result_t spisd_init(spisd_interface_t const *const io);
spisd_result_t spisd_read_block(uint32_t sector, uint8_t *buffer);
spisd_result_t spisd_write_block(uint32_t sector, const uint8_t *buffer);

spisd_result_t spisd_read_multi_block_begin(uint32_t sector);
spisd_result_t spisd_read_multi_block_read(uint8_t *buffer, uint32_t num_sectors);
spisd_result_t spisd_read_multi_block_end(void);

spisd_result_t spisd_write_multi_block(uint32_t sector, uint8_t const *buffer, uint32_t num_sectors);

int spisd_get_card_info(spisd_info_t *cardinfo);

#endif //__SPI_SDCARD_DRIVER_H__
