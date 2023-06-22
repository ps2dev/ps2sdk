#include <stdint.h>
#include <thbase.h>
#include <sifrpc.h>

#include "xprintf.h"    /* needed for vsnprintf, other implementation appears to be broken */
#include "rpc_printf.h"

#define RPC_ID 0x1a1a1a1b

static char msg_buff[128] __attribute__((aligned (64)));
static SifRpcClientData_t client;
static uint8_t rpc_init = 0; 

/* This is ONLY built in _rpc and _rpc_v builds and currently only used for the sdcard test app */

/* It's purpose is to pipe messages from the mx4sio driver over to the EE
 * in a way that doesn't require any special print statements, conditions, or 
 * changes to the behavior of the driver itself. */

int rpc_printf_init()
{
    while(sceSifBindRpc(&client, RPC_ID, 0) < 0 || client.server == NULL) {
        DelayThread(1000 * 1000);
    }

    rpc_init = 1;

    return 0;
}

int rpc_printf(const char *format, ...)
{
    int msg_len;

    if (rpc_init != 1) {
        rpc_printf_init();
    }

    va_list args;
    va_start(args, format);

    /* use vsnprintf from xprintf to create the formated msg */
    msg_len = vsnprintf(msg_buff, 128, format, args);
    va_end(args);

    /* send formatted msg to EE side */
    SifCallRpc(&client, RPC_ID, 0, &msg_buff, msg_len, NULL, 0, NULL, NULL);

    return 0;
}
