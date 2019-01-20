#include <stdio.h>
#include <string.h>

#include <draw.h>
#include <draw3d.h>

#include <font.h>

static prim_t charprim =
{
	PRIM_SPRITE, PRIM_SHADE_FLAT, DRAW_ENABLE,
	DRAW_DISABLE, DRAW_ENABLE, DRAW_DISABLE,
	PRIM_MAP_UV, PRIM_UNFIXED
};

#define TAB '\t'
#define NEWLINE '\n'
#define SPACE ' '

fsfont_t *fontstudio_init( int char_height)
{

	fsfont_t *font;

	font = (fsfont_t*)malloc(sizeof(fsfont_t));

	font->height = char_height;
	font->scale = 1.0f;

	return font;

}

void fontstudio_free(fsfont_t *font)
{

	if (font->charmap != NULL)
	{
		free(font->charmap);
	}

	if (font->chardata != NULL)
	{
		free(font->chardata);
	}

	free(font);

}

char *fontstudio_load_ini(const char *path)
{

	FILE *file;

	char *ini;
	int size;

	file = fopen(path, "r");

	if (file == NULL)
	{

		printf("Error opening %s.\n", path);
		return NULL;

	}

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	ini = (char *)malloc(size);

	if (ini == NULL)
	{
		printf("Error allocated %d bytes of memory.\n", size);
		fclose(file);
		return NULL;
	}

	fread(ini, size, 1, file);
	fclose(file);

	return ini;

}

int fontstudio_parse_ini(fsfont_t *font, char *ini, float tex_width, float tex_height)
{

	int i;

	char *temp0;
	char *temp1;

	temp0 = ini;

	temp1 = strtok(temp0,"=");
	if (temp1 == NULL)
	{
		printf("Error parsing number of chars.\n");
		return -1;
	}
	temp0 += strlen(temp1)+1;

	font->totalchars = (int)strtol(temp0,NULL,10);

	temp1 = strtok(temp0,"=");
	if (temp1 == NULL)
	{
		printf("Error parsing space width.\n");
		return -1;
	}
	temp0 += strlen(temp1)+1;

	font->spacewidth = (int)strtol(temp0,NULL,10);

	font->charmap = (unsigned short*)malloc(sizeof(short)*font->totalchars);

	if (font->charmap == NULL)
	{

		//131 kilobytes of memory
		printf("Error allocated %d bytes of memory.\n", sizeof(short)*font->totalchars);
		return -1;

	}

	// Clear memory
	memset(font->charmap,0,sizeof(short)*font->totalchars);

	font->chardata = (inidata_t*)malloc(sizeof(inidata_t)*font->totalchars);

	if (font->chardata == NULL)
	{

		printf("Error allocating %d bytes of memory.\n", sizeof(inidata_t)*font->totalchars);
		free(font->charmap);
		return -1;

	}

	// Clear memory
	memset(font->chardata,0,sizeof(inidata_t)*font->totalchars);

	for (i = 0; i < font->totalchars; i++)
	{

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing Char for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->charmap[i] = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing A for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].A = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing B for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].B = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing C for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].C = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing ox for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].ox = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing oy for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].oy = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing Wid for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].width = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing Hgt for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].height = (int)strtol(temp0,NULL,10);

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing X1 for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].u1 = ftoi4(((float)(strtod(temp0,NULL) * tex_width)));

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing Y1 for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].v1 = ftoi4(((float)(strtod(temp0,NULL) * tex_height)));

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing X2 for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].u2 = ftoi4(((float)(strtod(temp0,NULL) * tex_width)));

		temp1 = strtok(temp0,"=");
		if (temp1 == NULL)
		{

			printf("Error parsing Y2 for char %d.\n", i);
			free(font->charmap);
			free(font->chardata);
			return -1;

		}
		temp0 += strlen(temp1)+1;
		font->chardata[i].v2 = ftoi4(((float)(strtod(temp0,NULL) * tex_height)));
	}

	return 0;

}

void fontstudio_unload_ini(fsfont_t *font)
{
	if(font->charmap != NULL)
	{
		free(font->charmap);
		font->charmap = NULL;
	}
	if(font->chardata != NULL)
	{
		free(font->chardata);
		font->chardata = NULL;
	}
}

