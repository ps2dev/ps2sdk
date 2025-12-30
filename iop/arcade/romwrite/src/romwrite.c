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
#include <s147_mmio_hwport.h>
#include <sys/fcntl.h>

IRX_ID("ROMWRITE", 7, 1);
// Text section hash:
// 6674c59e79963dd4c1d7f6fccfecbf70

typedef struct nand_id_desc_info_
{
	u32 m_id[5];
	const char *m_nand_name;
	const char *m_nand_desc;
	int m_page_size_noecc;
	int m_page_size_withecc;
	int m_pages_per_block;
	int m_block_size;
} nand_id_desc_info_t;

typedef union romwrite_part_buf_
{
	u8 m_buf[0x20000];
	s147nand_header_t m_hdr;
	s147nand_dir_t m_dir;
	s147nand_direntry_t m_direntry[64];
} romwrite_part_buf_t;

static void thread_proc(void *userdata);
static void do_toggle_dev9addr_inner(int len, int cnt);
static void set_boot_video_mode(int flg);
static void do_set_flag(int flg);
static void do_set_secr_code(char code1, char code2);
static void do_handle_atfile_image(int part, const char *str);
static void do_set_atfile_147_dir(const char *str);
static int do_start_write_proc(void);
static int do_format_device(int abspart);
static int do_write_partition(int part);
static int check_badblock_count(void);
static int get_nand_partition_offset(int part, int abspart);
static int get_nand_partition_size(int part, int abspart);
static int get_nand_block_size_div_32_div_64(void);
static int get_nand_block_size_div_32(void);
static void do_dma_write_bytes_multi(void *ptr, int pageoffs, int pagecnt);
static int do_list_files(int part);
static void do_output_bb_info(int blocksd, int abspart, int bboffs);
static int do_verify(void *buf1, void *buf2, int len);
static const nand_id_desc_info_t *do_parse_device_info(const char *nandid);
// Unofficial: printf to IOP Kprintf instead of EE
#define USER_PRINTF(...) Kprintf(__VA_ARGS__)
// Unofficial: printf to EE is omitted
#define STATUS_PRINTF(...) Kprintf(__VA_ARGS__)

static const nand_id_desc_info_t g_nand_type_info[4] = {
	{
		{0xEC, 0xDA, 0xFFFFFFFF, 0x15, 0xFFFFFFFF},
		"SAMSUNG K9F2G08U0M",
		"8bit width, 256MBytes",
		0x800,
		0x840,
		0x40,
		0x800,
	},
	{
		{0xEC, 0xDC, 0x10, 0x95, 0x54},
		"SAMSUNG K9F4G08U0M",
		"8bit width, 512MBytes",
		0x800,
		0x840,
		0x40,
		0x1000,
	},
	{
		{0xEC, 0xD3, 0x51, 0x95, 0x58},
		"SAMSUNG K9K8G08U0M",
		"8bit width, 1GBytes",
		0x800,
		0x840,
		0x40,
		0x2000,
	},
	{{0x0, 0x0, 0x0, 0x0, 0x0}, NULL, NULL, 0, 0, 0, 0}};
// Unofficial: move to bss
static char g_secr_code[2];
// Unofficial: move to bss
static int g_curflag;
// Unofficial: move to bss
static int g_boot_video_mode;
// Unofficial: move to bss
static char *g_page_buf;
// Unofficial: move to bss
static int g_badblock_count;
static char g_product_code_tmp[32];
static u8 *g_blockinfo_str_buf;
static u16 *g_blockinfo_dat_buf;
static const nand_id_desc_info_t *g_device_info;
static char g_atfile_part_image[8][0x100];
static char g_atfile_info_image[0x100];
static char g_atfile_147_dir[0x100];
static romwrite_part_buf_t g_nand_partbuf __attribute__((__aligned__(16)));

static void do_format_nand_device(char devindchr)
{
	switch ( devindchr )
	{
		case '2':
			do_set_flag(0x10000);
			break;
		case '4':
			do_set_flag(0x20000);
			break;
		case '8':
			do_set_flag(0x30000);
			break;
		default:
			do_set_flag(0xF0000);
			devindchr = ' ';
			break;
	}
	STATUS_PRINTF(" -f%c: Format NAND(atfile:) device\n", devindchr);
}

static char *do_read_product_code(int fd)
{
	read(fd, g_product_code_tmp, sizeof(g_product_code_tmp));
	STATUS_PRINTF("  ---> OK, set product code - \"%s\"\n", &g_product_code_tmp[16]);
	// Unofficial: omit Blowfish hashing S147NBGI
	return &g_product_code_tmp[4];
}

