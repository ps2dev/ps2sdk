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
#include <usbmload.h>

#ifdef _IOP
IRX_ID("USB_module_loader", 2, 1);
#endif
// Based on the module from SCE SDK 3.1.0.

struct usbm_load_entry
{
	struct usbm_load_entry *m_next;
	char *m_devicename;
	int m_vendor;
	int m_product;
	int m_release;
	int m_class;
	int m_subclass;
	int m_protocol;
	char *m_category;
	char *m_driverpath;
	char *m_driverargs[8];
	int m_driverargs_len;
	int m_pad1[1];
	int m_ldd_module_id;
	int m_pad2[15];
};

static int module_unload(void);
static int do_parse_config_file(const char *fn);
static void do_print_device_config_info(USBDEV_t *devinfo);
static int usbmload_drv_probe(int dev_id);
static int usbmload_drv_connect(int dev_id);
static int usbmload_drv_disconenct(int dev_id);
static void ldd_loader_thread(void *userdata);
static void default_loadfunc(sceUsbmlPopDevinfo pop_devinfo);
static void do_push_device_rb(USBDEV_t *devinfo);
static USBDEV_t *is_rb_ok_callback(void);
static void do_clear_rb(void);
static int split_config_line(char *curbuf, int cursplitind, char **dstptr);
static int do_parse_cmd_int(const char *buf);
static void clean_config_line(char *buf);
static void sanitize_devicename(char *buf);
static void init_config_pos(void);
static char *read_config_line(char *dstbuf, int maxlen, int fd);

extern struct irx_export_table _exp_usbmload;
static sceUsbdLddOps g_usbmload_drv = {
	NULL,
	NULL,
	"usbmload",
	&usbmload_drv_probe,
	&usbmload_drv_connect,
	&usbmload_drv_disconenct,
	0u,
	0u,
	0u,
	0u,
	0u,
	NULL};
// Unofficial: move to bss
static int g_config_chr_pos;
static int g_param_conffile[128];
static char g_config_line_buf[256];
static char g_config_device_name_tmp[256];
static int g_param_debug;
static int g_usbmload_enabled;
static int g_param_rbsize;
static int g_rb_offset_read;
static int g_rb_offset_write;
static int g_rb_count;
static USBDEV_t **g_rb_entries;
static USBDEV_t *g_usbm_entry_list_end;
static USBDEV_t *g_usbm_entry_list_cur;
static int g_ef;
static int g_thid;
static sceUsbmlLoadFunc g_loadfunc_cb;
static char g_config_chr_buf[2048];

