
#ifndef __EEELFLOADER_H__
#define __EEELFLOADER_H__

extern int loadfile_elfload_innerproc(const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out);
extern int loadfile_mg_elfload_proc(const char *filename, int epc, const char *section_name, int *result_out, int *result_module_out);

#endif
