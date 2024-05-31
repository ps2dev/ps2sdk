/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "accdvd_internal.h"

static char *argv_20[2] = {"sceCdInit", 0};
static struct cdi_softc Cdic;

sceCdCBFunc sceCdCallback(sceCdCBFunc func)
{
	sceCdCBFunc result;

	result = (sceCdCBFunc)Cdic.done;
	Cdic.done = (cdc_done_t)func;
	return (sceCdCBFunc)result;
}

int sceCdBreak()
{
	return 0;
}

int sceCdDiskReady(int mode)
{
	return cdc_ready(mode);
}

int sceCdGetDiskType()
{
	return cdc_medium();
}

int sceCdGetError()
{
	return cdc_error();
}

u32 sceCdGetReadPos(void)
{
	return cdc_getpos() / 16;
}

static int cdi_xfer(void *dst, void *src, int len, enum cdc_xfer_dir dir)
{
	if ( dir )
		memcpy(dst, src, len);
	return len;
}

int sceCdGetToc(unsigned char *toc)
{
	if ( !toc )
		return cdc_seterrno(34);
	return cdc_readtoc(toc, cdi_xfer);
}

int sceCdInit(int mode)
{
	if ( mode < 0 )
		return 0;
	if ( mode >= 2 )
		return mode == 5 && cdc_module_stop() >= 0;
	if ( cdc_module_status() > 0 && cdc_module_stop() < 0 )
	{
		return 0;
	}
	if ( cdc_module_start(1, argv_20) < 0 )
	{
		return 0;
	}
	memset(&Cdic, 0, sizeof(Cdic));
	if ( !mode )
	{
		cdc_ready(0);
	}
	return 1;
}

int sceCdMmode(int media)
{
	(void)media;

	return 1;
}

int sceCdPause()
{
	return cdc_pause(Cdic.done);
}

int sceCdRead(u32 lbn, u32 sectors, void *buffer, sceCdRMode *mode)
{
	if ( !mode || !buffer || ((uiptr)buffer & 3) != 0 )
	{
		return cdc_seterrno(34);
	}
	if ( !sectors )
		return cdc_seterrno(0);
	return cdc_read(lbn, buffer, sectors, mode, 0, Cdic.done);
}

int sceCdSync(int mode)
{
	return cdc_sync(mode != 0);
}

int sceCdSearchFile(sceCdlFILE *file, const char *name)
{
	int namlen;

	namlen = 0;
	if ( file && name )
	{
		const char *v3;

		v3 = name;
		while ( 1 )
		{
			if ( *v3 == 0 )
				break;
			v3 = &name[++namlen];
			if ( namlen >= 1024 )
			{
				break;
			}
		}
		if ( namlen >= 1024 )
			namlen = 0;
	}
	if ( !namlen )
		return cdc_seterrno(34);
	return cdc_lookup(file, name, namlen, cdi_xfer);
}

int sceCdSeek(u32 lbn)
{
	return cdc_seek(lbn, Cdic.done);
}

int sceCdStandby()
{
	return cdc_standby(Cdic.done);
}

int sceCdStatus()
{
	return cdc_stat();
}

int sceCdStop()
{
	return cdc_stop(Cdic.done);
}

int sceCdTrayReq(int param, u32 *traychk)
{
	return cdc_tray(param, traychk);
}

int sceCdStInit(u32 bufmax, u32 bankmax, void *buffer)
{
	unsigned int v4;

	if ( !bufmax || !bankmax || !buffer || ((uiptr)buffer & 3) != 0 )
		return cdc_seterrno(34);
	v4 = bufmax / bankmax;
	if ( !(bufmax / bankmax) )
		v4 = 1;
	return cdc_inits((void *)buffer, bufmax, v4);
}

int sceCdStRead(u32 sectors, u32 *buffer, u32 mode, u32 *error)
{
	if ( mode >= 2 )
		buffer = 0;
	if ( !buffer || ((uiptr)buffer & 3) != 0 || !error )
	{
		if ( error )
			*error = 34;
		return cdc_seterrno(34);
	}
	if ( !sectors )
	{
		*error = 0;
		return 0;
	}
	return cdc_reads(buffer, sectors, mode, (int *)error, cdc_xfer);
}

int sceCdStSeek(u32 lbn)
{
	return cdc_seeks(lbn);
}

int sceCdStStart(u32 lbn, sceCdRMode *mode)
{
	if ( !mode )
		return cdc_seterrno(34);
	return cdc_starts(lbn, mode);
}

int sceCdStStat()
{
	return cdc_stats();
}

int sceCdStStop()
{
	return cdc_stops();
}

int sceCdStPause()
{
	return cdc_pauses();
}

int sceCdStResume()
{
	return cdc_resumes();
}
