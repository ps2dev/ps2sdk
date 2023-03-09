/* Includes ------------------------------------------------------------------*/
#include "spi_sdcard_driver.h"
#include "spi_sdcard_driver_config.h"
#include "spi_sdcard_crc7.h"

// #define DEBUG  //comment out this line when not debugging
#include "module_debug.h"
#define SPISD_LOG M_PRINTF

/* Private types -------------------------------------------------------------*/
typedef enum card_type_e {
    CARD_TYPE_MMC    = 0x00,
    CARD_TYPE_SDV1   = 0x01,
    CARD_TYPE_SDV2   = 0x02,
    CARD_TYPE_SDV2HC = 0x04
} card_type_t;

/* Private define ------------------------------------------------------------*/

#define DUMMY_BYTE 0xFF
#define BLOCK_SIZE 512

#define CMD0   0  /* Reset */
#define CMD1   1  /* Send Operator Condition - SEND_OP_COND */
#define CMD8   8  /* Send Interface Condition - SEND_IF_COND    */
#define CMD9   9  /* Read CSD */
#define CMD10  10 /* Read CID */
#define CMD12  12 /* Stop data transmit */
#define CMD16  16 /* Set block size, should return 0x00 */
#define CMD17  17 /* Read single block */
#define CMD18  18 /* Read multi block */
#define ACMD23 23 /* Prepare erase N-blokcs before multi block write */
#define CMD24  24 /* Write single block */
#define CMD25  25 /* Write multi block */
#define ACMD41 41 /* should return 0x00 */
#define CMD55  55 /* should return 0x01 */
#define CMD58  58 /* Read OCR */
#define CMD59  59 /* CRC disable/enbale, should return 0x00 */

#define CMD_WAIT_RESP_TIMEOUT (100U)
#define WAIT_IDLE_TIMEOUT     (50U)

#define SPI_SPEED_NO_INIT_HZ (400000U)
#define SPI_SPEED_MAX_HZ     (25000000U)

#define CMD_CRC_OFFSET        5
#define CRC7_SHIFT_MASK(crc7) ((crc7) << 1U | 1U)

/* Private variables ---------------------------------------------------------*/
static card_type_t _card_type;
static spisd_interface_t const *_io = NULL;

static uint8_t _send_command(uint8_t cmd, uint32_t arg)
{
    uint32_t i;
    uint8_t response = 0xFF;

    /* Send 8 CLKs before sending CMD */
    _io->wr_rd_byte(DUMMY_BYTE);
    
    uint8_t packet[]       = {cmd | 0x40, arg >> 24, arg >> 16, arg >> 8, arg, 0};
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
    
    /* When performing block commands, some cards are ready to send the 0xFE token
     * immediately after sending 0x0. By performing a dummy write after recieving 0x0
     * we risk discarding 0xFE these cards causing the operation to fail completely. 
    */

    /* Sidenote: When in SPI Mode CMD9 and CMD10 function like other block commands */

    switch(cmd){
        case CMD9:
        case CMD10:
        case CMD18:
        case CMD24:
        case CMD25:
        break;

        default:
        _io->wr_rd_byte(DUMMY_BYTE);
    }

    return response;
}

static uint8_t _send_command_recv_response(uint8_t cmd, uint32_t arg, uint8_t *data, size_t size)
{
    uint32_t i;

    /* Send 8 CLKs before sending CMD */
    _io->wr_rd_byte(DUMMY_BYTE);
    
    uint8_t packet[]       = {cmd | 0x40, arg >> 24, arg >> 16, arg >> 8, arg, 0};
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

    /* Send 8 CLKs after sending CMD */
    _io->wr_rd_byte(DUMMY_BYTE);

    return response;
}

