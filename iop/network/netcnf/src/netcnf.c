/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifdef _IOP
#include "irx_imports.h"
#else
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef intptr_t siptr;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uintptr_t uiptr;
#define iomanX_rename(old, new_) rename(old, new_)
#define iomanX_sync(...) 0
typedef struct
{
	unsigned int mode;
	unsigned int attr;
	unsigned int size;
	unsigned char ctime[8];
	unsigned char atime[8];
	unsigned char mtime[8];
	unsigned int hisize;
	unsigned int private_0;
	unsigned int private_1;
	unsigned int private_2;
	unsigned int private_3;
	unsigned int private_4;
	unsigned int private_5;
} iox_stat_t;
typedef struct
{
	iox_stat_t stat;
	char name[256];
	void *privdata;
} iox_dirent_t;
#endif
#include <errno.h>
#include <netcnf.h>

#ifdef _IOP
IRX_ID("NET_configuration", 2, 30);
#endif
// Based on the module from SCE SDK 3.1.0.

struct netcnf_callback_handle_info
{
	int m_fd;
	char m_device[16];
	char m_pathname[256];
	void *m_buf;
	int m_filesize;
	int m_bufpos;
	int m_allocstate;
};

struct netcnf_option
{
	int m_type;
	int m_offset;
	const char *m_key;
};

static void do_init_xor_magic(const char *in_id_buf);
static int magic_shift_write_netcnf_2(int inshft, int buflen);
static int magic_shift_read_netcnf_2(int inshft, int buflen);
static int magic_shift_write_netcnf_1(int inshft, int buflen);
static int magic_shift_read_netcnf_1(int inshft, int buflen);
static int do_check_capacity_inner(const char *fpath);
static int do_get_count_list_inner(const char *fname, int type, sceNetCnfList_t *p);
static int do_load_entry_inner(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e);
static int do_add_entry_inner(
	const char *fname,
	int type,
	const char *usr_name,
	sceNetCnfEnv_t *e,
	const char *icon_value,
	const char *iconsys_value,
	int no_check_capacity);
static int do_edit_entry_inner(
	const char *fname,
	int type,
	const char *usr_name,
	const char *new_usr_name,
	sceNetCnfEnv_t *e,
	const char *icon_value,
	const char *iconsys_value,
	int no_check_capacity);
static int do_delete_entry_inner(
	const char *fname,
	int type,
	const char *usr_name,
	const char *icon_value,
	const char *iconsys_value,
	int no_check_capacity);
static int do_set_latest_entry_inner(const char *fname, int type, const char *usr_name);
static int do_delete_all_inner(const char *dev);
static int do_check_special_provider_inner(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e);
static char *do_alloc_mem_inner(sceNetCnfEnv_t *e, unsigned int size, int align);
static void do_init_ifc_inner(sceNetCnfInterface_t *ifc);
static int do_merge_conf_inner(sceNetCnfEnv_t *e);
static int do_load_conf_inner(sceNetCnfEnv_t *e);
static int do_load_dial_inner(sceNetCnfEnv_t *e, sceNetCnfPair_t *pair);
static int do_export_netcnf(sceNetCnfEnv_t *e);
static void do_address_to_string_inner(char *dst, unsigned int srcint);
static int do_name_2_address_inner(unsigned int *dst, const char *buf);
static int do_conv_a2s_inner(char *sp_, char *dp_, int len);
static int do_conv_s2a_inner(char *sp_, char *dp_, int len);
static int do_read_check_netcnf(const char *netcnf_path, int type, int no_check_magic, int no_decode);
static int do_check_provider_inner(const sceNetCnfEnv_t *e, int type);
static const char *do_handle_netcnf_dirname(const char *fpath, const char *entry_buffer, char *netcnf_file_path);
static int do_open_netcnf(const char *netcnf_path, int file_flags, int file_mode);
static int do_readfile_netcnf(int fd, void *ptr, int size);
static int do_write_netcnf_no_encode(int fd, void *ptr, int size);
static int do_dopen_wrap(const char *fn);
static int do_dread_wrap(int fn, iox_dirent_t *buf);
static int do_remove_wrap(const char *fn);
static void do_close_netcnf(int fd);
static void do_dclose_wrap(int fd);
static int do_filesize_netcnf(int fd);
static void do_getstat_wrap(const char *fn, iox_stat_t *stx);
static void do_chstat_mode_copyprotect_wrap(const char *fn);
static void do_set_callback_inner(sceNetCnfCallback_t *pcallback);
#ifdef _IOP
static int do_init_heap(void);
#endif
static void *do_alloc_heapmem(int nbytes);
static void do_free_heapmem(void *ptr);
#ifdef _IOP
static void do_delete_heap(void);
#endif

#ifdef _IOP
extern struct irx_export_table _exp_netcnf;
#endif
// Unofficial: move to bss
static int g_no_check_capacity;
// Unofficial: move to bss
static int g_no_check_provider;
// Unofficial: move to bss
static u32 g_id_result;
// Unofficial: move to bss
static char *g_count_list_heapptr;
// Unofficial: move to bss
static char *g_load_entry_heapptr;
// Unofficial: move to bss
static char *g_add_entry_heapptr;
// Unofficial: move to bss
static char *g_edit_entry_heapptr;
// Unofficial: move to bss
static char *g_delete_entry_heapptr;
// Unofficial: move to bss
static char *g_set_latest_entry_heapptr;
// Unofficial: move to bss
static char *g_check_special_provider_heapptr;
static const struct netcnf_option g_options_net_cnf[] = {
	{112, 12, "chat_additional"},
	{52, 16, "redial_count"},
	{52, 20, "redial_interval"},
	{112, 24, "outside_number"},
	{112, 28, "outside_delay"},
	{68, 32, "dialing_type"},
	{0, 0, NULL}};
static const struct netcnf_option g_options_attach_cnf[] = {
	{84, 0, "type"},
	{112, 4, "vendor"},
	{112, 8, "product"},
	{112, 12, "location"},
	{98, 16, "dhcp"},
	{112, 20, "dhcp_host_name"},
	{98, 24, "dhcp_host_name_null_terminated"},
	{98, 25, "dhcp_release_on_stop"},
	{112, 28, "address"},
	{112, 32, "netmask"},
	{112, 36, "chat_additional"},
	{52, 40, "redial_count"},
	{52, 44, "redial_interval"},
	{112, 48, "outside_number"},
	{112, 52, "outside_delay"},
	{98, 96, "answer_mode"},
	{52, 100, "answer_timeout"},
	{68, 104, "dialing_type"},
	{112, 108, "chat_login"},
	{112, 112, "auth_name"},
	{112, 116, "auth_key"},
	{112, 120, "peer_name"},
	{112, 124, "peer_key"},
	{52, 128, "lcp_timeout"},
	{52, 132, "ipcp_timeout"},
	{52, 136, "idle_timeout"},
	{52, 140, "connect_timeout"},
	{98, 144, "want.mru_nego"},
	{98, 145, "want.accm_nego"},
	{98, 146, "want.magic_nego"},
	{98, 147, "want.prc_nego"},
	{98, 148, "want.acc_nego"},
	{98, 149, "want.address_nego"},
	{98, 150, "want.vjcomp_nego"},
	{98, 151, "want.dns1_nego"},
	{98, 152, "want.dns2_nego"},
	{77, 160, "want.mru"},
	{67, 164, "want.accm"},
	{65, 168, "want.auth"},
	{112, 172, "want.ip_address"},
	{112, 176, "want.ip_mask"},
	{112, 180, "want.dns1"},
	{112, 184, "want.dns2"},
	{98, 220, "allow.mru_nego"},
	{98, 221, "allow.accm_nego"},
	{98, 222, "allow.magic_nego"},
	{98, 223, "allow.prc_nego"},
	{98, 224, "allow.acc_nego"},
	{98, 225, "allow.address_nego"},
	{98, 226, "allow.vjcomp_nego"},
	{98, 227, "allow.dns1_nego"},
	{98, 228, "allow.dns2_nego"},
	{77, 236, "allow.mru"},
	{67, 240, "allow.accm"},
	{65, 244, "allow.auth"},
	{112, 248, "allow.ip_address"},
	{112, 252, "allow.ip_mask"},
	{112, 256, "allow.dns1"},
	{112, 260, "allow.dns2"},
	{76, 296, "log_flags"},
	{99, 300, "force_chap_type"},
	{98, 301, "omit_empty_frame"},
	{80, 332, "phy_config"},
	{98, 302, "pppoe"},
	{98, 303, "pppoe_host_uniq_auto"},
	{112, 308, "pppoe_service_name"},
	{112, 312, "pppoe_ac_name"},
	{52, 316, "mtu"},
	{49, 324, "auth_timeout"},
	{49, 321, "lcp_max_terminate"},
	{49, 323, "ipcp_max_terminate"},
	{49, 320, "lcp_max_configure"},
	{49, 322, "ipcp_max_configure"},
	{49, 325, "auth_max_failure"},
	{0, 0, NULL}};
static const struct netcnf_option g_options_dial_cnf[] = {
	{112, 12, "chat_init"}, {112, 16, "chat_dial"}, {112, 20, "chat_answer"}, {112, 24, "redial_string"}, {0, 0, NULL}};
static int g_callbacks_set;
#ifdef _IOP
// Unofficial: move to bss
static void *g_netcnf_heap;
static int g_semid;
#endif
static char g_icon_value[0x100];
static char g_iconsys_value[0x100];
static char g_id_xorbuf[24];
static char g_id_buffer[8];
static char g_ifc_buffer[0x3e8];
static char g_arg_fname[0x400];
static char g_entry_buffer[0x400];
static char g_netcnf_file_path[0x100];
static char g_dir_name[0x100];
static char g_combination_buf1[0x100];
static char g_combination_buf2[0x100];
static char *g_read_check_netcnf_heapptr;
static sceNetCnfCallback_t g_callbacks;
static struct netcnf_callback_handle_info g_callback_handle_infos[4];
static int g_open_callback_handle_count;

static int get_check_provider_eq_zero(void)
{
	return !g_no_check_provider;
}

#ifdef _IOP
static void do_print_usage(void)
{
	printf("Usage: netcnf [<option>] icon=<icon-path> iconsys=<iconsys-path>\n");
	printf("  <option>:\n");
	printf("    -no_check_capacity        do not check capacity\n");
	printf("    -no_check_provider        do not check special provider\n");
}

static int do_module_load(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	int semid;
	int i;
	iop_sema_t semaparam;
	int err;

	(void)startaddr;
	if ( ac < 3 )
	{
		do_print_usage();
		return MODULE_NO_RESIDENT_END;
	}
	err = 0;
	semaparam.attr = SA_THPRI;
	semaparam.initial = 1;
	semaparam.max = 1;
	semaparam.option = 0;
	semid = CreateSema(&semaparam);
	g_semid = semid;
	if ( semid <= 0 )
	{
		printf("netcnf: CreateSema (%d)\n", semid);
		return MODULE_NO_RESIDENT_END;
	}
	g_icon_value[0] = 0;
	g_iconsys_value[0] = 0;
	for ( i = 1; i < ac; i += 1 )
	{
		if ( !strncmp("icon=", av[i], 5) )
		{
			strcpy(g_icon_value, av[i] + 5);
		}
		else if ( !strncmp("iconsys=", av[i], 8) )
		{
			strcpy(g_iconsys_value, av[i] + 8);
		}
		else if ( !strcmp("-no_check_capacity", av[i]) )
		{
			g_no_check_capacity = 1;
		}
		else if ( !strcmp("-no_check_provider", av[i]) )
		{
			g_no_check_provider = 1;
		}
		else
		{
			err = 1;
			break;
		}
	}
	if ( !g_icon_value[0] || !g_iconsys_value[0] )
	{
		err = 1;
	}
	if ( !err )
	{
		int heap_inited;

		heap_inited = do_init_heap();
		if ( heap_inited < 0 )
		{
			printf("netcnf: init_heap(%d)\n", heap_inited);
		}
		else
		{
			int regres;

			regres = RegisterLibraryEntries(&_exp_netcnf);
			if ( !regres )
			{
#if 0
        return MODULE_REMOVABLE_END;
#else
				if ( mi && ((mi->newflags & 2) != 0) )
					mi->newflags |= 0x10;
				return MODULE_RESIDENT_END;
#endif
			}
			printf("netcnf: RegisterLibraryEntries(%d)\n", regres);
			do_delete_heap();
		}
	}
	if ( err == 1 )
	{
		do_print_usage();
	}
	DeleteSema(g_semid);
	return MODULE_NO_RESIDENT_END;
}

static int do_module_unload(void)
{
	int relres;
	int errstate;

	errstate = 0;
	relres = ReleaseLibraryEntries(&_exp_netcnf);
	if ( relres )
	{
		printf("netcnf: ReleaseLibraryEntries (%d)\n", relres);
	}
	else
	{
		errstate = 1;
		relres = DeleteSema(g_semid);
		if ( relres )
		{
			printf("netcnf: DeleteSema (%d)\n", relres);
		}
		else
		{
			errstate = 2;
		}
	}
	if ( errstate == 2 )
	{
		do_delete_heap();
		return MODULE_NO_RESIDENT_END;
	}
	if ( errstate == 1 )
	{
		RegisterLibraryEntries(&_exp_netcnf);
	}
	return MODULE_REMOVABLE_END;
}

int _start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	return (ac >= 0) ? do_module_load(ac, av, startaddr, mi) : do_module_unload();
}
#endif

