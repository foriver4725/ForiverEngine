#include "../headers/WindowHelper.h"
#include "../headers/InputHelper.h"

namespace ForiverEngine
{
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

	void WindowHelper::SetTargetFps(int fps)
	{
		WindowHelper::targetFps = fps;
		WindowHelper::targetFrameTime = (fps > 0) ? (1000.0 / fps) : -1;
	}

	void WindowHelper::PopupErrorDialog(const std::wstring& message)
	{
		MessageBox(nullptr, message.c_str(), L"error", MB_OK | MB_ICONERROR);
	}

	HWND WindowHelper::OnInit(HINSTANCE hInstance, const Lattice2& windowSize)
	{
		if (!InitializeWindowFromHInstance(hInstance, OnWindowProcedure, WindowName))
			ShowError(L"ウィンドウの初期化に失敗しました");

		const HWND hwnd = ForiverEngine::WindowHelper::CreateTheWindow(WindowName, WindowName, windowSize);

		if (!RegisterRawInput(hwnd))
			ShowError(L"Raw Input デバイス (マウス用) の登録に失敗しました");

		ForiverEngine::InputHelper::InitKeyTable();
		ForiverEngine::WindowHelper::InitTime();

		return hwnd;
	}

	bool WindowHelper::OnBeginFrame(HWND hwnd)
	{
		RecordTimeAtBeginFrame();

		InputHelper::OnEveryFrame();

		if (!HandleAllMessages())
			return false;

		if (!GetIsCursorEnabled())
			FixCursorAtCenter(hwnd);

		return true;
	}

	void WindowHelper::OnEndFrame()
	{
		CollectTimeAtEndFrameAndSleepIfNeeded();
		ResetTimeAtBeginFrame();
	}



	bool WindowHelper::InitializeWindowFromHInstance(HINSTANCE hInstance, WNDPROC windowProcedure, const std::wstring& className)
	{
		WNDCLASSW w = {};
		w.hInstance = hInstance;
		w.lpfnWndProc = windowProcedure;
		w.lpszClassName = className.c_str();

		// ウィンドウクラスの登録
		return RegisterClassW(&w) != 0;
	}

	HWND WindowHelper::CreateTheWindow(const std::wstring& className, const std::wstring& title, const Lattice2& size)
	{
		// スクリーンサイズ取得
		const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// スクリーン中央になるべき位置を計算
		const int x = (screenWidth - size.x) / 2;
		const int y = (screenHeight - size.y) / 2;

		return CreateWindowW(className.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			x, y, size.x, size.y, nullptr, nullptr, nullptr, nullptr);
	}

	bool WindowHelper::RegisterRawInput(HWND hwnd)
	{
		const RAWINPUTDEVICE device =
		{
			.usUsagePage = 0x01, // 普通のデスクトップ
			.usUsage = 0x02, // マウス
			.dwFlags = RIDEV_INPUTSINK,
			.hwndTarget = hwnd
		};

		return RegisterRawInputDevices(&device, 1, sizeof(device));
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

		// キー押下 (マウス左ボタン)
		case WM_LBUTTONDOWN:
		{
			InputHelper::OnPressed(Key::LMouse);
			return 0;
		}
		// キー解放 (マウス左ボタン)
		case WM_LBUTTONUP:
		{
			InputHelper::OnReleased(Key::LMouse);
			return 0;
		}
		// キー押下 (マウス右ボタン)
		case WM_RBUTTONDOWN:
		{
			InputHelper::OnPressed(Key::RMouse);
			return 0;
		}
		// キー解放 (マウス右ボタン)
		case WM_RBUTTONUP:
		{
			InputHelper::OnReleased(Key::RMouse);
			return 0;
		}
		// キー押下 (マウス中ボタン)
		case WM_MBUTTONDOWN:
		{
			InputHelper::OnPressed(Key::MMouse);
			return 0;
		}
		// キー解放 (マウス中ボタン)
		case WM_MBUTTONUP:
		{
			InputHelper::OnReleased(Key::MMouse);
			return 0;
		}

		// マウス移動 (Raw 入力)
		case WM_INPUT:
		{
			// 入力のデータを取得

			UINT size = 0;
			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

			std::vector<std::uint8_t> buffer(size);
			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());

			// 入力のデータを取得できた

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				int dx = static_cast<int>(raw->data.mouse.lLastX);
				int dy = static_cast<int>(raw->data.mouse.lLastY);

				InputHelper::OnMouseDelta({ dx, dy });
			}

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

	bool WindowHelper::HandleAllMessages()
	{
		MSG msg = {};

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return false;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return true;
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
}
