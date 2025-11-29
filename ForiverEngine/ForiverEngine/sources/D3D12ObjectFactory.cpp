#include "../headers/D3D12ObjectFactory.h"

#include <string>
#include <functional>

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <d3dcommon.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace ForiverEngine
{
	// グラフィックアダプターを順に列挙していき、その Description が最初に Comparer にマッチしたものを返す
	// 見つからなかった場合は nullptr を返す
	static IDXGIAdapter* FindAvailableAdapter(IDXGIFactory* factory, std::function<bool(const std::wstring&)> descriptionComparer);

	IDXGIFactory6* D3D12ObjectFactory::CreateDXGIFactory()
	{
		IDXGIFactory6* outFactory = nullptr;
		if (CreateDXGIFactory1(IID_PPV_ARGS(&outFactory)) == S_OK)
			return outFactory;

		return nullptr;
	}

	ID3D12Device* D3D12ObjectFactory::CreateD3D12Device(IDXGIFactory* factory, D3D_FEATURE_LEVEL& outD3DFeatureLevel)
	{
		// 上から順に、対応している機能レベルを探して行く
		constexpr D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		// 利用可能なアダプターを取得 (デバイスを作成時に渡す)
		// 判定は緩いが、nullptr でもOKなので、一旦これで行く
		IDXGIAdapter* adapter = FindAvailableAdapter(factory, [](const auto& desc) -> bool
			{
				return desc.find(L"NVIDIA") != std::string::npos;
			});

		for (auto featureLevel : featureLevels)
		{
			ID3D12Device* outDevice = nullptr;
			if (D3D12CreateDevice(adapter, featureLevel, IID_PPV_ARGS(&outDevice)) == S_OK)
			{
				outD3DFeatureLevel = featureLevel;
				return outDevice;
			}
		}

		return nullptr;
	}

	IDXGIAdapter* FindAvailableAdapter(IDXGIFactory* factory, std::function<bool(const std::wstring&)> descriptionComparer)
	{
		IDXGIAdapter* adapter = nullptr;
		for (int i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			if (descriptionComparer(desc.Description))
				return adapter;
		}

		return nullptr;
	}
}
