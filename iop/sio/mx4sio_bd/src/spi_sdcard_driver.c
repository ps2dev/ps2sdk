/* Includes ------------------------------------------------------------------*/
#include "spi_sdcard_driver.h"
#include "spi_sdcard_driver_config.h"
#include "spi_sdcard_crc7.h"

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"
#define SPISD_LOG M_PRINTF

/* Private types -------------------------------------------------------------*/
typedef enum card_type_e {
    CARD_TYPE_MMC = 0x00,
    CARD_TYPE_SDV1 = 0x01,
    CARD_TYPE_SDV2 = 0x02,
    CARD_TYPE_SDV2HC = 0x04
} card_type_t;

/* Private define ------------------------------------------------------------*/

#define DUMMY_BYTE                  0xFF
#define BLOCK_SIZE                  512

#define CMD0                        0       /* Reset */
#define CMD1                        1       /* Send Operator Condition - SEND_OP_COND */
#define CMD8                        8       /* Send Interface Condition - SEND_IF_COND    */
#define CMD9                        9       /* Read CSD */
#define CMD10                       10      /* Read CID */
#define CMD12                       12      /* Stop data transmit */
#define CMD16                       16      /* Set block size, should return 0x00 */
#define CMD17                       17      /* Read single block */
#define CMD18                       18      /* Read multi block */
#define ACMD23                      23      /* Prepare erase N-blokcs before multi block write */
#define CMD24                       24      /* Write single block */
#define CMD25                       25      /* Write multi block */
#define ACMD41                      41      /* should return 0x00 */
#define CMD55                       55      /* should return 0x01 */
#define CMD58                       58      /* Read OCR */
#define CMD59                       59      /* CRC disable/enbale, should return 0x00 */

#define CMD_WAIT_RESP_TIMEOUT       (100U)
#define WAIT_IDLE_TIMEOUT           (50U)

#define SPI_SPEED_NO_INIT_HZ        (400000U)
#define SPI_SPEED_MAX_HZ            (25000000U)

#define CMD_CRC_OFFSET              5
#define CRC7_SHIFT_MASK(crc7)       ((crc7) << 1U | 1U)

/* Private variables ---------------------------------------------------------*/
static card_type_t _card_type;
static spisd_interface_t const*_io = NULL;

static uint8_t _send_command(uint8_t cmd, uint32_t arg) {
    uint32_t i;
    uint8_t response = 0xFF;

    /* Dummy byte and chip enable */
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->select();

    uint8_t packet[] = {cmd | 0x40, arg >> 24, arg >> 16, arg >> 8, arg, 0};
    packet[CMD_CRC_OFFSET] = CRC7_SHIFT_MASK(crc7(packet, CMD_CRC_OFFSET));

    _io->write(packet, sizeof(packet));

    /* Discard stuff byte for CMD12 */
    if (cmd == CMD12) {
        _io->wr_rd_byte(DUMMY_BYTE);
    }

    /* Wait response, quit till timeout */
    for (i = 0; i < CMD_WAIT_RESP_TIMEOUT; i++) {
        response = _io->wr_rd_byte(DUMMY_BYTE);

        if (response != 0xFF) {
            break;
        }
    }

    /* Chip disable and dummy byte */
    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    return response;
}

static uint8_t _send_command_recv_response(uint8_t cmd, uint32_t arg, uint8_t* data, size_t size) {
    uint32_t i;

    /* Dummy byte and chip enable */
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->select();

    uint8_t packet[] = {cmd | 0x40, arg >> 24, arg >> 16, arg >> 8, arg, 0};
    packet[CMD_CRC_OFFSET] = CRC7_SHIFT_MASK(crc7(packet, CMD_CRC_OFFSET));
    _io->write(packet, sizeof(packet));

    uint8_t response = DUMMY_BYTE;

    /* Wait response, quit till timeout */
    for (i = 0; i < CMD_WAIT_RESP_TIMEOUT; i++) {

        response = _io->wr_rd_byte(DUMMY_BYTE);

        if (response != DUMMY_BYTE) {
            _io->read(data, size);
            break;
        }
    }

    /* Chip disable and dummy byte */
    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    return response;
}

static uint8_t _send_command_hold(uint8_t cmd, uint32_t arg) {
    uint32_t i;

    /* Dummy byte and chip enable */
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->select();

    uint8_t packet[] = {cmd | 0x40, arg >> 24, arg >> 16, arg >> 8, arg, 0};
    packet[CMD_CRC_OFFSET] = CRC7_SHIFT_MASK(crc7(packet, CMD_CRC_OFFSET));
    _io->write(packet, sizeof(packet));


    /* Wait response, quit till timeout */
    for (i = 0; i < 200; i++) {

        uint8_t response = _io->wr_rd_byte(DUMMY_BYTE);

        if (response != 0xFF) {
            return response;
        }
    }

    return 0xFF;
}

