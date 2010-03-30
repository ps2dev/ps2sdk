#ifndef __GRAPH_CONFIG_H__
#define __GRAPH_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

	// Creates a string from the current mode.
	// The string format is: mode:int:ffmd:filter:x:y:
	int graph_make_config(int mode, int interlace, int ffmd, int x, int y, int flicker_filter, char *config);

	// Gets the string made from the current mode.
	int graph_get_config(char *config);

	// Sets the mode from the configuration string.
	int graph_set_config(char *config);

	// Reads the configuration string from a file and sets it.
	int graph_load_config(char *filename);

	// Writes the current mode information into a config file as a string.
	int graph_save_config(char *filename);
 
#ifdef __cplusplus
}
#endif

#endif /*__GRAPH_CONFIG_H__*/
