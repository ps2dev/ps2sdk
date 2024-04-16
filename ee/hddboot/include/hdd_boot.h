#ifndef H_HDDBOOT_INC
#define H_HDDBOOT_INC

void CheckHDDUpdate(int device, const char *SysExecFolder);
int GetHddSupportEnabledStatus(void);
int GetHddUpdateStatus(void);
void DetermineHDDLoadStatus(void);

#endif