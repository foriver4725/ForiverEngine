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

		// マウス移動
		case WM_MOUSEMOVE:
		{
			const int x = GET_X_LPARAM(lparam);
			const int y = GET_Y_LPARAM(lparam);
			const Lattice2 position = Lattice2(x, y);

			if (position != GetScreenCenter(hwnd))
				InputHelper::OnMouseMove(position);

			return 0;
		}

		case WM_SETFOCUS:
		{
			// カーソルを無効化する
			SetCursorEnabled(false);

			return 0;
		}

		case WM_KILLFOCUS:
		{
			// カーソルを有効化する
			SetCursorEnabled(true);

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

	void WindowHelper::SetCursorEnabled(bool enabled)
	{
		if (enabled && !isCursorEnabled)
		{
			// カーソルを表示
			ShowCursor(TRUE);
			isCursorEnabled = true;
		}
		else if (!enabled && isCursorEnabled)
		{
			// カーソルを非表示
			ShowCursor(FALSE);
			isCursorEnabled = false;
		}
	}

	Lattice2 WindowHelper::GetScreenCenter(HWND hwnd)
	{
		RECT rect;
		GetClientRect(hwnd, &rect);

		POINT center =
		{
			(rect.right - rect.left) / 2,
			(rect.bottom - rect.top) / 2
		};

		ClientToScreen(hwnd, &center);
		return Lattice2(static_cast<int>(center.x), static_cast<int>(center.y));
	}

	void WindowHelper::FixCursorAtCenter(HWND hwnd)
	{
		const Lattice2 center = GetScreenCenter(hwnd);
		SetCursorPos(center.x, center.y);
	}

	void WindowHelper::SetTargetFps(int fps)
	{
		WindowHelper::targetFps = fps;
		WindowHelper::targetFrameTime = (fps > 0) ? (1000.0 / fps) : -1;
	}

	void WindowHelper::InitTime()
	{
		QueryPerformanceFrequency(&WindowHelper::timeFrequency);
	}

	double WindowHelper::GetTime()
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return 1000.0 * static_cast<double>(now.QuadPart) / static_cast<double>(WindowHelper::timeFrequency.QuadPart);
	}

	void WindowHelper::RecordTimeAtBeginFrame()
	{
		WindowHelper::timeAtBeginFrame = GetTime();
	}

	void WindowHelper::ResetTimeAtBeginFrame()
	{
		WindowHelper::timeAtBeginFrame = -1;
	}

	void WindowHelper::CollectTimeAtEndFrameAndSleepIfNeeded()
	{
		// フレーム開始時の時間が記録されていない
		if (WindowHelper::timeAtBeginFrame < 0)
			return;

		// 時間をコレクトし、フレームタイムを算出
		const double timeAtBeginFrame = WindowHelper::timeAtBeginFrame;
		const double timeAtEndFrame = GetTime();
		const double frameTime = timeAtEndFrame - timeAtBeginFrame;

		// deltaTime として更新する
		// 処理が早く終わった場合、この後の処理で再度更新される
		// 従って、deltaTime の最小値は targetFrameTime であり、
		// フレーム落ちなどでこれより大きくなることはあるが、小さくなることはない
		WindowHelper::deltaTime = frameTime;

		// 処理が早く終わった場合、スリープする
		if (WindowHelper::targetFrameTime > 0)
		{
			const double timeToSleep = WindowHelper::targetFrameTime - frameTime;
			if (timeToSleep > 0.0)
			{
				Sleep(static_cast<DWORD>(timeToSleep));

				// スリープ後の時間を再取得し、deltaTime を正確にする
				const double newTimeAtEndFrame = GetTime();
				WindowHelper::deltaTime = newTimeAtEndFrame - timeAtBeginFrame;
			}
		}
	}

	void WindowHelper::PopupErrorDialog(const wchar_t* message)
	{
		MessageBox(nullptr, message, L"error", MB_OK | MB_ICONERROR);
	}
}
