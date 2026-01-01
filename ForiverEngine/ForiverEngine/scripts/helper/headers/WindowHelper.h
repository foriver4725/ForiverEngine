#pragma once

#include <scripts/common/Include.h>
#include "./InputHelper.h"

// WinMain() のマクロ
// 既存マクロと重複しない命名にしている
#define WindowMain(hInstance) \
WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) \

	// デフォルトのウィンドウプロシージャを定義するマクロ
#define DEFINE_DEFAULT_WINDOW_PROCEDURE(FunctionName) \
static LRESULT CALLBACK FunctionName(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) \
{ \
    return ForiverEngine::WindowHelper::OnWindowProcedure(hwnd, msg, wparam, lparam); \
} \

// エラーのメッセージボックスを出すマクロ
#define ShowError(Message) \
ForiverEngine::WindowHelper::PopupErrorDialog(Message); \

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
	/* キー入力を初期化 */ \
    ForiverEngine::InputHelper::InitKeyTable(); \
\
	/* 時間計測を初期化 */ \
	ForiverEngine::WindowHelper::InitTime(); \
\
	ForiverEngine::WindowHelper::CreateConsoleInGUIApplication(); \

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
	HWND HwndName = ForiverEngine::WindowHelper::CreateTheWindow((WindowClassName), (WindowTitle), (WindowWidth), (WindowHeight)); \
\
    /* キー入力を初期化 */ \
	ForiverEngine::InputHelper::InitKeyTable(); \
\
	/* 時間計測を初期化 */ \
	ForiverEngine::WindowHelper::InitTime(); \

#endif

#ifdef ENABLE_CUI_CONSOLE

	// 初期化のマクロ 終了
#define END_INITIALIZE() \
	ForiverEngine::WindowHelper::CloseConsoleInGUIApplication(); \
    return 0; \
} \

#else

	// 初期化のマクロ 終了
#define END_INITIALIZE() \
    return 0; \
} \

#endif

#ifdef ENABLE_CUI_CONSOLE

	// 手動終了のマクロ
#define QUIT() \
{ \
	ForiverEngine::WindowHelper::CloseConsoleInGUIApplication(); \
	return 0; \
} \

#else

	// 手動終了のマクロ
#define QUIT() \
{ \
	return 0; \
} \

#endif

	// フレーム処理のマクロ 開始
#define BEGIN_FRAME(HwndName) \
MSG msg = {}; \
while (true) \
{ \
\
	/* フレーム開始時の時間を記録 */ \
	ForiverEngine::WindowHelper::RecordTimeAtBeginFrame(); \
\
    /* キー入力を更新 */ \
    ForiverEngine::InputHelper::OnEveryFrame(); \
\
    /* 全てのメッセージを処理する */ \
    /* WM_QUIT メッセージが来たらループを抜ける */ \
	if (ForiverEngine::WindowHelper::HandleAllMessages(msg) < 0) \
		return 0; \
\
    /* カーソルが無効なら、ウィンドウ中央に固定する */ \
	if (!ForiverEngine::WindowHelper::IsCursorEnabled()) \
		ForiverEngine::WindowHelper::FixCursorAtCenter(HwndName); \

// フレーム処理のマクロ 終了
#define END_FRAME() \
\
	/* フレーム終了時の時間を記録し、必要ならばスリープする */ \
	ForiverEngine::WindowHelper::CollectTimeAtEndFrameAndSleepIfNeeded(); \
	ForiverEngine::WindowHelper::ResetTimeAtBeginFrame(); \
}

namespace ForiverEngine
{
	class WindowHelper final
	{
	public:
		DELETE_DEFAULT_METHODS(WindowHelper);

	private:
		inline static bool isCursorEnabled = true;
	public:
		static bool IsCursorEnabled() { return isCursorEnabled; }

		/// <summary>
		/// ターゲットFPS
		/// </summary>
		static int GetTargetFps() { return targetFps; }

		/// <summary>
		/// 前フレームからの経過時間 [ms]
		/// </summary>
		template<std::floating_point TReturnValue>
		static TReturnValue GetDeltaMilliseconds() { return static_cast<TReturnValue>(deltaTime); }
		/// <summary>
		/// 前フレームからの経過時間 [s]
		/// </summary>
		template<std::floating_point TReturnValue>
		static TReturnValue GetDeltaSeconds() { return static_cast<TReturnValue>(deltaTime * 1.0e-3); }

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
		/// <para>ウィンドウプロシージャの内部実装</para>
		/// </summary>
		static LRESULT OnWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		/// <summary>
		/// <para>全てのメッセージを処理する</para>
		/// <para>戻り値が -1 の場合、WM_QUIT メッセージが来たことを示す</para>
		/// <para>それ以外の場合、0 を返す</para>
		/// </summary>
		static int HandleAllMessages(MSG& msg);

		/// <summary>
		/// <para>カーソルの表示・非表示を切り替える</para>
		/// <para>重複実行でもOK</para>
		/// </summary>
		static void SetCursorEnabled(bool enabled);

		/// <summary>
		/// <para>ウィンドウ中央のスクリーン座標を返す</para>
		/// </summary>
		static Lattice2 GetScreenCenter(HWND hwnd);

		/// <summary>
		/// <para>カーソルをウィンドウ中央に固定する</para>
		/// </summary>
		static void FixCursorAtCenter(HWND hwnd);

		/// <summary>
		/// ターゲットFPSを設定する
		/// </summary>
		static void SetTargetFps(int fps);

		/// <summary>
		/// 時間計測の初期化
		/// </summary>
		static void InitTime();

		/// <summary>
		/// 現在の時間を[ms]で返す
		/// </summary>
		static double GetTime();

		/// <summary>
		/// フレーム開始時の時間を記録する
		/// </summary>
		static void RecordTimeAtBeginFrame();

		/// <summary>
		/// フレーム開始時の時間記録をリセットする
		/// </summary>
		static void ResetTimeAtBeginFrame();

		/// <summary>
		/// フレーム終了時の時間を記録し、必要ならばスリープする
		/// </summary>
		static void CollectTimeAtEndFrameAndSleepIfNeeded();

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

	private:
		inline static int targetFps = -1;
		inline static double targetFrameTime = -1; // [ms]
		inline static LARGE_INTEGER timeFrequency{}; // 時間計測で使う値 (1回だけ初期化)
		inline static double timeAtBeginFrame = -1; // フレーム開始時の時間をメモっておく用
		inline static double deltaTime = -1; // 前フレームからの経過時間 [ms] を計算し、外部公開する
	};

#ifdef ENABLE_CUI_CONSOLE
	template <class... Types>
	inline void Print(const std::format_string<Types...> format, Types&&... args) {
		std::print(std::cout, format, std::forward<Types>(args)...);
	}
#endif
}
