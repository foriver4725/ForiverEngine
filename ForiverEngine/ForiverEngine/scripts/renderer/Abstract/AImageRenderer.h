#pragma once

#include "scripts/renderer/IncludeInternal.h"
#include "scripts/renderer/Context/Include.h"
#include "./AOffscreenRenderer.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>1枚の画像について、位置やスケールを指定して描画するレンダラー</para>
	/// <para>b0, t1 を使用するので、派生クラスでさらにアップロードしたい場合は、それぞれ</para>
	/// <para>b1~, t2~ から使用すること</para>
	/// </summary>
	class AImageRenderer : public AOffscreenRenderer
	{
	private:
		using Base = AOffscreenRenderer;

		// b0
		struct alignas(256) CBData0
		{
			std::uint32_t TextureSize[2]; // 画像テクスチャ全体のサイズ
			std::uint32_t ClipMin[2]; // 画像テクスチャの中で、どこからどこまでを切り取って使うか (min. テクスチャ自体のピクセル座標)
			std::uint32_t ClipMax[2]; // 画像テクスチャの中で、どこからどこまでを切り取って使うか (max. テクスチャ自体のピクセル座標)

			Lattice2 Position; // 画像の中心座標
			Vector2 Scale;

			std::uint32_t IsDrawEnabled; // 描画するかどうか (0: しない, 1: する)
		};

	public:
		void Init(
			const RenderContext& renderContext, const Lattice2& windowSize,
			const std::string& imageFilePath,
			const Lattice2& position, // スクリーン上の座標 (ピクセル単位. 画像の中心がこの位置に来る)
			const Vector2& clipUVMin, const Vector2& clipUVMax, // 画像のどの部分を切り取って使うか (UV座標で指定)
			const Lattice2& drawSize, // 描画サイズ (ピクセル単位. 画像の切り取った部分を、このサイズで描画する)
			bool initDrawEnabled // 初期状態で描画するかどうか
		)
		{
			// t1 (画像テクスチャ)
			const Texture sr1Metadata = D3D12Utils::LoadTexture({ imageFilePath });
			const GraphicsBuffer sr1 = D3D12Utils::InitSR(
				renderContext.device, renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, sr1Metadata);

			// 画像の元サイズに対するスケールを計算
			const Vector2 scale =
			{
				static_cast<float>(drawSize.x) / static_cast<float>((clipUVMax.x - clipUVMin.x) * sr1Metadata.size.x),
				static_cast<float>(drawSize.y) / static_cast<float>((clipUVMax.y - clipUVMin.y) * sr1Metadata.size.y),
			};

			// b0
			const CBData0 cbData0 =
			{
				.TextureSize =
				{
					static_cast<std::uint32_t>(sr1Metadata.size.x),
					static_cast<std::uint32_t>(sr1Metadata.size.y)
				},
				.ClipMin =
				{
					static_cast<std::uint32_t>(clipUVMin.x * sr1Metadata.size.x),
					static_cast<std::uint32_t>(clipUVMin.y * sr1Metadata.size.y)
				},
				.ClipMax =
				{
					static_cast<std::uint32_t>(clipUVMax.x * sr1Metadata.size.x) - 1,
					static_cast<std::uint32_t>(clipUVMax.y * sr1Metadata.size.y) - 1
				},

				.Position = position,
				.Scale = scale,

				.IsDrawEnabled = (initDrawEnabled ? 1u : 0u),
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(renderContext.device, cbData0, &cb0VirtualPtr);

			Base::Init(
				renderContext, windowSize,
				{ cb0 }, { { sr1, sr1Metadata } }, D3D12Utils::GetShaderFilePath("Image"));
		}

		// Init 後に使うこと!
		bool GetDrawEnabled() const
		{
			return (cb0VirtualPtr->IsDrawEnabled == 0u) ? false : true;
		}
		// Init 後に使うこと!
		void SetDrawEnabled(bool enabled)
		{
			cb0VirtualPtr->IsDrawEnabled = (enabled ? 1u : 0u);
		}

	private:
		CBData0* cb0VirtualPtr = nullptr;
	};
}