int _start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	int has_conffile;
	int i;
	int j;
	int thid1;
	iop_event_t efparam;
	iop_thread_t thparam;
	int state;

	(void)startaddr;
	has_conffile = 0;
	if ( ac < 0 )
		return module_unload();
	printf("----- USB auto module loader %s -----\n", "0.4.0");
	g_param_rbsize = 32;
	g_loadfunc_cb = default_loadfunc;
	g_param_debug = 0;
	g_usbmload_enabled = 0;
	g_rb_offset_read = 0;
	g_rb_offset_write = 0;
	g_rb_count = 0;
	g_usbm_entry_list_end = 0;
	g_usbm_entry_list_cur = 0;
	for ( i = 1; i < ac; i += 1 )
	{
		for ( j = 0; av[i][j] && av[i][j] != '='; j += 1 )
			;
		if ( av[i][j] )
		{
			av[i][j] = 0;
			j += 1;
		}
		if ( !strcmp(av[i], "conffile") )
		{
			if ( (unsigned int)strlen(&(av[i])[j]) < 0x200 )
			{
				strcpy((char *)g_param_conffile, &(av[i])[j]);
				has_conffile = 1;
				if ( g_param_debug > 0 )
					printf("conffile=%s\n", g_param_conffile);
			}
			else
			{
				printf("Too long file name : %s\n", &(av[i])[j]);
			}
		}
		else if ( !strcmp(av[i], "debug") )
		{
			g_param_debug = do_parse_cmd_int(&(av[i])[j]);
			printf("Debug level is %d\n", g_param_debug);
		}
		else if ( !strcmp(av[i], "rbsize") )
		{
			g_param_rbsize = do_parse_cmd_int(&(av[i])[j]);
			g_param_rbsize = (g_param_rbsize > 256) ? 256 : g_param_rbsize;
			g_param_rbsize = (g_param_rbsize < 8) ? 8 : g_param_rbsize;
			if ( g_param_debug > 0 )
				printf("usbmload : ring buffer size = %d\n", g_param_rbsize);
		}
	}
	if ( g_param_debug > 0 )
		printf("allocsize(for ring buffer) : %d\n", (int)(sizeof(USBDEV_t *) * g_param_rbsize));
	CpuSuspendIntr(&state);
	g_rb_entries = (USBDEV_t **)AllocSysMemory(0, sizeof(USBDEV_t *) * g_param_rbsize, 0);
	CpuResumeIntr(state);
	if ( !g_rb_entries )
	{
		printf("Ring buffer Initialize Error!!\n");
		return MODULE_NO_RESIDENT_END;
	}
	if ( RegisterLibraryEntries(&_exp_usbmload) )
		return MODULE_NO_RESIDENT_END;
	if ( has_conffile == 1 )
	{
		if ( do_parse_config_file((const char *)g_param_conffile) == -1 )
		{
			printf("usbmload : load_config NG\n");
		}
		else
		{
			if ( g_param_debug > 0 )
				printf("usbmload : load_config OK\n");
		}
	}
	memset(&efparam, 0, sizeof(efparam));
	g_ef = CreateEventFlag(&efparam);
	if ( g_ef < 0 )
	{
		printf("usbmload :  CreateEventFlag NG\n");
		return MODULE_NO_RESIDENT_END;
	}
	thparam.attr = TH_C;
	thparam.thread = ldd_loader_thread;
	thparam.priority = 88;
	thparam.stacksize = 4096;
	thparam.option = 0;
	thid1 = CreateThread(&thparam);
	if ( thid1 <= 0 )
	{
		printf("usbmload : CreateThread NG\n");
		DeleteEventFlag(g_ef);
		return MODULE_NO_RESIDENT_END;
	}
	if ( g_param_debug > 0 )
		printf("usbmload : CreateThread ID = %d\n", thid1);
	StartThread(thid1, 0);
	g_thid = thid1;
#if 0
  return MODULE_REMOVABLE_END;
#else
	if ( mi && ((mi->newflags & 2) != 0) )
		mi->newflags |= 0x10;
	return MODULE_RESIDENT_END;
#endif
}

static int module_unload(void)
{
	USBDEV_t *listent_1;
	int i;
	int stopres;
	int state;

	if ( ReleaseLibraryEntries(&_exp_usbmload) )
	{
		return MODULE_REMOVABLE_END;
	}
	sceUsbmlDisable();
	TerminateThread(g_thid);
	DeleteThread(g_thid);
	DeleteEventFlag(g_ef);
	listent_1 = g_usbm_entry_list_end;
	while ( listent_1 )
	{
		USBDEV_t *listent_3;

		if ( StopModule(listent_1->modid, 0, 0, &stopres) >= 0 )
		{
			if ( g_param_debug > 0 )
				printf("usbmload : Unload LDD module (%xh)\n", listent_1->modid);
			UnloadModule(listent_1->modid);
		}
		CpuSuspendIntr(&state);
		for ( i = 0; i < listent_1->argc; i += 1 )
		{
			FreeSysMemory(listent_1->argv[i]);
		}
		FreeSysMemory(listent_1->dispname);
		FreeSysMemory(listent_1->category);
		FreeSysMemory(listent_1->path);
		listent_3 = listent_1;
		listent_1 = listent_1->forw;
		FreeSysMemory(listent_3);
		CpuResumeIntr(state);
	}
	CpuSuspendIntr(&state);
	FreeSysMemory(g_rb_entries);
	CpuResumeIntr(state);
	return MODULE_NO_RESIDENT_END;
}