int _start(int ac, char **av)
{
	iop_thread_t thparam;
	int thid;
	int i;
	char secrcode1;
	char secrcode2;
	int fd;
	const char *product_code;
	int image_file_idx;

	// Unofficial: omit SIF output command set to 30
	if ( ac < 2 )
	{
		USER_PRINTF("SYS147 ROM Writer (version 0x%04x)\n\n", 0x701);
		DelayThread(10000);
		USER_PRINTF("usage: %s [OPTION]... [FILE]...\n", "romwrite.irx");
		DelayThread(10000);
		USER_PRINTF("  -m, --main ............. MainPCB mode (Send PRINTF to EE)\n");
		DelayThread(10000);
		USER_PRINTF("  -s0 .................... Set default security code\n");
		DelayThread(10000);
		USER_PRINTF("  -sr .................... Read \"s147secr.147\" and set security code\n");
		DelayThread(10000);
		USER_PRINTF("  -f(f2, f4, f8) ......... Format NAND(atfile:) device\n");
		DelayThread(10000);
		USER_PRINTF("  -0([1..7]) filename .... Write \"atfile[0..7]:\" image file\n");
		DelayThread(10000);
		USER_PRINTF("  -d directory ........... Search \"atfile*.147\" in the directory and Write\n");
		DelayThread(10000);
		USER_PRINTF("  -i filename ............ Write \"atfile9:info\" image from file\n");
		DelayThread(10000);
		USER_PRINTF("  -l ..................... Read NAND device and Display all file list\n");
		DelayThread(10000);
		USER_PRINTF("  -v, --vga .............. Set boot video mode (VGA)\n");
		DelayThread(10000);
		return MODULE_NO_RESIDENT_END;
	}
	// Unofficial: init var here instead
	g_badblock_count = 1;
	USER_PRINTF("\n====== romwrite(version 0x%04x): Check argument ======\n", 0x701);
	DelayThread(10000);
	for ( i = 1; i < ac; i += 1 )
	{
		if ( !strcmp(av[i], "-m") || !strcmp(av[i], "--main") )
		{
			USER_PRINTF(" -m, --main : MainPCB mode (Send PRINTF to EE)\n");
			DelayThread(10000);
			// Unofficial: Flag setting omitted
			break;
		}
		if ( !strcmp(av[i], "-v") || !strcmp(av[i], "--vga") )
		{
			USER_PRINTF(" -v, --vga : Set boot video mode (VGA)\n");
			DelayThread(10000);
			set_boot_video_mode(2);
			break;
		}
	}
	for ( i = 1; i < ac; i += 1 )
	{
		if ( av[i][0] != '-' )
			continue;
		switch ( av[i][1] )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				image_file_idx = strtol(av[i] + 1, 0, 10);
				STATUS_PRINTF(" -%d : Write \"atfile%d:\" image file\n", image_file_idx, image_file_idx);
				do_set_flag(1 << image_file_idx);
				i += 1;
				do_handle_atfile_image(image_file_idx, av[i]);
				break;
			case 'd':
				STATUS_PRINTF(" -d : Search \"atfile*.147\" in the directory and Write\n");
				i += 1;
				do_set_flag(0x2000000);
				do_set_atfile_147_dir(av[i]);
				break;
			case 'f':
				do_format_nand_device(av[i][2]);
				break;
			case 'i':
				STATUS_PRINTF(" -i : Write \"atfile9:info\" image\n");
				do_set_flag(0x200);
				i += 1;
				do_handle_atfile_image(9, av[i]);
				break;
			case 'l':
				STATUS_PRINTF(" -l : Read NAND device and Display all file list\n");
				do_set_flag(0x4000000);
				break;
			case 's':
				switch ( av[i][2] )
				{
					case '0':
						STATUS_PRINTF(" -s0: Set default security code\n");
						do_set_flag(0x1000000);
						do_set_secr_code(0xFF, 0xFF);
						break;
					case 'r':
						STATUS_PRINTF(" -sr: Read \"s147secr.147\" file\n");
						do_set_flag(0x1000000);
						i += 1;
						fd = open(av[i], O_RDONLY);
						if ( fd < 0 )
						{
							STATUS_PRINTF("  ---> File not found, set default code\n");
							do_set_secr_code(0xFF, 0xFF);
							break;
						}
						product_code = do_read_product_code(fd);
						do_set_secr_code(product_code[0], product_code[1]);
						close(fd);
						break;
					default:
						STATUS_PRINTF(" -s : Set immediate secrity code\n");
						i += 1;
						secrcode1 = strtol(av[i], 0, 10);
						i += 1;
						secrcode2 = strtol(av[i], 0, 10);
						do_set_flag(0x1000000);
						do_set_secr_code(secrcode1, secrcode2);
						break;
				}
				break;
			default:
				break;
		}
	}
	STATUS_PRINTF("\n");
	thparam.attr = TH_C;
	thparam.thread = thread_proc;
	thparam.priority = 0x7A;
	thparam.stacksize = 0x80000;
	thparam.option = 0;
	thid = CreateThread(&thparam);
	if ( thid <= 0 )
		return MODULE_NO_RESIDENT_END;
	StartThread(thid, 0);
	return MODULE_RESIDENT_END;
}

