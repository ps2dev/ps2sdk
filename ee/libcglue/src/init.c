/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * The global init/deinit code for our crt0.
 */

void _libcglue_timezone_update();

int chdir(const char *path);

#ifdef F__libcglue_init
__attribute__((weak))
void _libcglue_init()
{
    _libcglue_timezone_update();
}
#endif

#ifdef F__libcglue_deinit
__attribute__((weak))
void _libcglue_deinit()
{
}
#endif

#ifdef F__libcglue_args_parse
__attribute__((weak))
void _libcglue_args_parse(int argc, char ** argv)
{
    if (argc == 0) // naplink!
    {
	chdir("host:");
    } else {
	char * p, * s = 0;
	// let's find the last slash, or at worst, the :
	for (p = argv[0]; *p; p++) {
	    if ((*p == '/') || (*p == '\\') || (*p == ':')) {
		s = p;
	    }
	}
	// Nothing?! strange, let's use host.
	if (!s) {
	    chdir("host:");
	} else {
	    char backup = *(++s);
	    *s = 0;
	    chdir(argv[0]);
	    *s = backup;
	}
    }
}
#endif

