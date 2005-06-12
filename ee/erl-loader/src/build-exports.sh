#!/bin/sh
ee-readelf -a bin/tmp.elf |
grep '\(GLOBAL\|WEAK\)' |
grep -v '\(export_list\|UND\|HIDDEN\)' |
awk ' { print $8 } ' |
sort -n |
awk ' { t[NR] = $0 }
  END {
        for (i = 1; i < NR; i++)
	  printf("void %s();\n", t[i]);
	printf("struct export_list_t {\n");
	printf("    char * name;\n");
	printf("    void * pointer;\n");
	printf("} export_list[] = {\n");
	for (i = 1; i < NR; i++)
	  printf("    { \"%s\", %s }, \n", t[i], t[i]);
	printf("    { 0, 0},\n};\n");
      } ' > src/exports.c