static int do_parse_config_file(const char *fn)
{
	USBDEV_t *devstr;
	int has_encountered;
	int fd;
	int lineind;
	char *p[2];
	int state;
	int err;

	err = 0;
	devstr = 0;
	init_config_pos();
	has_encountered = 0;
	if ( g_param_debug > 0 )
		printf("open '%s'\n", fn);
	fd = open(fn, 1);
	lineind = 0;
	if ( fd < 0 )
	{
		printf("Cannot open '%s'\n", fn);
		return -1;
	}
	while ( read_config_line(g_config_line_buf, sizeof(g_config_line_buf), fd) )
	{
		int tokencnt;

		lineind += 1;
		if ( g_param_debug >= 2 )
		{
			strcpy(g_config_device_name_tmp, g_config_line_buf);
			sanitize_devicename(g_config_device_name_tmp);
			printf("%4d : %s\n", lineind, g_config_device_name_tmp);
		}
		clean_config_line(g_config_line_buf);
		tokencnt = split_config_line(g_config_line_buf, 3, p);
		if ( tokencnt )
		{
			if ( !strcmp(p[0], "end") )
				break;
			if ( tokencnt >= 2 )
			{
				if ( !strcmp(p[0], "DeviceName") )
				{
					if ( devstr )
					{
						if ( g_usbm_entry_list_cur )
							g_usbm_entry_list_cur->forw = devstr;
						else
							g_usbm_entry_list_end = devstr;
						devstr->forw = 0;
						g_usbm_entry_list_cur = devstr;
						if ( g_param_debug > 0 )
						{
							printf("Resistered\n");
							do_print_device_config_info(devstr);
						}
					}
					CpuSuspendIntr(&state);
					devstr = (USBDEV_t *)AllocSysMemory(0, sizeof(USBDEV_t), 0);
					CpuResumeIntr(state);
					if ( !devstr )
					{
						err = 1;
						break;
					}
					CpuSuspendIntr(&state);
					devstr->dispname = (char *)AllocSysMemory(0, strlen(p[1]) + 1, 0);
					CpuResumeIntr(state);
					if ( !devstr->dispname )
					{
						err = 1;
						break;
					}
					strcpy(devstr->dispname, p[1]);
					devstr->vendor = -1;
					devstr->product = -1;
					devstr->release = -1;
					devstr->class_ = -1;
					devstr->subclass = -1;
					devstr->protocol = -1;
					devstr->category = 0;
					devstr->path = 0;
					devstr->argc = 0;
					devstr->activate_flag = 0;
					devstr->modid = -1;
					devstr->modname[0] = 0;
					devstr->load_result = -1;
				}
				else if ( !strcmp(p[0], "Use") )
				{
					has_encountered = 1;
					if ( strcmp(p[1], "1") )
					{
						if ( g_param_debug > 0 )
						{
							sanitize_devicename(devstr->dispname);
							printf("Disable '%s'\n", devstr->dispname);
						}
						has_encountered = 0;
						if ( devstr->dispname )
						{
							CpuSuspendIntr(&state);
							FreeSysMemory(devstr->dispname);
							CpuResumeIntr(state);
							devstr->dispname = 0;
						}
						if ( devstr )
						{
							CpuSuspendIntr(&state);
							FreeSysMemory(devstr);
							devstr = 0;
							CpuResumeIntr(state);
						}
					}
				}
				else if ( has_encountered )
				{
					if ( !strcmp(p[0], "Vendor") )
					{
						devstr->vendor = do_parse_cmd_int(p[1]);
					}
					else if ( !strcmp(p[0], "Product") )
					{
						devstr->product = do_parse_cmd_int(p[1]);
					}
					else if ( !strcmp(p[0], "Release") )
					{
						devstr->release = do_parse_cmd_int(p[1]);
					}
					else if ( !strcmp(p[0], "Class") )
					{
						devstr->class_ = do_parse_cmd_int(p[1]);
					}
					else if ( !strcmp(p[0], "SubClass") )
					{
						devstr->subclass = do_parse_cmd_int(p[1]);
					}
					else if ( !strcmp(p[0], "Protocol") )
					{
						devstr->protocol = do_parse_cmd_int(p[1]);
					}
					else if ( !strcmp(p[0], "Category") )
					{
						if ( devstr->category )
						{
							CpuSuspendIntr(&state);
							FreeSysMemory(devstr->category);
							CpuResumeIntr(state);
							devstr->category = 0;
						}
						CpuSuspendIntr(&state);
						devstr->category = (char *)AllocSysMemory(0, strlen(p[1]) + 1, 0);
						CpuResumeIntr(state);
						if ( !devstr->category )
						{
							err = 1;
							break;
						}
						strcpy(devstr->category, p[1]);
					}
					else if ( !strcmp(p[0], "DriverPath") )
					{
						if ( devstr->path )
						{
							CpuSuspendIntr(&state);
							FreeSysMemory(devstr->path);
							CpuResumeIntr(state);
							devstr->path = 0;
						}
						CpuSuspendIntr(&state);
						devstr->path = (char *)AllocSysMemory(0, strlen(p[1]) + 1, 0);
						CpuResumeIntr(state);
						if ( !devstr->path )
						{
							err = 1;
							break;
						}
						strcpy(devstr->path, p[1]);
					}
					else if ( !strcmp(p[0], "DriverArg") && devstr->argc < 8 )
					{
						CpuSuspendIntr(&state);
						devstr->argv[devstr->argc] = (char *)AllocSysMemory(0, strlen(p[1]) + 1, 0);
						CpuResumeIntr(state);
						if ( !devstr->argv[devstr->argc] )
						{
							err = 1;
							break;
						}
						strcpy(devstr->argv[devstr->argc], p[1]);
						devstr->argc += 1;
					}
					else
					{
						if ( g_param_debug > 0 )
							printf("%s : %d : Illegal parameter '%s'\n", fn, lineind, p[0]);
					}
				}
			}
		}
	}
	if ( err )
	{
		printf("%s : %d : malloc error\n", fn, lineind);
		close(fd);
		return -1;
	}
	if ( devstr )
	{
		if ( g_usbm_entry_list_cur )
			g_usbm_entry_list_cur->forw = devstr;
		else
			g_usbm_entry_list_end = devstr;
		devstr->forw = 0;
		g_usbm_entry_list_cur = devstr;
		if ( g_param_debug > 0 )
		{
			printf("Resistered\n");
			do_print_device_config_info(devstr);
		}
	}
	close(fd);
	return 0;
}

