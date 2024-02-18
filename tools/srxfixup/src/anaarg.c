/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "srxfixup_internal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int analize_arguments(const Opttable *dopttable, int argc, char **argv)
{
	Opt_strings *optstr;
	const char *opt;
	Opttable *otp;
	Opttable *igadd;
	Opttable *opttable;
	char *cp;
	char **argvp;
	char **nargv;
	int nargc;
	int i;
	int argca;
	char **argva;

	for ( i = 0; dopttable[i].option; i += 1 )
	{
		;
	}
	opttable = (Opttable *)__builtin_alloca((argc + i) * sizeof(Opttable));
	memset(opttable, 0, (argc + i) * sizeof(Opttable));
	memcpy(opttable, dopttable, i * sizeof(Opttable));
	igadd = &opttable[i];
	nargv = (char **)__builtin_alloca((argc + 1) * sizeof(char *));
	memset(nargv, 0, (argc + 1) * sizeof(char *));
	argvp = argv;
	*nargv = *argv;
	nargc = 1;
	for ( argca = argc - 1, argva = argv + 1;; argca -= 1, argva += 1 )
	{
		if ( argca <= 0 )
		{
			for ( i = 0; i < nargc + 1; i += 1 )
			{
				argvp[i] = nargv[i];
			}
			for ( i = 0; opttable[i].option; i += 1 )
			{
				if ( opttable[i].vartype == 'l' )
				{
					*(SLink **)opttable[i].var = ring_to_liner(*(SLink **)opttable[i].var);
				}
			}
			return nargc;
		}
		if ( **argva == '-' )
		{
			opt = 0;
			for ( i = 0; opttable[i].option; i += 1 )
			{
				if ( opttable[i].havearg == ARG_HAVEARG_UNK3 )
				{
					if ( !strcmp(opttable[i].option, *argva) )
					{
						break;
					}
				}
				else
				{
					if ( !strncmp(opttable[i].option, *argva, strlen(opttable[i].option)) )
					{
						break;
					}
				}
			}
			if ( !opttable[i].option )
			{
				return -1;
			}
			if ( opttable[i].havearg != ARG_HAVEARG_NONE && opttable[i].havearg != ARG_HAVEARG_UNK3 )
			{
				if ( opttable[i].havearg == ARG_HAVEARG_UNK4 || (*argva)[strlen(opttable[i].option)] )
				{
					opt = &(*argva)[strlen(opttable[i].option)];
				}
				else if ( argca > 1 )
				{
					opt = argva[1];
					argva += 1;
					argca -= 1;
				}
			}
			if ( opttable[i].havearg != ARG_HAVEARG_REQUIRED || opt )
			{
				switch ( opttable[i].vartype )
				{
					case 'F':
					case 'f':
						if ( (*argva)[strlen(opttable[i].option)] )
						{
							*(uint32_t *)opttable[i].var = strtoul(&(*argva)[strlen(opttable[i].option)], NULL, 16);
						}
						else
						{
							*(uint32_t *)opttable[i].var = (opttable[i].vartype == 'f') ? 1 : 0;
						}
						break;
					case 'h':
						if ( opt != NULL )
						{
							*(uint32_t *)opttable[i].var = strtoul(opt, 0, 16);
						}
						break;
					case 'i':
						if ( opt != NULL )
						{
							for ( otp = igadd; opttable < otp; otp -= 1 )
							{
								*otp = otp[-1];
							}
							igadd += 1;
							opttable->option = opt;
							opttable->vartype = 'n';
							opttable->havearg = ARG_HAVEARG_UNK3;
							cp = strchr(opttable->option, ':');
							if ( cp )
							{
								*cp = 0;
								switch ( cp[1] )
								{
									case 'c':
										opttable->havearg = ARG_HAVEARG_UNK4;
										break;
									case 'n':
										opttable->havearg = ARG_HAVEARG_REQUIRED;
										break;
									case 'o':
										opttable->havearg = ARG_HAVEARG_UNK1;
										break;
									default:
										break;
								}
							}
						}
						break;
					case 'l':
						optstr = (Opt_strings *)calloc(1, sizeof(Opt_strings));
						optstr->string = opt;
						*(SLink **)opttable[i].var = add_ring_tail(*(SLink **)opttable[i].var, (SLink *)optstr);
						break;
					case 'n':
						break;
					case 's':
						*(const char **)opttable[i].var = opt;
						break;
					default:
						fprintf(stderr, "internal error\n");
						return -1;
				}
			}
			else
			{
				return -1;
			}
		}
		else
		{
			nargv[nargc] = *argva;
			nargc += 1;
		}
	}
}
