#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

struct Renderer: public std::enable_shared_from_this<Renderer>
{
	~Renderer() {}
	static std::shared_ptr<Renderer> GetPtr();
	static const UINT numFrames = 3;
	static const constexpr float fovAngleY = (70.0f * XM_PI / 180.0f);

	//app window reference
	//Agile<CoreWindow> window;
	HWND hwnd;

	//d3d device resources in order of creation
	CComPtr<ID3D12Device2> d3dDevice;
	CComPtr<ID3D12CommandQueue>              commandQueue;
	CComPtr<IDXGISwapChain4>                 swapChain;

	//d3d12 descriptor heaps
	UINT                                    rtvDescriptorSize;
	CComPtr<ID3D12DescriptorHeap>            rtvDescriptorHeap;
	
	CComPtr<ID3D12DescriptorHeap>            dsvDescriptorHeap;
	UINT																		dsvDescriptorSize;

	CComPtr<ID3D12CommandAllocator>          commandAllocators[numFrames];
	CComPtr<ID3D12GraphicsCommandList2>      commandList;

	//d3d11on12 resources to allow dwrite&d2d1 interop
	CComPtr<ID3D11Device>                    d3d11Device;
	CComPtr<ID3D11On12Device>                d3d11on12Device;
	CComPtr<ID3D11DeviceContext>             d3d11DeviceContext;
	CComPtr<IDXGIDevice>                     dxgiDevice;
	CComPtr<ID3D11Resource>                  wrappedBackBuffers[numFrames];

	//d2d1 resources
	CComPtr<ID2D1Factory6>                   d2d1Factory;
	CComPtr<ID2D1Device5>                    d2d1Device;
	CComPtr<ID2D1DeviceContext5>             d2d1DeviceContext;
	CComPtr<ID2D1Bitmap1>                    d2dRenderTargets[numFrames];

	//dwrite resources
	CComPtr<IDWriteFactory>                  dWriteFactory;

	//GPU <-> CPU synchronization 
	CComPtr<ID3D12Fence>                     fence;
	UINT64                                  fenceValue = 0;
	UINT64                                  frameFenceValues[numFrames] = {};
	HANDLE                                  fenceEvent;

	//window based values
	D3D12_VIEWPORT                          screenViewport;
	D3D12_RECT                              scissorRect;

	//back buffer and depth stencil targets
	CComPtr<ID3D12Resource>                  renderTargets[numFrames];
	CComPtr<ID3D12Resource>                  depthStencil;
	UINT                                    backBufferIndex;

	void Initialize(HWND hwnd);
	void Destroy();
	void UpdateViewportPerspective();
	void Resize(UINT width, UINT height);
	void ResetCommands();
	void SetCSUDescriptorHeap();
	void CloseCommandsAndFlush();
	void SetShadowMapTarget(CComPtr<ID3D12Resource>& shadowMap, D3D12_CPU_DESCRIPTOR_HANDLE shadowMapCpuHandle, D3D12_RECT	shadowMapScissorRect, D3D12_VIEWPORT shadowMapViewport);
	void SetRenderTargets();
	void Set2DRenderTarget();
	void ExecuteCommands();
	void Present();
};

