#include "main.h"

int main()
{
	return Program();
}

int WINAPI WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ LPSTR CommandLine, _In_ int ShowMode)
{
	return Program();
}

int Program()
{
	LogSetFile(LOG_FILE_NAME);

	InitSettings();

	WNDCLASS WindowClass = { 0 };

	WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	WindowClass.lpszClassName = WINDOW_CLASS_NAME;

	WindowClass.lpfnWndProc = WindowProcedure;

	if (RegisterClass(&WindowClass) == 0)
	{
		LogMessage(LOG_ERROR, _T("Failed to register the window class."));

		return 1;
	}

	while (TRUE)
	{
		QUIT_CODE QuitCode = RunWindow();

		if (QuitCode == QUIT_ERROR)
		{
			Sleep(500);
		}

		if (QuitCode == QUIT_CLOSED)
		{
			break;
		}
	}

	return 0;
}

QUIT_CODE RunWindow()
{
	HWND TaskbarWindow = FindTaskbarWindow();

	if (TaskbarWindow == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to find the taskbar window."));

		return QUIT_ERROR;
	}

	HWND AppContainerWindow = FindAppContainerWindow(TaskbarWindow);

	if (AppContainerWindow == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to find the app container window."));

		return QUIT_ERROR;
	}

	RECT AppContainerRect;

	if (GetWindowRect(AppContainerWindow, &AppContainerRect) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to get the dimensions of the container window."));

		return QUIT_ERROR;
	}

	HWND ParentWindow = GetParent(AppContainerWindow);

	if (ParentWindow == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to get the parent window."));

		return QUIT_ERROR;
	}

	RECT ParentRect;

	if (GetWindowRect(ParentWindow, &ParentRect) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to get the dimensions of the parent window."));

		return QUIT_ERROR;
	}

	SIZE AppContainerSize;

	GetRectSize(&AppContainerRect, &AppContainerSize);

	int WindowHeight = AppContainerSize.cy - PADDING * 2;

	SETTINGS Settings;

	GetSettings(&Settings);

	int TotalWidth = 0;

	TotalWidth += SIZE_CPU_MEM_COLUMN * Settings.ShowCpuAndMemoryUsage;

	TotalWidth += SIZE_DISK_IO_COLUMN * Settings.ShowDiskIoSpeeds;

	TotalWidth += SIZE_NETWORK_COLUMN * Settings.ShowNetworkTrafficSpeeds;

	int WindowWidth = WindowHeight * SIZE_RATIO / SIZE_DENOM * TotalWidth / SIZE_DENOM;

	SIZE ReducedSize = { AppContainerSize.cx - WindowWidth - PADDING * 2, AppContainerSize.cy };

	if (SetWindowPos(AppContainerWindow, NULL, 0, 0, ReducedSize.cx, ReducedSize.cy, SWP_NOZORDER | SWP_NOMOVE) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to adjust the size of the app container window."));

		return QUIT_ERROR;
	}

	int WindowX = AppContainerRect.left - ParentRect.left + ReducedSize.cx + PADDING;

	int WindowY = AppContainerRect.top - ParentRect.top + PADDING;

	HWND Window = CreateWindow(WINDOW_CLASS_NAME, NULL, WS_CHILD | WS_VISIBLE, WindowX, WindowY, WindowWidth, WindowHeight, ParentWindow, NULL, NULL, NULL);

	if (Window == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to create the main window."));

		return QUIT_ERROR;
	}

	UINT_PTR TimerId = SetTimer(Window, TRUE, UPDATE_INTERVAL, NULL);

	if (TimerId == 0)
	{
		LogMessage(LOG_ERROR, _T("Failed to set a timer."));

		return QUIT_ERROR;
	}

	LogMessage(LOG_INFO, _T("Entering the message loop."));

	MSG Msg;

	while (GetMessage(&Msg, Window, 0, 0) > 0)
	{
		TranslateMessage(&Msg);

		DispatchMessage(&Msg);
	}

	QUIT_CODE QuitCode = (QUIT_CODE)Msg.wParam;

	LogMessage(LOG_INFO, _T("The message loop exited with code: %d."), QuitCode);

	if (KillTimer(Window, TimerId) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to kill the timer."));

		return QUIT_ERROR;
	}

	if (DestroyWindow(Window) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to destroy the main window."));

		return QUIT_ERROR;
	}

	if (QuitCode == QUIT_CLOSED || QuitCode == QUIT_RESTART)
	{
		if (SetWindowPos(AppContainerWindow, NULL, 0, 0, AppContainerSize.cx, AppContainerSize.cy, SWP_NOZORDER | SWP_NOMOVE) == FALSE)
		{
			LogMessage(LOG_ERROR, _T("Failed to reset the app container window."));

			return QUIT_ERROR;
		}
	}

	return QuitCode;
}

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
	static BOOL Created;

	static HWND AppContainerWindow;

	switch (Message)
	{
		case WM_CREATE:
		{
			LogMessage(LOG_INFO, _T("The window has been created."));

			Created = TRUE;

			HWND TaskbarWindow = FindTaskbarWindow();

			if (TaskbarWindow == NULL)
			{
				LogMessage(LOG_ERROR, _T("Cannot find the taskbar window."));

				PostQuitMessage(QUIT_ERROR);

				return 0;
			}

			AppContainerWindow = FindAppContainerWindow(TaskbarWindow);

			if (AppContainerWindow == NULL)
			{
				LogMessage(LOG_ERROR, _T("Cannot find the app container window."));

				PostQuitMessage(QUIT_ERROR);

				return 0;
			}

			InitCounters();

			return 0;
		}
		case WM_DESTROY:
		{
			LogMessage(LOG_INFO, _T("The window has been destroyed."));

			Created = FALSE;

			return 0;
		}
		case WM_NCDESTROY:
		{
			if (Created)
			{
				LogMessage(LOG_WARNING, _T("The window has been unexpectedly terminated."));

				PostQuitMessage(QUIT_ERROR);
			}

			return 0;
		}
		case WM_CLOSE:
		{
			LogMessage(LOG_INFO, _T("Window has been closed."));

			PostQuitMessage(QUIT_CLOSED);

			return 0;
		}
		case WM_PAINT: 
		{
			SETTINGS Settings;

			GetSettings(&Settings);

			PAINTSTRUCT PaintStruct;

			HDC DeviceContext = BeginPaint(Window, &PaintStruct);

			SetTextColor(DeviceContext, Settings.TextColor);

			SetBkMode(DeviceContext, TRANSPARENT);

			SIZE WindowSize;

			GetRectSize(&PaintStruct.rcPaint, &WindowSize);
			
			HFONT Font = CreateFont(WindowSize.cy / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FONT_NAME);

			HGDIOBJ OldFont = SelectObject(DeviceContext, Font);

			RECT Rect;

			TCHAR Buffer[16];

			int TotalWidth = WindowSize.cy * SIZE_RATIO / SIZE_DENOM;

			int CurrentWidth = 0;

			if (Settings.ShowCpuAndMemoryUsage)
			{
				SetRect(&Rect, TotalWidth * CurrentWidth / SIZE_DENOM + PADDING, 0, 0, WindowSize.cy - 1);

				_stprintf(Buffer, _T("CPU: %2d %%"), GetCpuUsage());

				DrawText(DeviceContext, Buffer, (int)_tcslen(Buffer), &Rect, DT_TOP | DT_SINGLELINE | DT_NOCLIP);

				_stprintf(Buffer, _T("MEM: %2d %%"), GetMemoryUsage());

				DrawText(DeviceContext, Buffer, (int)_tcslen(Buffer), &Rect, DT_BOTTOM | DT_SINGLELINE | DT_NOCLIP);

				CurrentWidth += SIZE_CPU_MEM_COLUMN;
			}

			if (Settings.ShowDiskIoSpeeds)
			{
				SetRect(&Rect, TotalWidth * CurrentWidth / SIZE_DENOM + PADDING, 0, 0, WindowSize.cy - 1);

				_stprintf(Buffer, _T("DR: "));

				FormatDataQuantity(Buffer + _tcslen(Buffer), GetDiskReadSpeed());

				_stprintf(Buffer + _tcslen(Buffer), _T("/s"));

				DrawText(DeviceContext, Buffer, (int)_tcslen(Buffer), &Rect, DT_TOP | DT_SINGLELINE | DT_NOCLIP);

				_stprintf(Buffer, _T("DW: "));

				FormatDataQuantity(Buffer + _tcslen(Buffer), GetDiskWriteSpeed());

				_stprintf(Buffer + _tcslen(Buffer), _T("/s"));

				DrawText(DeviceContext, Buffer, (int)_tcslen(Buffer), &Rect, DT_BOTTOM | DT_SINGLELINE | DT_NOCLIP);

				CurrentWidth += SIZE_DISK_IO_COLUMN;
			}

			if (Settings.ShowNetworkTrafficSpeeds)
			{
				SetRect(&Rect, TotalWidth * CurrentWidth / SIZE_DENOM + PADDING, 0, 0, WindowSize.cy - 1);

				_stprintf(Buffer, _T("NU: "));

				FormatDataQuantity(Buffer + _tcslen(Buffer), GetUploadSpeed());

				_stprintf(Buffer + _tcslen(Buffer), _T("/s"));

				DrawText(DeviceContext, Buffer, (int)_tcslen(Buffer), &Rect, DT_TOP | DT_SINGLELINE | DT_NOCLIP);

				_stprintf(Buffer, _T("ND: "));

				FormatDataQuantity(Buffer + _tcslen(Buffer), GetDownloadSpeed());

				_stprintf(Buffer + _tcslen(Buffer), _T("/s"));

				DrawText(DeviceContext, Buffer, (int)_tcslen(Buffer), &Rect, DT_BOTTOM | DT_SINGLELINE | DT_NOCLIP);

				CurrentWidth += SIZE_NETWORK_COLUMN;
			}
			
			SelectObject(DeviceContext, OldFont);
			
			DeleteObject(Font);
			
			EndPaint(Window, &PaintStruct);
			
			return 0;
		}
		case WM_ERASEBKGND:
		{
			HDC DeviceContext = (HDC)WParam;

			RECT ClientRect = { 0 };

			GetClientRect(Window, &ClientRect);

			SETTINGS Settings;

			GetSettings(&Settings);

			if (Settings.Transparent)
			{
				HBRUSH Brush = (HBRUSH)GetStockObject(BLACK_BRUSH);

				if (FillRect(DeviceContext, &ClientRect, Brush) == FALSE)
				{
					LogMessage(LOG_WARNING, _T("Failed to clear background."));
				}
			}
			else
			{
				BufferedPaintInit();

				HDC BufferedDeviceContext;

				HPAINTBUFFER PaintBuffer = BeginBufferedPaint(DeviceContext, &ClientRect, BPBF_DIB, NULL, &BufferedDeviceContext);

				if (PaintBuffer) 
				{
					HBRUSH Brush = CreateSolidBrush(Settings.BackgroundColor);

					if (FillRect(BufferedDeviceContext, &ClientRect, Brush) == FALSE)
					{
						LogMessage(LOG_WARNING, _T("Failed to clear the buffered device context."));
					}

					DeleteObject(Brush);

					if (BufferedPaintSetAlpha(PaintBuffer, NULL, 255) != S_OK)
					{
						LogMessage(LOG_WARNING, _T("Failed to set the alpha."));
					}

					EndBufferedPaint(PaintBuffer, TRUE);
				}
				else
				{
					LogMessage(LOG_WARNING, _T("Failed to begin buffered painting."));
				}

				BufferedPaintUnInit();
			}

			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			LogMessage(LOG_INFO, _T("Left click received."));

			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			LogMessage(LOG_INFO, _T("Right click received."));

			if (ShowSettingsDialog(Window))
			{
				LogMessage(LOG_INFO, _T("Settings changed."));

				PostQuitMessage(QUIT_RESTART);
			}

			return 0;
		}
		case WM_LBUTTONUP:
		{
			return 0;
		}
		case WM_RBUTTONUP:
		{
			return 0;
		}
		case WM_TIMER:
		{
			if (IsWindow(AppContainerWindow) == FALSE)
			{
				LogMessage(LOG_ERROR, _T("The app container window disappeared."));

				PostQuitMessage(QUIT_ERROR);

				return 0;
			}

			RECT WindowRect = { 0 };

			GetWindowRect(Window, &WindowRect);

			RECT AppContainerRect = { 0 };

			GetWindowRect(AppContainerWindow, &AppContainerRect);

			RECT IntersectionRect;

			if (IntersectRect(&IntersectionRect, &WindowRect, &AppContainerRect))
			{
				LogMessage(LOG_INFO, _T("Overlap detected."));

				PostQuitMessage(QUIT_CHANGED);

				return 0;
			}

			UpdateCounters();

			RedrawWindow(Window, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

			return 0;
		}
	}

	return DefWindowProc(Window, Message, WParam, LParam);
}

