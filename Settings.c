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

	LoadSettings();

	SaveSettings();
}

void GetSettings(SETTINGS* DestSettings)
{
	memcpy(DestSettings, &Settings, sizeof(SETTINGS));
}

BOOL ShowSettingsDialog(HWND Window)
{
	HMENU Menu = CreatePopupMenu();

	if (Menu == NULL)
	{
		return FALSE;
	}

	AppendMenu(Menu, MF_STRING, MENU_CHANGE_TEXT_COLOR, TEXT("Change text color"));

	AppendMenu(Menu, MF_STRING, MENU_CHANGE_BACKGROUND_COLOR, TEXT("Change background color"));

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.Transparent, MENU_TOGGLE_TRANSPARENCY, TEXT("Transparent background"));

	AppendMenu(Menu, MF_SEPARATOR, 0, NULL);

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.ShowCpuAndMemoryUsage, MENU_TOGGLE_CPU_MEM, TEXT("Show CPU and memory usage"));

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.ShowNetworkTrafficSpeeds, MENU_TOGGLE_NETWORK_SPEED, TEXT("Show network traffic speeds"));

	AppendMenu(Menu, MF_STRING | MF_CHECKED * Settings.ShowDiskIoSpeeds, MENU_TOGGLE_DISK_SPEED, TEXT("Show disk read and write speeds"));

	AppendMenu(Menu, MF_SEPARATOR, 0, NULL);

	AppendMenu(Menu, MF_STRING, MENU_CLOSE, TEXT("Close"));

	POINT CursorPosition;

	GetCursorPos(&CursorPosition);

	MENU_OPTION Option = TrackPopupMenu(Menu, TPM_RETURNCMD, CursorPosition.x, CursorPosition.y, 0, Window, NULL);

	DestroyMenu(Menu);

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
		case MENU_TOGGLE_NETWORK_SPEED:
		{
			Settings.ShowNetworkTrafficSpeeds ^= TRUE;

			break;
		}
		case MENU_TOGGLE_DISK_SPEED:
		{
			Settings.ShowDiskIoSpeeds ^= TRUE;

			break;
		}
		case MENU_CLOSE:
		{
			SendMessage(Window, WM_CLOSE, 0, 0);

			return FALSE;
		}
	}

	SaveSettings();

	return TRUE;
}
