#include <string.h>
#include <gtk/gtk.h>
#include "osdialog.h"


extern osdialog_save_callback osdialog_save_cb;
extern osdialog_restore_callback osdialog_restore_cb;

#define SAVE_CALLBACK \
	void* context = NULL; \
	if (osdialog_save_cb) { \
		context = osdialog_save_cb(); \
	}

#define RESTORE_CALLBACK \
	if (osdialog_restore_cb) { \
		osdialog_restore_cb(context); \
	}


static GtkWidget* osdialog_message_create(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	if (!gtk_init_check(NULL, NULL))
		return NULL;

	GtkMessageType messageType =
		(level == OSDIALOG_WARNING) ? GTK_MESSAGE_WARNING :
		(level == OSDIALOG_ERROR) ? GTK_MESSAGE_ERROR :
		GTK_MESSAGE_INFO;

	GtkButtonsType buttonsType =
		(buttons == OSDIALOG_OK_CANCEL) ? GTK_BUTTONS_OK_CANCEL :
		(buttons == OSDIALOG_YES_NO) ? GTK_BUTTONS_YES_NO :
		GTK_BUTTONS_OK;

	GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, messageType, buttonsType, "%s", message);

	// Uncomment to customize dialog
	// gtk_window_set_title(GTK_WINDOW(dialog), "");
	// gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "");

	return dialog;
}


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	SAVE_CALLBACK

	GtkWidget* dialog = osdialog_message_create(level, buttons, message);
	if (!dialog) {
		RESTORE_CALLBACK
		return 0;
	}

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	RESTORE_CALLBACK

	int result = (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_YES);
	return result;
}


typedef struct {
	osdialog_message_callback cb;
	void* user;
	void* context;
} osdialog_message_data;


static void osdialog_message_response(GtkDialog* dialog, gint response, gpointer ptr) {
	osdialog_message_data* data = ptr;

	gtk_widget_destroy(GTK_WIDGET(dialog));

	void* context = data->context;
	RESTORE_CALLBACK

	int result = (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_YES);
	if (data->cb)
		data->cb(result, data->user);

	OSDIALOG_FREE(data);
}


void osdialog_message_async(osdialog_message_level level, osdialog_message_buttons buttons, const char* message, void* user, osdialog_message_callback cb) {
	SAVE_CALLBACK

	GtkWidget* dialog = osdialog_message_create(level, buttons, message);
	if (!dialog) {
		RESTORE_CALLBACK
		cb(0, user);
		return;
	}

	osdialog_message_data* data = OSDIALOG_MALLOC(sizeof(osdialog_message_data));
	data->cb = cb;
	data->user = user;
	data->context = context;

	g_signal_connect(dialog, "response", G_CALLBACK(osdialog_message_response), data);
	gtk_widget_show_all(dialog);
}


char* osdialog_prompt(osdialog_message_level level, const char* message, const char* text) {
	if (!gtk_init_check(NULL, NULL))
		return 0;

	SAVE_CALLBACK

	GtkMessageType messageType =
		(level == OSDIALOG_WARNING) ? GTK_MESSAGE_INFO :
		(level == OSDIALOG_ERROR) ? GTK_MESSAGE_ERROR :
		GTK_MESSAGE_INFO;

	GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, messageType, GTK_BUTTONS_OK_CANCEL, "%s", message);

	GtkWidget* entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), text);

	GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(content_area), entry);
	gtk_widget_show_all(dialog);

	gint button = gtk_dialog_run(GTK_DIALOG(dialog));
	const char* result_str = gtk_entry_get_text(GTK_ENTRY(entry));

	char* result = NULL;
	if (button == GTK_RESPONSE_OK) {
		result = osdialog_strndup(result_str, strlen(result_str));
	}
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	RESTORE_CALLBACK

	return result;
}


void osdialog_prompt_async(osdialog_message_level level, const char* message, const char* text, void* user, osdialog_prompt_callback cb) {
	// TODO Replace this blocking placeholder with actual async
	char* result = osdialog_prompt(level, message, text);

	if (cb)
		cb(result, user);
}


