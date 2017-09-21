#include <stdlib.h>
#include <stdio.h>
#include "osdialog.h"


int main() {
	/*
	// Message
	{
		printf("%d\n", osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, "Info"));
		printf("%d\n", osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, "Warning"));
		printf("%d\n", osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, "Error"));
	}

	// Open file with default arguments
	{
		char *filename = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		if (filename) {
			printf("%s\n", filename);
			free(filename);
		}
		else {
			printf("Canceled\n");
		}
	}

	// Open directory with default arguments
	{
		char *filename = osdialog_file(OSDIALOG_OPEN_DIR, NULL, NULL, NULL);
		if (filename) {
			printf("%s\n", filename);
			free(filename);
		}
		else {
			printf("Canceled\n");
		}
	}

	// Save file with default arguments
	{
		char *filename = osdialog_file(OSDIALOG_SAVE, NULL, NULL, NULL);
		if (filename) {
			printf("%s\n", filename);
			free(filename);
		}
		else {
			printf("Canceled\n");
		}
	}

	// Open file with custom arguments
	{
		char *filename = osdialog_file(OSDIALOG_OPEN, ".", "test", NULL);
		if (filename) {
			printf("%s\n", filename);
			free(filename);
		}
		else {
			printf("Canceled\n");
		}
	}

	// Open directory with custom arguments
	{
		char *filename = osdialog_file(OSDIALOG_OPEN_DIR, ".", "test", NULL);
		if (filename) {
			printf("%s\n", filename);
			free(filename);
		}
		else {
			printf("Canceled\n");
		}
	}

	// Save file with custom arguments
	{
		char *filename = osdialog_file(OSDIALOG_SAVE, ".", "test", NULL);
		if (filename) {
			printf("%s\n", filename);
			free(filename);
		}
		else {
			printf("Canceled\n");
		}
	}
	*/

	// Color selector
	{
		osdialog_color color = {255, 0, 255, 255};
		printf("%d\n", osdialog_color_picker(&color, 0));
		printf("#%02x%02x%02x%02x\n", color.r, color.g, color.b, color.a);
		printf("%d\n", osdialog_color_picker(&color, 1));
		printf("#%02x%02x%02x%02x\n", color.r, color.g, color.b, color.a);
	}
}