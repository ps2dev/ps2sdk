#include <dirent.h>

#ifdef F_opendir
/** Open a directory
    @param path
    @return DIR struct to be used for rest of dirent functions
*/
DIR *opendir(const char *path)
{
	printf("opendir not implemented\n");
	return 0;
}
#endif

#ifdef F_readdir
/** Reads an entry from handle opened previously by opendir
    @param d
    @return
*/
struct dirent *readdir(DIR *dir)
{
	printf("readdir not implemented\n");
	return 0;
}
#endif

#ifdef F_rewinddir
/** Rewinds
    @param d
*/
void rewinddir(DIR *dir)
{
	printf("rewinddir not implemented\n");
}
#endif

#ifdef F_closedir
/** Release DIR handle
    @param d
    @return Zero on sucess
*/
int closedir(DIR *dir)
{
	printf("closedir not implemented\n");
	return 0;
}
#endif

	