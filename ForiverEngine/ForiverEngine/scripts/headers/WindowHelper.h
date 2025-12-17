#pragma once

#include <scripts/common/Include.h>
#include "./InputHelper.h"

struct HINSTANCE__; typedef HINSTANCE__* HINSTANCE;
struct HWND__; typedef HWND__* HWND;

typedef unsigned int UINT;
typedef __int64 LONG_PTR;
typedef unsigned __int64 UINT_PTR;
typedef LONG_PTR LRESULT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LRESULT(__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

// WinMain() のマクロ
// 既存マクロと重複しない命名にしている
#define WindowMain(hInstance) \
WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)

	// デフォルトのウィンドウプロシージャを定義するマクロ
#define DEFINE_DEFAULT_WINDOW_PROCEDURE(FunctionName) \
static LRESULT CALLBACK FunctionName(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) \
{ \
    /* ウィンドウが破棄されたら呼ばれる */ \
	if (msg == WM_DESTROY) \
	{ \
        /* OSに対して「もうこのアプリは終わる」と伝える */ \
		PostQuitMessage(0); \
		return 0; \
	} \
\
    /* デフォルトの処理を行う */ \
	return DefWindowProc(hwnd, msg, wparam, lparam); \
}

// エラーのメッセージボックスを出すマクロ
#define ShowError(Message) \
ForiverEngine::WindowHelper::PopupErrorDialog(Message);

#ifdef ENABLE_CUI_CONSOLE

	// 初期化のマクロ 開始
#define BEGIN_INITIALIZE(WindowClassName, WindowTitle, HwndName, WindowWidth, WindowHeight) \
DEFINE_DEFAULT_WINDOW_PROCEDURE(WindowProcedure) \
\
int WindowMain(hInstance) \
{ \
	if (!ForiverEngine::WindowHelper::InitializeWindowFromHInstance(hInstance, WindowProcedure, (WindowClassName))) \
    { \
		ShowError(L"ウィンドウの初期化に失敗しました"); \
		return -1; \
	} \
\
	HWND HwndName = ForiverEngine::WindowHelper::CreateTheWindow((WindowClassName), (WindowTitle), (WindowWidth), (WindowHeight)); \
\
	ForiverEngine::WindowHelper::CreateConsoleInGUIApplication();

#else

	// 初期化のマクロ 開始
#define BEGIN_INITIALIZE(WindowClassName, WindowTitle, HwndName, WindowWidth, WindowHeight) \
DEFINE_DEFAULT_WINDOW_PROCEDURE(WindowProcedure) \
\
int WindowMain(hInstance) \
{ \
	if (!ForiverEngine::WindowHelper::InitializeWindowFromHInstance(hInstance, WindowProcedure, (WindowClassName))) \
    { \
		ShowError(L"ウィンドウの初期化に失敗しました"); \
		return -1; \
	} \
\
	HWND HwndName = ForiverEngine::WindowHelper::CreateTheWindow((WindowClassName), (WindowTitle), (WindowWidth), (WindowHeight));

#endif

#ifdef ENABLE_CUI_CONSOLE

	// 初期化のマクロ 終了
#define END_INITIALIZE(ReturnIntValue) \
	ForiverEngine::WindowHelper::CloseConsoleInGUIApplication(); \
    return ReturnIntValue; \
}

#else

	// 初期化のマクロ 終了
#define END_INITIALIZE(ReturnIntValue) \
    return ReturnIntValue; \
}

#endif

	// メッセージループのマクロ 開始
#define BEGIN_MESSAGE_LOOP \
MSG msg = {}; \
while (GetMessage(&msg, nullptr, 0, 0)) { \
	TranslateMessage(&msg); \
	DispatchMessage(&msg);

	// メッセージループのマクロ 終了
#define END_MESSAGE_LOOP \
}

namespace ForiverEngine
{
	class WindowHelper final
	{
	public:
		DELETE_DEFAULT_METHODS(WindowHelper);

		/// <summary>
		/// ウィンドウを初期化する
		/// </summary>
		/// <param name="hInstance">WinMain() の hInstance を渡す</param>
		/// <param name="windowProcedure">ウィンドウプロシージャの関数ポインタ</param>
		/// <returns>成功したら true, 失敗したら false</returns>
		static bool InitializeWindowFromHInstance(HINSTANCE hInstance, WNDPROC windowProcedure, const wchar_t* className);

		/// <summary>
		/// <para>ウィンドウを作成し、ハンドルを返す</para>
		/// 既存マクロと重複しない命名にしている
		/// </summary>
		static HWND CreateTheWindow(const wchar_t* className, const wchar_t* title, int width, int height);

		/// <summary>
		/// エラーのメッセージボックスを出す
		/// </summary>
		static void PopupErrorDialog(const wchar_t* message);

#ifdef ENABLE_CUI_CONSOLE
		/// <summary>
		/// GUIアプリケーションでコンソールウィンドウを作成する
		/// </summary>
		static void CreateConsoleInGUIApplication()
		{
			FILE* fp;
			AllocConsole();
			freopen_s(&fp, "CONOUT$", "w", stdout); // 標準出力の割り当て (stdout)
		}

		/// <summary>
		/// GUIアプリケーションで作成したコンソールウィンドウを閉じる
		/// </summary>
		static void CloseConsoleInGUIApplication()
		{
			FreeConsole();
		}
#endif
	};

#ifdef ENABLE_CUI_CONSOLE
	template <class... Types>
	inline void Print(const std::format_string<Types...> format, Types&&... args) {
		std::print(std::cout, format, std::forward<Types>(args)...);
	}
#endif
}