static void do_print_device_config_info(USBDEV_t *devinfo)
{
	int i;
	char dispname_tmp[256];

	strcpy(dispname_tmp, devinfo->dispname);
	sanitize_devicename(dispname_tmp);
	printf(" DeviceName:%s\n", dispname_tmp);
	printf(" Vendor    :%04X\n", devinfo->vendor);
	printf(" Product   :%04X\n", devinfo->product);
	printf(" Release   :%04X\n", devinfo->release);
	printf(" Class     :%02X\n", devinfo->class_);
	printf(" SubClass  :%02X\n", devinfo->subclass);
	printf(" Protocol  :%02X\n", devinfo->protocol);
	printf(" Category  :%s\n", devinfo->category);
	printf(" DriverPath:%s\n", devinfo->path);
	for ( i = 0; i < devinfo->argc; i += 1 )
	{
		printf(" DriverArg%d:%s\n", i, devinfo->argv[i]);
	}
	printf("\n");
}

static int usbmload_drv_probe(int dev_id)
{
	UsbDeviceDescriptor *devdesc;
	const UsbInterfaceDescriptor *intfdesc;
	int found_info_count;
	USBDEV_t *devinfo;

	if ( g_param_debug > 0 )
		printf("Call usbmload_probe\n");
	devdesc = (UsbDeviceDescriptor *)sceUsbdScanStaticDescriptor(dev_id, 0, 1u);
	if ( !devdesc )
		return 0;
	intfdesc = (UsbInterfaceDescriptor *)sceUsbdScanStaticDescriptor(dev_id, devdesc, 4u);
	if ( !intfdesc )
		return 0;
	found_info_count = 0;
	if ( !g_usbm_entry_list_end )
	{
		if ( g_param_debug > 0 )
			printf("usbmload : Not registered\n");
		return 0;
	}
	for ( devinfo = g_usbm_entry_list_end; devinfo; devinfo = devinfo->forw )
	{
		if (
			devinfo->activate_flag && (devinfo->vendor == devdesc->idVendor || devinfo->vendor == -1)
			&& (devinfo->product == devdesc->idProduct || devinfo->product == -1)
			&& (devinfo->release == devdesc->bcdUSB || devinfo->release == -1)
			&& (devinfo->class_ == intfdesc->bInterfaceClass || devinfo->class_ == -1)
			&& (devinfo->protocol == intfdesc->bInterfaceProtocol || devinfo->protocol == -1)
			&& (devinfo->subclass == intfdesc->bInterfaceSubClass || devinfo->subclass == -1) )
		{
			if ( g_param_debug > 0 )
				printf("push_devinfo : %s\n", devinfo->path);
			do_push_device_rb(devinfo);
			found_info_count += 1;
		}
	}
	if ( found_info_count )
	{
		if ( g_param_debug > 0 )
			printf("SetEventFlag\n");
		SetEventFlag(g_ef, 1u);
	}
	return 0;
}

