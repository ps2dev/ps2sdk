#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "testsuite.h"

static const char *test_fopen_fclose(void *arg)
{
    FILE *fp = fopen((char *)arg, "rt");
    if (fp == NULL)
    {
        return "failed to open test file";
    }

    fclose(fp);
    printf("\nSUCCESS: all checks passed\n");
    return NULL;
}

static const char *test_fgets(void *arg)
{
    char buf[64], *ret;

    FILE *fp = fopen((char *)arg, "rt");

    if (fp == NULL)
    {
        return "failed to open test file";
    }

    ret = fgets(buf, sizeof(buf), fp);
    fclose(fp);

    if (ret != buf || strcmp(buf, "hello world\n") != 0)
    {
        return "read wrong data from file";
    }

    printf("\nSUCCESS: all checks passed\n");
    return NULL;
}

static const char *test_fgetc(void *arg)
{
    int p, ch;
    char *error;

    FILE *fp = fopen((char *)arg, "rt");
    if (fp == NULL)
    {
        return "failed to open test file";
    }

    /* read one byte */
    ch = fgetc(fp);
    if (ch != 'h')
    {
        error = "fgetc failed";
        goto failed;
    }

    /* read until EOF */
    for (p = 0; p < 512; p++)
    {
        ch = fgetc(fp);
        if (ch == EOF)
        {
            break;
        }
    }

    if (ch != EOF)
    {
        error = "EOF not returned when should";
        goto failed;
    }

    fclose(fp);
    printf("\nSUCCESS: all checks passed\n");
    return NULL;

failed:
    fclose(fp);
    return error;
}

static const char *test_fseek_ftell(void *arg)
{
    int off, ret;
    char buf[512], *error;

    FILE *fp = fopen((char *)arg, "rt");
    if (fp == NULL)
    {
        return "failed to open test file";
    }

    /* seek to the beginning of a file */
    ret = fseek(fp, 0, SEEK_SET);
    if (ret != 0)
    {
        error = "failed to seek to beginning of file";
        goto failed;
    }

    off = ftell(fp);
    if (off != 0)
    {
        error = "ftell returned wrong offset";
        goto failed;
    }

    ret = fread(buf, 2, 1, fp);
    if (ret != 1 || strncmp(buf, "he", 2) != 0)
    {
        error = "failed to read first two bytes";
        goto failed;
    }

    /* seek forward */
    ret = fseek(fp, +2, SEEK_CUR);
    off = ftell(fp);
    if (ret != 0 || off != 4)
    {
        error = "failed to seek forward / tell returned wrong offset";
        goto failed;
    }

    ret = fread(buf, 2, 1, fp);
    if (ret != 1 || strncmp(buf, "o ", 2) != 0)
    {
        error = "error reading from test file";
        goto failed;
    }

    /* seek backward */
    ret = fseek(fp, -2, SEEK_CUR);
    off = ftell(fp);
    if (ret != 0 || off != 4)
    {
        error = "error seeking backwards / tell returned wrong offset";
        goto failed;
    }

    ret = fread(buf, 2, 1, fp);
    if (ret != 1 || strncmp(buf, "o ", 2) != 0)
    {
        error = "error reading from test file";
        goto failed;
    }

    /* seek to end of file
     "hello world" = 12 bytes */
    ret = fseek(fp, 0, SEEK_END);
    off = ftell(fp);
    if (ret != 0 || off != 12)
    {
        error = "error seeking end of file";
        goto failed;
    }

    fclose(fp);
    printf("\nSUCCESS: all checks passed\n");
    return NULL;

failed:
    fclose(fp);
    return error;
}

static const char *test_fread(void *arg)
{
    int ret;
    char buf[512], *error;

    FILE *fp = fopen((char *)arg, "rt");
    if (fp == NULL)
    {
        return "cannot open test file";
    }

    /* test of reading one chunk */
    ret = fread(buf, 3, 1, fp);
    if (ret != 1 || strncmp(buf, "hel", 3) != 0)
    {
        error = "failed to read one block";
        goto failed;
    }

    /* three chunks */
    ret = fread(buf, 2, 3, fp);
    if (ret != 3 || strncmp(buf, "lo wor", 6) != 0)
    {
        error = "failed to read three blocks";
        goto failed;
    }

    /* until end of file */
    ret = fread(buf, 1, sizeof(buf), fp);
    if (ret < 2 || strncmp(buf, "ld", 2) != 0)
    {
        error = "failed to read until eof";
        goto failed;
    }

    /* past end of file */
    ret = fread(buf, 1, sizeof(buf), fp);
    if (ret != 0)
    {
        error = "read past end of file";
        goto failed;
    }

    fclose(fp);
    printf("\nSUCCESS: all checks passed\n");
    return NULL;

failed:
    fclose(fp);
    return error;
}

static const char *test_fwrite(void *arg)
{
    FILE *fp;
    int ret;

    char buf[12] = "hello world";
    char buf2[20];

    fp = fopen((char *)arg, "w+");
    if (fp == NULL)
    {
        return "cannot open/create test file";
    }

    ret = fwrite(buf, 2, 1, fp);
    if (ret != 1)
    {
        fclose(fp);
        return "failed to write 1 block of 2 bytes";
    }

    fseek(fp, 0, SEEK_SET);
    ret = fread(buf2, 2, 1, fp);
    if (ret != 1 || strncmp(buf2, "he", 2) != 0)
    {
        fclose(fp);
        printf("buf2 = %s ret = %d\n", buf2, ret);
        return "failed to write/read 2 bytes correctly";
    }

    ret = fwrite(buf, 1, strlen(buf), fp);
    if (ret != 11)
    {
        fclose(fp);
        return "failed to write 11 bytes";
    }

    fseek(fp, 0, SEEK_SET);
    ret = fread(buf2, 1, sizeof(buf2), fp);
    if (ret != 13 || strncmp(buf2, "hehello world", 13))
    {
        fclose(fp);
        return "failed to write rest of file";
    }

    fclose(fp);

    printf("\nSUCCESS: all checks passed\n");
    return NULL;
}