static void thread_proc(void *userdata)
{
	int i;
	USE_S147_DEV9_MEM_MMIO();

	(void)userdata;
	if ( do_start_write_proc() )
	{
		STATUS_PRINTF("\n****** Aborted ******\n\n");
		for ( ;; )
		{
			s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
			s147_dev9_mem_mmio->m_led = 1;
			DelayThread(250000);
			s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
			s147_dev9_mem_mmio->m_led = 0;
			DelayThread(250000);
		}
	}
	STATUS_PRINTF("====== Completed ======\n\n");
	for ( ;; )
	{
		for ( i = 1; i < 20; i += 1 )
		{
			s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
			do_toggle_dev9addr_inner(5 * i, 50);
		}
		for ( i = 20; i > 0; i -= 1 )
		{
			s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
			do_toggle_dev9addr_inner(5 * i, 50);
		}
	}
}

static void do_toggle_dev9addr_inner(int len, int cnt)
{
	int i;
	USE_S147_DEV9_MEM_MMIO();

	for ( i = 0; i < cnt; i += 1 )
	{
		if ( len > 0 )
		{
			s147_dev9_mem_mmio->m_led = 3;
			DelayThread(10 * len);
		}
		if ( len < 100 )
		{
			s147_dev9_mem_mmio->m_led = 0;
			DelayThread(10 * (100 - len));
		}
	}
}

// Unused function omitted

static unsigned int generate_acio_delay_val(char dmat_val, char rddl_val, char wrdl_val)
{
	return (((dmat_val - 1) & 0xF) << 24) | (((rddl_val - 1) & 0xF) << 4) | 0xA01A0100 | ((wrdl_val - 1) & 0xF);
}

// Unused function omitted

// Send print to OSDSYS related omitted

static void set_boot_video_mode(int flg)
{
	g_boot_video_mode = flg;
}

static void do_set_flag(int flg)
{
	g_curflag |= flg;
}

static void do_set_secr_code(char code1, char code2)
{
	g_secr_code[0] = code1;
	g_secr_code[1] = code2;
}

static void do_handle_atfile_image(int part, const char *str)
{
	if ( part == 9 )
	{
		strcpy(g_atfile_info_image, str);
		return;
	}
	if ( part >= 0 && part < 8 && g_atfile_part_image[part] != str )
	{
		// Unofficial: check if pointer is different
		strcpy(g_atfile_part_image[part], str);
	}
}

static void do_set_atfile_147_dir(const char *str)
{
	strcpy(g_atfile_147_dir, str);
}

