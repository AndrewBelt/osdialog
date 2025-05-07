#include <string.h>
#include <gtk/gtk.h>
#include "osdialog.h"


extern osdialog_save_callback* osdialog_save_cb;
extern osdialog_restore_callback* osdialog_restore_cb;

#define SAVE_CALLBACK \
	void* context = NULL; \
	if (osdialog_save_cb) { \
		context = osdialog_save_cb(); \
	}

#define RESTORE_CALLBACK \
	if (osdialog_restore_cb) { \
		osdialog_restore_cb(context); \
	}

#if GTK_MAJOR_VERSION <= 3
	#define GTK_INIT gtk_init_check(NULL, NULL)
#else // GTK_MAJOR_VERSION == 4
	#define GTK_INIT gtk_init_check()
#endif


#if GTK_MAJOR_VERSION <= 3
#define MESSAGE_TYPE GtkWidget
#else // GTK_MAJOR_VERSION == 4
#define MESSAGE_TYPE GtkAlertDialog
#endif
static MESSAGE_TYPE* message_create(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	if (!GTK_INIT)
		return NULL;

#if GTK_MAJOR_VERSION <= 3
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

#else // GTK_MAJOR_VERSION == 4
	GtkAlertDialog* dialog = gtk_alert_dialog_new("%s", message);
	gtk_alert_dialog_set_buttons(dialog, (const char*[]) {"Cancel", "OK", NULL});
	gtk_alert_dialog_set_default_button(dialog, 1);
	gtk_alert_dialog_set_cancel_button(dialog, 0);
#endif

	return dialog;
}


#if GTK_MAJOR_VERSION == 4
typedef struct {
	int response;
	GMainLoop* loop;
} message_data;

static void message_callback(GObject* source, GAsyncResult* res, gpointer ptr) {
	message_data* data = ptr;
	data->response = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), res, NULL);
	g_main_loop_quit(data->loop);
}
#endif


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	SAVE_CALLBACK

	MESSAGE_TYPE* dialog = message_create(level, buttons, message);
	if (!dialog) {
		RESTORE_CALLBACK
		return 0;
	}

#if GTK_MAJOR_VERSION <= 3
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	int result = (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_YES);
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

#else // GTK_MAJOR_VERSION == 4
	message_data data;
	data.loop = g_main_loop_new(NULL, FALSE);

	GtkWindow* parent = NULL;
	gtk_alert_dialog_choose(dialog, parent, NULL, message_callback, &data);

	g_main_loop_run(data.loop);
	g_main_loop_unref(data.loop);
	g_object_unref(dialog);
	int result = (data.response == 1);
#endif

	RESTORE_CALLBACK
	return result;
}


typedef struct {
	osdialog_message_callback* cb;
	void* user;
	void* context;
} osdialog_message_data;


#if GTK_MAJOR_VERSION <= 3
static void message_response(GtkDialog* dialog, gint response, gpointer ptr) {
	osdialog_message_data* data = ptr;

	int result = (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_YES);
	gtk_widget_destroy(GTK_WIDGET(dialog));

	void* context = data->context;
	RESTORE_CALLBACK

	if (data->cb)
		data->cb(result, data->user);

	OSDIALOG_FREE(data);
}
#endif


void osdialog_message_async(osdialog_message_level level, osdialog_message_buttons buttons, const char* message, osdialog_message_callback* cb, void* user) {
	SAVE_CALLBACK

#if GTK_MAJOR_VERSION <= 3
	GtkWidget* dialog = message_create(level, buttons, message);
	if (!dialog) {
		RESTORE_CALLBACK
		if (cb)
			cb(0, user);
		return;
	}
#else // GTK_MAJOR_VERSION == 4
	// TODO
#endif

	osdialog_message_data* data = OSDIALOG_MALLOC(sizeof(osdialog_message_data));
	data->cb = cb;
	data->user = user;
	data->context = context;

#if GTK_MAJOR_VERSION <= 3
	g_signal_connect(dialog, "response", G_CALLBACK(message_response), data);
	gtk_widget_show_all(dialog);
#else // GTK_MAJOR_VERSION == 4
	// TODO
#endif
}


static GtkWidget* prompt_create(osdialog_message_level level, const char* message, const char* text) {
	if (!GTK_INIT)
		return NULL;

#if GTK_MAJOR_VERSION <= 3
	GtkMessageType messageType =
		(level == OSDIALOG_WARNING) ? GTK_MESSAGE_INFO :
		(level == OSDIALOG_ERROR) ? GTK_MESSAGE_ERROR :
		GTK_MESSAGE_INFO;

	GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, messageType, GTK_BUTTONS_OK_CANCEL, "%s", message);

	GtkWidget* entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), text);
	g_object_set_data(G_OBJECT(dialog), "entry", entry);

	GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(content_area), entry);

	gtk_widget_show_all(dialog);
