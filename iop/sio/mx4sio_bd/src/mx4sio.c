#include <bdm.h>
#include <dmacman.h>
#include <intrman.h>
#include <loadcore.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#include <xsio2man.h>

#include "ioplib.h"
#include "sio2man_hook.h"
#include "sio2regs.h"
#include "spi_sdcard_driver.h"

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

IRX_ID("mx4sio", 1, 1);

#define WELCOME_STR "mx4sio v1.1\n"

// 3 is the second memory card slot
#define PORT_NR 3
#define MAX_SIO2_TRANSFER_SIZE 256 // 0x100
#define SECTOR_SIZE 512

/* Event flags */
#define EF_SIO2_INTR_REVERSE	0x00000100
#define EF_SIO2_INTR_COMPLETE	0x00000200

static int event_flag = -1;
static int sd_detect_thread_id = -1;
static u32 sio2man_save_crtl;
static int card_used = 0;
static int card_inserted = 0;
static int driver_state = 0;
// Fast SPI clock
static int fastDiv = 0;
// Slave/Chip Select
static int spi_ss = 0;
// SIO tranfer data, only 1 transfer at a time possible
static sio2_transfer_data_t global_td;

struct dma_command {
    uint8_t* buffer;
    u16 sector_count;
    u16 sectors_transferred;
    u16 sectors_reversed;
    u16 portNr;
    uint8_t response;
};
struct dma_command cmd;

static uint8_t wait_equal(uint8_t value, int count, int portNr);
static void sendCmd_Rx_DMA_start(uint8_t* rdBufA, int portNr);
static inline void reverseBuffer32(u32* buffer, u32 count);

int sio2_intr_handler(void* arg)
{
    int ef = *(int *)arg;
    int eflags = EF_SIO2_INTR_REVERSE;

    // Clear interrupt?
    sio2_stat_set(sio2_stat_get());

    // Wait for completion
    while ((sio2_stat6c_get() & (1 << 12)) == 0)
        ;

    // Finish sector read, read 2 dummy crc bytes
    sio2_data_in();
    sio2_data_in();
    cmd.sectors_transferred++;

    if (cmd.sectors_transferred < cmd.sector_count) {
        // Start next DMA transfer ASAP
        cmd.response = wait_equal(0xFE, 200, cmd.portNr);
        if (cmd.response == SPISD_RESULT_OK)
            sendCmd_Rx_DMA_start(&cmd.buffer[cmd.sectors_transferred * SECTOR_SIZE], cmd.portNr);
    }

    if ((cmd.sectors_transferred >= cmd.sector_count) || (cmd.response != SPISD_RESULT_OK)) {
        // Done or error, notify user task
        eflags |= EF_SIO2_INTR_COMPLETE;
    }

    iSetEventFlag(ef, eflags);

    return 1;
}

