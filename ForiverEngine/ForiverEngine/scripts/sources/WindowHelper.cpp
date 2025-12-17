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

	LRESULT WindowHelper::OnWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
			// キー押下
		case WM_KEYDOWN:
		{
			const Key key = InputHelper::ConvertVKToKey(wparam, lparam);
			if (key != Key::Unknown)
				InputHelper::OnPressed(key);

			return 0;
		}

		// キー解放
		case WM_KEYUP:
		{
			const Key key = InputHelper::ConvertVKToKey(wparam, lparam);
			if (key != Key::Unknown)
				InputHelper::OnReleased(key);

			return 0;
		}

		// ウィンドウが破棄されたら呼ばれる
		case WM_DESTROY:
		{
			// OSに対して「もうこのアプリは終わる」と伝える
			PostQuitMessage(0);
			return 0;
		}
		}

		// デフォルトの処理を行う
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	int WindowHelper::HandleAllMessages(MSG& msg)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return -1;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return 0;
	}

	void WindowHelper::PopupErrorDialog(const wchar_t* message)
	{
		MessageBox(nullptr, message, L"error", MB_OK | MB_ICONERROR);
	}
}
