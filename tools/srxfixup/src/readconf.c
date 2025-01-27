/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "srxfixup_internal.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct lowtoken_
{
	const char *str;
	int line;
	int col;
} LowToken;
enum TokenCode
{
	TC_NULL = 0,
	TC_STRING = 1,
	TC_VECTOR = 2,
	TC_IOP = 3,
	TC_EE = 4,
	TC_Define = 5,
	TC_Segments_name = 6,
	TC_Memory_segment = 7,
	TC_remove = 8,
	TC_Program_header_order = 9,
	TC_Program_header_data = 10,
	TC_Segment_data = 11,
	TC_segment = 12,
	TC_createinfo = 13,
	TC_Section_header_table = 14,
	TC_CreateSymbols = 15,
	TC_MAX_NUMBER = 16
};
typedef struct TokenTree_
{
	enum TokenCode tkcode;
	union TokenTree_value
	{
		struct TokenTree_ *subtree;
		LowToken *lowtoken;
	} value;
} TokenTree;
struct fstrbuf
{
	int line;
	int col;
	const char *cp;
	const char *ep;
	char buf[];
};

static int bgetc(struct fstrbuf *fb);
static void bungetc(struct fstrbuf *fb);
static int skipsp(struct fstrbuf *fb);
static int skip_to_eol(struct fstrbuf *fb);
static int gettoken(char **strbuf, struct fstrbuf *fb);
static void split_conf(LowToken *result, char *strbuf, struct fstrbuf *fb);
static TokenTree *make_conf_vector(LowToken **lowtokens);
static TokenTree *make_conf_tree(LowToken *lowtokens);
static int get_vector_len(TokenTree *ttp);
static int get_stringvector_len(const char **str);
static const char **add_stringvector(const char **str, const char *newstr);
static int setup_reserved_symbol_table(CreateSymbolConf *result, TokenTree *ttp, Srx_gen_table *conf);
static int gen_define(TokenTree *ttp, Srx_gen_table *result);
static void get_section_type_flag(TokenTree *ttp, int *rtype, int *rflag);
static elf_section *make_empty_section(const char *name, int type, int flag);
static Srx_gen_table *make_srx_gen_table(TokenTree *tokentree);
static void check_change_bit(unsigned int oldbit, unsigned int newbit, unsigned int *up, unsigned int *down);
static int check_srx_gen_table(Srx_gen_table *tp);

Srx_gen_table *read_conf(const char *indata, const char *infile, int dumpopt)
{
	LowToken *lowtokens;
	struct fstrbuf *fbuf;
	unsigned int fsize;
	FILE *fp;

	fp = 0;
	if ( !indata )
	{
		fprintf(stderr, "internal error read_conf()\n");
		return 0;
	}
	if ( infile )
	{
		fp = fopen(infile, "re");
		if ( !fp )
		{
			fprintf(stderr, "\"%s\" can't open (errno=%d)\n", infile, errno);
			return 0;
		}
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp) + 4;
		fseek(fp, 0, SEEK_SET);
	}
	else
	{
		fsize = strlen(indata);
	}
	lowtokens = (LowToken *)calloc(
		((sizeof(LowToken) + 2) * (fsize + 1) + (sizeof(LowToken) - 1)) / sizeof(LowToken), sizeof(LowToken));
	fbuf = (struct fstrbuf *)malloc(fsize + sizeof(struct fstrbuf) + 1);
	fbuf->cp = fbuf->buf;
	fbuf->line = 1;
	fbuf->col = 1;
	if ( infile )
	{
		fbuf->ep = &fbuf->cp[fread(fbuf->buf, 1, fsize, fp)];
		fclose(fp);
	}
	else
	{
		strcpy(fbuf->buf, indata);
		fbuf->ep = &fbuf->cp[fsize];
	}
	if ( dumpopt == 1 )
	{
		for ( ;; )
		{
			int ch_;

			ch_ = bgetc(fbuf);
			if ( ch_ == -1 )
			{
				break;
			}
			fputc(ch_, stdout);
		}
		free(lowtokens);
		free(fbuf);
		return 0;
	}
	TokenTree *tokentree;
	Srx_gen_table *srx_gen_table;

	split_conf(lowtokens, (char *)&lowtokens[fsize + 1], fbuf);
	free(fbuf);
	tokentree = make_conf_tree(lowtokens);
	srx_gen_table = make_srx_gen_table(tokentree);
	free(tokentree);
	if ( check_srx_gen_table(srx_gen_table) )
	{
		free(srx_gen_table);
		return 0;
	}
	return srx_gen_table;
}

