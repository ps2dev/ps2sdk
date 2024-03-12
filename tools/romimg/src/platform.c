/*
    platform.c	- Contains platform-specific functions.
*/

#include <errno.h>
#include <stdlib.h>
#include "dprintf.h"
#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#endif

#include "platform.h"

#if defined(_WIN32) || defined(WIN32)
int GetUsername(char *buffer, unsigned int BufferSize)
{
	int ret;
	DWORD lpnSize;
	lpnSize = BufferSize;
	ret = (GetUserNameA(buffer, &lpnSize) == 0 ? EIO : 0);
	DPRINTF("(%d): %s\n", ret, buffer);
	return ret;
}
#endif

int GetLocalhostName(char *buffer, unsigned int BufferSize)
{
	int ret;
#if defined(_WIN32) || defined(WIN32)
	DWORD lpnSize;
	lpnSize = BufferSize;
	ret = (GetComputerNameA(buffer, &lpnSize) == 0 ? EIO : 0);
#else
	ret = gethostname(buffer, BufferSize);
#endif
	DPRINTF("(%d): %s\n", ret, buffer);
	return ret;
}

// macros for converting between an integer and a BCD number
#ifndef btoi
#define btoi(b) ((b) / 16 * 10 + (b) % 16)  // BCD to int
#endif
#ifndef itob
#define itob(i) ((i) / 10 * 16 + (i) % 10)  // int to BCD
#endif

/* Converts the specified value to a value that looks the same in base 16. E.g. It converts 2012 in decimal to 0x2012 in hexadecimal. */
static unsigned short int ConvertToBase16(unsigned short int value)
{
	unsigned short int result;

	result = value + value / 10 * 0x06;
	result += value / 100 * 0x60;
	return (result + value / 1000 * 0x600);
}


unsigned int GetSystemDate(void)
{
#if defined(_WIN32) || defined(WIN32)
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);

	return (((unsigned int)ConvertToBase16(SystemTime.wYear)) << 16 | ConvertToBase16(SystemTime.wMonth) << 8 | ConvertToBase16(SystemTime.wDay));
#else

	time_t time_raw_format;
	struct tm *ptr_time;

	time(&time_raw_format);
	ptr_time = localtime(&time_raw_format);
	return (((unsigned int)ConvertToBase16(ptr_time->tm_year + 1900)) << 16 | ConvertToBase16(ptr_time->tm_mon + 1) << 8 | ConvertToBase16(ptr_time->tm_mday));
#endif
}

unsigned int GetFileCreationDate(const char *path)
{
#if defined(_WIN32) || defined(WIN32)
	HANDLE hFile;
	FILETIME CreationTime;
	SYSTEMTIME CreationSystemTime;

	if ((hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE) {
		GetFileTime(hFile, &CreationTime, NULL, NULL);
		CloseHandle(hFile);

		FileTimeToSystemTime(&CreationTime, &CreationSystemTime);
	} else
		GetSystemTime(&CreationSystemTime);

	return (((unsigned int)ConvertToBase16(CreationSystemTime.wYear)) << 16 | ConvertToBase16(CreationSystemTime.wMonth) << 8 | ConvertToBase16(CreationSystemTime.wDay));
#else
	struct tm *clock;                    // create a time structure
	struct stat attrib;                  // create a file attribute structure
	stat(path, &attrib);                 // get the attributes of afile.txt
	clock = gmtime(&(attrib.st_mtime));  // Get the last modified time and put it into the time structure
	return (((unsigned int)ConvertToBase16(clock->tm_year + 1900)) << 16 | ConvertToBase16(clock->tm_mon) << 8 | ConvertToBase16(clock->tm_mday));
#endif
}

int GetCurrentWorkingDirectory(char *buffer, unsigned int BufferSize)
{
#if defined(_WIN32) || defined(WIN32)
	return (GetCurrentDirectoryA(BufferSize, buffer) == 0 ? EIO : 0);
#else
	if (getcwd(buffer, BufferSize) != NULL)
		return 0;
	else
		return EIO;
#endif
}
