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
	//Agile<CoreWindow>                       window;
	HWND									hwnd;

	//d3d device resources in order of creation
	ComPtr<ID3D12Device2>                   d3dDevice;
	ComPtr<ID3D12CommandQueue>              commandQueue;
	ComPtr<IDXGISwapChain4>                 swapChain;

	//d3d12 descriptor heaps
	UINT                                    rtvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap>            rtvDescriptorHeap;
	
	ComPtr<ID3D12DescriptorHeap>            dsvDescriptorHeap;
	UINT																		dsvDescriptorSize;

	ComPtr<ID3D12CommandAllocator>          commandAllocators[numFrames];
	ComPtr<ID3D12GraphicsCommandList2>      commandList;

	//d3d11on12 resources to allow dwrite&d2d1 interop
	ComPtr<ID3D11Device>                    d3d11Device;
	ComPtr<ID3D11On12Device>                d3d11on12Device;
	ComPtr<ID3D11DeviceContext>             d3d11DeviceContext;
	ComPtr<IDXGIDevice>                     dxgiDevice;
	ComPtr<ID3D11Resource>                  wrappedBackBuffers[numFrames];

	//d2d1 resources
	ComPtr<ID2D1Factory6>                   d2d1Factory;
	ComPtr<ID2D1Device5>                    d2d1Device;
	ComPtr<ID2D1DeviceContext5>             d2d1DeviceContext;
	ComPtr<ID2D1Bitmap1>                    d2dRenderTargets[numFrames];

	//dwrite resources
	ComPtr<IDWriteFactory>                  dWriteFactory;

	//GPU <-> CPU synchronization 
	ComPtr<ID3D12Fence>                     fence;
	UINT64                                  fenceValue = 0;
	UINT64                                  frameFenceValues[numFrames] = {};
	HANDLE                                  fenceEvent;

	//window based values
	D3D12_VIEWPORT                          screenViewport;
	D3D12_RECT                              scissorRect;

	//back buffer and depth stencil targets
	ComPtr<ID3D12Resource>                  renderTargets[numFrames];
	ComPtr<ID3D12Resource>                  depthStencil;
	UINT                                    backBufferIndex;

	//matrices
	XMMATRIX                                perspectiveMatrix;

	void Initialize(HWND hwnd);
	void Destroy();
	void UpdateViewportPerspective();
	void Resize(UINT width, UINT height);
	void ResetCommands();
	void SetCSUDescriptorHeap();
	void CloseCommandsAndFlush();
	void SetShadowMapTarget(ComPtr<ID3D12Resource> shadowMap, D3D12_CPU_DESCRIPTOR_HANDLE shadowMapCpuHandle, D3D12_RECT	shadowMapScissorRect, D3D12_VIEWPORT shadowMapViewport);
	void SetRenderTargets();
	void Set2DRenderTarget();
	void ExecuteCommands();
	void Present();
};

