#pragma once

#include <scripts/common/Include.h>

namespace ForiverEngine
{
	class ISound
	{
	public:
		virtual ~ISound() = default;

		virtual bool Play() = 0;
	};

	class SoundHelper final
	{
	public:
		// Impl の定義は .cpp にあるので、コンストラクタ/デストラクタも .cpp に書く必要がある
		SoundHelper();
		~SoundHelper();

		/// <returns>失敗したら nullptr</returns>
		std::unique_ptr<ISound> LoadSound(std::string_view path);

	private:
		struct Impl;                // 中身はここでは定義しない
		std::unique_ptr<Impl> impl; // 実装を所有する
	};
}