int sceNetCnfGetCount(const char *fname, int type)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_get_count_list_inner(fname, type, 0);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfGetList(const char *fname, int type, sceNetCnfList_t *p)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_get_count_list_inner(fname, type, p);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfLoadEntry(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_load_entry_inner(fname, type, usr_name, e);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfAddEntry(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_add_entry_inner(fname, type, usr_name, e, g_icon_value, g_iconsys_value, g_no_check_capacity);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfEditEntry(const char *fname, int type, const char *usr_name, const char *new_usr_name, sceNetCnfEnv_t *e)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres =
		do_edit_entry_inner(fname, type, usr_name, new_usr_name, e, g_icon_value, g_iconsys_value, g_no_check_capacity);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfDeleteEntry(const char *fname, int type, const char *usr_name)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_delete_entry_inner(fname, type, usr_name, g_icon_value, g_iconsys_value, g_no_check_capacity);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfSetLatestEntry(const char *fname, int type, const char *usr_name)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_set_latest_entry_inner(fname, type, usr_name);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

void *sceNetCnfAllocMem(sceNetCnfEnv_t *e, int size, int align)
{
	return do_alloc_mem_inner(e, (unsigned int)size, align);
}

int sceNetCnfInitIFC(sceNetCnfInterface_t *ifc)
{
	if ( ifc )
	{
		memset(ifc, 0, sizeof(sceNetCnfInterface_t));
		do_init_ifc_inner(ifc);
	}
	return 0;
}

int sceNetCnfLoadConf(sceNetCnfEnv_t *e)
{
	return do_load_conf_inner(e);
}

int sceNetCnfLoadDial(sceNetCnfEnv_t *e, sceNetCnfPair_t *pair)
{
	return do_load_dial_inner(e, pair);
}

int sceNetCnfMergeConf(sceNetCnfEnv_t *e)
{
	return do_merge_conf_inner(e);
}

int sceNetCnfName2Address(sceNetCnfAddress_t *paddr, const char *buf)
{
	unsigned int paddr_tmp;

	paddr_tmp = 0;
	if ( buf && !do_name_2_address_inner(&paddr_tmp, buf) )
	{
		return -1;
	}
	memset(paddr, 0, sizeof(sceNetCnfAddress_t));
	memcpy(paddr->data, &paddr_tmp, sizeof(paddr_tmp));
	return 0;
}

int sceNetCnfAddress2String(char *buf, int len, const sceNetCnfAddress_t *paddr)
{
	unsigned int buflen;
	char buf_tmp[24];
	unsigned int srcintx;

	if ( paddr->reserved )
	{
		return -1;
	}
	memcpy(&srcintx, paddr->data, sizeof(srcintx));
	do_address_to_string_inner(buf_tmp, srcintx);
	buflen = (u32)strlen(buf_tmp) + 1;
	if ( (unsigned int)len < buflen )
	{
		return -1;
	}
	memcpy(buf, buf_tmp, buflen);
	return 0;
}

int sceNetCnfDeleteAll(const char *dev)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_delete_all_inner(dev);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfCheckCapacity(const char *fname)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_check_capacity_inner(fname);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

int sceNetCnfConvA2S(char *sp_, char *dp_, int len)
{
	int retres;

	retres = do_conv_a2s_inner(sp_, dp_, len);
	if ( retres )
	{
		return retres;
	}
	if ( len < (int)(strlen(sp_) + 1) )
	{
		return -19;
	}
	strcpy(dp_, sp_);
	return 0;
}

int sceNetCnfConvS2A(char *sp_, char *dp_, int len)
{
	int retres;

	retres = do_conv_s2a_inner(sp_, dp_, len);
	if ( retres )
	{
		return retres;
	}
	if ( len < (int)(strlen(sp_) + 1) )
	{
		return -19;
	}
	strcpy(dp_, sp_);
	return 0;
}

int sceNetCnfCheckSpecialProvider(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e)
{
	int retres;

#ifdef _IOP
	WaitSema(g_semid);
#endif
	retres = do_check_special_provider_inner(fname, type, usr_name, e);
#ifdef _IOP
	SignalSema(g_semid);
#endif
	return retres;
}

void sceNetCnfSetCallback(sceNetCnfCallback_t *pcallback)
{
#ifdef _IOP
	WaitSema(g_semid);
#endif
	do_set_callback_inner(pcallback);
#ifdef _IOP
	SignalSema(g_semid);
#endif
}

static int do_read_ilink_id(void)
{
#ifdef _IOP
	int i;

	for ( i = 0; i < 20; i += 1 )
	{
		g_id_result = 0;
		if ( sceCdRI((u8 *)g_id_buffer, &g_id_result) == 1 )
		{
			if ( !g_id_result )
				return 0;
		}
		DelayThread(100000);
	}
	return -13;
#else
	memset(&g_id_buffer, 0, sizeof(g_id_buffer));
	g_id_result = 0;
	return 0;
#endif
}

static int do_read_netcnf_decode(const char *netcnf_path, char **netcnf_heap_ptr)
{
	int result;
	int fd;
	int netcnf_size;
	char *heapmem;
	int xorind1;
	int readres;
	int i;

	*netcnf_heap_ptr = 0;
	result = do_read_ilink_id();
	// cppcheck-suppress knownConditionTrueFalse
	if ( result < 0 )
		return result;
	do_init_xor_magic(g_id_buffer);
	fd = do_open_netcnf(netcnf_path, 1, 0);
	if ( fd < 0 )
	{
		return (fd == -EIO) ? -18 : -3;
	}
	netcnf_size = do_filesize_netcnf(fd);
	if ( netcnf_size < 0 )
	{
		do_close_netcnf(fd);
		return netcnf_size;
	}
	heapmem = (char *)do_alloc_heapmem(netcnf_size + 1);
	*netcnf_heap_ptr = heapmem;
	if ( !heapmem )
	{
		do_close_netcnf(fd);
		return -2;
	}
	xorind1 = 0;
	readres = 0;
	for ( i = 0; i < netcnf_size; i += 2 )
	{
		readres = do_readfile_netcnf(fd, &heapmem[i], 2);
		if ( readres < 0 )
			break;
		*((u16 *)&heapmem[i]) ^= 0xFFFF;
		*((u16 *)&heapmem[i]) = (u16)magic_shift_read_netcnf_1(*((u16 *)&heapmem[i]), (u8)g_id_xorbuf[xorind1 + 2]);
		xorind1 += 1;
		xorind1 = (xorind1 != sizeof(g_id_xorbuf)) ? xorind1 : 0;
	}
	if ( readres >= 0 )
	{
		if ( (netcnf_size & 1) != 0 )
		{
			readres = do_readfile_netcnf(fd, &heapmem[netcnf_size - 1], 1);
			if ( readres >= 0 )
			{
				heapmem[netcnf_size - 1] ^= 0xFF;
				heapmem[netcnf_size - 1] =
					(char)magic_shift_read_netcnf_2(heapmem[netcnf_size - 1], (u8)g_id_xorbuf[xorind1 + 2]);
			}
		}
		if ( readres >= 0 )
		{
			heapmem[netcnf_size] = 0;
			do_close_netcnf(fd);
			return netcnf_size;
		}
	}
	do_free_heapmem(heapmem);
	*netcnf_heap_ptr = 0;
	do_close_netcnf(fd);
	return (readres != -EIO) ? -4 : -18;
}

static int do_write_netcnf_encode(const char *netcnf_path, void *buf, int netcnf_len)
{
	int result;
	int fd;
	u16 *buf_1;
	int netcnf_len_1;
	int xorind1;
	int xoroffs;
	int writeres;
	u16 bufflipx1;
	char bufflipx2;

	result = do_read_ilink_id();
	// cppcheck-suppress knownConditionTrueFalse
	if ( result < 0 )
		return result;
	do_init_xor_magic(g_id_buffer);
	fd = do_open_netcnf(netcnf_path, 1538, 511);
	buf_1 = (u16 *)buf;
	if ( fd < 0 )
	{
		return (fd == -EIO) ? -18 : -3;
	}
	netcnf_len_1 = netcnf_len;
	xorind1 = 0;
	xoroffs = 0;
	writeres = 0;
	while ( writeres >= 0 )
	{
		while ( netcnf_len_1 >= 2 )
		{
			bufflipx1 = (u16)magic_shift_write_netcnf_1(*buf_1, (u8)g_id_xorbuf[xorind1 + 2]);
			xorind1 += 1;
			xorind1 = (xorind1 != sizeof(g_id_xorbuf)) ? xorind1 : 0;
			bufflipx1 ^= 0xFFFF;
			writeres = do_write_netcnf_no_encode(fd, &bufflipx1, sizeof(bufflipx1));
			buf_1 += 1;
			if ( writeres < 0 )
				break;
			netcnf_len_1 -= 2;
			xoroffs += 2;
		}
		if ( writeres >= 0 && !netcnf_len_1 )
		{
			do_close_netcnf(fd);
			return xoroffs;
		}
		if ( writeres >= 0 )
		{
			bufflipx2 = (char)magic_shift_write_netcnf_2(*(u8 *)buf_1, (u8)g_id_xorbuf[xorind1 + 2]);
			xorind1 += 1;
			xorind1 = (xorind1 != sizeof(g_id_xorbuf)) ? xorind1 : 0;
			bufflipx2 ^= 0xFF;
			writeres = do_write_netcnf_no_encode(fd, &bufflipx2, sizeof(bufflipx2));
			netcnf_len_1 -= 1;
			xoroffs += 1;
		}
	}
	do_close_netcnf(fd);
	return (writeres != -EIO) ? -5 : -18;
}

static int do_read_netcnf_no_decode(const char *netcnf_path, char **netcnf_heap_ptr)
{
	int fd;
	int netcnf_size;
	char *netcnf_data;

	*netcnf_heap_ptr = 0;
	fd = do_open_netcnf(netcnf_path, 1, 0);
	if ( fd < 0 )
	{
		return (fd != -EIO) ? -3 : -18;
	}
	netcnf_size = do_filesize_netcnf(fd);
	if ( netcnf_size < 0 )
	{
		do_close_netcnf(fd);
		return netcnf_size;
	}
	netcnf_data = (char *)do_alloc_heapmem(netcnf_size + 1);
	*netcnf_heap_ptr = netcnf_data;
	if ( !netcnf_data )
	{
		do_close_netcnf(fd);
		return -2;
	}
	netcnf_size = do_readfile_netcnf(fd, netcnf_data, netcnf_size);
	if ( netcnf_size < 0 )
	{
		do_free_heapmem(*netcnf_heap_ptr);
		*netcnf_heap_ptr = 0;
		do_close_netcnf(fd);
		return (netcnf_size != -EIO) ? -4 : -18;
	}
	netcnf_data[netcnf_size] = 0;
	do_close_netcnf(fd);
	return netcnf_size;
}

static void do_init_xor_magic(const char *in_id_buf)
{
	int i;

	for ( i = 0; (i + 1) < 8; i += 1 )
	{
		g_id_xorbuf[(i * 3) + 2] = ((u8)in_id_buf[i] >> 5) + 1;
		g_id_xorbuf[(i * 3) + 3] = (((u8)in_id_buf[i] >> 2) & 7) + 1;
		g_id_xorbuf[(i * 3) + 4] = (in_id_buf[i] & 3) + 1;
	}
}

static int magic_shift_write_netcnf_2(int inshft, int buflen)
{
	for ( ; buflen; buflen -= 1 )
		inshft = ((u8)inshft >> 7) | (inshft << 1);
	return (u8)inshft;
}

static int magic_shift_read_netcnf_2(int inshft, int buflen)
{
	for ( ; buflen; buflen -= 1 )
		inshft = ((u8)inshft >> 1) | (inshft << 7);
	return (u8)inshft;
}

static int magic_shift_write_netcnf_1(int inshft, int buflen)
{
	for ( ; buflen; buflen -= 1 )
		inshft = ((u16)inshft >> 15) | (inshft << 1);
	return (u16)inshft;
}

static int magic_shift_read_netcnf_1(int inshft, int buflen)
{
	for ( ; buflen; buflen -= 1 )
		inshft = ((u16)inshft >> 1) | (inshft << 15);
	return (u16)inshft;
}

static void do_safe_strcpy(char *dst, size_t maxlen, const char *src, int linenum)
{
	if ( strlen(src) >= maxlen )
	{
		printf("[netcnf] strcpy failed(%d)\n", linenum);
		return;
	}
	strcpy(dst, src);
}

static void do_safe_strcat(char *dst, size_t maxlen, const char *src, int linenum)
{
	if ( strlen(dst) + strlen(src) >= maxlen )
	{
		printf("[netcnf] strcat failed(%d)\n", linenum);
		return;
	}
	strcat(dst, src);
}

static void do_safe_make_pathname(char *dst, size_t maxlen, const char *srcdir, const char *srcbase)
{
	if ( strlen(srcdir) + strlen(srcbase) + 1 >= maxlen )
	{
		printf("[netcnf] make_pathname failed\n");
		return;
	}
	strcpy(dst, srcdir);
	strcat(dst, "/");
	strcat(dst, srcbase);
}

static void do_safe_make_name(char *dst, size_t maxlen, const char *src1, const char *src2)
{
	if ( strlen(src1) + strlen(src2) >= maxlen )
	{
		printf("[netcnf] make_name failed\n");
		return;
	}
	strcpy(dst, src1);
	strcat(dst, src2);
}

static int do_check_capacity_inner2(const char *fpath, int minsize)
{
	int i;
	int zonefree;
	char devname[8];

	for ( i = 0; i < 5; i += 1 )
	{
		devname[i] = fpath[i];
		if ( fpath[i] == ':' )
		{
			devname[i + 1] = 0;
#ifdef _IOP
			zonefree = iomanX_devctl(devname, 0x5002, 0, 0, 0, 0) * ((int)iomanX_devctl(devname, 0x5001, 0, 0, 0, 0) / 1024);
#else
			{
				struct statvfs st;

				statvfs(devname, &st);
				zonefree = (int)(u32)(st.f_bfree * st.f_frsize);
			}
#endif
			return (zonefree < minsize) ? -16 : 0;
		}
	}
	return -9;
}

static int do_check_capacity_inner(const char *fpath)
{
	int minsize;

	if ( !strncmp(fpath, "mc", 2) )
		minsize = 0x5E;
	else if ( !strncmp(fpath, "pfs", 3) )
		minsize = 0xF4;
	else
		return -9;
	return do_check_capacity_inner2(fpath, minsize);
}

static int do_handle_combination_path(int type, const char *fpath, char *dst, size_t maxlen, const char *usr_name)
{
	char *i;
	int j;
	int devnr;
	char devnum[8];

	if ( !usr_name )
		return -11;
	do_safe_strcpy(dst, maxlen, usr_name, 139);
	if ( type )
		return 0;
	for ( i = dst; !isdigit(*i); i += 1 )
		;
	for ( j = 0; j < 4 && isdigit(i[j]); j += 1 )
	{
		devnum[j] = i[j];
	}
	if ( j >= 4 )
		return -11;
	devnum[j] = 0;
	devnr = (int)strtol(devnum, 0, 10);
	if (
		!strncmp(fpath, "mc", 2) ? ((unsigned int)(devnr - 1) >= 6) :
															 (!strncmp(fpath, "pfs", 3) ? (unsigned int)(devnr - 1) >= 0xA :
																														(unsigned int)(devnr - 1) >= sizeof(g_ifc_buffer)) )
		return -11;
	do_safe_make_name(dst, maxlen, "Combination", devnum);
	return 0;
}

static int do_copy_netcnf_path(const char *netcnf_path_1, const char *netcnf_path_2)
{
	int fd2;
	int fd1;
	int readres;
	char tmpbuf[512];

	fd2 = do_open_netcnf(netcnf_path_2, 1538, 511);
	if ( fd2 < 0 )
		return -3;
	fd1 = do_open_netcnf(netcnf_path_1, 1, 0);
	if ( fd1 < 0 )
	{
		do_close_netcnf(fd2);
		return -3;
	}
	while ( 1 )
	{
		int writeres;

		readres = do_readfile_netcnf(fd1, tmpbuf, sizeof(tmpbuf));
		if ( readres <= 0 )
			break;
		writeres = do_write_netcnf_no_encode(fd2, tmpbuf, readres);
		if ( readres != writeres )
		{
			do_close_netcnf(fd2);
			do_close_netcnf(fd1);
			return -5;
		}
	}
	do_close_netcnf(fd2);
	do_close_netcnf(fd1);
	return (readres < 0) ? -4 : 0;
}

static char *do_write_memcard_pathcopy(char *dst, size_t maxlen, const char *src)
{
	char *dst_end_slash;

	do_safe_strcpy(dst, maxlen, src, 218);
	for ( dst_end_slash = &dst[strlen(dst)]; *dst_end_slash != '/'; dst_end_slash -= 1 )
		;
	for ( ; *dst_end_slash == '/'; dst_end_slash -= 1 )
		;
	if ( *dst_end_slash == ':' )
		return 0;
	dst_end_slash[1] = 0;
	return dst;
}

static int do_write_memcard_files(const char *fpath, const char *icon_value, const char *iconsys_value)
{
	int result;
	char cur_basepath[256];
	char cur_combpath[256];

	if ( !do_write_memcard_pathcopy(cur_basepath, sizeof(cur_basepath), fpath) )
		return 0;
	do_safe_make_pathname(cur_combpath, sizeof(cur_combpath), cur_basepath, "SYS_NET.ICO");
	result = do_copy_netcnf_path(icon_value, cur_combpath);
	if ( result < 0 )
		return result;
	do_safe_make_pathname(cur_combpath, sizeof(cur_combpath), cur_basepath, "icon.sys");
	result = do_copy_netcnf_path(iconsys_value, cur_combpath);
	if ( result < 0 )
		return result;
	return 0;
}

static int do_handle_fname(char *fpath, size_t maxlen, const char *fname)
{
	char *index_res;
	const char *pathname;
	int maxbuf;

	do_safe_strcpy(fpath, maxlen, fname, 266);
	index_res = index(fpath, ':');
	if ( !index_res )
		return -9;
	if ( !strncmp(fpath, "mc", 2) )
	{
		index_res[1] = 0;
		pathname = "/BWNETCNF/BWNETCNF";
		maxbuf = 275;
	}
	else if ( !strncmp(fpath, "pfs", 3) )
	{
		index_res[1] = 0;
		pathname = "/etc/network/net.db";
		maxbuf = 279;
	}
	else
		return 0;
	do_safe_strcat(fpath, maxlen, pathname, maxbuf);
	return 0;
}

static const char *do_get_str_line(const char *buf)
{
	for ( ; *buf && *buf != '\n'; buf += 1 )
		;
	return &buf[*buf == '\n'];
}

static int do_split_str_comma_index(char *dst, const char *src, int commaind)
{
	int i;

	for ( i = 0; i < commaind; i += 1 )
	{
		for ( ; *src && *src != '\n' && *src != ','; src += 1 )
			;
		if ( *src != ',' )
			return -1;
		src += 1;
	}
	for ( ; *src && *src != ',' && *src != '\n' && *src != '\r'; src += 1 )
	{
		*dst = *src;
		dst += 1;
	}
	*dst = 0;
	return 0;
}

static int
do_remove_old_config(const char *fpath, const char *netcnf_heap_buf, const char *icon_value, const char *iconsys_value)
{
	int sysneticoflag;
	int dfd;
	int fileop_res;
	const char *curheapbuf1;
	char cur_basepath[256];
	char cur_combpath[256];
	iox_dirent_t statname;
	iox_stat_t statsize;
	int iconsysflag;

	sysneticoflag = 1;
	iconsysflag = 1;
	if ( !do_write_memcard_pathcopy(cur_basepath, sizeof(cur_basepath), fpath) )
		return 0;
	dfd = do_dopen_wrap(cur_basepath);
	// cppcheck-suppress knownConditionTrueFalse
	if ( dfd < 0 )
	{
		return (dfd == -EIO) ? -18 : 0;
	}
	while ( 1 )
	{
		int sizeflag;

		fileop_res = do_dread_wrap(dfd, &statname);
		// cppcheck-suppress knownConditionTrueFalse
		if ( fileop_res <= 0 )
			break;
		sizeflag = 1;
		if ( strlen(statname.name) == 10 )
		{
			if ( !strncmp(&statname.name[6], ".cnf", 4) || !strncmp(&statname.name[6], ".dat", 4) )
			{
				for ( curheapbuf1 = netcnf_heap_buf; curheapbuf1 && *curheapbuf1; curheapbuf1 = do_get_str_line(curheapbuf1) )
				{
					do_split_str_comma_index(cur_combpath, curheapbuf1, 2);
					if ( !strcmp(statname.name, cur_combpath) )
					{
						do_safe_make_pathname(cur_combpath, sizeof(cur_combpath), cur_basepath, statname.name);
						sizeflag = 0;
						break;
					}
				}
			}
		}
		else if ( !strncmp(fpath, "mc", 2) )
		{
			if ( !strcmp(statname.name, "SYS_NET.ICO") )
			{
				do_safe_make_pathname(cur_combpath, sizeof(cur_combpath), cur_basepath, statname.name);
				do_getstat_wrap(cur_combpath, &statsize);
				if ( statsize.size != 0x8398 )
				{
					sizeflag = 0;
				}
				else
				{
					sysneticoflag = 0;
				}
			}
			else if ( !strcmp(statname.name, "icon.sys") )
			{
				do_safe_make_pathname(cur_combpath, sizeof(cur_combpath), cur_basepath, statname.name);
				do_getstat_wrap(cur_combpath, &statsize);
				if ( statsize.size != 0x3C4 )
				{
					sizeflag = 0;
				}
				else
				{
					iconsysflag = 0;
				}
			}
			else if ( strcmp(statname.name, "BWNETCNF") )
			{
				do_safe_make_pathname(cur_combpath, sizeof(cur_combpath), cur_basepath, statname.name);
				sizeflag = 0;
			}
		}
		else if ( !strncmp(fpath, "pfs", 3) && strcmp(statname.name, "net.db") )
		{
			do_safe_make_pathname(cur_combpath, sizeof(cur_combpath), cur_basepath, statname.name);
			sizeflag = 0;
		}
		if ( !sizeflag )
		{
			fileop_res = do_remove_wrap(cur_combpath);
			if ( fileop_res == -EIO )
				break;
		}
	}
	if ( fileop_res == -EIO )
	{
		do_dclose_wrap(dfd);
		return -18;
	}
	do_dclose_wrap(dfd);
	if ( !strncmp(fpath, "mc", 2) )
	{
		if ( sysneticoflag || iconsysflag )
			fileop_res = do_write_memcard_files(fpath, icon_value, iconsys_value);
		do_chstat_mode_copyprotect_wrap(cur_basepath);
	}
	return fileop_res;
}

static int do_type_check(int type, const char *buf)
{
	return (do_split_str_comma_index(g_arg_fname, buf, 0)) ? -1 : (strtol(g_arg_fname, 0, 10) == type);
}

static int do_read_current_netcnf_nodecode(const char *fpath, char **netcnf_heap_ptr)
{
	int retres;

	if ( !fpath )
		return -9;
	retres = do_read_netcnf_no_decode(fpath, netcnf_heap_ptr);
	return (retres < 0) ? ((retres != -3) ? retres : 0) : retres;
}

static int do_write_noencode_netcnf_atomic(const char *fpath, void *ptr, int size)
{
	int fd;
	int writeres;
	char fpath_comb[256];

	if ( !fpath )
		return -9;
	do_safe_make_name(fpath_comb, sizeof(fpath_comb), fpath, ".tmp");
	fd = do_open_netcnf(fpath_comb, 1538, 511);
	if ( fd < 0 )
	{
		return (fd == -EIO) ? -18 : -3;
	}
	writeres = do_write_netcnf_no_encode(fd, ptr, size);
	do_close_netcnf(fd);
	// Unofficial: dead code removed
	return (size != writeres) ? ((writeres != -EIO) ? -5 : -18) : ((iomanX_rename(fpath_comb, fpath) == -EIO) ? -18 : 0);
}

static int do_remove_netcnf_dirname(const char *dirpath, const char *entry_buffer)
{
	const char *p_dirname;
	int remove_res_1;

	p_dirname = do_handle_netcnf_dirname(dirpath, entry_buffer, g_netcnf_file_path);
	if ( !p_dirname )
		return -7;
	remove_res_1 = do_remove_wrap(p_dirname);
	return (remove_res_1 < 0) ? ((remove_res_1 == -EIO) ? -18 : -7) : 0;
}

static int do_get_count_list_inner(const char *fname, int type, sceNetCnfList_t *p)
{
	int result;
	const char *curheapbuf1;
	int curind1;

	result = do_handle_fname(g_dir_name, sizeof(g_dir_name), fname);
	if ( result < 0 )
		return result;
	result = do_read_current_netcnf_nodecode(g_dir_name, &g_count_list_heapptr);
	if ( result <= 0 )
		return result;
	curind1 = 0;
	for ( curheapbuf1 = g_count_list_heapptr; *curheapbuf1; curheapbuf1 = do_get_str_line(curheapbuf1) )
	{
		if ( do_type_check(type, curheapbuf1) <= 0 )
			continue;
		curind1 += 1;
		if ( !p )
			continue;
		p->type = type;
		if ( do_split_str_comma_index(g_arg_fname, curheapbuf1, 1) )
			continue;
		p->stat = (int)strtol(g_arg_fname, 0, 10);
		if (
			do_split_str_comma_index(p->sys_name, curheapbuf1, 2) || do_split_str_comma_index(p->usr_name, curheapbuf1, 3) )
			continue;
		p += 1;
	}
	do_free_heapmem(g_count_list_heapptr);
	return curind1;
}

static int do_load_entry_inner(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e)
{
	int result;
	const char *curheapbuf1;

	result = do_handle_fname(g_dir_name, sizeof(g_dir_name), fname);
	if ( result < 0 )
		return result;
	result = do_handle_combination_path(type, g_dir_name, g_combination_buf1, sizeof(g_combination_buf1), usr_name);
	if ( result < 0 )
		return result;
	result = do_read_current_netcnf_nodecode(g_dir_name, &g_load_entry_heapptr);
	if ( result <= 0 )
		return !result ? -8 : result;
	for ( curheapbuf1 = g_load_entry_heapptr; *curheapbuf1; curheapbuf1 = do_get_str_line(curheapbuf1) )
	{
		if (
			do_type_check(type, curheapbuf1) >= 0 && !do_split_str_comma_index(g_arg_fname, curheapbuf1, 3)
			&& !strcmp(g_arg_fname, g_combination_buf1) && !do_split_str_comma_index(g_arg_fname, curheapbuf1, 2) )
		{
			do_free_heapmem(g_load_entry_heapptr);
			e->dir_name = g_dir_name;
			e->arg_fname = g_arg_fname;
			e->req = type ? 2 : 1;
			return do_load_conf_inner(e);
		}
	}
	do_free_heapmem(g_load_entry_heapptr);
	return -8;
}

static void do_extra_ifc_handling(const char *arg_fname)
{
	const char *i;
	const char *curptr1;
	unsigned int curbufsz1;
	unsigned int curindx;

	if ( !arg_fname || !*arg_fname )
		return;
	for ( i = &arg_fname[strlen(arg_fname) - 1]; i >= arg_fname && *i != '.'; i -= 1 )
		;
	curptr1 = i - 1;
	if ( curptr1 < arg_fname || *i != '.' || !isdigit(*curptr1) )
		return;
	curbufsz1 = 0;
	curindx = 1;
	for ( ; curptr1 >= arg_fname && isdigit(*curptr1); curptr1 -= 1 )
	{
		curbufsz1 += curindx * (u8)(*curptr1 - '0');
		curindx *= 10;
	}
	if ( curbufsz1 < sizeof(g_ifc_buffer) )
		g_ifc_buffer[curbufsz1] = 1;
}

static void do_extra_pair_handling(char *fpath, int type, const char *src, const sceNetCnfEnv_t *e)
{
	sceNetCnfEnv_t *heapmem;
	int conf_inner;
	struct sceNetCnfPair *i;

	if ( do_split_str_comma_index(g_arg_fname, src, 2) )
		return;
	heapmem = (sceNetCnfEnv_t *)do_alloc_heapmem(sizeof(sceNetCnfEnv_t) + 4096);
	if ( !heapmem )
		return;
	heapmem->req = 1;
	heapmem->mem_ptr = &heapmem[1];
	heapmem->mem_base = &heapmem[1];
	heapmem->dir_name = fpath;
	heapmem->arg_fname = g_arg_fname;
	heapmem->mem_last = ((char *)&heapmem[1]) + 4096;
	heapmem->f_no_check_magic = e->f_no_check_magic;
	heapmem->f_no_decode = e->f_no_decode;
	heapmem->f_verbose = e->f_verbose;
	conf_inner = do_load_conf_inner(heapmem);
	if ( (!conf_inner || conf_inner == -21) && heapmem->root )
	{
		for ( i = heapmem->root->pair_head; i; i = i->forw )
		{
			switch ( type )
			{
				case 1:
					do_extra_ifc_handling((char *)i->attach_ifc);
					break;
				case 2:
					do_extra_ifc_handling((char *)i->attach_dev);
					break;
				default:
					continue;
			}
		}
	}
	do_free_heapmem(heapmem);
}

static int do_add_entry_inner(
	const char *fname,
	int type,
	const char *usr_name,
	sceNetCnfEnv_t *e,
	const char *icon_value,
	const char *iconsys_value,
	int no_check_capacity)
{
	int result;
	int retres2;
	const char *curentry1;
	int i;
	const char *curentry2;
	const char *k;
	int fd;
	char atomicrenamepath[256];
	int retres1;
	int maxflag;

	maxflag = 1;
	fd = -EPERM;
	if ( get_check_provider_eq_zero() )
	{
		result = do_check_provider_inner(e, type);
		if ( result < 0 )
			return result;
	}
	result = do_handle_fname(g_dir_name, sizeof(g_dir_name), fname);
	if ( result < 0 )
		return result;
	if ( !no_check_capacity )
	{
		result = do_check_capacity_inner(g_dir_name);
		if ( result < 0 )
			return result;
	}
	atomicrenamepath[0] = 0;
	result = do_handle_combination_path(type, g_dir_name, g_combination_buf1, sizeof(g_combination_buf1), usr_name);
	if ( result < 0 )
		return result;
	retres1 = do_read_current_netcnf_nodecode(g_dir_name, &g_add_entry_heapptr);
	if ( retres1 < 0 )
		return result;
	retres2 = do_remove_old_config(g_dir_name, g_add_entry_heapptr, icon_value, iconsys_value);
	if ( retres2 < 0 )
		maxflag = 0;
	if ( maxflag )
	{
		memset(g_ifc_buffer, 0, sizeof(g_ifc_buffer));
		if ( retres1 )
		{
			if ( !strncmp(g_dir_name, "mc", 2) )
			{
				i = 0;
				for ( curentry1 = g_add_entry_heapptr; *curentry1; curentry1 = do_get_str_line(curentry1) )
				{
					if ( do_type_check(type, curentry1) == 1 )
						i += 1;
				}
				switch ( type )
				{
					case 0:
						retres2 = -12;
						if ( i >= 6 )
							maxflag = 0;
						break;
					case 1:
					case 2:
						retres2 = -12;
						if ( i >= 4 )
							maxflag = 0;
						break;
					default:
						break;
				}
			}
			else if ( !strncmp(g_dir_name, "pfs", 3) )
			{
				i = 0;
				for ( curentry2 = g_add_entry_heapptr; *curentry2; curentry2 = do_get_str_line(curentry2) )
				{
					if ( do_type_check(type, curentry2) == 1 )
						i += 1;
				}
				switch ( type )
				{
					case 0:
						retres2 = -12;
						if ( i >= 10 )
							maxflag = 0;
						break;
					case 1:
					case 2:
						retres2 = -12;
						if ( i >= 30 )
							maxflag = 0;
						break;
					default:
						break;
				}
			}
			if ( maxflag )
			{
				for ( k = g_add_entry_heapptr; *k; k = do_get_str_line(k) )
				{
					if ( (unsigned int)(type - 1) < 2 && do_type_check(0, k) > 0 )
						do_extra_pair_handling(g_dir_name, type, k, e);
					if (
						do_type_check(type, k) > 0 && !do_split_str_comma_index(g_arg_fname, k, 1)
						&& strtol(g_arg_fname, 0, 10) == 1 && !do_split_str_comma_index(g_arg_fname, k, 2) )
					{
						do_extra_ifc_handling(g_arg_fname);
						if ( !do_split_str_comma_index(g_arg_fname, k, 3) && !strcmp(g_combination_buf1, g_arg_fname) )
						{
							do_free_heapmem(g_add_entry_heapptr);
							return -11;
						}
					}
				}
			}
		}
	}
	if ( maxflag )
	{
		char *endbuf;
		const char *fileext;

		do_safe_strcpy(g_arg_fname, sizeof(g_arg_fname), g_dir_name, 740);
		for ( endbuf = &g_arg_fname[strlen(g_arg_fname) - 1];
					endbuf >= g_arg_fname && *endbuf != ':' && *endbuf != '/' && *endbuf != '\\';
					endbuf -= 1 )
			;
		if ( endbuf >= g_arg_fname && *endbuf != ':' && *endbuf != '/' && *endbuf != '\\' )
		{
			*endbuf = 0;
		}
		else
		{
			endbuf[1] = 0;
		}
		fileext = (type && !e->f_no_decode) ? ".dat" : ".cnf";
		for ( i = 0; i < (int)(sizeof(g_ifc_buffer)); i += 1 )
		{
			if ( !g_ifc_buffer[i] )
			{
				switch ( type )
				{
					case 1:
						// Unofficial: specify max length for g_arg_fname to avoid overflow
						sprintf(g_netcnf_file_path, "%.245sifc%03d%s", g_arg_fname, i, fileext);
						break;
					case 2:
						// Unofficial: specify max length for g_arg_fname to avoid overflow
						sprintf(g_netcnf_file_path, "%.245sdev%03d%s", g_arg_fname, i, fileext);
						break;
					case 3:
						// Unofficial: specify max length for g_arg_fname to avoid overflow
						sprintf(g_netcnf_file_path, "%.245snet%03d%s", g_arg_fname, i, fileext);
						break;
					default:
						do_free_heapmem(g_add_entry_heapptr);
						return -10;
				}
				fd = do_open_netcnf(g_netcnf_file_path, 1, 0);
				if ( fd < 0 )
				{
					if ( fd == -EIO )
						return -18;
					break;
				}
				do_close_netcnf(fd);
			}
		}
		retres2 = -12;
		if ( i < (int)(sizeof(g_ifc_buffer)) )
		{
			char *dirname_buf1;
			char *cur_entry_buffer;

			cur_entry_buffer = g_entry_buffer;
			for ( dirname_buf1 = g_dir_name; *dirname_buf1; dirname_buf1 += 1 )
			{
				if ( (*dirname_buf1 == '/' || *dirname_buf1 == '\\') && dirname_buf1[1] )
				{
					*cur_entry_buffer = 0;
					retres2 = mkdir(g_entry_buffer, 511);
					if ( !retres2 && !strncmp(g_dir_name, "mc", 2) )
					{
						do_chstat_mode_copyprotect_wrap(g_entry_buffer);
						retres2 = do_write_memcard_files(g_dir_name, icon_value, iconsys_value);
						if ( retres2 < 0 )
							break;
					}
					if ( retres2 == -5 )
						break;
					retres2 = -18;
				}
				*cur_entry_buffer = *dirname_buf1;
				cur_entry_buffer += 1;
			}
			if ( *dirname_buf1 )
			{
				e->dir_name = g_dir_name;
				e->arg_fname = &g_netcnf_file_path[strlen(g_arg_fname)];
				e->req = type ? 2 : 1;
				retres2 = -1;
				if ( !do_export_netcnf(e) )
				{
					do_safe_make_name(atomicrenamepath, sizeof(atomicrenamepath), g_dir_name, ".tmp");
					fd = do_open_netcnf(atomicrenamepath, 1538, 511);
					if ( fd < 0 )
					{
						retres2 = (fd == -EIO) ? -18 : -3;
					}
					else
					{
						int strlenx;
						int writeres;

						strlenx = sprintf(
							g_entry_buffer, "%d,%d,%s,%s\n", type, 1, &g_netcnf_file_path[strlen(g_arg_fname)], g_combination_buf1);
						writeres = do_write_netcnf_no_encode(fd, g_entry_buffer, strlenx);
						retres2 = 0;
						if ( strlenx != writeres )
						{
							retres2 = (writeres == -EIO) ? -18 : -5;
						}
						else
						{
							writeres = do_write_netcnf_no_encode(fd, g_add_entry_heapptr, retres1);
							if ( retres1 != writeres )
							{
								retres2 = (writeres == -EIO) ? -18 : -5;
							}
						}
					}
				}
			}
		}
	}
	do_free_heapmem(g_add_entry_heapptr);
	if ( fd >= 0 )
		do_close_netcnf(fd);
	// Unofficial: dead code removed
	return (atomicrenamepath[0] && iomanX_rename(atomicrenamepath, g_dir_name) == -EIO) ?
				 -18 :
				 ((strncmp(g_dir_name, "pfs", 3) || iomanX_sync(g_dir_name, 0) != -EIO) ? retres2 : -18);
}

static int do_handle_set_usrname(const char *fpath, int type, const char *usrname_buf2, const char *usrname_bufnew)
{
	int result;
	int retres1;
	char *heapmem;
	const char *ptr_1;
	char *heapmem_1;
	int writeres1;
	char *ptr;

	ptr = 0;
	if ( !usrname_buf2 )
		return -11;
	result = do_handle_combination_path(type, g_dir_name, g_combination_buf1, sizeof(g_combination_buf1), usrname_bufnew);
	if ( result < 0 )
		return result;
	retres1 = do_read_current_netcnf_nodecode(fpath, &ptr);
	if ( retres1 <= 0 )
		return (!retres1) ? -3 : retres1;
	heapmem = (char *)do_alloc_heapmem((int)((unsigned int)retres1 + strlen(usrname_bufnew) + 1));
	if ( !heapmem )
	{
		do_free_heapmem(ptr);
		return -2;
	}
	ptr_1 = ptr;
	heapmem_1 = heapmem;
	while ( *ptr_1 )
	{
		if ( do_type_check(type, ptr_1) > 0 && !do_split_str_comma_index(g_arg_fname, ptr_1, 3) )
		{
			if ( !strcmp(g_arg_fname, usrname_buf2) )
			{
				if ( !do_split_str_comma_index(g_arg_fname, ptr_1, 2) )
				{
					heapmem_1 += sprintf(heapmem_1, "%d,%d,%s,%s\n", type, 1, g_arg_fname, g_combination_buf1);
					ptr_1 = do_get_str_line(ptr_1);
					continue;
				}
			}
			else if ( !strcmp(g_arg_fname, g_combination_buf1) )
			{
				do_free_heapmem(ptr);
				do_free_heapmem(heapmem);
				return -11;
			}
		}
		for ( ; *ptr_1 && *ptr_1 != '\n'; ptr_1 += 1 )
		{
			*heapmem_1 = *ptr_1;
			heapmem_1 += 1;
		}
		if ( *ptr_1 == '\n' )
		{
			*heapmem_1 = *ptr_1;
			heapmem_1 += 1;
			ptr_1 += 1;
		}
	}
	do_free_heapmem(ptr);
	writeres1 = do_write_noencode_netcnf_atomic(fpath, heapmem, (int)(heapmem_1 - heapmem));
	do_free_heapmem(heapmem);
	return writeres1;
}

static int do_edit_entry_inner(
	const char *fname,
	int type,
	const char *usr_name,
	const char *new_usr_name,
	sceNetCnfEnv_t *e,
	const char *icon_value,
	const char *iconsys_value,
	int no_check_capacity)
{
	int result;
	int rmoldcfgres;
	const char *curentry1;
	char curentrybuf1[256];
	char curfilepath1[256];

	if ( get_check_provider_eq_zero() )
	{
		result = do_check_provider_inner(e, type);
		if ( result < 0 )
			return result;
	}
	result = do_handle_fname(g_dir_name, sizeof(g_dir_name), fname);
	if ( result < 0 )
		return result;
	if ( !no_check_capacity )
	{
		result = do_check_capacity_inner(g_dir_name);
		if ( result < 0 )
			return result;
	}
	result = do_handle_combination_path(type, g_dir_name, g_combination_buf2, sizeof(g_combination_buf2), usr_name);
	if ( result < 0 )
		return result;
	result = do_read_current_netcnf_nodecode(g_dir_name, &g_edit_entry_heapptr);
	if ( result <= 0 )
	{
		return !result ? -3 : result;
	}
	rmoldcfgres = do_remove_old_config(g_dir_name, g_edit_entry_heapptr, icon_value, iconsys_value);
	if ( rmoldcfgres >= 0 )
	{
		int flg;

		rmoldcfgres = 0;
		for ( curentry1 = g_edit_entry_heapptr; *curentry1; curentry1 = do_get_str_line(curentry1) )
		{
			if (
				do_type_check(type, curentry1) > 0 && !do_split_str_comma_index(g_arg_fname, curentry1, 3)
				&& !strcmp(g_arg_fname, g_combination_buf2) && !do_split_str_comma_index(curentrybuf1, curentry1, 2) )
			{
				rmoldcfgres += 1;
			}
		}
		flg = 0;
		if ( !rmoldcfgres )
		{
			rmoldcfgres = -8;
		}
		else
		{
			if ( !get_check_provider_eq_zero() )
			{
				flg = 1;
			}
			if ( !flg )
			{
				rmoldcfgres = !do_handle_netcnf_dirname(g_dir_name, curentrybuf1, curfilepath1) ?
											-11 :
											do_read_check_netcnf(curfilepath1, type, e->f_no_check_magic, e->f_no_decode);
			}
		}
		if ( flg || rmoldcfgres >= 0 )
		{
			do_safe_make_name(curfilepath1, sizeof(curfilepath1), curentrybuf1, ".tmp");
			e->dir_name = g_dir_name;
			// cppcheck-suppress autoVariables
			e->arg_fname = curfilepath1;
			e->req = type ? 2 : 1;
			if ( do_export_netcnf(e) )
			{
				rmoldcfgres = -1;
			}
			else
			{
				char *curfilepath1end;

				do_safe_strcpy(curfilepath1, sizeof(curfilepath1), g_dir_name, 1010);
				for ( curfilepath1end = &curfilepath1[strlen(curfilepath1)]; curfilepath1end != curfilepath1;
							curfilepath1end -= 1 )
				{
					if ( *curfilepath1end == '/' || *curfilepath1end == '\\' )
					{
						curfilepath1end[1] = 0;
						break;
					}
				}
				do_safe_strcat(curfilepath1, sizeof(curfilepath1), curentrybuf1, 1017);
				do_safe_strcpy(curentrybuf1, sizeof(curentrybuf1), curfilepath1, 1018);
				do_safe_strcat(curfilepath1, sizeof(curfilepath1), ".tmp", 1019);
				if ( iomanX_rename(curfilepath1, curentrybuf1) == -EIO )
					rmoldcfgres = -18;
			}
		}
	}
	do_free_heapmem(g_edit_entry_heapptr);
	if ( rmoldcfgres >= 0 )
	{
		do_set_latest_entry_inner(g_dir_name, type, g_combination_buf2);
		if ( new_usr_name )
			rmoldcfgres = do_handle_set_usrname(g_dir_name, type, g_combination_buf2, new_usr_name);
	}
	return (!strncmp(g_dir_name, "pfs", 3) && iomanX_sync(g_dir_name, 0) == -EIO) ? -18 : rmoldcfgres;
}

static int do_delete_entry_inner(
	const char *fname,
	int type,
	const char *usr_name,
	const char *icon_value,
	const char *iconsys_value,
	int no_check_capacity)
{
	int has_comma;
	int result;
	char *heapmem;
	const char *curentry1;

	has_comma = 0;
	g_delete_entry_heapptr = 0;
	result = do_handle_fname(g_dir_name, sizeof(g_dir_name), fname);
	if ( result < 0 )
	{
		return result;
	}
	if ( !no_check_capacity )
	{
		result = do_check_capacity_inner(g_dir_name);
		if ( result < 0 )
			return result;
	}
	result = do_handle_combination_path(type, g_dir_name, g_combination_buf1, sizeof(g_combination_buf1), usr_name);
	if ( result < 0 )
		return result;
	result = do_read_current_netcnf_nodecode(g_dir_name, &g_delete_entry_heapptr);
	if ( result <= 0 )
		return !result ? -3 : result;
	heapmem = (char *)do_alloc_heapmem(result);
	if ( !heapmem )
	{
		do_free_heapmem(g_delete_entry_heapptr);
		return -2;
	}
	result = do_remove_old_config(g_dir_name, g_delete_entry_heapptr, icon_value, iconsys_value);
	if ( result >= 0 )
	{
		char *heapmem_1;

		curentry1 = g_delete_entry_heapptr;
		heapmem_1 = heapmem;
		while ( *curentry1 )
		{
			if (
				do_type_check(type, curentry1) <= 0 || do_split_str_comma_index(g_arg_fname, curentry1, 3)
				|| strcmp(g_arg_fname, g_combination_buf1) )
			{
				for ( ; *curentry1 && *curentry1 != '\n'; curentry1 += 1 )
				{
					*heapmem_1 = *curentry1;
					heapmem_1 += 1;
				}
				if ( *curentry1 == '\n' )
				{
					*heapmem_1 = *curentry1;
					heapmem_1 += 1;
					curentry1 += 1;
				}
			}
			else
			{
				if ( !do_split_str_comma_index(g_entry_buffer, curentry1, 2) )
					has_comma = 1;
				curentry1 = do_get_str_line(curentry1);
			}
		}
		result = do_write_noencode_netcnf_atomic(g_dir_name, heapmem, (int)(heapmem_1 - heapmem));
		if ( result >= 0 && has_comma )
			result = do_remove_netcnf_dirname(g_dir_name, g_entry_buffer);
	}
	do_free_heapmem(g_delete_entry_heapptr);
	do_free_heapmem(heapmem);
	return (!strncmp(g_dir_name, "pfs", 3) && iomanX_sync(g_dir_name, 0) == -EIO) ? -18 : result;
}

static int do_set_latest_entry_inner(const char *fname, int type, const char *usr_name)
{
	int isbeforeend1;
	int result;
	char *heapmem2;
	int readsz;
	char *heapmem1;
	char *heapmem1_1;
	char *curentry1;

	isbeforeend1 = 0;
	result = do_handle_fname(g_dir_name, sizeof(g_dir_name), fname);
	if ( result < 0 )
	{
		return result;
	}
	result = do_handle_combination_path(type, g_dir_name, g_combination_buf1, sizeof(g_combination_buf1), usr_name);
	heapmem2 = 0;
	if ( result < 0 )
	{
		return result;
	}
	g_set_latest_entry_heapptr = 0;
	result = do_read_current_netcnf_nodecode(g_dir_name, &g_set_latest_entry_heapptr);
	readsz = result;
	if ( result <= 0 )
	{
		return !result ? -3 : result;
	}
	result = -2;
	heapmem1 = (char *)do_alloc_heapmem(readsz);
	heapmem1_1 = heapmem1;
	if ( heapmem1 )
	{
		char *heapmem2_1;

		heapmem2 = (char *)do_alloc_heapmem(readsz);
		heapmem2_1 = heapmem2;
		if ( heapmem2 )
		{
			curentry1 = g_set_latest_entry_heapptr;
			result = 0;
			while ( *curentry1 )
			{
				if (
					do_type_check(type, curentry1) > 0 && !do_split_str_comma_index(g_arg_fname, curentry1, 3)
					&& !strcmp(g_arg_fname, g_combination_buf1) )
				{
					for ( ; *curentry1 && *curentry1 != '\n'; curentry1 += 1 )
					{
						*heapmem1_1 = *curentry1;
						heapmem1_1 += 1;
					}
					if ( *curentry1 == '\n' )
					{
						*heapmem1_1 = *curentry1;
						heapmem1_1 += 1;
						curentry1 += 1;
					}
					result += 1;
					if ( heapmem2 < heapmem2_1 )
						isbeforeend1 = 1;
				}
				else
				{
					for ( ; *curentry1 && *curentry1 != '\n'; curentry1 += 1 )
					{
						*heapmem2_1 = *curentry1;
						heapmem2_1 += 1;
					}
					if ( *curentry1 == '\n' )
					{
						*heapmem2_1 = *curentry1;
						heapmem2_1 += 1;
						curentry1 += 1;
					}
				}
			}
			if ( isbeforeend1 )
			{
				memcpy(heapmem1_1, heapmem2, (u32)(heapmem2_1 - heapmem2));
				result =
					do_write_noencode_netcnf_atomic(g_dir_name, heapmem1, (int)(heapmem1_1 - heapmem1 + heapmem2_1 - heapmem2));
			}
		}
	}
	do_free_heapmem(g_set_latest_entry_heapptr);
	do_free_heapmem(heapmem1);
	do_free_heapmem(heapmem2);
	return (!strncmp(g_dir_name, "pfs", 3) && iomanX_sync(g_dir_name, 0) == -EIO) ? -18 : result;
}

static int do_delete_all_inner(const char *dev)
{
	int i;
	int dread_res;
	iox_dirent_t statname;

	if ( !strncmp(dev, "mc", 2) )
	{
		int dfd1;

		for ( i = 0; dev[i] != ':'; i += 1 )
		{
			g_netcnf_file_path[i] = dev[i];
		}
		g_netcnf_file_path[i] = dev[i];
		g_netcnf_file_path[i + 1] = 0;
		do_safe_strcat(g_netcnf_file_path, sizeof(g_netcnf_file_path), "/BWNETCNF", 1199);
		dfd1 = do_dopen_wrap(g_netcnf_file_path);
		// cppcheck-suppress knownConditionTrueFalse
		if ( dfd1 < 0 )
			return 0;
		while ( 1 )
		{
			dread_res = do_dread_wrap(dfd1, &statname);
			// cppcheck-suppress knownConditionTrueFalse
			if ( dread_res <= 0 )
				break;
			if ( strcmp(statname.name, ".") && strcmp(statname.name, "..") )
			{
				do_safe_make_pathname(g_dir_name, sizeof(g_dir_name), g_netcnf_file_path, statname.name);
				if ( do_remove_wrap(g_dir_name) < 0 )
				{
					do_dclose_wrap(dfd1);
					return -7;
				}
			}
		}
		do_dclose_wrap(dfd1);
		return (rmdir(g_netcnf_file_path) < 0) ? -7 : 0;
	}
	else if ( !strncmp(dev, "pfs", 3) )
	{
		int dfd2;
		int remove_res2;
		int rmdir_res1;

		for ( i = 0; dev[i] != ':'; i += 1 )
		{
			g_netcnf_file_path[i] = dev[i];
		}
		g_netcnf_file_path[i] = dev[i];
		g_netcnf_file_path[i + 1] = 0;
		do_safe_strcat(g_netcnf_file_path, sizeof(g_netcnf_file_path), "/etc/network", 1229);
		dfd2 = do_dopen_wrap(g_netcnf_file_path);
		// cppcheck-suppress knownConditionTrueFalse
		if ( dfd2 < 0 )
		{
			return (dfd2 == -EIO) ? -18 : 0;
		}
		while ( 1 )
		{
			dread_res = do_dread_wrap(dfd2, &statname);
			// cppcheck-suppress knownConditionTrueFalse
			if ( dread_res <= 0 )
				break;
			if ( strcmp(statname.name, ".") && strcmp(statname.name, "..") )
			{
				do_safe_make_pathname(g_dir_name, sizeof(g_dir_name), g_netcnf_file_path, statname.name);
				remove_res2 = do_remove_wrap(g_dir_name);
				if ( remove_res2 < 0 )
				{
					do_dclose_wrap(dfd2);
					return (remove_res2 == -EIO) ? -18 : -7;
				}
			}
		}
		if ( dread_res == -EIO )
		{
			do_dclose_wrap(dfd2);
			return -18;
		}
		do_dclose_wrap(dfd2);
		rmdir_res1 = rmdir(g_netcnf_file_path);
		return (rmdir_res1 >= 0) ? ((iomanX_sync(g_netcnf_file_path, 0) != -EIO) ? 0 : -18) :
															 ((rmdir_res1 == -EIO) ? -18 : -7);
	}
	return -17;
}

static int do_check_special_provider_inner(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e)
{
	int result;
	const char *curentry1;
	int curentcount;
	int retres;

	result = do_handle_fname(g_dir_name, sizeof(g_dir_name), fname);
	if ( result < 0 )
		return result;
	result = do_handle_combination_path(type, g_dir_name, g_combination_buf2, sizeof(g_combination_buf2), usr_name);
	if ( result < 0 )
		return result;
	result = do_read_current_netcnf_nodecode(g_dir_name, &g_check_special_provider_heapptr);
	if ( result <= 0 )
	{
		return !result ? -3 : result;
	}
	curentcount = 0;
	for ( curentry1 = g_check_special_provider_heapptr; *curentry1; curentry1 = do_get_str_line(curentry1) )
	{
		if (
			do_type_check(type, curentry1) > 0 && !do_split_str_comma_index(g_arg_fname, curentry1, 3)
			&& !strcmp(g_arg_fname, g_combination_buf2) && !do_split_str_comma_index((char *)e->lbuf, curentry1, 2) )
		{
			curentcount += 1;
		}
	}
	retres = curentcount ? ((do_handle_netcnf_dirname(g_dir_name, (const char *)e->lbuf, (char *)e->dbuf)) ?
														do_read_check_netcnf((const char *)e->dbuf, type, e->f_no_check_magic, e->f_no_decode) :
														-11) :
												 -8;
	do_free_heapmem(g_check_special_provider_heapptr);
	return retres;
}

static char *do_alloc_mem_inner(sceNetCnfEnv_t *e, unsigned int size, int align)
{
	void *mem_ptr;
	char *retptrbegin;

	mem_ptr = e->mem_ptr;
	if ( !mem_ptr )
	{
		e->alloc_err += 1;
		return 0;
	}
	retptrbegin = (char *)(((uiptr)mem_ptr + (1 << align) - 1) & (uiptr) ~((1 << align) - 1));
	if ( &retptrbegin[size] >= (char *)e->mem_last )
	{
		e->alloc_err += 1;
		return 0;
	}
	e->mem_ptr = &retptrbegin[size];
	memset(retptrbegin, 0, size);
	return retptrbegin;
}

static const char *do_netcnf_parse_string(sceNetCnfEnv_t *e, const char *e_arg)
{
	u8 *dbuf;
	const char *argbegin;
	int i;
	int hexnum;
	int err;

	dbuf = e->dbuf;
	if ( *e_arg != '"' )
		return e_arg;
	err = 0;
	for ( argbegin = e_arg + 1; *argbegin && *argbegin != '"' && (char *)(dbuf + 1) < (char *)&(e->dbuf[1023]); )
	{
		char argchr_1;

		argchr_1 = *argbegin;
		argbegin += 1;
		if ( argchr_1 == '\\' )
		{
			if ( !*argbegin )
			{
				err = 1;
				break;
			}
			argchr_1 = 0;
			if ( (unsigned int)(((u8)*argbegin) - '0') >= 8 )
			{
				if ( ((u8)*argbegin) == 'x' || ((u8)*argbegin) == 'X' )
				{
					argbegin += 1;
					argchr_1 = 0;
					if ( !isxdigit(*argbegin) )
					{
						err = 2;
						break;
					}
					for ( i = 0; i < 2 && isxdigit(*argbegin); i += 1 )
					{
						hexnum = 16 * argchr_1;
						argchr_1 =
							(char)(hexnum
										 + (isdigit(*argbegin) ? (((u8)*argbegin) - '0') : ((!islower(*argbegin)) ? ((u8)*argbegin) - '7' : ((u8)*argbegin) - 'W')));
						argbegin += 1;
					}
				}
				else
				{
					argchr_1 = *argbegin;
					argbegin += 1;
					switch ( argchr_1 )
					{
						case 'a':
							argchr_1 = 7;
							break;
						case 'b':
							argchr_1 = 8;
							break;
						case 'f':
							argchr_1 = 12;
							break;
						case 'n':
							argchr_1 = 10;
							break;
						case 'r':
							argchr_1 = 13;
							break;
						case 't':
							argchr_1 = 9;
							break;
						case 'v':
							argchr_1 = 11;
							break;
						default:
							break;
					}
				}
			}
			else
			{
				for ( i = 0; i < 3 && (((u8)*argbegin) - (unsigned int)'0' < 8); i += 1 )
				{
					argchr_1 = 8 * argchr_1 + *argbegin - '0';
					argbegin += 1;
				}
			}
		}
		else if ( (unsigned int)(argchr_1 - 129) < 0x1F || (unsigned int)(argchr_1 - 224) < 0x1D )
		{
			if ( (u8)(((u8)*argbegin) - 64) < 0xBDu && ((u8)*argbegin) != 127 )
			{
				*dbuf = (u8)argchr_1;
				dbuf += 1;
				argchr_1 = *argbegin;
				argbegin += 1;
			}
		}
		*dbuf = (u8)argchr_1;
		dbuf += 1;
	}
	if ( !err )
	{
		if ( ((u8)*argbegin) != '"' )
		{
			err = 3;
		}
		else if ( argbegin[1] )
		{
			err = 4;
		}
	}
	if ( err )
	{
		printf("netcnf: \"%s\" line %d: ", e->fname, e->lno);
		switch ( err )
		{
			case 1:
				printf("invalid escape (%s)", e_arg);
				break;
			case 2:
				printf("missing hexa-decimal(%s)", e_arg);
				break;
			case 3:
				printf("invalid quote (%s)", e_arg);
				break;
			case 4:
				printf("invalid extra chars (%s)", e_arg);
				break;
			default:
				break;
		}
		printf("\n");
		e->syntax_err += 1;
		return 0;
	}
	*dbuf = 0;
	return (const char *)e->dbuf;
}

static char *do_alloc_mem_for_write(sceNetCnfEnv_t *e, const char *str)
{
	char *strptr;

	strptr = do_alloc_mem_inner(e, (unsigned int)strlen(str) + 1, 0);
	if ( !strptr )
		return 0;
	strcpy(strptr, str);
	return strptr;
}

static char *do_check_e_arg(sceNetCnfEnv_t *e, const char *e_arg)
{
	const char *strptr;

	strptr = do_netcnf_parse_string(e, e_arg);
	return strptr ? do_alloc_mem_for_write(e, strptr) : 0;
}

static int do_parse_number(sceNetCnfEnv_t *e, const char *e_arg, unsigned int *n_result)
{
	const char *e_arg_1;
	unsigned int curbasex;
	unsigned int curnum;

	e_arg_1 = e_arg;
	curbasex = 10;
	if ( *e_arg == '0' && e_arg[1] )
	{
		e_arg_1 = e_arg + 1;
		curbasex = 8;
		if ( e_arg[1] == 'x' )
		{
			e_arg_1 = e_arg + 2;
			curbasex = 16;
		}
	}
	curnum = 0;
	if ( *e_arg_1 )
	{
		while ( 1 )
		{
			u32 e_arg_1_num;

			e_arg_1_num = (((u8)*e_arg_1)) - '0';
			if ( ((u8)*e_arg_1) - (unsigned int)'0' >= 0xA )
			{
				e_arg_1_num = (((u8)*e_arg_1)) - 'W';
				if ( ((u8)*e_arg_1) - (unsigned int)'a' >= 6 )
					break;
			}
			if ( e_arg_1_num >= curbasex )
				break;
			e_arg_1 += 1;
			curnum = curnum * curbasex + e_arg_1_num;
			if ( !*e_arg_1 )
			{
				*n_result = curnum;
				return 0;
			}
		}
	}
	printf("netcnf: \"%s\" line %d: ", e->fname, e->lno);
	printf("invalid digit (%s)", e_arg);
	printf("\n");
	e->syntax_err += 1;
	return -1;
}

static int do_netcnfname2address_wrap(sceNetCnfEnv_t *e, const char *buf, sceNetCnfAddress_t *paddr)
{
	int errret;

	errret = sceNetCnfName2Address(paddr, buf);
	if ( errret < 0 )
	{
		printf("netcnf: \"%s\" line %d: ", e->fname, e->lno);
		printf("sceNetCnfName2Address(%s) -> %d\n", buf, errret);
		printf("\n");
		e->syntax_err += 1;
		return -1;
	}
	return 0;
}

static int do_parse_phone_argument(sceNetCnfEnv_t *e, int opt_argc, const char **opt_argv, unsigned int *p_result)
{
	int i;
	unsigned int bitflags1;
	unsigned int numval;

	bitflags1 = 0;
	for ( i = 0; i < opt_argc; i += 1 )
	{
		if ( !strcmp("phase", opt_argv[i]) )
		{
			bitflags1 |= 1;
		}
		else if ( !strcmp("cp", opt_argv[i]) )
		{
			bitflags1 |= 2;
		}
		else if ( !strcmp("auth", opt_argv[i]) )
		{
			bitflags1 |= 4;
		}
		else if ( !strcmp("chat", opt_argv[i]) )
		{
			bitflags1 |= 8;
		}
		else if ( !strcmp("private", opt_argv[i]) )
		{
			bitflags1 |= 0x10;
		}
		else if ( !strcmp("dll", opt_argv[i]) )
		{
			bitflags1 |= 0x20;
		}
		else if ( !strcmp("dump", opt_argv[i]) )
		{
			bitflags1 |= 0x40;
		}
		else if ( !strcmp("timer", opt_argv[i]) )
		{
			bitflags1 |= 0x10000;
		}
		else if ( !strcmp("event", opt_argv[i]) )
		{
			bitflags1 |= 0x20000;
		}
		else if ( do_parse_number(e, opt_argv[i], &numval) )
			return -1;
	}
	*p_result = bitflags1;
	return 0;
}

static int do_check_interface_keyword(
	sceNetCnfEnv_t *e, const char *display_name_arg, const char *attach_ifc_arg, const char *attach_dev_arg)
{
	struct sceNetCnfPair *cnfpair1;
	struct sceNetCnfPair *pair_tail;

	cnfpair1 = (sceNetCnfPair_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfPair_t), 2);
	if ( !cnfpair1 )
		return -1;
	cnfpair1->display_name = (u8 *)do_check_e_arg(e, display_name_arg);
	if ( !cnfpair1->display_name )
		return -1;
	cnfpair1->attach_ifc = (u8 *)do_check_e_arg(e, attach_ifc_arg);
	if ( !cnfpair1->attach_ifc )
		return -1;
	if ( attach_dev_arg && *attach_dev_arg )
	{
		cnfpair1->attach_dev = (u8 *)do_check_e_arg(e, attach_dev_arg);
		if ( !cnfpair1->attach_dev )
			return -1;
	}
	pair_tail = e->root->pair_tail;
	cnfpair1->back = pair_tail;
	if ( !pair_tail )
		pair_tail = (sceNetCnfPair_t *)e->root;
	pair_tail->forw = cnfpair1;
	cnfpair1->forw = 0;
	e->root->pair_tail = cnfpair1;
	return 0;
}

static int do_check_nameserver(sceNetCnfEnv_t *e, struct sceNetCnfInterface *ifc, int opt_argc, const char **opt_argv)
{
	int addordel;
	nameserver_t *nameservermem_1;

	if ( opt_argc < 3 )
		return 0;

	if ( !strcmp("add", opt_argv[1]) )
		addordel = 1;
	else if ( !strcmp("del", opt_argv[1]) )
		addordel = 2;
	else
		return 0;
	nameservermem_1 = (nameserver_t *)do_alloc_mem_inner(e, sizeof(nameserver_t), 2);
	if ( !nameservermem_1 )
		return -1;
	nameservermem_1->cmd.code = addordel;
	if ( do_netcnfname2address_wrap(e, opt_argv[2], &nameservermem_1->address) )
		return -1;
	nameservermem_1->cmd.back = ifc->cmd_tail;
	if ( ifc->cmd_tail )
		ifc->cmd_tail->forw = &nameservermem_1->cmd;
	else
		ifc->cmd_head = &nameservermem_1->cmd;
	nameservermem_1->cmd.forw = 0;
	ifc->cmd_tail = &nameservermem_1->cmd;
	return 0;
}

static int do_check_route(sceNetCnfEnv_t *e, struct sceNetCnfInterface *ifc, int opt_argc, const char **opt_argv)
{
	int addordel;
	route_t *route_mem_1;
	int i;

	if ( opt_argc < 3 )
		return 0;
	if ( !strcmp("add", opt_argv[1]) )
		addordel = 3;
	else if ( !strcmp("del", opt_argv[1]) )
		addordel = 4;
	else
		return 0;
	route_mem_1 = (route_t *)do_alloc_mem_inner(e, sizeof(route_t), 2);
	if ( !route_mem_1 )
		return -1;
	i = 2;
	route_mem_1->cmd.code = addordel;
	if ( !strcmp("-net", opt_argv[i]) )
	{
		i += 1;
		route_mem_1->re.flags &= ~2;
	}
	else if ( !strcmp("-host", opt_argv[i]) )
	{
		i += 1;
		route_mem_1->re.flags |= 2;
	}
	if ( i >= opt_argc )
		return 0;
	if ( do_netcnfname2address_wrap(e, 0, &route_mem_1->re.dstaddr) )
		return -1;
	if ( do_netcnfname2address_wrap(e, 0, &route_mem_1->re.gateway) )
		return -1;
	if ( do_netcnfname2address_wrap(e, 0, &route_mem_1->re.genmask) )
		return -1;
	if ( (strcmp("default", opt_argv[i])
				&& do_netcnfname2address_wrap(e, (const char *)opt_argv[i], &route_mem_1->re.dstaddr)) )
		return -1;
	i += 1;
	for ( ; i < opt_argc; i += 2 )
	{
		if ( !strcmp("gw", opt_argv[i]) )
		{
			if ( do_netcnfname2address_wrap(e, opt_argv[i + 1], &route_mem_1->re.gateway) )
				return -1;
			route_mem_1->re.flags |= 4;
		}
		else if ( !strcmp("netmask", opt_argv[i]) )
		{
			if ( do_netcnfname2address_wrap(e, opt_argv[i + 1], &route_mem_1->re.genmask) )
				return -1;
		}
	}
	route_mem_1->cmd.back = ifc->cmd_tail;
	if ( ifc->cmd_tail )
		ifc->cmd_tail->forw = &route_mem_1->cmd;
	else
		ifc->cmd_head = &route_mem_1->cmd;
	route_mem_1->cmd.forw = 0;
	ifc->cmd_tail = &route_mem_1->cmd;
	return 0;
}

static void do_init_ifc_inner(sceNetCnfInterface_t *ifc)
{
	const struct netcnf_option *curentry1;

	for ( curentry1 = g_options_attach_cnf; curentry1->m_key; curentry1 += 1 )
	{
		switch ( curentry1->m_type )
		{
			case '1':
			case 'b':
			case 'c':
				*((char *)ifc + curentry1->m_offset) = 0xFF;
				break;
			case '4':
			case 'D':
			case 'L':
			case 'P':
			case 'T':
				*(u32 *)((char *)ifc + curentry1->m_offset) = 0xFFFFFFFF;
				break;
			default:
				break;
		}
	}
}

static int do_check_args(sceNetCnfEnv_t *e, struct sceNetCnfUnknownList *unknown_list)
{
	unsigned int lenx1;
	int i;
	struct sceNetCnfUnknown *listtmp;
	struct sceNetCnfUnknown *cpydst_1;

	lenx1 = 0;
	for ( i = 0; i < e->ac; i += 1 )
	{
		lenx1 += 3 + strlen(e->av[i]);
	}
	listtmp = (sceNetCnfUnknown_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfUnknown_t) + lenx1, 2);
	if ( !listtmp )
		return -1;
	cpydst_1 = listtmp + 1;
	for ( i = 0; i < e->ac; i += 1 )
	{
		unsigned int cpysz;

		cpysz = (unsigned int)strlen(e->av[i]);
		memcpy(cpydst_1, e->av[i], cpysz);
		((char *)cpydst_1)[cpysz] = 32 * (i < e->ac - 1);
		cpydst_1 = (struct sceNetCnfUnknown *)&((char *)cpydst_1)[cpysz + 1];
	}
	listtmp->back = unknown_list->tail;
	if ( unknown_list->tail )
		unknown_list->tail->forw = listtmp;
	else
		unknown_list->head = listtmp;
	listtmp->forw = 0;
	unknown_list->tail = listtmp;
	return 0;
}