static spisd_result_t _read_buffer(uint8_t *buff, uint32_t len) {
    uint32_t i;
    uint8_t response = 0;

    /* Wait start-token 0xFE */
    for (i = 0; i < 2000; i++) {
        response = _io->wr_rd_byte(DUMMY_BYTE);

        if (response == 0xFE) {
            break;
        }
    }

    if (response != 0xFE) {

        return SPISD_RESULT_ERROR;
    }

    _io->read(buff, len);

    /* 2bytes dummy CRC */
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->wr_rd_byte(DUMMY_BYTE);

    return SPISD_RESULT_OK;
}

spisd_result_t spisd_init(spisd_interface_t const *const io) {
    uint32_t i;
    SPISD_ASSERT(io);

    _io = io;

    if ( !_io->is_present() ) {
        SPISD_LOG("There is no card detected! \r\n");
        return SPISD_RESULT_NO_CARD;
    }

#if CRC7_RAM_TABLE == 1
    crc7_generate_table();
#endif //CRC7_RAM_TABLE == 1

    _io->relese();
    _io->set_speed(SPI_SPEED_NO_INIT_HZ);

    /* Start send 74 clocks at least */
    for (i = 0; i < 20; i++) {
        _io->wr_rd_byte(DUMMY_BYTE);
    }

    uint32_t timeout = WAIT_IDLE_TIMEOUT;
    uint8_t response = 0;

    do {
        response = _send_command(CMD0, 0);
        timeout--;
    } while ((response != SPISD_R1_IDLE_FLAG) && timeout > 0 );

    if (!timeout) {
        //SPISD_LOG("Reset card into IDLE state failed!\r\n");
        return SPISD_RESULT_TIMEOUT;
    }

    uint8_t buff[4];
    response = _send_command_recv_response(CMD8, 0x1AA, buff, sizeof(buff));

    if (response == SPISD_R1_IDLE_FLAG) {

        /* Check voltage range be 2.7-3.6V    */
        if (buff[2] == 0x01 && buff[3] == 0xAA) {

            for (i = 0; i < 0xFFF; i++) {
                response = _send_command(CMD55, 0);            /* should be return 0x01 */

                if (response != 0x01) {
                    SPISD_LOG("Send CMD55 should return 0x01, response=0x%02x\r\n", response);
                    return SPISD_RESULT_TIMEOUT;
                }

                response = _send_command(ACMD41, 0x40000000);    /* should be return 0x00 */

                if (response == 0x00) {
                    break;
                }
            }

            if (response != 0x00) {
                SPISD_LOG("Send ACMD41 should return 0x00, response=0x%02x\r\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            /* Read OCR by CMD58 */
            response = _send_command_recv_response(CMD58, 0, buff, sizeof(buff));

            if (response != 0x00) {
                SPISD_LOG("Send CMD58 should return 0x00, response=0x%02x\r\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            /* OCR -> CCS(bit30)  1: SDV2HC     0: SDV2 */
            _card_type = (buff[0] & 0x40) ? CARD_TYPE_SDV2HC : CARD_TYPE_SDV2;

            _io->set_speed(SPI_SPEED_MAX_HZ);
        }

#if USE_MMC_CARD == 1
    } else if (response & SPISD_R1_ILLEGAL_CMD_FLAG) {

        _card_type = CARD_TYPE_SDV1;

        /* End of CMD8, chip disable and dummy byte */
        _io->relese();
        _io->wr_rd_byte(DUMMY_BYTE);

        /* SD1.0/MMC start initialize */
        /* Send CMD55+ACMD41, No-response is a MMC card, otherwise is a SD1.0 card */
        for (i = 0; i < 0xFFF; i++) {
            response = _send_command(CMD55, 0);

            if (response != 0x01) {
                SPISD_LOG("Send CMD55 should return 0x01, response=0x%02x\r\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            response = _send_command(ACMD41, 0);

            if (response == 0x00) {
                break;
            }
        }

        /* MMC card initialize start */
        if (response != 0x00) {
            for (i = 0; i < 0xFFF; i++) {
                response = _send_command(CMD1, 0);

                if (response == 0x00) {
                    break;
                }
            }

            /* Timeout return */
            if (response != 0x00) {
                SPISD_LOG("Send CMD1 should return 0x00, response=0x%02x\r\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            _card_type = CARD_TYPE_MMC;
            SPISD_LOG("Card Type : MMC\r\n");
        } else {
            SPISD_LOG("Card Type : SD V1\r\n");
        }

        _io->set_speed(true);

        /* CRC disable */
        response = _send_command(CMD59, 0);

        if (response != 0x00) {
            SPISD_LOG("Send CMD59 should return 0x00, response=0x%02x\r\n", response);
            return SPISD_RESULT_TIMEOUT;
        }

        /* Set the block size */
        response = _send_command(CMD16, BLOCK_SIZE);

        if (response != 0x00) {
            SPISD_LOG("Send CMD16 should return 0x00, response=0x%02x\r\n", response);
            return SPISD_RESULT_TIMEOUT;
        }

#endif //
    } else {
        SPISD_LOG("Send CMD8 should return 0x01, response=0x%02x\r\n", response);
        return SPISD_RESULT_ERROR;
    }

    return SPISD_RESULT_OK;
}

int spisd_get_card_info(spisd_info_t *cardinfo) {

    _io->wr_rd_byte(DUMMY_BYTE);

    /* Send CMD9, Read CSD */
    uint8_t response = _send_command(CMD9, 0);

    if (response != 0x00) {
        return response;
    }

    uint8_t temp[16];

    _io->select();

    spisd_result_t ret = _read_buffer(temp, sizeof(temp));
    /* chip disable and dummy byte */
    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    if (ret != SPISD_RESULT_OK) {
        return 1;
    }

    /* Byte 0 */
    cardinfo->csd.CSDStruct = (temp[0] & 0xC0) >> 6;
    cardinfo->csd.SysSpecVersion = (temp[0] & 0x3C) >> 2;
    cardinfo->csd.Reserved1 = temp[0] & 0x03;
    /* Byte 1 */
    cardinfo->csd.TAAC = temp[1] ;
    /* Byte 2 */
    cardinfo->csd.NSAC = temp[2];
    /* Byte 3 */
    cardinfo->csd.MaxBusClkFrec = temp[3];
    /* Byte 4 */
    cardinfo->csd.CardComdClasses = temp[4] << 4;
    /* Byte 5 */
    cardinfo->csd.CardComdClasses |= (temp[5] & 0xF0) >> 4;
    cardinfo->csd.RdBlockLen = temp[5] & 0x0F;
    /* Byte 6 */
    cardinfo->csd.PartBlockRead = (temp[6] & 0x80) >> 7;
    cardinfo->csd.WrBlockMisalign = (temp[6] & 0x40) >> 6;
    cardinfo->csd.RdBlockMisalign = (temp[6] & 0x20) >> 5;
    cardinfo->csd.DSRImpl = (temp[6] & 0x10) >> 4;
    cardinfo->csd.Reserved2 = 0; /* Reserved */
    cardinfo->csd.DeviceSize = (temp[6] & 0x03) << 10;
    /* Byte 7 */
    cardinfo->csd.DeviceSize |= (temp[7]) << 2;
    /* Byte 8 */
    cardinfo->csd.DeviceSize |= (temp[8] & 0xC0) >> 6;
    cardinfo->csd.MaxRdCurrentVDDMin = (temp[8] & 0x38) >> 3;
    cardinfo->csd.MaxRdCurrentVDDMax = (temp[8] & 0x07);
    /* Byte 9 */
    cardinfo->csd.MaxWrCurrentVDDMin = (temp[9] & 0xE0) >> 5;
    cardinfo->csd.MaxWrCurrentVDDMax = (temp[9] & 0x1C) >> 2;
    cardinfo->csd.DeviceSizeMul = (temp[9] & 0x03) << 1;
    /* Byte 10 */
    cardinfo->csd.DeviceSizeMul |= (temp[10] & 0x80) >> 7;
    cardinfo->csd.EraseGrSize = (temp[10] & 0x7C) >> 2;
    cardinfo->csd.EraseGrMul = (temp[10] & 0x03) << 3;
    /* Byte 11 */
    cardinfo->csd.EraseGrMul |= (temp[11] & 0xE0) >> 5;
    cardinfo->csd.WrProtectGrSize = (temp[11] & 0x1F);
    /* Byte 12 */
    cardinfo->csd.WrProtectGrEnable = (temp[12] & 0x80) >> 7;
    cardinfo->csd.ManDeflECC = (temp[12] & 0x60) >> 5;
    cardinfo->csd.WrSpeedFact = (temp[12] & 0x1C) >> 2;
    cardinfo->csd.MaxWrBlockLen = (temp[12] & 0x03) << 2;
    /* Byte 13 */
    cardinfo->csd.MaxWrBlockLen |= (temp[13] & 0xc0) >> 6;
    cardinfo->csd.WriteBlockPaPartial = (temp[13] & 0x20) >> 5;
    cardinfo->csd.Reserved3 = 0;
    cardinfo->csd.ContentProtectAppli = (temp[13] & 0x01);
    /* Byte 14 */
    cardinfo->csd.FileFormatGrouop = (temp[14] & 0x80) >> 7;
    cardinfo->csd.CopyFlag = (temp[14] & 0x40) >> 6;
    cardinfo->csd.PermWrProtect = (temp[14] & 0x20) >> 5;
    cardinfo->csd.TempWrProtect = (temp[14] & 0x10) >> 4;
    cardinfo->csd.FileFormat = (temp[14] & 0x0C) >> 2;
    cardinfo->csd.ECC = (temp[14] & 0x03);
    /* Byte 15 */
    cardinfo->csd.CSD_CRC = (temp[15] & 0xFE) >> 1;
    cardinfo->csd.Reserved4 = 1;

    if (cardinfo->card_type == CARD_TYPE_SDV2HC) {
        /* Byte 7 */
        cardinfo->csd.DeviceSize = (uint16_t)(temp[8]) * 256;
        /* Byte 8 */
        cardinfo->csd.DeviceSize += temp[9] ;
    }

    cardinfo->capacity = cardinfo->csd.DeviceSize * BLOCK_SIZE * 1024;
    cardinfo->block_size = BLOCK_SIZE;

    /* Send CMD10, Read CID */
    response = _send_command(CMD10, 0);

    if (response != 0x00) {
        return response;
    }


    _io->select();
    ret = _read_buffer(temp, sizeof(temp));
    /* chip disable and dummy byte */
    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    if (ret != SPISD_RESULT_OK) {
        return 2;
    }


    /* Byte 0 */
    cardinfo->cid.ManufacturerID = temp[0];
    /* Byte 1 */
    cardinfo->cid.OEM_AppliID = temp[1] << 8;
    /* Byte 2 */
    cardinfo->cid.OEM_AppliID |= temp[2];
    /* Byte 3 */
    cardinfo->cid.ProdName1 = temp[3] << 24;
    /* Byte 4 */
    cardinfo->cid.ProdName1 |= temp[4] << 16;
    /* Byte 5 */
    cardinfo->cid.ProdName1 |= temp[5] << 8;
    /* Byte 6 */
    cardinfo->cid.ProdName1 |= temp[6];
    /* Byte 7 */
    cardinfo->cid.ProdName2 = temp[7];
    /* Byte 8 */
    cardinfo->cid.ProdRev = temp[8];
    /* Byte 9 */
    cardinfo->cid.ProdSN = temp[9] << 24;
    /* Byte 10 */
    cardinfo->cid.ProdSN |= temp[10] << 16;
    /* Byte 11 */
    cardinfo->cid.ProdSN |= temp[11] << 8;
    /* Byte 12 */
    cardinfo->cid.ProdSN |= temp[12];
    /* Byte 13 */
    cardinfo->cid.Reserved1 |= (temp[13] & 0xF0) >> 4;
    /* Byte 14 */
    cardinfo->cid.ManufactDate = (temp[13] & 0x0F) << 8;
    /* Byte 15 */
    cardinfo->cid.ManufactDate |= temp[14];
    /* Byte 16 */
    cardinfo->cid.CID_CRC = (temp[15] & 0xFE) >> 1;
    cardinfo->cid.Reserved2 = 1;

    return 0;
}

spisd_result_t spisd_read_block(uint32_t sector, uint8_t *buffer) {

    if (_card_type != CARD_TYPE_SDV2HC) {
        sector = sector << 9;
    }

    spisd_result_t ret = SPISD_RESULT_ERROR;

    if (_send_command_hold(CMD17, sector) == 0x00) {

        ret = _read_buffer(buffer, BLOCK_SIZE);
    }

    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    return ret;
}

spisd_result_t spisd_write_block(uint32_t sector, const uint8_t *buffer) {
    uint32_t i;
    spisd_result_t ret = SPISD_RESULT_ERROR;

    if (_card_type != CARD_TYPE_SDV2HC) {
        sector = sector << 9;
    }

    if (_send_command(CMD24, sector) != 0x00) {
        return ret;
    }

    _io->select();

    _io->wr_rd_byte(DUMMY_BYTE);
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->wr_rd_byte(DUMMY_BYTE);

    /* Start data write token: 0xFE */
    _io->wr_rd_byte(0xFE);

    _io->write(buffer, BLOCK_SIZE);

    /* 2Bytes dummy CRC */
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->wr_rd_byte(DUMMY_BYTE);

    /* MSD card accept the data */
    uint8_t response = _io->wr_rd_byte(DUMMY_BYTE);

    if ((response & 0x1F) == 0x05) {

        /* Wait all the data programm finished */
        for (i = 0; i < 0x40000; i++) {
            if ( _io->wr_rd_byte(DUMMY_BYTE) != 0x00) {
                ret = SPISD_RESULT_OK;
                break;
            }
        }
    }

    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    return ret;
}

spisd_result_t spisd_read_multi_block_begin(uint32_t sector) {

    /* if ver = SD2.0 HC, sector need <<9 */
    if (_card_type != CARD_TYPE_SDV2HC) {
        sector = sector << 9;
    }

    if (_send_command(CMD18, sector) != 0x00) {
        return SPISD_RESULT_ERROR;
    }

    return SPISD_RESULT_OK;
}

spisd_result_t spisd_read_multi_block_read(uint8_t *buffer, uint32_t num_sectors) {
    uint32_t i;
    _io->wr_rd_byte(DUMMY_BYTE);    //todo it
    _io->select();

    for (i = 0; i < num_sectors; i++) {

        spisd_result_t ret = _read_buffer(&buffer[i * BLOCK_SIZE], BLOCK_SIZE);

        if (ret != SPISD_RESULT_OK) {
            /* Send stop data transmit command - CMD12    */
            spisd_read_multi_block_end();
            return SPISD_RESULT_ERROR;
        }
    }

    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    return SPISD_RESULT_OK;
}

spisd_result_t spisd_read_multi_block_end(void) {

    /* Send stop data transmit command - CMD12 */
    _send_command(CMD12, 0);

    return SPISD_RESULT_OK;
}

spisd_result_t spisd_write_multi_block(uint32_t sector, uint8_t const *buffer, uint32_t num_sectors) {
    uint32_t i, j;
    /* if ver = SD2.0 HC, sector need <<9 */
    if (_card_type != CARD_TYPE_SDV2HC) {
        sector = sector << 9;
    }

    /* Send command ACMD23 berfore multi write if is not a MMC card */
    if (_card_type != CARD_TYPE_MMC) {
        _send_command(CMD55, 0);
        _send_command(ACMD23, num_sectors);
    }

    if (_send_command(CMD25, sector) != 0x00) {
        return SPISD_RESULT_ERROR;
    }

    _io->select();
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->wr_rd_byte(DUMMY_BYTE);
    _io->wr_rd_byte(DUMMY_BYTE);

    for (i = 0; i < num_sectors; i++) {

        /* Start multi block write token: 0xFC */
        _io->wr_rd_byte(0xFC);

        _io->write(&buffer[i * BLOCK_SIZE], BLOCK_SIZE);

        /* 2Bytes dummy CRC */
        _io->wr_rd_byte(DUMMY_BYTE);
        _io->wr_rd_byte(DUMMY_BYTE);

        /* MSD card accept the data */
        if ((_io->wr_rd_byte(DUMMY_BYTE) & 0x1F) != 0x05) {
            _io->relese();
            _io->wr_rd_byte(DUMMY_BYTE);
            return SPISD_RESULT_ERROR;
        }

        /* Wait all the data programm finished    */
        uint32_t timeout = 0;

        while (_io->wr_rd_byte(DUMMY_BYTE) != 0xFF) {
            /* Timeout return */
            if (timeout++ == 0x40000) {
                _io->relese();
                _io->wr_rd_byte(DUMMY_BYTE);
                return SPISD_RESULT_ERROR;
            }
        }
    }

    /* Send end of transmit token: 0xFD */
    if (_io->wr_rd_byte(0xFD) != 0x00) {

        _io->wr_rd_byte(DUMMY_BYTE);
        _io->wr_rd_byte(DUMMY_BYTE);

        /* Wait all the data programm finished */
        for (i = 0; i < 0x40000; i++) {
            if (_io->wr_rd_byte(DUMMY_BYTE) == 0xFF) {
                _io->relese();
                _io->wr_rd_byte(DUMMY_BYTE);

                for (j = 0; j < 0x40000; j++) {
                    if (_io->wr_rd_byte(DUMMY_BYTE) == 0xFF) {
                        return SPISD_RESULT_OK;
                    }
                }
            }
        }
    }

    _io->relese();
    _io->wr_rd_byte(DUMMY_BYTE);

    return SPISD_RESULT_ERROR;
}
