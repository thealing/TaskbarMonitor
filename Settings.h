#pragma once

#include "Log.h"

#define SETTINGS_FILE_NAME _T("TaskbarMonitor.dat")

struct SETTINGS
{
	COLORREF TextColor;

	COLORREF BackgroundColor;

	BOOL Transparent;

	BOOL ShowCpuAndMemoryUsage;

	BOOL ShowNetworkTrafficSpeeds;

	BOOL ShowDiskIoSpeeds;
};

enum MENU_OPTION
{
	MENU_OPTION_NONE,

	MENU_CHANGE_TEXT_COLOR,

	MENU_CHANGE_BACKGROUND_COLOR,

	MENU_TOGGLE_TRANSPARENCY,

	MENU_TOGGLE_CPU_MEM,

	MENU_TOGGLE_NETWORK_SPEED,

	MENU_TOGGLE_DISK_SPEED,

	MENU_CLOSE
};

void InitSettings();

void GetSettings(SETTINGS* DestSettings);

BOOL ShowSettingsDialog(HWND Window);