static int do_check_other_keywords(
	sceNetCnfEnv_t *e, const struct netcnf_option *options, void *cnfdata, struct sceNetCnfUnknownList *unknown_list)
{
	int wasprefixed;
	unsigned int numval;

	wasprefixed = (e->av[0][0] == '-') ? 1 : 0;
	if ( e->av[0][wasprefixed] )
	{
		for ( ; options->m_key && strcmp(&(e->av[0])[wasprefixed], options->m_key); options += 1 )
			;
		if ( !options->m_key )
			return do_check_args(e, unknown_list);
		switch ( options->m_type )
		{
			case '1':
				numval = 0xFF;
				if ( !wasprefixed && (e->ac < 2 || do_parse_number(e, e->av[1], &numval)) )
					break;
				*((u8 *)cnfdata + options->m_offset) = (u8)numval;
				return 0;
			case '4':
				numval = 0xFFFFFFFF;
				if ( !wasprefixed && (e->ac < 2 || do_parse_number(e, e->av[1], &numval)) )
					break;
				*(u32 *)((char *)cnfdata + options->m_offset) = numval;
				return 0;
			case 'A':
				if ( !wasprefixed )
				{
					if ( e->ac < 2 )
						break;
					if ( !strcmp("any", (const char *)e->av[1]) )
					{
						numval = 0;
					}
					else if ( !strcmp("pap", (const char *)e->av[1]) )
					{
						numval = 1;
					}
					else if ( !strcmp("chap", (const char *)e->av[1]) )
					{
						numval = 2;
					}
					else if ( !strcmp("pap/chap", (const char *)e->av[1]) )
					{
						numval = 3;
					}
					else if ( !strcmp("chap/pap", (const char *)e->av[1]) )
					{
						numval = 4;
					}
					else if ( do_parse_number(e, e->av[1], &numval) )
					{
						return -1;
					}
					*((u8 *)cnfdata + options->m_offset) = (u8)numval;
				}
				if ( !strcmp("want.auth", (const char *)e->av[0]) )
					*((u8 *)cnfdata + 171) = !wasprefixed;
				else
					*((u8 *)cnfdata + 247) = !wasprefixed;
				return 0;
			case 'C':
				if ( !wasprefixed )
				{
					if ( e->ac < 2 || do_parse_number(e, e->av[1], &numval) )
						break;
					*(u32 *)((char *)cnfdata + options->m_offset) = numval;
				}
				if ( !strcmp("want.accm", (const char *)e->av[0]) )
					*((u8 *)cnfdata + 170) = !wasprefixed;
				else
					*((u8 *)cnfdata + 246) = !wasprefixed;
				return 0;
			case 'D':
				numval = 0xFFFFFFFF;
				if ( !wasprefixed )
				{
					if ( e->ac < 2 )
						break;
					if ( !strcmp("tone", (const char *)e->av[1]) )
					{
						numval = 0;
					}
					else if ( !strcmp("pulse", (const char *)e->av[1]) )
					{
						numval = 1;
					}
					else if ( !strcmp("any", (const char *)e->av[1]) )
					{
						numval = 2;
					}
					else if ( do_parse_number(e, e->av[1], &numval) )
					{
						return -1;
					}
				}
				*(u32 *)((char *)cnfdata + options->m_offset) = numval;
				return 0;
			case 'L':
				numval = 0xFFFFFFFF;
				if ( !wasprefixed && do_parse_phone_argument(e, e->ac - 1, (const char **)&e->av[1], &numval) )
					return -1;
				*(u32 *)((char *)cnfdata + options->m_offset) = numval;
				return 0;
			case 'M':
				if ( !wasprefixed )
				{
					if ( e->ac < 2 || do_parse_number(e, e->av[1], &numval) )
						break;
					*(u16 *)((char *)cnfdata + options->m_offset) = (u16)numval;
				}
				if ( !strcmp("want.mru", (const char *)e->av[0]) )
					*((u8 *)cnfdata + 169) = !wasprefixed;
				else
					*((u8 *)cnfdata + 245) = !wasprefixed;
				return 0;
			case 'P':
				numval = 0xFFFFFFFF;
				if ( !wasprefixed )
				{
					if ( e->ac < 2 )
						break;
					if ( !strcmp("auto", (const char *)e->av[1]) )
					{
						numval = 1;
					}
					else if ( !strcmp("10", (const char *)e->av[1]) )
					{
						numval = 2;
					}
					else if ( !strcmp("10_fd", (const char *)e->av[1]) )
					{
						numval = 3;
					}
					else if ( !strcmp("10_fd_pause", (const char *)e->av[1]) )
					{
						numval = 4;
					}
					else if ( !strcmp("tx", (const char *)e->av[1]) )
					{
						numval = 5;
					}
					else if ( !strcmp("tx_fd", (const char *)e->av[1]) )
					{
						numval = 6;
					}
					else if ( !strcmp("tx_fd_pause", (const char *)e->av[1]) )
					{
						numval = 7;
					}
					else if ( do_parse_number(e, e->av[1], &numval) )
					{
						return -1;
					}
				}
				*(u32 *)((char *)cnfdata + options->m_offset) = numval;
				return 0;
			case 'T':
				numval = 0xFFFFFFFF;
				if ( !wasprefixed )
				{
					if ( e->ac < 2 )
						break;
					if ( !strcmp("any", (const char *)e->av[1]) )
					{
						numval = 0;
					}
					if ( !strcmp("eth", (const char *)e->av[1]) )
					{
						numval = 1;
					}
					else if ( !strcmp("ppp", (const char *)e->av[1]) )
					{
						numval = 2;
					}
					else if ( !strcmp("nic", (const char *)e->av[1]) )
					{
						numval = 3;
					}
					else if ( do_parse_number(e, e->av[1], &numval) )
					{
						return -1;
					}
				}
				*(u32 *)((char *)cnfdata + options->m_offset) = numval;
				return 0;
			case 'b':
				numval = !wasprefixed;
				*((u8 *)cnfdata + options->m_offset) = (u8)numval;
				return 0;
			case 'c':
				numval = 255;
				if ( !wasprefixed )
				{
					if ( e->ac < 2 )
						break;
					if ( !strcmp("no", (const char *)e->av[1]) )
					{
						numval = 0;
					}
					else if ( !strcmp("md5", (const char *)e->av[1]) )
					{
						numval = 5;
					}
					else if ( !strcmp("ms", (const char *)e->av[1]) )
					{
						numval = 128;
					}
					else if ( !strcmp("ms-v1", (const char *)e->av[1]) )
					{
						numval = 128;
					}
					else if ( !strcmp("ms-v2", (const char *)e->av[1]) )
					{
						numval = 129;
					}
					else if ( do_parse_number(e, e->av[1], &numval) )
					{
						return -1;
					}
				}
				*((u8 *)cnfdata + options->m_offset) = (u8)numval;
				return 0;
			case 'p':
				if ( !wasprefixed )
				{
					if ( e->ac < 2 )
					{
						break;
					}
					*(char **)((char *)cnfdata + options->m_offset) = do_check_e_arg(e, e->av[1]);
					return (!*(char **)((char *)cnfdata + options->m_offset)) ? -1 : 0;
				}
				*(u32 *)((char *)cnfdata + options->m_offset) = 0;
				return 0;
			default:
				return printf("netcnf: internal load err (%d, type=%c)\n", 606, options->m_type);
		}
	}
	printf("netcnf: \"%s\" line %d: ", e->fname, e->lno);
	printf("ac=%d", e->ac);
	printf("\n");
	e->syntax_err += 1;
	return -1;
}