static int do_start_write_proc(void)
{
	s147nand_info_t *nandinf;
	int state;
	int part;
	int logaddrtable;
	u8 nandid[5];
	USE_S147_DEV9_MEM_MMIO();

	close(open("ctrl99:watchdog-stop", O_RDONLY));
	if ( (g_curflag & 0x1000000) != 0 )
	{
		STATUS_PRINTF("====== Set security code ======\n");
		s147_dev9_mem_mmio->m_security_unlock_set1 = g_secr_code[0];
		s147_dev9_mem_mmio->m_security_unlock_set2 = g_secr_code[1];
		STATUS_PRINTF("\n");
	}
	STATUS_PRINTF("====== Device information ======\n");
	s147nand_26_nand_readid(nandid);
	g_device_info = do_parse_device_info((char *)nandid);
	if ( !g_device_info )
	{
		STATUS_PRINTF(" ID = %02X/%02X/%02X/%02X/%02X\n", nandid[0], nandid[1], nandid[2], nandid[3], nandid[4]);
		STATUS_PRINTF("\nError: Unknown Device\n");
		return -1;
	}
	STATUS_PRINTF(" ID = %02X/%02X/%02X/%02X/%02X\n", nandid[0], nandid[1], nandid[2], nandid[3], nandid[4]);
	STATUS_PRINTF(" \"%s\", %s\n", g_device_info->m_nand_name, g_device_info->m_nand_desc);
	STATUS_PRINTF(
		" PageSize    = %d + %d (Bytes)\n",
		g_device_info->m_page_size_noecc,
		g_device_info->m_page_size_withecc - g_device_info->m_page_size_noecc);
	STATUS_PRINTF(" Pages/Block = %d (Pages)\n", g_device_info->m_pages_per_block);
	STATUS_PRINTF(" BlockSize   = %d (Blocks)\n", g_device_info->m_block_size);
	STATUS_PRINTF("\n");
	CpuSuspendIntr(&state);
	g_blockinfo_str_buf = (u8 *)AllocSysMemory(ALLOC_FIRST, g_device_info->m_block_size, 0);
	g_blockinfo_dat_buf = (u16 *)AllocSysMemory(ALLOC_FIRST, sizeof(u16) * g_device_info->m_block_size, 0);
	CpuResumeIntr(state);
	if ( !g_blockinfo_str_buf || !g_blockinfo_dat_buf )
	{
		STATUS_PRINTF("\nError: AllocSysMemory failed\n\n");
		return -1;
	}
	nandinf = s147nand_16_getnandinfo();
	CpuSuspendIntr(&state);
	nandinf->m_page_size_noecc = g_device_info->m_page_size_noecc;
	nandinf->m_page_size_withecc = g_device_info->m_page_size_withecc;
	nandinf->m_pages_per_block = g_device_info->m_pages_per_block;
	nandinf->m_block_size = g_device_info->m_block_size;
	nandinf->m_page_count = g_device_info->m_block_size * g_device_info->m_pages_per_block;
	CpuResumeIntr(state);
	logaddrtable = 0;
	switch ( g_curflag & 0xFF0000 )
	{
		case 0x10000:
			logaddrtable = do_format_device(2);
			break;
		case 0x20000:
			logaddrtable = do_format_device(4);
			break;
		case 0x30000:
			logaddrtable = do_format_device(8);
			break;
		case 0xF0000:
			logaddrtable = do_format_device(0);
			break;
		default:
			break;
	}
	if ( logaddrtable )
		return logaddrtable;
	s147nand_6_checkformat();
	close(open("atfile9:acdelay", O_RDONLY));
	if ( (g_curflag & 0x2000000) != 0 )
	{
		STATUS_PRINTF("====== Search directory ======\n");
		for ( part = 0; part < 8; part += 1 )
		{
			int fd;

			if ( s147nand_10_get_nand_partition_size(part) <= 0 )
			{
				STATUS_PRINTF(" atfile%d: Unformatted - Do nothing\n", part);
				continue;
			}
			sprintf(g_atfile_part_image[part], "%satfile%d.147", g_atfile_147_dir, part);
			fd = open(g_atfile_part_image[part], O_RDONLY);
			if ( fd < 0 )
			{
				DelayThread(10000);
				continue;
			}
			do_set_flag(1 << part);
			do_handle_atfile_image(part, g_atfile_part_image[part]);
			close(fd);
			STATUS_PRINTF(" \"%s\" is found\n", g_atfile_part_image[part]);
		}
		STATUS_PRINTF(" \n");
	}
	for ( part = 0; part < 8; part += 1 )
	{
		if ( ((1 << part) & g_curflag) == 0 )
			continue;
		STATUS_PRINTF("====== Write \"%s\" to atfile%d: ======\n", g_atfile_part_image[part], part);
		logaddrtable = do_write_partition(part);
		if ( logaddrtable )
			return logaddrtable;
		STATUS_PRINTF(" \n");
	}
	if ( (g_curflag & 0x200) != 0 )
	{
		STATUS_PRINTF("====== Write \"%s\" to \"atfile9:info\" ======\n", g_atfile_info_image);
		logaddrtable = do_write_partition(9);
		if ( logaddrtable )
			return logaddrtable;
		STATUS_PRINTF(" \n");
	}
	if ( (g_curflag & 0x4000000) != 0 )
	{
		STATUS_PRINTF("====== Display file list ======\n");
		logaddrtable = s147nand_12_load_logaddrtable();
		if ( logaddrtable )
		{
			STATUS_PRINTF(" Error: Unformatted device (%d)\n", logaddrtable);
			return logaddrtable;
		}
		for ( part = 0; part < 8; part += 1 )
			do_list_files(part);
	}
	return 0;
}

