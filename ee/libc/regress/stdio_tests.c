#include <stdio.h>
#include <string.h>

#include <testsuite.h>

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

	return NULL;
}

static const char *test_fopen_fclose(void *arg)
{
	FILE *fp = fopen((char *)arg, "rt");
	if (fp == NULL)
	{
		return "failed to open test file";
	}

	fclose(fp);
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
	for (p=0; p<512; p++)
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
		error = "ftell returned wrong offset\n";
		goto failed;
	}

	ret = fread(buf, 2, 1, fp);
	if (ret != 1 || strncmp(buf, "he", 2) != 0)
	{
		error = "failed to read first two bytes";
		goto failed;
	}

	/* seek forward */
	ret = fseek(fp, +3, SEEK_CUR);
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
	ret = fseek(fp, -1, SEEK_CUR);
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

	/* seek to end of file */
	ret = fseek(fp, 0, SEEK_END);
	off = ftell(fp);
	if (ret != 0 || off < 8)
	{
		/* fixme: what's the real size of ''arg'' file? */
		error = "error seeking end of file";
		goto failed;
	}

	fclose(fp);
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
	return NULL;

	failed:
	fclose(fp);
	return error;
}

int libc_add_tests(test_suite *p)
{
	const char *textfile;

	#ifdef _EE
	textfile = "host:testfiles/dummy";
	#else
	textfile = "testfiles/dummy";
	#endif

        add_test(p, "fopen, fclose", test_fopen_fclose, (void *)textfile);
	add_test(p, "fgets", test_fgets, (void *)textfile);
	add_test(p, "fread", test_fread, (void *)textfile);
	add_test(p, "fgetc", test_fgetc, (void *)textfile);
	add_test(p, "fseek, ftell", test_fseek_ftell, (void *)textfile);
	return 0;
}

