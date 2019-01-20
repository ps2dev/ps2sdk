#include <draw.h>
#include <draw3d.h>

#include <gif_tags.h>
#include <gs_gp.h>

#include <stdio.h>
#include <string.h>

#include <font.h>

// Single byte fonts have only a single table whose offset starts after type.
typedef struct {
	/** "FONTX2" id Identifier */
	char id[7];
	/** Name of the font */
	char name[9];
	/** Font Width XSize */
	unsigned char width;
	/** Font Height YSize */
	unsigned char height;
	/** Type of Font */
	unsigned char type;
	// Single-byte font headers end here
	/** Number of tables */
	unsigned char table_num;
	struct {
		/** Table[n] starting position */
		unsigned short start;
		/** Table[n] ending position */
		unsigned short end;
	} block[];
} fontx_hdr;

static prim_t charprim =
{
	PRIM_POINT, PRIM_SHADE_FLAT, DRAW_DISABLE,
	DRAW_DISABLE, DRAW_ENABLE, DRAW_DISABLE,
	PRIM_MAP_ST, PRIM_UNFIXED
};

// These are the SJIS table ranges for character lookup
unsigned short sjis_table[] = {
0x8140,0x817e,
0x8180,0x81ac,
0x81b8,0x81bf,
0x81c8,0x81ce,
0x81da,0x81e8,
0x81f0,0x81f7,
0x81fc,0x81fc,
0x824f,0x8258,
0x8260,0x8279,
0x8281,0x829a,
0x829f,0x82f1,
0x8340,0x837e,
0x8380,0x8396,
0x839f,0x83b6,
0x83bf,0x83d6,
0x8440,0x8460,
0x8470,0x847e,
0x8480,0x8491,
0x849f,0x84be,
0x889f,0x88fc,
0x8940,0x897e,
0x8980,0x89fc,
0x8a40,0x8a7e,
0x8a80,0x8afc,
0x8b40,0x8b7e,
0x8b80,0x8bfc,
0x8c40,0x8c7e,
0x8c80,0x8cfc,
0x8d40,0x8d7e,
0x8d80,0x8dfc,
0x8e40,0x8e7e,
0x8e80,0x8efc,
0x8f40,0x8f7e,
0x8f80,0x8ffc,
0x9040,0x907e,
0x9080,0x90fc,
0x9140,0x917e,
0x9180,0x91fc,
0x9240,0x927e,
0x9280,0x92fc,
0x9340,0x937e,
0x9380,0x93fc,
0x9440,0x947e,
0x9480,0x94fc,
0x9540,0x957e,
0x9580,0x95fc,
0x9640,0x967e,
0x9680,0x96fc,
0x9740,0x977e,
0x9780,0x97fc,
0x9840,0x9872
};

int fontx_load_single_krom(fontx_t *fontx)
{

	fontx_hdr *fontx_header;

	int header_size = 17;
	int char_size = 15;

	int fd = 0;
	int size;

	fd = fioOpen("rom0:KROM", O_RDONLY);

	if (fd < 0)
	{

		printf("Error opening KROM font.\n");
		return -1;

	}

	// header without table pointer + size of a character * 256 characters
	size = header_size + char_size * 256;

	fontx->font = (char*)malloc(size);

	if (fontx->font == NULL)
	{

		printf("Error allocating %d bytes of memory.\n", size);
		fioClose(fd);

		return -1;
	}

	// Clear the memory
	memset(fontx->font,0,size);

	// The offset for the ASCII characters
	fioLseek(fd, 0x198DE, SEEK_SET);

	// 17 bytes of header and 15 bytes per 33 characters
	// Read in 95 characters
	if (fioRead(fd,fontx->font + header_size+char_size*33, char_size*95) < 0)
	{

		printf("Error reading rom0:KROM.\n");

		free(fontx->font);
		fioClose(fd);

		return -1;

	}

	fioClose(fd);

	fontx_header = (fontx_hdr*)fontx->font;

	// define header as single-byte font
	strncpy(fontx_header->id, "FONTX2", 6);
	fontx_header->id[6] = '\0';
	strncpy(fontx_header->name, "KROM", 8);
	fontx_header->name[8] = '\0';

	fontx_header->width = 8;
	fontx_header->height = 15;
	fontx_header->type = SINGLE_BYTE;

	// Save it as a font
	//fd=fioOpen("host:KROM_ascii.fnt",O_WRONLY | O_TRUNC | O_CREAT);
	//fioWrite(fd, fontx->font, size);
	//fioClose(fd);

	return 0;

}

