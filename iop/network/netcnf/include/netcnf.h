/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _NETCNF_H
#define _NETCNF_H

typedef struct sceNetCnfList
{
	int type;
	int stat;
	char sys_name[256];
	char usr_name[256];
} sceNetCnfList_t;

typedef struct sceNetCnfAddress
{
	int reserved;
	char data[16];
} sceNetCnfAddress_t;

typedef struct sceNetCnfCommand
{
	struct sceNetCnfCommand *forw;
	struct sceNetCnfCommand *back;
	int code;
} sceNetCnfCommand_t;

typedef struct sceNetCnfUnknown
{
	struct sceNetCnfUnknown *forw;
	struct sceNetCnfUnknown *back;
} sceNetCnfUnknown_t;

typedef struct sceNetCnfUnknownList
{
	struct sceNetCnfUnknown *head;
	struct sceNetCnfUnknown *tail;
} sceNetCnfUnknownList_t;

typedef int (*sceNetCnfOpenFunction)(const char *device, const char *pathname, int flags, int mode, int *filesize);
typedef int (*sceNetCnfReadFunction)(int fd, const char *device, const char *pathname, void *buf, int offset, int size);
typedef int (*sceNetCnfCloseFunction)(int fd);

typedef struct sceNetCnfCallback
{
	int type;
	sceNetCnfOpenFunction open;
	sceNetCnfReadFunction read;
	sceNetCnfCloseFunction close;
} sceNetCnfCallback_t;

struct sceNetCnfInterface_want_allow
{
	unsigned char mru_nego;
	unsigned char accm_nego;
	unsigned char magic_nego;
	unsigned char prc_nego;
	unsigned char acc_nego;
	unsigned char address_nego;
	unsigned char vjcomp_nego;
	unsigned char dns1_nego;
	unsigned char dns2_nego;
	unsigned char reserved_nego[7];
	unsigned short mru;
	unsigned int accm;
	unsigned char auth;
	unsigned char f_mru;
	unsigned char f_accm;
	unsigned char f_auth;
	unsigned char *ip_address;
	unsigned char *ip_mask;
	unsigned char *dns1;
	unsigned char *dns2;
	unsigned int reserved_value[8];
};

typedef struct sceNetCnfInterface
{
	int type;
	unsigned char *vendor;
	unsigned char *product;
	unsigned char *location;
	unsigned char dhcp;
	unsigned char *dhcp_host_name;
	unsigned char dhcp_host_name_null_terminated;
	unsigned char dhcp_release_on_stop;
	unsigned char *address;
	unsigned char *netmask;
	unsigned char *chat_additional;
	int redial_count;
	int redial_interval;
	unsigned char *outside_number;
	unsigned char *outside_delay;
	unsigned char *phone_numbers[10];
	unsigned char answer_mode;
	int answer_timeout;
	int dialing_type;
	unsigned char *chat_login;
	unsigned char *auth_name;
	unsigned char *auth_key;
	unsigned char *peer_name;
	unsigned char *peer_key;
	int lcp_timeout;
	int ipcp_timeout;
	int idle_timeout;
	int connect_timeout;
	struct sceNetCnfInterface_want_allow want;
	struct sceNetCnfInterface_want_allow allow;
	int log_flags;
	unsigned char force_chap_type;
	unsigned char omit_empty_frame;
	unsigned char pppoe;
	unsigned char pppoe_host_uniq_auto;
	unsigned char pppoe_reserved[2];
	unsigned char *pppoe_service_name;
	unsigned char *pppoe_ac_name;
	int mtu;
	unsigned char lcp_max_configure;
	unsigned char lcp_max_terminate;
	unsigned char ipcp_max_configure;
	unsigned char ipcp_max_terminate;
	unsigned char auth_timeout;
	unsigned char auth_max_failure;
	unsigned char reserved[6];
	int phy_config;
	struct sceNetCnfCommand *cmd_head;
	struct sceNetCnfCommand *cmd_tail;
	struct sceNetCnfUnknownList unknown_list;
} sceNetCnfInterface_t;

typedef struct sceNetCnfDial
{
	unsigned char *tone_dial;
	unsigned char *pulse_dial;
	unsigned char *any_dial;
	unsigned char *chat_init;
	unsigned char *chat_dial;
	unsigned char *chat_answer;
	unsigned char *redial_string;
	struct sceNetCnfUnknownList unknown_list;
} sceNetCnfDial_t;

typedef struct sceNetCnfCtl
{
	struct sceNetCnfDial *dial;
	struct sceNetCnfInterface *ifc;
	int id;
	int phone_index;
	int redial_index;
	char interface[9];
} sceNetCnfCtl_t;

typedef struct sceNetCnfPair
{
	struct sceNetCnfPair *forw;
	struct sceNetCnfPair *back;
	unsigned char *display_name;
	unsigned char *attach_ifc;
	unsigned char *attach_dev;
	struct sceNetCnfInterface *ifc;
	struct sceNetCnfInterface *dev;
	struct sceNetCnfUnknownList unknown_list;
	struct sceNetCnfCtl *ctl;
} sceNetCnfPair_t;

typedef struct sceNetCnfRoot
{
	struct sceNetCnfPair *pair_head;
	struct sceNetCnfPair *pair_tail;
	int version;
	unsigned char *chat_additional;
	int redial_count;
	int redial_interval;
	unsigned char *outside_number;
	unsigned char *outside_delay;
	int dialing_type;
	struct sceNetCnfUnknownList unknown_list;
} sceNetCnfRoot_t;

