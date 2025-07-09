#pragma once

#include <Windows.h>

#include <psapi.h>

#include <iphlpapi.h>

#include <assert.h>

#include <tchar.h>

#include <stdio.h>

#include <uxtheme.h>

#include <UiAutomationclient.h>

#pragma comment(lib, "iphlpapi.lib")

#pragma comment(lib, "UxTheme.lib")

#pragma comment(lib, "Uiautomationcore.lib")

typedef enum LOG_LEVEL LOG_LEVEL;

typedef enum QUIT_CODE QUIT_CODE;

typedef enum MENU_OPTION MENU_OPTION;

typedef struct SETTINGS SETTINGS;