static int bgetc(struct fstrbuf *fb)
{
	int ret;

	if ( fb->ep <= fb->cp )
	{
		return -1;
	}
	if ( *fb->cp == '\n' )
	{
		fb->line += 1;
		fb->col = 0;
	}
	else
	{
		fb->col += 1;
	}
	ret = (unsigned char)(*fb->cp);
	fb->cp += 1;
	return ret;
}

static void bungetc(struct fstrbuf *fb)
{
	if ( fb->cp > fb->buf )
	{
		fb->cp -= 1;
		fb->col -= 1;
		if ( *fb->cp == '\n' )
		{
			fb->line -= 1;
		}
	}
}

static int skipsp(struct fstrbuf *fb)
{
	int ch_;

	do
	{
		ch_ = bgetc(fb);
	} while ( ch_ != -1 && isspace(ch_) != 0 );
	if ( ch_ != -1 )
	{
		bungetc(fb);
	}
	return ch_ != -1;
}

static int skip_to_eol(struct fstrbuf *fb)
{
	int ch_;

	do
	{
		ch_ = bgetc(fb);
	} while ( ch_ != -1 && ch_ != '\n' && ch_ != '\r' );
	return ch_ != -1;
}

static int gettoken(char **strbuf, struct fstrbuf *fb)
{
	int ch_;
	char *cp;

	for ( cp = *strbuf;; *cp = 0 )
	{
		ch_ = bgetc(fb);
		if ( ch_ == -1 || (ch_ != '.' && ch_ != '_' && ch_ != '*' && isalnum(ch_) == 0) )
		{
			break;
		}
		*cp = (char)(ch_ & 0xFF);
		cp += 1;
	}
	if ( ch_ != -1 )
	{
		bungetc(fb);
	}
	*strbuf = cp + 1;
	return ch_ != -1;
}

static void split_conf(LowToken *result, char *strbuf, struct fstrbuf *fb)
{
	char *cp;

	cp = strbuf;
	for ( ; skipsp(fb); )
	{
		int cuchar;

		cuchar = bgetc(fb);
		if ( cuchar == '@' || cuchar == '.' || cuchar == '_' || cuchar == '*' || isalnum(cuchar) != 0 )
		{
			result->str = cp;
			result->line = fb->line;
			result->col = fb->col;
			result += 1;
			result->str = 0;
			*cp = (char)(cuchar & 0xFF);
			cp += 1;
			gettoken(&cp, fb);
		}
		else if ( cuchar == '#' )
		{
			skip_to_eol(fb);
		}
		else if ( isprint(cuchar) != 0 )
		{
			result->str = cp;
			result->line = fb->line;
			result->col = fb->col;
			result += 1;
			result->str = 0;
			*cp = (char)(cuchar & 0xFF);
			cp += 1;
			*cp = 0;
			cp += 1;
		}
	}
}

// clang-format off
struct keyword_table_
{
	int code;
	const char * name;
} keyword_table[] =
{
	{ 3, "IOP" },
	{ 4, "EE" },
	{ 5, "Define" },
	{ 6, "Segments_name" },
	{ 7, "Memory_segment" },
	{ 8, "remove" },
	{ 9, "Program_header_order" },
	{ 10, "Program_header_data" },
	{ 11, "Segment_data" },
	{ 12, "segment" },
	{ 13, "createinfo" },
	{ 14, "Section_header_table" },
	{ 15, "CreateSymbols" },
	{ -1, NULL }
};
// clang-format on