static int do_format_device(int abspart)
{
	int blocks;
	int bboffs;
	int nand_partition_offset;
	int bbcnt1;
	int i;
	USE_S147_DEV9_MEM_MMIO();

	STATUS_PRINTF("====== Format NAND device ======\n");
	STATUS_PRINTF(" [1/3]Block Erase and Check Bad Blocks\n");
	STATUS_PRINTF(" BadBlock =");
	for ( blocks = 0; blocks < g_device_info->m_block_size; blocks += 1 )
	{
		int eraseres;

		s147_dev9_mem_mmio->m_led = (blocks >> 4) & 3;
		eraseres = blocks ? s147nand_24_eraseoffset(s147nand_27_blocks2pages(blocks)) :
												s147nand_25_nand_blockerase(s147nand_27_blocks2pages(0));
		switch ( eraseres )
		{
			case -1470020:
				g_blockinfo_str_buf[blocks] = 'X';
				STATUS_PRINTF(" %d", blocks);
				break;
			case 0:
				g_blockinfo_str_buf[blocks] = '=';
				break;
			default:
				STATUS_PRINTF(" %d*(%d)", blocks, eraseres);
				break;
		}
	}
	STATUS_PRINTF("\n\n");
	STATUS_PRINTF(" [2/3]Replace Bad Blocks ('B':Boot, 'I':Info, 'X':Broken, 'R':Reserved, '@':Occupied)\n");
	g_blockinfo_dat_buf[0] = 0xEEEE;
	g_blockinfo_str_buf[0] = 'B';
	nand_partition_offset = get_nand_partition_offset(8, 8);
	for ( blocks = 1; blocks < nand_partition_offset - 1; blocks += 1 )
	{
		if ( g_blockinfo_str_buf[blocks] != '=' )
			continue;
		g_blockinfo_dat_buf[blocks] = 0xEEEE;
		g_blockinfo_str_buf[blocks] = 'R';
	}
	bbcnt1 = 0;
	if ( g_blockinfo_str_buf[nand_partition_offset - 1] == 'X' )
	{
		bbcnt1 = check_badblock_count();
		if ( bbcnt1 >= 0 )
		{
			g_blockinfo_dat_buf[nand_partition_offset - 1] = bbcnt1;
			g_blockinfo_dat_buf[bbcnt1] = 0xCCCC;
			g_blockinfo_str_buf[bbcnt1] = '@';
			bboffs = bbcnt1;
		}
	}
	else
	{
		g_blockinfo_dat_buf[nand_partition_offset - 1] = 0xAAAA;
		bboffs = nand_partition_offset - 1;
	}
	if ( bbcnt1 >= 0 )
	{
		for ( blocks = nand_partition_offset; blocks < g_device_info->m_block_size; blocks += 1 )
		{
			if ( g_blockinfo_str_buf[blocks] != 'X' )
			{
				g_blockinfo_dat_buf[blocks] = 0xAAAA;
				continue;
			}
			bbcnt1 = check_badblock_count();
			if ( bbcnt1 < 0 )
				break;
			g_blockinfo_dat_buf[blocks] = bbcnt1;
			g_blockinfo_dat_buf[bbcnt1] = 0xCCCC;
			g_blockinfo_str_buf[bbcnt1] = '@';
		}
	}
	if ( bbcnt1 < 0 )
	{
		STATUS_PRINTF(" Error: Too many bad blocks to replace\n");
		return -1;
	}
	for ( blocks = 0; blocks < g_device_info->m_block_size; blocks += 1 )
	{
		do_output_bb_info(blocks, abspart, bboffs);
		s147_dev9_mem_mmio->m_watchdog_flag2 = 0;
	}
	STATUS_PRINTF("\n");
	STATUS_PRINTF(" [3/3]Write Boot Sector and Logical Address Table\n");
	memset(&g_nand_partbuf.m_hdr, 0, sizeof(g_nand_partbuf.m_hdr));
	strncpy(g_nand_partbuf.m_hdr.m_sig, "S147NAND", 9);
	g_nand_partbuf.m_hdr.m_bootsector_ver_1 = 3;
	g_nand_partbuf.m_hdr.m_bootsector_ver_2 = 0;
	for ( i = 0; i < 8; i += 1 )
	{
		g_nand_partbuf.m_hdr.m_nand_partition_info[i].m_offset = get_nand_partition_offset(i, abspart);
		g_nand_partbuf.m_hdr.m_nand_partition_info[i].m_size = get_nand_partition_size(i, abspart);
		STATUS_PRINTF(
			" atfile%d: StartBlock = 0x%04x(%4d) / BlockSize = 0x%04x(%4d)\n",
			i,
			g_nand_partbuf.m_hdr.m_nand_partition_info[i].m_offset,
			g_nand_partbuf.m_hdr.m_nand_partition_info[i].m_offset,
			g_nand_partbuf.m_hdr.m_nand_partition_info[i].m_size,
			g_nand_partbuf.m_hdr.m_nand_partition_info[i].m_size);
	}
	g_nand_partbuf.m_hdr.m_nand_partition_8_info.m_offset = get_nand_partition_offset(8, abspart);
	g_nand_partbuf.m_hdr.m_nand_partition_8_info.m_size = get_nand_partition_size(8, abspart);
	STATUS_PRINTF(
		" system : StartBlock = 0x%04x(%4d) / BlockSize = 0x%04x(%4d)\n",
		g_nand_partbuf.m_hdr.m_nand_partition_8_info.m_offset,
		g_nand_partbuf.m_hdr.m_nand_partition_8_info.m_offset,
		g_nand_partbuf.m_hdr.m_nand_partition_8_info.m_size,
		g_nand_partbuf.m_hdr.m_nand_partition_8_info.m_size);
	STATUS_PRINTF("\n");
	g_nand_partbuf.m_hdr.m_nand_seccode[0] = g_secr_code[0];
	g_nand_partbuf.m_hdr.m_nand_seccode[1] = g_secr_code[1];
	g_nand_partbuf.m_hdr.m_nand_vidmode[0] = g_boot_video_mode;
	strncpy(g_nand_partbuf.m_hdr.m_nand_desc, g_device_info->m_nand_name, sizeof(g_nand_partbuf.m_hdr.m_nand_desc));
	g_nand_partbuf.m_hdr.m_page_size_noecc = g_device_info->m_page_size_noecc;
	g_nand_partbuf.m_hdr.m_page_size_withecc = g_device_info->m_page_size_withecc;
	g_nand_partbuf.m_hdr.m_pages_per_block = g_device_info->m_pages_per_block;
	g_nand_partbuf.m_hdr.m_block_size = g_device_info->m_block_size;
	g_nand_partbuf.m_hdr.m_acmem_delay_val = 0;
	g_nand_partbuf.m_hdr.m_acio_delay_val = generate_acio_delay_val(3, 3, 3);
	s147nand_22_nand_write_dma(&g_nand_partbuf.m_hdr, 0, 0, sizeof(g_nand_partbuf.m_hdr));
	do_dma_write_bytes_multi(g_blockinfo_dat_buf, 1, sizeof(u16) * g_device_info->m_block_size);
	return 0;
}