static int do_handle_net_cnf(sceNetCnfEnv_t *e, void *userdata)
{
	int wasprefixed;

	(void)userdata;
	wasprefixed = (e->av[0][0] == '-') ? 1 : 0;
	if ( strcmp("interface", &(e->av[0])[wasprefixed]) )
	{
		if ( strcmp("zero_prefix", &(e->av[0])[wasprefixed]) )
			return do_check_other_keywords(e, g_options_net_cnf, e->root, &e->root->unknown_list);
		printf("netcnf: \"%s\" line %d: ", e->fname, e->lno);
		printf("obsoleted keyword (%s)", &(e->av[0])[wasprefixed]);
		printf("\n");
		e->syntax_err += 1;
		return 0;
	}
	if ( wasprefixed )
		return 0;
	if ( e->ac < 3 )
	{
		printf("netcnf: \"%s\" line %d: ", e->fname, e->lno);
		printf("ac=%d", e->ac);
		printf("\n");
		e->syntax_err += 1;
		return -1;
	}
	return !do_check_interface_keyword(e, e->av[1], e->av[2], (e->ac >= 4) ? e->av[3] : 0) ? 0 : -1;
}

static int do_handle_attach_cnf(sceNetCnfEnv_t *e, void *userdata)
{
	int wasprefixed;
	struct sceNetCnfInterface *ifc;

	ifc = (struct sceNetCnfInterface *)userdata;
	wasprefixed = (e->av[0][0] == '-') ? 1 : 0;
	if ( !strncmp("phone_number", &(e->av[0])[wasprefixed], 12) )
	{
		int keyasnum;

		keyasnum = 0;
		if ( e->av[0][wasprefixed + 12] )
		{
			if ( !isdigit(e->av[0][wasprefixed + 12]) || e->av[0][wasprefixed + 13] )
				return 0;
			keyasnum = e->av[0][wasprefixed + 12] - 48;
		}
		if ( wasprefixed )
		{
			ifc->phone_numbers[keyasnum] = 0;
			return 0;
		}
		if ( e->ac >= 2 )
		{
			ifc->phone_numbers[keyasnum] = (u8 *)do_check_e_arg(e, e->av[1]);
			return (ifc->phone_numbers[keyasnum]) ? 0 : -1;
		}
	}
	else if ( !strcmp("nameserver", &(e->av[0])[wasprefixed]) )
	{
		if ( !wasprefixed )
			return do_check_nameserver(e, ifc, e->ac, &e->av[0]);
	}
	else if ( !strcmp("route", &(e->av[0])[wasprefixed]) )
	{
		if ( !wasprefixed )
			return do_check_route(e, ifc, e->ac, &e->av[0]);
	}
	else if ( !strcmp("zero_prefix", &(e->av[0])[wasprefixed]) || !strcmp("dial_cnf", &(e->av[0])[wasprefixed]) )
	{
		printf("netcnf: \"%s\" line %d: ", e->fname, e->lno);
		printf("obsoleted keyword (%s)", &(e->av[0])[wasprefixed]);
		printf("\n");
		e->syntax_err += 1;
	}
	else
	{
		return do_check_other_keywords(e, g_options_attach_cnf, ifc, &ifc->unknown_list);
	}
	return 0;
}

