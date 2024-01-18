#include <AppKit/AppKit.h>
#include <Availability.h>
#include "osdialog.h"


extern osdialog_save_callback osdialog_save_cb;
extern osdialog_restore_callback osdialog_restore_cb;

#define SAVE_CALLBACK \
	void* cb_ptr = NULL; \
	if (osdialog_save_cb) { \
		cb_ptr = osdialog_save_cb(); \
	}

#define RESTORE_CALLBACK \
	if (osdialog_restore_cb) { \
		osdialog_restore_cb(cb_ptr); \
	}


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	@autoreleasepool {
		SAVE_CALLBACK

		NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];

		NSAlert* alert = [[NSAlert alloc] init];

		switch (level) {
			default:
#ifdef __MAC_10_12
			case OSDIALOG_INFO: [alert setAlertStyle:NSAlertStyleInformational]; break;
			case OSDIALOG_WARNING: [alert setAlertStyle:NSAlertStyleWarning]; break;
			case OSDIALOG_ERROR: [alert setAlertStyle:NSAlertStyleCritical]; break;
#else
			case OSDIALOG_INFO: [alert setAlertStyle:NSInformationalAlertStyle]; break;
			case OSDIALOG_WARNING: [alert setAlertStyle:NSWarningAlertStyle]; break;
			case OSDIALOG_ERROR: [alert setAlertStyle:NSCriticalAlertStyle]; break;
#endif
		}

		switch (buttons) {
			default:
			case OSDIALOG_OK:
				[alert addButtonWithTitle:@"OK"];
				break;
			case OSDIALOG_OK_CANCEL:
				[alert addButtonWithTitle:@"OK"];
				[alert addButtonWithTitle:@"Cancel"];
				break;
			case OSDIALOG_YES_NO:
				[alert addButtonWithTitle:@"Yes"];
				[alert addButtonWithTitle:@"No"];
				break;
		}

		NSString* messageString = [NSString stringWithUTF8String:message];
		[alert setMessageText:messageString];
		// Non-bold text
		// [alert setInformativeText:messageString];

		NSInteger button = [alert runModal];

		[keyWindow makeKeyAndOrderFront:nil];
		bool success = (button == NSAlertFirstButtonReturn);

		RESTORE_CALLBACK

		return success;
	} // @autoreleasepool
}


char* osdialog_prompt(osdialog_message_level level, const char* message, const char* text) {
	@autoreleasepool {

		SAVE_CALLBACK

		NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];

		NSAlert* alert = [[NSAlert alloc] init];

		switch (level) {
			default:
#ifdef __MAC_10_12
			case OSDIALOG_INFO: [alert setAlertStyle:NSAlertStyleInformational]; break;
			case OSDIALOG_WARNING: [alert setAlertStyle:NSAlertStyleWarning]; break;
			case OSDIALOG_ERROR: [alert setAlertStyle:NSAlertStyleCritical]; break;
#else
			case OSDIALOG_INFO: [alert setAlertStyle:NSInformationalAlertStyle]; break;
			case OSDIALOG_WARNING: [alert setAlertStyle:NSWarningAlertStyle]; break;
			case OSDIALOG_ERROR: [alert setAlertStyle:NSCriticalAlertStyle]; break;
#endif
		}

		[alert addButtonWithTitle:@"OK"];
		[alert addButtonWithTitle:@"Cancel"];

		NSString* messageString = [NSString stringWithUTF8String:message];
		[alert setMessageText:messageString];

		NSTextField* input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
		[alert setAccessoryView:input];

		if (text) {
			NSString* path_str = [NSString stringWithUTF8String:text];
			[input setStringValue:path_str];
		}

		NSInteger button = [alert runModal];

		char* result = NULL;
		if (button == NSAlertFirstButtonReturn) {
			[input validateEditing];
			NSString* result_str = [input stringValue];
			// Don't use NSString.length because it returns the number of the UTF-16 code units, not the number of bytes.
			result = osdialog_strdup([result_str UTF8String]);
		}

		[keyWindow makeKeyAndOrderFront:nil];

		RESTORE_CALLBACK

		return result;
	} // @autoreleasepool
}