static TokenTree *make_conf_vector(LowToken **lowtokens)
{
	struct keyword_table_ *kt;
	const LowToken *sltp;
	LowToken *ltp;
	int entries;
	TokenTree *v14;

	entries = 0;
	ltp = *lowtokens;
	v14 = (TokenTree *)calloc(1, sizeof(TokenTree));
	v14->tkcode = TC_NULL;
	for ( ; ltp->str && strcmp(ltp->str, "}") != 0; )
	{
		{
			TokenTree *realloc_tmp = (TokenTree *)realloc(v14, (entries + 2) * sizeof(TokenTree));
			if ( realloc_tmp == NULL )
			{
				fprintf(stderr, "Failure to allocate token tree\n");
				exit(1);
			}
			v14 = realloc_tmp;
		}

		if ( !strcmp(ltp->str, "{") )
		{
			sltp = ltp;
			ltp += 1;
			v14[entries].tkcode = TC_VECTOR;
			v14[entries].value.subtree = make_conf_vector(&ltp);
			if ( !ltp->str || strcmp(ltp->str, "}") != 0 )
			{
				fprintf(stderr, "make_conf_vector(): missing '}' line:%d col=%d\n", sltp->line, sltp->col);
				exit(1);
			}
			ltp += 1;
		}
		else
		{
			if ( *ltp->str != '@' && *ltp->str != '.' && *ltp->str != '_' && *ltp->str != '*' && isalnum(*ltp->str) == 0 )
			{
				fprintf(stderr, "make_conf_vector(): unexcepted data '%s' line:%d col=%d\n", ltp->str, ltp->line, ltp->col);
				exit(1);
			}
			if ( *ltp->str == '@' )
			{
				for ( kt = keyword_table; kt->code >= 0 && strcmp(kt->name, ltp->str + 1) != 0; kt += 1 )
				{
					;
				}
				if ( kt->code < 0 )
				{
					fprintf(stderr, "make_conf_vector(): unknown keyword '%s' line:%d col=%d\n", ltp->str, ltp->line, ltp->col);
					exit(1);
				}
				v14[entries].tkcode = kt->code;
			}
			else
			{
				v14[entries].tkcode = TC_STRING;
			}
			v14[entries].value.subtree = (TokenTree *)ltp;
			ltp += 1;
		}
		entries += 1;
		v14[entries].tkcode = TC_NULL;
	}
	*lowtokens = ltp;
	return v14;
}

static TokenTree *make_conf_tree(LowToken *lowtokens)
{
	TokenTree *v4;

	v4 = (TokenTree *)calloc(1, sizeof(TokenTree));
	v4->tkcode = TC_VECTOR;
	v4->value.subtree = make_conf_vector(&lowtokens);
	if ( lowtokens->str )
	{
		fprintf(
			stderr,
			"make_conf_tree(): unexcepted data '%s' line:%d col=%d\n",
			lowtokens->str,
			lowtokens->line,
			lowtokens->col);
		exit(1);
	}
	return v4;
}

static int get_vector_len(TokenTree *ttp)
{
	int v2;

	for ( v2 = 0; ttp->tkcode; v2 += 1, ttp += 1 )
	{
		;
	}
	return v2;
}

static int get_stringvector_len(const char **str)
{
	int v2;

	for ( v2 = 0; *str; v2 += 1, str += 1 )
	{
		;
	}
	return v2;
}

static const char **add_stringvector(const char **str, const char *newstr)
{
	int stringvector_len;
	const char **result;
	int nstr;

	stringvector_len = get_stringvector_len(str);
	nstr = stringvector_len + 1;
	result = (const char **)realloc(str, (stringvector_len + 2) * sizeof(const char *));
	result[nstr - 1] = newstr;
	result[nstr] = 0;
	return result;
}

