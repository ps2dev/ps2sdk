#include <stdio.h>
#include <cdvdman.h>
#include <sysclib.h>

#include "secrman.h"

#include "main.h"
#include "MechaAuth.h"

#if 0
static void _printf3(const char *format, ...);
#endif

// 0x00001570
#if 0
static void _printf3(const char *format, ...){

}
#endif

// 0x00001588
int SendMechaCommand(int command, const void *input, unsigned short int length, void *output)
{
    unsigned int i;

    _printf3("mecha command:%02x param:", (unsigned char)command);
    for (i = 0; i < length; i++) {
        _printf3(" %02x", ((unsigned char *)input)[i]);
    }
    _printf3("\n");

    return sceCdApplySCmd((unsigned char)command, (void *)input, length, output);
}

// 0x00001634
int mechacon_auth_80(int cnum)
{
    unsigned char ret[16], parameter[16];

    parameter[0] = cnum;
    return (SendMechaCommand(0x80, parameter, 1, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x0000167c
int mechacon_auth_81(int cnum)
{
    unsigned char ret[16], parameter[16];

    parameter[0] = cnum;
    return (SendMechaCommand(0x81, parameter, 1, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x000016c4
int mechacon_auth_82(const void *buffer, const void *buffer2)
{
    unsigned char ret[16], parameters[16];
    unsigned int i;

    for (i = 0; i < 8; i++) {
        parameters[i]     = ((unsigned char *)buffer)[i];
        parameters[8 + i] = ((unsigned char *)buffer2)[i];
    }
    return (SendMechaCommand(0x82, parameters, 16, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x00001740
int mechacon_auth_83(const void *buffer)
{
    unsigned char ret[16], parameters[16];

    memcpy(parameters, buffer, 8);
    return (SendMechaCommand(0x83, parameters, 8, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x000017ac
int mechacon_auth_84(void *buffer, void *buffer2)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x84, NULL, 0, ret) != 0 && ret[0] == 0) {
        memcpy(buffer, &ret[1], 8);
        memcpy(buffer2, &ret[9], 4);
        result = 1;
    } else
        result = 0;

    return result;
}

// 0x0000185c
int mechacon_auth_85(void *buffer, void *buffer2)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x85, NULL, 0, ret) != 0 && ret[0] == 0) {
        memcpy(buffer, &ret[1], 4);
        memcpy(buffer2, &ret[5], 8);
        result = 1;
    } else
        result = 0;

    return result;
}

// 0x0000190c
int mechacon_auth_86(void *buffer, void *buffer2)
{
    unsigned char ret[16], parameters[16];
    unsigned int i;

    for (i = 0; i < 8; i++) {
        parameters[i]     = ((unsigned char *)buffer)[i];
        parameters[8 + i] = ((unsigned char *)buffer2)[i];
    }
    return (SendMechaCommand(0x86, parameters, 16, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x00001988
int mechacon_auth_87(void *buffer)
{
    unsigned char ret[16], parameters[16];

    memcpy(parameters, buffer, 8);
    return (SendMechaCommand(0x87, parameters, 8, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x000019f4
int mechacon_auth_88(void)
{
    unsigned char ret[16];

    return (SendMechaCommand(0x88, NULL, 0, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x00001b88
int write_HD_start(unsigned char mode, int cnum, int arg2, unsigned short int HeaderLength)
{
    unsigned char ret[16], parameters[16];

    parameters[0] = mode;
    parameters[1] = (unsigned char)HeaderLength;
    parameters[2] = (unsigned char)(HeaderLength >> 8);
    parameters[3] = cnum;
    parameters[4] = arg2;
    return (SendMechaCommand(0x90, parameters, 5, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x00001a88
int write_data(const void *buffer, unsigned short int length)
{
    int result;

    if (length <= 16) {
        unsigned char ret[16], input[16];

        memcpy(input, buffer, length);
        result = SendMechaCommand(0x8D, input, length, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0;
    } else
        result = 0;

    return result;
}

// 0x00001a38
int pol_cal_cmplt(void)
{
    unsigned char ret[16];

    do {
        if (!SendMechaCommand(0x8F, NULL, 0, ret))
            return 0;
    } while (ret[0] == 1);

    return ((ret[0] == 0) ? 1 : 0);
}

// 0x00001c98
int func_00001c98(unsigned short int size)
{
    unsigned char params[16], ret[16];
    int result;

    params[0] = (unsigned char)size;
    params[1] = (unsigned char)(size >> 8);
    if (SendMechaCommand(0x93, params, 2, ret)) {
        result = ret[0] == 0 ? 1 : 0;
    } else
        result = 0;

    return result;
}

// 0x00001be4
int get_BIT_length(unsigned short int *BitLength)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x91, NULL, 0, ret) != 0) {
        if (ret[0] == 0) {
            *BitLength = ret[1] + ((int)ret[2] << 8);
            result     = 1;
        } else {
            result = 0;
        }
    } else {
        result = 0;
    }

    return result;
}

// 0x00001b00
int func_00001b00(void *data, unsigned short int length)
{
    int result;

    if (length <= 16) {
        unsigned char ret[16];

        if (SendMechaCommand(0x8E, NULL, 0, ret)) {
            memcpy(data, ret, length);
            result = 1;
        } else
            result = 0;
    } else
        result = 0;

    return result;
}

// 0x00001c48
int mechacon_set_block_size(unsigned short int size)
{
    unsigned char ret[16], parameters[16];

    parameters[0] = (unsigned char)size;
    parameters[1] = (unsigned char)(size >> 8);
    return (SendMechaCommand(0x92, parameters, 2, ret) != 0 ? (ret[0] == 0 ? 1 : 0) : 0);
}

// 0x00001ce8
int func_00001ce8(void *kbit1)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x94, NULL, 0, ret) && ret[0] == 0) {
        memcpy(kbit1, &ret[1], 8);
        result = 1;
    } else
        result = 0;

    return result;
}

// 0x00001d64
int func_00001d64(void *kbit2)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x95, NULL, 0, ret) && ret[0] == 0) {
        memcpy(kbit2, &ret[1], 8);
        result = 1;
    } else
        result = 0;

    return result;
}

// 0x00001de0
int func_00001de0(void *kc1)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x96, NULL, 0, ret) && ret[0] == 0) {
        memcpy(kc1, &ret[1], 8);
        result = 1;
    } else
        result = 0;

    return result;
}

// 0x00001e5c
int func_00001e5c(void *kc2)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x97, NULL, 0, ret) && ret[0] == 0) {
        memcpy(kc2, &ret[1], 8);
        result = 1;
    } else
        result = 0;

    return result;
}

// 0x00001ed8
int func_00001ed8(void *icvps2)
{
    unsigned char ret[16];
    int result;

    if (SendMechaCommand(0x98, NULL, 0, ret) && ret[0] == 0) {
        memcpy(icvps2, &ret[1], 8);
        result = 1;
    } else
        result = 0;

    return result;
}
