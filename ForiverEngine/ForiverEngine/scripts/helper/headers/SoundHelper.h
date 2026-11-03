#pragma once

#include <scripts/common/Include.h>

namespace ForiverEngine
{
	class ISound
	{
	public:
		virtual ~ISound() = default;

		virtual bool Play() = 0;
		virtual void StopAll() = 0;
	};

	class SoundHelper final
	{
	public:
		// Impl の定義は .cpp にあるので、コンストラクタ/デストラクタも .cpp に書く必要がある
		SoundHelper();
		~SoundHelper();

		/// <summary>
		/// サウンドをロードする
		/// </summary>
		/// <param name="path">パス</param>
		/// <param name="maxPlayCount">同時に鳴らせる最大数。超えたら再生は常に失敗する</param>
		/// <returns>失敗したら nullptr</returns>
		std::unique_ptr<ISound> LoadSound(std::string_view path, int maxPlayCount);

	private:
		struct Impl;                // 中身はここでは定義しない
		std::unique_ptr<Impl> impl; // 実装を所有する
	};
}