char* osdialog_file(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters) {
	if (!gtk_init_check(NULL, NULL))
		return 0;

	SAVE_CALLBACK

	GtkFileChooserAction gtkAction;
	const char* title;
	const char* acceptText;
	if (action == OSDIALOG_OPEN) {
		title = "Open File";
		acceptText = "Open";
		gtkAction = GTK_FILE_CHOOSER_ACTION_OPEN;
	}
	else if (action == OSDIALOG_OPEN_DIR) {
		title = "Open Folder";
		acceptText = "Open Folder";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	}
	else {
		title = "Save File";
		acceptText = "Save";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SAVE;
	}

	GtkWidget* dialog = gtk_file_chooser_dialog_new(title, NULL, gtkAction, "_Cancel", GTK_RESPONSE_CANCEL, acceptText, GTK_RESPONSE_ACCEPT, NULL);

	for (; filters; filters = filters->next) {
		GtkFileFilter* fileFilter = gtk_file_filter_new();
		gtk_file_filter_set_name(fileFilter, filters->name);
		for (const osdialog_filter_patterns* patterns = filters->patterns; patterns; patterns = patterns->next) {
			char patternBuf[1024];
			snprintf(patternBuf, sizeof(patternBuf), "*.%s", patterns->pattern);
			gtk_file_filter_add_pattern(fileFilter, patternBuf);
		}
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fileFilter);
	}

	if (action == OSDIALOG_SAVE)
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dir);

	if (action == OSDIALOG_SAVE && filename)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);

	char* chosen_filename = NULL;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		chosen_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}
	gtk_widget_destroy(dialog);

	char* result = NULL;
	if (chosen_filename) {
		result = osdialog_strndup(chosen_filename, strlen(chosen_filename));
		g_free(chosen_filename);
	}

	while (gtk_events_pending())
		gtk_main_iteration();

	RESTORE_CALLBACK

	return result;
}


void osdialog_file_async(osdialog_file_action action, const char* path, const char* filename, const osdialog_filters* filters, void* user, osdialog_file_callback cb) {
	// TODO Replace this blocking placeholder with actual async
	char* result = osdialog_file(action, path, filename, filters);

	if (cb)
		cb(result, user);
}


int osdialog_color_picker(osdialog_color* color, int opacity) {
	if (!color)
		return 0;
	if (!gtk_init_check(NULL, NULL))
		return 0;

	SAVE_CALLBACK

#if GTK_MAJOR_VERSION == 3
	GtkWidget* dialog = gtk_color_chooser_dialog_new("Color", NULL);
	GtkColorChooser* colorsel = GTK_COLOR_CHOOSER(dialog);
	gtk_color_chooser_set_use_alpha(colorsel, opacity);
	GdkRGBA c;
	// uint8_t to float
	c.red = color->r / 255.0;
	c.green = color->g / 255.0;
	c.blue = color->b / 255.0;
	c.alpha = color->a / 255.0;
	gtk_color_chooser_set_rgba(colorsel, &c);
#elif GTK_MAJOR_VERSION == 2
	GtkWidget* dialog = gtk_color_selection_dialog_new("Color");
	GtkColorSelection* colorsel = GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog)));
	gtk_color_selection_set_has_opacity_control(colorsel, opacity);
	GdkColor c;
	// uint8_t to uint16_t
	c.red = (uint16_t) color->r * 257;
	c.green = (uint16_t) color->g * 257;
	c.blue = (uint16_t) color->b * 257;
	gtk_color_selection_set_current_color(colorsel, &c);
	gtk_color_selection_set_current_alpha(colorsel, (uint16_t) color->a * 257);
#endif

	int result = 0;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
#if GTK_MAJOR_VERSION == 3
		GdkRGBA c;
		gtk_color_chooser_get_rgba(colorsel, &c);
		// float to uint8_t
		color->r = c.red * 255.0;
		color->g = c.green * 255.0;
		color->b = c.blue * 255.0;
		color->a = c.alpha * 255.0;
#elif GTK_MAJOR_VERSION == 2
		GdkColor c;
		gtk_color_selection_get_current_color(colorsel, &c);
		// uint16_t to uint8_t
		color->r = c.red / 257;
		color->g = c.green / 257;
		color->b = c.blue / 257;
		color->a = gtk_color_selection_get_current_alpha(colorsel) / 257;
#endif

		result = 1;
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	RESTORE_CALLBACK

	return result;
}


void osdialog_color_picker_async(osdialog_color* color, int opacity, void* user, osdialog_color_picker_callback cb) {
	// TODO Replace this blocking placeholder with actual async
	int result = osdialog_color_picker(color, opacity);

	if (cb)
		cb(result, user);
}