static int usbmload_drv_connect(int dev_id)
{
	(void)dev_id;

	return -1;
}

static int usbmload_drv_disconenct(int dev_id)
{
	(void)dev_id;

	return 0;
}

static void ldd_loader_thread(void *userdata)
{
	u32 efres;

	(void)userdata;
	while ( 1 )
	{
		WaitEventFlag(g_ef, 1u, WEF_OR | WEF_CLEAR, &efres);
		if ( g_param_debug > 0 )
			printf("ldd_loader_thread : get event!\n");
		g_loadfunc_cb(is_rb_ok_callback);
		do_clear_rb();
	}
}

static void default_loadfunc(sceUsbmlPopDevinfo pop_devinfo)
{
	int modid;
	int i;
	unsigned int cur_argv_len;
	unsigned int cur_argv_len_1;
	char modarg[256];
	ModuleStatus modstat;
	int modres;

	modid = -1;
	if ( g_param_debug > 0 )
		printf("Entering default_loadfunc()\n");
	while ( 1 )
	{
		USBDEV_t *curdev;

		curdev = pop_devinfo();
		if ( !curdev )
			break;
		if ( curdev->modid < 0 || ReferModuleStatus(modid, &modstat) || strcmp(modstat.name, curdev->modname) )
		{
			if ( g_param_debug > 0 )
			{
				do_print_device_config_info(curdev);
			}
			cur_argv_len = 0;
			for ( i = 0; i < curdev->argc; i += 1 )
			{
				cur_argv_len_1 = cur_argv_len + strlen(curdev->argv[i]) + 1;
				if ( cur_argv_len_1 > (int)(sizeof(modarg) - 16) )
					break;
				cur_argv_len = cur_argv_len_1;
				strcpy(&modarg[cur_argv_len], curdev->argv[i]);
			}
			strcpy(&modarg[cur_argv_len], "lmode=AUTOLOAD");
			if ( g_param_debug > 0 )
				printf("LoadStartModule : %s\n", curdev->path);
			modid = LoadStartModule(curdev->path, cur_argv_len + strlen("lmode=AUTOLOAD") + 1, modarg, &modres);
			if ( g_param_debug > 0 )
				printf("LoadStartModule done: %d\n", modid);
			if ( modid >= 0 )
			{
				curdev->modid = modid;
				curdev->load_result = modres;
				ReferModuleStatus(modid, &modstat);
				strcpy(curdev->modname, modstat.name);
			}
		}
	}
}

