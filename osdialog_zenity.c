#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "osdialog.h"


static const char zenityBin[] = "/usr/bin/zenity";


static void clear_string_list(char** list) {
	while (*list) {
		OSDIALOG_FREE(*list);
		*list = NULL;
		list++;
	}
}


static int exec_string_list(const char* path, char** args) {
	// The classic fork-and-exec routine
	pid_t cid = fork();
	if (cid < 0) {
		return -1;
	}
	else if (cid == 0) {
		// child process
		int err = execv(path, args);
		if (err)
			exit(0);
	}
	else if (cid > 0) {
		// parent process
		int status = -1;
		int options = 0;
		waitpid(cid, &status, options);
		return status;
	}
	// Will never reach
	return -1;
}


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	char* args[32];
	int argIndex = 0;

	args[argIndex++] = osdialog_strdup(zenityBin);
	// The API doesn't provide a title, so just make it blank.
	args[argIndex++] = osdialog_strdup("--title");
	args[argIndex++] = osdialog_strdup("");

	if (buttons == OSDIALOG_OK_CANCEL) {
		args[argIndex++] = osdialog_strdup("--question");
		args[argIndex++] = osdialog_strdup("--ok-label");
		args[argIndex++] = osdialog_strdup("OK");
		args[argIndex++] = osdialog_strdup("--cancel-label");
		args[argIndex++] = osdialog_strdup("Cancel");
	}
	else if (buttons == OSDIALOG_YES_NO) {
		args[argIndex++] = osdialog_strdup("--question");
		args[argIndex++] = osdialog_strdup("--ok-label");
		args[argIndex++] = osdialog_strdup("Yes");
		args[argIndex++] = osdialog_strdup("--cancel-label");
		args[argIndex++] = osdialog_strdup("No");
	}
	else if (level == OSDIALOG_INFO) {
		args[argIndex++] = osdialog_strdup("--info");
	}
	else if (level == OSDIALOG_WARNING) {
		args[argIndex++] = osdialog_strdup("--warning");
	}
	else if (level == OSDIALOG_ERROR) {
		args[argIndex++] = osdialog_strdup("--error");
	}

	args[argIndex++] = osdialog_strdup("--text");
	args[argIndex++] = osdialog_strdup(message);

	args[argIndex++] = NULL;
	int ret = exec_string_list(zenityBin, args);
	clear_string_list(args);
	return ret == 0;
}


char* osdialog_prompt(osdialog_message_level level, const char* message, const char* text) {
	char* args[32];
	int argIndex = 0;

	args[argIndex++] = osdialog_strdup(zenityBin);
	args[argIndex++] = osdialog_strdup("--title");
	args[argIndex++] = osdialog_strdup("");
	// Unfortunately the level is ignored

	args[argIndex++] = NULL;
	int ret = exec_string_list(zenityBin, args);
	clear_string_list(args);
	// TODO
	return NULL;
}


char* osdialog_file(osdialog_file_action action, const char* path, const char* filename, osdialog_filters* filters) {
	char* args[32];
	int argIndex = 0;

	args[argIndex++] = osdialog_strdup(zenityBin);
	args[argIndex++] = osdialog_strdup("--title");
	args[argIndex++] = osdialog_strdup("");
	args[argIndex++] = osdialog_strdup("--file-selection");
	if (action == OSDIALOG_OPEN) {
	}
	else if (action == OSDIALOG_OPEN_DIR) {
		args[argIndex++] = osdialog_strdup("--directory");
	}
	else if (action == OSDIALOG_SAVE) {
		args[argIndex++] = osdialog_strdup("--save");
		args[argIndex++] = osdialog_strdup("--confirm-overwrite");
	}

	if (path) {
		args[argIndex++] = osdialog_strdup("--filename");
		args[argIndex++] = osdialog_strdup(path);
	}
	// TODO file filters

	args[argIndex++] = NULL;
	int ret = exec_string_list(zenityBin, args);
	clear_string_list(args);
	// TODO
	return NULL;
}


int osdialog_color_picker(osdialog_color* color, int opacity) {
	char* args[32];
	int argIndex = 0;

	args[argIndex++] = osdialog_strdup(zenityBin);
	args[argIndex++] = osdialog_strdup("--title");
	args[argIndex++] = osdialog_strdup("");
	// Unfortunately the level is ignored
	args[argIndex++] = osdialog_strdup("--color-selection");

	args[argIndex++] = NULL;
	int ret = exec_string_list(zenityBin, args);
	clear_string_list(args);
	// TODO
	return 0;
}
