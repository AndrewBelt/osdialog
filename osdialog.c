#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "osdialog.h"


char* osdialog_strdup(const char* s) {
	if (!s)
		return NULL;
	return osdialog_strndup(s, strlen(s));
}


char* osdialog_strndup(const char* s, size_t n) {
	if (!s)
		return NULL;
	char* d = OSDIALOG_MALLOC(n + 1);
	memcpy(d, s, n);
	d[n] = '\0';
	return d;
}


osdialog_filters* osdialog_filters_parse(const char* str) {
	if (!str)
		return NULL;

	osdialog_filters* filters_head = OSDIALOG_MALLOC(sizeof(osdialog_filters));
	filters_head->name = NULL;
	filters_head->patterns = NULL;
	filters_head->next = NULL;

	osdialog_filters* filters = filters_head;
	osdialog_filter_patterns* patterns = NULL;

	const char* text = str;
	while (1) {
		char c = *str;
		if (!patterns) {
			// Scan filter name
			if (c == ':') {
				// End of filter name
				filters->name = osdialog_strndup(text, str - text);
				filters->patterns = OSDIALOG_MALLOC(sizeof(osdialog_filter_patterns));
				patterns = filters->patterns;
				patterns->pattern = NULL;
				patterns->next = NULL;
				text = str + 1;
			}
			else if (c == '\0') {
				// Filter has no pattern, leave as NULL
				filters = NULL;
				break;
			}
		}
		else {
			// Scan pattern
			if (c == ',') {
				// Next pattern
				patterns->pattern = osdialog_strndup(text, str - text);
				patterns->next = OSDIALOG_MALLOC(sizeof(osdialog_filter_patterns));
				patterns = patterns->next;
				patterns->pattern = NULL;
				patterns->next = NULL;
				text = str + 1;
			}
			else if (c == ';') {
				// End of patterns
				patterns->pattern = osdialog_strndup(text, str - text);
				patterns = NULL;
				filters->next = OSDIALOG_MALLOC(sizeof(osdialog_filters));
				filters = filters->next;
				filters->name = NULL;
				filters->patterns = NULL;
				filters->next = NULL;
				text = str + 1;
			}
			else if (c == '\0') {
				// End of string
				patterns->pattern = osdialog_strndup(text, str - text);
				patterns = NULL;
				break;
			}
		}
		str++;
	}

	return filters_head;
}


void osdialog_filter_patterns_free(osdialog_filter_patterns* patterns) {
	if (!patterns)
		return;
	OSDIALOG_FREE(patterns->pattern);
	osdialog_filter_patterns* next = patterns->next;
	OSDIALOG_FREE(patterns);
	osdialog_filter_patterns_free(next);
}


void osdialog_filters_free(osdialog_filters* filters) {
	if (!filters)
		return;
	OSDIALOG_FREE(filters->name);
	osdialog_filter_patterns_free(filters->patterns);
	osdialog_filters* next = filters->next;
	OSDIALOG_FREE(filters);
	osdialog_filters_free(next);
}


static osdialog_filter_patterns* osdialog_filter_patterns_copy(const osdialog_filter_patterns* src) {
	if (!src)
		return NULL;

	osdialog_filter_patterns* dest = OSDIALOG_MALLOC(sizeof(osdialog_filter_patterns));
	dest->pattern = osdialog_strdup(src->pattern);
	dest->next = osdialog_filter_patterns_copy(src->next);
	return dest;
}


osdialog_filters* osdialog_filters_copy(const osdialog_filters* src) {
	if (!src)
		return NULL;

	osdialog_filters* dest = OSDIALOG_MALLOC(sizeof(osdialog_filters));
	dest->name = osdialog_strdup(src->name);
	dest->patterns = osdialog_filter_patterns_copy(src->patterns);
	dest->next = osdialog_filters_copy(src->next);
	return dest;
}


osdialog_save_callback* osdialog_save_cb = NULL;
osdialog_restore_callback* osdialog_restore_cb = NULL;


void osdialog_set_save_callback(osdialog_save_callback* cb) {
	osdialog_save_cb = cb;
}


void osdialog_set_restore_callback(osdialog_restore_callback* cb) {
	osdialog_restore_cb = cb;
}
