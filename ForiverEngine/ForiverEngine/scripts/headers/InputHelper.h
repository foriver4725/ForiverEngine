#pragma once

#include <scripts/common/Include.h>

namespace ForiverEngine
{
	using KeyEnumInt = std::uint8_t;

	enum class Key : KeyEnumInt
	{
		Unknown = 0,

		// 数字
		N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,

		// 英字
		A, B, C, D, E, F, G, H, I, J, K, L, M,
		N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		// ファンクション
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

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

	class InputHelper final
	{
	public:
		DELETE_DEFAULT_METHODS(InputHelper);

		/// <summary>
		/// <para>キー入力テーブルを初期化する</para>
		/// <para>Released フラグのみ true に設定される</para>
		/// <para>複数回呼び出しても問題ない</para>
		/// </summary>
		static void InitKeyTable()
		{
			for (KeyEnumInt i = 0; i < static_cast<KeyEnumInt>(Key::Count); ++i)
				KeyTable[i] = { .released = true };
		}

		static void OnEveryFrame()
		{
			for (KeyEnumInt i = 0; i < static_cast<KeyEnumInt>(Key::Count); ++i)
			{
				KeyInfo& keyInfo = KeyTable[i];
				keyInfo.pressedNow = false;
				keyInfo.releasedNow = false;
			}
		}

		static void OnPressed(Key key)
		{
			KeyInfo& keyInfo = KeyTable[static_cast<KeyEnumInt>(key)];
			if (!keyInfo.pressed)
			{
				keyInfo.pressed = true;
				keyInfo.released = false;
				keyInfo.pressedNow = true;
			}
		}

		static void OnReleased(Key key)
		{
			KeyInfo& keyInfo = KeyTable[static_cast<KeyEnumInt>(key)];
			if (!keyInfo.released)
			{
				keyInfo.pressed = false;
				keyInfo.released = true;
				keyInfo.releasedNow = true;
			}
		}
	};
}
