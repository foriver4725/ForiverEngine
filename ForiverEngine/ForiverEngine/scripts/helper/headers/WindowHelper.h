#pragma once

#include <scripts/common/Include.h>

#define Main(HInstanceName) \
WINAPI WinMain(_In_ HINSTANCE HInstanceName, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) \

// 書くのが簡単なように、一律このマクロを使うものとする
#define ShowError(Message) \
ForiverEngine::WindowHelper::PopupErrorDialog(Message); \

namespace ForiverEngine
{
	class WindowHelper final
	{
	public:
		DELETE_DEFAULT_METHODS(WindowHelper);

		// ウィンドウクラス名・ウィンドウタイトル名
		inline static const std::wstring WindowName = L"ForiverEngine";

		static bool GetIsCursorEnabled() { return isCursorEnabled; }

		static int GetTargetFps() { return targetFps; }

		template<std::floating_point TReturnValue = double>
		static TReturnValue GetDeltaMilliseconds() { return static_cast<TReturnValue>(deltaTime); }

		template<std::floating_point TReturnValue = float>
		static TReturnValue GetDeltaSeconds() { return static_cast<TReturnValue>(deltaTime * 1.0e-3); }

		/// <summary>
		/// <para>カーソルの表示・非表示を切り替える</para>
		/// <para>重複実行でもOK</para>
		/// </summary>
		static void SetCursorEnabled(bool enabled);

		/// <summary>
		/// ターゲットFPSを設定する
		/// </summary>
		static void SetTargetFps(int fps);

		/// <summary>
		/// エラーのメッセージボックスを出す
		/// </summary>
		static void PopupErrorDialog(const std::wstring& message);

		/// <summary>
		/// WinMain() 後、ただちに呼び出すこと
		/// </summary>
		static HWND OnInit(HINSTANCE hInstance, const Lattice2& windowSize);

		/// <summary>
		/// <para>フレーム開始時に呼び出すこと</para>
		/// <para>フレームループを終了するなら false を、それ以外は true を返す</para>
		/// </summary>
		static bool OnBeginFrame(HWND hwnd);

		/// <summary>
		/// フレーム終了時に呼び出すこと
		/// </summary>
		static void OnEndFrame();

	private:
		inline static bool isCursorEnabled = true;

		inline static int targetFps = -1;
		inline static double targetFrameTime = -1; // [ms]
		inline static LARGE_INTEGER timeFrequency{}; // 時間計測で使う値 (1回だけ初期化)
		inline static double timeAtBeginFrame = -1; // フレーム開始時の時間をメモっておく用
		inline static double deltaTime = -1; // 前フレームからの経過時間 [ms] を計算し、外部公開する

		/// <summary>
		/// ウィンドウを初期化する
		/// </summary>
		/// <param name="hInstance">WinMain() の hInstance を渡す</param>
		/// <param name="windowProcedure">ウィンドウプロシージャの関数ポインタ</param>
		/// <returns>成功したら true, 失敗したら false</returns>
		static bool InitializeWindowFromHInstance(HINSTANCE hInstance, WNDPROC windowProcedure, const std::wstring& className);

		/// <summary>
		/// <para>ウィンドウを作成し、ハンドルを返す</para>
		/// 既存マクロと重複しない命名にしている
		/// </summary>
		static HWND CreateTheWindow(const std::wstring& className, const std::wstring& title, const Lattice2& size);

		/// <summary>
		/// <para>ウィンドウプロシージャの内部実装</para>
		/// </summary>
		static LRESULT OnWindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		/// <summary>
		/// <para>全てのメッセージを処理する</para>
		/// <para>WM_QUIT メッセージが来たなら false を、それ以外は true を返す</para>
		/// </summary>
		static bool HandleAllMessages();

		/// <summary>
		/// <para>ウィンドウ中央のスクリーン座標を返す</para>
		/// </summary>
		static Lattice2 GetScreenCenter(HWND hwnd);

		/// <summary>
		/// <para>カーソルをウィンドウ中央に固定する</para>
		/// </summary>
		static void FixCursorAtCenter(HWND hwnd);

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
	};
}
