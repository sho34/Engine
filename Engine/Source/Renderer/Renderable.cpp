#include "pch.h"
#include <ppltasks.h>
#include <vector>
#include "Renderable.h"
#include "../Common/d3dx12.h"
#include "../Common/DirectXHelper.h"
#include "../Scene/Camera/Camera.h"
#include "../Scene/Lights/Lights.h"
#include "../Templates/Model3D.h"
#include "../Animation/Animated.h"
#include <DirectXMath.h>

extern std::mutex rendererMutex;
namespace Renderable {

  std::map<std::wstring, std::shared_ptr<Renderable>> renderables;
  std::map<std::wstring, std::shared_ptr<Renderable>> animables;

  std::shared_ptr<Renderable> CreateRenderable(const std::shared_ptr<Renderer>& renderer, const RenderableDefinition& renderableParams, LoadRenderableFn loadFn)
  {
    assert(renderableParams.meshMaterials.empty() xor renderableParams.modelMaterials.second.empty());
    assert(renderableParams.name != L"");

    auto meshMaterials = renderableParams.meshMaterials;
    auto meshMaterialsShadowMap = renderableParams.meshMaterialsShadowMap;
    using namespace Animation;

    std::set<MeshPtr> skipMeshes;

    std::shared_ptr<Animated> animation = nullptr;
    if (meshMaterials.empty())  {
      meshMaterials = MeshMaterialMap();
      auto model = renderableParams.modelMaterials.first;
      auto materials = renderableParams.modelMaterials.second;
      animation = renderableParams.modelMaterials.first->animations;

      for (UINT i = 0; i < model->meshes.size(); i++) {
        meshMaterials[model->meshes[i]] = materials[i];
        if (renderableParams.skipMeshes.contains(i)) {
          skipMeshes.insert(model->meshes[i]);
        }
      }
    }

    if (meshMaterialsShadowMap.empty() && !renderableParams.modelMaterialsShadowMap.second.empty()) {
      meshMaterialsShadowMap = MeshMaterialMap();
      auto model = renderableParams.modelMaterialsShadowMap.first;
      auto materials = renderableParams.modelMaterialsShadowMap.second;

      for (UINT i = 0; i < model->meshes.size(); i++) {
        meshMaterialsShadowMap[model->meshes[i]] = materials[i];
      }
    }

    auto r = std::make_shared<RenderableT>(RenderableT{
      .name = renderableParams.name,
      .meshMaterials = meshMaterials,
      .meshMaterialsShadowMap = meshMaterialsShadowMap,
      .position = renderableParams.position,
      .scale = renderableParams.scale,
      .rotation = renderableParams.rotation,
      .skipMeshes = skipMeshes,
      .animations = animation,
    });
    r->this_ptr = r;
    r->Initialize(renderer).wait();
    renderables[r->name] = r;
    if (r->animations) animables[r->name] = r;
    if (loadFn) loadFn(r);
    return r;
  }

  std::map<std::wstring, std::shared_ptr<Renderable>>& GetRenderables()
  {
    return renderables;
  }

  std::map<std::wstring, std::shared_ptr<Renderable>>& GetAnimables() {
    return animables;
  }

  static void OnMaterialChangeStart(void* renderablePtr) {
    auto renderable = static_cast<Renderable*>(renderablePtr);
    renderable->loading = true;
  }

  static void OnMaterialChangeComplete(void* renderablePtr, void* materialPtr) {
    auto renderable = static_cast<Renderable*>(renderablePtr);
    std::shared_ptr<Renderer> renderer = Renderer::GetPtr();
    renderable->Initialize(renderer).wait();
    renderable->loading = false;
  }

  static void OnMaterialDestroy(void* renderablePtr, void* materialPtr) {
  }

