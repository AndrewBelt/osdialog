#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


typedef enum {
	OSDIALOG_INFO,
	OSDIALOG_WARNING,
	OSDIALOG_ERROR,
} osdialog_message_level;

typedef enum {
	OSDIALOG_OK,
	OSDIALOG_OK_CANCEL,
	OSDIALOG_YES_NO,
} osdialog_message_buttons;

/** Launches a message box */
int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message);


typedef enum {
	OSDIALOG_OPEN,
	OSDIALOG_OPEN_DIR,
	OSDIALOG_SAVE,
} osdialog_file_action;

/** Launches a file dialog and returns the selected path
If the return result is not NULL, caller must free() it

`path` is the default folder the file dialog will attempt to open in.
`filename` is the default text that will appear in the filename input. Relevant to save dialog only.
`filters` is not supported yet TODO
*/
char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, const char *filters);


typedef struct {
	uint8_t r, g, b, a;
} osdialog_color;

// TODO
void osdialog_color_picker();


#ifdef __cplusplus
}
#endif