static int do_handle_dial_cnf(sceNetCnfEnv_t *e, void *userdata)
{
	int wasprefixed;
	struct sceNetCnfDial *dial;

	dial = (struct sceNetCnfDial *)userdata;
	wasprefixed = (e->av[0][0] == '-') ? 1 : 0;
	if ( strcmp("dialing_type_string", &(e->av[0])[wasprefixed]) )
		return do_check_other_keywords(e, g_options_dial_cnf, dial, &dial->unknown_list);
	if ( wasprefixed || e->ac < 2 )
		return 0;
	dial->tone_dial = (u8 *)do_check_e_arg(e, e->av[1]);
	if ( !dial->tone_dial )
		return -1;
	if ( e->ac < 3 )
		return 0;
	dial->pulse_dial = (u8 *)do_check_e_arg(e, e->av[2]);
	if ( !dial->pulse_dial )
		return -1;
	if ( e->ac < 4 )
		return 0;
	dial->any_dial = (u8 *)do_check_e_arg(e, e->av[3]);
	return dial->any_dial ? 0 : -1;
}

static int
do_check_line_buffer(sceNetCnfEnv_t *e, u8 *lbuf, int (*readcb)(sceNetCnfEnv_t *e, void *userdata), void *userdata)
{
	u8 *i;
	char *j;

	for ( i = lbuf; e->lbuf < i && i[-1] < 0x21u; i -= 1 )
		;
	*i = 0;
	for ( j = (char *)e->lbuf; *j && isspace(*j); j += 1 )
		;
	e->ac = 0;
	while ( *j && e->ac < '\n' && (u8)*j != '#' )
	{
		u32 condtmp1;

		e->av[e->ac] = j;
		condtmp1 = 0;
		if ( *j == '#' )
		{
			*j = 0;
			break;
		}
		if ( !isspace(*j) )
		{
			while ( *j )
			{
				if ( *j == '\\' )
				{
					if ( j[1] )
						j += 1;
				}
				else
				{
					condtmp1 = (*j == '"') ? (condtmp1 ? 0 : 1) : (condtmp1 ? 1 : 0);
				}
				j += 1;
				if ( !condtmp1 && (*j == '#' || isspace(*j)) )
				{
					break;
				}
			}
		}
		if ( *j == '#' )
		{
			*j = 0;
			break;
		}
		if ( *j )
		{
			*j = 0;
			j += 1;
		}
		for ( ; *j && isspace(*j); j += 1 )
			;
		e->ac += 1;
	}
	*j = 0;
	return (e->ac <= 0) ? 0 : readcb(e, userdata);
}

static int do_read_netcnf(sceNetCnfEnv_t *e, const char *netcnf_path, char **netcnf_heap_ptr, int is_attach_cnf)
{
	int result;

	result = (!is_attach_cnf || e->f_no_decode) ? do_read_netcnf_no_decode(netcnf_path, netcnf_heap_ptr) :
																								do_read_netcnf_decode(netcnf_path, netcnf_heap_ptr);
	if ( result < 0 )
		e->file_err += 1;
	return result;
}

static const char *do_handle_netcnf_prerw(sceNetCnfEnv_t *e, const char *entry_buffer)
{
	const char *result;

	result = do_handle_netcnf_dirname(e->dir_name, entry_buffer, (char *)e->lbuf);
	return (result == (const char *)e->lbuf) ? do_alloc_mem_for_write(e, result) : result;
}

