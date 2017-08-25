#pragma once

#include <stdint.h>


typedef enum {
	OSDIALOG_OPEN,
	OSDIALOG_SAVE,
} osdialog_file_action;

/** Launches a file dialog and results the selected file path
If the return result is not NULL, caller must free() it
*/
char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, const char *filters);


typedef struct {
	uint8_t r, g, b, a;
} osdialog_color;

// TODO
void osdialog_color_picker();