static uint8_t _send_command_hold(uint8_t cmd, uint32_t arg)
{
    uint32_t i;

    /* Send 8 CLKs before sending CMD */
    _io->wr_rd_byte(DUMMY_BYTE);

    uint8_t packet[]       = {cmd | 0x40, arg >> 24, arg >> 16, arg >> 8, arg, 0};
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

static spisd_result_t _read_buffer(uint8_t *buff, uint32_t len)
{
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

spisd_result_t spisd_init(spisd_interface_t const *const io)
{
    uint32_t i;
    SPISD_ASSERT(io);

    _io = io;

    if (!_io->is_present()) {
        SPISD_LOG("There is no card detected! \r\n");
        return SPISD_RESULT_NO_CARD;
    }

#if CRC7_RAM_TABLE == 1
    crc7_generate_table();
#endif // CRC7_RAM_TABLE == 1

    
    _io->set_speed(SPI_SPEED_NO_INIT_HZ);

    /* Start send 74 CLKs at least */
    for (i = 0; i < 20; i++) {
        _io->wr_rd_byte(DUMMY_BYTE);
    }

    uint32_t timeout = WAIT_IDLE_TIMEOUT;
    uint8_t response = 0;

    do {
        response = _send_command(CMD0, 0);
        timeout--;
    } while ((response != SPISD_R1_IDLE_FLAG) && timeout > 0);

    if (!timeout) {
        // SPISD_LOG("Reset card into IDLE state failed!\r\n");
        return SPISD_RESULT_TIMEOUT;
    }

    /* The response to CMD8 is R7, which is 6 bytes if you include the CRC and other bits
     * Not reading all 6 bytes causes issues on some cards, so its best to just read them all.
     *
     * This buffer is shared with CMD58, fortunately CMD58's response (R3) is also 6 bytes long. 
    */

    uint8_t buff[6];
    response = _send_command_recv_response(CMD8, 0x1AA, buff, sizeof(buff));

    if (response == SPISD_R1_IDLE_FLAG) {

        /* Check voltage range be 2.7-3.6V    */
        if (buff[2] == 0x01 && buff[3] == 0xAA) {

            for (i = 0; i < 0xFFF; i++) {
                response = _send_command(CMD55, 0); /* should be return 0x01 */

                if (response != 0x01) {
                    SPISD_LOG("Send CMD55 should return 0x01, response=0x%02x\r\n", response);
                    return SPISD_RESULT_TIMEOUT;
                }

                response = _send_command(ACMD41, 0x40000000); /* should be return 0x00 */

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

#endif 
    } else {
        SPISD_LOG("Send CMD8 should return 0x01, response=0x%02x\r\n", response);
        return SPISD_RESULT_ERROR;
    }

    return SPISD_RESULT_OK;
}

int spisd_get_card_info(spisd_info_t *cardinfo)
{

    /* Send CMD9, Read CSD */
    uint8_t response = _send_command(CMD9, 0);

    if (response != 0x00) {
        return response;
    }

    /* CMD9 and CMD10 have R2 responses which are 16 bytes long + 2 bytes CRC.
     * The 2 CRC16 bytes are read and discarded by _read_buffer */
    uint8_t temp[16];

    spisd_result_t ret = _read_buffer(cardinfo->csd, sizeof(temp));
    
    /* _read_buffer will only send two dummy bytes for CRC
     * send another 8 clks just to be safe */
    _io->wr_rd_byte(DUMMY_BYTE);

    if (ret != SPISD_RESULT_OK) {
        return 1;
    }

    /* Send CMD10, Read CID */
    response = _send_command(CMD10, 0);

    if (response != 0x00) {
        return response;
    }

    ret = _read_buffer(temp, sizeof(temp));
    
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
    cardinfo->cid.CID_CRC   = (temp[15] & 0xFE) >> 1;
    cardinfo->cid.Reserved2 = 1;

    return 0;
}

spisd_result_t spisd_read_block(uint32_t sector, uint8_t *buffer)
{

    if (_card_type != CARD_TYPE_SDV2HC) {
        sector = sector << 9;
    }

    spisd_result_t ret = SPISD_RESULT_ERROR;

    if (_send_command_hold(CMD17, sector) == 0x00) {
        ret = _read_buffer(buffer, BLOCK_SIZE);
    }

    _io->wr_rd_byte(DUMMY_BYTE);

    return ret;
}

spisd_result_t spisd_write_block(uint32_t sector, const uint8_t *buffer)
{
    uint32_t i;
    spisd_result_t ret = SPISD_RESULT_ERROR;

    if (_card_type != CARD_TYPE_SDV2HC) {
        sector = sector << 9;
    }

    if (_send_command(CMD24, sector) != 0x00) {
        return ret;
    }

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
            if (_io->wr_rd_byte(DUMMY_BYTE) != 0x00) {
                ret = SPISD_RESULT_OK;
                break;
            }
        }
    }

    _io->wr_rd_byte(DUMMY_BYTE);

    return ret;
}

spisd_result_t spisd_read_multi_block_begin(uint32_t sector)
{
    /* if ver = SD2.0 HC, sector need <<9 */
    if (_card_type != CARD_TYPE_SDV2HC) {
        sector = sector << 9;
    }

    if (_send_command(CMD18, sector) != 0x00) {
        return SPISD_RESULT_ERROR;
    }

    return SPISD_RESULT_OK;
}

spisd_result_t spisd_read_multi_block_read(uint8_t *buffer, uint32_t num_sectors)
{
    uint32_t i;

    for (i = 0; i < num_sectors; i++) {
        spisd_result_t ret = _read_buffer(&buffer[i * BLOCK_SIZE], BLOCK_SIZE);

        if (ret != SPISD_RESULT_OK) {
            /* Send stop data transmit command - CMD12    */
            spisd_read_multi_block_end();
            return SPISD_RESULT_ERROR;
        }
    }

    _io->wr_rd_byte(DUMMY_BYTE);

    return SPISD_RESULT_OK;
}

spisd_result_t spisd_read_multi_block_end(void)
{
    /* Send stop data transmit command - CMD12 */
    _send_command(CMD12, 0);

    return SPISD_RESULT_OK;
}

spisd_result_t spisd_write_multi_block(uint32_t sector, uint8_t const *buffer, uint32_t num_sectors)
{
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
    //M_DEBUG("Multiblock write\n");
    if (_send_command(CMD25, sector) != 0x00) {
        return SPISD_RESULT_ERROR;
    }

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
            _io->wr_rd_byte(DUMMY_BYTE);
            return SPISD_RESULT_ERROR;
        }

        /* Wait all the data programm finished    */
        uint32_t timeout = 0;

        while (_io->wr_rd_byte(DUMMY_BYTE) != 0xFF) {
            /* Timeout return */
            if (timeout++ == 0x40000) {
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
                _io->wr_rd_byte(DUMMY_BYTE);

                for (j = 0; j < 0x40000; j++) {
                    if (_io->wr_rd_byte(DUMMY_BYTE) == 0xFF) {
                        return SPISD_RESULT_OK;
                    }
                }
            }
        }
    }

    _io->wr_rd_byte(DUMMY_BYTE);

    return SPISD_RESULT_ERROR;
}