static int setup_reserved_symbol_table(CreateSymbolConf *result, TokenTree *ttp, Srx_gen_table *conf)
{
	if ( !strcmp("GLOBAL", ttp[1].value.lowtoken->str) )
	{
		result->bind = STB_GLOBAL;
	}
	else if ( !strcmp("LOCAL", ttp[1].value.lowtoken->str) )
	{
		result->bind = STB_LOCAL;
	}
	else if ( !strcmp("WEAK", ttp[1].value.lowtoken->str) )
	{
		result->bind = STB_WEAK;
	}
	else
	{
		fprintf(stderr, "Unsupported bind '%s' for '%s'\n", ttp[1].value.lowtoken->str, ttp->value.lowtoken->str);
		return 1;
	}
	if ( strcmp("OBJECT", ttp[2].value.lowtoken->str) != 0 )
	{
		fprintf(stderr, "Unsupported type '%s' for '%s'\n", ttp[2].value.lowtoken->str, ttp->value.lowtoken->str);
		return 1;
	}
	result->type = STT_OBJECT;
	result->segment = lookup_segment(conf, ttp[3].value.lowtoken->str, 0);
	if ( !result->segment && ttp[3].value.lowtoken->str[0] == '.' )
	{
		result->sectname = ttp[3].value.lowtoken->str;
	}
	else
	{
		result->sectname = 0;
		if ( !result->segment )
		{
			fprintf(stderr, "Unknown segment '%s' for '%s'\n", ttp[3].value.lowtoken->str, ttp->value.lowtoken->str);
			return 1;
		}
	}
	if ( !strcmp("SHN_RADDR", ttp[4].value.lowtoken->str) )
	{
		result->shindex = 65311;
	}
	else if ( !strcmp("0", ttp[4].value.lowtoken->str) )
	{
		result->shindex = 0;
	}
	else
	{
		fprintf(stderr, "Unknown shindex '%s' for '%s'\n", ttp[4].value.lowtoken->str, ttp->value.lowtoken->str);
		return 1;
	}
	if ( !strcmp("start", ttp[5].value.lowtoken->str) )
	{
		result->seflag = 0;
	}
	else if ( !strcmp("end", ttp[5].value.lowtoken->str) )
	{
		result->seflag = 1;
	}
	else if ( !strcmp("gpbase", ttp[5].value.lowtoken->str) )
	{
		result->seflag = 2;
	}
	else
	{
		fprintf(stderr, "Unknown base '%s' for '%s'\n", ttp[5].value.lowtoken->str, ttp->value.lowtoken->str);
		return 1;
	}
	result->name = ttp->value.lowtoken->str;
	return 0;
}

static int gen_define(TokenTree *ttp, Srx_gen_table *result)
{
	PheaderInfo *phrlist;
	SegConf *segp;
	SegConf *seglist;
	TokenTree *subarg;
	TokenTree *arg;
	int n;
	int nseg;
	int m;
	int j;
	int k;
	int i;
	int entries_1;
	int entries_2;
	int entries_3;
	int entries_4;

	for ( ; ttp->tkcode; ttp += 2 )
	{
		if ( ttp[1].tkcode != TC_VECTOR )
		{
			fprintf(
				stderr,
				"argument not found for '%s' line:%d col=%d\n",
				ttp->value.lowtoken->str,
				ttp->value.lowtoken->line,
				ttp->value.lowtoken->col);
			return 1;
		}
		arg = ttp[1].value.subtree;
		switch ( ttp->tkcode )
		{
			case TC_Segments_name:
				entries_1 = get_vector_len(arg);
				seglist = (SegConf *)calloc(entries_1 + 1, sizeof(SegConf));
				result->segment_list = seglist;
				for ( m = 0; m < entries_1; m += 1 )
				{
					seglist[m].name = arg[m].value.lowtoken->str;
					seglist[m].sect_name_patterns = (const char **)calloc(1, sizeof(const char *));
				}
				break;
			case TC_Memory_segment:
				entries_4 = get_vector_len(arg);
				for ( i = 0; i < entries_4; i += 1 )
				{
					segp = lookup_segment(result, arg[i].value.lowtoken->str, 1);
					if ( !segp )
					{
						return 1;
					}
					segp->bitid = 1 << (segp - result->segment_list);
				}
				break;
			case TC_Program_header_order:
				entries_2 = get_vector_len(arg);
				phrlist = (PheaderInfo *)calloc(entries_2 + 1, sizeof(PheaderInfo));
				result->program_header_order = phrlist;
				for ( j = 0; j < entries_2; j += 1 )
				{
					switch ( arg[j].tkcode )
					{
						case TC_STRING:
							phrlist[j].sw = SRX_PH_TYPE_MOD;
							phrlist[j].d.section_name = arg[j].value.lowtoken->str;
							break;
						case TC_VECTOR:
							subarg = arg[j].value.subtree;
							nseg = get_vector_len(subarg);
							phrlist[j].sw = SRX_PH_TYPE_TEXT;
							phrlist[j].d.segment_list = (SegConf **)calloc(nseg + 1, sizeof(SegConf *));
							for ( n = 0; n < nseg; n += 1 )
							{
								phrlist[j].d.segment_list[n] = lookup_segment(result, subarg[n].value.lowtoken->str, 1);
								if ( !phrlist[j].d.segment_list[n] )
								{
									return 1;
								}
							}
							break;
						default:
							break;
					}
				}
				break;
			case TC_CreateSymbols:
				entries_3 = get_vector_len(arg);
				result->create_symbols = (CreateSymbolConf *)calloc(entries_3 + 1, sizeof(CreateSymbolConf));
				for ( k = 0; k < entries_3; k += 1 )
				{
					if ( arg[k].tkcode != TC_VECTOR || get_vector_len(arg[k].value.subtree) != 6 )
					{
						fprintf(stderr, "unexcepted data in @CreateSymbols\n");
						return 1;
					}
					if ( setup_reserved_symbol_table(&result->create_symbols[k], arg[k].value.subtree, result) )
					{
						return 1;
					}
				}
				break;
			default:
				fprintf(
					stderr,
					"unexcepted data '%s' line:%d col=%d\n",
					ttp->value.lowtoken->str,
					ttp->value.lowtoken->line,
					ttp->value.lowtoken->col);
				return 1;
		}
	}
	return 0;
}