#else // GTK_MAJOR_VERSION == 4
	GtkWidget* dialog = NULL;
#endif

	return dialog;
}


char* osdialog_prompt(osdialog_message_level level, const char* message, const char* text) {
	SAVE_CALLBACK

	GtkWidget* dialog = prompt_create(level, message, text);
	if (!dialog) {
		RESTORE_CALLBACK
		return NULL;
	}

#if GTK_MAJOR_VERSION <= 3
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	GtkWidget* entry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "entry"));
	char* result = NULL;
	if (response == GTK_RESPONSE_OK) {
		result = osdialog_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();
#else // GTK_MAJOR_VERSION == 4
	// TODO
	char* result = NULL;
#endif

	RESTORE_CALLBACK
	return result;
}


typedef struct {
	osdialog_prompt_callback* cb;
	void* user;
	void* context;
} osdialog_prompt_data;


#if GTK_MAJOR_VERSION <= 3
static void prompt_response(GtkDialog* dialog, gint response, gpointer ptr) {
	osdialog_prompt_data* data = ptr;

	GtkWidget* entry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "entry"));
	char* result = NULL;
	if (response == GTK_RESPONSE_OK) {
		result = osdialog_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));

	void* context = data->context;
	RESTORE_CALLBACK

	if (data->cb)
		data->cb(result, data->user);

	OSDIALOG_FREE(data);
}
#endif


void osdialog_prompt_async(osdialog_message_level level, const char* message, const char* text, osdialog_prompt_callback* cb, void* user) {
	SAVE_CALLBACK

	GtkWidget* dialog = prompt_create(level, message, text);
	if (!dialog) {
		RESTORE_CALLBACK
		if (cb)
			cb(NULL, user);
		return;
	}

	osdialog_prompt_data* data = OSDIALOG_MALLOC(sizeof(osdialog_prompt_data));
	data->cb = cb;
	data->user = user;
	data->context = context;

#if GTK_MAJOR_VERSION <= 3
	g_signal_connect(dialog, "response", G_CALLBACK(prompt_response), data);
	gtk_widget_show_all(dialog);
#else // GTK_MAJOR_VERSION == 4
	// TODO
#endif
}


static GtkWidget* file_create(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters) {
	if (!GTK_INIT)
		return NULL;

#if GTK_MAJOR_VERSION <= 3
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
#else // GTK_MAJOR_VERSION == 4
	// TODO
	GtkWidget* dialog = NULL;
#endif

	return dialog;
}


char* osdialog_file(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters) {
	SAVE_CALLBACK

	GtkWidget* dialog = file_create(action, dir, filename, filters);
	if (!dialog) {
		RESTORE_CALLBACK
		return NULL;
	}

#if GTK_MAJOR_VERSION <= 3
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	char* result = NULL;
	if (response == GTK_RESPONSE_ACCEPT) {
		gchar* chosen_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		result = osdialog_strdup(chosen_filename);
		g_free(chosen_filename);
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();
#else // GTK_MAJOR_VERSION == 4
	// TODO
	char* result = NULL;
#endif

	RESTORE_CALLBACK
	return result;
}


typedef struct {
	osdialog_file_callback* cb;
	void* user;
	void* context;
} osdialog_file_data;


#if GTK_MAJOR_VERSION <= 3
static void file_response(GtkDialog* dialog, gint response, gpointer ptr) {
	osdialog_file_data* data = ptr;

	char* result = NULL;
	if (response == GTK_RESPONSE_ACCEPT) {
		gchar* chosen_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		result = osdialog_strdup(chosen_filename);
		g_free(chosen_filename);
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));

	void* context = data->context;
	RESTORE_CALLBACK

	if (data->cb)
		data->cb(result, data->user);

	OSDIALOG_FREE(data);
}
#endif


void osdialog_file_async(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters, osdialog_file_callback* cb, void* user) {
	SAVE_CALLBACK

	GtkWidget* dialog = file_create(action, dir, filename, filters);
	if (!dialog) {
		RESTORE_CALLBACK
		if (cb)
			cb(NULL, user);
		return;
	}

	osdialog_file_data* data = OSDIALOG_MALLOC(sizeof(osdialog_file_data));
	data->cb = cb;
	data->user = user;
	data->context = context;

#if GTK_MAJOR_VERSION <= 3
	g_signal_connect(dialog, "response", G_CALLBACK(file_response), data);
	gtk_widget_show_all(dialog);
#else // GTK_MAJOR_VERSION == 4
	// TODO
#endif
}


