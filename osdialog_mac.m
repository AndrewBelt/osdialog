#include <AppKit/AppKit.h>
#include <Availability.h>
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

#ifdef __MAC_10_9
	#define OK NSModalResponseOK
#else
	#define OK NSOKButton
#endif


/** Caller must release
*/
static NSAlert* message_create(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
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

	if (message) {
		NSString* messageString = [NSString stringWithUTF8String:message];
		[alert setMessageText:messageString];
	}

	// Non-bold text
	// [alert setInformativeText:messageString];

	return alert;
}


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char* message) {
	SAVE_CALLBACK

	NSAlert* alert = message_create(level, buttons, message);

	NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
	NSInteger response = [alert runModal];
	[alert release];
	[keyWindow makeKeyAndOrderFront:nil];

	RESTORE_CALLBACK

	bool success = (response == NSAlertFirstButtonReturn);
	return success;
}


void osdialog_message_async(osdialog_message_level level, osdialog_message_buttons buttons, const char* message, osdialog_message_callback* cb, void* user) {
	SAVE_CALLBACK

	NSAlert* alert = message_create(level, buttons, message);

	NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
	[alert beginSheetModalForWindow:keyWindow completionHandler:^(NSModalResponse response) {
		// Must reactivate key window on the main queue for some reason
		dispatch_async(dispatch_get_main_queue(), ^{
			[keyWindow makeKeyAndOrderFront:nil];
		});

		RESTORE_CALLBACK

		bool success = (response == NSAlertFirstButtonReturn);
		if (cb) {
			cb(success, user);
		}
	}];
	[alert release];
}


/** Caller must release
*/
static NSAlert* prompt_create(osdialog_message_level level, const char* message, const char* text) {
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

	if (message) {
		NSString* messageString = [NSString stringWithUTF8String:message];
		[alert setMessageText:messageString];
	}

	NSTextField* input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
	[alert setAccessoryView:input];

	if (text) {
		NSString* path_str = [NSString stringWithUTF8String:text];
		[input setStringValue:path_str];
	}

	return alert;
}


static char* prompt_text_get(NSAlert* alert) {
	NSTextField* input = (NSTextField*) [alert accessoryView];
	[input validateEditing];
	NSString* result_str = [input stringValue];
	// Don't use NSString.length because it returns the number of the UTF-16 code units, not the number of bytes.
	const char* result = [result_str UTF8String];
	return osdialog_strdup(result);
}


char* osdialog_prompt(osdialog_message_level level, const char* message, const char* text) {
	SAVE_CALLBACK

	NSAlert* alert = prompt_create(level, message, text);

	NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
	NSInteger response = [alert runModal];
	[keyWindow makeKeyAndOrderFront:nil];

	RESTORE_CALLBACK

	char* result = NULL;
	if (response == NSAlertFirstButtonReturn) {
		result = prompt_text_get(alert);
	}
	[alert release];
	return result;
}


void osdialog_prompt_async(osdialog_message_level level, const char* message, const char* text, osdialog_prompt_callback* cb, void* user) {
	SAVE_CALLBACK

	NSAlert* alert = prompt_create(level, message, text);

	NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
	[alert beginSheetModalForWindow:keyWindow completionHandler:^(NSModalResponse response) {
		// Must reactivate key window on the main queue for some reason
		dispatch_async(dispatch_get_main_queue(), ^{
			[keyWindow makeKeyAndOrderFront:nil];
		});

		RESTORE_CALLBACK

		if (!cb)
			return;

		char* result = NULL;
		if (response == NSAlertFirstButtonReturn) {
			result = prompt_text_get(alert);
		}
		cb(result, user);
	}];
	[alert release];
}


static NSSavePanel* file_create(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters) {
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
			for (const osdialog_filter_patterns* patterns = filters->patterns; patterns; patterns = patterns->next) {
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

	return panel;
}


static char* file_path_get(NSSavePanel* panel) {
	NSURL* result_url = [panel URL];
	NSString* result_str = [result_url path];
	const char* result = [result_str UTF8String];
	// Don't use NSString.length because it returns the number of the UTF-16 code units, not the number of bytes.
	return osdialog_strdup(result);
}


char* osdialog_file(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters) {
	SAVE_CALLBACK

	NSSavePanel* panel = file_create(action, dir, filename, filters);

	NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
	NSModalResponse response = [panel runModal];

	[keyWindow makeKeyAndOrderFront:nil];

	RESTORE_CALLBACK

	char* result = NULL;
	if (response == OK) {
		result = file_path_get(panel);
	}
	return result;
}


void osdialog_file_async(osdialog_file_action action, const char* dir, const char* filename, const osdialog_filters* filters, osdialog_file_callback* cb, void* user) {
	SAVE_CALLBACK

	NSSavePanel* panel = file_create(action, dir, filename, filters);

	NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];
	[panel beginSheetModalForWindow:keyWindow completionHandler:^(NSModalResponse response) {
		// Must reactivate key window on the main queue for some reason
		dispatch_async(dispatch_get_main_queue(), ^{
			[keyWindow makeKeyAndOrderFront:nil];
		});

		RESTORE_CALLBACK

		if (!cb)
			return;

		char* result = NULL;
		if (response == OK) {
			result = file_path_get(panel);
		}
		cb(result, user);
	}];
}


static NSColorPanel* color_picker_create(osdialog_color color, int opacity) {
	// Set default picker tab
	// [NSColorPanel setPickerMode:NSColorPanelModeWheel];

	// Get color panel instance
	NSColorPanel* panel = [NSColorPanel sharedColorPanel];

	// Set color
	NSColor* c = [NSColor colorWithCalibratedRed:color.r / 255.f green:color.g / 255.f blue:color.b / 255.f alpha:color.a / 255.f];
	[panel setColor:c];
	[panel setShowsAlpha:(bool) opacity];

	return panel;
}


static osdialog_color color_picker_get(NSColorPanel* panel) {
	NSColor* c = [panel color];
	NSColor* cRGB = [c colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
	CGFloat r, g, b, a;
	[cRGB getRed:&r green:&g blue:&b alpha:&a];

	osdialog_color color;
	color.r = r * 255.f;
	color.g = g * 255.f;
	color.b = b * 255.f;
	color.a = a * 255.f;
	return color;
}


int osdialog_color_picker(osdialog_color* color, int opacity) {
	if (!color)
		return 0;

	SAVE_CALLBACK

	NSColorPanel* panel = color_picker_create(*color, opacity);

	NSWindow* keyWindow = [[NSApplication sharedApplication] keyWindow];

	// Run panel as a modal window
	NSModalSession modal = [NSApp beginModalSessionForWindow:panel];

	// Wait until user hides modal with X
	while (true) {
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
		if ([NSApp runModalSession:modal] != NSModalResponseContinue)
			break;
		if (![panel isVisible])
			break;
	}

	[NSApp endModalSession:modal];
	[keyWindow makeKeyAndOrderFront:nil];

	RESTORE_CALLBACK

	*color = color_picker_get(panel);

	// Always accept user choice
	return 1;
}


void osdialog_color_picker_async(osdialog_color color, int opacity, osdialog_color_picker_callback* cb, void* user) {
	// Fake async placeholder
	int result = osdialog_color_picker(&color, opacity);
	if (cb) {
		cb(result, color, user);
	}
}
