#include <stdio.h>
#include <string.h>

#include <graph.h>
#include <graph_config.h>

int graph_make_config(int mode, int interlace, int ffmd, int x, int y, int flicker_filter, char *config)
{

	// Save the current mode value.
	switch (mode)
	{

		case GRAPH_MODE_NTSC:			sprintf(config, "GRAPH_MODE_NTSC:");		break;
		case GRAPH_MODE_PAL:			sprintf(config, "GRAPH_MODE_PAL:");			break;
		case GRAPH_MODE_HDTV_480P:		sprintf(config, "GRAPH_MODE_HDTV_480P:");	break;
		case GRAPH_MODE_HDTV_576P:		sprintf(config, "GRAPH_MODE_HDTV_576P:");	break;
		case GRAPH_MODE_HDTV_720P:		sprintf(config, "GRAPH_MODE_HDTV_720P:");	break;
		case GRAPH_MODE_HDTV_1080I:		sprintf(config, "GRAPH_MODE_HDTV_1080I:");	break;
		case GRAPH_MODE_VGA_640_60:		sprintf(config, "GRAPH_MODE_VGA_640_60:");	break;
		case GRAPH_MODE_VGA_640_72:		sprintf(config, "GRAPH_MODE_VGA_640_72:");	break;
		case GRAPH_MODE_VGA_640_75:		sprintf(config, "GRAPH_MODE_VGA_640_75:");	break;
		case GRAPH_MODE_VGA_640_85:		sprintf(config, "GRAPH_MODE_VGA_640_85:");	break;
		case GRAPH_MODE_VGA_800_56:		sprintf(config, "GRAPH_MODE_VGA_800_56:");	break;
		case GRAPH_MODE_VGA_800_60:		sprintf(config, "GRAPH_MODE_VGA_800_60:");	break;
		case GRAPH_MODE_VGA_800_72:		sprintf(config, "GRAPH_MODE_VGA_800_72:");	break;
		case GRAPH_MODE_VGA_800_75:		sprintf(config, "GRAPH_MODE_VGA_800_75:");	break;
		case GRAPH_MODE_VGA_800_85:		sprintf(config, "GRAPH_MODE_VGA_800_85:");	break;
		case GRAPH_MODE_VGA_1024_60:	sprintf(config, "GRAPH_MODE_VGA_1024_60:");	break;
		case GRAPH_MODE_VGA_1024_70:	sprintf(config, "GRAPH_MODE_VGA_1024_70:");	break;
		case GRAPH_MODE_VGA_1024_75:	sprintf(config, "GRAPH_MODE_VGA_1024_75:");	break;
		case GRAPH_MODE_VGA_1024_85:	sprintf(config, "GRAPH_MODE_VGA_1024_85:");	break;
		case GRAPH_MODE_VGA_1280_60:	sprintf(config, "GRAPH_MODE_VGA_1280_60:");	break;
		case GRAPH_MODE_VGA_1280_75:	sprintf(config, "GRAPH_MODE_VGA_1280_75:");	break;
		default:						sprintf(config, "GRAPH_MODE_AUTO:");		break;

	}

	// Save the current interlacing mode.
	switch (interlace)
	{

		case GRAPH_MODE_INTERLACED:		sprintf(config, "GRAPH_MODE_NONINTERLACED:");	break;
		case GRAPH_MODE_NONINTERLACED:	sprintf(config, "GRAPH_MODE_INTERLACED:");		break;
		default:						sprintf(config, "GRAPH_MODE_INTERLACED:");		break;

	}

	// Save the current field mode.
	switch (ffmd)
	{

		case GRAPH_MODE_FIELD:	sprintf(config, "GRAPH_MODE_FIELD:");		break;
		case GRAPH_MODE_FRAME:	sprintf(config, "GRAPH_MODE_FRAME:");		break;
		default:					sprintf(config, "GRAPH_MODE_FIELD:");	break;

	}

	// Save status of flicker filter.
	switch (flicker_filter)
	{

		case GRAPH_DISABLE:	sprintf(config, "GRAPH_DISABLE:");		break;
		case GRAPH_ENABLE:	sprintf(config, "GRAPH_ENABLE:");		break;
		default:				sprintf(config, "GRAPH_DISABLE:");	break;

	}

	// Save the current screen offset.
	sprintf(config, "%d:",x);
	sprintf(config, "%d:",y);

	// End function.
	return 0;

}

