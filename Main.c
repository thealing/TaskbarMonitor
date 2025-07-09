#include "main.h"

int main()
{
	return Run();
}

int WINAPI WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ LPSTR CommandLine, _In_ int ShowMode)
{
	return Run();
}

int Run()
{
	LogSetFile(LOG_FILE_NAME);

	InitSettings();

	WNDCLASS WindowClass = { 0 };

	WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	WindowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	WindowClass.lpszClassName = MONITOR_WINDOW_CLASS;

	WindowClass.lpfnWndProc = WindowProcedure;

	if (RegisterClass(&WindowClass) == 0)
	{
		LogMessage(LOG_ERROR, _T("Failed to register the monitor window class."));

		return 1;
	}

	while (TRUE)
	{
		QUIT_CODE QuitCode = InjectWindow();

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

QUIT_CODE InjectWindow()
{
	HWND AppContainerWindow = FindAppContainerWindow();

	if (AppContainerWindow == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to find the app container window."));

		return QUIT_ERROR;
	}

	RECT AppContainerRect;

	if (GetWindowRect(AppContainerWindow, &AppContainerRect) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to get the dimensions of the app container."));

		return QUIT_ERROR;
	}

	SIZE AppContainerSize;

	GetRectSize(&AppContainerRect, &AppContainerSize);

	HWND Window = CreateWindow(MONITOR_WINDOW_CLASS, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL);

	if (Window == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to create the monitor window."));

		return QUIT_ERROR;
	}

	if (InsertMonitorWindow(AppContainerWindow, Window) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to insert the monitor window."));

		DestroyWindow(Window);

		return QUIT_ERROR;
	}

	UINT_PTR TimerId = SetTimer(Window, TRUE, UPDATE_INTERVAL, NULL);

	if (TimerId == 0)
	{
		LogMessage(LOG_ERROR, _T("Failed to set a timer."));

		DestroyWindow(Window);

		return QUIT_ERROR;
	}

	LogMessage(LOG_INFO, _T("Entering the message loop."));

	ShowWindow(Window, SW_SHOW);

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
		LogMessage(LOG_ERROR, _T("Failed to destroy the timer."));

		DestroyWindow(Window);

		return QUIT_ERROR;
	}

	if (DestroyWindow(Window) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to destroy the monitor window."));

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

	static ULONGLONG LastUpdateTime;

	static HWND AppContainerWindow;

	switch (Message)
	{
		case WM_CREATE:
		{
			LogMessage(LOG_INFO, _T("The window has been created."));

			Created = TRUE;

			LastUpdateTime = GetTickCount64();

			AppContainerWindow = FindAppContainerWindow();

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
						LogMessage(LOG_WARNING, _T("Failed to clear the paint buffer."));
					}

					DeleteObject(Brush);

					if (BufferedPaintSetAlpha(PaintBuffer, NULL, 255) != S_OK)
					{
						LogMessage(LOG_WARNING, _T("Failed to set the opacity of the paint buffer."));
					}

					EndBufferedPaint(PaintBuffer, TRUE);
				}
				else
				{
					LogMessage(LOG_WARNING, _T("Failed to create the paint buffer."));
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

				InsertMonitorWindow(AppContainerWindow, Window);
			}
			else
			{
				ULONGLONG CurrentTime = GetTickCount64();

				if (CurrentTime > LastUpdateTime + MEASURE_INTERVAL)
				{
					LastUpdateTime = max(CurrentTime - MEASURE_INTERVAL, LastUpdateTime + MEASURE_INTERVAL);

					UpdateCounters();

					RedrawWindow(Window, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
				}
			}
			
			return 0;
		}
	}

	return DefWindowProc(Window, Message, WParam, LParam);
}

BOOL InsertMonitorWindow(HWND AppContainerWindow, HWND MonitorWindow)
{
	RECT AppContainerRect;

	if (GetWindowRect(AppContainerWindow, &AppContainerRect) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to get the dimensions of the app container."));

		return FALSE;
	}

	HWND ToolbarWindow = GetParent(AppContainerWindow);

	if (ToolbarWindow == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to find the toolbar window window."));

		return FALSE;
	}

	RECT ToolbarRect;

	if (GetWindowRect(ToolbarWindow, &ToolbarRect) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to get the dimensions of the toolbar window."));

		return FALSE;
	}

	SIZE AppContainerSize;

	GetRectSize(&AppContainerRect, &AppContainerSize);

	SETTINGS Settings;

	GetSettings(&Settings);

	int TotalWidth = 0;

	TotalWidth += SIZE_CPU_MEM_COLUMN * Settings.ShowCpuAndMemoryUsage;

	TotalWidth += SIZE_DISK_IO_COLUMN * Settings.ShowDiskIoSpeeds;

	TotalWidth += SIZE_NETWORK_COLUMN * Settings.ShowNetworkTrafficSpeeds;

	int MonitorHeight = AppContainerSize.cy - PADDING * 2;

	int MonitorWidth = MonitorHeight * SIZE_RATIO / SIZE_DENOM * TotalWidth / SIZE_DENOM;

	if (MonitorWidth <= 0 || MonitorHeight <= 0)
	{
		LogMessage(LOG_ERROR, _T("The monitor window size is not positive."));

		return FALSE;
	}

	SIZE ReducedSize = { AppContainerSize.cx - MonitorWidth - PADDING * 2, AppContainerSize.cy };

	if (ReducedSize.cx <= 0 || ReducedSize.cy <= 0)
	{
		LogMessage(LOG_ERROR, _T("The reduced app container window size is not positive."));

		return FALSE;
	}

	if (SetWindowPos(AppContainerWindow, NULL, 0, 0, ReducedSize.cx, ReducedSize.cy, SWP_NOZORDER | SWP_NOMOVE) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to adjust the size of the app container window."));

		return FALSE;
	}

	int MonitorX = AppContainerRect.left - ToolbarRect.left + ReducedSize.cx + PADDING;

	int MonitorY = AppContainerRect.top - ToolbarRect.top + PADDING;

	if (SetWindowPos(AppContainerWindow, NULL, 0, 0, ReducedSize.cx, ReducedSize.cy, SWP_NOZORDER | SWP_NOMOVE) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to adjust the size of the app container window."));

		return FALSE;
	}

	if (SetParent(MonitorWindow, ToolbarWindow) == NULL)
	{
		LogMessage(LOG_ERROR, _T("Failed to set the parent of the monitor window."));

		return FALSE;
	}

	if (SetWindowPos(MonitorWindow, NULL, MonitorX, MonitorY, MonitorWidth, MonitorHeight, SWP_NOZORDER) == FALSE)
	{
		LogMessage(LOG_ERROR, _T("Failed to position the monitor window."));

		return FALSE;
	}

	return TRUE;
}

HWND FindAppContainerWindow()
{
	HRESULT Result = S_OK;

	IUIAutomation* Automation = NULL;

	IUIAutomationElement* RootElement = NULL;

	IUIAutomationElement* TrayElement = NULL;

	IUIAutomationElement* AppListElement = NULL;

	VARIANT TrayClassName = { 0 };

	VARIANT AppListClassName = { 0 };

	IUIAutomationCondition* TrayCondition = NULL;

	IUIAutomationCondition* AppListCondition = NULL;

	HWND AppListWindow = NULL;

	HWND AppContainerWindow = NULL;

	Result = CoInitialize(NULL);

	if (FAILED(Result))
	{
		goto Cleanup;
	}

	Result = CoCreateInstance(&CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, &IID_IUIAutomation, (void**)&Automation);

	if (FAILED(Result) || Automation == NULL)
	{
		goto Cleanup;
	}

	Result = Automation->lpVtbl->GetRootElement(Automation, &RootElement);

	if (FAILED(Result) || RootElement == NULL)
	{
		goto Cleanup;
	}

	TrayClassName.vt = VT_BSTR;

	TrayClassName.bstrVal = SysAllocString(L"Shell_TrayWnd");

	Result = Automation->lpVtbl->CreatePropertyCondition(Automation, UIA_ClassNamePropertyId, TrayClassName, &TrayCondition);

	if (FAILED(Result) || TrayCondition == NULL)
	{
		goto Cleanup;
	}

	Result = RootElement->lpVtbl->FindFirst(RootElement, TreeScope_Children, TrayCondition, &TrayElement);

	if (FAILED(Result) || TrayElement == NULL)
	{
		goto Cleanup;
	}

	AppListClassName.vt = VT_BSTR;

	AppListClassName.bstrVal = SysAllocString(L"MSTaskListWClass");

	Result = Automation->lpVtbl->CreatePropertyCondition(Automation, UIA_ClassNamePropertyId, AppListClassName, &AppListCondition);

	if (FAILED(Result) || AppListCondition == NULL)
	{
		goto Cleanup;
	}

	Result = TrayElement->lpVtbl->FindFirst(TrayElement, TreeScope_Descendants, AppListCondition, &AppListElement);

	if (FAILED(Result) || AppListElement == NULL)
	{
		goto Cleanup;
	}

	Result = AppListElement->lpVtbl->get_CurrentNativeWindowHandle(AppListElement, &AppListWindow);

	if (FAILED(Result) || AppListWindow == NULL)
	{
		goto Cleanup;
	}

	AppContainerWindow = GetParent(AppListWindow);

Cleanup:

	SysFreeString(TrayClassName.bstrVal);

	SysFreeString(AppListClassName.bstrVal);

	SafeRelease(Automation);

	SafeRelease(RootElement);

	SafeRelease(TrayElement);

	SafeRelease(AppListElement);

	SafeRelease(TrayCondition);

	SafeRelease(AppListCondition);

	return AppContainerWindow;
}

void SafeRelease(IUnknown* Unknown)
{
	if (Unknown != NULL)
	{
		Unknown->lpVtbl->Release(Unknown);
	}
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