char* osdialog_file(osdialog_file_action action, const char* dir, const char* filename, osdialog_filters* filters) {
	@autoreleasepool {

		SAVE_CALLBACK

		NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];

		NSSavePanel* panel;
		// NSOpenPanel is a subclass of NSSavePanel. Not defined for OSDIALOG_SAVE.
		NSOpenPanel* open_panel;

		if (action == OSDIALOG_OPEN || action == OSDIALOG_OPEN_DIR) {
			panel = open_panel = [NSOpenPanel openPanel];
		}
		else {
			panel = [NSSavePanel savePanel];
		}

		// Bring dialog to front
		// https://stackoverflow.com/a/2402069
		// Thanks Dave!
		[panel setLevel:CGShieldingWindowLevel()];

		if (filters) {
			NSMutableArray* fileTypes = [[NSMutableArray alloc] init];

			for (; filters; filters = filters->next) {
				for (osdialog_filter_patterns* patterns = filters->patterns; patterns; patterns = patterns->next) {
					NSString* fileType = [NSString stringWithUTF8String:patterns->pattern];
					[fileTypes addObject:fileType];
				}
			}

			[panel setAllowedFileTypes:fileTypes];
		}

		if (action == OSDIALOG_OPEN || action == OSDIALOG_OPEN_DIR) {
			[open_panel setAllowsMultipleSelection:NO];
		}
		if (action == OSDIALOG_OPEN) {
			[open_panel setCanChooseDirectories:NO];
			[open_panel setCanChooseFiles:YES];
		}
		if (action == OSDIALOG_OPEN_DIR) {
			[open_panel setCanCreateDirectories:YES];
			[open_panel setCanChooseDirectories:YES];
			[open_panel setCanChooseFiles:NO];
		}

		if (dir) {
			NSString* dir_str = [NSString stringWithUTF8String:dir];
			NSURL* dir_url = [NSURL fileURLWithPath:dir_str];
			[panel setDirectoryURL:dir_url];
		}

		if (filename) {
			NSString* filenameString = [NSString stringWithUTF8String:filename];
			[panel setNameFieldStringValue:filenameString];
		}

		char* result = NULL;

		NSModalResponse response = [panel runModal];
#ifdef __MAC_10_9
#define OK NSModalResponseOK
#else
#define OK NSOKButton
#endif
		if (response == OK) {
			NSURL* result_url = [panel URL];
			NSString* result_str = [result_url path];
			// Don't use NSString.length because it returns the number of the UTF-16 code units, not the number of bytes.
			result = osdialog_strdup([result_str UTF8String]);
		}

		[keyWindow makeKeyAndOrderFront:nil];

		RESTORE_CALLBACK

		return result;
	} // @autoreleasepool
}


int osdialog_color_picker(osdialog_color* color, int opacity) {
	if (!color)
		return 0;

	@autoreleasepool {

		SAVE_CALLBACK

		NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];

		// Set default picker tab
		// [NSColorPanel setPickerMode:NSColorPanelModeWheel];

		// Get color panel instance
		NSColorPanel* panel = [NSColorPanel sharedColorPanel];

		// Set color
		NSColor* c = [NSColor colorWithCalibratedRed:color->r / 255.f green:color->g / 255.f blue:color->b / 255.f alpha:color->a / 255.f];
		[panel setColor:c];
		[panel setShowsAlpha:(bool) opacity];

		// Run panel as a modal window
		NSModalSession modal = [NSApp beginModalSessionForWindow:panel];

		// Wait until user hides modal with X
		for (;;) {
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
			if ([NSApp runModalSession:modal] != NSModalResponseContinue)
				break;
			if (![panel isVisible])
				break;
		}

		[NSApp endModalSession:modal];

		// Get color
		c = [panel color];
		NSColor* cRGB = [c colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
		CGFloat r, g, b, a;
		[cRGB getRed:&r green:&g blue:&b alpha:&a];
		color->r = r * 255.f;
		color->g = g * 255.f;
		color->b = b * 255.f;
		color->a = a * 255.f;

		[keyWindow makeKeyAndOrderFront:nil];

		RESTORE_CALLBACK

		// Always accept user choice
		return 1;
	} // @autoreleasepool
}