  Concurrency::task<void> Renderable::Initialize(const std::shared_ptr<Renderer>& renderer)
  {
    meshConstantsBuffer.clear();
    meshRootSignatures.clear();
    meshPipelineStates.clear();

    std::vector<concurrency::task<void>> tasks;

    if (animations) {
      using namespace Animation;
      AttachAnimation(renderer, this_ptr, animations);
    }
    
    for (auto& [mesh, material] : this->meshMaterials) {
      tasks.push_back(concurrency::create_task([this, &renderer ,&mesh, &material]() {
        
        std::lock_guard<std::mutex> lock(rendererMutex);

        //initialize material textures
        BuildMaterialTextures(renderer, material);

        //create the constants buffer mapped to the mesh
        using namespace DeviceUtils::ConstantsBuffer;
        using namespace Scene::Lights;

        for (INT nCBuffers = 0U; nCBuffers < material->variablesBufferSize.size(); nCBuffers++) {
          ConstantsBufferViewDataPtr cbvData = CreateConstantsBufferViewData(renderer, material->variablesBufferSize[nCBuffers]);
          for (UINT n = 0; n < renderer->numFrames; n++) {
            WriteMaterialVariablesToConstantsBufferSpace(material, cbvData, n);
          }
          meshConstantsBuffer[mesh].push_back(cbvData);
        }

        //create the root signature for this material mapped to the mesh
        using namespace DeviceUtils::RootSignature;
        auto rootSignature = CreateRootSignature(renderer->d3dDevice, material);
        meshRootSignatures[mesh] = rootSignature;

        //create the pipeline state
        using namespace DeviceUtils::PipelineState;
        auto pipelineState = CreatePipelineState(renderer->d3dDevice, mesh->vertexClass, material, rootSignature);
        meshPipelineStates[mesh] = pipelineState;
        
      }).then([this,&mesh,&material]() {
        BindToMaterialTemplate(material->materialName, this,
          { .onLoadStart = OnMaterialChangeStart, .onLoadComplete = OnMaterialChangeComplete, .onDestroy = OnMaterialDestroy }
        );
        })
      );
    }

    return when_all(tasks.begin(), tasks.end()).then([this]() { this->loading = false; });
  }

  void Renderable::WriteConstantsBuffer(UINT backbufferIndex)
  {
    XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector(rotation);
    XMMATRIX scaleM = XMMatrixScalingFromVector(scale);
    XMMATRIX positionM = XMMatrixTranslationFromVector(position);
    XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);

    WriteConstantsBuffer(L"world", world, backbufferIndex);

