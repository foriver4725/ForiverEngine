#include "../headers/WindowHelper.h"

namespace ForiverEngine
{
	bool WindowHelper::InitializeWindowFromHInstance(HINSTANCE hInstance, WNDPROC windowProcedure, const wchar_t* className)
	{
		WNDCLASSW w = {};
		w.hInstance = hInstance;
		w.lpfnWndProc = windowProcedure;
		w.lpszClassName = className;

		// ウィンドウクラスの登録
		return RegisterClassW(&w) != 0;
	}

	HWND WindowHelper::CreateTheWindow(const wchar_t* className, const wchar_t* title, int width, int height)
	{
		return CreateWindowW(className, title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, nullptr, nullptr);
	}

	void WindowHelper::PopupErrorDialog(const wchar_t* message)
	{
		MessageBox(nullptr, message, L"error", MB_OK | MB_ICONERROR);
	}
}