static void get_section_type_flag(TokenTree *ttp, int *rtype, int *rflag)
{
	int flag;
	int type;

	type = 0;
	flag = 0;
	for ( ; ttp->tkcode; ttp += 1 )
	{
		const char *info;

		info = ttp->value.lowtoken->str;
		if ( !strcmp(info, "PROGBITS") )
		{
			type = SHT_PROGBITS;
		}
		else if ( !strcmp(info, "NOBITS") )
		{
			type = SHT_NOBITS;
		}
		else if ( !strcmp(info, "WRITE") )
		{
			flag |= SHF_WRITE;
		}
		else if ( !strcmp(info, "ALLOC") )
		{
			flag |= SHF_ALLOC;
		}
		else if ( !strcmp(info, "EXECINSTR") )
		{
			flag |= SHF_EXECINSTR;
		}
	}
	*rtype = type;
	*rflag = flag;
}

static elf_section *make_empty_section(const char *name, int type, int flag)
{
	elf_section *result;

	result = (elf_section *)calloc(1, sizeof(elf_section));
	result->name = strdup(name);
	result->shr.sh_type = type;
	result->shr.sh_flags = flag;
	result->shr.sh_size = 0;
	result->shr.sh_addralign = 4;
	result->shr.sh_entsize = 0;
	return result;
}

