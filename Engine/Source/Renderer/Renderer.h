#pragma once

#include <RenderPass/RenderPass.h>
#include <dxgi1_6.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Templates;

struct Renderer : public std::enable_shared_from_this<Renderer>
{
	~Renderer() {}
	static const constexpr unsigned int numFrames = 3;
	static const constexpr float fovAngleY = (70.0f * XM_PI / 180.0f);

	//app window reference
	HWND hwnd;

	//d3d device resources in order of creation
	CComPtr<ID3D12Device2> d3dDevice;
	CComPtr<ID3D12CommandQueue> commandQueue;
	CComPtr<IDXGISwapChain4> swapChain;

	//command
	CComPtr<ID3D12CommandAllocator> commandAllocators[numFrames];
	CComPtr<ID3D12GraphicsCommandList2> commandList;

	//GPU <-> CPU synchronization 
	CComPtr<ID3D12Fence> fence;
	unsigned long long fenceValue = 0;
	unsigned long long frameFenceValues[numFrames] = {};
	HANDLE fenceEvent;

	//window based values
	D3D12_VIEWPORT screenViewport;
	D3D12_RECT scissorRect;

	unsigned int backBufferIndex;

	//the swap chain pass
	std::shared_ptr<RenderPassInstance> swapChainPass;

	//CREATE
	void Initialize(HWND hwnd);
	void CreateComputeEngine();
	void CreateSwapChainPass();

	//READ&GET

	//UPDATE
	void UpdateViewportPerspective();
	void Resize(unsigned int width, unsigned int height);
	void ResetCommands() const;
	void SetCSUDescriptorHeap() const;
	void CloseCommandsAndFlush();
	void RenderCriticalFrame(std::function<void()> callback = []() {});
	void ExecuteCommands() const;
	void Present();
	void Flush();

	//DESTROY
	void DestroySwapChainPass();
	void Destroy();
};