int fontx_load_double_krom(fontx_t *fontx)
{

	fontx_hdr *fontx_header;

	int size;
	int fd = 0;

	// Font characteristics for double-byte font
	int header_size = 18;
	int table_num = 51;
	int table_size = 4;
	int char_size = 30;
	int char_num = 3489;

	fd = fioOpen("rom0:KROM", O_RDONLY);

	if (fd < 0)
	{

		printf("Error opening KROM font.\n");

	}

	size = header_size + table_num*table_size +  char_num*char_size;

	fontx->font = (char*)malloc(size);

	if (fontx->font == NULL)
	{

		printf("Error allocating memory.\n");
		fioClose(fd);

		return -1;
	}

	// Clear memory
	memset(fontx->font,0,size);

	// Make sure we're at the beginning
	fioLseek(fd, 0, SEEK_SET);

	// Read in 95 characters
	if (fioRead(fd, fontx->font+header_size+table_num*table_size, char_size*char_num) < 0)
	{

		printf("Error reading font.\n");
		free(fontx->font);
		fioClose(fd);

		return -1;

	}

	fioClose(fd);

	fontx_header = (fontx_hdr*)fontx->font;

	// define the header as double-byte font
	strncpy(fontx_header->id, "FONTX2", 6);
	fontx_header->id[6] = '\0';
	strncpy(fontx_header->name, "KROM", 8);
	fontx_header->name[8] = '\0';

	fontx_header->width = 16;
	fontx_header->height = 15;
	fontx_header->type = DOUBLE_BYTE;
	fontx_header->table_num = table_num;

	// Add the SJIS tables to the font
	memcpy(fontx->font+header_size,sjis_table,table_num*table_size);

	// Save it as a font
	//fd=fioOpen("host:KROM_kanji.fnt",O_WRONLY | O_TRUNC | O_CREAT);
	//fioWrite(fd, fontx->font, size);
	//fioClose(fd);

	return 0;

}

int fontx_load(const char *path, fontx_t* fontx, int type, int wmargin, int hmargin, int bold)
{

	FILE *file = NULL;

	int ret = -1;
	long size = 0;

	fontx_hdr *fontx_header = NULL;

	if (!strcmp("rom0:KROM",path) || !strcmp("rom0:/KROM",path))
	{

		if (type == SINGLE_BYTE)
		{

			ret = fontx_load_single_krom(fontx);

		}
		else
		{

			ret = fontx_load_double_krom(fontx);

		}

		if (ret < 0)
		{

			printf("Error opening %s\n", path);
			return -1;

		}

	}

	else
	{

		file = fopen(path, "r");

		if (file == NULL)
		{

			printf("Error opening %s\n", path);
			return -1;

		}

		// get size of file
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);

		fontx->font = (char *)malloc(size);

		if (fontx->font == NULL)
		{

			printf("Error allocating %ld bytes of memory.\n", size);
			fclose(file);
			return -1;

		}

		fread(fontx->font, size, 1, file);

		fclose(file);

	}

	fontx_header = (fontx_hdr*)fontx->font;

	if (strncmp(fontx_header->id, "FONTX2", 6) != 0)
	{

		printf("Not FONTX2 type font!\n");
		free(fontx->font);

		return -1;

	}

	if (fontx_header->type != type)
	{

		printf("Type mismatch\n");
		free(fontx->font);

		return -1;

	}

	// Fill in some information about the font
	strcpy(fontx->name,fontx_header->name);

	fontx->rowsize = ((fontx_header->width+7)>>3);
	fontx->charsize = fontx->rowsize * fontx_header->height;
	fontx->w_margin = wmargin;
	fontx->h_margin = hmargin;
	fontx->bold = bold;

	// This is the offset that character data starts
	if (fontx_header->type == SINGLE_BYTE)
	{

		fontx->offset = 17;

	}
	else
	{

		// 17 + 1 + (number of tables * 4) bytes
		fontx->offset = 18 + (fontx_header->table_num * 4);

	}

	return 0;

}

