#include "pch.h"
#include "Builder.h"
#include "../../../Common/DirectXHelper.h"

namespace DeviceUtils::D3D12Device {

	ComPtr<IDXGIAdapter4> GetAdapter() {

		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		DX::ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
						D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)) &&
					dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					DX::ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
				}
			}
		}

		return dxgiAdapter4;
	}

	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter)
	{
		ComPtr<ID3D12Device2> d3d12Device2;
		DX::ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&d3d12Device2)));

		// Enable debug messages in debug mode.
#if defined(_DEBUG)
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// Suppress whole categories of messages

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
					D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,				// I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,										// This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,									// This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,									// This occurs when there are uninitialized descriptors in a descriptor table, even when a shader does not access the missing descriptors.
				D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED,								// D3D11On12
				//D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_RENDERTARGETVIEW_NOT_SET		// Shadow Map of 5.1 sucks
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			NewFilter.DenyList.NumSeverities = _countof(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = _countof(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			DX::ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
		}
#endif

		return d3d12Device2;
	}

	ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		DX::ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

		return d3d12CommandQueue;
	}

	ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hwnd, ComPtr<ID3D12CommandQueue> commandQueue, UINT bufferCount)
	{
		ComPtr<IDXGISwapChain4> dxgiSwapChain4;
		ComPtr<IDXGIFactory4> dxgiFactory4;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		DX::ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

		RECT rect;
		GetWindowRect(hwnd, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = bufferCount;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		ComPtr<IDXGISwapChain1> swapChain1;
		DX::ThrowIfFailed(
			dxgiFactory4->CreateSwapChainForHwnd(
				commandQueue.Get(), hwnd, &swapChainDesc, &fsSwapChainDesc, nullptr, swapChain1.GetAddressOf()
			)
		);

		DX::ThrowIfFailed(swapChain1.As(&dxgiSwapChain4));

		return dxgiSwapChain4;
	}

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
	{
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		D3D12_DESCRIPTOR_HEAP_DESC desc = { .Type = type, .NumDescriptors = numDescriptors, .Flags = flags };
		DX::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
		return descriptorHeap;
	}

	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		DX::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
		return commandAllocator;
	}

	ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> commandAllocator)
	{
		ComPtr<ID3D12GraphicsCommandList2> commandList;
		DX::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
		DX::ThrowIfFailed(commandList->Close());
		return commandList;
	}

	ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3D12Fence> fence;
		DX::ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		return fence;
	}

	HANDLE CreateEventHandle()
	{
		HANDLE fenceEvent;
		fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (fenceEvent == nullptr) {
			DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		return fenceEvent;
	}

	void CreateD3D11On12Device(ComPtr<ID3D12Device2> d3dDevice, ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D11Device>& d3d11Device, ComPtr<ID3D11On12Device>& d3d11on12Device, ComPtr<ID3D11DeviceContext>& d3d11DeviceContext, ComPtr<IDXGIDevice>& dxgiDevice) {
		UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
		// Enable the debug layer (requires the Graphics Tools "optional feature").
		// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				// Enable additional debug layers.
				d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			}
		}
#endif

		DX::ThrowIfFailed(D3D11On12CreateDevice(d3dDevice.Get(), d3d11DeviceFlags,
			nullptr, 0, reinterpret_cast<IUnknown**>(commandQueue.GetAddressOf()),
			1, 0, &d3d11Device, &d3d11DeviceContext, nullptr));

		DX::ThrowIfFailed(d3d11Device.As(&d3d11on12Device));
		DX::ThrowIfFailed(d3d11on12Device.As(&dxgiDevice));
	}

	void CreateD2D1Device(ComPtr<IDXGIDevice> dxgiDevice, ComPtr<ID2D1Factory6>& d2d1Factory, ComPtr<ID2D1Device5>& d2d1Device, ComPtr<ID2D1DeviceContext5>& d2d1DeviceContext) {
		D2D1_FACTORY_OPTIONS d2d1FactoryOptions = {};
#if defined(_DEBUG)
		d2d1FactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
		DX::ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory6), &d2d1FactoryOptions, &d2d1Factory));
		DX::ThrowIfFailed(d2d1Factory->CreateDevice(dxgiDevice.Get(), &d2d1Device));
		DX::ThrowIfFailed(d2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d1DeviceContext));
	}

}