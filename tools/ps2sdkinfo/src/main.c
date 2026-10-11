/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <string.h>
#include "gitinfo.h"
#include "buildinfo.h"

typedef struct info_item_
{
	const char *key;
	const char *value;
} info_item_t;

static info_item_t info_items[] =
{
	{"PS2SDK_GIT_REMOTE_URL", PS2SDK_GIT_REMOTE_URL},
	{"PS2SDK_GIT_HASH", PS2SDK_GIT_HASH},
	{"PS2SDK_GIT_TIME", PS2SDK_GIT_TIME},
	{"PS2SDK_GIT_TIME_ISO8601", PS2SDK_GIT_TIME_ISO8601},
	{"PS2SDK_GIT_TIME_RFC2822", PS2SDK_GIT_TIME_RFC2822},
	{"PS2SDK_GIT_TAG", PS2SDK_GIT_TAG},
	{"PS2SDK_BUILD_PATH", PS2SDK_BUILD_PATH},
	{"PS2SDK_BUILD_USER", PS2SDK_BUILD_USER},
	{"PS2SDK_BUILD_MACHINE", PS2SDK_BUILD_MACHINE},
	{"PS2SDK_BUILD_TIME", PS2SDK_BUILD_TIME},
	{"PS2SDK_BUILD_TIME_ISO8601", PS2SDK_BUILD_TIME_ISO8601},
	{"PS2SDK_BUILD_TIME_RFC2822", PS2SDK_BUILD_TIME_RFC2822},
	{NULL, NULL},
};

int main(int argc, char *argv[])
{
	int retval;
	retval = 0;
	if (argc <= 1)
	{
		for (int i = 0; info_items[i].key != NULL; i += 1)
		{
			printf("%s=%s\n", info_items[i].key, info_items[i].value);
		}
		return retval;
	}

	for (int i = 1; i < argc; i += 1)
	{
		int found;
		found = 0;
		for (int j = 0; info_items[j].key != NULL; j += 1)
		{
			if (strcmp(info_items[j].key, argv[i]) == 0)
			{
				printf("%s\n", info_items[j].value);
				found = 1;
				break;
			}
		}
		if (found == 0)
		{
			printf("%s\n", "NOTFOUND");
			retval = 1;
		}
	}

	return retval;
}
