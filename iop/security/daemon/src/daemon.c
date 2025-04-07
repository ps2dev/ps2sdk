#include "irx_imports.h"

IRX_ID("Sec_Checker", 0x1, 0x1)

void sec_checker(void) {
    int x;
    printf("check card routine start\n");
    do {
        McDetectCard2(0,0);
        x = 0x3c;
        while (0 < x) {
            DelayThread(1000000);
            x--;
        }
    } while ( 1 );

}

//Original debug symbol: `start`
int _start() {
    iop_thread_t sec_thread;
    int thid, bVar1;
    CpuEnableIntr();
    sec_thread.attr = TH_C;
    sec_thread.thread = sec_checker;
    sec_thread.priority = LOWEST_PRIORITY;
    sec_thread.stacksize = 0x800;
    sec_thread.option = 0;
    thid = CreateThread(&sec_thread);
    bVar1 = thid < 1;
    if (!bVar1)
    StartThread(thid,0);
    return bVar1;
}