BOOL IsOverlappingTaskbar(HWND Window, BOOL* Overlap)
{
	RECT WindowRect = { 0 };

	GetWindowRect(Window, &WindowRect);

	HWND TaskbarWindow = FindTaskbarWindow();

	if (TaskbarWindow == NULL) 
	{
		LogMessage(LOG_ERROR, _T("The taskbar window disappeared."));

		return FALSE;
	}

	HWND AppContainerWindow = FindAppContainerWindow(TaskbarWindow);

	if (AppContainerWindow == NULL) 
	{
		LogMessage(LOG_ERROR, _T("The app container window disappeared."));

		return FALSE;
	}

	RECT AppContainerRect = { 0 };

	GetWindowRect(AppContainerWindow, &AppContainerRect);

	RECT IntersectionRect;

	*Overlap = IntersectRect(&IntersectionRect, &WindowRect, &AppContainerRect);

	return TRUE;
}

BOOL IsOverlappingWindow(HWND Window, RECT Rect)
{
	HWND ChildWindow = GetWindow(Window, GW_CHILD);

	while (ChildWindow != NULL) 
	{
		RECT ChildRect = { 0 };

		GetWindowRect(ChildWindow, &ChildRect);

		RECT IntersectionRect;

		if (IntersectRect(&IntersectionRect, &Rect, &ChildRect)) 
		{
			return TRUE;
		}

		if (IsOverlappingWindow(ChildWindow, Rect)) 
		{
			return TRUE;
		}

		ChildWindow = GetWindow(ChildWindow, GW_HWNDNEXT);
	}

	return FALSE;
}

