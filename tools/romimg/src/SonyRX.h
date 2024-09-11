#ifndef __SONY_RX_H__
#define __SONY_RX_H__

int IsSonyRXModule(const char *path);
int GetSonyRXModInfo(const char *path, char *description, unsigned int MaxLength, unsigned short int *version);

#endif /* __SONY_RX_H__ */