static int do_netcnf_read_related(
	sceNetCnfEnv_t *e, const char *path, int (*readcb)(sceNetCnfEnv_t *e, void *userdata), void *userdata)
{
	int cur_linelen;
	const char *fullpath;
	int read_res1;
	u8 *lbuf;
	char *ptr;
	int i;

	cur_linelen = 0;
	if ( e->f_verbose )
		printf("netcnf: dir=%s path=%s\n", e->dir_name ? e->dir_name : "NULL", path ? path : "NULL");
	fullpath = do_handle_netcnf_prerw(e, path);
	if ( !fullpath )
		return -1;
	if ( e->f_verbose )
	{
		printf("netcnf: reading \"%s\" as ", fullpath);
		if ( (char *)readcb == (char *)do_handle_net_cnf )
		{
			printf("NET_CNF");
		}
		else if ( (char *)readcb == (char *)do_handle_attach_cnf )
		{
			printf("ATTACH_CNF");
		}
		else if ( (char *)readcb == (char *)do_handle_dial_cnf )
		{
			printf("DIAL_CNF");
		}
		else
		{
			printf("???");
		}
		printf("\n");
	}
	e->fname = fullpath;
	read_res1 = do_read_netcnf(e, fullpath, &ptr, readcb == do_handle_attach_cnf);
	if ( read_res1 < 0 )
	{
		printf("netcnf: can't load %s (%d)\n", e->fname, read_res1);
		return -1;
	}
	e->lno = 0;
	if ( !e->f_no_check_magic && (read_res1 < 36 || strncmp(ptr, "# <Sony Computer Entertainment Inc.>", 36)) )
	{
		printf("netcnf: decoding error (magic=\"");
		for ( i = 0; i < read_res1 && i < 36; i += 1 )
		{
			printf("%c", ((u8)ptr[i] - (unsigned int)' ' < '_') ? ((u8)ptr[i]) : '?');
		}
		printf("\")\n");
		do_free_heapmem(ptr);
		return -15;
	}
	lbuf = e->lbuf;
	for ( i = 0; i < read_res1; i += 1 )
	{
		if ( ptr[i] == '\n' )
		{
			e->lno += 1;
			if ( e->lbuf < lbuf && lbuf[-1] == '\\' )
			{
				lbuf -= 1;
			}
			else
			{
				cur_linelen += do_check_line_buffer(e, lbuf, readcb, userdata);
				lbuf = e->lbuf;
			}
		}
		else
		{
			if ( lbuf < &e->lbuf[1023] && ptr[i] != '\r' )
			{
				*lbuf = (u8)ptr[i];
				lbuf += 1;
			}
		}
	}
	if ( e->lbuf < lbuf )
		cur_linelen += do_check_line_buffer(e, lbuf, readcb, userdata);
	do_free_heapmem(ptr);
	return cur_linelen;
}

static int do_netcnf_dial_related(sceNetCnfEnv_t *e)
{
	e->root = (sceNetCnfRoot_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfRoot_t), 2);
	if ( !e->root )
		return -2;
	e->root->version = 3;
	e->root->redial_count = -1;
	e->root->redial_interval = -1;
	e->root->dialing_type = -1;
	return do_netcnf_read_related(e, e->arg_fname, do_handle_net_cnf, 0);
}

static int do_netcnf_ifc_related(sceNetCnfEnv_t *e)
{
	e->ifc = (sceNetCnfInterface_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfInterface_t), 2);
	if ( !e->ifc )
		return -2;
	do_init_ifc_inner(e->ifc);
	return do_netcnf_read_related(e, e->arg_fname, do_handle_attach_cnf, e->ifc);
}

static void do_dialauth_related(sceNetCnfInterface_t *ncid, struct sceNetCnfInterface *ncis)
{
	int i;
	u32 typadd1_1;
	u32 typadd1_3;
	u8 typadd1;

	if ( !ncis )
	{
		return;
	}
	for ( i = 0; g_options_attach_cnf[i].m_key; i += 1 )
	{
		switch ( g_options_attach_cnf[i].m_type )
		{
			case '1':
			case 'b':
			case 'c':
				typadd1 = *((u8 *)&ncis->type + g_options_attach_cnf[i].m_offset);
				if ( typadd1 != 0xFF )
					*((u8 *)&ncid->type + g_options_attach_cnf[i].m_offset) = typadd1;
				break;
			case '4':
			case 'D':
			case 'L':
			case 'P':
			case 'T':
				typadd1_1 = *(u32 *)((char *)&ncis->type + g_options_attach_cnf[i].m_offset);
				if ( typadd1_1 != 0xFFFFFFFF )
					*(u32 *)((char *)&ncid->type + g_options_attach_cnf[i].m_offset) = typadd1_1;
				break;
			case 'A':
				if ( !strcmp("want.auth", g_options_attach_cnf[i].m_key) )
				{
					if ( ncis->want.f_auth )
					{
						ncid->want.f_auth = 1;
						ncid->want.auth = ncis->want.auth;
					}
				}
				else if ( ncis->allow.f_auth )
				{
					ncid->allow.f_auth = 1;
					ncid->allow.auth = ncis->allow.auth;
				}
				break;
			case 'C':
				if ( !strcmp("want.accm", g_options_attach_cnf[i].m_key) )
				{
					if ( ncis->want.f_accm )
					{
						ncid->want.f_accm = 1;
						ncid->want.accm = ncis->want.accm;
					}
				}
				else if ( ncis->allow.f_accm )
				{
					ncid->allow.f_accm = 1;
					ncid->allow.accm = ncis->allow.accm;
				}
				break;
			case 'M':
				if ( !strcmp("want.mru", g_options_attach_cnf[i].m_key) )
				{
					if ( ncis->want.f_mru )
					{
						ncid->want.f_mru = 1;
						ncid->want.mru = ncis->want.mru;
					}
				}
				else if ( ncis->allow.f_mru )
				{
					ncid->allow.f_mru = 1;
					ncid->allow.mru = ncis->allow.mru;
				}
				break;
			case 'p':
				typadd1_3 = *(u32 *)((char *)&ncis->type + g_options_attach_cnf[i].m_offset);
				if ( typadd1_3 )
					*(u32 *)((char *)&ncid->type + g_options_attach_cnf[i].m_offset) = typadd1_3;
				break;
			default:
				break;
		}
	}
	for ( i = 0; i < 10; i += 1 )
	{
		if ( ncis->phone_numbers[i] )
			ncid->phone_numbers[i] = ncis->phone_numbers[i];
	}
}

static int do_merge_conf_inner(sceNetCnfEnv_t *e)
{
	struct sceNetCnfPair *pair_head;

	if ( !e->root )
	{
		return -1;
	}
	for ( pair_head = e->root->pair_head; pair_head; pair_head = pair_head->forw )
	{
		int type;

		if ( !pair_head->ctl )
		{
			pair_head->ctl = (sceNetCnfCtl_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfCtl_t), 2);
			if ( !pair_head->ctl )
				return -2;
		}
		if ( !pair_head->ctl->dial )
		{
			pair_head->ctl->dial = (sceNetCnfDial_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfDial_t), 2);
			if ( !pair_head->ctl->dial )
				return -2;
		}
		if ( !pair_head->ctl->ifc )
		{
			pair_head->ctl->ifc = (sceNetCnfInterface_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfInterface_t), 2);
			if ( !pair_head->ctl->ifc )
				return -2;
		}
		do_init_ifc_inner(pair_head->ctl->ifc);
		pair_head->ctl->ifc->chat_additional = e->root->chat_additional;
		pair_head->ctl->ifc->redial_count = e->root->redial_count;
		pair_head->ctl->ifc->redial_interval = e->root->redial_interval;
		pair_head->ctl->ifc->outside_number = e->root->outside_number;
		pair_head->ctl->ifc->outside_delay = e->root->outside_delay;
		pair_head->ctl->ifc->dialing_type = e->root->dialing_type;
		do_dialauth_related(pair_head->ctl->ifc, pair_head->ifc);
		type = pair_head->dev->type;
		pair_head->dev->type = -1;
		do_dialauth_related(pair_head->ctl->ifc, pair_head->dev);
		pair_head->dev->type = type;
	}
	return 0;
}

static int do_load_conf_inner(sceNetCnfEnv_t *e)
{
	int retres1;
	struct sceNetCnfPair *pair_head;

	retres1 = 0;
	switch ( e->req )
	{
		case 1:
		{
			int ifcres1_1;

			ifcres1_1 = do_netcnf_dial_related(e);
			if ( ifcres1_1 )
				return ifcres1_1;
			if ( !e->root )
				return -14;
			pair_head = e->root->pair_head;
			if ( !pair_head )
				return -14;
			if ( index(e->arg_fname, ':') )
				e->dir_name = e->arg_fname;
			for ( ; pair_head; pair_head = pair_head->forw )
			{
				if ( pair_head->attach_ifc )
				{
					e->arg_fname = (char *)pair_head->attach_ifc;
					ifcres1_1 = do_netcnf_ifc_related(e);
					if ( ifcres1_1 )
					{
						printf("netcnf: load_attach ifc(%d)\n", ifcres1_1);
						pair_head->ifc = 0;
						if ( (unsigned int)(ifcres1_1 + 15) < 2 )
						{
							pair_head->dev = 0;
							break;
						}
						retres1 = -21;
					}
					else
					{
						pair_head->ifc = e->ifc;
					}
				}
				else
				{
					pair_head->ifc = 0;
				}
				if ( pair_head->attach_dev )
				{
					e->arg_fname = (char *)pair_head->attach_dev;
					ifcres1_1 = do_netcnf_ifc_related(e);
					if ( ifcres1_1 )
					{
						printf("netcnf: load_attach dev(%d)\n", ifcres1_1);
						pair_head->dev = 0;
						if ( (unsigned int)(ifcres1_1 + 15) < 2 )
							break;
						retres1 = -21;
					}
					else
					{
						pair_head->dev = e->ifc;
					}
				}
				else
				{
					pair_head->dev = 0;
				}
			}
			return (retres1 == -21) ? -21 : ifcres1_1;
		}
		case 2:
			return do_netcnf_ifc_related(e);
		default:
			return -1;
	}
}

static int do_load_dial_inner(sceNetCnfEnv_t *e, sceNetCnfPair_t *pair)
{
	if ( !pair->ctl )
		return -1;
	pair->ctl->dial = (sceNetCnfDial_t *)do_alloc_mem_inner(e, sizeof(sceNetCnfDial_t), 2);
	return pair->ctl->dial ? do_netcnf_read_related(e, e->arg_fname, do_handle_dial_cnf, pair->ctl->dial) : -2;
}

static int do_netcnf_vsprintf_buffer(sceNetCnfEnv_t *e, const char *fmt, va_list va)
{
	char *mem_ptr_03;
	void *mem_ptr_rval_04;
	int has_negative;
	char has_sero;
	int strlened;
	int fmt_flag_str;
	char *mem_ptr_01;
	char *strptr1;
	int strlenmax;
	int strpad1;
	int cur_va1;
	int strlencalc;
	char *mem_ptr_02;
	char *mem_ptr_04;
	char *mem_ptr_05;
	char *i;
	char *mem_ptr_0a;
	void *mem_ptr_rval_02;
	char i_curchr2;
	void *mem_ptr_rval_03;
	char *mem_ptr_07;
	char *mem_ptr_09;
	void *mem_ptr_rval_01;
	char *mem_ptr_08;
	char strptr_curchr1;
	char *mem_ptr_06;

	strptr1 = 0;
	strlenmax = 0;
	while ( *fmt )
	{
		if ( *fmt == '%' )
		{
			fmt += 1;
			has_negative = 0;
			if ( *fmt == '-' )
			{
				fmt += 1;
				has_negative = 1;
			}
			has_sero = ' ';
			if ( *fmt == '0' )
			{
				has_sero = '0';
				fmt += 1;
			}
			strlened = 0;
			for ( ; isdigit(*fmt); fmt += 1 )
			{
				strlened = 10 * strlened - '0' + *fmt;
			}
			if ( *fmt == 'l' )
				fmt += 1;
			fmt_flag_str = -1;
			strpad1 = 0;
			switch ( *fmt )
			{
				case 'c':
					mem_ptr_01 = (char *)e->mem_ptr;
					mem_ptr_rval_04 = mem_ptr_01 + 1;
					if ( mem_ptr_01 >= (char *)e->mem_last )
						return -2;
					*mem_ptr_01 = (char)va_arg(va, int);
					e->mem_ptr = mem_ptr_rval_04;
					fmt += 1;
					continue;
				case 'p':
					has_sero = '0';
					if ( strlened < 8 )
						strlened = 8;
					strpad1 = 16;
					break;
				case 'X':
				case 'x':
					strpad1 = 16;
					break;
				case 'd':
				case 'u':
					strpad1 = 10;
					break;
				case 'o':
					strpad1 = 8;
					break;
				case 'S':
					fmt_flag_str = 1;
					break;
				case 's':
					fmt_flag_str = 0;
					break;
				default:
					mem_ptr_03 = (char *)e->mem_ptr;
					mem_ptr_rval_04 = mem_ptr_03 + 1;
					if ( mem_ptr_03 >= (char *)e->mem_last )
						return -2;
					*mem_ptr_03 = *fmt;
					e->mem_ptr = mem_ptr_rval_04;
					fmt += 1;
					continue;
			}
			if ( fmt_flag_str != -1 )
			{
				strptr1 = va_arg(va, char *);
				strlenmax = 0;
				if ( !strptr1 )
					strpad1 = 0;
			}
			if ( strpad1 )
			{
				cur_va1 = va_arg(va, int);
				strptr1 = __builtin_alloca(strpad1 + 1);
				strptr1[strpad1] = 0;
				strptr1 += strpad1;
				strlenmax = 0;
				if ( *fmt == 'd' && cur_va1 < 0 )
				{
					cur_va1 = -cur_va1;
					strlenmax = 1;
				}
				while ( 1 )
				{
					strptr1 -= 1;
					*strptr1 = ((*fmt == 'X') ? "0123456789ABCDEF" : "0123456789abcdef")[cur_va1 % strpad1];
					cur_va1 /= strpad1;
					if ( !cur_va1 )
						break;
				}
			}
			if ( strptr1 )
			{
				strlencalc = strlenmax ? (strlened + 1) : strlened;
				strlencalc -= strlen(strptr1);
				if ( has_sero == '0' && strlenmax )
				{
					mem_ptr_02 = (char *)e->mem_ptr;
					if ( mem_ptr_02 >= (char *)e->mem_last )
						return -2;
					*mem_ptr_02 = '-';
					e->mem_ptr = mem_ptr_02 + 1;
				}
				if ( !has_negative )
				{
					for ( ; strlencalc > 0; strlencalc -= 1 )
					{
						mem_ptr_04 = (char *)e->mem_ptr;
						if ( mem_ptr_04 >= (char *)e->mem_last )
							return -2;
						*mem_ptr_04 = has_sero;
						e->mem_ptr = mem_ptr_04 + 1;
					}
				}
				if ( has_sero != '0' && strlenmax )
				{
					mem_ptr_05 = (char *)e->mem_ptr;
					if ( mem_ptr_05 >= (char *)e->mem_last )
						return -2;
					*mem_ptr_05 = '-';
					e->mem_ptr = mem_ptr_05 + 1;
				}
				if ( fmt_flag_str != 1 )
				{
					for ( ; *strptr1; strptr1 += 1 )
					{
						strptr_curchr1 = *strptr1;
						mem_ptr_06 = (char *)e->mem_ptr;
						if ( mem_ptr_06 >= (char *)e->mem_last )
							return -2;
						*mem_ptr_06 = strptr_curchr1;
						e->mem_ptr = mem_ptr_06 + 1;
					}
				}
				else
				{
					for ( i = strptr1; *i; i += 1 )
					{
						if (
							(((u8)*i - 0x81 >= 0x1F) && ((u8)*i - 0xE0 >= 0x1D)) || ((u8)((int)*i - 64) >= 0xBDu) || (u8)*i == 0x7F )
						{
							if ( (u8)*i == '"' || (u8)*i == '\\' )
							{
								mem_ptr_07 = (char *)e->mem_ptr;
								if ( mem_ptr_07 >= (char *)e->mem_last )
									return -2;
								*mem_ptr_07 = '\\';
								e->mem_ptr = mem_ptr_07 + 1;
								mem_ptr_rval_03 = mem_ptr_07 + 2;
								if ( mem_ptr_07 + 1 >= (char *)e->mem_last )
									return -2;
								mem_ptr_07[1] = *i;
							}
							else if ( (u8)*i - ' ' < '_' )
							{
								mem_ptr_08 = (char *)e->mem_ptr;
								mem_ptr_rval_03 = mem_ptr_08 + 1;
								if ( mem_ptr_08 >= (char *)e->mem_last )
									return -2;
								*mem_ptr_08 = *i;
							}
							else
							{
								mem_ptr_09 = (char *)e->mem_ptr;
								if ( mem_ptr_09 >= (char *)e->mem_last )
									return -2;
								*mem_ptr_09 = '\\';
								e->mem_ptr = mem_ptr_09 + 1;
								if ( mem_ptr_09 + 1 >= (char *)e->mem_last )
									return -2;
								mem_ptr_09[1] = 'x';
								e->mem_ptr = mem_ptr_09 + 2;
								mem_ptr_rval_01 = mem_ptr_09 + 3;
								if ( mem_ptr_09 + 2 >= (char *)e->mem_last )
									return -2;
								mem_ptr_09[2] = ("0123456789abcdef")[(u8)*i >> 4];
								e->mem_ptr = mem_ptr_rval_01;
								mem_ptr_rval_03 = mem_ptr_09 + 4;
								if ( (char *)mem_ptr_rval_01 >= (char *)e->mem_last )
									return -2;
								mem_ptr_09[3] = ("0123456789abcdef")[(u8)*i & 0xF];
							}
						}
						else
						{
							mem_ptr_0a = (char *)e->mem_ptr;
							mem_ptr_rval_02 = mem_ptr_0a + 1;
							if ( mem_ptr_0a >= (char *)e->mem_last )
								return -2;
							*mem_ptr_0a = *i;
							e->mem_ptr = mem_ptr_rval_02;
							i_curchr2 = *i;
							i += 1;
							if ( (char *)mem_ptr_rval_02 >= (char *)e->mem_last )
								return -2;
							mem_ptr_rval_03 = mem_ptr_0a + 2;
							mem_ptr_0a[1] = i_curchr2;
						}
						e->mem_ptr = mem_ptr_rval_03;
					}
				}
				for ( ; has_negative && strlencalc > 0; strlencalc -= 1 )
				{
					if ( (char *)e->mem_ptr >= (char *)e->mem_last )
						return -2;
					*((char *)e->mem_ptr) = ' ';
					e->mem_ptr = ((char *)e->mem_ptr) + 1;
				}
			}
		}
		else
		{
			mem_ptr_03 = (char *)e->mem_ptr;
			mem_ptr_rval_04 = mem_ptr_03 + 1;
			if ( mem_ptr_03 >= (char *)e->mem_last )
				return -2;
			*mem_ptr_03 = *fmt;
			e->mem_ptr = mem_ptr_rval_04;
		}
		fmt += 1;
	}
	return 0;
}

static int do_netcnf_sprintf_buffer(sceNetCnfEnv_t *e, const char *fmt, ...)
{
	va_list va;
	int retval;

	va_start(va, fmt);
	retval = do_netcnf_vsprintf_buffer(e, fmt, va);
	va_end(va);
	return retval;
}