static int do_write_partition(int part)
{
	int fd;
	int state;
	int bytes;
	int pages;
	int blocks;
	int partblocks1;
	int actual_readres;
	int expected_readres;
	int err;
	USE_S147_DEV9_MEM_MMIO();

	actual_readres = 0;
	expected_readres = 0;
	err = 0;
	if ( part == 9 )
	{
		partblocks1 = (s147nand_9_get_nand_partition(8) - 1) * g_device_info->m_pages_per_block;
		if ( partblocks1 < 0 )
		{
			STATUS_PRINTF(" Error: No partition #0 table\n");
			return -1;
		}
		fd = open(g_atfile_info_image, O_RDONLY);
		if ( fd < 0 )
		{
			STATUS_PRINTF(" Error: File not found - \"%s\"\n", g_atfile_info_image);
			return -1;
		}
	}
	else
	{
		partblocks1 = s147nand_9_get_nand_partition(part) * g_device_info->m_pages_per_block;
		if ( partblocks1 < 0 )
		{
			STATUS_PRINTF(" Error: Invalid unit number\n");
			return -1;
		}
		fd = open(g_atfile_part_image[part], O_RDONLY);
		if ( fd < 0 )
		{
			STATUS_PRINTF(" Error: File not found - \"%s\"\n", g_atfile_part_image[part]);
			return -1;
		}
	}
	CpuSuspendIntr(&state);
	g_page_buf = (char *)AllocSysMemory(ALLOC_FIRST, g_device_info->m_page_size_noecc, 0);
	CpuResumeIntr(state);
	if ( !g_page_buf )
	{
		STATUS_PRINTF("\nError: AllocSysMemory failed\n\n");
		err = 1;
	}
	if ( !err )
	{
		if ( part == 9 )
		{
			bytes = lseek(fd, 0, SEEK_END);
			if ( g_device_info->m_page_size_noecc < bytes )
			{
				STATUS_PRINTF(" Error: INFO image file is too large - \"%s\"\n", g_atfile_info_image);
				STATUS_PRINTF(" FileSize(%d) > info(%d)\n", bytes, g_device_info->m_page_size_noecc);
				err = 1;
			}
			if ( !err )
			{
				lseek(fd, 0, SEEK_SET);
				expected_readres = 8;
				actual_readres = read(fd, g_page_buf, expected_readres);
				if ( actual_readres < expected_readres )
					err = 1;
				if ( !err )
				{
					if ( strncmp(g_page_buf, "S147INFO", 8) )
					{
						STATUS_PRINTF(" Error: \"%s\" is not a S147INFO-image file\n", g_atfile_info_image);
						err = 1;
					}
					if ( !err )
					{
						pages = 1;
						blocks = 1;
					}
				}
			}
		}
		else
		{
			int partsizebytes;

			bytes = lseek(fd, 0, SEEK_END);
			partsizebytes =
				s147nand_10_get_nand_partition_size(part) * g_device_info->m_pages_per_block * g_device_info->m_page_size_noecc;
			if ( partsizebytes < bytes )
			{
				STATUS_PRINTF(" Error: ROM image file is too large - \"%s\"\n", g_atfile_part_image[part]);
				STATUS_PRINTF(" FileSize(%d) > atfile%d(%d)\n", bytes, part, partsizebytes);
				err = 1;
			}
			if ( !err )
			{
				lseek(fd, 0, SEEK_SET);
				expected_readres = 0x20;
				actual_readres = read(fd, g_page_buf, expected_readres);
				if ( actual_readres < expected_readres )
					err = 1;
				if ( !err )
				{
					if ( strncmp(g_page_buf, "S147ROM", 8) )
					{
						STATUS_PRINTF(" Error: \"%s\" is not a S147ROM-image file\n", g_atfile_part_image[part]);
						err = 1;
					}
					if ( !err )
					{
						pages = s147nand_30_bytes2pagesnoeccround(bytes);
						blocks = s147nand_29_pages2blockround(pages);
					}
				}
			}
		}
	}
	if ( !err )
	{
		int xind2;
		int xind1;
		int pageoffs;
		int finished;

		STATUS_PRINTF(" FileSize = %dbytes SectorSize=%dsectors BlockSize=%dblocks\n", bytes, pages, blocks);
		lseek(fd, 0, SEEK_SET);
		xind2 = 0;
		pageoffs = partblocks1;
		finished = 0;
		for ( xind1 = 0; xind1 < blocks; xind1 += 1 )
		{
			int xind3;

			STATUS_PRINTF(
				" atfile%d(%d/%d): LogBlock=%d (PhyBlock=%d) ",
				part,
				xind1,
				blocks - 1,
				s147nand_28_pages2blocks(pageoffs),
				s147nand_13_translate_blockoffs(s147nand_28_pages2blocks(pageoffs)));
			s147_dev9_mem_mmio->m_led = s147nand_28_pages2blocks(pageoffs) & 3;
			if ( (g_curflag & 0xFF0000) == 0 )
			{
				STATUS_PRINTF("Erase -> ");
				if ( s147nand_11_erasetranslatepageoffs(pageoffs) == -1470020 )
				{
					STATUS_PRINTF("\nromwrite: Bad block error, use \"-f\" option.\n");
					err = 1;
					break;
				}
			}
			STATUS_PRINTF("Write -> Verify\n");
			for ( xind3 = 0; xind3 < g_device_info->m_pages_per_block; )
			{
				int i;

				// Unofficial: don't use global variable for partition buffer
				if ( part != 9 )
				{
					int xindbytes;

					xindbytes = bytes - xind2 * g_device_info->m_page_size_noecc;
					expected_readres = (xindbytes > 0x20000) ? 0x20000 : xindbytes;
				}
				else
				{
					memset(g_nand_partbuf.m_buf, 0, g_device_info->m_page_size_noecc);
					expected_readres = (g_device_info->m_page_size_noecc < bytes) ? g_device_info->m_page_size_noecc : bytes;
				}
				// Unofficial: check against read bytes instead of 0
				actual_readres = read(fd, g_nand_partbuf.m_buf, expected_readres);
				if ( actual_readres < expected_readres )
				{
					err = 1;
					finished = 1;
					break;
				}
				for ( i = 0; i <= 0x1FFFF; i += g_device_info->m_page_size_noecc )
				{
					s147nand_8_multi_write_dma(((char *)g_nand_partbuf.m_buf) + i, pageoffs, 1);
					s147nand_7_multi_read_dma(g_page_buf, pageoffs, 1);
					if ( do_verify(((char *)g_nand_partbuf.m_buf) + i, g_page_buf, g_device_info->m_page_size_noecc) )
					{
						STATUS_PRINTF(
							"romwrite: Verify error - LogBlock=%d LogPage=%d\n", s147nand_28_pages2blocks(pageoffs), pageoffs);
						err = 1;
						finished = 1;
						break;
					}
					pageoffs += 1;
					xind2 += 1;
					xind3 += 1;
					if ( xind2 >= pages )
					{
						finished = 1;
						break;
					}
				}
				if ( finished )
					break;
			}
			if ( finished )
				break;
		}
	}
	if ( actual_readres < expected_readres )
		STATUS_PRINTF(" Error: File-I/O fault (%d)\n", actual_readres);
	if ( fd >= 0 )
		close(fd);
	if ( err )
	{
		CpuSuspendIntr(&state);
		// Unofficial: don't free partition buffer
		if ( g_page_buf )
			FreeSysMemory(g_page_buf);
		CpuResumeIntr(state);
	}
	return err ? -1 : 0;
}

