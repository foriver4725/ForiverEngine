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
		/// <para>ウィンドウプロシージャ内で、WM_INPUT メッセージを受け取ったとき、</para>
		/// <para>マウスの相対移動量を算出して呼び出すこと</para>
		/// </summary>
		static void OnMouseDelta(const Lattice2& delta);

		/// <summary>
		/// <para>仮想キーコード (WPARAM) を Key 列挙型に変換して返す</para>
		/// </summary>
		static Key ConvertVKToKey(WPARAM wparam, LPARAM lparam);

		/// <summary>
		/// <para>外部公開用に、便利なアクセサを提供する</para>
		/// </summary>
		static KeyInfo GetKeyInfo(Key key);

		/// <summary>
		/// <para>マウスの移動量を返す</para>
		/// </summary>
		static Vector2 GetMouseDelta();

		/// <summary>
		/// <para>複数のキーを1つの1D入力として扱う</para>
		/// <para>正規化して返す</para>
		/// </summary>
		static float GetAsAxis1D(Key positiveKey, Key negativeKey);

		/// <summary>
		/// <para>複数のキーを1つの2D入力として扱う</para>
		/// <para>正規化して返す</para>
		/// </summary>
		static Vector2 GetAsAxis2D(Key upKey, Key downKey, Key leftKey, Key rightKey);
	};
}
