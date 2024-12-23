#include "pch.h"
#include <ppltasks.h>
#include <vector>
#include "Renderable.h"
#include "../../Common/d3dx12.h"
#include "../../Common/DirectXHelper.h"
#include "../Camera/CameraImpl.h"
#include "../Lights/LightsImpl.h"
#include "../../Templates/Model3D/Model3DImpl.h"
#include "../../Templates/Mesh/MeshImpl.h"
#include "../../Templates/Material/MaterialImpl.h"
#include "../../Animation/Animated.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Builder.h"
#include "../../Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "../../Renderer/DeviceUtils/RootSignature/RootSignature.h"
#include "../../Renderer/DeviceUtils/PipelineState/PipelineState.h"
#if defined(_EDITOR)
#include "../../Animation/Effects/Effects.h"
#endif

extern std::mutex rendererMutex;
extern std::shared_ptr<Renderer> renderer;
namespace Scene::Renderable {

  static void OnMaterialChangeStart(void* renderablePtr) {
    auto renderable = static_cast<Renderable*>(renderablePtr);
    renderable->loading = true;
  }

  static void OnMaterialChangeComplete(void* renderablePtr, void* materialPtr) {
    auto renderable = static_cast<Renderable*>(renderablePtr);
    std::shared_ptr<Renderer> renderer = Renderer::GetPtr();
    renderable->Initialize(renderer);
    renderable->loading = false;
  }

  static void OnMaterialDestroy(void* renderablePtr, void* materialPtr) { }

  std::map<std::wstring, std::shared_ptr<Renderable>> renderables;
  std::map<std::wstring, std::shared_ptr<Renderable>> animables;

