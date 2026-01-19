#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// <para>一定時間経過を計測するタイマークラス</para>
	/// <para>毎フレーム OnEveryFrame() を呼び出し、経過時間を進める</para>
	/// <para>経過時間が設定された時間に達したかどうかは IsFinished() で取得できる</para>
	/// <para>タイマーをリセットするには Reset() を呼び出す (経過時間が0に戻る)</para>
	/// </summary>
	class Timer
	{
	public:
		// 派生クラスとか作りそうなので、一応書いておく
		virtual ~Timer() = default;

		explicit Timer(float durationSeconds) noexcept
			: durationSeconds(durationSeconds)
		{
		}

		/// <summary>
		/// 毎フレーム呼び出すこと
		/// </summary>
		void OnEveryFrame(float deltaSeconds) noexcept
		{
			elapsedSeconds += deltaSeconds;
			elapsedSeconds = std::clamp(elapsedSeconds, 0.0f, durationSeconds);
		}

		/// <summary>
		/// タイマーが終了しているかどうか
		/// </summary>
		bool IsFinished() const noexcept
		{
			return elapsedSeconds >= durationSeconds;
		}

		/// <summary>
		/// タイマーをリセットする
		/// </summary>
		void Reset() noexcept
		{
			elapsedSeconds = 0.0f;
		}

	private:
		const float durationSeconds;
		float elapsedSeconds = 0.0f;
	};
}
