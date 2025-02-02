#pragma once

#include "Log.h"

#define SETTINGS_FILE_NAME _T("TaskbarMonitor.dat")

struct SETTINGS
{
	COLORREF TextColor;

	COLORREF BackgroundColor;

	BOOL Transparent;
};

void InitSettings();

void GetSettings(SETTINGS* DestSettings);

void ShowSettingsDialog();
