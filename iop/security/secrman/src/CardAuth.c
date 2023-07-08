#include <stdio.h>
#include <sysclib.h>

#include "secrman.h"

#include "main.h"
#include "CardAuth.h"

static McCommandHandler_t McCommandHandler; // 0x00003d10
static McDevIDHandler_t McDevIDHandler;     // 0x00003d14

#if 0
static void _printf4(const char *format, ...);
#endif

// 0x00001f60
#if 0
static void _printf4(const char *format, ...){

}
#endif

// 0x00001fd0
void ResetMcHandlers(void)
{
    McCommandHandler = NULL;
    McDevIDHandler   = NULL;
}

// 0x00002a00
void SetMcCommandHandler(McCommandHandler_t handler)
{
    McCommandHandler = handler;
}

// 0x00002a10
McCommandHandler_t GetMcCommandHandler(void)
{
    return McCommandHandler;
}

// 0x00002a20
void SetMcDevIDHandler(McDevIDHandler_t handler)
{
    McDevIDHandler = handler;
}

// 0x00002a30
McDevIDHandler_t GetMcDevIDHandler(void)
{
    return McDevIDHandler;
}

// 0x00002a40
int McDeviceIDToCNum(int port, int slot)
{
    if (McDevIDHandler == NULL) {
        return -1;
    }

    return McDevIDHandler(port, slot);
}

// 0x00001f78
int SendMcCommand(int port, int slot, sio2_transfer_data_t *sio2packet)
{
    return McCommandHandler(port, slot, sio2packet);
}

// 0x00001fa0
unsigned char calculate_sio2_buffer_checksum(const void *buffer, unsigned int length)
{
    unsigned int i, checksum;

    for (checksum = 0, i = 0; i < length; i++)
        checksum ^= ((const unsigned char *)buffer)[i];

    return checksum;
}

