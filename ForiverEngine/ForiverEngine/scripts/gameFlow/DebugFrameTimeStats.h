#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// <para>フレーム時間の統計情報をデバッグ表示するためのクラス</para>
	/// <para>一定数分のフレーム時間を記録し、その代表値を算出できる</para>
	/// </summary>
	class DebugFrameTimeStats
	{
	public:
		/// <summary>
		/// recordCount 分のフレーム時間を記録するように初期化する
		/// </summary>
		explicit DebugFrameTimeStats(int recordCount)
			: recordCount(recordCount)
		{
			frameTimes.resize(recordCount, 0.0);
		}

		/// <summary>
		/// <para>値を記録する</para>
		/// <para>古い値から上書きされていく</para>
		/// </summary>
		void Record(double frameTime) noexcept
		{
			frameTimes[currentIndex] = frameTime;
			currentIndex = (currentIndex + 1) % recordCount;
		}

		/// <summary>
		/// 平均値を算出する
		/// </summary>
		double CalculateMean() const noexcept
		{
			double sum = 0.0;
			for (const double ft : frameTimes)
				sum += ft;

			return sum / recordCount;
		}

	private:
		const int recordCount; // 記録するフレーム時間の数

		std::vector<double> frameTimes; // フレーム時間の記録配列 (リングバッファ)
		int currentIndex = 0; // 現在の書き込みインデックス
	};
}
