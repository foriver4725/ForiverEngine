#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>

namespace ForiverEngine
{
	template<typename TVertexData>
	struct AMesh
	{
	public:
		virtual ~AMesh() = default;

	protected:
		virtual const std::vector<TVertexData>& GetVertices() const = 0;
		virtual const std::vector<std::uint32_t>& GetIndices() const = 0;

	public:
		/// <summary>
		/// VBV, IBV を作成して返す
		/// </summary>
		std::pair<VertexBufferView, IndexBufferView> CreateRenderViews(const Device& device) const
		{
			const std::vector<TVertexData>& vertices = this->GetVertices();          // メッシュのプロパティ
			const TVertexData* verticesPtr = vertices.data();                        // 先頭ポインタ
			const int vertexSize = static_cast<int>(sizeof(vertices[0]));            // 要素1つ分のメモリサイズ
			const int verticesSize = static_cast<int>(vertices.size() * vertexSize); // 全体のメモリサイズ

			const std::vector<std::uint32_t>& indices = this->GetIndices();          // メッシュのプロパティ
			const std::uint32_t* indicesPtr = indices.data();                        // 先頭ポインタ
			const int indexSize = static_cast<int>(sizeof(indices[0]));              // 要素1つ分のメモリサイズ
			const int indicesSize = static_cast<int>(indices.size() * indexSize);    // 全体のメモリサイズ

			const GraphicsBuffer vb = D3D12Helper::CreateGraphicsBuffer1D(device, verticesSize, true);
			if (!vb)
				ShowError(L"頂点バッファーの作成に失敗しました");
			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(vb, static_cast<const void*>(verticesPtr), verticesSize))
				ShowError(L"頂点バッファーを GPU 側にコピーすることに失敗しました");
			const VertexBufferView vbv = D3D12Helper::CreateVertexBufferView(vb, verticesSize, vertexSize);

			const GraphicsBuffer ib = D3D12Helper::CreateGraphicsBuffer1D(device, indicesSize, true);
			if (!ib)
				ShowError(L"インデックスバッファーの作成に失敗しました");
			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(ib, static_cast<const void*>(indicesPtr), indicesSize))
				ShowError(L"インデックスバッファーを GPU 側にコピーすることに失敗しました");
			const IndexBufferView ibv = D3D12Helper::CreateIndexBufferView(ib, indicesSize, Format::R_U32);

			return { vbv, ibv };
		}
	};
}
