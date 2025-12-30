/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include <loadcore.h>
#include <sys/fcntl.h>

IRX_ID("S147NETB", 2, 2);
// Text section hash:
// 49d8ea7bd41706df5cfcebac25acdc5e

int _start(int ac, char **av)
{
	char probebuf;
	int fd;
	int probe_i;

	(void)ac;
	(void)av;
	Kprintf("\ns147netb.irx: System147 Network Boot Manager v%d.%d\n", 2, 2);
	fd = open("atfile19:usb-probe", O_RDONLY);
	if ( fd < 0 )
	{
		Kprintf("s147netb.irx: Could not initialize USB memory driver\n");
		return MODULE_NO_RESIDENT_END;
	}
	for ( probe_i = 5; probe_i > 0; probe_i -= 1 )
	{
		read(fd, &probebuf, sizeof(probebuf));
		Kprintf("\ns147netb.irx: === Wait %dsec for USB initialize... (probe=%d) ===\n", probe_i, (u8)probebuf);
		if ( probebuf == 1 )
			break;
		DelayThread(1000000);
	}
	close(fd);
	if ( !probebuf )
	{
		Kprintf("\ns147netb.irx: *** No USB memory ***\n");
		return MODULE_RESIDENT_END;
	}
	fd = open("atfile10:ifc000.cnf", O_RDONLY);
	if ( fd < 0 )
	{
		Kprintf("s147netb.irx: Could not find \"%s\"\n", "atfile10:ifc000.cnf");
		return MODULE_RESIDENT_END;
	}
	close(fd);
	Kprintf("s147netb.irx: \"%s\" is found\n", "atfile10:ifc000.cnf");
	fd = open("atfile10:inet.irx", O_RDONLY);
	if ( fd < 0 )
	{
		Kprintf("s147netb.irx: Could not find \"%s\"\n", "atfile10:inet.irx");
		return MODULE_RESIDENT_END;
	}
	close(fd);
	Kprintf("s147netb.irx: \"%s\" is found\n", "atfile10:inet.irx");
	fd = open("atfile10:an986.irx", O_RDONLY);
	if ( fd < 0 )
	{
		Kprintf("s147netb.irx: Could not find \"%s\"\n", "atfile10:an986.irx");
		return MODULE_RESIDENT_END;
	}
	close(fd);
	Kprintf("s147netb.irx: \"%s\" is found\n", "atfile10:an986.irx");
	fd = open("atfile10:s147http.irx", O_RDONLY);
	if ( fd < 0 )
	{
		Kprintf("s147netb.irx: Could not find \"%s\"\n", "atfile10:s147http.irx");
		return MODULE_RESIDENT_END;
	}
	close(fd);
	Kprintf("s147netb.irx: \"%s\" is found\n", "atfile10:s147http.irx");
	Kprintf(
		"s147netb.irx: LoadStartModule \"%s\" (%d)\n", "atfile10:inet.irx", LoadStartModule("atfile10:inet.irx", 0, 0, 0));
	Kprintf(
		"s147netb.irx: LoadStartModule \"%s\" (%d)\n",
		"atfile10:an986.irx",
		LoadStartModule("atfile10:an986.irx", 0, 0, 0));
	Kprintf(
		"s147netb.irx: LoadStartModule \"%s\" (%d)\n",
		"atfile10:s147http.irx",
		LoadStartModule("atfile10:s147http.irx", 0, 0, 0));
	return MODULE_RESIDENT_END;
}
