#include "../headers/InputHelper.h"

namespace ForiverEngine
{
	void InputHelper::InitKeyTable()
	{
		for (KeyEnumInt i = 0; i < static_cast<KeyEnumInt>(Key::Count); ++i)
			KeyTable[i] = { .released = true };
	}

	void InputHelper::OnEveryFrame()
	{
		for (KeyEnumInt i = 0; i < static_cast<KeyEnumInt>(Key::Count); ++i)
		{
			KeyInfo& keyInfo = KeyTable[i];
			keyInfo.pressedNow = false;
			keyInfo.releasedNow = false;
		}
	}

	void InputHelper::OnPressed(Key key)
	{
		KeyInfo& keyInfo = KeyTable[static_cast<KeyEnumInt>(key)];
		if (!keyInfo.pressed)
		{
			keyInfo.pressed = true;
			keyInfo.released = false;
			keyInfo.pressedNow = true;
		}
	}

	void InputHelper::OnReleased(Key key)
	{
		KeyInfo& keyInfo = KeyTable[static_cast<KeyEnumInt>(key)];
		if (!keyInfo.released)
		{
			keyInfo.pressed = false;
			keyInfo.released = true;
			keyInfo.releasedNow = true;
		}
	}

	void InputHelper::OnMouseMove(LPARAM lparam)
	{
		const int x = GET_X_LPARAM(lparam);
		const int y = GET_Y_LPARAM(lparam);
		const Vector2 position = Vector2(static_cast<float>(x), static_cast<float>(y));

		MousePosition = position;
		MouseDelta = MousePosition - MousePositionPrev;
		MousePositionPrev = MousePosition;
	}

	Key InputHelper::ConvertVKToKey(WPARAM wparam, LPARAM lparam)
	{
		switch (wparam)
		{
			// マウス
		case VK_LBUTTON: return Key::LMouse;
		case VK_RBUTTON: return Key::RMouse;
		case VK_MBUTTON: return Key::MMouse;

			// 数字
		case '0': return Key::N0;
		case '1': return Key::N1;
		case '2': return Key::N2;
		case '3': return Key::N3;
		case '4': return Key::N4;
		case '5': return Key::N5;
		case '6': return Key::N6;
		case '7': return Key::N7;
		case '8': return Key::N8;
		case '9': return Key::N9;

			// 数字 (テンキー)
		case VK_NUMPAD0: return Key::NP0;
		case VK_NUMPAD1: return Key::NP1;
		case VK_NUMPAD2: return Key::NP2;
		case VK_NUMPAD3: return Key::NP3;
		case VK_NUMPAD4: return Key::NP4;
		case VK_NUMPAD5: return Key::NP5;
		case VK_NUMPAD6: return Key::NP6;
		case VK_NUMPAD7: return Key::NP7;
		case VK_NUMPAD8: return Key::NP8;
		case VK_NUMPAD9: return Key::NP9;

			// 英字
		case 'A': return Key::A;
		case 'B': return Key::B;
		case 'C': return Key::C;
		case 'D': return Key::D;
		case 'E': return Key::E;
		case 'F': return Key::F;
		case 'G': return Key::G;
		case 'H': return Key::H;
		case 'I': return Key::I;
		case 'J': return Key::J;
		case 'K': return Key::K;
		case 'L': return Key::L;
		case 'M': return Key::M;
		case 'N': return Key::N;
		case 'O': return Key::O;
		case 'P': return Key::P;
		case 'Q': return Key::Q;
		case 'R': return Key::R;
		case 'S': return Key::S;
		case 'T': return Key::T;
		case 'U': return Key::U;
		case 'V': return Key::V;
		case 'W': return Key::W;
		case 'X': return Key::X;
		case 'Y': return Key::Y;
		case 'Z': return Key::Z;

			// ファンクション
		case VK_F1: return Key::F1;
		case VK_F2: return Key::F2;
		case VK_F3: return Key::F3;
		case VK_F4: return Key::F4;
		case VK_F5: return Key::F5;
		case VK_F6: return Key::F6;
		case VK_F7: return Key::F7;
		case VK_F8: return Key::F8;
		case VK_F9: return Key::F9;
		case VK_F10: return Key::F10;
		case VK_F11: return Key::F11;
		case VK_F12: return Key::F12;
		case VK_F13: return Key::F13;
		case VK_F14: return Key::F14;
		case VK_F15: return Key::F15;
		case VK_F16: return Key::F16;
		case VK_F17: return Key::F17;
		case VK_F18: return Key::F18;
		case VK_F19: return Key::F19;
		case VK_F20: return Key::F20;
		case VK_F21: return Key::F21;
		case VK_F22: return Key::F22;
		case VK_F23: return Key::F23;
		case VK_F24: return Key::F24;

			// 矢印
		case VK_UP: return Key::Up;
		case VK_DOWN: return Key::Down;
		case VK_LEFT: return Key::Left;
		case VK_RIGHT: return Key::Right;

			// その他 (1つのみ)
		case VK_SPACE: return Key::Space;
		case VK_TAB: return Key::Tab;
		case VK_RETURN: return Key::Enter;
		case VK_BACK: return Key::Backspace;
		case VK_DELETE: return Key::Delete;
		case VK_ESCAPE: return Key::Escape;

			// その他 (複数存在)
		case VK_SHIFT:
		{
			const UINT scancode = (lparam >> 16) & 0xff; // lparam の 16〜23bit
			const UINT vkEx = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX); // scan code → 仮想キー（左右判定可能）
			return (vkEx == VK_RSHIFT) ? Key::RShift : Key::LShift;
		}
		case VK_CONTROL:
		{
			const bool isRight = (lparam & (1 << 24)) != 0;
			return isRight ? Key::RCtrl : Key::LCtrl;
		}
		case VK_MENU:
		{
			const bool isRight = (lparam & (1 << 24)) != 0;
			return isRight ? Key::RAlt : Key::LAlt;
		}
		}

		return Key::Unknown;
	}

	KeyInfo InputHelper::GetKeyInfo(Key key)
	{
		return KeyTable[static_cast<KeyEnumInt>(key)];
	}
}
