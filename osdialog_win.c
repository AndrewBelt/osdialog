#include "osdialog.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, const char *filters) {
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	char strFile[_MAX_PATH] = "";
	if (filename)
		snprintf(strFile, sizeof(strFile), "%s", filename);
	char *strInitialDir = path ? strdup(path) : NULL;

	ofn.lStructSize = sizeof(ofn);
	// ofn.lpstrFilter = filters;
	// ofn.nFilterIndex = 1;
	ofn.lpstrFile = strFile;
	ofn.nMaxFile = sizeof(strFile);
	ofn.lpstrInitialDir = strInitialDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	BOOL success;
	if (action == OSDIALOG_OPEN)
		success = GetOpenFileName(&ofn);
	else
		success = GetSaveFileName(&ofn);

	if (strInitialDir)
		free(strInitialDir);
	return success ? strdup(strFile) : NULL;
}


void osdialog_color_picker() {
	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));

	static COLORREF acrCustClr[16];

	cc.lStructSize = sizeof(cc);
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	ChooseColor(&cc);
}