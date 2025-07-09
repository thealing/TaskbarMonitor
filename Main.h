#pragma once

#include "Counters.h"

#include "Settings.h"

#define LOG_FILE_NAME _T("TaskbarMonitor.log")

#define MONITOR_WINDOW_CLASS _T("TaskbarMonitorWindowClass")

#define TASKBAR_WINDOW_CLASS _T("Shell_TrayWnd")

#define APP_CONTAINER_WINDOW_CLASS _T("MSTaskSwWClass")

#define FONT_NAME _T("Calibri")

#define UPDATE_INTERVAL 100

#define MEASURE_INTERVAL 1000

#define PADDING 3

#define SIZE_RATIO 750

#define SIZE_CPU_MEM_COLUMN 30

#define SIZE_DISK_IO_COLUMN 36

#define SIZE_NETWORK_COLUMN 36

#define SIZE_DENOM 100

enum QUIT_CODE 
{
	QUIT_ERROR,

	QUIT_CLOSED,

	QUIT_CHANGED,

	QUIT_RESTART,

	QUIT_COUNT
};

int Run();

QUIT_CODE InjectWindow();

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

BOOL InsertMonitorWindow(HWND AppContainerWindow, HWND MonitorWindow);

HWND FindAppContainerWindow();

void SafeRelease(IUnknown* Unknown);

void GetRectSize(const RECT* Rect, SIZE* Size);

void FormatDataQuantity(TCHAR* Buffer, double Value);
