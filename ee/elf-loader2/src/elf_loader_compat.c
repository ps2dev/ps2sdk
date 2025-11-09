/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "elf_loader_common.h"

#include <iopcontrol.h>
#include <loadfile.h>
#include <sifrpc.h>
#include <stdlib.h>

int LoadELFFromFileWithPartition(const char *filename, const char *partition, int argc, char *argv[])
{
	int ret;
	elf_loader_reader_info_stdio_resultbuf_t resultbuf;
	elf_loader_execinfo_t execinfo;

	ret = elf_loader_reader_read_elf_file_stdio(&resultbuf, filename, "rb");
	if ( ret < 0 )
	{
		return ret;
	}
	ret = elf_loader_exec_elf_prepare_loadinfo(&execinfo, resultbuf.m_buf, resultbuf.m_bufsize);
	if ( ret < 0 )
	{
		free(resultbuf.m_buf);
		return ret;
	}
	ret = elf_loader_exec_elf_prepare_arginfo(&execinfo, filename, partition, argc, argv);
	if ( ret < 0 )
	{
		free(resultbuf.m_buf);
		return ret;
	}

	// No turning back here.
	while ( !SifIopReset(NULL, 0) )
		;
	while ( !SifIopSync() )
		;

	sceSifInitRpc(0);
	SifLoadFileInit();
	SifLoadModule("rom0:SIO2MAN", 0, NULL);
	SifLoadModule("rom0:MCMAN", 0, NULL);
	SifLoadModule("rom0:MCSERV", 0, NULL);
	SifLoadFileExit();
	sceSifExitRpc();

	return elf_loader_exec_elf(&execinfo);
}

int LoadELFFromFile(const char *filename, int argc, char *argv[])
{
	return LoadELFFromFileWithPartition(filename, NULL, argc, argv);
}
