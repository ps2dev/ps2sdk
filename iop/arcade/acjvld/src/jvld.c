/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <accore.h>
#include <irx_imports.h>

#define MODNAME "JV_loader"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// f0b629a31e9fcde972afce3df69d5342
// Known titles:
// NM00017
// NM00018
// NM00019
// NM00021
// NM00024
// NM00027
// NM00029
// NM00030
// NM00032
// NM00034
// NM00035
// NM00037
// NM00040
// NM00042
// NM00043
// NM00048
// NM00052
// Path strings:
// /home/kyota/fs16/psalmT/psalm-0.2.8/jvld-iop-0.2.2/src/

typedef char *(*jvld_strop_t)(char *name);

static char *jvld_path(char *name);
static char *jvld_basename(char *name);

static jvld_strop_t ops_8[2] = {&jvld_path, &jvld_basename};
static char buf_13[8192];

static char *jvld_path(char *name)
{
	char *str;

	str = strchr(name, ':');
	if ( str == 0 )
		return name;
	return str + 1;
}

static char *jvld_basename(char *name)
{
	char *str;
	char *str_v3;
	char *str_v4;
	char *str_v5;
	char *str_v6;

	str = strchr(name, ':');
	if ( str == 0 )
		return name;
	str_v3 = str + 1;
	str_v4 = strrchr(str_v3, '/');
	if ( str_v4 == 0 )
	{
		str_v6 = strrchr(str_v3, '\\');
		if ( str_v6 == 0 )
			return str_v3;
		str_v5 = str_v6 + 1;
	}
	else
	{
		str_v5 = str_v4 + 1;
	}
	return str_v5;
}

#define acJvLoader _start

int acJvLoader(int argc, char **argv)
{
	char *prog;
	int ret;
	char *name;
	int fd;

	prog = 0;
	if ( argv )
		prog = *argv;
	if ( argc < 2 )
	{
		ret = -22;
		return 4 * ret + 1;
	}
	name = argv[1];
	if ( jvld_path(name) == name )
	{
		fd = -6;
	}
	else
	{
		fd = open(name, 1);
		if ( fd < 0 && prog )
		{
			int index;

			index = 0;
			while ( (unsigned int)index < 2 )
			{
				jvld_strop_t op;
				const char *v9;

				op = ops_8[index];
				v9 = (char *)op(prog);
				if ( v9 != prog )
				{
					memcpy(buf_13, prog, v9 - prog);
					strcpy(&buf_13[v9 - prog], op(name));
					fd = open(buf_13, 1);
					if ( fd >= 0 )
						break;
				}
				index += 1;
			}
		}
	}
	if ( fd < 0 )
	{
		printf("acjvld:open: error %d\n", fd);
		ret = fd;
	}
	else
	{
		volatile acUint16 *ret_v10;
		int sent;

		*((volatile acUint16 *)0xB2416000) = 0;
		ret_v10 = (volatile acUint16 *)0xB2400000;
		sent = 0;
		while ( 1 )
		{
			int c448;
			const unsigned char *ptr;
			int ret_v15;

			c448 = read(fd, buf_13, 0x2000);
			if ( c448 <= 0 )
				break;
			ptr = (unsigned char *)buf_13;
			sent += c448;
			ret_v15 = c448 - 1;
			while ( ret_v15 >= 0 )
			{
				--ret_v15;
				*ret_v10++ = *ptr++;
			}
		}
		if ( sent != lseek(fd, 0, 2) )
		{
			ret = -5;
		}
		else
		{
			ret = sent;
			if ( !sent )
			{
				ret = -8;
			}
		}
		close(fd);
	}
	if ( ret >= 0 )
		return 1;
	return 4 * ret + 1;
}
