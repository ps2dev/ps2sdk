/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Testsuite runner
*/

#include <stdio.h>
#include "testsuite.h"

#ifdef _EE
#include <sifrpc.h>
#endif

extern int libc_add_tests(test_suite *p);

int main(int argc, char *argv[])
{
	test_suite suite;

	#ifdef _EE
	SifInitRpc(0); 
	#endif

	/* initialize test suite */
	init_testsuite(&suite);

	/* add all tests to this suite */
	libc_add_tests(&suite);

	/* run all tests */
	run_testsuite(&suite);

	return 0;
}