static int do_netcnf_other_write(sceNetCnfEnv_t *e, const struct netcnf_option *options, void *cnfdata)
{
	char *offsptr1;
	unsigned int offsptr6;
	unsigned int offsptr4;
	int i;

	for ( ; options->m_key; options += 1 )
	{
		unsigned int offsptr3;
		int result;
		const char *lbuf;

		offsptr3 = 0;
		result = 0;
		lbuf = (const char *)e->lbuf;
		switch ( options->m_type )
		{
			case '1':
				if ( *((u8 *)cnfdata + options->m_offset) == 255 )
				{
					lbuf = 0;
					break;
				}
				result = do_netcnf_sprintf_buffer(e, "%s %d\n", options->m_key, *((u8 *)cnfdata + options->m_offset));
				lbuf = 0;
				break;
			case '4':
				offsptr1 = (char *)cnfdata + options->m_offset;
				if ( *(int *)offsptr1 < 0 )
				{
					lbuf = 0;
					break;
				}
				result = do_netcnf_sprintf_buffer(e, "%s %d\n", options->m_key, *(u32 *)offsptr1);
				lbuf = 0;
				break;
			case 'A':
				if ( !strcmp("want.auth", options->m_key) ? !*((u8 *)cnfdata + 171) : !*((u8 *)cnfdata + 247) )
				{
					lbuf = 0;
					break;
				}
				offsptr3 = (u32)(s32) * ((char *)cnfdata + options->m_offset);
				switch ( offsptr3 )
				{
					case 0:
						lbuf = "any";
						break;
					case 1:
						lbuf = "pap";
						break;
					case 2:
						lbuf = "chap";
						break;
					case 3:
						lbuf = "pap/chap";
						break;
					case 4:
						lbuf = "chap/pap";
						break;
					default:
						break;
				}
				break;
			case 'C':
				if ( !strcmp("want.accm", options->m_key) ? !*((u8 *)cnfdata + 170) : !*((u8 *)cnfdata + 246) )
				{
					lbuf = 0;
					break;
				}
				result =
					do_netcnf_sprintf_buffer(e, "%s 0x%08x\n", options->m_key, *(u32 *)((char *)cnfdata + options->m_offset));
				lbuf = 0;
				break;
			case 'D':
				offsptr3 = *(u32 *)((char *)cnfdata + options->m_offset);
				switch ( offsptr3 )
				{
					case 0xFFFFFFFF:
						lbuf = 0;
						break;
					case 0:
						lbuf = "tone";
						break;
					case 1:
						lbuf = "pulse";
						break;
					case 2:
						lbuf = "any";
						break;
					default:
						break;
				}
				break;
			case 'L':
				offsptr4 = *(u32 *)((char *)cnfdata + options->m_offset);
				if ( offsptr4 == 0xFFFFFFFF )
				{
					lbuf = 0;
					break;
				}
				result = do_netcnf_sprintf_buffer(e, "%s", options->m_key);
				if ( result < 0 )
					return result;
				for ( i = 0; i < 32; i += 1 )
				{
					lbuf = 0;
					switch ( ((u32)1 << i) & offsptr4 )
					{
						case 1u:
							lbuf = "phase";
							break;
						case 2u:
							lbuf = "cp";
							break;
						case 4u:
							lbuf = "auth";
							break;
						case 8u:
							lbuf = "chat";
							break;
						case 0x10u:
							lbuf = "private";
							break;
						case 0x20u:
							lbuf = "dll";
							break;
						case 0x40u:
							lbuf = "dump";
							break;
						case 0x10000:
							lbuf = "timer";
							break;
						case 0x20000:
							lbuf = "event";
							break;
						default:
							break;
					}
					if ( lbuf )
					{
						offsptr4 &= ~((u32)1 << i);
						result = do_netcnf_sprintf_buffer(e, " %s", lbuf);
						if ( result < 0 )
							return result;
					}
				}
				if ( offsptr4 )
				{
					result = do_netcnf_sprintf_buffer(e, " 0x%x", offsptr4);
					if ( result < 0 )
						return result;
				}
				result = do_netcnf_sprintf_buffer(e, "\n");
				lbuf = 0;
				break;
			case 'M':
				if ( !strcmp("want.mru", options->m_key) ? !*((u8 *)cnfdata + 169) : !*((u8 *)cnfdata + 245) )
				{
					lbuf = 0;
					break;
				}
				result = do_netcnf_sprintf_buffer(e, "%s %d\n", options->m_key, *(u16 *)((char *)cnfdata + options->m_offset));
				lbuf = 0;
				break;
			case 'P':
				offsptr3 = *(u32 *)((char *)cnfdata + options->m_offset);
				switch ( offsptr3 )
				{
					case 0xFFFFFFFF:
						lbuf = 0;
						break;
					case 1:
						lbuf = "auto";
						break;
					case 2:
						lbuf = "10";
						break;
					case 3:
						lbuf = "10_fd";
						break;
					case 4:
						lbuf = "10_fd_pause";
						break;
					case 5:
						lbuf = "tx";
						break;
					case 6:
						lbuf = "tx_fd";
						break;
					case 7:
						lbuf = "tx_fd_pause";
						break;
					default:
						break;
				}
				break;
			case 'T':
				offsptr3 = *(u32 *)((char *)cnfdata + options->m_offset);
				switch ( offsptr3 )
				{
					case 0xFFFFFFFF:
						lbuf = 0;
						break;
					case 0:
						lbuf = "any";
						break;
					case 1:
						lbuf = "eth";
						break;
					case 2:
						lbuf = "ppp";
						break;
					case 3:
						lbuf = "nic";
						break;
					default:
						break;
				}
				break;
			case 'b':
				if ( *((u8 *)cnfdata + options->m_offset) == 255 )
				{
					lbuf = 0;
					break;
				}
				result = do_netcnf_sprintf_buffer(e, "%s%s\n", *((u8 *)cnfdata + options->m_offset) ? "" : "-", options->m_key);
				lbuf = 0;
				break;
			case 'c':
				offsptr3 = (u32)(s32) * ((char *)cnfdata + options->m_offset);
				switch ( offsptr3 )
				{
					case 0xFFFFFFFF:
						lbuf = 0;
						break;
					case 0:
						lbuf = "no";
						break;
					case 5:
						lbuf = "md5";
						break;
					case 128:
						lbuf = "ms-v1";
						break;
					case 129:
						lbuf = "ms-v2";
						break;
					default:
						break;
				}
				break;
			case 'p':
				offsptr6 = *(u32 *)((char *)cnfdata + options->m_offset);
				if ( !offsptr6 )
				{
					lbuf = 0;
					break;
				}
				result = do_netcnf_sprintf_buffer(e, "%s \"%S\"\n", options->m_key, offsptr6);
				lbuf = 0;
				break;
			default:
				return printf("netcnf: internal save error (%d, type=%c)\n", 302, options->m_type);
		}
		if ( lbuf )
		{
			if ( (const char *)e->lbuf == (const char *)lbuf )
			{
				sprintf((char *)e->lbuf, "0x%x", offsptr3);
			}
			result = do_netcnf_sprintf_buffer(e, "%s %s\n", options->m_key, lbuf);
		}
		if ( result < 0 )
			return result;
	}
	return 0;
}

static int do_netcnf_net_write(sceNetCnfEnv_t *e, struct sceNetCnfInterface *ifc)
{
	struct sceNetCnfCommand *cmd_head;
	int result;

	for ( cmd_head = ifc->cmd_head; cmd_head; cmd_head = cmd_head->forw )
	{
		int nameserverflag;

		nameserverflag = -1;
		switch ( cmd_head->code )
		{
			case 1:
				nameserverflag = 1;
				break;
			case 2:
				nameserverflag = 0;
				break;
			case 3:
			{
				result = do_netcnf_sprintf_buffer(e, "route add -%s", (((route_t *)cmd_head)->re.flags & 2) ? "host" : "net");
				if ( result < 0 )
					return result;
				if ( sceNetCnfAddress2String((char *)e->lbuf, sizeof(e->lbuf), &((route_t *)cmd_head)->re.dstaddr) )
					return -1;
				result = do_netcnf_sprintf_buffer(e, " %s", (const char *)e->lbuf);
				if ( result < 0 )
					return result;
				if ( (((route_t *)cmd_head)->re.flags & 4) )
				{
					if ( sceNetCnfAddress2String((char *)e->lbuf, sizeof(e->lbuf), &((route_t *)cmd_head)->re.gateway) )
						return -1;
					result = do_netcnf_sprintf_buffer(e, " gw %s", (const char *)e->lbuf);
					if ( result < 0 )
						return result;
				}
				if ( sceNetCnfAddress2String((char *)e->lbuf, sizeof(e->lbuf), &((route_t *)cmd_head)->re.genmask) )
					return -1;
				result = do_netcnf_sprintf_buffer(e, " netmask %s", (const char *)e->lbuf);
				if ( result < 0 )
					return result;
				result = do_netcnf_sprintf_buffer(e, "\n");
				if ( result < 0 )
					return result;
				break;
			}
			case 4:
			{
				if ( sceNetCnfAddress2String((char *)e->lbuf, sizeof(e->lbuf), &((route_t *)cmd_head)->re.dstaddr) )
					return -1;
				result = do_netcnf_sprintf_buffer(e, "route del %s\n", (const char *)e->lbuf);
				if ( result < 0 )
					return result;
				break;
			}
			default:
				return -1;
		}
		if ( nameserverflag != -1 )
		{
			if ( sceNetCnfAddress2String((char *)e->lbuf, sizeof(e->lbuf), &((nameserver_t *)cmd_head)->address) )
				return -1;
			result = do_netcnf_sprintf_buffer(e, "nameserver %s %s\n", nameserverflag ? "add" : "del", (const char *)e->lbuf);
			if ( result < 0 )
				return result;
		}
	}
	return 0;
}

static int do_netcnf_phone_write(sceNetCnfEnv_t *e, struct sceNetCnfInterface *ifc)
{
	int i;
	int result;

	for ( i = 0; i < (int)(sizeof(ifc->phone_numbers) / sizeof(ifc->phone_numbers[0])); i += 1 )
	{
		if ( ifc->phone_numbers[i] )
		{
			result = do_netcnf_sprintf_buffer(e, "phone_number%d \"%S\"\n", i, ifc->phone_numbers[i]);
			if ( result < 0 )
				return result;
		}
	}
	return 0;
}

static int do_netcnf_unknown_write(sceNetCnfEnv_t *e, struct sceNetCnfUnknownList *unknown_list)
{
	struct sceNetCnfUnknown *head;

	for ( head = unknown_list->head; head; head = head->forw )
	{
		int result;

		result = do_netcnf_sprintf_buffer(e, "%s\n", (const char *)&head[1]);
		if ( result < 0 )
			return result;
	}
	return 0;
}

static int do_write_netcnf(sceNetCnfEnv_t *e, const char *path, int is_attach_cnf)
{
	int memsize;
	const char *fullpath;

	memsize = (int)((char *)e->mem_ptr - (char *)e->mem_base);
	if ( e->f_verbose )
		printf("netcnf: dir=%s path=%s\n", e->dir_name ? e->dir_name : "NULL", path ? path : "NULL");
	fullpath = do_handle_netcnf_prerw(e, path);
	if ( !fullpath )
		return -1;
	if ( e->f_verbose )
	{
		printf("netcnf: writing \"%s\" as ", fullpath);
		if ( is_attach_cnf )
			printf("ATTACH_CNF");
		else
			printf("NET_CNF");
		printf("\n");
	}
	if ( !is_attach_cnf || e->f_no_decode )
	{
		int fd;
		int writeres;

		fd = do_open_netcnf(fullpath, 1538, 511);
		if ( fd < 0 )
		{
			e->file_err += 1;
			return (fd == -EIO) ? -18 : -3;
		}
		writeres = do_write_netcnf_no_encode(fd, e->mem_base, memsize);
		if ( memsize != writeres )
		{
			e->file_err += 1;
			do_close_netcnf(fd);
			return (writeres == -EIO) ? -18 : -5;
		}
		do_close_netcnf(fd);
	}
	else
	{
		if ( do_write_netcnf_encode(fullpath, e->mem_base, memsize) < 0 )
		{
			e->file_err += 1;
			return -1;
		}
	}
	return 0;
}

static int do_export_netcnf_inner(sceNetCnfEnv_t *e, const char *arg_fname, struct sceNetCnfInterface *ifc)
{
	void *memalign;
	int result;
	struct sceNetCnfPair *pair_head;

	memalign = (void *)(((uiptr)e->mem_base + 3) & (uiptr)~3);
	e->mem_base = memalign;
	e->mem_ptr = memalign;
	result = do_netcnf_sprintf_buffer(e, "%s\n\n", "# <Sony Computer Entertainment Inc.>");
	if ( result < 0 )
		return result;
	if ( ifc )
	{
		result = do_netcnf_other_write(e, g_options_attach_cnf, ifc);
		if ( result < 0 )
			return result;
		result = do_netcnf_phone_write(e, ifc);
		if ( result < 0 )
			return result;
		result = do_netcnf_net_write(e, ifc);
		if ( result < 0 )
			return result;
		result = do_netcnf_unknown_write(e, &ifc->unknown_list);
		if ( result < 0 )
			return result;
		return do_write_netcnf(e, arg_fname, 1);
	}
	for ( pair_head = e->root->pair_head; pair_head; pair_head = pair_head->forw )
	{
		result = do_netcnf_sprintf_buffer(
			e, "interface \"%S\" \"%S\" \"%S\"\n", pair_head->display_name, pair_head->attach_ifc, pair_head->attach_dev);
		if ( result < 0 )
			return result;
	}
	result = do_netcnf_other_write(e, g_options_net_cnf, e->root);
	if ( result < 0 )
		return result;
	result = do_netcnf_unknown_write(e, &e->root->unknown_list);
	if ( result < 0 )
		return result;
	return do_write_netcnf(e, arg_fname, 0);
}

static int do_export_netcnf(sceNetCnfEnv_t *e)
{
	return ((e->req != 1 && e->req != 2) || do_export_netcnf_inner(e, e->arg_fname, (e->req == 1) ? 0 : e->ifc)) ? -1 : 0;
}

static char *do_address_to_string_inner_element(char *dst, int srcbyte)
{
	char *tmpstk_ptr;
	char tmpstk[16];

	tmpstk_ptr = tmpstk;
	if ( srcbyte < 0 )
	{
		*dst = '-';
		dst += 1;
		srcbyte = -srcbyte;
	}
	for ( ; srcbyte > 0; srcbyte /= 10 )
	{
		*tmpstk_ptr = srcbyte % 10 + '0';
		tmpstk_ptr += 1;
	}
	for ( ; tmpstk < tmpstk_ptr; tmpstk_ptr -= 1 )
	{
		*dst = tmpstk_ptr[-1];
		dst += 1;
	}
	return dst;
}

static void do_address_to_string_inner(char *dst, unsigned int srcint)
{
	char *elm1;

	elm1 = do_address_to_string_inner_element(dst, (srcint >> 24) & 0xFF);
	*elm1 = '.';
	elm1 = do_address_to_string_inner_element(elm1 + 1, (srcint >> 16) & 0xFF);
	*elm1 = '.';
	elm1 = do_address_to_string_inner_element(elm1 + 1, (srcint >> 8) & 0xFF);
	*elm1 = '.';
	elm1 = do_address_to_string_inner_element(elm1 + 1, srcint & 0xFF);
	*elm1 = 0;
}

static int do_name_2_address_inner(unsigned int *dst, const char *buf)
{
	int prefixchkn;
	unsigned int i;
	int offsbase1;
	unsigned int tmpstk1[4];

	i = 0;
	for ( prefixchkn = 0; prefixchkn < (int)(sizeof(tmpstk1) / sizeof(tmpstk1[0])); prefixchkn += 1 )
	{
		unsigned int base;

		base = 10;
		if ( *buf == '0' )
		{
			base = 8;
			buf += 1;
			if ( *buf == 'x' || *buf == 'X' )
			{
				buf += 1;
				base = 16;
			}
		}
		for ( i = 0; isxdigit(*buf); i = i * base + (unsigned int)offsbase1 )
		{
			offsbase1 = *buf - '0';
			if ( !isdigit(*buf) )
			{
				offsbase1 = *buf - '7';
				if ( !isupper(*buf) )
					offsbase1 = *buf - 'W';
			}
			if ( offsbase1 >= (int)base )
				break;
			buf += 1;
		}
		if ( prefixchkn > 0 && (unsigned int)tmpstk1[prefixchkn - 1] >= 0x100 )
			return 0;
		tmpstk1[prefixchkn] = i;
		if ( *buf != '.' )
			break;
		buf += 1;
	}
	if ( *buf && *buf != ' ' )
		return 0;
	switch ( prefixchkn )
	{
		case 0:
			break;
		case 1:
			if ( (i >> 24) )
				return 0;
			i |= tmpstk1[0] << 24;
			break;
		case 2:
			if ( (i >> 16) )
				return 0;
			i |= (tmpstk1[0] << 24) | (tmpstk1[1] << 16);
			break;
		case 3:
			if ( (i >> 8) )
				return 0;
			i |= (tmpstk1[0] << 24) | (tmpstk1[1] << 16) | (tmpstk1[2] << 8);
			break;
		default:
			return 0;
	}
	*dst = i;
	return 1;
}

static int do_conv_a2s_inner(char *sp_, char *dp_, int len)
{
	int len_minus_three;
	int curindx1;
	char *dp_ptroffs1;
	char *dp_ptroffs2;
	char *dp_ptroffs3;

	len_minus_three = len - 3;
	curindx1 = 0;
	if ( len_minus_three <= 0 )
		return -19;
	*dp_ = '"';
	dp_ptroffs1 = dp_ + 1;
	*dp_ptroffs1 = '"';
	dp_ptroffs1 += 1;
	*dp_ptroffs1 = ' ';
	dp_ptroffs2 = dp_ptroffs1 + 1;
	while ( 1 )
	{
		for ( ; *sp_ == ' ' || *sp_ == '\t'; sp_ += 1 )
			;
		if ( !*sp_ )
			break;
		if ( (*sp_ != 'A' && *sp_ != 'a') || (sp_[1] != 'T' && sp_[1] != 't') )
			return 0;
		for ( ; *sp_ && *sp_ != ' ' && *sp_ != '\t'; sp_ += 1 )
		{
			len_minus_three -= 1;
			if ( len_minus_three <= 0 )
				return -19;
			if ( *sp_ == '-' || *sp_ == '\\' || *sp_ == '"' || *sp_ == '^' )
			{
				len_minus_three -= 1;
				if ( len_minus_three <= 0 )
					return -19;
				*dp_ptroffs2 = '\\';
				dp_ptroffs2 += 1;
			}
			*dp_ptroffs2 = *sp_;
			dp_ptroffs2 += 1;
		}
		len_minus_three -= 4;
		if ( len_minus_three <= 0 )
			return -19;
		*dp_ptroffs2 = ' ';
		dp_ptroffs3 = dp_ptroffs2 + 1;
		*dp_ptroffs3 = 'O';
		dp_ptroffs3 += 1;
		*dp_ptroffs3 = 'K';
		dp_ptroffs3 += 1;
		*dp_ptroffs3 = ' ';
		dp_ptroffs2 = dp_ptroffs3 + 1;
		curindx1 += 1;
	}
	if ( curindx1 <= 0 )
		return 0;
	if ( (len_minus_three - 2) <= 0 )
		return -19;
	strcpy(dp_ptroffs2, "\\c");
	return 1;
}