static unsigned char reverseByte_LUT8_table[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static inline u8 reverseByte_LUT8(u8 n)
{
    return reverseByte_LUT8_table[n];
}

static u32 reverseByteInWord_mask(u32 n, u32 maskF, u32 mask3, u32 mask5)
{
    n = ((n & (maskF << 4)) >> 4) | ((n & maskF) << 4);
    n = ((n & (mask3 << 2)) >> 2) | ((n & mask3) << 2);
    n = ((n & (mask5 << 1)) >> 1) | ((n & mask5) << 1);
    return n;
}

static inline u32 reverseByteInWord(u32 n)
{
    return reverseByteInWord_mask(n, 0x0F0F0F0F, 0x33333333, 0x55555555);
}

/* The masks need to be defined as registers outside of the loop
 * This extra function makes sure gcc 3.2.3 generates an efficient function
 * Inner loop with 19 instructions:
        $L38:
            and	$2,$2,$10
            sll	$3,$3,4
            srl	$2,$2,4
            or	$2,$2,$3
            and	$4,$2,$7
            and	$2,$2,$9
            srl	$2,$2,2
            sll	$4,$4,2
            or	$2,$2,$4
            and	$3,$2,$12
            and	$2,$2,$5
            srl	$2,$2,1
            sll	$3,$3,1
            or	$2,$2,$3
            sw	$2,0($8)
            addu	$8,$8,4
            lw	$2,0($8)
            bne	$8,$11,$L38
            and	$3,$2,$6
*/
static void reverseBuffer32_mask(u32* buffer, u32 count, u32 maskF, u32 mask3, u32 mask5)
{
    u32 n = *buffer;
    u32* end = buffer + count;

    while (buffer != end) {
        n = ((n & (maskF <<4)) >>4) | ((n & maskF) <<4);
        n = ((n & (mask3 <<2)) >>2) | ((n & mask3) <<2);
        n = ((n & (mask5 <<1)) >>1) | ((n & mask5) <<1);

        *buffer = n;
        buffer++;
        n = *buffer;
    }
}

//  7037KB/s (FAT  PS2, GCC 3.2.3, -O3)
// 12052KB/s (slim PS2, GCC 3.2.3, -O3)
static inline void reverseBuffer32(u32* buffer, u32 count)
{
    reverseBuffer32_mask(buffer, count, 0x0F0F0F0F, 0x33333333, 0x55555555);
}

// 4194KB/s (FAT  PS2, GCC 3.2.3, -O3)
// 3771KB/s (slim PS2, GCC 3.2.3, -O3)
static inline void reverseBuffer8x1_LUT(u8* buffer, u32 count)
{
    u8* end = buffer + count;

    while (buffer != end) {
        *buffer = reverseByte_LUT8(*buffer);
        buffer++;
    }
}

static void _init_td(sio2_transfer_data_t* td, int portNr)
{
    static const uint8_t TxByte = 0xff;
    static uint8_t RxByte;
    int i;

    /*
     * Clock divider for 48MHz clock:
     * 1 = 48  MHz - Damaged data
     * 2 = 24  MHz - Fastest usable speed
     * 3 = 16  MHz
     * 4 = 12  MHz
     * 5 =  9.6MHz
     * 6 =  6  MHz
     * ...
     * 0x78 = 400KHz - Initialization speed
     */
    const int slowDivVal   = 0x78;
    const int fastDivVal   = 2;
    const int interBytePer = 0; //2;

    for (i = 0; i < 4; i++) {
        td->port_ctrl1[i] = 0;
        td->port_ctrl2[i] = 0;
    }

    td->port_ctrl1[portNr] =
        PCTRL0_ATT_LOW_PER(0x5) |
        PCTRL0_ATT_MIN_HIGH_PER(0x5) |
        PCTRL0_BAUD0_DIV(slowDivVal) |
        PCTRL0_BAUD1_DIV(fastDivVal);

    td->port_ctrl2[portNr] =
        PCTRL1_ACK_TIMEOUT_PER(0x12C) |
        PCTRL1_INTER_BYTE_PER(interBytePer) |
        PCTRL1_UNK24(0) |
        PCTRL1_IF_MODE_SPI_DIFF(0);

    // create dummy transfer to unlock old rom0:SIO2MAN
    // - Tx 1 byte PIO
    // - Rx 1 byte PIO
    td->regdata[0] =
        TR_CTRL_PORT_NR(portNr) |
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(0) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_TX_DATA_SZ(1) |
        TR_CTRL_RX_DATA_SZ(1) |
        TR_CTRL_BAUD_DIV(fastDiv) |
        TR_CTRL_WAIT_ACK_FOREVER(0);
    td->regdata[1] = 0;

    // Tx 1 byte PIO
    td->in_size      = 1;
    td->in           = (void*)&TxByte;
    td->in_dma.addr  = NULL;
    td->in_dma.size  = 0;
    td->in_dma.count = 0;

    // Rx 1 byte PIO
    td->out_size      = 1;
    td->out           = &RxByte;
    td->out_dma.addr  = NULL;
    td->out_dma.size  = 0;
    td->out_dma.count = 0;
}

static void _init_ports(sio2_transfer_data_t* td)
{
    int i;

    for (i = 0; i < 4; i++) {
        sio2_portN_ctrl1_set(i, td->port_ctrl1[i]);
        sio2_portN_ctrl2_set(i, td->port_ctrl2[i]);
    }
}

static uint8_t sendCmd_Tx1_Rx1(uint8_t data, int portNr)
{
    sio2_ctrl_set(0x0bc); // no interrupt

    sio2_regN_set(0,
        TR_CTRL_PORT_NR(portNr) |
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(0) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_BAUD_DIV(fastDiv) |
        TR_CTRL_WAIT_ACK_FOREVER(0) |
        TR_CTRL_TX_DATA_SZ(1) |
        TR_CTRL_RX_DATA_SZ(1));
    sio2_regN_set(1, 0);

    // Put byte in queue
    sio2_data_out(reverseByte_LUT8(data));

    // Start queue exec
    sio2_ctrl_set(sio2_ctrl_get() | 1);

    // Wait for completion
    while ((sio2_stat6c_get() & (1 << 12)) == 0)
        ;

    // Get byte from queue
    return reverseByte_LUT8(sio2_data_in());
}

static void sendCmd_Tx_PIO(const uint8_t* wrBufA, int sndSz, int portNr)
{
    sio2_ctrl_set(0x0bc); // no interrupt

    sio2_regN_set(0,
        TR_CTRL_PORT_NR(portNr) |
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(0) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_BAUD_DIV(fastDiv) |
        TR_CTRL_WAIT_ACK_FOREVER(0) |
        TR_CTRL_TX_DATA_SZ(sndSz) |
        TR_CTRL_RX_DATA_SZ(0));
    sio2_regN_set(1, 0);

    // PIO: IOP -> SIO2
    // Fill the queue
    while (sndSz--)
        sio2_data_out(reverseByte_LUT8(*wrBufA++));

    // Start queue exec
    sio2_ctrl_set(sio2_ctrl_get() | 1);

    // Wait for completion
    while ((sio2_stat6c_get() & (1 << 12)) == 0)
        ;
}

static void sendCmd_Rx_PIO(uint8_t* rdBufA, int rcvSz, int portNr)
{
    sio2_ctrl_set(0x0bc); // no interrupt

    sio2_regN_set(0,
        TR_CTRL_PORT_NR(portNr) |
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(0) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_BAUD_DIV(fastDiv) |
        TR_CTRL_WAIT_ACK_FOREVER(0) |
        TR_CTRL_TX_DATA_SZ(0) |
        TR_CTRL_RX_DATA_SZ(rcvSz));
    sio2_regN_set(1, 0);

    // Start queue exec
    sio2_ctrl_set(sio2_ctrl_get() | 1);

    // Wait for completion
    while ((sio2_stat6c_get() & (1 << 12)) == 0)
        ;

    // PIO: IOP <- SIO2
    // Empty the queue
    while (rcvSz--)
        *rdBufA++ = reverseByte_LUT8(sio2_data_in());
}

static uint8_t wait_equal(uint8_t value, int count, int portNr)
{
    uint32_t i;
    uint8_t response = 0;

    for (i = 0; i < count; i++) {
        response = sendCmd_Tx1_Rx1(0xff, portNr);
        if (response == value)
            break;
    }

    return (response != value) ? SPISD_RESULT_ERROR : SPISD_RESULT_OK;
}

static void sendCmd_Rx_DMA_start(uint8_t* rdBufA, int portNr)
{
    const uint32_t trSettingsDMA =
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(1) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_TX_DATA_SZ(0) |
        TR_CTRL_RX_DATA_SZ(0x100) |
        TR_CTRL_BAUD_DIV(1) |
        TR_CTRL_WAIT_ACK_FOREVER(0);

    const uint32_t trSettingsPIO =
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(0) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_TX_DATA_SZ(0) |
        TR_CTRL_RX_DATA_SZ(2) |
        TR_CTRL_BAUD_DIV(1) |
        TR_CTRL_WAIT_ACK_FOREVER(0);

    sio2_ctrl_set(0x0bc); // no interrupt

    sio2_regN_set(0, trSettingsDMA | TR_CTRL_PORT_NR(portNr));
    sio2_regN_set(1, trSettingsDMA | TR_CTRL_PORT_NR(portNr));
    sio2_regN_set(2, trSettingsPIO | TR_CTRL_PORT_NR(portNr));
    sio2_regN_set(3, 0);

    dmac_request(IOP_DMAC_SIO2out, rdBufA, 0x100 >> 2, 2, DMAC_TO_MEM);
    dmac_transfer(IOP_DMAC_SIO2out);

    // Start queue exec
    sio2_ctrl_set(sio2_ctrl_get() | 1);
}

static void sio2_lock()
{
    int state;

    //M_DEBUG("%s()\n", __FUNCTION__);

    // Lock sio2man driver so we can use it exclusively
    sio2man_hook_sio2_lock();

    // Save ctrl state
    sio2man_save_crtl = sio2_ctrl_get();

    // We're in control, setup the ports for our use
    _init_ports(&global_td);

    // Enable DMA interrupts
    CpuSuspendIntr(&state);
    RegisterIntrHandler(IOP_IRQ_DMA_SIO2_OUT, 1, sio2_intr_handler, &event_flag);
    EnableIntr(IOP_IRQ_DMA_SIO2_OUT);
    CpuResumeIntr(state);
}

static void sio2_unlock()
{
    int state;
    int res;

    // Disable DMA interrupts
    CpuSuspendIntr(&state);
    DisableIntr(IOP_IRQ_DMA_SIO2_OUT, &res);
    CpuResumeIntr(state);

    // Restore ctrl state
    sio2_ctrl_set(sio2man_save_crtl);

    // Unlock sio2man driver
    sio2man_hook_sio2_unlock();

    //M_DEBUG("%s()\n", __FUNCTION__);
}

/*
 * SPI interface:
 * - "spi_sdcard" library -> SPI
 */
static void ps2_spi_set_speed(uint32_t freq)
{
    int newFastDiv = freq > 400000;

    //M_DEBUG("%s(%d)\n", __FUNCTION__, (int)freq);

    if (fastDiv != newFastDiv) {
        M_DEBUG("%s(%d)\n", __FUNCTION__, (int)freq);

        fastDiv = newFastDiv;
        _init_td(&global_td, PORT_NR);
        _init_ports(&global_td);
    }
}

static void ps2_spi_select(void)
{
    //M_DEBUG("%s()\n", __FUNCTION__);
    spi_ss = 1;
}

static void ps2_spi_relese(void)
{
    //M_DEBUG("%s()\n", __FUNCTION__);
    spi_ss = 0;
}

static bool ps2_spi_is_present(void)
{
    //M_DEBUG("%s()\n", __FUNCTION__);
    return true;
}

static uint8_t ps2_spi_wr_rd_byte(uint8_t byte)
{
    if (spi_ss == 0) {
        // We're ignoring a lot of dummy read/writes, this is not an error
        //M_DEBUG("%s(%d) - ignoring, not selected\n", __FUNCTION__, byte);
        return 0;
    }

    //M_DEBUG("%s(%d)\n", __FUNCTION__, byte);
    return sendCmd_Tx1_Rx1(byte, PORT_NR);
}

static void ps2_spi_write(uint8_t const* buffer, uint32_t size)
{
    uint32_t ts;

    if (spi_ss == 0) {
        M_DEBUG("%s(..., %d) - ignoring, not selected\n", __FUNCTION__, (int)size);
        return;
    }

    //M_DEBUG("%s(..., %d)\n", __FUNCTION__, (int)size);
    while (size > 0) {
        ts = size > MAX_SIO2_TRANSFER_SIZE ? MAX_SIO2_TRANSFER_SIZE : size;
        sendCmd_Tx_PIO(buffer, ts, PORT_NR);
        buffer += ts;
        size -= ts;
    }
}

static void ps2_spi_read(uint8_t* buffer, uint32_t size)
{
    uint32_t ts;

    if (spi_ss == 0) {
        M_DEBUG("%s(..., %d) - ignoring, not selected\n", __FUNCTION__, (int)size);
        return;
    }

    //M_DEBUG("%s(..., %d)\n", __FUNCTION__, (int)size);
    while (size > 0) {
        ts = size > MAX_SIO2_TRANSFER_SIZE ? MAX_SIO2_TRANSFER_SIZE : size;
        sendCmd_Rx_PIO((uint8_t*)buffer, ts, PORT_NR);
        buffer += ts;
        size -= ts;
    }
}

static spisd_interface_t spi = {
    ps2_spi_set_speed,
    ps2_spi_select,
    ps2_spi_relese,
    ps2_spi_is_present,
    ps2_spi_wr_rd_byte,
    ps2_spi_write,
    ps2_spi_read
};

static void error_recovery()
{
    int rv, i;

    // Flush 4KiB
    for (i = 0; i < 1024; i++)
        sendCmd_Rx_PIO((void*)&rv, 4, PORT_NR);

    spisd_init(&spi);
}

/*
 * BDM interface:
 * - BDM -> "spi_sdcard" library
 */
static int spi_sdcard_read(struct block_device* bd, u32 sector, void* buffer, u16 count)
{
    int rv, i;

    //M_DEBUG("%s(%d,%d)\n", __FUNCTION__, (int)sector, (int)count);

    if (count == 0)
        return 0;

    sio2_lock();

    driver_state = 100;

    for (i = 0; i < 10; i++) {
        // Wait idle
        rv = wait_equal(0xFF, 4000, PORT_NR);
        if (rv != SPISD_RESULT_OK)
            rv = wait_equal(0xFF, 4000, PORT_NR);
        if (rv != SPISD_RESULT_OK) {
            M_PRINTF("ERROR: card is not idle after 2 tries\n");
            goto recovery;
        }

        // Send multi block read command
        rv = spisd_read_multi_block_begin(sector);
        if (rv != SPISD_RESULT_OK) {
            M_PRINTF("ERROR: failed to start multi-block read\n");
            goto recovery;
        }

        // Wait for first start token (first one takes a long time)
        rv = wait_equal(0xFE, 100000, PORT_NR);
        if (rv != SPISD_RESULT_OK)
            rv = wait_equal(0xFE, 100000, PORT_NR);
        if (rv != SPISD_RESULT_OK) {
            M_PRINTF("ERROR: no start token after 2 tries\n");
            goto recovery;
        }
        break;

recovery:
        error_recovery();
    }

    if (rv != SPISD_RESULT_OK) {
        M_PRINTF("ERROR: failed to start multi-block read after 10 tries\n");
        sio2_unlock();
        return 0;
    }

    // Setup DMA cmd struct
    cmd.buffer              = buffer;
    cmd.portNr              = PORT_NR;
    cmd.sector_count        = count;
    cmd.sectors_transferred = 0;
    cmd.sectors_reversed    = 0;
    cmd.response            = SPISD_RESULT_OK;

    driver_state = 101;

    // Start first DMA transfer (1 sector)
    sendCmd_Rx_DMA_start(buffer, PORT_NR);

    // Process events from DMA completion interrupt
    while (1) {
        uint32_t resbits[4];

        driver_state = 102;
        WaitEventFlag(event_flag, EF_SIO2_INTR_REVERSE | EF_SIO2_INTR_COMPLETE, 1, resbits);

        if (resbits[0] & EF_SIO2_INTR_REVERSE) {
            ClearEventFlag(event_flag, ~EF_SIO2_INTR_REVERSE);
            while (cmd.sectors_reversed < cmd.sectors_transferred) {
                reverseBuffer32((u32*)&cmd.buffer[cmd.sectors_reversed * SECTOR_SIZE], SECTOR_SIZE / 4);
                cmd.sectors_reversed++;
            }
        }

        if (resbits[0] & EF_SIO2_INTR_COMPLETE) {
            ClearEventFlag(event_flag, ~EF_SIO2_INTR_COMPLETE);
            if (cmd.response != SPISD_RESULT_OK) {
                M_PRINTF("ERROR: (isr)wait_equal(0xFE)\n");
                sio2_unlock();
                return cmd.sectors_reversed;
            }
            break;
        }
    }

    driver_state = 103;

    rv = spisd_read_multi_block_end();
    if (rv != SPISD_RESULT_OK) {
        M_PRINTF("ERROR: spisd_read_multi_block_end = %d\n", rv);
        sio2_unlock();
        return 0;
    }

    // Let detection thread know the card has been used succesfully
    card_used = 1;

    driver_state = 104;
    sio2_unlock();

    driver_state = 199;
    return count;
}

static int spi_sdcard_write(struct block_device* bd, u32 sector, const void* buffer, u16 count)
{
    int rv;

    //M_DEBUG("%s(%d,%d)\n", __FUNCTION__, (int)sector, (int)count);

    sio2_lock();

    rv = spisd_write_multi_block(sector, buffer, count);
    if (rv != SPISD_RESULT_OK) {
        M_PRINTF("ERROR: spisd_write_multi_block = %d\n", rv);
        sio2_unlock();
        return 0;
    }

    // Let detection thread know the card has been used succesfully
    card_used = 1;

    sio2_unlock();

    return count;
}

static void spi_sdcard_flush(struct block_device* bd)
{
    return;
}

static int spi_sdcard_stop(struct block_device* bd)
{
    return 0;
}

// BDM interface
static struct block_device bd = {
    NULL,        /* priv */
    "sdc",       /* name */
    0,           /* devNr */
    0,           /* parNr */
    SECTOR_SIZE, /* sectorSize */
    0,           /* sectorOffset */
    0,           /* sectorCount */
    spi_sdcard_read,
    spi_sdcard_write,
    spi_sdcard_flush,
    spi_sdcard_stop
};

static void sd_detect()
{
    spisd_info_t cardinfo;
    int rv = SPISD_RESULT_ERROR;

    //M_DEBUG("%s\n", __FUNCTION__);

    // Detect card
    if (card_inserted == 0) {
        // Try to detect a card by initializing
        rv = spisd_init(&spi);
    } else {
        // Try to detect card removal by sending a dummy command
        rv = spisd_read_multi_block_begin(0);
        if (rv == SPISD_RESULT_OK)
            rv = spisd_read_multi_block_end();
    }

    // Change state
    if ((card_inserted == 0) && (rv == SPISD_RESULT_OK)) {
        M_PRINTF("card insertion detected\n");
        card_inserted = 1;
        driver_state  = 1;

        rv = spisd_get_card_info(&cardinfo);

        if (rv != 0) {
            M_PRINTF("ERROR: spisd_get_card_info returned %d\n", rv);
            return;
        }

        // FIXME: cardinfo.csd.DeviceSize * 1024
        bd.sectorCount = 16 * 1024 * 1024 * 2;

        M_PRINTF("%u %u-byte logical blocks: (%uMB / %uMiB)\n", bd.sectorCount, bd.sectorSize, bd.sectorCount / ((1000 * 1000) / bd.sectorSize), bd.sectorCount / ((1024 * 1024) / bd.sectorSize));

        // Connect to block device manager
        bdm_connect_bd(&bd);
    } else if ((card_inserted == 1) && (rv != SPISD_RESULT_OK)) {
        M_PRINTF("card removal detected\n");
        card_inserted = 0;
        driver_state  = 0;

        // Disconnect from block device manager
        bdm_disconnect_bd(&bd);
    }
}

// SD card detection thread
static void sd_detect_thread(void* arg)
{
    M_PRINTF("card detection thread running\n");

    while (1) {
        // Sleep for 1 second
        DelayThread(1000 * 1000);

        M_DEBUG("Check card, used = %d, state = %d\n", card_used, driver_state);

        // Detect card if it has not been used recently
        if (card_used == 0) {
            sio2_lock();
            sd_detect();
            card_used = 0;
            sio2_unlock();
        }
    }
}

/*
 * Module start/stop
 */
int module_start(int argc, char* argv[])
{
    iop_library_t* lib_modload;
    iop_event_t event;
    iop_thread_t thread;
    int i, rv;

    M_PRINTF("Starting module\n");
    for (i = 0; i < argc; i++)
        M_PRINTF(" - argv[%d] = %s\n", i, argv[i]);

    // Create default tranfer descriptor
    _init_td(&global_td, PORT_NR);

    // Create event flag
    event.attr   = 2;
    event.option = 0;
    event.bits   = 0;
    rv = event_flag = CreateEventFlag(&event);
    if (rv < 0) {
        M_PRINTF("ERROR: CreateEventFlag returned %d\n", rv);
        goto error1;
    }

    rv = sio2man_hook_init();
    if (rv < 0) {
        M_PRINTF("ERROR: sio2man_hook_init returned %d\n", rv);
        goto error2;
    }

    // Just in case sio2man was not loaded, we initialize the dmac channels too
    dmac_ch_set_dpcr(IOP_DMAC_SIO2in, 3);
    dmac_ch_set_dpcr(IOP_DMAC_SIO2out, 3);
    dmac_enable(IOP_DMAC_SIO2in);
    dmac_enable(IOP_DMAC_SIO2out);

    /* After a reboot the SD will always respond with:
     * - 0xff 0xff 0xc1 0x3f
     * - followed by an infinite amount of 0xff
     * Also after a reboot during transfer (OPL IGR for instance) the SD card
     * will have many bytes to send. This is to flush these bytes so the card
     * always works */
    sio2_lock();
    for (i = 0; i < 1024; i++)
        sendCmd_Rx_PIO((void*)&rv, 4, PORT_NR);
    sio2_unlock();

    // Creat SD card detection thread
    thread.attr      = TH_C;
    thread.thread    = sd_detect_thread;
    thread.option    = 0;
    thread.priority  = USER_LOWEST_PRIORITY;
    thread.stacksize = 0x1000; // 4KiB
    rv = sd_detect_thread_id = CreateThread(&thread);
    if (rv < 0) {
        M_PRINTF("ERROR: CreateThread returned %d\n", rv);
        goto error3;
    }

    // Start thread
    rv = StartThread(sd_detect_thread_id, NULL);
    if (rv < 0) {
        M_PRINTF("ERROR: StartThread returned %d\n", rv);
        goto error4;
    }

    M_DEBUG("Loaded succesfully!\n");

    lib_modload = ioplib_getByName("modload");
    if (lib_modload != NULL) {
        M_DEBUG("modload 0x%x detected\n", lib_modload->version);
        // Newer modload versions allow modules to be unloaded
        // Let modload know we support unloading
        if (lib_modload->version > 0x102)
            return MODULE_REMOVABLE_END;
    } else {
        M_DEBUG("modload not detected!\n");
    }

    return MODULE_RESIDENT_END;

error4:
    DeleteThread(sd_detect_thread_id);
error3:
    sio2man_hook_deinit();
error2:
    DeleteEventFlag(event_flag);
error1:
    return MODULE_NO_RESIDENT_END;
}

int module_stop(int argc, char* argv[])
{
    int i;

    M_PRINTF("Stopping module\n");
    for (i = 0; i < argc; i++)
        M_PRINTF(" - argv[%d] = %s\n", i, argv[i]);

    DeleteThread(sd_detect_thread_id);
    sio2man_hook_deinit();
    DeleteEventFlag(event_flag);

    return MODULE_NO_RESIDENT_END;
}

int _start(int argc, char* argv[])
{
    M_PRINTF(WELCOME_STR);

    if (argc >= 0)
        return module_start(argc, argv);
    else
        return module_stop(-argc, argv);
}
