#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

namespace ForiverEngine
{
	// TValue は比較可能であること
	template<typename TValue>
	concept TrackedValue_CompatibleType = requires(TValue a, TValue b)
	{
		{ a == b } -> std::convertible_to<bool>;
		{ a != b } -> std::convertible_to<bool>;
	};

	// TValue はコピー代入・ムーブ代入が可能であること、かつ例外を投げないこと
	template<typename TValue>
	concept TrackedValue_AssignableType =
		std::is_nothrow_copy_assignable_v<TValue> &&
		std::is_nothrow_move_assignable_v<TValue>;

	template<typename TValue>
	concept TrackedValue_Type =
		TrackedValue_CompatibleType<TValue> &&
		TrackedValue_AssignableType<TValue>;

	/// <summary>
	/// <para>値の変更を追跡するラッパークラス</para>
	/// <para>値が変更されたかどうかを示す dirty フラグを持つ</para>
	/// <para>- コンストラクタ : 常に dirty フラグが立つ</para>
	/// <para>- コピー代入 : 値が変化した場合に dirty フラグが立つ</para>
	/// <para>- ムーブ代入 : 常に dirty フラグが立つ</para>
	/// <para>DropDirty() を使うことで、 dirty フラグを取得しつつリセットできる</para>
	/// <para>dirty フラグは値の変更に連動して変化するだけなので、dirty フラグ自体を使ったロジックは一切無い</para>
	/// <para>なので DropDirty() は気軽に呼んでね. 代入ごとに1回まで呼ぶとかがおススメ</para>
	/// </summary>
	template<TrackedValue_Type TValue>
	class TrackedValue
	{
	public:
		// 派生クラスとか作りそうなので、一応書いておく
		virtual ~TrackedValue() = default;

		// コンストラクタ
		explicit TrackedValue(const TValue& initValue) noexcept
			: value(initValue), dirty(true)
		{
		}
		explicit TrackedValue(TValue&& initValue) noexcept
			: value(std::move(initValue)), dirty(true)
		{
		}

		// インスタンス自体の代入は許可しない
		TrackedValue& operator=(const TrackedValue&) = delete;
		TrackedValue& operator=(TrackedValue&&) = delete;

		// コピー代入
		TrackedValue& operator=(const TValue& value) noexcept
		{
			if (this->value != value)
			{
				dirty = true;
				this->value = value;
			}

			return *this;
		}
		// ムーブ代入
		TrackedValue& operator=(TValue&& value) noexcept
		{
			// 常に dirty フラグを立てる
			// コンストラクタで dirty フラグを立てているのと同じ理由
			dirty = true;
			this->value = std::move(value);

			return *this;
		}

		const TValue& GetValue() const noexcept
		{
			return value;
		}

		bool DropDirty() noexcept
		{
			if (dirty)
			{
				dirty = false;
				return true;
			}
			else
			{
				return false;
			}
		}

	private:
		TValue value;
		bool dirty = false;
	};
}
