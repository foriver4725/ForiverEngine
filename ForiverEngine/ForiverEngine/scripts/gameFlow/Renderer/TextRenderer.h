#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./AOffscreenRenderer.h"

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

		explicit TextRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize
		)
		{
			data = TextUIData::CreateEmpty(windowSize / TextUIData::FontTextureTextLength);

			// t1 (フォントテクスチャ)
			const Texture sr1Metadata = D3D12Utils::LoadTexture({ "assets/font.png" });
			const GraphicsBuffer sr1 = D3D12Utils::InitSR(device, commandList, commandQueue, commandAllocator, sr1Metadata);

			// t2 (データをテクスチャとしてアップロードする)
			const Texture sr2Metadata = data.CreateTexture();
			const GraphicsBuffer sr2 = D3D12Utils::InitSR(device, commandList, commandQueue, commandAllocator, sr2Metadata);

			// b0
			const CBData0 cbData0 =
			{
				.FontTextureSize = { static_cast<std::uint32_t>(sr1Metadata.width), static_cast<std::uint32_t>(sr1Metadata.height) },
				.TextUIDataSize = { static_cast<std::uint32_t>(sr2Metadata.width), static_cast<std::uint32_t>(sr2Metadata.height) },
				.InvalidFontTextureIndex = static_cast<std::uint32_t>(Text::InvalidFontTextureIndex),
				.FontTextureTextLength = static_cast<std::uint32_t>(TextUIData::FontTextureTextLength),
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(device, cbData0);

			Base::Init(device, windowSize, { cb0 }, { { sr1, sr1Metadata }, { sr2, sr2Metadata } }, "./shaders/Text.hlsl");
		}

		/// <summary>
		/// <para>データのテクスチャを再作成し、アップロードし直す</para>
		/// <para>このメソッドを実行しないと、data の変更を GPU 側に反映できない</para>
		/// </summary>
		void UpdateDataAtGPU(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator
		)
		{
			const Texture sr2Texture = data.CreateTexture();
			Base::ReUploadTexture(device, commandList, commandQueue, commandAllocator, sr2Texture, ShaderRegister::t2); // t2
		}
	};
}
