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

#define MODNAME "FPGA_loader"
IRX_ID(MODNAME, 1, 1);
// Text section hash:
// 3555dfa26ee67afededdfa0d981cd68f
// Known titles:
// NM00005
// NM00006
// NM00008
// Path strings:
// /home/ueda/tmp/psalm-0.1.3/fpgald-iop-0.1.4/src/
// TODO: diff with module text hash fb29cec4461e70e3f6efd1d7259278cb

typedef char *(*fpga_strop_t)(char *name);

static char *fpgald_path(char *name);
static char *fpgald_basename(char *name);

static fpga_strop_t ops_12[2] = {&fpgald_path, &fpgald_basename};
static char buf_17[8192];

static int fpgald_wait(acC448Reg c448, acUint32 flag, int verbose)
{
	int errors;
	acUint32 fbit;
	int sr;

	errors = 0;
	fbit = 32;
	while ( fbit )
	{
		int count;

		if ( !flag )
			break;
		count = 0xFFFF;
		if ( (flag & fbit) != 0 )
		{
			flag ^= fbit;
			while ( count >= 0 )
			{
				sr = (c448[0x3800] & 0xFF00) >> 8;
				if ( (sr & fbit) != 0 )
					break;
				--count;
			}
			if ( count < 0 && verbose )
			{
				printf("acfpgald:wait: TIMEDOUT %02x %02x\n", sr, (unsigned int)fbit);
				++errors;
			}
		}
		fbit >>= 1;
	}
	return errors;
}

static char *fpgald_path(char *name)
{
	char *str;

	str = strchr(name, ':');
	if ( str == 0 )
		return name;
	return str + 1;
}

static char *fpgald_basename(char *name)
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

#define acFpgaLoader _start

int acFpgaLoader(int argc, char **argv)
{
	char *prog;
	int ret;
	char *name;
	int fd;

	prog = 0;
	if ( argv )
		prog = *argv;
	if ( argc < 2 )
		return -22;
	name = argv[1];
	if ( fpgald_path(name) == name )
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
				fpga_strop_t op;
				const char *v9;

				op = ops_12[index];
				v9 = (char *)op(prog);
				if ( v9 != prog )
				{
					memcpy(buf_17, prog, v9 - prog);
					strcpy(&buf_17[v9 - prog], op(name));
					fd = open(buf_17, 1);
					if ( fd >= 0 )
						break;
				}
				index++;
			}
		}
	}
	if ( fd < 0 )
	{
		printf("acfpgald:open: error %d\n", fd);
		ret = fd;
	}
	else
	{
		int size;
		int errors;
		int ret_v13;

		*((volatile acUint16 *)0xB2416008) = 0;
		fpgald_wait((acC448Reg)0xB2415000, 0x30u, 0);
		*((volatile acUint16 *)0xB241600A) = 0;
		size = 0;
		errors = fpgald_wait((acC448Reg)0xB2415000, 0x20u, 1);
		DelayThread(1000);
		while ( 1 )
		{
			int xlen;
			const acUint8 *ptr;

			ret_v13 = read(fd, buf_17, 0x2000);
			xlen = ret_v13 - 1;
			if ( ret_v13 <= 0 )
				break;
			ptr = (acUint8 *)buf_17;
			size += ret_v13;
			while ( xlen >= 0 )
			{
				int value;
				int bytecount;

				value = *ptr++;
				bytecount = 7;
				while ( bytecount >= 0 )
				{
					int ret_v18;

					ret_v18 = value & 1;
					value >>= 1;
					--bytecount;
					*((volatile acUint16 *)0xB2416014) = 0;
					*(acUint16 *)(2 * ret_v18 + 0xB2415000 + 0x1018) = 0;
					*((volatile acUint16 *)0xB2416016) = 0;
				}
				--xlen;
			}
		}
		*((volatile acUint16 *)0xB2416014) = 0;
		if ( ret_v13 < 0 )
		{
			ret = -5;
		}
		else
		{
			if ( errors + fpgald_wait((acC448Reg)0xB2415000, 0x10u, 1) > 0 )
			{
				ret = -116;
			}
			else
			{
				int ret_v19;

				ret_v19 = 38;
				while ( ret_v19 >= 0 )
				{
					ret_v19--;
					*((volatile acUint16 *)0xB2416016) = 0;
					*((volatile acUint16 *)0xB2416014) = 0;
				}
				DelayThread(1000);
				*((volatile acUint16 *)0xB2416012) = 0;
				ret = size;
			}
		}
		close(fd);
	}
	if ( ret >= 0 )
		return 1;
	return ret;
}