static GtkWidget* color_picker_create(osdialog_color color, int opacity) {
	if (!GTK_INIT)
		return NULL;

#if GTK_MAJOR_VERSION == 2
	GtkWidget* dialog = gtk_color_selection_dialog_new("Color");
	GtkColorSelection* colorsel = GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog)));
	g_object_set_data(G_OBJECT(dialog), "colorsel", colorsel);
	gtk_color_selection_set_has_opacity_control(colorsel, opacity);
	GdkColor c;
	// uint8_t to uint16_t
	c.red = (uint16_t) color.r * 257;
	c.green = (uint16_t) color.g * 257;
	c.blue = (uint16_t) color.b * 257;
	gtk_color_selection_set_current_color(colorsel, &c);
	gtk_color_selection_set_current_alpha(colorsel, (uint16_t) color.a * 257);
#elif GTK_MAJOR_VERSION == 3
	GtkWidget* dialog = gtk_color_chooser_dialog_new("Color", NULL);
	GtkColorChooser* colorsel = GTK_COLOR_CHOOSER(dialog);
	gtk_color_chooser_set_use_alpha(colorsel, opacity);
	GdkRGBA c;
	// uint8_t to float
	c.red = color.r / 255.0;
	c.green = color.g / 255.0;
	c.blue = color.b / 255.0;
	c.alpha = color.a / 255.0;
	gtk_color_chooser_set_rgba(colorsel, &c);
#else // GTK_MAJOR_VERSION == 4
	// TODO
	GtkWidget* dialog = NULL;
#endif

	return dialog;
}


static void color_picker_get_color(GtkWidget* dialog, osdialog_color* color) {
	if (!dialog || !color)
		return;

#if GTK_MAJOR_VERSION == 2
	GtkColorSelection* colorsel = GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog)));
	GdkColor c;
	gtk_color_selection_get_current_color(colorsel, &c);
	// uint16_t to uint8_t
	color->r = c.red / 257;
	color->g = c.green / 257;
	color->b = c.blue / 257;
	color->a = gtk_color_selection_get_current_alpha(colorsel) / 257;
#elif GTK_MAJOR_VERSION == 3
	GtkColorChooser* colorsel = GTK_COLOR_CHOOSER(dialog);
	GdkRGBA c;
	gtk_color_chooser_get_rgba(colorsel, &c);
	// float to uint8_t
	color->r = c.red * 255.0;
	color->g = c.green * 255.0;
	color->b = c.blue * 255.0;
	color->a = c.alpha * 255.0;
#else // GTK_MAJOR_VERSION == 4
	// TODO
#endif
}


int osdialog_color_picker(osdialog_color* color, int opacity) {
	if (!color)
		return 0;

	SAVE_CALLBACK

	GtkWidget* dialog = color_picker_create(*color, opacity);
	if (!dialog) {
		RESTORE_CALLBACK
		return 0;
	}

#if GTK_MAJOR_VERSION <= 3
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));

	int result = (response == GTK_RESPONSE_OK);
	if (response == GTK_RESPONSE_OK) {
		color_picker_get_color(dialog, color);
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();
#else // GTK_MAJOR_VERSION == 4
	// TODO
	int result = 0;
#endif

	RESTORE_CALLBACK
	return result;
}


typedef struct {
	osdialog_color_picker_callback* cb;
	void* user;
	void* context;
} osdialog_color_picker_data;


#if GTK_MAJOR_VERSION <= 3
static void color_picker_response(GtkDialog* dialog, gint response, gpointer ptr) {
	osdialog_color_picker_data* data = ptr;

	osdialog_color color = {0, 0, 0, 0};
	int result = (response == GTK_RESPONSE_OK);
	if (response == GTK_RESPONSE_OK) {
		color_picker_get_color(GTK_WIDGET(dialog), &color);
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));

	void* context = data->context;
	RESTORE_CALLBACK

	if (data->cb)
		data->cb(result, color, data->user);

	OSDIALOG_FREE(data);
}
#endif


void osdialog_color_picker_async(osdialog_color color, int opacity, osdialog_color_picker_callback* cb, void* user) {
	SAVE_CALLBACK

	GtkWidget* dialog = color_picker_create(color, opacity);
	if (!dialog) {
		RESTORE_CALLBACK
		if (cb)
			cb(0, color, user);
		return;
	}

	osdialog_color_picker_data* data = OSDIALOG_MALLOC(sizeof(osdialog_color_picker_data));
	data->cb = cb;
	data->user = user;
	data->context = context;

#if GTK_MAJOR_VERSION <= 3
	g_signal_connect(dialog, "response", G_CALLBACK(color_picker_response), data);
	gtk_widget_show_all(dialog);
#else // GTK_MAJOR_VERSION == 4
	// TODO
#endif
}
