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
#include "../Level.h"
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

  std::map<std::string, std::shared_ptr<Renderable>> renderables;
  std::map<std::string, std::shared_ptr<Renderable>> animables;

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

        BindToMaterialTemplate(material->name, this, {
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

    WriteConstantsBuffer("world", world, backbufferIndex);

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

  void Renderable::SetCurrentAnimation(std::string animation, float animationTime, float timeFactor, bool autoPlay)
  {
    currentAnimation = animation;
    currentAnimationTime = animationTime;
    animationTimeFactor = timeFactor;
    playingAnimation = autoPlay;
  }

  void Renderable::StepAnimation(double elapsedSeconds)
  {
    currentAnimationTime += playingAnimation ? animationTimeFactor * static_cast<float>(elapsedSeconds) * 1000.0f : 0.0f;
    float animationLength = animations->animationsLength[currentAnimation];

    if (animationTimeFactor > 0.0f) {
      if (currentAnimationTime >= animationLength) {
        currentAnimationTime = fmodf(currentAnimationTime, animationLength);
      }
    } else if (animationTimeFactor < 0.0f) {
      if (currentAnimationTime < 0.0f) {
        currentAnimationTime = animationLength - fmodf(currentAnimationTime, animationLength);
      }
    }

    using namespace Animation;
    TraverseMultiplycationQueue(currentAnimationTime, animations->multiplyNavigator, animations->animationsBonesKeys[currentAnimation], bonesTransformation, animations->bonesOffsets, animations->rootNodeInverseTransform, XMMatrixIdentity());
  }

#if defined(_EDITOR)
  nlohmann::json Renderable::json() {
    nlohmann::json j = nlohmann::json({});

    j["name"] = name;
    j["position"] = { position.x, position.y, position.z };
    j["scale"] = { scale.x, scale.y, scale.z };
    j["rotation"] = { rotation.x, rotation.y, rotation.z };

    auto buildJsonMeshMaterials = [](auto& j, std::string objectName, MeshMaterialMap& meshMatMap){
      j[objectName] = nlohmann::json::array();
      for (auto& [mesh, material] : meshMatMap) {
        j[objectName].push_back({ {"mesh", mesh->name }, {"material", material->name } });
      }
    };

    if (model3DName == "") {
      buildJsonMeshMaterials(j, "meshMaterials", meshMaterials);
      buildJsonMeshMaterials(j, "meshMaterialsShadowMap", meshMaterialsShadowMap);
    } else {
      j["model"] = model3DName;
    }

    j["skipMeshes"] = nlohmann::json::array();
    for (MeshPtr mesh : skipMeshes) {
      j["skipMeshes"].push_back(mesh->name);
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
      return MeshMaterialPair(GetMeshTemplate(value["mesh"]), GetMaterialTemplate(value["material"]));
    });
    return map;
  }

  std::shared_ptr<Renderable> CreateRenderable(nlohmann::json renderablej) {

    assert(renderablej["meshMaterials"].empty() xor renderablej["model"].empty());
    assert(renderablej["name"] != "");

    using namespace Animation;

    std::set<MeshPtr> skipMeshes;
    std::shared_ptr<Animated> animation = nullptr;

    std::string name = renderablej["name"];
    XMFLOAT3 position = JsonToFloat3(renderablej["position"]);
    XMFLOAT3 scale = JsonToFloat3(renderablej["scale"]);
    XMFLOAT3 rotation = JsonToFloat3(renderablej["rotation"]);
    std::set<std::string> meshesToSkip = TransformJsonArrayToSet<std::string>(renderablej["skipMeshes"]);

    MeshMaterialMap meshMaterials;
    MeshMaterialMap meshMaterialsShadowMap;
    std::vector<MeshPtr> meshesVector;

    std::string model3DName = "";
    if (!renderablej["meshMaterials"].empty()) {
      meshMaterials = TranformJsonToMeshMaterialMap(renderablej["meshMaterials"]);
      meshMaterialsShadowMap = TranformJsonToMeshMaterialMap(renderablej["meshMaterialsShadowMap"]);
    } else {
      using namespace Templates::Model3D;
      model3DName = renderablej["model"];
      Model3DPtr model = GetModel3DTemplate(model3DName);
      animation = model->animations;
      MaterialPtr shadowMapMaterial = model->animations ? GetMaterialTemplate("ShadowMapAlphaSkinning") : GetMaterialTemplate("ShadowMapAlpha");
      for (UINT meshIndex = 0; meshIndex < model->meshes.size(); meshIndex++) {
        MeshPtr mesh = model->meshes[meshIndex];
#if defined(_EDITOR)
        meshesVector.push_back(mesh);
#endif
        meshMaterials.insert_or_assign(mesh, GetMaterialTemplate(BuildMaterialName(model3DName, meshIndex)));
        meshMaterialsShadowMap.insert_or_assign(mesh, shadowMapMaterial);
        if (meshesToSkip.contains(mesh->name)) {
          skipMeshes.insert(mesh);
        }
      }
    }

    auto r = std::make_shared<RenderableT>(RenderableT{
      .name = name,
      .model3DName = model3DName,
      .meshMaterials = meshMaterials,
      .meshMaterialsShadowMap = meshMaterialsShadowMap,
      .position = position,
      .scale = scale,
      .rotation = rotation,
      .skipMeshes = skipMeshes,
#if defined(_EDITOR)
      .meshesVector = meshesVector,
#endif
      .animations = animation,
    });
    r->this_ptr = r;
    r->Initialize(renderer);// .wait();
    renderables[r->name] = r;
    if (r->animations) animables[r->name] = r;
    return r;
  }

  std::map<std::string, std::shared_ptr<Renderable>>& GetRenderables() { return renderables; }

  std::map<std::string, std::shared_ptr<Renderable>>& GetAnimables() { return animables; }

  std::vector<std::string> GetRenderablesNames() {
    std::vector<std::string> names;
    std::transform(renderables.begin(), renderables.end(), std::back_inserter(names), [](const std::pair<std::string, std::shared_ptr<Renderable>> pair) { return pair.second->name; });
    return names;
  }

#if defined(_EDITOR)
  void SelectRenderable(std::string renderableName, void*& ptr) {
    ptr = renderables.at(renderableName).get();
  }

  int resizeCb(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
      std::string* str = (std::string*)data->UserData;
      //IM_ASSERT(str->begin() == data->Buf);
      str->resize(data->BufSize); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
      data->Buf = &*str->begin();
    }
    return 0;
  }

  void Renderable::DrawEditorInformationAttributes() {

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    std::string tableName = "renderable-information-atts";
    if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
    {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      std::string currentName = name;
      if (ImGui::InputText("name", &currentName)) {

        if (!renderables.contains(currentName)) {
          renderables[currentName] = renderables[name];
          renderables.erase(name);
        }

        if (!animables.contains(currentName) && animables.contains(name)) {
          animables[currentName] = animables[name];
          animables.erase(name);
        }

        name = currentName;
      }
      ImGui::EndTable();
    }
    
  }

  void Renderable::DrawEditorWorldAttributes() {

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    std::string tableName = "renderable-world-atts";
    if (ImGui::BeginTable(tableName.c_str(), 4, ImGuiTableFlags_NoSavedSettings))
    {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::PushID("position");
      ImGui::Text("position");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputFloat("x", &position.x);
      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat("y", &position.y);
      ImGui::TableSetColumnIndex(3);
      ImGui::InputFloat("z", &position.z);
      ImGui::PopID();

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::PushID("rotation");
      ImGui::Text("rotation");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputFloat("roll", &rotation.x);
      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat("pitch", &rotation.y);
      ImGui::TableSetColumnIndex(3);
      ImGui::InputFloat("yaw", &rotation.z);
      ImGui::PopID();

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::PushID("scale");
      ImGui::Text("scale");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputFloat("x", &scale.x);
      ImGui::TableSetColumnIndex(2);
      ImGui::InputFloat("y", &scale.y);
      ImGui::TableSetColumnIndex(3);
      ImGui::InputFloat("z", &scale.z);
      ImGui::PopID();

      ImGui::EndTable();
    }
  }



  void Renderable::DrawEditorAnimationAttributes() {
    if (!animations) return;

    ImGui::Text("animations");
    std::vector<std::string> selectables = { " " };
    for (auto& [name, length] : animations->animationsLength) {
      if (name == "") continue;
      selectables.push_back(name);
    }

    int current_item = 0;
    if (currentAnimation != "") {
      current_item = static_cast<int>(std::find(selectables.begin(), selectables.end(), currentAnimation) - selectables.begin());
    }

    DrawComboSelection(selectables[current_item], selectables, [this](std::string animName) {
      SetCurrentAnimation(animName != " " ? animName : "");
    });

    if (ImGui::Button(ICON_FA_BACKWARD)) {
      int animIdx = current_item - 1;
      if (animIdx == -1) animIdx = static_cast<int>(selectables.size()) - 1;
      std::string animName = selectables[animIdx];
      SetCurrentAnimation(animName != " " ? animName : "");
    }
    ImGui::SameLine();

    if (playingAnimation) {
      if (ImGui::Button(ICON_FA_PAUSE)) {
        playingAnimation = false;
      }
    } else {
      if (ImGui::Button(ICON_FA_PLAY)) {
        playingAnimation = true;
      }
    }
    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_STOP)) {
      playingAnimation = false;
      currentAnimationTime = 0.0f;
    }
    ImGui::SameLine();
    if (animationTimeFactor > 0.0f) {
      if (ImGui::Button(ICON_FA_UNDO)) {
        animationTimeFactor = -animationTimeFactor;
      }
    } else if (animationTimeFactor < 0.0f) {
      if (ImGui::Button(ICON_FA_REDO)) {
        animationTimeFactor = -animationTimeFactor;
      }
    } else {
      if (ImGui::Button(ICON_FA_SYNC)) {
        animationTimeFactor = 1.0f;
      }
    }
    ImGui::SameLine();
    
    if (ImGui::Button(ICON_FA_FORWARD)) {
      int animIdx = current_item + 1;
      if (animIdx == selectables.size()) animIdx = 0;
      std::string animName = selectables[animIdx];
      SetCurrentAnimation(animName != " " ? animName : "");
    }
    ImGui::NewLine();
  }

  void Renderable::DrawEditorMeshesAttributes() {
    
    std::vector<std::string> modelsNames = GetModels3DNames();
    std::vector<std::string> meshesNames = GetMeshesNames();

    std::vector<std::string> selectables = { " " };
    selectables.insert(selectables.end(), modelsNames.begin(), modelsNames.end());
    selectables.insert(selectables.end(), meshesNames.begin(), meshesNames.end());

    int current_item = 0;
    int current_model = static_cast<int>(std::find(modelsNames.begin(), modelsNames.end(), model3DName) - modelsNames.begin());

    if (current_model != modelsNames.size()) {
      current_item = 1 + current_model;
    } else if (meshMaterials.size() > 0) {
      MeshPtr mesh = meshMaterials.begin()->first;
      int current_mesh = static_cast<int>(std::find(meshesNames.begin(), meshesNames.end(), mesh->name) - meshesNames.begin());
      if (current_mesh != meshesNames.size()) {
        current_item = 1 + static_cast<int>(modelsNames.size()) + current_mesh;
      }
    }

    if (ImGui::BeginCombo("model", selectables[current_item].c_str())) {

      for (int i = 0; i < selectables.size(); i++) {
        ImGui::Selectable(selectables[i].c_str(), current_item == i);
      }

      ImGui::EndCombo();
    }

    std::string tableName = "renderable-meshes-atts";
    if (ImGui::BeginTable(tableName.c_str(), 4, ImGuiTableFlags_NoSavedSettings))
    {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TableHeader("skip");
      ImGui::TableSetColumnIndex(1);
      ImGui::TableHeader("mesh");
      ImGui::TableSetColumnIndex(2);
      ImGui::TableHeader("material");
      ImGui::TableSetColumnIndex(3);
      ImGui::TableHeader("shadowmap material");

      for (auto& mesh : meshesVector) {
        ImGui::TableNextRow();

        bool skip_v = skipMeshes.contains(mesh);
        ImGui::TableSetColumnIndex(0);
        std::string comboIdSkip = "mesh#" + mesh->name + "#skip";
        ImGui::PushID(comboIdSkip.c_str());
        if (ImGui::Checkbox("", &skip_v)) {
          if (skip_v) {
            skipMeshes.insert(mesh);
          } else {
            skipMeshes.erase(mesh);
          }
        }
        ImGui::PopID();

        ImGui::TableSetColumnIndex(1);
        ImGui::Text(mesh->name.c_str());

        ImGui::TableSetColumnIndex(2);
        std::string comboId = "mesh#" + mesh->name + "#material";
        ImGui::PushID(comboId.c_str());
        auto material = meshMaterials[mesh];
        DrawComboSelection(material->name, GetMaterialsNamesMatchingClass(mesh->vertexClass), [this,mesh](std::string matName) {
          loading = true;
          MaterialPtr newMat = GetMaterialTemplate(matName);
          meshMaterials[mesh] = newMat;
          std::thread pushReload([this]() {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(200ms);
            Scene::Level::PushRenderableToReloadQueue(this_ptr);
          });
          pushReload.detach();
        });
        ImGui::PopID();

        ImGui::TableSetColumnIndex(3);
        std::string comboIdSM = "mesh#" + mesh->name + "#materialShadowMap";
        ImGui::PushID(comboIdSM.c_str());
        auto materialShadowMap = meshMaterialsShadowMap[mesh];
        DrawComboSelection(materialShadowMap->name, GetShadowMapMaterialsNamesMatchingClass(mesh->vertexClass), [this,mesh](std::string matName) {
          loading = true;
          MaterialPtr newMat = GetMaterialTemplate(matName);
          meshMaterialsShadowMap[mesh] = newMat;
          std::thread pushReload([this]() {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(200ms);
            Scene::Level::PushRenderableToReloadQueue(this_ptr);
          });
          pushReload.detach();
        });
        ImGui::PopID();
      }
      
      ImGui::EndTable();
    }
  }

  void DrawRenderablePanel(void*& ptr, ImVec2 pos, ImVec2 size)
  {
    Renderable* renderable = (Renderable*)ptr;

    std::string tableName = "renderable-panel";
    if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
    {
      renderable->DrawEditorInformationAttributes();
      renderable->DrawEditorWorldAttributes();
      renderable->DrawEditorAnimationAttributes();
      renderable->DrawEditorMeshesAttributes();
      ImGui::EndTable();
    }
  }

  std::string GetRenderableName(void* ptr)
  {
    Renderable* renderable = (Renderable*)ptr;
    return renderable->name;
  }
#endif

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