int graph_set_config(char *config)
{
	char *temp0, *temp1;
	int mode, interlace, ffmd;
	int flicker_filter, x, y;

	// Extract the mode config value.
	temp0 = config; temp1 = strtok(temp0, ":"); temp0 += strlen(temp1) + 1;

	// Parse the mode config value.
	if (strcmp(temp1, "GRAPH_MODE_AUTO"        ) == 0) { mode = GRAPH_MODE_AUTO;        } else
	if (strcmp(temp1, "GRAPH_MODE_NTSC"        ) == 0) { mode = GRAPH_MODE_NTSC;        } else
	if (strcmp(temp1, "GRAPH_MODE_PAL"         ) == 0) { mode = GRAPH_MODE_PAL;         } else
	if (strcmp(temp1, "GRAPH_MODE_HDTV_480P"   ) == 0) { mode = GRAPH_MODE_HDTV_480P;   } else
	if (strcmp(temp1, "GRAPH_MODE_HDTV_576P"   ) == 0) { mode = GRAPH_MODE_HDTV_576P;   } else
	if (strcmp(temp1, "GRAPH_MODE_HDTV_720P"   ) == 0) { mode = GRAPH_MODE_HDTV_720P;   } else
	if (strcmp(temp1, "GRAPH_MODE_HDTV_1080I"  ) == 0) { mode = GRAPH_MODE_HDTV_1080I;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_640_60"  ) == 0) { mode = GRAPH_MODE_VGA_640_60;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_640_72"  ) == 0) { mode = GRAPH_MODE_VGA_640_72;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_640_75"  ) == 0) { mode = GRAPH_MODE_VGA_640_75;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_640_85"  ) == 0) { mode = GRAPH_MODE_VGA_640_85;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_800_56"  ) == 0) { mode = GRAPH_MODE_VGA_800_56;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_800_60"  ) == 0) { mode = GRAPH_MODE_VGA_800_60;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_800_72"  ) == 0) { mode = GRAPH_MODE_VGA_800_72;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_800_75"  ) == 0) { mode = GRAPH_MODE_VGA_800_75;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_800_85"  ) == 0) { mode = GRAPH_MODE_VGA_800_85;  } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_1024_60" ) == 0) { mode = GRAPH_MODE_VGA_1024_60; } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_1024_70" ) == 0) { mode = GRAPH_MODE_VGA_1024_70; } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_1024_75" ) == 0) { mode = GRAPH_MODE_VGA_1024_75; } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_1024_85" ) == 0) { mode = GRAPH_MODE_VGA_1024_85; } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_1280_60" ) == 0) { mode = GRAPH_MODE_VGA_1280_60; } else
	if (strcmp(temp1, "GRAPH_MODE_VGA_1280_75" ) == 0) { mode = GRAPH_MODE_VGA_1280_75; }
	else
	{

		mode = GRAPH_MODE_AUTO;

	}

	// Read the interlace config value.
	temp1 = strtok(temp0, ":"); temp0 += strlen(temp1) + 1;

	// Parse the interlace config value.
	if (strcmp(temp1, "GRAPH_MODE_NONINTERLACED" ) == 0) { interlace = GRAPH_MODE_NONINTERLACED; } else
	if (strcmp(temp1, "GRAPH_MODE_INTERLACED" ) == 0) { interlace = GRAPH_MODE_INTERLACED; }
	else
	{

		interlace = GRAPH_MODE_INTERLACED;

	}

	// Read the field mode config value.
	temp1 = strtok(temp0, ":"); temp0 += strlen(temp1) + 1;

	// Parse the field mode config value.
	if (strcmp(temp1, "GRAPH_MODE_FRAME" ) == 0) { ffmd = GRAPH_MODE_FRAME; } else
	if (strcmp(temp1, "GRAPH_MODE_FIELD" ) == 0) { ffmd = GRAPH_MODE_FIELD; }
	else
	{

		ffmd = GRAPH_MODE_FIELD;

	}

	// Read the flicker_filter config value.
	temp1 = strtok(temp0, ":"); temp0 += strlen(temp1) + 1;

	// Parse the flicker_filter config value.
	if (strcmp(temp1, "GRAPH_DISABLE" ) == 0) { flicker_filter = GRAPH_DISABLE; } else
	if (strcmp(temp1, "GRAPH_ENABLE"  ) == 0) { flicker_filter = GRAPH_ENABLE;  }
	else
	{

		flicker_filter = GRAPH_ENABLE;

	}

	// Disable flicker filter if the mode is noninterlaced.
	if (interlace == GRAPH_MODE_NONINTERLACED)
	{

		flicker_filter = GRAPH_DISABLE;

	}

	// Read the x config value.
	temp1 = strtok(temp0, ":"); temp0 += strlen(temp1) + 1;

	// Parse the x config value.
	x = (int)strtol(temp1,NULL,10);

	// Read the y config value.
	temp1 = strtok(temp0, ":"); temp0 += strlen(temp1) + 1;

	// Parse the y config value.
	y = (int)strtol(temp1,NULL,10);

	graph_set_mode(interlace,mode,ffmd,flicker_filter);
	// End function.
	return 0;

}

int graph_load_config(char *filename)
{

	FILE *infile; char config[512];

	// Open the config file.
	if ((infile = fopen(filename, "r")) < 0)
	{

		return -1;

	}

	// Read the config file contents.
	if (fread(config, 1, sizeof(config), infile) < 0)
	{

		return -1;

	}

	// Close the config file.
	if (fclose(infile) < 0)
	{

		return -1;

	}

	// Set the current mode config information.
	return graph_set_config(config);

}

int graph_save_config(char *filename)
{

	FILE *outfile; char config[512];

	// Get the current mode config information.  
	graph_get_config(config);

	// Open the config file.
	if ((outfile = fopen(filename, "w")) < 0)
	{

		return -1;

	}

	// Write the config file contents.
	if (fwrite(config, 1, strlen(config), outfile) < 0)
	{

		return -1;

	}

	// Close the config file.
	if (fclose(outfile) < 0)
	{

		return -1;

	}

	// End function.
	return 0;

}