// Decode unicode byte sequences into unicode a single numerical character U+XXXX
// Returns the number of actual unicode characters
int decode_unicode(const unsigned char *in, unsigned short *out)
{

	// 0x00 - 0x7f	- single byte ascii
	// 0x80 - 0xbf	- continuation bytes
	// 0xC2 - 0xDF	- start of 2 byte sequence (1 byte after)
	//				 first_byte & 0x1F << 6 + second_byte & 0x3f = index
	// 0xE0 - 0xEF	- start of 3 byte sequence (2 bytes after)
	//				 first_byte & 0xF << 12 + second_byte & 0x3f << 6 + third_byte & 0x3f
	// 0xF0 - 0xF4	- start of 4 byte sequence (3 bytes after)
	//				 first_byte & 0x7 << 18 + second_byte & 0x3f << 12 + third_byte & 0x3f << 6 + fourth_byte & 0x3f

	int i;

	int j = 0;

	int length = strlen((const char *)in);

	for (i = 0; i < length; i++)
	{
		if (in[i] < 0x80)
		{
			out[j++] = in[i];
		}

		if (in[i] > 0xC1)
		{
			if (in[i] < 0xE0)
			{
				out[j] = (in[i++] & 0x1f)<<6;
				out[j++] += (in[i] & 0x3f);
			}
			else if ((in[i] > 0xDF) && (in[i] < 0xF0))
			{
				out[j] = (in[i++] & 0xF)<<12;
				out[j] += (in[i++] & 0x3f)<<6;
				out[j++] += (in[i] & 0x3f);
			}
			else if ((in[i] > 0xEF) && (in[i] < 0xF5))
			{
				// Supports only up to U+FFFF
				i+=3;
			}
		}
	}

	return j;

}

unsigned short get_char(unsigned short c, fsfont_t *font)
{

	unsigned short i;

	for (i = 0; i < font->totalchars; i++)
	{

		if (c == font->charmap[i])
		{
			return i;

		}

	}

	// Use unknown <?> character if character isn't in character map
	return 0xFFFD;

}

void convert_to_index(unsigned short *in, int num, fsfont_t *font)
{

	int i;

	for (i = 0; i < num; i++)
	{
		// These characters aren't included in the FontStudio index, I think
		while (in[i] == '\n' || in[i] == '\t' || in[i] == ' ')
		{
			/*
			if (in[i] == '\n')
			{
				in[i] = NEWLINE;
			}
			if (in[i] == '\t')
			{
				in[i] = TAB;
			}
			if (in[i] == ' ')
			{
				in[i] = SPACE;
			}
			*/
			i++;
		}

		in[i] = get_char(in[i],font);
	}

}

#define DRAW_ST_REGLIST \
	((u64)GIF_REG_ST)   << 0 | \
	((u64)GIF_REG_XYZ2) << 4

qword_t *draw_fontstudio_char(qword_t *q, unsigned int c, vertex_t *v0, fsfont_t *font)
{

	int x,y;

	x = ftoi4(v0->x);
	y = ftoi4(v0->y);

	q->dw[0] = GIF_SET_UV(font->chardata[c].u1,font->chardata[c].v1);
	q->dw[1] = GIF_SET_XYZ(x + 32759,y + 32759,v0->z);
	q++;

	q->dw[0] = GIF_SET_UV(font->chardata[c].u2, font->chardata[c].v2);
	q->dw[1] = GIF_SET_XYZ(x + (int)((font->chardata[c].width*font->scale)*16.0f) + 32777,y + (int)((font->chardata[c].height*font->scale)*16.0f) + 32777,v0->z);
	q++;

	return q;

}

