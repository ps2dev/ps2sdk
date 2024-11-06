#ifndef __PLATFORM_H__
#define __PLATFORM_H__

int GetUsername(char *buffer, unsigned int BufferSize);
int GetLocalhostName(char *buffer, unsigned int BufferSize);
unsigned int GetSystemDate(void);
unsigned int GetFileCreationDate(const char *path);
int GetCurrentWorkingDirectory(char *buffer, unsigned int BufferSize);
void upperbuff(char *temp);

#if defined(_WIN32) || defined(WIN32)
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif
#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

#endif /* __PLATFORM_H__ */
