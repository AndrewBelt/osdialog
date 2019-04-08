#include "osdialog.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>


static char *wchar_to_utf8(const wchar_t *s) {
	if (!s) return NULL;
	int len = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
	if (!len) return NULL;
	char *r = OSDIALOG_MALLOC(len);
	WideCharToMultiByte(CP_UTF8, 0, s, -1, r, len, NULL, NULL);
	return r;
}

static wchar_t *utf8_to_wchar(const char *s) {
	if (!s) return NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
	if (!len) return NULL;
	wchar_t *r = OSDIALOG_MALLOC(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, s, -1, r, len);
	return r;
}


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	UINT type = MB_APPLMODAL;
	switch (level) {
		default:
		case OSDIALOG_INFO: type |= MB_ICONINFORMATION; break;
		case OSDIALOG_WARNING: type |= MB_ICONWARNING; break;
		case OSDIALOG_ERROR: type |= MB_ICONERROR; break;
	}

	switch (buttons) {
		default:
		case OSDIALOG_OK: type |= MB_OK; break;
		case OSDIALOG_OK_CANCEL: type |= MB_OKCANCEL; break;
		case OSDIALOG_YES_NO: type |= MB_YESNO; break;
	}

	HWND window = GetActiveWindow();
	wchar_t *messageW = utf8_to_wchar(message);
	int result = MessageBoxW(window, messageW, L"", type);
	OSDIALOG_FREE(messageW);

	switch (result) {
		case IDOK:
		case IDYES:
			return 1;
		default:
			return 0;
	}
}


char *osdialog_prompt(osdialog_message_level level, const char *message, const char *text) {
	// TODO
	(void) level;
	(void) message;
	(void) text;
	assert(0);
	return NULL;
}


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
	char *result = NULL;

	if (action == OSDIALOG_OPEN_DIR) {
		// open directory dialog
		TCHAR szDir[MAX_PATH] = "";

		BROWSEINFO bInfo;
		ZeroMemory(&bInfo, sizeof(bInfo));
		bInfo.hwndOwner = GetActiveWindow();
		bInfo.pszDisplayName = szDir;
		bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		bInfo.iImage = -1;

		LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
		if (lpItem) {
		  SHGetPathFromIDList(lpItem, szDir);
		  result = osdialog_strndup(szDir, strlen(szDir));
		}
	}
	else {
		char fBuf[4096];
		// open or save file dialog
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));

		char strFile[_MAX_PATH] = "";
		if (filename)
			snprintf(strFile, sizeof(strFile), "%s", filename);
		char *strInitialDir = NULL;
		if (path) {
			strInitialDir = osdialog_strndup(path, strlen(path));
		}

		ofn.hwndOwner = GetActiveWindow();
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = strFile;
		ofn.nMaxFile = sizeof(strFile);
		ofn.lpstrInitialDir = strInitialDir;
		ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (filters) {
			int fLen = 0;

			for (; filters; filters = filters->next) {
				fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, "%s", filters->name);
				fLen++;
				for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
					fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, "*.%s", patterns->pattern);
					if (patterns->next)
						fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, ";");
				}
				fLen++;
			}

			ofn.lpstrFilter = fBuf;
			ofn.nFilterIndex = 1;
		}

		BOOL success;
		if (action == OSDIALOG_OPEN)
			success = GetOpenFileName(&ofn);
		else
			success = GetSaveFileName(&ofn);

		if (strInitialDir)
			OSDIALOG_FREE(strInitialDir);

		if (success) {
			result = osdialog_strndup(strFile, strlen(strFile));
		}
	}

	return result;
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	(void) opacity;
	if (!color)
		return 0;

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));

	COLORREF c = RGB(color->r, color->g, color->b);
	static COLORREF acrCustClr[16];

	cc.lStructSize = sizeof(cc);
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = c;
	cc.Flags = CC_FULLOPEN | CC_ANYCOLOR | CC_RGBINIT;

	if (ChooseColor(&cc)) {
		color->r = GetRValue(cc.rgbResult);
		color->g = GetGValue(cc.rgbResult);
		color->b = GetBValue(cc.rgbResult);
		color->a = 255;
		return 1;
	}

	return 0;
}
