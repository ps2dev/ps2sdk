/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Testsuite runner
*/

#include <stdio.h>
#include "testsuite.h"

#ifdef _EE
#include <sifrpc.h>
#include <sbv_patches.h>
#include <unistd.h>
#endif

extern int libc_add_tests(test_suite *p);

static void iop_start(void)
{
    SifInitRpc(0);

    sbv_patch_fileio();
}

int main(int argc, char *argv[])
{
    test_suite suite;

    iop_start();

    /* initialize test suite */
    init_testsuite(&suite);

    /* add all tests to this suite */
    libc_add_tests(&suite);

    /* run all tests */
    run_testsuite(&suite);

    /* do some stuff or ps2client will freeze */
    while (1)
    {
        sleep(10);
        printf("I am alive\n");
    };

    return 0;
}
