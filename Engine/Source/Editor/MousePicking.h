#pragma once
#include <memory>
#include <RenderPass/RenderPass.h>
#include <SceneObject.h>
#include <Mouse.h>

struct MousePicking
{
	std::shared_ptr<RenderPassInstance> pickingPass;
	CComPtr<ID3D12Resource> pickingCpuBuffer;
	unsigned int pickingX = 0U;
	unsigned int pickingY = 0U;
	bool doPicking = false;
	nostd::VectorSet<std::shared_ptr<SceneObject>> pickedObjects;
	SceneObjectType pickedType;
	void Reset();
	void StartPicking(DirectX::Mouse::State state);
	bool CanPick(DirectX::Mouse::State state) const;
	void Pick();
	bool MouseMoved(DirectX::Mouse::State state) const;
};