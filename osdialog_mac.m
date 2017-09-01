#include "osdialog.h"
#include <AppKit/AppKit.h>
#include <Availability.h>


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, const char *filters) {
	NSSavePanel *panel;
	NSOpenPanel *open_panel;

	// Memory leaks everywhere. Someone please help!

	if (action == OSDIALOG_OPEN) {
		open_panel = [NSOpenPanel openPanel];
		panel = open_panel;
	}
	else {
		panel = [NSSavePanel savePanel];
	}

	// Bring dialog to front
	// https://stackoverflow.com/a/2402069
	// Thanks Dave!
	[panel setLevel:CGShieldingWindowLevel()];

	if (action == OSDIALOG_OPEN) {
		open_panel.allowsMultipleSelection = NO;
		open_panel.canChooseDirectories = NO;
		open_panel.canChooseFiles = YES;
	}

	if (path) {
		NSString *path_str = [NSString stringWithUTF8String:path];
		NSURL *path_url = [NSURL fileURLWithPath:path_str];
		panel.directoryURL = path_url;
		// [path_url release];
		// [path_str release];
	}

	if (filename) {
		NSString *filename_str = [NSString stringWithUTF8String:filename];
		panel.nameFieldStringValue = filename_str;
		// [filename_str release];
	}

	char *result = NULL;

#ifdef __MAC_10_9
	#define OK NSModalResponseOK
#else
	#define OK NSOKButton
#endif
	if ([panel runModal] == OK) {
		NSURL *result_url = [panel URL];
		result = strdup([[result_url path] UTF8String]);
		[result_url release];
	}

	// [panel release];
	return result;
}


void osdialog_color_picker() {
	NSColorPanel *panel = [NSColorPanel sharedColorPanel];
	[panel isVisible];
	// [panel setShowAlpha:NO];
	// NSColorWell *well = ;
	// well.color = [NSColor randomColor];
}
