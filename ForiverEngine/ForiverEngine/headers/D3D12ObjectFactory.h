#pragma once

struct ID3D12Device;
struct IDXGIFactory;
struct IDXGIFactory6;
enum D3D_FEATURE_LEVEL;

namespace ForiverEngine
{
	class D3D12ObjectFactory final
	{
	public:
		D3D12ObjectFactory() = delete;
		~D3D12ObjectFactory() = delete;

		/// <summary>
		/// DXGIFactory を作成して返す (失敗したら nullptr)
		/// </summary>
		static IDXGIFactory6* CreateDXGIFactory();

		/// <summary>
		/// <para>D3D12Device を作成して返す (失敗したら nullptr)</para>
		/// なるべく最新の FeatureLevel に対応したものを探す (この値も outD3DFeatureLevel にセットされる)
		/// </summary>
		static ID3D12Device* CreateD3D12Device(IDXGIFactory* factory, D3D_FEATURE_LEVEL& outD3DFeatureLevel);
	};
}