typedef struct sceNetCnfEnv
{
	char *dir_name;
	char *arg_fname;
	void *mem_base;
	void *mem_ptr;
	void *mem_last;
	int req;
	struct sceNetCnfRoot *root;
	struct sceNetCnfInterface *ifc;
	int f_no_check_magic;
	int f_no_decode;
	int f_verbose;
	int file_err;
	int alloc_err;
	int syntax_err;
	const char *fname;
	int lno;
	unsigned char lbuf[1024];
	unsigned char dbuf[1024];
	int ac;
	const char *av[11];
} sceNetCnfEnv_t;

typedef struct sceNetCnfRoutingEntry
{
	struct sceNetCnfAddress dstaddr;
	struct sceNetCnfAddress gateway;
	struct sceNetCnfAddress genmask;
	int flags;
	int mss;
	int window;
	char interface[9];
} sceNetCnfRoutingEntry_t;

typedef struct route
{
	sceNetCnfCommand_t cmd;
	sceNetCnfRoutingEntry_t re;
} route_t;

typedef struct nameserver
{
	sceNetCnfCommand_t cmd;
	sceNetCnfAddress_t address;
} nameserver_t;

extern int sceNetCnfGetCount(const char *fname, int type);
extern int sceNetCnfGetList(const char *fname, int type, sceNetCnfList_t *p);
extern int sceNetCnfLoadEntry(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e);
extern int sceNetCnfAddEntry(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e);
extern int sceNetCnfDeleteEntry(const char *fname, int type, const char *usr_name);
extern int sceNetCnfSetLatestEntry(const char *fname, int type, const char *usr_name);
extern void *sceNetCnfAllocMem(sceNetCnfEnv_t *e, int size, int align);
extern int sceNetCnfInitIFC(sceNetCnfInterface_t *ifc);
extern int sceNetCnfLoadConf(sceNetCnfEnv_t *e);
extern int sceNetCnfLoadDial(sceNetCnfEnv_t *e, sceNetCnfPair_t *pair);
extern int sceNetCnfMergeConf(sceNetCnfEnv_t *e);
extern int sceNetCnfName2Address(sceNetCnfAddress_t *paddr, const char *buf);
extern int sceNetCnfAddress2String(char *buf, int len, const sceNetCnfAddress_t *paddr);
extern int
sceNetCnfEditEntry(const char *fname, int type, const char *usr_name, const char *new_usr_name, sceNetCnfEnv_t *e);
extern int sceNetCnfDeleteAll(const char *dev);
extern int sceNetCnfCheckCapacity(const char *fname);
extern int sceNetCnfConvA2S(char *sp_, char *dp_, int len);
extern int sceNetCnfConvS2A(char *sp_, char *dp_, int len);
extern int sceNetCnfCheckSpecialProvider(const char *fname, int type, const char *usr_name, sceNetCnfEnv_t *e);
extern void sceNetCnfSetCallback(sceNetCnfCallback_t *pcallback);

#define netcnf_IMPORTS_start DECLARE_IMPORT_TABLE(netcnf, 1, 32)
#define netcnf_IMPORTS_end END_IMPORT_TABLE

#define I_sceNetCnfGetCount DECLARE_IMPORT(4, sceNetCnfGetCount)
#define I_sceNetCnfGetList DECLARE_IMPORT(5, sceNetCnfGetList)
#define I_sceNetCnfLoadEntry DECLARE_IMPORT(6, sceNetCnfLoadEntry)
#define I_sceNetCnfAddEntry DECLARE_IMPORT(7, sceNetCnfAddEntry)
#define I_sceNetCnfDeleteEntry DECLARE_IMPORT(8, sceNetCnfDeleteEntry)
#define I_sceNetCnfSetLatestEntry DECLARE_IMPORT(9, sceNetCnfSetLatestEntry)
#define I_sceNetCnfAllocMem DECLARE_IMPORT(10, sceNetCnfAllocMem)
#define I_sceNetCnfInitIFC DECLARE_IMPORT(11, sceNetCnfInitIFC)
#define I_sceNetCnfLoadConf DECLARE_IMPORT(12, sceNetCnfLoadConf)
#define I_sceNetCnfLoadDial DECLARE_IMPORT(13, sceNetCnfLoadDial)
#define I_sceNetCnfMergeConf DECLARE_IMPORT(14, sceNetCnfMergeConf)
#define I_sceNetCnfName2Address DECLARE_IMPORT(15, sceNetCnfName2Address)
#define I_sceNetCnfAddress2String DECLARE_IMPORT(16, sceNetCnfAddress2String)
#define I_sceNetCnfEditEntry DECLARE_IMPORT(17, sceNetCnfEditEntry)
#define I_sceNetCnfDeleteAll DECLARE_IMPORT(18, sceNetCnfDeleteAll)
#define I_sceNetCnfCheckCapacity DECLARE_IMPORT(19, sceNetCnfCheckCapacity)
#define I_sceNetCnfConvA2S DECLARE_IMPORT(20, sceNetCnfConvA2S)
#define I_sceNetCnfConvS2A DECLARE_IMPORT(21, sceNetCnfConvS2A)
#define I_sceNetCnfCheckSpecialProvider DECLARE_IMPORT(22, sceNetCnfCheckSpecialProvider)
#define I_sceNetCnfSetCallback DECLARE_IMPORT(23, sceNetCnfSetCallback)

#endif
