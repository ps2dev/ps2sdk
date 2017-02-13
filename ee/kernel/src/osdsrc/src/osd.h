typedef union
{
	u64 EEGS;
	u8 data[40];
} SystemConfiguration_t;

void InitSystemConfig(void *SysConf, int SysConfLen);
void SetOsdConfigParam(ConfigParam *config);
void GetOsdConfigParam(ConfigParam *config);
void SetOsdConfigParam2(void *config, int size, int offset);
int GetOsdConfigParam2(void *config, int size, int offset);
