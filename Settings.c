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

void ShowSettingsDialog()
{
	PickColor(NULL, &Settings.TextColor);

	SaveSettings();
}
