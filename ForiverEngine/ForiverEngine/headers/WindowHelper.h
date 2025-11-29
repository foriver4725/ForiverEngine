#pragma once

struct HINSTANCE__; typedef HINSTANCE__* HINSTANCE;
struct HWND__; typedef HWND__* HWND;

typedef unsigned int UINT;
typedef __int64 LONG_PTR;
typedef unsigned __int64 UINT_PTR;
typedef LONG_PTR LRESULT;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LRESULT(__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

namespace ForiverEngine
{
	class WindowHelper final
	{
		// 既存マクロと重複しない命名にしている
#define WindowMain(hInstance) WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)

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

	public:
		WindowHelper() = delete;
		~WindowHelper() = delete;

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
	};
}
