#pragma once
#include <Windows.h>

namespace Win32Utils {
	bool InitWindow(HINSTANCE hInstance, HWND& hwnd, int showWnd, int width, int height,
		bool fullscreen, LPCTSTR wndName, LPCTSTR wndTitle, WNDPROC wndProc)
	{
		if (fullscreen)
		{
			HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO monInf = { sizeof(monInf) };
			GetMonitorInfo(hmon, &monInf);

			width = monInf.rcMonitor.right - monInf.rcMonitor.left;
			height = monInf.rcMonitor.bottom - monInf.rcMonitor.top;
		}

		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = wndProc;
		wc.cbClsExtra = NULL;
		wc.cbWndExtra = NULL;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = wndName;
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		if (!RegisterClassEx(&wc))
		{
			MessageBox(NULL, L"Error registering class",
				L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		hwnd = CreateWindowEx(NULL, wndName, wndTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			width, height, NULL, NULL, hInstance, NULL);
		if (!hwnd)
		{
			MessageBox(NULL, L"Error creating window",
				L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		if (fullscreen)
		{
			SetWindowLong(hwnd, GWL_STYLE, 0);
		}

		ShowWindow(hwnd, showWnd);
		UpdateWindow(hwnd);

		return true;
	}
}