static int do_conv_s2a_inner(char *sp_, char *dp_, int len)
{
	int curindx1;
	char *sp_minus_one1;
	char *sp_ptroffs1;
	char *sp_ptroffs2;
	char *sp_ptroffs3;
	char *sp_ptroffs4;

	curindx1 = 0;
	for ( ; *sp_ == ' ' || *sp_ == '\t'; sp_ += 1 )
		;
	sp_minus_one1 = sp_;
	if ( *sp_minus_one1 != '"' || sp_minus_one1[1] != '"' || (sp_minus_one1[2] != ' ' && sp_minus_one1[2] != '\t') )
		return 0;
	sp_ptroffs1 = sp_minus_one1 + 3;
	while ( 1 )
	{
		for ( ; *sp_ptroffs1 == ' ' || *sp_ptroffs1 == '\t'; sp_ptroffs1 += 1 )
			;
		sp_ptroffs2 = sp_ptroffs1;
		if ( !*sp_ptroffs2 || *sp_ptroffs2 == '\\' )
		{
			if ( *sp_ptroffs2 != '\\' || sp_ptroffs2[1] != 'c' )
				return 0;
			sp_ptroffs4 = sp_ptroffs2 + 2;
			for ( ; *sp_ptroffs4 == ' ' || *sp_ptroffs4 == '\t'; sp_ptroffs4 += 1 )
				;
			if ( *sp_ptroffs4 )
				return -19;
			if ( curindx1 <= 0 )
				return 0;
			if ( (int)(len - 1) < 0 )
				return -19;
			*dp_ = 0;
			return 1;
		}
		if ( (*sp_ptroffs2 != 'A' && *sp_ptroffs2 != 'a') || (sp_ptroffs2[1] != 'T' && sp_ptroffs2[1] != 't') )
			return 0;
		if ( curindx1 > 0 )
		{
			len -= 1;
			if ( len <= 0 )
				return -19;
			*dp_ = ' ';
			dp_ += 1;
		}
		sp_ptroffs2 += 1;
		if ( (sp_ptroffs2[-1]) != ' ' )
		{
			sp_ptroffs2 -= 1;
			while ( *sp_ptroffs2 != '\t' )
			{
				len -= 1;
				if ( len <= 0 )
					return -19;
				if ( *sp_ptroffs2 == '\\' )
				{
					if ( sp_ptroffs2[1] != '-' && sp_ptroffs2[1] != '\\' && sp_ptroffs2[1] != '"' && sp_ptroffs2[1] != '^' )
						return 0;
					sp_ptroffs2 += 1;
				}
				*dp_ = *sp_ptroffs2;
				dp_ += 1;
				sp_ptroffs2 += 1;
				if ( !*sp_ptroffs2 || *sp_ptroffs2 == ' ' )
					break;
			}
		}
		for ( ; *sp_ptroffs2 == ' ' || *sp_ptroffs2 == '\t'; sp_ptroffs2 += 1 )
			;
		sp_ptroffs3 = sp_ptroffs2;
		if ( *sp_ptroffs3 != 'O' || sp_ptroffs3[1] != 'K' || (sp_ptroffs3[2] != ' ' && sp_ptroffs3[2] != '\t') )
			return 0;
		sp_ptroffs1 = sp_ptroffs3 + 3;
		curindx1 += 1;
	}
}

static int do_check_aolnet(const char *auth_name)
{
	int i;
	const char *periodpos;

	if ( strncmp(auth_name, "aolnet/", 7) )
		return 0;
	periodpos = auth_name;
	for ( i = 0; periodpos; i += 1 )
	{
		periodpos = strchr(periodpos, '.');
		if ( periodpos )
			periodpos += 1;
	}
	return (i != 5) ? 0 : -20;
}

static int do_check_authnet(char *argst, char *arged)
{
	char *i;
	char *j;

	for ( i = arged; argst < i && i[-1] < '!'; i -= 1 )
		;
	*i = 0;
	for ( j = argst; *j && isspace(*j); j += 1 )
		;
	if ( !strncmp(j, "auth_name", 9) )
	{
		int result;

		for ( ; *j && !isspace(*j); j += 1 )
			;
		for ( ; *j && isspace(*j); j += 1 )
			;
		if ( *j == '"' )
			j += 1;
		result = do_check_aolnet(j);
		if ( result < 0 )
			return result;
	}
	return 0;
}

static int do_read_check_netcnf(const char *netcnf_path, int type, int no_check_magic, int no_decode)
{
	int read_res2;
	char *heapmem;
	int errretres;
	char *curheapptr1;
	char *heapmem_2;

	switch ( type )
	{
		case 0:
		case 2:
			return 0;
		default:
			return -10;
		case 1:
			break;
	}
	read_res2 = no_decode ? do_read_netcnf_no_decode(netcnf_path, &g_read_check_netcnf_heapptr) :
													do_read_netcnf_decode(netcnf_path, &g_read_check_netcnf_heapptr);
	if ( read_res2 < 0 )
		return read_res2;
	heapmem = (char *)do_alloc_heapmem(1024);
	errretres = 0;
	if ( !heapmem )
	{
		do_free_heapmem(g_read_check_netcnf_heapptr);
		return -2;
	}
	curheapptr1 = g_read_check_netcnf_heapptr;
	heapmem_2 = heapmem;
	if (
		no_check_magic
		&& (read_res2 < 36 || strncmp(g_read_check_netcnf_heapptr, "# <Sony Computer Entertainment Inc.>", 36)) )
	{
		int i;

		printf("netcnf: decoding error (magic=\"");
		for ( i = 0; i < read_res2 && i < 36; i += 1 )
		{
			printf("%c", (unsigned int)((u8)curheapptr1[i] - 32) >= 0x5F ? '?' : (char)(u8)curheapptr1[i]);
		}
		errretres = -15;
		printf("\")\n");
	}
	if ( !errretres && read_res2 > 0 )
	{
		read_res2 -= 1;
		for ( ; read_res2 > 0; read_res2 -= 1 )
		{
			if ( *curheapptr1 == '\n' )
			{
				if ( heapmem < heapmem_2 && heapmem_2[-1] == '\\' )
				{
					heapmem_2 -= 1;
				}
				else
				{
					*heapmem_2 = 0;
					errretres = do_check_authnet(heapmem, heapmem_2);
					heapmem_2 = heapmem;
					if ( errretres < 0 )
						break;
				}
			}
			else
			{
				if ( heapmem_2 < &heapmem[1023] && *curheapptr1 != '\r' )
				{
					*heapmem_2 = *curheapptr1;
					heapmem_2 += 1;
				}
			}
			curheapptr1 += 1;
		}
	}
	if ( !errretres && heapmem < heapmem_2 )
		errretres = do_check_authnet(heapmem, heapmem_2);
	do_free_heapmem(g_read_check_netcnf_heapptr);
	do_free_heapmem(heapmem);
	return errretres;
}

static int do_check_provider_inner(const sceNetCnfEnv_t *e, int type)
{
	switch ( type )
	{
		case 0:
		case 2:
			return 0;
		case 1:
		{
			if ( !e || !e->ifc )
				return -14;
			if ( e->ifc->auth_name )
			{
				int result;

				result = do_check_aolnet((const char *)e->ifc->auth_name);
				if ( result < 0 )
					return result;
			}
			return 0;
		}
		default:
			return -10;
	}
}

static const char *do_handle_netcnf_dirname(const char *fpath, const char *entry_buffer, char *netcnf_file_path)
{
	const char *entry_buffer_1;
	const char *fpath_1;
	const char *fpath_1_minus_1;
	const char *fpath_2;
	char *i;
	const char *entry_buffer_2;

	if ( !entry_buffer || !*entry_buffer )
		return 0;
	for ( entry_buffer_1 = entry_buffer; *entry_buffer_1; entry_buffer_1 += 1 )
	{
		if ( *entry_buffer_1 == ':' )
			return entry_buffer;
	}
	if ( !fpath || !*fpath )
		return entry_buffer;
	for ( fpath_1 = fpath; fpath_1[1]; fpath_1 += 1 )
		;
	fpath_1_minus_1 = fpath_1 - 1;
	if ( fpath < fpath_1_minus_1 || *entry_buffer == '/' || *entry_buffer == '\\' )
	{
		for ( ; fpath < fpath_1_minus_1 && *fpath_1_minus_1 != ':'; fpath_1_minus_1 -= 1 )
			;
		if ( fpath <= fpath_1_minus_1 && (*fpath_1_minus_1 == ':' || *fpath_1_minus_1 == '/' || *fpath_1_minus_1 == '\\') )
		{
			fpath_1_minus_1 += 1;
		}
	}
	else if ( fpath <= fpath_1_minus_1 && *fpath_1_minus_1 != ':' )
	{
		for ( ; fpath < fpath_1_minus_1 && *fpath_1_minus_1 != ':' && *fpath_1_minus_1 != '/' && *fpath_1_minus_1 != '\\';
					fpath_1_minus_1 -= 1 )
			;
		if ( *fpath_1_minus_1 == ':' || *fpath_1_minus_1 == '/' || *fpath_1_minus_1 == '\\' )
		{
			fpath_1_minus_1 += 1;
		}
	}
	fpath_2 = fpath;
	for ( i = netcnf_file_path; fpath_2 < fpath_1_minus_1; i += 1 )
	{
		*i = *fpath_2;
		fpath_2 += 1;
	}
	for ( entry_buffer_2 = entry_buffer; *entry_buffer_2; entry_buffer_2 += 1 )
	{
		*i = *entry_buffer_2;
		i += 1;
	}
	*i = 0;
	return netcnf_file_path;
}

static int do_get_filesize_inner(int fd)
{
	int lseek_end_res;
	int lseek_start_res;

	lseek_end_res = lseek(fd, 0, 2);
	if ( lseek_end_res < 0 )
		return (lseek_end_res != -EIO) ? -6 : -18;
	lseek_start_res = lseek(fd, 0, 0);
	return (lseek_start_res < 0) ? ((lseek_start_res != -EIO) ? -6 : -18) : lseek_end_res;
}

static int is_special_file_path(const char *netcnf_path)
{
	switch ( g_callbacks.type )
	{
		case 1:
			return !strncmp(netcnf_path, "mc", 2) ? 1 : 0;
		case 2:
			return !strncmp(netcnf_path, "ext", 3) ? 1 : 0;
		default:
			return 1;
	}
}

static void do_init_callback_handles(void)
{
	int i;

	for ( i = 0; i < (int)(sizeof(g_callback_handle_infos) / sizeof(g_callback_handle_infos[0])); i += 1 )
	{
		g_callback_handle_infos[i].m_fd = -1;
		g_callback_handle_infos[i].m_filesize = 0;
		g_callback_handle_infos[i].m_allocstate = 0;
	}
}

static int do_get_empty_callback_handle(int in_fd, int in_allocstate)
{
	int i;

	for ( i = 0; i < (int)(sizeof(g_callback_handle_infos) / sizeof(g_callback_handle_infos[0])); i += 1 )
	{
		if ( g_callback_handle_infos[i].m_fd == -1 )
		{
			g_callback_handle_infos[i].m_fd = in_fd;
			g_callback_handle_infos[i].m_allocstate = in_allocstate;
			g_open_callback_handle_count += 1;
			return i;
		}
	}
	return -1;
}

static int do_filesize_callback_handles(int in_fd, int in_allocstate)
{
	int i;

	for ( i = 0; i < (int)(sizeof(g_callback_handle_infos) / sizeof(g_callback_handle_infos[0])); i += 1 )
	{
		if (
			g_callback_handle_infos[i].m_fd == in_fd
			&& (g_callback_handle_infos[i].m_allocstate == in_allocstate || !in_allocstate) )
		{
			return i;
		}
	}
	return -1;
}

static void do_clear_callback_handles(int fd, int allocmatch)
{
	int i;

	for ( i = 0; i < (int)(sizeof(g_callback_handle_infos) / sizeof(g_callback_handle_infos[0])); i += 1 )
	{
		if ( g_callback_handle_infos[i].m_fd == fd && g_callback_handle_infos[i].m_allocstate == allocmatch )
		{
			g_callback_handle_infos[i].m_fd = -1;
			g_callback_handle_infos[i].m_allocstate = 0;
			g_open_callback_handle_count -= 1;
			break;
		}
	}
}

static const char *do_colon_callback_handles(const char *netcnf_path, char *device)
{
	char *index_res;
	u32 devnameend;

	index_res = index(netcnf_path, ':');
	if ( !index_res )
		return 0;
	devnameend = (u32)(int)(index_res - netcnf_path) + 1;
	if ( devnameend >= 17 )
		return 0;
	memcpy(device, netcnf_path, (u32)devnameend);
	device[devnameend] = 0;
	return (strlen(index_res + 1) + 1 < 257) ? (index_res + 1) : 0;
}

static int do_open_netcnf(const char *netcnf_path, int file_flags, int file_mode)
{
	const char *cbind;
	int openret1;
	int empty_callback_handle;
	char pathconcat[16];
	int filesz1;

	if ( !g_callbacks.open || !is_special_file_path(netcnf_path) )
		return open(netcnf_path, file_flags, file_mode);
	if ( g_open_callback_handle_count >= (int)(sizeof(g_callback_handle_infos) / sizeof(g_callback_handle_infos[0])) )
		return -EPERM;
	cbind = do_colon_callback_handles(netcnf_path, pathconcat);
	if ( !cbind )
		return -EPERM;
	openret1 = g_callbacks.open(pathconcat, cbind, file_flags, file_mode, &filesz1);
	if ( openret1 < 0 )
		return openret1;
	empty_callback_handle = do_get_empty_callback_handle(openret1, 1);
	strcpy(g_callback_handle_infos[empty_callback_handle].m_device, pathconcat);
	strcpy(g_callback_handle_infos[empty_callback_handle].m_pathname, cbind);
	g_callback_handle_infos[empty_callback_handle].m_buf = 0;
	g_callback_handle_infos[empty_callback_handle].m_bufpos = 0;
	g_callback_handle_infos[empty_callback_handle].m_filesize = filesz1;
	return openret1;
}

static int do_read_callback_handles(int handlefd, int fd, void *ptr, int size)
{
	struct netcnf_callback_handle_info *cbh;

	cbh = &g_callback_handle_infos[handlefd];
	if ( !cbh->m_bufpos )
	{
		cbh->m_buf = do_alloc_heapmem(cbh->m_filesize);
		if ( !cbh->m_buf )
			return -EPERM;
		if ( g_callbacks.read(fd, cbh->m_device, cbh->m_pathname, cbh->m_buf, 0, cbh->m_filesize) != cbh->m_filesize )
		{
			do_free_heapmem(cbh->m_buf);
			cbh->m_buf = 0;
			return -EPERM;
		}
	}
	memcpy(ptr, (char *)cbh->m_buf + cbh->m_bufpos, (u32)size);
	cbh->m_bufpos += size;
	return size;
}

static int do_readfile_netcnf(int fd, void *ptr, int size)
{
	int cbind;

	if ( !g_callbacks.read )
		return read(fd, ptr, size);
	cbind = do_filesize_callback_handles(fd, 1);
	return (cbind == -1) ? read(fd, ptr, size) : do_read_callback_handles(cbind, fd, ptr, size);
}

static int do_write_netcnf_no_encode(int fd, void *ptr, int size)
{
	if ( !g_callbacks_set || g_callbacks.type == 2 )
		return write(fd, ptr, size);
	printf("[err] netcnf write()\n");
	return -EPERM;
}

static int do_dopen_wrap(const char *fn)
{
	if ( !g_callbacks_set || g_callbacks.type == 2 )
	{
#ifdef _IOP
		return dopen(fn);
#else
		(void)fn;
		return -EPERM;
#endif
	}
	printf("[err] netcnf dopen()\n");
	return -EPERM;
}

static int do_dread_wrap(int fn, iox_dirent_t *buf)
{
	if ( !g_callbacks_set || g_callbacks.type == 2 )
	{
#ifdef _IOP
		return dread(fn, buf);
#else
		(void)fn;
		(void)buf;
		return -EPERM;
#endif
	}
	printf("[err] netcnf dread()\n");
	return -EPERM;
}

static int do_remove_wrap(const char *fn)
{
	if ( !g_callbacks_set || g_callbacks.type == 2 )
		return remove(fn);
	printf("[err] netcnf remove()\n");
	return -EPERM;
}

static void do_close_netcnf(int fd)
{
	int cbind;

	if ( !g_callbacks.close )
	{
		close(fd);
		return;
	}
	cbind = do_filesize_callback_handles(fd, 1);
	if ( cbind == -1 )
	{
		close(fd);
		return;
	}
	g_callbacks.close(fd);
	do_free_heapmem(g_callback_handle_infos[cbind].m_buf);
	g_callback_handle_infos[cbind].m_buf = 0;
	do_clear_callback_handles(fd, 1);
}

static void do_dclose_wrap(int fd)
{
	if ( !g_callbacks_set || g_callbacks.type == 2 )
	{
#ifdef _IOP
		dclose(fd);
#else
		(void)fd;
#endif
		return;
	}
	printf("[err] netcnf dclose()\n");
}

static int do_filesize_netcnf(int fd)
{
	int cbind;

	cbind = do_filesize_callback_handles(fd, 0);
	return (cbind == -1) ? do_get_filesize_inner(fd) : g_callback_handle_infos[cbind].m_filesize;
}

static void do_getstat_wrap(const char *fn, iox_stat_t *stx)
{
	if ( !g_callbacks_set || g_callbacks.type == 2 )
	{
#ifdef _IOP
		getstat(fn, stx);
#else
		{
			struct stat st;

			stat(fn, &st);
			stx->size = (unsigned int)(int)st.st_size;
		}
#endif
		return;
	}
	printf("[err] netcnf getstat()\n");
}

static void do_chstat_mode_copyprotect_wrap(const char *fn)
{
#ifdef _IOP
	iox_stat_t statmode;
#endif

	if ( !g_callbacks_set || g_callbacks.type == 2 )
	{
#ifdef _IOP
		do_getstat_wrap(fn, &statmode);
		statmode.mode |= 8u;
		chstat(fn, &statmode, 1u);
#else
		(void)fn;
#endif
		return;
	}
	printf("[err] netcnf chstat()\n");
}

static void do_set_callback_inner(sceNetCnfCallback_t *pcallback)
{
	if ( pcallback )
	{
		g_callbacks.type = pcallback->type;
		g_callbacks.open = pcallback->open;
		g_callbacks.read = pcallback->read;
		g_callbacks.close = pcallback->close;
		g_callbacks_set = 1;
	}
	else
	{
		memset(&g_callbacks, 0, sizeof(g_callbacks));
		g_callbacks_set = 0;
	}
	do_init_callback_handles();
}

#ifdef _IOP
static int do_init_heap(void)
{
	if ( g_netcnf_heap )
		return -2;
	g_netcnf_heap = CreateHeap(1024, 1);
	return g_netcnf_heap ? 0 : -1;
}
#endif

static void *do_alloc_heapmem(int nbytes)
{
#ifdef _IOP
	return AllocHeapMemory(g_netcnf_heap, nbytes);
#else
	return malloc((size_t)nbytes);
#endif
}

static void do_free_heapmem(void *ptr)
{
#ifdef _IOP
	if ( ptr )
		FreeHeapMemory(g_netcnf_heap, ptr);
#else
	if ( ptr )
		free(ptr);
#endif
}

#ifdef _IOP
static void do_delete_heap(void)
{
	DeleteHeap(g_netcnf_heap);
	g_netcnf_heap = 0;
}
#endif
