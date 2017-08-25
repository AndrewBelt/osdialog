#include "osdialog.h"
#include <AppKit/AppKit.h>
#include <Availability.h>


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, const char *filters) {
	NSSavePanel *panel;
	NSOpenPanel *open_panel;

	if (action == OSDIALOG_OPEN) {
		panel = open_panel = [NSOpenPanel openPanel];
	}
	else {
		panel = [NSSavePanel savePanel];
	}

	// Bring dialog to front
	// https://stackoverflow.com/a/2402069
	// Thanks Dave!
	[panel setLevel:CGShieldingWindowLevel()];

	// Enable the selection of files in the dialog.
	[panel setCanChooseFiles:YES];

	// Multiple files not allowed
	[panel setAllowsMultipleSelection:NO];

	// Can't select a directory
	[panel setCanChooseDirectories:NO];

	NSModalResponse success =
#ifdef __MAC_10_9
	NSModalResponseOK;
#else
	NSOKButton;
#endif
	if ([panel runModal] == success) {
		
	}
	return NULL;
}


void osdialog_color_picker() {
}
