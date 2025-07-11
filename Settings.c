#include "Settings.h"

static SETTINGS Settings;

static void PickColor(HWND Window, COLORREF* Color)
{
	static COLORREF CustomColors[16];

	CHOOSECOLOR ColorChoose = { sizeof(ColorChoose) };

	ColorChoose.hwndOwner = Window;

	ColorChoose.rgbResult = *Color;

	ColorChoose.lpCustColors = (LPDWORD)CustomColors;

	ColorChoose.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&ColorChoose))
	{
		*Color = ColorChoose.rgbResult;
	}
}

static void LoadSettings()
{
	FILE* File = _tfopen(SETTINGS_FILE_NAME, _T("rb"));

	if (File == NULL)
	{
		LogMessage(LOG_WARNING, _T("Failed to read the settings file."));

		return;
	}

	fread(&Settings, 1, sizeof(Settings), File);

	fclose(File);
}

static void SaveSettings()
{
	FILE* File = _tfopen(SETTINGS_FILE_NAME, _T("wb"));

	if (File == NULL)
	{
		LogMessage(LOG_WARNING, _T("Failed to write the settings file."));

		return;
	}

	fwrite(&Settings, 1, sizeof(Settings), File);

	fclose(File);
}

void InitSettings()
{
	Settings.TextColor = RGB(255, 255, 255);

	Settings.BackgroundColor = RGB(64, 64, 64);

	Settings.Transparent = TRUE;

	Settings.ShowCpuAndMemoryUsage = TRUE;

	Settings.ShowDiskIoSpeeds = TRUE;

	Settings.ShowNetworkTrafficSpeeds = TRUE;

	LoadSettings();

	SaveSettings();
}

void GetSettings(SETTINGS* DestSettings)
{
	memcpy(DestSettings, &Settings, sizeof(Settings));
}

BOOL ShowSettingsDialog(HWND Window)
{
	HMENU Menu = CreatePopupMenu();

	if (Menu == NULL)
	{
		return FALSE;
	}

	AppendMenu(Menu, MF_STRING, MENU_CHANGE_TEXT_COLOR, _T("Change text color"));

	AppendMenu(Menu, MF_STRING, MENU_CHANGE_BACKGROUND_COLOR, _T("Change background color"));

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.Transparent, MENU_TOGGLE_TRANSPARENCY, _T("Transparent background"));

	AppendMenu(Menu, MF_SEPARATOR, 0, NULL);

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.ShowCpuAndMemoryUsage, MENU_TOGGLE_CPU_MEM, _T("Show CPU and memory usage"));

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.ShowDiskIoSpeeds, MENU_TOGGLE_DISK_SPEED, _T("Show disk read and write speeds"));

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.ShowNetworkTrafficSpeeds, MENU_TOGGLE_NETWORK_SPEED, _T("Show network traffic speeds"));

	AppendMenu(Menu, MF_SEPARATOR, 0, NULL);

	AppendMenu(Menu, MF_STRING, MENU_CLOSE, _T("Close"));

	POINT CursorPosition;

	GetCursorPos(&CursorPosition);

	MENU_OPTION Option = TrackPopupMenu(Menu, TPM_RETURNCMD, CursorPosition.x, CursorPosition.y, 0, Window, NULL);

	DestroyMenu(Menu);

	SETTINGS OldSettings = Settings;

	switch (Option)
	{
		case MENU_OPTION_NONE:
		{
			return FALSE;
		}
		case MENU_CHANGE_TEXT_COLOR:
		{
			PickColor(Window, &Settings.TextColor);

			break;
		}
		case MENU_CHANGE_BACKGROUND_COLOR:
		{
			PickColor(Window, &Settings.BackgroundColor);

			break;
		}
		case MENU_TOGGLE_TRANSPARENCY:
		{
			Settings.Transparent ^= TRUE;

			break;
		}
		case MENU_TOGGLE_CPU_MEM:
		{
			Settings.ShowCpuAndMemoryUsage ^= TRUE;

			break;
		}
		case MENU_TOGGLE_DISK_SPEED:
		{
			Settings.ShowDiskIoSpeeds ^= TRUE;

			break;
		}
		case MENU_TOGGLE_NETWORK_SPEED:
		{
			Settings.ShowNetworkTrafficSpeeds ^= TRUE;

			break;
		}
		case MENU_CLOSE:
		{
			SendMessage(Window, WM_CLOSE, 0, 0);

			return FALSE;
		}
	}

	if (Settings.ShowCpuAndMemoryUsage == FALSE && Settings.ShowDiskIoSpeeds == FALSE && Settings.ShowNetworkTrafficSpeeds == FALSE)
	{
		Settings = OldSettings;

		MessageBox(Window, _T("At least one column must be selected!"), _T("Warning"), MB_ICONWARNING);
	}

	if (memcmp(&OldSettings, &Settings, sizeof(Settings)) != 0)
	{
		SaveSettings();

		return TRUE;
	}

	return FALSE;
}