  void Renderable::Initialize(const std::shared_ptr<Renderer>& renderer)
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
      tasks.push_back(concurrency::create_task([this, &renderer, &mesh, &material]() {

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

        BindToMaterialTemplate(material->materialName, this, {
          .onLoadStart = OnMaterialChangeStart,
          .onLoadComplete = OnMaterialChangeComplete,
          .onDestroy = OnMaterialDestroy
          });
      }));
    }

    when_all(tasks.begin(), tasks.end()).then([this]() { this->loading = false; }).wait();
  }

  void Renderable::WriteConstantsBuffer(UINT backbufferIndex)
  {
    XMMATRIX rotationM = XMMatrixRotationRollPitchYawFromVector({ rotation.x, rotation.y, rotation.z, 0.0f });
    XMMATRIX scaleM = XMMatrixScalingFromVector({ scale.x, scale.y, scale.z });
    XMMATRIX positionM = XMMatrixTranslationFromVector({ position.x, position.y, position.z });
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

    PIXBeginEvent(commandList.p, 0, name.c_str());

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

      commandList->SetGraphicsRootSignature(rootSignature);
      commandList->SetPipelineState(pipelineState);

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
  
    PIXEndEvent(commandList.p);
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
      commandList->SetGraphicsRootSignature(rootSignature);
      commandList->SetPipelineState(pipelineState);

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

  void Renderable::SetCurrentAnimation(std::wstring animation, float animationTime)
  {
    currentAnimation = animation;
    currentAnimationTime = animationTime;
  }

  void Renderable::StepAnimation(double elapsedSeconds)
  {
    currentAnimationTime += static_cast<float>(elapsedSeconds) * 1000.0f;

    float animationLength = animations->animationsLength[currentAnimation];
    float animationTime = (animationLength != 0.0f) ? fmodf(currentAnimationTime, animationLength) : 0.0f;

    using namespace Animation;
    TraverseMultiplycationQueue(animationTime, animations->multiplyNavigator, animations->animationsBonesKeys[currentAnimation], bonesTransformation, animations->bonesOffsets, animations->rootNodeInverseTransform, XMMatrixIdentity());
  }

#if defined(_EDITOR)
  nlohmann::json Renderable::json() {
    nlohmann::json j = nlohmann::json({});

    j["name"] = WStringToString(name);
    j["position"] = { position.x, position.y, position.z };
    j["scale"] = { scale.x, scale.y, scale.z };
    j["rotation"] = { rotation.x, rotation.y, rotation.z };

    auto buildJsonMeshMaterials = [](auto& j, std::string objectName, MeshMaterialMap& meshMatMap){
      j[objectName] = nlohmann::json::array();
      for (auto& [mesh, material] : meshMatMap) {
        j[objectName].push_back({ {"mesh", WStringToString(mesh->name)}, {"material", WStringToString(material->materialName) } });
      }
    };

    if (model3DName == L"") {
      buildJsonMeshMaterials(j, "meshMaterials", meshMaterials);
      buildJsonMeshMaterials(j, "meshMaterialsShadowMap", meshMaterialsShadowMap);
    } else {
      j["model"] = WStringToString(model3DName);
    }

    j["skipMeshes"] = nlohmann::json::array();
    for (auto meshIdx : definition.skipMeshes) {
      j["skipMeshes"].push_back(meshIdx);
    }

    using namespace Animation::Effects;
    auto effects = GetRenderableEffects(this_ptr);
    if (!effects.empty()) {
      j["effects"] = effects;
    }

    return j;
  }
#endif

  MeshMaterialMap TranformJsonToMeshMaterialMap(nlohmann::json j) {
    MeshMaterialMap map;
    std::transform(j.begin(), j.end(), std::inserter(map, map.end()), [](const nlohmann::json& value) {
      return MeshMaterialPair(GetMeshTemplate(StringToWString(value["mesh"])), GetMaterialTemplate(StringToWString(value["material"])));
    });
    return map;
  }

  std::shared_ptr<Renderable> CreateRenderable(nlohmann::json renderablej) {

    assert(renderablej["meshMaterials"].empty() xor renderablej["model"].empty());
    assert(renderablej["name"] != "");

    using namespace Animation;

    std::set<MeshPtr> skipMeshes;
    std::shared_ptr<Animated> animation = nullptr;

    RenderableDefinition renderableParams = {
      .name = StringToWString(renderablej["name"]),
      .position = JsonToFloat3(renderablej["position"]),
      .scale = JsonToFloat3(renderablej["scale"]),
      .rotation = JsonToFloat3(renderablej["rotation"]),
      .skipMeshes = TransformJsonArrayToSet(renderablej["skipMeshes"])
    };

    std::wstring model3DName = L"";
    if (!renderablej["meshMaterials"].empty()) {
      renderableParams.meshMaterials = TranformJsonToMeshMaterialMap(renderablej["meshMaterials"]);
      renderableParams.meshMaterialsShadowMap = TranformJsonToMeshMaterialMap(renderablej["meshMaterialsShadowMap"]);
    }
    else {
      using namespace Templates::Model3D;
      model3DName = StringToWString(renderablej["model"]);
      Model3DPtr model = GetModel3DTemplate(model3DName);
      animation = model->animations;
      auto shadowMapMaterial = model->animations ? GetMaterialTemplate(L"ShadowMapAlphaSkinning") : GetMaterialTemplate(L"ShadowMapAlpha");
      for (UINT meshIndex = 0; meshIndex < model->meshes.size(); meshIndex++) {
        auto mesh = model->meshes[meshIndex];
        renderableParams.meshMaterials.insert_or_assign(mesh, GetMaterialTemplate(getMaterialName(model3DName, meshIndex)));
        renderableParams.meshMaterialsShadowMap.insert_or_assign(mesh, shadowMapMaterial);
      }
    }

    auto meshMaterials = renderableParams.meshMaterials;
    auto meshMaterialsShadowMap = renderableParams.meshMaterialsShadowMap;

    auto r = std::make_shared<RenderableT>(RenderableT{
      .definition = renderableParams,
      .name = renderableParams.name,
      .model3DName = model3DName,
      .meshMaterials = meshMaterials,
      .meshMaterialsShadowMap = meshMaterialsShadowMap,
      .position = renderableParams.position,
      .scale = renderableParams.scale,
      .rotation = renderableParams.rotation,
      .skipMeshes = skipMeshes,
      .animations = animation,
      });
    r->this_ptr = r;
    r->Initialize(renderer);// .wait();
    renderables[r->name] = r;
    if (r->animations) animables[r->name] = r;
    return r;
  }

  std::map<std::wstring, std::shared_ptr<Renderable>>& GetRenderables() { return renderables; }

  std::map<std::wstring, std::shared_ptr<Renderable>>& GetAnimables() { return animables; }

  void DestroyRenderables()
  {
    for (auto& [name, renderable] : renderables) {

      if (renderable->animations) {
        renderable->animations.reset();
        renderable->animations = nullptr;
      }

      for (auto& [mesh, pipelinestate] : renderable->meshPipelineStates) {
        pipelinestate.Release();
        pipelinestate = nullptr;
      }
      for (auto& [mesh, rootsignature] : renderable->meshRootSignatures) {
        rootsignature.Release();
        rootsignature = nullptr;
      }

      for (auto& [mesh, cbvvec] : renderable->meshConstantsBuffer) {
        for (auto& cbv : cbvvec) {
          cbv->constantBuffer.Release();
          cbv->constantBuffer = nullptr;
          cbv.reset();
          cbv = nullptr;
        }
      }

      renderable->this_ptr = nullptr;
      renderable.reset();
      renderable = nullptr;
    }
    renderables.clear();

    for (auto& [name, renderable] : animables) {
      renderable.reset();
      renderable = nullptr;
    }
    animables.clear();
  }
  
}