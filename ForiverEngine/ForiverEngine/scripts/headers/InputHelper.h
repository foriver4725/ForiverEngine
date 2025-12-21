#pragma once

#include <scripts/common/Include.h>

namespace ForiverEngine
{
	using KeyEnumInt = std::uint8_t;

	enum class Key : KeyEnumInt
	{
		Unknown = 0,

		// マウス
		LMouse, RMouse, MMouse,

		// 数字
		N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,

		// 数字 (テンキー)
		NP0, NP1, NP2, NP3, NP4, NP5, NP6, NP7, NP8, NP9,

		// 英字
		A, B, C, D, E, F, G, H, I, J, K, L, M,
		N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		// ファンクション
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,

		// 矢印
		Up, Down, Left, Right,

		// その他 (1つのみ)
		Space,
		Tab,
		Enter,
		Backspace,
		Delete,
		Escape,

		// その他 (複数存在)
		LShift, RShift,
		LCtrl, RCtrl,
		LAlt, RAlt,

		// 要素の個数
		Count,
	};

	struct KeyInfo
	{
		bool pressed = false; // 押されているか
		bool released = false; // 離されているか

		bool pressedNow = false; // 今フレームで押されたか
		bool releasedNow = false; // 今フレームで離されたか
	};

	// キー入力情報を記録するテーブル
	// Key 列挙型の値をインデックスにして使う
	inline std::array<KeyInfo, static_cast<std::size_t>(Key::Count)> KeyTable = {};

	// マウスの移動を記録する変数
	inline Vector2 MousePositionPrev = Vector2::Zero();
	inline Vector2 MousePosition = Vector2::Zero();
	inline Vector2 MouseDelta = Vector2::Zero();

	class InputHelper final
	{
	public:
		DELETE_DEFAULT_METHODS(InputHelper);

		/// <summary>
		/// <para>キー入力テーブルを初期化する</para>
		/// <para>Released フラグのみ true に設定される</para>
		/// <para>複数回呼び出しても問題ない</para>
		/// </summary>
		static void InitKeyTable();

		/// <summary>
		/// <para>メッセージループ内で、毎フレーム呼び出しすこと</para>
		/// </summary>
		static void OnEveryFrame();

		/// <summary>
		/// <para>ウィンドウプロシージャ内で、WM_KEYDOWN メッセージを受け取ったときに呼び出すこと</para>
		/// </summary>
		static void OnPressed(Key key);

		/// <summary>
		/// <para>ウィンドウプロシージャ内で、WM_KEYUP メッセージを受け取ったときに呼び出すこと</para>
		/// </summary>
		static void OnReleased(Key key);

		/// <summary>
		/// <para>ウィンドウプロシージャ内で、WM_MOUSEMOVE メッセージを受け取ったときに呼び出すこと</para>
		/// </summary>
		static void OnMouseMove(LPARAM lparam);

		/// <summary>
		/// <para>仮想キーコード (WPARAM) を Key 列挙型に変換して返す</para>
		/// </summary>
		static Key ConvertVKToKey(WPARAM wparam, LPARAM lparam);

		/// <summary>
		/// <para>外部公開用に、便利なアクセサを提供する</para>
		/// </summary>
		static KeyInfo GetKeyInfo(Key key);

		/// <summary>
		/// <para>マウスの現在位置を取得する</para>
		/// </summary>
		static Vector2 GetMousePosition() { return MousePosition; }

		/// <summary>
		/// <para>マウスがフレーム間で動いた値を取得する</para>
		/// </summary>
		static Vector2 GetMouseDelta() { return MouseDelta; }
	};
}
