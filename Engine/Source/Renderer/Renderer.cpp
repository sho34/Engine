#include "pch.h"
#include "Renderer.h"
#include "DeviceUtils.h"
#include "../Common/DirectXHelper.h"
#include "DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

std::mutex rendererMutex;

static std::shared_ptr<Renderer> renderer = nullptr;
std::shared_ptr<Renderer> Renderer::GetPtr() { return renderer; }

void Renderer::Initialize(HWND coreHwnd){
	renderer = shared_from_this();
	using namespace DeviceUtils::D3D12Device;
	using namespace DeviceUtils::RenderTarget;
	using namespace DeviceUtils::ConstantsBuffer;

	hwnd = coreHwnd;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug1> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	ComPtr<IDXGIAdapter4> dxgiAdapter4 = GetAdapter();
	d3dDevice = CreateDevice(dxgiAdapter4);
	commandQueue = CreateCommandQueue(d3dDevice);
	swapChain = CreateSwapChain(hwnd, commandQueue, numFrames);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	rtvDescriptorHeap = CreateDescriptorHeap(d3dDevice, numFrames);
	rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorHeap = CreateDescriptorHeap(d3dDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	NAME_D3D12_OBJECT(d3dDevice);
	NAME_D3D12_OBJECT(commandQueue);
	NAME_D3D12_OBJECT(rtvDescriptorHeap);
	NAME_D3D12_OBJECT(dsvDescriptorHeap);

	CreateCSUDescriptorHeap(d3dDevice, numFrames);
	
	UpdateRenderTargetViews(d3dDevice, swapChain, rtvDescriptorHeap, renderTargets, numFrames);
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	UpdateDepthStencilView(d3dDevice, dsvDescriptorHeap, depthStencil, width, height);
	
	for (int i = 0; i < numFrames; ++i) {
		commandAllocators[i] = CreateCommandAllocator(d3dDevice);
		commandAllocators[i]->SetName((L"commandAllocator[" + std::to_wstring(i) + L"]").c_str());
	}
	
	commandList = CreateCommandList(d3dDevice, commandAllocators[backBufferIndex]);
	NAME_D3D12_OBJECT(commandList);

	fence = CreateFence(d3dDevice);
	fenceEvent = CreateEventHandle();
	UpdateViewportPerspective();

	//D3D11On12
	CreateD3D11On12Device(d3dDevice, commandQueue, d3d11Device, d3d11on12Device, d3d11DeviceContext, dxgiDevice);

	//D2D1
	CreateD2D1Device(dxgiDevice, d2d1Factory, d2d1Device, d2d1DeviceContext);
	DX::ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &dWriteFactory));
	UpdateD2D1RenderTargets(d3d11on12Device, d2d1DeviceContext, renderTargets, wrappedBackBuffers, d2dRenderTargets, numFrames);
}

void Renderer::Destroy() {
	fence = nullptr;// ->Release();

	//release d2d
	//liberar d2d
	dWriteFactory = nullptr;//->Release();
	for (int i = 0; i < numFrames; i++) {
		d2dRenderTargets[i] = nullptr;// ->Release();
	}
	d2d1DeviceContext = nullptr;//->Release();
	d2d1Device = nullptr;//->Release();
	d2d1Factory = nullptr;//->Release();
	for (int i = 0; i < numFrames; i++) {
		wrappedBackBuffers[i] = nullptr;// ->Release();
	}
	dxgiDevice = nullptr;//->Release();
	d3d11DeviceContext = nullptr;//->Release();
	d3d11on12Device = nullptr;//->Release();
	d3d11Device = nullptr;//->Release();

	//release d3d12
	//liberar d3d12
	commandList = nullptr;// ->Release();
	//for (auto commandAllocator : commandAllocators) {
	//	commandAllocator->Release();
	//}
	for (int i = 0; i < numFrames; i++) {
		commandAllocators[i] = nullptr;// ->Release();
	}
	depthStencil = nullptr;// ->Release();
	//for (auto renderTarget : renderTargets) {
	//	renderTarget->Release();
	//}
	for (int i = 0; i < numFrames; i++) {
		renderTargets[i] = nullptr;// ->Release();
	}
	dsvDescriptorHeap = nullptr;// ->Release();
	rtvDescriptorHeap = nullptr;// ->Release();
	swapChain = nullptr;// ->Release();
	commandQueue = nullptr;// ->Release();
	d3dDevice = nullptr;//->Release();
}