static void do_push_device_rb(USBDEV_t *devinfo)
{
	int state;

	CpuSuspendIntr(&state);
	if ( g_rb_count < g_param_rbsize )
	{
		g_rb_entries[g_rb_offset_write] = devinfo;
		g_rb_offset_write += 1;
		if ( g_rb_offset_write >= g_param_rbsize )
			g_rb_offset_write = 0;
		g_rb_count += 1;
	}
	CpuResumeIntr(state);
}

static USBDEV_t *is_rb_ok_callback(void)
{
	USBDEV_t *devinfo;
	int state;

	CpuSuspendIntr(&state);
	devinfo = NULL;
	if ( g_rb_count )
	{
		devinfo = g_rb_entries[g_rb_offset_read];
		g_rb_offset_read += 1;
		if ( g_rb_offset_read >= g_param_rbsize )
			g_rb_offset_read = 0;
		g_rb_count -= 1;
	}
	CpuResumeIntr(state);
	return devinfo;
}

static void do_clear_rb(void)
{
	int state;

	CpuSuspendIntr(&state);
	g_rb_count = 0;
	CpuResumeIntr(state);
}

static int split_config_line(char *curbuf, int cursplitind, char **dstptr)
{
	char *curbuf_1;
	char *curbuf_2;
	int splitfound;
	char **dstptr_1;
	int i;

	curbuf_1 = curbuf;
	curbuf_2 = curbuf;
	splitfound = 0;
	if ( cursplitind <= 0 || !strlen(curbuf) )
		return 0;
	dstptr_1 = dstptr;
	while ( splitfound != cursplitind )
	{
		for ( i = 0; curbuf_1[i] == ' ' || curbuf_1[i] == '\t'; i += 1 )
			;
		curbuf_2 += i;
		if ( !curbuf_1[i] || curbuf_1[i] == '\r' || curbuf_1[i] == '\n' )
		{
			curbuf_1[i] = 0;
			break;
		}
		*dstptr_1 = curbuf_2;
		splitfound += 1;
		if ( curbuf_1[i] != '"' )
		{
			for ( ; curbuf_1[i] && curbuf_1[i] != '\r' && curbuf_1[i] != '\n' && curbuf_1[i] != ' ' && curbuf_1[i] != '\t';
						i += 1 )
			{
				curbuf_2 += 1;
			}
		}
		else
		{
			i += 1;
			curbuf_2 += 1;
			*dstptr_1 = curbuf_2;
			if ( !curbuf_1[i] )
			{
				curbuf_1[i] = 0;
				break;
			}
			for ( ; curbuf_1[i] && curbuf_1[i] != '\r' && curbuf_1[i] != '\n' && curbuf_1[i] != '"'; i += 1 )
			{
				curbuf_2 += 1;
			}
		}
		dstptr_1 += 1;
		if ( !curbuf_1[i] || curbuf_1[i] == '\r' || curbuf_1[i] == '\n' )
		{
			curbuf_1[i] = 0;
			break;
		}
		curbuf_2 += 1;
		curbuf_1[i] = 0;
		curbuf_1 = curbuf_2;
	}
	return splitfound;
}