HWND FindTaskbarWindow()
{
	return FindWindow(TASKBAR_WINDOW_TEXT, NULL);
}

HWND FindAppContainerWindow(HWND Window)
{
	HWND ChildWindow = GetWindow(Window, GW_CHILD);

	while (ChildWindow != NULL) 
	{
		if (IsRunningAppsWindow(ChildWindow)) 
		{
			return Window;
		}

		HWND FoundWindow = FindAppContainerWindow(ChildWindow);

		if (FoundWindow != NULL) 
		{
			return FoundWindow;
		}

		ChildWindow = GetWindow(ChildWindow, GW_HWNDNEXT);
	}

	return NULL;
}

BOOL IsRunningAppsWindow(HWND Window)
{
	TCHAR WindowTitle[64];

	GetWindowText(Window, WindowTitle, ARRAYSIZE(WindowTitle));

	return _tcsstr(WindowTitle, RUNNING_APPS_TEXT) != NULL;
}

void GetRectSize(const RECT* Rect, SIZE* Size)
{
	Size->cx = Rect->right - Rect->left;

	Size->cy = Rect->bottom - Rect->top;
}

void FormatDataQuantity(TCHAR* Buffer, double Value)
{
	const TCHAR* Units[] = { _T("KB"), _T("MB"), _T("GB"), _T("TB") };

	for (int i = 0; i < ARRAYSIZE(Units); i++)
	{
		Value /= 1024;

		if (Value < 10)
		{
			_stprintf(Buffer, _T("%.2f %s"), Value, Units[i]);

			return;
		}

		if (Value < 100)
		{
			_stprintf(Buffer, _T("%.1f %s"), Value, Units[i]);

			return;
		}

		if (Value < 1000)
		{
			_stprintf(Buffer, _T("%.0f %s"), Value, Units[i]);

			return;
		}
	}

	_stprintf(Buffer, _T("INF"));

	LogMessage(LOG_WARNING, _T("Quantity out of range."));
}
