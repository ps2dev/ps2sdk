#include "irx_imports.h"
#include <errno.h>


int acUartWrite(void *buf, int count);
int acUartRead(void *buf, int count);

int dummy() {return -ENOTSUP;}

static int acuart_read(iop_file_t *f, void *buffer, int size) {
    (void)f;
    return acUartRead(buffer, size);
}
static int acuart_write(iop_file_t *f, void *buffer, int size) {
    (void)f;
    return acUartWrite(buffer, size);
}

static iop_device_ops_t uart_ops = {
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    &acuart_read,
    &acuart_write,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
    (void *)&dummy,
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