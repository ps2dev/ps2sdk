#ifndef __PLATFORM_H__
#define __PLATFORM_H__

extern int GetUsername(char *buffer, unsigned int BufferSize);
extern int GetLocalhostName(char *buffer, unsigned int BufferSize);
extern unsigned int GetSystemDate(void);
extern unsigned int GetFileCreationDate(const char *path);
extern int GetCurrentWorkingDirectory(char *buffer, unsigned int BufferSize);
extern void upperbuff(char *temp);

#if defined(_WIN32) || defined(WIN32)
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

#endif /* __PLATFORM_H__ */