// 0x00002814
int card_auth_write(int port, int slot, const void *buffer, int arg3, int command)
{
    int i;
    sio2_transfer_data_t sio2packet;
    unsigned char wrbuf[14], rdbuf[14];

    sio2packet.in  = wrbuf;
    sio2packet.out = rdbuf;
    memset(sio2packet.port_ctrl1, 0, sizeof(sio2packet.port_ctrl1));
    memset(sio2packet.port_ctrl2, 0, sizeof(sio2packet.port_ctrl2));
    sio2packet.port_ctrl1[port] = 0xFF060505;
    sio2packet.port_ctrl2[port] = 0x0003FFFF;
    sio2packet.regdata[0]       = (port & 3) | 0x00380E40;
    sio2packet.regdata[1]       = 0;
    sio2packet.out_size         = sizeof(rdbuf);
    sio2packet.in_size          = sizeof(wrbuf);
    memset(sio2packet.in, 0, sizeof(wrbuf));
    sio2packet.in[0] = 0x81;
    sio2packet.in[1] = arg3;
    sio2packet.in[2] = command;
    for (i = 0; i < 8; i++)
        sio2packet.in[3 + i] = ((const unsigned char *)buffer)[7 - i];
    sio2packet.in[11]       = calculate_sio2_buffer_checksum(buffer, 8);
    sio2packet.out_dma.addr = NULL;
    sio2packet.in_dma.addr  = NULL;

    if (SendMcCommand(port, slot, &sio2packet) == 0) {
        _printf4("card_command error %d\n", (unsigned char)command);
        return 0;
    }

    if ((!((sio2packet.stat6c >> 13) & 1) && !((sio2packet.stat6c >> 14) & 3)) == 0) {
        _printf4("sio2 command error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[12] != 0x2B) {
        _printf4("ID error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[13] == 0x66) {
        _printf4("result failed %d\n", (unsigned char)command);
        return 0;
    }

    return 1;
}

// 0x00002620
int card_auth_read(int port, int slot, void *buffer, int arg3, int command)
{
    int i;
    sio2_transfer_data_t sio2packet;
    u8 wrbuf[14];
    u8 rdbuf[14];

    sio2packet.in  = wrbuf;
    sio2packet.out = rdbuf;
    memset(sio2packet.port_ctrl1, 0, sizeof(sio2packet.port_ctrl1));
    memset(sio2packet.port_ctrl2, 0, sizeof(sio2packet.port_ctrl2));
    sio2packet.port_ctrl1[port] = 0xFF060505;
    sio2packet.port_ctrl2[port] = 0x0003FFFF;
    sio2packet.regdata[0]       = (port & 3) | 0x00380E40;
    sio2packet.regdata[1]       = 0;
    sio2packet.out_size         = sizeof(rdbuf);
    sio2packet.in_size          = sizeof(wrbuf);
    memset(sio2packet.in, 0, sizeof(wrbuf));
    sio2packet.in[0]        = 0x81;
    sio2packet.in[1]        = arg3;
    sio2packet.in[2]        = command;
    sio2packet.out_dma.addr = NULL;
    sio2packet.in_dma.addr  = NULL;

    if (SendMcCommand(port, slot, &sio2packet) == 0) {
        _printf4("card_command error %d\n", (unsigned char)command);
        return 0;
    }

    if ((!((sio2packet.stat6c >> 13) & 1) && !((sio2packet.stat6c >> 14) & 3)) == 0) {
        _printf4("sio2 command error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[3] != 0x2B) {
        _printf4("ID error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[13] == 0x66) {
        _printf4("result failed %d\n", (unsigned char)command);
        return 0;
    }

    if ((unsigned char)calculate_sio2_buffer_checksum(&rdbuf[4], 8) != rdbuf[12]) {
        _printf4("check sum error %d\n", (unsigned char)command);
        return 0;
    }

    for (i = 0; i < 8; i++)
        ((unsigned char *)buffer)[7 - i] = rdbuf[4 + i];

    return 1;
}

// 0x00001fe4
int card_auth_60(int port, int slot)
{
    sio2_transfer_data_t sio2packet;
    unsigned char wrbuf[5], rdbuf[5];

    sio2packet.in  = wrbuf;
    sio2packet.out = rdbuf;
    memset(sio2packet.port_ctrl1, 0, sizeof(sio2packet.port_ctrl1));
    memset(sio2packet.port_ctrl2, 0, sizeof(sio2packet.port_ctrl2));
    sio2packet.port_ctrl1[port] = 0xFF060505;
    sio2packet.port_ctrl2[port] = 0x0003FFFF;
    sio2packet.regdata[0]       = (port & 3) | 0x00140540;
    sio2packet.regdata[1]       = 0;
    sio2packet.out_size         = sizeof(rdbuf);
    sio2packet.in_size          = sizeof(wrbuf);
    memset(sio2packet.in, 0, sizeof(wrbuf));
    sio2packet.in[0]        = 0x81;
    sio2packet.in[1]        = 0xF3;
    sio2packet.out_dma.addr = NULL;
    sio2packet.in_dma.addr  = NULL;

    if (SendMcCommand(port, slot, &sio2packet) == 0) {
        _printf4("card_command error 0\n");
        return 0;
    }

    if ((!((sio2packet.stat6c >> 13) & 1) && !((sio2packet.stat6c >> 14) & 3)) == 0) {
        _printf4("sio2 command error 0\n");
        return 0;
    }

    if (rdbuf[3] != 0x2B) {
        _printf4("ID error 0\n");
        return 0;
    }

    if (rdbuf[4] == 0x66) {
        _printf4("result failed 0\n");
        return 0;
    }

    return 1;
}

// 0x00002168
int card_auth_key_change(int port, int slot, int command)
{
    sio2_transfer_data_t sio2packet;
    unsigned char wrbuf[5], rdbuf[5];

    sio2packet.in  = wrbuf;
    sio2packet.out = rdbuf;
    memset(sio2packet.port_ctrl1, 0, sizeof(sio2packet.port_ctrl1));
    memset(sio2packet.port_ctrl2, 0, sizeof(sio2packet.port_ctrl2));
    sio2packet.port_ctrl1[port] = 0xFF060505;
    sio2packet.port_ctrl2[port] = 0x0003FFFF;
    sio2packet.regdata[0]       = (port & 3) | 0x00140540;
    sio2packet.regdata[1]       = 0;
    sio2packet.out_size         = sizeof(rdbuf);
    sio2packet.in_size          = sizeof(wrbuf);
    memset(sio2packet.in, 0, sizeof(wrbuf));
    sio2packet.in[0]        = 0x81;
    sio2packet.in[1]        = 0xF7;
    sio2packet.in[2]        = command;
    sio2packet.out_dma.addr = NULL;
    sio2packet.in_dma.addr  = NULL;

    if (SendMcCommand(port, slot, &sio2packet) == 0) {
        _printf4("card_command error 0\n");
        return 0;
    }

    if ((!((sio2packet.stat6c >> 13) & 1) && !((sio2packet.stat6c >> 14) & 3)) == 0) {
        _printf4("sio2 command error 0\n");
        return 0;
    }

    if (rdbuf[3] != 0x2B) {
        _printf4("ID error 0\n");
        return 0;
    }

    if (rdbuf[4] == 0x66) {
        _printf4("result failed 0\n");
        return 0;
    }

    return 1;
}

// 0x0000248c
int card_auth2(int port, int slot, int arg3, int command)
{
    sio2_transfer_data_t sio2packet;
    unsigned char wrbuf[5], rdbuf[5];

    sio2packet.in  = wrbuf;
    sio2packet.out = rdbuf;
    memset(sio2packet.port_ctrl1, 0, sizeof(sio2packet.port_ctrl1));
    memset(sio2packet.port_ctrl2, 0, sizeof(sio2packet.port_ctrl2));
    sio2packet.port_ctrl1[port] = 0xFF060505;
    sio2packet.port_ctrl2[port] = 0x0003FFFF;
    sio2packet.regdata[0]       = (port & 3) | 0x00140540;
    sio2packet.regdata[1]       = 0;
    sio2packet.out_size         = sizeof(rdbuf);
    sio2packet.in_size          = sizeof(wrbuf);
    memset(sio2packet.in, 0, sizeof(wrbuf));
    sio2packet.in[0]        = 0x81;
    sio2packet.in[1]        = arg3;
    sio2packet.in[2]        = command;
    sio2packet.out_dma.addr = NULL;
    sio2packet.in_dma.addr  = NULL;

    if (SendMcCommand(port, slot, &sio2packet) == 0) {
        _printf4("card_command error %d\n", (unsigned char)command);
        return 0;
    }

    if ((!((sio2packet.stat6c >> 13) & 1) && !((sio2packet.stat6c >> 14) & 3)) == 0) {
        _printf4("sio2 command error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[3] != 0x2B) {
        _printf4("ID error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[4] == 0x66) {
        _printf4("result failed %d\n", (unsigned char)command);
        return 0;
    }

    return 1;
}

// 0x000022f8
int card_auth(int port, int slot, int arg3, int command)
{
    sio2_transfer_data_t sio2packet;
    unsigned char wrbuf[5], rdbuf[5];

    sio2packet.in  = wrbuf;
    sio2packet.out = rdbuf;
    memset(sio2packet.port_ctrl1, 0, sizeof(sio2packet.port_ctrl1));
    memset(sio2packet.port_ctrl2, 0, sizeof(sio2packet.port_ctrl2));
    sio2packet.port_ctrl1[port] = 0xFF060505;
    sio2packet.port_ctrl2[port] = 0x0003FFFF;
    sio2packet.regdata[0]       = (port & 3) | 0x00140540;
    sio2packet.regdata[1]       = 0;
    sio2packet.out_size         = sizeof(rdbuf);
    sio2packet.in_size          = sizeof(wrbuf);
    memset(sio2packet.in, 0, sizeof(wrbuf));
    sio2packet.in[0]        = 0x81;
    sio2packet.in[1]        = arg3;
    sio2packet.in[2]        = command;
    sio2packet.out_dma.addr = NULL;
    sio2packet.in_dma.addr  = NULL;

    if (SendMcCommand(port, slot, &sio2packet) == 0) {
        _printf4("card_command error %d\n", (unsigned char)command);
        return 0;
    }

    if ((!((sio2packet.stat6c >> 13) & 1) && !((sio2packet.stat6c >> 14) & 3)) == 0) {
        _printf4("sio2 command error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[3] != 0x2B) {
        _printf4("ID error %d\n", (unsigned char)command);
        return 0;
    }

    if (rdbuf[4] == 0x66) {
        _printf4("result failed %d\n", (unsigned char)command);
        return 0;
    }

    return 1;
}
