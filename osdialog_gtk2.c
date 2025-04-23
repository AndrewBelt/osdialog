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
		(level == OSDIALOG_WARNING) ? GTK_MESSAGE_INFO :
		(level == OSDIALOG_ERROR) ? GTK_MESSAGE_ERROR :
		GTK_MESSAGE_INFO;

	GtkButtonsType buttonsType =
		(buttons == OSDIALOG_OK_CANCEL) ? GTK_BUTTONS_OK_CANCEL :
		(buttons == OSDIALOG_YES_NO) ? GTK_BUTTONS_YES_NO :
		GTK_BUTTONS_OK;

	return gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, messageType, buttonsType, "%s", message);
}


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	SAVE_CALLBACK

	GtkWidget* dialog = osdialog_message_create(level, buttons, message);
	if (!dialog) {
		RESTORE_CALLBACK
		return 0;
	}

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	RESTORE_CALLBACK

	return (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_YES);
}


typedef struct {
	osdialog_message_callback cb;
	void* user;
	void* context;
} osdialog_message_data;


static void osdialog_message_response(GtkDialog* dialog, gint response, gpointer ptr) {
	gtk_widget_destroy(GTK_WIDGET(dialog));

	while (gtk_events_pending())
		gtk_main_iteration();

	osdialog_message_data* data = ptr;
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


int osdialog_color_picker(osdialog_color* color, int opacity) {
	if (!color)
		return 0;
	if (!gtk_init_check(NULL, NULL))
		return 0;

	SAVE_CALLBACK

#ifdef OSDIALOG_GTK3
	GtkWidget* dialog = gtk_color_chooser_dialog_new("Color", NULL);
	GtkColorChooser* colorsel = GTK_COLOR_CHOOSER(dialog);
	gtk_color_chooser_set_use_alpha(colorsel, opacity);
#else
	GtkWidget* dialog = gtk_color_selection_dialog_new("Color");
	GtkColorSelection* colorsel = GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog)));
	gtk_color_selection_set_has_opacity_control(colorsel, opacity);
#endif

	int result = 0;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
#ifdef OSDIALOG_GTK3
		GdkRGBA c;
		gtk_color_chooser_get_rgba(colorsel, &c);
		color->r = c.red * 65535 + 0.5;
		color->g = c.green * 65535 + 0.5;
		color->b = c.blue * 65535 + 0.5;
		color->a = c.alpha * 65535 + 0.5;
#else
		GdkColor c;
		gtk_color_selection_get_current_color(colorsel, &c);
		color->r = c.red >> 8;
		color->g = c.green >> 8;
		color->b = c.blue >> 8;
		color->a = gtk_color_selection_get_current_alpha(colorsel) >> 8;
#endif

		result = 1;
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	RESTORE_CALLBACK

	return result;
}
