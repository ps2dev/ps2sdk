#include <iopcontrol.h>
#include <stdint.h>
#include <kernel.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <stdio.h>
#include <sifrpc.h>
#include <string.h>

#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>

int __iomanX_id = -1;
int __fileXio_id = -1;

/* References to IOMANX.IRX */
extern unsigned char iomanX_irx[] __attribute__((aligned(16)));
extern unsigned int size_iomanX_irx;

/* References to FILEXIO.IRX */
extern unsigned char fileXio_irx[] __attribute__((aligned(16)));
extern unsigned int size_fileXio_irx;

static void reset_IOP() {
    SifInitRpc(0);
#if !defined(DEBUG) || defined(BUILD_FOR_PCSX2)
    /* Comment this line if you don't wanna debug the output */
    while (!SifIopReset(NULL, 0)) {};
#endif

    while (!SifIopSync()) {};
    SifInitRpc(0);
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
}

static int loadIRXs(void) {
    /* IOMANX.IRX */
    __iomanX_id = SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx, 0, NULL, NULL);
    if (__iomanX_id < 0)
        return -1;

    /* FILEXIO.IRX */
    __fileXio_id = SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx, 0, NULL, NULL);
    if (__fileXio_id < 0)
        return -2;

    return 0;
}

static int initLibraries(void) {
    return fileXioInit();
}

static int init_fileXio_driver() {
    int __fileXio_init_status = loadIRXs();
    if (__fileXio_init_status < 0)
        return __fileXio_init_status;

    __fileXio_init_status = initLibraries();

    return __fileXio_init_status;
}

int main(int argc, char *argv[])
{
    reset_IOP();
	init_fileXio_driver();

	while (1)
	{
		printf("Hello using fileXio\n");
	}

	return 0;
}