static int do_parse_cmd_int(const char *buf)
{
	int hexval;
	const char *i;

	hexval = 0;
	if ( *buf == '*' )
		return -1;
	if ( *buf != '0' || buf[1] != 'x' )
		return strtol(buf, 0, 10);
	for ( i = buf + 2; *i; i += 1 )
	{
		hexval = (16 * hexval) + (*i & 0xF) + ((*i >= ':') ? 9 : 0);
	}
	return hexval;
}

static void clean_config_line(char *buf)
{
	int in_quotes;

	in_quotes = 0;
	for ( ; *buf != '\n' && *buf != '\r' && *buf; buf += 1 )
	{
		if ( *buf == '"' )
			in_quotes = !in_quotes;
		if ( *buf == '#' && !in_quotes )
			break;
	}
	*buf = 0;
}

static void sanitize_devicename(char *buf)
{
	while ( *buf && *buf != '\n' && *buf != '\r' )
	{
		unsigned int curchr_2;
		unsigned int curchr_3;

		curchr_2 = (u8)buf[0];
		curchr_3 = (u8)buf[1];
		if (
			curchr_2 < 0x80 || (char)curchr_2 - 0xA0 < 0x40 || (char)curchr_2 - 0xF0 < 0x10 || curchr_3 < 0x40
			|| curchr_3 == 0x7F || curchr_3 - 0xFD < 3 )
		{
			buf += 1;
		}
		else
		{
			if ( curchr_3 >= 0x9F )
			{
				buf[0] = curchr_2 >= 0xA0 ? 2 * curchr_2 + 32 : 2 * curchr_2 - '`';
				buf[1] = curchr_3 + 2;
			}
			else
			{
				if ( curchr_3 >= 0x80 )
					curchr_3 = curchr_3 - 1;
				buf[0] = curchr_2 >= 0xA0 ? 2 * curchr_2 + 31 : 2 * curchr_2 - 'a';
				buf[1] = curchr_3 + 'a';
			}
			buf += 2;
		}
	}
}

int sceUsbmlDisable(void)
{
	int unregres;

	unregres = sceUsbdUnregisterAutoloader();
	if ( unregres )
	{
		printf("sceUsbmlDisable:Error(0x%X)\n", unregres);
		return -1;
	}
	g_usbmload_enabled = 0;
	return 0;
}

int sceUsbmlEnable(void)
{
	int regres;

	sceUsbdUnregisterAutoloader();
	regres = sceUsbdRegisterAutoloader(&g_usbmload_drv);
	if ( regres )
	{
		printf("sceUsbmlEnable:Error(0x%X)\n", regres);
		return -1;
	}
	g_usbmload_enabled = 1;
	return 0;
}

int sceUsbmlActivateCategory(const char *category)
{
	USBDEV_t *devinfo;
	int i;

	i = 0;
	for ( devinfo = g_usbm_entry_list_end; devinfo; devinfo = devinfo->forw )
	{
		if ( !strcmp(devinfo->category, category) )
		{
			devinfo->activate_flag = 1;
			i += 1;
		}
	}
	if ( g_usbmload_enabled == 1 )
		sceUsbmlEnable();
	return i ? i : -1;
}

int sceUsbmlInactivateCategory(const char *category)
{
	USBDEV_t *devinfo;
	int i;

	i = 0;
	for ( devinfo = g_usbm_entry_list_end; devinfo; devinfo = devinfo->forw )
	{
		if ( !strcmp(devinfo->category, category) )
		{
			devinfo->activate_flag = 0;
			i += 1;
		}
	}
	if ( g_usbmload_enabled == 1 )
		sceUsbmlEnable();
	return i ? i : -1;
}

int sceUsbmlRegisterLoadFunc(sceUsbmlLoadFunc loadfunc)
{
	if ( g_loadfunc_cb == default_loadfunc )
	{
		g_loadfunc_cb = loadfunc;
		return 0;
	}
	return -1;
}