static Srx_gen_table *make_srx_gen_table(TokenTree *tokentree)
{
	SegConf *seg_1;
	SegConf *seg_2;
	const SegConf *seg_3;
	TokenTree *nttp;
	TokenTree *ttp2_1;
	TokenTree *ttp2_2;
	TokenTree *ttp1;
	TokenTree *ttp;
	Srx_gen_table *result;
	int sectflag;
	int secttype;
	unsigned int bitid;
	int nsect;
	const char **strp;
	const char *str;
	char *str2;

	result = (Srx_gen_table *)calloc(1, sizeof(Srx_gen_table));
	if ( tokentree->tkcode != TC_VECTOR )
	{
		fprintf(stderr, "Internal error:make_srx_gen_table();\n");
		free(result);
		return 0;
	}
	ttp = tokentree->value.subtree;
	nsect = 0;
	result->section_table_order = (const char **)calloc(1, sizeof(const char *));
	result->file_layout_order = (const char **)calloc(1, sizeof(const char *));
	result->removesection_list = (const char **)calloc(1, sizeof(const char *));
	result->section_list = (SectConf *)calloc(1, sizeof(SectConf));
	for ( ; ttp->tkcode; )
	{
		if ( ttp[1].tkcode == TC_VECTOR )
		{
			ttp1 = ttp[1].value.subtree;
			nttp = ttp + 2;
		}
		else
		{
			ttp1 = 0;
			nttp = ttp + 1;
		}
		switch ( ttp->tkcode )
		{
			case TC_STRING:
				str = ttp->value.lowtoken->str;
				result->section_table_order = add_stringvector(result->section_table_order, str);
				if ( !ttp1 )
				{
					result->file_layout_order = add_stringvector(result->file_layout_order, str);
					ttp = nttp;
					break;
				}
				if ( ttp1->tkcode == TC_remove )
				{
					result->removesection_list = add_stringvector(result->removesection_list, str);
					ttp = nttp;
					break;
				}
				if ( ttp1->tkcode == TC_segment && ttp1[1].tkcode == TC_VECTOR )
				{
					bitid = 0;
					for ( ttp2_1 = ttp1[1].value.subtree; ttp2_1->tkcode; ttp2_1 += 1 )
					{
						seg_1 = lookup_segment(result, ttp2_1->value.lowtoken->str, 1);
						if ( !seg_1 )
						{
							return 0;
						}
						seg_1->sect_name_patterns = add_stringvector(seg_1->sect_name_patterns, str);
						bitid |= seg_1->bitid;
					}
					if ( bitid )
					{
						result->section_list[nsect].sect_name_pattern = str;
						result->section_list[nsect].flag = bitid;
						nsect += 1;
						result->section_list = (SectConf *)realloc(result->section_list, (nsect + 1) * sizeof(SectConf));
						result->section_list[nsect].sect_name_pattern = 0;
						result->section_list[nsect].flag = 0;
						result->section_list[nsect].secttype = 0;
						result->section_list[nsect].sectflag = 0;
					}
					if ( ttp1[2].tkcode != TC_createinfo || ttp1[3].tkcode != TC_VECTOR )
					{
						ttp = nttp;
						break;
					}
					get_section_type_flag(ttp1[3].value.subtree, &secttype, &sectflag);
					if ( secttype && sectflag )
					{
						if ( bitid )
						{
							result->section_list[nsect - 1].secttype = secttype;
							result->section_list[nsect - 1].sectflag = sectflag;
						}
						seg_2 = NULL;
						for ( ttp2_2 = ttp1[1].value.subtree;; ttp2_2 += 1 )
						{
							if ( ttp2_2->tkcode == TC_NULL )
							{
								ttp = nttp;
								break;
							}
							seg_2 = lookup_segment(result, ttp2_2->value.lowtoken->str, 1);
							if ( !seg_2 )
							{
								break;
							}
							if ( !seg_2->empty_section )
							{
								seg_2->empty_section = make_empty_section(str, secttype, sectflag);
							}
						}
						if ( !seg_2 )
						{
							free(result);
							return 0;
						}
					}
					else
					{
						fprintf(
							stderr, "Illegal @createinfo line:%d col=%d\n", ttp1->value.lowtoken->line, ttp1->value.lowtoken->col);
						free(result);
						return 0;
					}
				}
				else
				{
					fprintf(
						stderr,
						"unexcepted data '%s' line:%d col=%d\n",
						ttp1->value.lowtoken->str,
						ttp1->value.lowtoken->line,
						ttp1->value.lowtoken->col);
					free(result);
					return 0;
				}
				break;
			case TC_IOP:
				result->target = SRX_TARGET_IOP;
				ttp = nttp;
				break;
			case TC_EE:
				result->target = SRX_TARGET_EE;
				ttp = nttp;
				break;
			case TC_Define:
				if ( !ttp1 )
				{
					fprintf(
						stderr,
						"argument not found for '%s' line:%d col=%d\n",
						ttp->value.lowtoken->str,
						ttp->value.lowtoken->line,
						ttp->value.lowtoken->col);
					free(result);
					return 0;
				}
				if ( !gen_define(ttp1, result) )
				{
					ttp = nttp;
					break;
				}
				free(result);
				return 0;
			case TC_Program_header_data:
				if ( !ttp1 || ttp1->tkcode != TC_STRING )
				{
					if ( ttp1 )
					{
						fprintf(
							stderr,
							"unexcepted data '%s' line:%d col=%d\n",
							ttp1->value.lowtoken->str,
							ttp1->value.lowtoken->line,
							ttp1->value.lowtoken->col);
					}
					else
					{
						fprintf(
							stderr,
							"%s missing '{ <n> }' line:%d col=%d\n",
							ttp->value.lowtoken->str,
							ttp->value.lowtoken->line,
							ttp->value.lowtoken->col);
					}
					free(result);
					return 0;
				}
				str2 = (char *)malloc(0x32);
				sprintf(str2, "@Program_header_data %s", ttp1->value.lowtoken->str);
				result->file_layout_order = add_stringvector(result->file_layout_order, str2);
				ttp = nttp;
				break;
			case TC_Segment_data:
				for ( ;; )
				{
					if ( ttp1 != NULL && ttp1->tkcode == TC_NULL )
					{
						ttp = nttp;
						break;
					}
					if ( ttp1 != NULL )
					{
						seg_3 = lookup_segment(result, ttp1->value.lowtoken->str, 1);
						if ( seg_3 )
						{
							for ( strp = seg_3->sect_name_patterns; *strp; strp += 1 )
							{
								result->file_layout_order = add_stringvector(result->file_layout_order, *strp);
							}
							ttp1 += 1;
							continue;
						}
					}
					free(result);
					return 0;
				}
				break;
			case TC_Section_header_table:
				result->file_layout_order = add_stringvector(result->file_layout_order, "@Section_header_table");
				ttp = nttp;
				break;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
			case TC_VECTOR:
				ttp = ttp->value.subtree;
			default:
				if ( ttp->value.lowtoken != NULL )
				{
					fprintf(
						stderr,
						"unexcepted data '%s' line:%d col=%d\n",
						ttp->value.lowtoken->str,
						ttp->value.lowtoken->line,
						ttp->value.lowtoken->col);
				}
				free(result);
				return 0;
#pragma GCC diagnostic pop
		}
	}
	switch ( result->target )
	{
		case 1:
		case 2:
			return result;
		default:
			fprintf(stderr, "@IOP or @EE not found error !\n");
			free(result);
			return 0;
	}
}

