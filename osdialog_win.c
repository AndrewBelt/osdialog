#include "osdialog.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <shlobj.h>


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, const char *filters) {
	if (action == OSDIALOG_OPEN_DIR) {
		// open directory dialog
		TCHAR szDir[MAX_PATH] = "";

		BROWSEINFO bInfo;
		ZeroMemory(&bInfo, sizeof(bInfo));
		bInfo.hwndOwner = NULL;
		bInfo.pidlRoot = NULL; 
		bInfo.pszDisplayName = szDir;
		bInfo.lpszTitle = NULL;
		bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		bInfo.lpfn = NULL;
		bInfo.lParam = 0;
		bInfo.iImage = -1;

		LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
		if (lpItem) {
		  SHGetPathFromIDList(lpItem, szDir);
			return strdup(szDir);
		}
		else {
			return NULL;
		}
	}
	else {
		// open or save file dialog
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