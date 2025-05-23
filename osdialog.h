#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stddef.h>


#ifndef OSDIALOG_MALLOC
#define OSDIALOG_MALLOC malloc
#endif

#ifndef OSDIALOG_FREE
#define OSDIALOG_FREE free
#endif


char* osdialog_strdup(const char* s);
char* osdialog_strndup(const char* s, size_t n);


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

/** Launches a message box.

Returns 1 if the "OK" or "Yes" button was pressed.
*/
int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char* message);

typedef void osdialog_message_callback(int result, void* user);

/** Launches a message box asynchronously.

Calls cb(result, user) when dialog is finished.
`message` can be freed before the callback is called.
*/
void osdialog_message_async(osdialog_message_level level, osdialog_message_buttons buttons, const char* message, osdialog_message_callback* cb, void* user);

/** Launches an input prompt with an "OK" and "Cancel" button.

`text` is the default string to fill the input box.

Returns the entered text, or NULL if the dialog was cancelled.
If the returned result is not NULL, caller must free() it.
*/
char* osdialog_prompt(osdialog_message_level level, const char* message, const char* text);

typedef void osdialog_prompt_callback(char* result, void* user);

/** Launches an input prompt asynchronously.

Calls cb(result, user) when dialog is finished.
`message` and `text` can be freed before the callback is called.
*/
void osdialog_prompt_async(osdialog_message_level level, const char* message, const char* text, osdialog_prompt_callback* cb, void* user);


/** Linked list of patterns. */
typedef struct osdialog_filter_patterns {
	char* pattern;
	struct osdialog_filter_patterns* next;
} osdialog_filter_patterns;

/** Linked list of file filters. */
typedef struct osdialog_filters {
	char* name;
	osdialog_filter_patterns* patterns;
	struct osdialog_filters* next;
} osdialog_filters;

/** Parses a filter string.
Example: "Source:c,cpp,m;Header:h,hpp"
Caller must eventually free with osdialog_filters_free().
*/
osdialog_filters* osdialog_filters_parse(const char* str);
void osdialog_filter_patterns_free(osdialog_filter_patterns* patterns);
void osdialog_filters_free(osdialog_filters* filters);
osdialog_filters* osdialog_filters_copy(const osdialog_filters* src);

typedef enum {
	OSDIALOG_OPEN,
	OSDIALOG_OPEN_DIR,
	OSDIALOG_SAVE,
} osdialog_file_action;

/** Launches a file dialog and returns the selected path or NULL if nothing was selected.

`dir` is the default folder the file dialog will attempt to open in, or NULL for the OS's default.
`filename` is the default text that will appear in the filename input, or NULL for the OS's default. Relevant to save dialog only.
`filters` is a list of patterns to filter the file selection, or NULL.

Returns the selected file, or NULL if the dialog was cancelled.
If the return result is not NULL, caller must free() it.
*/
char* osdialog_file(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters);

typedef void osdialog_file_callback(char* result, void* user);

/** Launches a file dialog asynchronously.

Calls cb(result, user) when dialog is finished.
`dir`, `filename` and `filters` can be freed before the callback is called.
*/
void osdialog_file_async(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters, osdialog_file_callback* cb, void* user);


typedef struct {
	uint8_t r, g, b, a;
} osdialog_color;

/** Launches an RGBA color picker dialog and sets `color` to the selected color.
Returns 1 if "OK" was pressed.

`color` should be set to the initial color before calling. It is only overwritten if the user selects "OK".
`opacity` enables the opacity slider by setting to 1. Not supported on Windows.
*/
int osdialog_color_picker(osdialog_color* color, int opacity);

typedef void osdialog_color_picker_callback(int result, osdialog_color color, void* user);

void osdialog_color_picker_async(osdialog_color color, int opacity, osdialog_color_picker_callback* cb, void* user);


typedef void* osdialog_save_callback(void);
typedef void osdialog_restore_callback(void* ptr);

/** Sets callback that is called before each dialog is opened.
This is useful for saving/restoring global state that an OS dialog might modify.
*/
void osdialog_set_save_callback(osdialog_save_callback* cb);

/** Sets callback that is called after each dialog is closed.
The pointer returned by osdialog_save_callback() is passed as an argument to osdialog_restore_callback().
*/
void osdialog_set_restore_callback(osdialog_restore_callback* cb);


#ifdef __cplusplus
}
#endif
