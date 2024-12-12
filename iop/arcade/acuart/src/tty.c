#include "irx_imports.h"
#include <errno.h>


int acUartWrite(void *buf, int count);
int acUartRead(void *buf, int count);

static int acuart_read(iop_file_t *f, void *buffer, int size) {
    (void)f;
    return acUartRead(buffer, size);
}
static int acuart_write(iop_file_t *f, void *buffer, int size) {
    (void)f;
    return acUartWrite(buffer, size);
}

IOMAN_RETURN_VALUE_IMPL(0);
IOMAN_RETURN_VALUE_IMPL(ENOTSUP);

static iop_device_ops_t uart_ops = {
    IOMAN_RETURN_VALUE(0), // init
    IOMAN_RETURN_VALUE(0), // deinit
    IOMAN_RETURN_VALUE(ENOTSUP), // format
    IOMAN_RETURN_VALUE(ENOTSUP), // open
    IOMAN_RETURN_VALUE(ENOTSUP), // close
    &acuart_read, // read
    &acuart_write, // write
    IOMAN_RETURN_VALUE(ENOTSUP), // lseek
    IOMAN_RETURN_VALUE(ENOTSUP), // ioctl
    IOMAN_RETURN_VALUE(ENOTSUP), // remove
    IOMAN_RETURN_VALUE(ENOTSUP), // mkdir
    IOMAN_RETURN_VALUE(ENOTSUP), // rmdir
    IOMAN_RETURN_VALUE(ENOTSUP), // dopen
    IOMAN_RETURN_VALUE(ENOTSUP), // dclose
    IOMAN_RETURN_VALUE(ENOTSUP), // dread
    IOMAN_RETURN_VALUE(ENOTSUP), // getstat
    IOMAN_RETURN_VALUE(ENOTSUP), // chstat
};

#define DEVNAME "tty"

static iop_device_t uart_tty = {
    DEVNAME,
    IOP_DT_CHAR	| IOP_DT_CONS,
    1,
    "TTY via arcade UART",
    &uart_ops,
};

int CreateTTY() {
    if(AddDrv(&uart_tty) != 0) return MODULE_NO_RESIDENT_END;
    
    close(0);
    open(DEVNAME "00:", 0x1000 | O_RDWR);
    
    close(1);
    open(DEVNAME "00:", O_WRONLY);

    close(2);
    open(DEVNAME "00:", O_WRONLY);
    return MODULE_RESIDENT_END;
}