void fontx_unload(fontx_t *fontx)
{

	if (fontx->font != NULL)
	{

		free(fontx->font);

	}

}

char *fontx_get_char(fontx_t* fontx, unsigned short c)
{

	unsigned int i;

	int table = -1;
	int table_offset = 0;

	fontx_hdr *fontx_header;

	if (fontx->font == NULL)
	{

		printf("Font data not loaded.\n");
		return NULL;

	}

	fontx_header = (fontx_hdr*)fontx->font;

	if ((fontx_header->type == SINGLE_BYTE) && (c <= 0xFF))
	{

		return (fontx->font + (fontx->offset + c * fontx->charsize));

	}

	for (i = 0; i < fontx_header->table_num; i++)
	{

		if ((fontx_header->block[i].start <= c) && (fontx_header->block[i].end >= c))
		{

				table = i;
				break;

		}
	}

	// If table not found
	if (table < 0)
	{

		return NULL;

	}

	for (i = 0; i < table; i++)
	{

		table_offset += fontx_header->block[i].end - fontx_header->block[i].start;

	}

	table_offset = table_offset + table + ( c - fontx_header->block[table].start);

	return (fontx->font + (fontx->offset + table_offset*fontx->charsize));

}

// draws a single byte
u64 *draw_fontx_row(u64 *dw, unsigned char byte, int x, int y, int z, int bold)
{

	unsigned char mask = 0x80;

	int i;
	int px = 0;

	// for each bit in a row
	for (i=0;i<8;i++)
	{

		// if there's a bit
		if (byte & mask)
		{

			*dw++ = GS_SET_XYZ((x+(i<<4)) + 32768,y+32768,z);
			px = 1;

		}
		else
		{

			if (bold && px)
				*dw++ = GS_SET_XYZ((x+(i<<4)) + 32768,y+32768,z);

			px = 0;

		}

		mask >>= 1;

	}

	// Add some boldness to the last pixel in a row
	if (bold && px)
	{

		*dw++ = GS_SET_XYZ((x+(i<<4)) + 32768,y+32768,z);

	}

	return dw;

}

qword_t *draw_fontx_char(qword_t *q, unsigned short c, vertex_t *v0, fontx_t *fontx)
{

	u64 pdw;
	u64 *dw = (u64 *)q;

	char *char_offset;
	int i, j;
	int x,y,z;
	int xi,yi;

	x = ftoi4(v0->x);
	y = ftoi4(v0->y);
	z = v0->z;

	fontx_hdr* fontx_header = (fontx_hdr*)fontx->font;

	char_offset = fontx_get_char(fontx,c);

	if (!char_offset)
	{

		return q;

	}

	for (i=0;i<fontx_header->height;i++)
	{

		// Increment one row down
		yi = y + (i << 4);

		for (j=0;j < fontx->rowsize;j++)
		{

			// Increment one row right
			xi = x + (j << 7);
			dw = draw_fontx_row(dw,char_offset[(i*fontx->rowsize) + j],xi,yi,z,fontx->bold);

		}

	}

	if ((u32)dw % 16)
	{

		pdw = *(dw-1);
		*dw++ = pdw;

	}

	q = (qword_t*)dw;

	return q;

}

