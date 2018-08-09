/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

typedef union {
	u64 EEGS;
	u8 data[40];
} SystemConfiguration_t;

void InitSystemConfig(SystemConfiguration_t *SysConf, int SysConfLen);
void SetOsdConfigParam(ConfigParam* config);
void GetOsdConfigParam(ConfigParam* config);
void SetOsdConfigParam2(void* config, int size, int offset);
int GetOsdConfigParam2(void* config, int size, int offset);