static int check_badblock_count(void)
{
	int retval;

	retval = -1;
	for ( ; g_badblock_count < get_nand_partition_offset(8, 8) - 1; g_badblock_count += 1 )
	{
		if ( g_blockinfo_str_buf[g_badblock_count] == 'R' )
		{
			retval = g_badblock_count;
			g_badblock_count += 1;
			break;
		}
	}
	return retval;
}

static int get_nand_partition_offset(int part, int abspart)
{
	if ( part == 8 )
		return get_nand_block_size_div_32();
	if ( abspart )
	{
		if ( part < 0 || part >= abspart )
			return -1;
		if ( abspart == -1 && g_device_info->m_block_size == (int)0x80000000 )
			__builtin_trap();
	}
	if ( !part )
		return get_nand_block_size_div_32_div_64();
	if ( abspart )
		return g_device_info->m_block_size / abspart * part;
	if ( part != 1 )
		return -1;
	return g_device_info->m_block_size / 4;
}

static int get_nand_partition_size(int part, int abspart)
{
	if ( part == 8 )
		return get_nand_block_size_div_32_div_64() - get_nand_block_size_div_32();
	if ( abspart )
	{
		if ( part < 0 || part >= abspart )
			return 0;
		if ( abspart == -1 && g_device_info->m_block_size == (int)0x80000000 )
			__builtin_trap();
		return g_device_info->m_block_size / abspart - (part ? 0 : get_nand_block_size_div_32_div_64());
	}
	if ( part )
		return (part == 1) ? (3 * (g_device_info->m_block_size / 4)) : 0;
	return g_device_info->m_block_size / 4 - get_nand_block_size_div_32_div_64();
}

