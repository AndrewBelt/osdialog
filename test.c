#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "osdialog.h"


static sem_t semaphore;


static void message_callback(int res, void* user) {
	fprintf(stderr, "\tcallback: %d %p\n", res, user);
	sem_post(&semaphore);
}

static void prompt_callback(char* res, void* user) {
	fprintf(stderr, "\tcallback: %s %p\n", res, user);
	OSDIALOG_FREE(res);
	sem_post(&semaphore);
}

static void file_callback(char* res, void* user) {
	fprintf(stderr, "\tcallback: %s %p\n", res, user);
	OSDIALOG_FREE(res);
	sem_post(&semaphore);
}

static void color_picker_callback(int res, osdialog_color color, void* user) {
	fprintf(stderr, "\tcallback: %d #%02x%02x%02x%02x %p\n", res, color.r, color.g, color.b, color.a, user);
	sem_post(&semaphore);
}


int main(int argc, char* argv[]) {
	int test = -1;
	if (argc >= 2) {
		test = atoi(argv[1]);
	}

	// Message
	if (test < 0 || test == 1) {
		int res;

		fprintf(stderr, "message info\n");
		res = osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, "Info こんにちは");
		fprintf(stderr, "\t%d\n", res);

		fprintf(stderr, "message warning\n");
		res = osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, "Warning こんにちは");
		fprintf(stderr, "\t%d\n", res);

		fprintf(stderr, "message error\n");
		res = osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, "Error こんにちは");
		fprintf(stderr, "\t%d\n", res);
	}

	// Prompt
	if (test < 0 || test == 2) {
		char* text;

		fprintf(stderr, "prompt info\n");
		text = osdialog_prompt(OSDIALOG_INFO, "Info", "default text");
		if (text) {
			fprintf(stderr, "\t%s\n", text);
			free(text);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}

		fprintf(stderr, "prompt warning\n");
		text = osdialog_prompt(OSDIALOG_WARNING, "Warning", "default text");
		if (text) {
			fprintf(stderr, "\t%s\n", text);
			free(text);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}

		fprintf(stderr, "prompt error\n");
		text = osdialog_prompt(OSDIALOG_ERROR, "Error", "default text");
		if (text) {
			fprintf(stderr, "\t%s\n", text);
			free(text);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}
	}

	// Open directory with default arguments
	if (test < 0 || test == 3) {
		fprintf(stderr, "file open dir\n");
		char* filename = osdialog_file(OSDIALOG_OPEN_DIR, NULL, NULL, NULL);
		if (filename) {
			fprintf(stderr, "\t%s\n", filename);
			free(filename);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}
	}

	// Open file with default arguments
	if (test < 0 || test == 4) {
		fprintf(stderr, "file open\n");
		char* filename = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		if (filename) {
			fprintf(stderr, "\t%s\n", filename);
			free(filename);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}
	}

	// Save file with default arguments
	if (test < 0 || test == 5) {
		fprintf(stderr, "file save\n");
		char* filename = osdialog_file(OSDIALOG_SAVE, NULL, NULL, NULL);
		if (filename) {
			fprintf(stderr, "\t%s\n", filename);
			free(filename);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}
	}

	// Open directory with custom arguments
	if (test < 0 || test == 6) {
		fprintf(stderr, "file open dir in cwd\n");
		char* filename = osdialog_file(OSDIALOG_OPEN_DIR, ".", "こんにちは", NULL);
		if (filename) {
			fprintf(stderr, "\t%s\n", filename);
			free(filename);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}
	}

	// Open file with custom arguments
	if (test < 0 || test == 7) {
		fprintf(stderr, "file open in cwd\n");
		osdialog_filters* filters = osdialog_filters_parse("Source:c,cpp,m;Header:h,hpp");
		char* filename = osdialog_file(OSDIALOG_OPEN, ".", "こんにちは", filters);
		if (filename) {
			fprintf(stderr, "\t%s\n", filename);
			free(filename);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}
		osdialog_filters_free(filters);
	}

	// Save file with custom arguments
	if (test < 0 || test == 8) {
		osdialog_filters* filters = osdialog_filters_parse("Source:c,cpp,m;Header:h,hpp");
		fprintf(stderr, "file save in cwd\n");
		char* filename = osdialog_file(OSDIALOG_SAVE, ".", "こんにちは", filters);
		if (filename) {
			fprintf(stderr, "\t%s\n", filename);
			free(filename);
		}
		else {
			fprintf(stderr, "\tCanceled\n");
		}
		osdialog_filters_free(filters);
	}

	// Color selector
	if (test < 0 || test == 9) {
		int res;
		osdialog_color color = {255, 0, 255, 255};
		fprintf(stderr, "color picker\n");
		res = osdialog_color_picker(&color, 0);
		fprintf(stderr, "\t%d #%02x%02x%02x%02x\n", res, color.r, color.g, color.b, color.a);

		fprintf(stderr, "color picker with opacity\n");
		res = osdialog_color_picker(&color, 1);
		fprintf(stderr, "\t%d #%02x%02x%02x%02x\n", res, color.r, color.g, color.b, color.a);
	}

	// Async message
	if (test < 0 || test == 10) {
		fprintf(stderr, "async message\n");
		sem_init(&semaphore, 0, 0);
		osdialog_message_async(OSDIALOG_INFO, OSDIALOG_OK, "Info こんにちは", message_callback, NULL);
		sem_wait(&semaphore);
		sem_destroy(&semaphore);
	}

	// Async prompt
	if (test < 0 || test == 11) {
		fprintf(stderr, "async prompt\n");
		sem_init(&semaphore, 0, 0);
		osdialog_prompt_async(OSDIALOG_INFO, "Info", "default text", prompt_callback, NULL);
		sem_wait(&semaphore);
		sem_destroy(&semaphore);
	}

	// Async file
	if (test < 0 || test == 12) {
		fprintf(stderr, "async file\n");
		sem_init(&semaphore, 0, 0);
		osdialog_file_async(OSDIALOG_OPEN, NULL, NULL, NULL, file_callback, NULL);
		sem_wait(&semaphore);
		sem_destroy(&semaphore);
	}

	// Async color picker
	if (test < 0 || test == 13) {
		fprintf(stderr, "async color picker\n");
		sem_init(&semaphore, 0, 0);
		osdialog_color color = {255, 0, 255, 255};
		osdialog_color_picker_async(color, 1, color_picker_callback, NULL);
		sem_wait(&semaphore);
		sem_destroy(&semaphore);
	}

	fprintf(stderr, "done\n");
	return 0;
}