static const char *test_stat_file(void *arg)
{
    struct stat st;

    if (stat((const char *)arg, &st) != 0)
    {
        return "failed to get stat for file";
    }

    printf("fn %s mode %lo, size %d\n time %s\n",
           (char *)arg, st.st_mode, (int)st.st_size,
           ctime(&st.st_mtime));

    if (S_ISDIR(st.st_mode))
    {
        return "expected file, not a directory (ignore if using host:)";
    }

    printf("\nSUCCESS: all checks passed\n");
    return 0;
}

static const char *test_stat_dir(void *arg)
{
    struct stat st;

    if (stat((const char *)arg, &st) != 0)
    {
        return "failed to get stat for directory";
    }

    printf("fn %s mode %lo, size %d\n time %s\n",
           (char *)arg, st.st_mode, (int)st.st_size,
           ctime(&st.st_mtime));

    if (S_ISDIR(st.st_mode) == 0)
    {
        return "expected directory, not regular file (ignore if using host:)";
    }

    printf("\nSUCCESS: all checks passed\n");
    return 0;
}

static const char *test_mkdir(void *arg)
{
    mode_t mode = 0x0755;

    if (mkdir(arg, mode) != 0)
    {
        return "failed to create directory";
    }

    printf("\nSUCCESS: all checks passed\n");
    return 0;
}

static const char *test_rmdir(void *arg)
{
    if (rmdir(arg) != 0)
    {
        return "failed to delete directory";
    }

    printf("\nSUCCESS: all checks passed\n");
    return 0;
}

static const char *test_opendir_closedir(void *arg)
{
    DIR *dd = opendir((char *)arg);

    if (dd == NULL)
        return "failed to open directory";

    if (closedir(dd) != 0)
        return "failed to close directory";

    printf("\nSUCCESS: all checks passed\n");
    return NULL;
}

static const char *test_readdir_rewinddir(void *arg)
{
    int i   = 0;
    int j   = 0;

    DIR *dd = opendir((char *)arg);
    struct dirent *de;

    if (dd == NULL)
        return "failed to open directory";

    while ((de = readdir(dd)) != NULL)
    {
        if (de->d_name == NULL)
        {
            closedir(dd);
            return "bad dirent returned";
        }
        printf("entry[%d] = %s\n", i, de->d_name);
        if (!strlen(de->d_name))
        {
            closedir(dd);
            return "empty dirent returned (ignore if using host:)";
        }
        i++;
    }

    rewinddir(dd);

    while ((de = readdir(dd)) != NULL)
    {
        if (de->d_name == NULL)
        {
            closedir(dd);
            return "bad dirent returned after rewind";
        }
        printf("entry[%d] = %s\n", j, de->d_name);
        if (!strlen(de->d_name))
        {
            closedir(dd);
            return "empty dirent returned after rewind";
        }
        j++;
    }

    closedir(dd);

    if ((i < 1) || (j < 1))
        return "no directory entries read";

    if (i != j)
        return "directory entries don't match/rewind failed";

    printf("\nSUCCESS: all checks passed\n");
    return NULL;
}

static const char *test_failed_open(void *arg)
{
    int handle = open((char *)arg, O_RDONLY);
    if (handle != -1)
        return "wrong return error opening non existing file";

    printf("\nSUCCESS: all checks passed\n");
    return NULL;
}

int libc_add_tests(test_suite *p)
{
    const char *textfile, *textfile2, *invalidtextfile;
    const char *dir, *dir2;

#ifdef _EE
    textfile        = "host:testfiles/dummy";
    textfile2       = "host:testfiles/dummy2";
    invalidtextfile = "host:testfiles/invalidtextfile"; // This file shouldn't exist
    dir             = "host:testfiles/";
    dir2            = "host:dummydir/";
#else
    textfile        = "testfiles/dummy";
    textfile2       = "testfiles/dummy2";
    invalidtextfile = "testfiles/invalidtextfile";
    dir             = "testfiles/";
    dir2            = "dummydir/";
#endif

    /* If testing using usbd/usbhdfsd or ps2hdd/ps2fs, this adds a 10
       second delay so that the devices can initialize fully.
    sleep(10); */

    add_test(p, "fopen, fclose\n", test_fopen_fclose, (void *)textfile);
    add_test(p, "fgets\n", test_fgets, (void *)textfile);
    add_test(p, "fread\n", test_fread, (void *)textfile);
    add_test(p, "fwrite\n", test_fwrite, (void *)textfile2);
    add_test(p, "fgetc\n", test_fgetc, (void *)textfile);
    add_test(p, "fseek, ftell\n", test_fseek_ftell, (void *)textfile);
    add_test(p, "stat file\n", test_stat_file, (void *)textfile);
    add_test(p, "stat dir\n", test_stat_dir, (void *)dir);
    add_test(p, "mkdir\n", test_mkdir, (void *)dir2);
    add_test(p, "rmdir\n", test_rmdir, (void *)dir2);
    add_test(p, "opendir, closedir\n", test_opendir_closedir, (void *)dir);
    add_test(p, "readdir, rewinddir\n", test_readdir_rewinddir, (void *)dir);
    add_test(p, "failed open\n", test_failed_open, (void *)invalidtextfile);

    return 0;
}