static int get_nand_block_size_div_32_div_64(void)
{
	return get_nand_block_size_div_32() + g_device_info->m_block_size / 64;
}

static int get_nand_block_size_div_32(void)
{
	return g_device_info->m_block_size / 32;
}

static void do_dma_write_bytes_multi(void *ptr, int pageoffs, int pagecnt)
{
	int i;
	int bytecnt;

	bytecnt = s147nand_30_bytes2pagesnoeccround(pagecnt);
	for ( i = 0; i < bytecnt; i += 1 )
		s147nand_22_nand_write_dma(
			(char *)ptr + ((g_device_info->m_page_size_noecc >> 2) << 2) * i,
			pageoffs + i,
			0,
			g_device_info->m_page_size_noecc);
}

static int do_list_files(int part)
{
	int hdrret;
	int pageoffs;
	int xind1;
	int i;
	int dircnt;
	int filcnt;
	int finished;
	char pathtmp[18];

	hdrret = -1;
	dircnt = 0;
	filcnt = 0;
	finished = 0;
	pageoffs = s147nand_9_get_nand_partition(part) * g_device_info->m_pages_per_block;
	if ( pageoffs < 0 )
		return -19;
	for ( xind1 = 0; xind1 < 64; xind1 += 1 )
	{
		s147nand_7_multi_read_dma(g_nand_partbuf.m_buf, pageoffs + xind1, 1);
		for ( i = 0; i < 64; i += 1 )
		{
			if ( (xind1 << 6) - 1 + i == -1 )
			{
				if ( strncmp(g_nand_partbuf.m_dir.m_sig, "S147ROM", 8) )
				{
					STATUS_PRINTF(" \"%s%d:\" ... No data\n", "atfile", part);
					STATUS_PRINTF(" -----------------------------\n\n");
					return -19;
				}
				hdrret = g_nand_partbuf.m_dir.m_entrycnt;
				STATUS_PRINTF(" \"%s%d:\"\n", "atfile", part);
				STATUS_PRINTF(" -----------------------------\n");
			}
			else
			{
				if ( (xind1 << 6) - 1 + i >= hdrret )
				{
					finished = 1;
					break;
				}
				strcpy(pathtmp, g_nand_partbuf.m_direntry[i].m_name);
				if ( g_nand_partbuf.m_direntry[i].m_type == 'D' )
				{
					strcat(pathtmp, "/");
					dircnt += 1;
				}
				else
				{
					filcnt += 1;
				}
				STATUS_PRINTF(" %9d  %s\n", g_nand_partbuf.m_direntry[i].m_size, pathtmp);
				DelayThread(20000);
			}
		}
		if ( finished )
			break;
	}
	STATUS_PRINTF(" -----------------------------\n");
	STATUS_PRINTF("   %d directories, %d files\n", dircnt, filcnt);
	STATUS_PRINTF("\n");
	return hdrret;
}

static void do_output_bb_info(int blocksd, int abspart, int bboffs)
{
	(void)abspart;
	if ( (blocksd & 0x3F) == 0 )
		STATUS_PRINTF(" %04X(%4d):", blocksd, blocksd);
	STATUS_PRINTF("%c", (blocksd == bboffs) ? 'I' : g_blockinfo_str_buf[blocksd]);
	if ( (blocksd & 0xF) == 15 )
		STATUS_PRINTF(" ");
	if ( (blocksd & 0x3F) == '?' )
		STATUS_PRINTF("\n");
}

static int do_verify(void *buf1, void *buf2, int len)
{
	int i;

	for ( i = 0; i < len / 4; i += 1 )
		if ( ((u32 *)buf1)[i] != ((u32 *)buf2)[i] )
			return ((u32 *)buf1)[i] - ((u32 *)buf2)[i];
	return 0;
}

static const nand_id_desc_info_t *do_parse_device_info(const char *nandid)
{
	int i;
	int j;

	for ( i = 0; g_nand_type_info[i].m_nand_name; i += 1 )
	{
		int cmpval;

		cmpval = 0;
		for ( j = 0; j < 5; j += 1 )
			if ( ((int)g_nand_type_info[i].m_id[j] == -1) || ((u8)g_nand_type_info[i].m_id[j] == (u8)nandid[j]) )
				cmpval += 1;
		if ( cmpval == 5 )
			return &g_nand_type_info[i];
	}
	return 0;
}