void Renderer::UpdateViewportPerspective() {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	float aspectRatio = static_cast<FLOAT>(width) / static_cast<FLOAT>(height);
	scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	screenViewport = { 0.0f, 0.0f, static_cast<FLOAT>(width), static_cast<FLOAT>(height), 0.0f, 1.0f };
	perspectiveMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 100.0f);
}

void Renderer::Resize(UINT width, UINT height) {
	using namespace DeviceUtils::D3D12Device;
	using namespace DeviceUtils::RenderTarget;

	Flush(commandQueue, fence, fenceValue, fenceEvent);

	for (UINT i = 0; i < numFrames; ++i)
	{
		renderTargets[i].Reset();
		frameFenceValues[i] = frameFenceValues[backBufferIndex];
	}
	depthStencil.Reset();

	//free d3d11on12 & d2d resources
	for (UINT i = 0; i < numFrames; ++i)
	{
		wrappedBackBuffers[i].Reset();
		d2dRenderTargets[i].Reset();
	}
	d2d1DeviceContext->SetTarget(nullptr);
	d3d11DeviceContext->Flush();

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	DX::ThrowIfFailed(swapChain->GetDesc(&swapChainDesc));
	DX::ThrowIfFailed(swapChain->ResizeBuffers(numFrames, width, height,
		swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	UpdateRenderTargetViews(d3dDevice, swapChain, rtvDescriptorHeap, renderTargets, numFrames);
	UpdateDepthStencilView(d3dDevice, dsvDescriptorHeap, depthStencil, width, height);
	UpdateViewportPerspective();

	//d2d1 render targets
	UpdateD2D1RenderTargets(d3d11on12Device, d2d1DeviceContext, renderTargets, wrappedBackBuffers, d2dRenderTargets, numFrames);
}

void Renderer::ResetCommands() {
	auto commandAllocator = commandAllocators[backBufferIndex];
	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);
}

void Renderer::SetCSUDescriptorHeap() {
	using namespace DeviceUtils::ConstantsBuffer;
	ID3D12DescriptorHeap* ppHeaps[] = { GetCSUDescriptorHeap().Get()};
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void Renderer::SetShadowMapTarget(ComPtr<ID3D12Resource> shadowMap, D3D12_CPU_DESCRIPTOR_HANDLE shadowMapCpuHandle, D3D12_RECT	shadowMapScissorRect, D3D12_VIEWPORT shadowMapViewport) {
	commandList->RSSetViewports(1, &shadowMapViewport);
	commandList->RSSetScissorRects(1, &shadowMapScissorRect);
	commandList->ClearDepthStencilView(shadowMapCpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->OMSetRenderTargets(0, NULL, false, &shadowMapCpuHandle);
}

void Renderer::SetRenderTargets() {
	auto backBuffer = renderTargets[backBufferIndex];

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	commandList->RSSetViewports(1, &screenViewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, rtvDescriptorSize);
	commandList->ClearRenderTargetView(rtv, DirectX::Colors::CornflowerBlue, 0, nullptr);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
}

void Renderer::ExecuteCommands()
{
	//auto backBuffer = renderTargets[backBufferIndex];
	//CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	//commandList->ResourceBarrier(1, &barrier);

	DX::ThrowIfFailed(commandList->Close());
	ID3D12CommandList* const commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}

void Renderer::Set2DRenderTarget() {

	d3d11on12Device->AcquireWrappedResources(wrappedBackBuffers[backBufferIndex].GetAddressOf(), 1);
	d2d1DeviceContext->SetTarget(d2dRenderTargets[backBufferIndex].Get());
}

void Renderer::Present() {
	using namespace DeviceUtils::D3D12Device;

	//flush d3d11on12
	d3d11on12Device->ReleaseWrappedResources(wrappedBackBuffers[backBufferIndex].GetAddressOf(), 1);
	d3d11DeviceContext->Flush();

	//present
	DX::ThrowIfFailed(swapChain->Present(1, 0));
	frameFenceValues[backBufferIndex] = Signal(commandQueue, fence, fenceValue);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//make the CPU to wait for the GPU to finish the current processing
	//hacer que la CPU espere a que el procesamiento de la GPU termine
	WaitForFenceValue(fence, frameFenceValues[backBufferIndex], fenceEvent);
}

void Renderer::CloseCommandsAndFlush() {
	using namespace DeviceUtils::D3D12Device;

	commandList->Close();
	ID3D12CommandList* const commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	Flush(commandQueue, fence, fenceValue, fenceEvent);
}
