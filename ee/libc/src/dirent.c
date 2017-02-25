#include <dirent.h>
#include <stdio.h>

#ifdef F_opendir
DIR *opendir(const char *path)
{
	printf("opendir not implemented\n");
	return 0;
}
#endif

#ifdef F_readdir
struct dirent *readdir(DIR *dir)
{
	printf("readdir not implemented\n");
	return 0;
}
#endif

#ifdef F_rewinddir
void rewinddir(DIR *dir)
{
	printf("rewinddir not implemented\n");
}
#endif

#ifdef F_closedir
int closedir(DIR *dir)
{
	printf("closedir not implemented\n");
	return 0;
}
#endif