qword_t *fontx_print_ascii(qword_t *q, int context, const unsigned char *str, int alignment, vertex_t *v0, color_t *c0, fontx_t *fontx)
{

	int i,j;

	fontx_hdr *fontx_header = (fontx_hdr*)fontx->font;

	vertex_t v_pos = *v0;

	int length = strlen((const char *)str);
	short line_num[100];
	int line = 0;

	float x_orig[100];
	x_orig[0] = 0;

	float w = fontx_header->width;
	float h = fontx_header->height;

	float wm = fontx->w_margin;
	float hm = fontx->h_margin;

	// line_num is used to keep track of number of characters per line
	line_num[0] = 0;

	switch (alignment)
	{
		default:
		case LEFT_ALIGN:
		{
			for (i = 0; i < length; i++)
			{

				while (str[i] == '\t' || str[i] == '\n')
				{

					if(str[i] == '\n')
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

				while (str[i] == '\t' || str[i] == '\n')
				{

					if (str[i] == '\t')
					{
						line_num[line] += 4;
					}

					if (str[i] == '\n')
					{
						x_orig[line] = v_pos.x - (line_num[line] * (w + wm));
						line++;
						line_num[line] = 0;
					}

					i++;

				}


				line_num[line]++;

				if (i == length-1)
				{
					x_orig[line] = v_pos.x - (line_num[line] * (w + wm));
					line++;
				}

			}
			break;

		}
		case CENTER_ALIGN:
		{
			for (i = 0; i < length; i++)
			{

				while (str[i] == '\t' || str[i] == '\n')
				{

					if (str[i] == '\t')
					{
						line_num[line] += 4;
					}

					if (str[i] == '\n')
					{
						x_orig[line] = v_pos.x - (line_num[line] * (w + wm))/2.0f;
						line++;
						line_num[line] = 0;
					}

					i++;

				}

				line_num[line]++;

				if (i == length-1)
				{
					x_orig[line] = v_pos.x - (line_num[line] * (w + wm))/2.0f;
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

		while(str[j] == '\n' || str[j] == '\t')
		{
			if (str[j] == '\n')
			{
				line++;
				v_pos.y += h + hm;
				v_pos.x = x_orig[line];
			}
			if (str[j] == '\t')
			{
				v_pos.x += w * 5.0f;
			}
			j++;
		}

		if (str[j] < 0x80)
		{
			q = draw_fontx_char(q,str[j],&v_pos,fontx);
		}
		else if (str[j] >= 0xA1 && str[j] <= 0xDF)
		{
			q = draw_fontx_char(q,str[j],&v_pos,fontx);
		}

		v_pos.x += w + wm;

	}

	q = draw_prim_end(q,2,DRAW_XYZ_REGLIST);

	return q;

}

qword_t *fontx_print_sjis(qword_t *q, int context, const unsigned char *str, int alignment, vertex_t *v0, color_t *c0, fontx_t *ascii, fontx_t *kanji)
{

	int i,j;

	fontx_hdr *ascii_header = (fontx_hdr*)ascii->font;
	fontx_hdr *kanji_header = (fontx_hdr*)kanji->font;

	vertex_t v_pos = *v0;

	int length = strlen((const char *)str);

	int line = 0;
	short halfwidth[100];
	short fullwidth[100];
	float x_orig[100];
	x_orig[0] = 0;

	unsigned short wide;

	float hw = ascii_header->width;

	float fw = kanji_header->width;
	float h = kanji_header->height;

	float wm = kanji->w_margin;
	float hm = kanji->h_margin;

	// line_num is used to keep track of number of characters per line
	halfwidth[0] = 0;
	fullwidth[0] = 0;

	switch (alignment)
	{
		default:
		case LEFT_ALIGN:
		{
			for (i = 0; i < length; i++)
			{

				while (str[i] == '\t'|| str[i] == '\n')
				{
					if (str[i] == '\t')
					{
						halfwidth[line] += 4;
					}
					if (str[i] == '\n')
					{
						x_orig[line] = v_pos.x;
						line++;
						halfwidth[line] = 0;
						fullwidth[line] = 0;
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

				while (str[i] == '\t'|| str[i] == '\n')
				{
					if (str[i] == '\t')
					{
						halfwidth[line] += 4;
					}
					if (str[i] == '\n')
					{
						x_orig[line] = v_pos.x - ((halfwidth[line] * (hw+wm)) + (fullwidth[line] * (fw + wm)));;
						line++;
						halfwidth[line] = 0;
						fullwidth[line] = 0;
					}
					i++;
				}

				if (str[i] < 0x80)
				{
					halfwidth[line]++;
				}
				else if (str[i] >= 0xA1 && str[i] <= 0xDF)
				{
					halfwidth[line]++;
				}
				else if (str[i] >= 0x81 && str[i] <= 0x9F)
				{
					fullwidth[line]++;
				}
				else if (str[i] >= 0xE0 && str[i] <= 0xEF)
				{
					fullwidth[line]++;
				}

				if (i == length-1)
				{
					x_orig[line] = v_pos.x - ((halfwidth[line] * (hw+wm)) + (fullwidth[line] * (fw + wm)));
					line++;
				}

			}
			break;

		}
		case CENTER_ALIGN:
		{
			for (i = 0; i < length; i++)
			{

				while (str[i] == '\t'|| str[i] == '\n')
				{
					if (str[i] == '\t')
					{
						halfwidth[line] += 4;
					}
					if (str[i] == '\n')
					{
						x_orig[line] = v_pos.x - ((halfwidth[line] * (hw+wm)) + (fullwidth[line] * (fw + wm)))/2.0f;
						line++;
						halfwidth[line] = 0;
						fullwidth[line] = 0;
					}
					i++;
				}

				if (str[i] < 0x80)
				{
					halfwidth[line]++;
				}
				else if (str[i] >= 0xA1 && str[i] <= 0xDF)
				{
					halfwidth[line]++;
				}
				else if (str[i] >= 0x81 && str[i] <= 0x9F)
				{
					fullwidth[line]++;
				}
				else if (str[i] >= 0xE0 && str[i] <= 0xEF)
				{
					fullwidth[line]++;
				}

				if (i == length-1)
				{
					x_orig[line] = v_pos.x - ((halfwidth[line] * (hw+wm)) + (fullwidth[line] * (fw + wm)))/2.0f;
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

		wide = 0;

		while(str[j] == '\n' || str[j] == '\t')
		{
			if (str[j] == '\n')
			{
				line++;
				v_pos.y += h + hm;
				v_pos.x = x_orig[line];
			}
			if (str[j] == '\t')
			{
				v_pos.x += hw * 5.0f;
			}
			j++;
		}

		if (str[j] < 0x80)
		{
			q = draw_fontx_char(q,str[j],&v_pos,ascii);
			v_pos.x += hw + wm;
		}
		else if (str[j] >= 0xA1 && str[j] <= 0xDF)
		{
			q = draw_fontx_char(q,str[j],&v_pos,ascii);
			v_pos.x += hw + wm;
		}
		else if (str[j] >= 0x81 && str[j] <= 0x9F)
		{
			wide = str[j++]<<8;
			wide += str[j];
			q = draw_fontx_char(q,wide,&v_pos,kanji);
			v_pos.x += fw + wm;
		}
		else if (str[j] >= 0xE0 && str[j] <= 0xEF)
		{
			wide = str[j++]<<8;
			wide += str[j];
			q = draw_fontx_char(q,wide,&v_pos,kanji);
			v_pos.x += fw + wm;
		}
		else
		{
			v_pos.x += fw + wm;
		}

	}

	q = draw_prim_end(q,2,DRAW_XYZ_REGLIST);

	return q;

}
