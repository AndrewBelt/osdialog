#include "osdialog.h"
#include <assert.h>
#include <gtk/gtk.h>


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, const char *filters) {
	assert(gtk_init_check(NULL, NULL));

	GtkWidget *dialog = gtk_file_chooser_dialog_new(
		action == OSDIALOG_OPEN ? "Open File" : "Save File",
		NULL,
		action == OSDIALOG_OPEN ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		action == OSDIALOG_OPEN ? GTK_STOCK_OPEN : GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	if (action == OSDIALOG_SAVE)
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (path)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);

	if (action == OSDIALOG_SAVE && filename)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);

	char *result = NULL;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		result = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}
	gtk_widget_destroy(dialog);

	// while (gtk_events_pending()) gtk_main_iteration();
	return result;
}
