#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "scripts/gameFlow/Renderer/Context/Include.h"
#include "scripts/gameFlow/Renderer/Offscreen/AOffscreenRenderer.h"

namespace ForiverEngine
{
	/// <summary>
	/// テキスト描画用のレンダラー
	/// </summary>
	class TextRenderer : public AOffscreenRenderer
	{
	private:
		using Base = AOffscreenRenderer;

		// b0
		struct alignas(256) CBData0
		{
			std::uint32_t FontTextureSize[2];
			std::uint32_t TextUIDataSize[2];
			std::uint32_t InvalidFontTextureIndex;
			std::uint32_t FontTextureTextLength;
		};

	public:
		/// <summary>
		/// <para>コンストラクタで初期化される (全て空で埋まる)</para>
		/// <para>一方で、その後はデータを変更しても GPU 側に一切反映されないため、確実に UpdateDataAtGPU() を呼び出すこと</para>
		/// </summary>
		TextUIData data;

		explicit TextRenderer(const RenderContext& renderContext, const Lattice2& windowSize)
		{
			data = TextUIData::CreateEmpty(windowSize / TextUIData::FontTextureTextLength);

			// t1 (フォントテクスチャ)
			const Texture sr1Metadata = D3D12Utils::LoadTexture({ "assets/textures/font.png" });
			const GraphicsBuffer sr1 = D3D12Utils::InitSR(
				renderContext.device, renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, sr1Metadata);

			// t2 (データをテクスチャとしてアップロードする)
			const Texture sr2Metadata = data.CreateTexture();
			const GraphicsBuffer sr2 = D3D12Utils::InitSR(
				renderContext.device, renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, sr2Metadata);

			// b0
			const CBData0 cbData0 =
			{
				.FontTextureSize = { static_cast<std::uint32_t>(sr1Metadata.size.x), static_cast<std::uint32_t>(sr1Metadata.size.y) },
				.TextUIDataSize = { static_cast<std::uint32_t>(sr2Metadata.size.x), static_cast<std::uint32_t>(sr2Metadata.size.y) },
				.InvalidFontTextureIndex = static_cast<std::uint32_t>(Text::InvalidFontTextureIndex),
				.FontTextureTextLength = static_cast<std::uint32_t>(TextUIData::FontTextureTextLength),
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(renderContext.device, cbData0);

			Base::Init(
				renderContext, windowSize,
				{ cb0 }, { { sr1, sr1Metadata }, { sr2, sr2Metadata } }, D3D12Utils::GetShaderFilePath("Text"));
		}

		/// <summary>
		/// <para>データのテクスチャを再作成し、アップロードし直す</para>
		/// <para>このメソッドを実行しないと、data の変更を GPU 側に反映できない</para>
		/// </summary>
		void UpdateDataAtGPU(const RenderContext& renderContext)
		{
			// t2
			const Texture sr2Texture = data.CreateTexture();
			Base::ReUploadTexture(renderContext, sr2Texture, ShaderRegister::t2);
		}
	};
}