static void check_change_bit(unsigned int oldbit, unsigned int newbit, unsigned int *up, unsigned int *down)
{
	*up = ~oldbit & newbit & (newbit ^ oldbit);
	*down = ~newbit & oldbit & (newbit ^ oldbit);
}

static int check_srx_gen_table(Srx_gen_table *tp)
{
	SegConf *scnfp;
	SectConf *sctp;
	int nsegment;
	int b;
	int error;
	unsigned int defbitid;
	unsigned int downdelta;
	unsigned int updelta;
	unsigned int oldbitid;

	nsegment = 0;
	for ( scnfp = tp->segment_list; scnfp && scnfp->name; scnfp += 1 )
	{
		nsegment += 1;
	}
	defbitid = 0;
	oldbitid = 0;
	error = 0;
	for ( sctp = tp->section_list; sctp->sect_name_pattern; sctp += 1 )
	{
		check_change_bit(oldbitid, sctp->flag, &updelta, &downdelta);
		if ( (defbitid & updelta) != 0 )
		{
			error += 1;
			for ( b = 0; b < nsegment; b += 1 )
			{
				if ( (sctp->flag & (1 << b)) != 0 )
				{
					fprintf(stderr, "Segment '%s' restart by section `%s`\n", tp->segment_list[b].name, sctp->sect_name_pattern);
					break;
				}
			}
		}
		oldbitid = sctp->flag;
		check_change_bit(oldbitid, sctp[1].flag, &updelta, &downdelta);
		defbitid |= downdelta;
	}
	return error;
}
