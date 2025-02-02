#pragma once

#include "Counters.h"

#include "Log.h"

#define WINDOW_CLASS_NAME _T("A0EBA48E-45B2-4AB9-84D8-286CA6C89637")

#define TASKBAR_WINDOW_TEXT _T("Shell_TrayWnd")

#define RUNNING_APPS_TEXT _T("Running applications")

#define FONT_NAME _T("Calibri")

#define UPDATE_INTERVAL 1000

#define PADDING 3

enum QUIT_CODE 
{
	QUIT_ERROR,

	QUIT_CLOSED,

	QUIT_CHANGED,

	QUIT_COUNT
};

QUIT_CODE RunWindow();

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

HWND FindTaskbarWindow();

HWND FindAppContainerWindow(HWND Window);

BOOL IsRunningAppsWindow(HWND Window);

void GetRectSize(const RECT* Rect, SIZE* Size);

void FormatDataQuantity(TCHAR* Buffer, double Value);