void sceUsbmlUnregisterLoadFunc(void)
{
	g_loadfunc_cb = default_loadfunc;
}

int sceUsbmlLoadConffile(const char *conffile)
{
	return do_parse_config_file(conffile);
}

int sceUsbmlRegisterDevice(USBDEV_t *device)
{
	USBDEV_t *devinfo;
	int i;
	int state;
	int failed;

	failed = 0;
	CpuSuspendIntr(&state);
	devinfo = (USBDEV_t *)AllocSysMemory(0, sizeof(USBDEV_t), 0);
	CpuResumeIntr(state);
	if ( !devinfo )
	{
		failed = 1;
	}
	if ( !failed )
	{
		// The following memcpy was inlined
		memcpy(devinfo, device, sizeof(USBDEV_t));
		CpuSuspendIntr(&state);
		devinfo->dispname = (char *)AllocSysMemory(0, strlen(device->dispname) + 1, 0);
		CpuResumeIntr(state);
		if ( !devinfo->dispname )
		{
			failed = 2;
		}
	}
	if ( !failed )
	{
		strcpy(devinfo->dispname, device->dispname);
		CpuSuspendIntr(&state);
		devinfo->path = (char *)AllocSysMemory(0, strlen(device->path) + 1, 0);
		CpuResumeIntr(state);
		if ( !devinfo->path )
		{
			failed = 3;
		}
	}
	if ( !failed )
	{
		strcpy(devinfo->path, device->path);
		for ( i = 0; i < device->argc; i += 1 )
		{
			CpuSuspendIntr(&state);
			devinfo->argv[i] = (char *)AllocSysMemory(0, strlen(device->argv[i]) + 1, 0);
			CpuResumeIntr(state);
			if ( !devinfo->argv[i] )
			{
				failed = 4;
				break;
			}
			strcpy(devinfo->argv[i], device->argv[i]);
		}
	}
	if ( failed )
	{
		printf("sceUsbmlRegisterDevice : malloc error%d\n", failed);
		if ( failed >= 2 )
			CpuSuspendIntr(&state);
		if ( failed >= 4 )
		{
			int j;

			for ( j = 0; j < i; j += 1 )
			{
				FreeSysMemory(devinfo->argv[j]);
			}
			FreeSysMemory(devinfo->path);
		}
		if ( failed >= 3 )
		{
			FreeSysMemory(devinfo->dispname);
		}
		if ( failed >= 2 )
		{
			FreeSysMemory(devinfo);
			CpuResumeIntr(state);
		}
		return -1;
	}
	if ( g_usbm_entry_list_cur )
		g_usbm_entry_list_cur->forw = devinfo;
	else
		g_usbm_entry_list_end = devinfo;
	devinfo->forw = 0;
	g_usbm_entry_list_cur = devinfo;
	return 0;
}

int sceUsbmlChangeThreadPriority(int prio1)
{
	return ChangeThreadPriority(g_thid, prio1) ? -1 : 0;
}

static void init_config_pos(void)
{
	g_config_chr_pos = sizeof(g_config_chr_buf);
}

static char read_config_byte(int fd)
{
	char tmpret;

	if ( g_config_chr_pos == sizeof(g_config_chr_buf) )
	{
		read(fd, g_config_chr_buf, sizeof(g_config_chr_buf));
		g_config_chr_pos = 0;
	}
	tmpret = g_config_chr_buf[g_config_chr_pos];
	g_config_chr_pos += 1;
	return tmpret;
}

static char *read_config_line(char *dstbuf, int maxlen, int fd)
{
	int i;

	for ( i = 0; i < maxlen; i += 1 )
	{
		dstbuf[i] = read_config_byte(fd);
		if ( dstbuf[i] == '\n' )
			break;
		dstbuf[i] = dstbuf[i] == '\r' ? 0 : dstbuf[i];
	}
	dstbuf[i] = 0;
	return dstbuf;
}