    if (animations) {
      using namespace Animation;
      WriteBoneTransformationsToConstantsBuffer(this_ptr, bonesTransformation, backbufferIndex);
    }
  }

  auto PickRegister = [](auto& mat, auto pick) {
    auto v = pick(mat->shader->vertexShader);
    return v != -1 ? v : pick(mat->shader->pixelShader);
  };

  void Renderable::Render(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Scene::Camera::Camera>& camera)
  {
    if (loading) return;

    auto& commandList = renderer->commandList;

    PIXBeginEvent(commandList.Get(), 0, name.c_str());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (auto& [mesh, cbuffers] : meshConstantsBuffer) {

      if (skipMeshes.contains(mesh)) continue;

      using namespace DeviceUtils::RootSignature;
      MaterialPtr material = meshMaterials[mesh];
      if (material->loading) continue;
      auto rootSignature = meshRootSignatures[mesh];
      auto pipelineState = meshPipelineStates[mesh];

      using namespace DeviceUtils::ConstantsBuffer;
      using namespace Scene::Lights;
      using namespace Animation;

      commandList->SetGraphicsRootSignature(rootSignature.Get());
      commandList->SetPipelineState(pipelineState.Get());

      UINT cbvSlot = 0U;
      for (UINT i = 0U; i < cbuffers.size(); i++){
        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(GetGpuDescriptorHandle(cbuffers[i], renderer->backBufferIndex));
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, cbvGpuHandle);
        cbvSlot++;
      }

      INT CamRegister = PickRegister(material, [](auto output) { return output->cameraCBVRegister; });
      if (CamRegister != -1) {
        CD3DX12_GPU_DESCRIPTOR_HANDLE camCbvGpuHandle(GetGpuDescriptorHandle(camera->cameraCbv, renderer->backBufferIndex));
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, camCbvGpuHandle);
        cbvSlot++;
      }

      INT LightRegister = PickRegister(material, [](auto output) { return output->lightCBVRegister; });
      if (LightRegister != -1) {
        CD3DX12_GPU_DESCRIPTOR_HANDLE lightCbvGpuHandle(GetGpuDescriptorHandle(GetLightsConstantBufferView(), renderer->backBufferIndex));
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, lightCbvGpuHandle);
        cbvSlot++;
      }

      INT LightsShadowMapCBVRegister = PickRegister(material, [](auto output) { return output->lightsShadowMapCBVRegister; });
      if (LightsShadowMapCBVRegister != -1) {
        CD3DX12_GPU_DESCRIPTOR_HANDLE lightsShadowMapCbvGpuHandle(GetGpuDescriptorHandle(GetShadowMapConstantBufferView(), renderer->backBufferIndex));
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, lightsShadowMapCbvGpuHandle);
        cbvSlot++;
      }

      INT AnimationRegister = PickRegister(material, [](auto output) { return output->animationCBVRegister; });
      if (AnimationRegister != -1) {
        CD3DX12_GPU_DESCRIPTOR_HANDLE animationCbvGpuHandle(GetGpuDescriptorHandle(GetAnimatedConstantBufferView(this_ptr), renderer->backBufferIndex));
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, animationCbvGpuHandle);
        cbvSlot++;
      }

      for (auto& [texShaderName, texParam] : *material->shader->pixelShader->texturesParametersDef) {
        if (texParam.numTextures == 0xFFFFFFFF) continue;
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, material->textures[texParam.registerId].textureGpuHandle);
        cbvSlot++;
      }

      INT LightsShadowMapSRVRegister = PickRegister(material, [](auto output) { return output->lightsShadowMapSRVRegister; });
      if (LightsShadowMapSRVRegister !=-1) {
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, GetShadowMapGpuDescriptorHandleStart());
        cbvSlot++;
      }
      
      commandList->IASetVertexBuffers(0, 1, &mesh->vbvData.vertexBufferView);
      commandList->IASetIndexBuffer(&mesh->ibvData.indexBufferView);
      commandList->DrawIndexedInstanced(mesh->ibvData.indexBufferView.SizeInBytes / sizeof(UINT32), 1, 0, 0, 0);
    }
  
    PIXEndEvent(commandList.Get());
  }

  void Renderable::RenderShadowMap(const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<Scene::Lights::Light>& light, UINT cameraIndex) {
    if (loading) return;

    using namespace Scene::Lights;
    if (!meshMaterialsShadowMap.size()) return;

    auto& commandList = renderer->commandList;
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (auto& [mesh, material] : meshMaterialsShadowMap) {
      if (material->loading || skipMeshes.contains(mesh)) continue;

      INT CamRegister = PickRegister(material, [](auto output) { return output->cameraCBVRegister; });
      if (CamRegister == -1) continue; //don't draw things without a camera, bad shader(sorry)

      auto [rootSignature, pipelineState] =  GetShadowMapRenderAttributes(mesh->vertexClass, material);
      commandList->SetGraphicsRootSignature(rootSignature.Get());
      commandList->SetPipelineState(pipelineState.Get());

      auto cbuffers = meshConstantsBuffer[mesh];
      UINT cbvSlot = 0U;
      for (UINT i = 0U; i < cbuffers.size(); i++) {
        CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGpuHandle(GetGpuDescriptorHandle(cbuffers[i], renderer->backBufferIndex));
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, cbvGpuHandle);
        cbvSlot++;
      }

      CD3DX12_GPU_DESCRIPTOR_HANDLE camCbvGpuHandle(GetGpuDescriptorHandle(light->shadowMapCameras[cameraIndex]->cameraCbv, renderer->backBufferIndex));
      commandList->SetGraphicsRootDescriptorTable(cbvSlot, camCbvGpuHandle);
      cbvSlot++;

      INT AnimationRegister = PickRegister(material, [](auto output) { return output->animationCBVRegister; });
      if (AnimationRegister != -1) {
        using namespace Animation;
        CD3DX12_GPU_DESCRIPTOR_HANDLE animationCbvGpuHandle(GetGpuDescriptorHandle(GetAnimatedConstantBufferView(this_ptr), renderer->backBufferIndex));
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, animationCbvGpuHandle);
        cbvSlot++;
      }

      auto meshMaterial = meshMaterials[mesh];
      if (meshMaterial->textures.size() > 0) {
        commandList->SetGraphicsRootDescriptorTable(cbvSlot, meshMaterial->textures[0].textureGpuHandle);
      }
      
      commandList->IASetVertexBuffers(0, 1, &mesh->vbvData.vertexBufferView);
      commandList->IASetIndexBuffer(&mesh->ibvData.indexBufferView);
      commandList->DrawIndexedInstanced(mesh->ibvData.indexBufferView.SizeInBytes / sizeof(UINT32), 1, 0, 0, 0);
    }
  }

  void Renderable::SetCurrentAnimation(std::wstring animation)
  {

  }

  void Renderable::StepAnimation(double elapsedSeconds)
  {
    currentAnimationTime += static_cast<float>(elapsedSeconds) * 1000.0f;

    float animationLength = animations->animationsLength[currentAnimation];
    float animationTime = (animationLength != 0.0f) ? fmodf(currentAnimationTime, animationLength) : 0.0f;

    using namespace Animation;
    TraverseMultiplycationQueue(animationTime, animations->multiplyNavigator, animations->animationsBonesKeys[currentAnimation], bonesTransformation, animations->bonesOffsets, animations->rootNodeInverseTransform, XMMatrixIdentity());
  }

}