qword_t *fontstudio_print_string(qword_t *q, int context, const unsigned char *str, int alignment, vertex_t *v0, color_t *c0, fsfont_t *font)
{

	int i = 0,j;

	unsigned short curchar = 0;

	int length;

	vertex_t v_pos = *v0;

	unsigned short utf8[2048];

	memset(utf8,0,sizeof(short)*2048);

	// Decodes the encoded string into unicode numbers U+xxxx
	// length is the number of characters in the string
	length = decode_unicode(str, utf8);

	//for (i = 0; i < length; i++)
	//{
	//	printf("utf8[%d] = %d\n", i, utf8[i]);
	//}

	// Converts the unicode numbers into the index numbers
	// used by the FontStudio ini
	convert_to_index(utf8,length,font);

	//for (i = 0; i < length; i++)
	//{
	//	printf("utf8[%d] = %d\n", i, utf8[i]);
	//}

	float line_num[100];
	int line = 0;

	float x_orig[100];
	x_orig[0] = 0;

	// line_num is used to keep track of number of characters per line
	line_num[0] = 0;

	switch (alignment)
	{
		default:
		case LEFT_ALIGN:
		{
			for (i = 0; i < length; i++)
			{

				while (utf8[i] == TAB || utf8[i] == NEWLINE)
				{
					if (utf8[i] == NEWLINE)
					{
						x_orig[line] = v_pos.x;
						line++;
						line_num[line] = 0;
					}
					i++;
				}

				if (i == length-1)
				{
					x_orig[line] = v_pos.x;
					line++;
				}

			}
			break;

		}
		case RIGHT_ALIGN:
		{
			for (i = 0; i < length; i++)
			{

				while (utf8[i] == TAB || utf8[i] == NEWLINE || utf8[i] == SPACE)
				{
					if (utf8[i] == NEWLINE)
					{
						x_orig[line] = v_pos.x - line_num[line]*font->scale;
						line++;
						line_num[line] = 0;
					}
					if (utf8[i] == TAB)
					{
						line_num[line] += font->spacewidth * 4;
					}
					if (utf8[i] == SPACE)
					{
						line_num[line] += font->spacewidth;
					}
					i++;
				}

				curchar = utf8[i];
				line_num[line] += font->chardata[curchar].A + font->chardata[curchar].B + font->chardata[curchar].C;

				if (i == length-1)
				{
					x_orig[line] = v_pos.x - line_num[line]*font->scale;
					line++;
				}

			}
			break;

		}
		case CENTER_ALIGN:
		{
			for (i = 0; i < length; i++)
			{

				while (utf8[i] == TAB || utf8[i] == NEWLINE || utf8[i] == SPACE)
				{
					if (utf8[i] == NEWLINE)
					{
						x_orig[line] = v_pos.x - (line_num[line]*font->scale)/2.0f;
						line++;
						line_num[line] = 0;
					}
					if (utf8[i] == TAB)
					{
						line_num[line] += font->spacewidth * 4;
					}
					if (utf8[i] == SPACE)
					{
						line_num[line] += font->spacewidth;
					}
					i++;
				}

				curchar = utf8[i];
				line_num[line] += font->chardata[curchar].A + font->chardata[curchar].B + font->chardata[curchar].C;

				if (i == length-1)
				{
					x_orig[line] = v_pos.x - (line_num[line]*font->scale)/2.0f;
					line++;
				}

			}
			break;

		}

	};

	line = 0;
	v_pos.x = x_orig[0];

	q = draw_prim_start(q,context,&charprim,c0);

	for (j = 0; j < length; j++)
	{

		while(utf8[j] == NEWLINE || utf8[j] == TAB || utf8[j] == SPACE)
		{
			if (utf8[j] == NEWLINE)
			{
				line++;
				v_pos.y += font->height*font->scale;
				v_pos.x = x_orig[line];
			}
			if (utf8[j] == TAB)
			{
				v_pos.x += font->spacewidth*font->scale * 4.0f;
			}
			if (utf8[j] == SPACE)
			{
				v_pos.x += font->spacewidth*font->scale;
			}
			j++;
		}

		v_pos.x += (font->chardata[utf8[j]].A*font->scale);

		q = draw_fontstudio_char(q,utf8[j],&v_pos,font);

		v_pos.x += (font->chardata[utf8[j]].B*font->scale) + (font->chardata[utf8[j]].C*font->scale);


	}

	q = draw_prim_end(q,2,DRAW_UV_REGLIST);

	return q;
}

