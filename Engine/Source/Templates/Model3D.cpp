#include "pch.h"
#include "Model3D.h"
#include "Mesh.h"
#include "Material.h"
#include "../Renderer/VertexFormats.h"
#include "../Animation/Animated.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>

extern std::mutex rendererMutex;
extern std::shared_ptr<Renderer> renderer;

namespace Templates::Model3D {

  static std::map<std::wstring, Model3DPtr> model3DTemplates;

  template<typename T>
  void CopyVertexPos(T& vertexData, aiVector3D& pos) {
    vertexData.Position.x = pos[0]; vertexData.Position.y = pos[1]; vertexData.Position.z = pos[2];
  }

  template<typename T>
  void CopyVertexNormal(T& vertexData, aiVector3D& normal) {
    vertexData.Normal.x = normal[0]; vertexData.Normal.y = normal[1]; vertexData.Normal.z = normal[2];
  }

  template<typename T>
  void CopyVertexTangent(T& vertexData, aiVector3D& tangent) {
    vertexData.Tangent.x = tangent[0]; vertexData.Tangent.y = tangent[1]; vertexData.Tangent.z = tangent[2];
  }

  template<typename T>
  void CopyVertexBiTangent(T& vertexData, aiVector3D& bitangent) {
    vertexData.BiTangent.x = bitangent[0]; vertexData.BiTangent.y = bitangent[1]; vertexData.BiTangent.z = bitangent[2];
  }

  template<typename T>
  void CopyVertexTexCoord0(T& vertexData, aiVector3D& texcoord) {
    vertexData.TexCoord.x = texcoord[0]; vertexData.TexCoord.y = texcoord[1];
  }

  template<VertexClass T>
  std::shared_ptr<byte*> LoadVertices(aiMesh* mesh) { return nullptr; }

  template<>
  std::shared_ptr<byte*> LoadVertices<POS_NORMAL_TANGENT_TEXCOORD0>(aiMesh* mesh) {

    const size_t size = sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0>) * mesh->mNumVertices;
    std::shared_ptr<byte*> vertices = std::make_shared<byte*>(new byte[size]);

    std::shared_ptr<Vertex<POS_NORMAL_TANGENT_TEXCOORD0>*> vertexData = reinterpret_cast<const std::shared_ptr<Vertex<POS_NORMAL_TANGENT_TEXCOORD0>*>&>(vertices);
    
    //copy every vertex in the mesh
    for (UINT vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
      
      CopyVertexPos((*vertexData)[vertexIndex], mesh->mVertices[vertexIndex]);
      CopyVertexNormal((*vertexData)[vertexIndex], mesh->mNormals[vertexIndex]);
      CopyVertexTangent((*vertexData)[vertexIndex], mesh->mTangents[vertexIndex]);
      //CopyVertexBiTangent((*vertexData)[vertexIndex], mesh->mBitangents[vertexIndex]);
      CopyVertexTexCoord0((*vertexData)[vertexIndex], mesh->mTextureCoords[0][vertexIndex]);

    }

    return vertices;
  }

  template<>
  std::shared_ptr<byte*> LoadVertices<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>(aiMesh* mesh) {
    const size_t size = sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>) * mesh->mNumVertices;
    std::shared_ptr<byte*> vertices = std::make_shared<byte*>(new byte[size]);

    std::shared_ptr<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*> vertexData = reinterpret_cast<const std::shared_ptr<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*>&>(vertices);

    //copy every vertex in the mesh
    for (UINT vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {

      CopyVertexPos((*vertexData)[vertexIndex], mesh->mVertices[vertexIndex]);
      CopyVertexNormal((*vertexData)[vertexIndex], mesh->mNormals[vertexIndex]);
      CopyVertexTangent((*vertexData)[vertexIndex], mesh->mTangents[vertexIndex]);
      //CopyVertexBiTangent((*vertexData)[vertexIndex], mesh->mBitangents[vertexIndex]);
      CopyVertexTexCoord0((*vertexData)[vertexIndex], mesh->mTextureCoords[0][vertexIndex]);
      
      //put some values to the bone weight and ids(will be override after)
      (*vertexData)[vertexIndex].BoneIds = { 0, 0, 0, 0 };
      (*vertexData)[vertexIndex].BoneWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
    }

    return vertices;
  }

  std::map<VertexClass, std::shared_ptr<byte*>(*)(aiMesh*)> VerticesLoader = {
    {POS_NORMAL_TANGENT_TEXCOORD0, LoadVertices<POS_NORMAL_TANGENT_TEXCOORD0> },
    {POS_NORMAL_TANGENT_TEXCOORD0_SKINNING, LoadVertices<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING> },
  };

  std::vector<UINT32> LoadIndices(aiMesh* mesh) {
    UINT totalIndices = mesh->mNumFaces * mesh->mFaces[0].mNumIndices;
    std::vector<UINT32> indicesData;
    //UINT faceIndex = 0;
    for (UINT meshFaceIndex = 0; meshFaceIndex < mesh->mNumFaces; meshFaceIndex++) {
      for (UINT index = 0; index < mesh->mFaces[meshFaceIndex].mNumIndices; index++) {
        indicesData.push_back(static_cast<UINT32>(mesh->mFaces[meshFaceIndex].mIndices[index]));
      }
    }
    return indicesData;
  }

  void LoadBonesInVertices(aiMesh* mesh, Animation::BonesTransformations& bones, std::shared_ptr<byte*>& vertices) {

    std::shared_ptr<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*> vertexData = reinterpret_cast<const std::shared_ptr<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*>&>(vertices);

    std::vector<UINT> numBonesPerVertex(mesh->mNumVertices, 0U);
    for (UINT meshBoneIndex = 0; meshBoneIndex < mesh->mNumBones; meshBoneIndex++) {

      auto bone = mesh->mBones[meshBoneIndex];
      std::string boneName = bone->mName.C_Str();
      std::wstring boneNameW(boneName.begin(), boneName.end());
      UINT boneId = static_cast<UINT>(std::distance(bones.begin(), bones.find(boneNameW)));

      for (UINT weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++) {
        auto weight = bone->mWeights[weightIndex];

        //skip bones with 0 weight
        if (weight.mWeight == 0.0f)
          continue;

        UINT offset = numBonesPerVertex[weight.mVertexId];
        if (offset >= 4) continue;

        //use offset to put the value in the propper slot (x:0, y:1, z:2, w:3)
        uint32_t* boneIdSlot = &(*vertexData)[weight.mVertexId].BoneIds.x + offset;
        float* weightSlot = &(*vertexData)[weight.mVertexId].BoneWeights.x + offset;

        *boneIdSlot = boneId;
        *weightSlot = weight.mWeight;
        numBonesPerVertex[weight.mVertexId]++;
      }
    }
  }

  void PushToMaterialDefinitionTextures(aiString& textureName, std::filesystem::path& relativePath, Templates::Material::MaterialDefinition& matDef, std::wstring fallbackTexture = L"", DXGI_FORMAT textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
  {
    if (textureName.length > 0) {
      std::string textureNameA = textureName.C_Str();
      std::wstring textureNameW(textureNameA.begin(), textureNameA.end());
      std::filesystem::path texturePath = relativePath.parent_path().append(textureNameW).replace_extension(".dds");
      matDef.textures.push_back({ .texturePath = texturePath.c_str(), .textureFormat = textureFormat });
    }
    else if(fallbackTexture != L"")
    {
      matDef.textures.push_back({ .texturePath = fallbackTexture, .textureFormat = textureFormat });
    }
  }

  void CreateModel3DMaterial(std::wstring materialName, std::wstring materialShader, std::filesystem::path relativePath, aiMaterial* material){

    using namespace Templates::Material;

    MaterialDefinition matDef = {
      .shaderTemplate = materialShader
    };

    material->Get(AI_MATKEY_TWOSIDED, matDef.twoSided);

    ai_real shininess;
    material->Get(AI_MATKEY_SHININESS, shininess);
    matDef.mappedValues[L"specularExponent"] ={ MaterialVariablesTypes::MAT_VAR_FLOAT, shininess };

    ai_real metallic;
    material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    matDef.mappedValues[L"metallicFactor"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, metallic };

    ai_real roughness;
    material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
    matDef.mappedValues[L"roughnessFactor"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, roughness };

    aiString alphaMode;
    material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    if (strcmp(alphaMode.C_Str(), "OPAQUE")==0) {
      matDef.mappedValues[L"alphaCut"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, 1.0f };
    }
    else
    {
      ai_real alphaCut;
      material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCut);
      matDef.mappedValues[L"alphaCut"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, alphaCut };
    }

    aiString diffuseName;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseName);
    PushToMaterialDefinitionTextures(diffuseName, relativePath, matDef, L"Assets/textures/gridmap.dds");

    aiString normalMapName;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), normalMapName);
    PushToMaterialDefinitionTextures(normalMapName, relativePath, matDef, L"Assets/textures/bumpmapflat.dds", DXGI_FORMAT_R8G8B8A8_UNORM);

    aiString metallicRoughnessName;
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessName);
    PushToMaterialDefinitionTextures(metallicRoughnessName, relativePath, matDef, L"", DXGI_FORMAT_R8G8B8A8_UNORM);

    CreateMaterialTemplate(materialName, matDef).wait();

  }

  std::wstring getMeshName(std::wstring model3DName, UINT meshIndex)
  {
    return L"mesh." + model3DName + L"." + std::to_wstring(meshIndex);
  }

  std::wstring getMaterialName(std::wstring model3DName, UINT meshIndex)
  {
    return L"mat." + model3DName + L"." + std::to_wstring(meshIndex);
  }

  Concurrency::task<void> CreateModel3DTemplate(std::wstring model3DName, std::wstring assetPath, std::shared_ptr<Renderer>& renderer, Model3DDefinition params)
  {
    const std::wstring filename = assetsRootFolder + assetPath;

    return concurrency::create_task([filename, assetPath, model3DName, &renderer,params] {

      std::lock_guard<std::mutex> lock(rendererMutex);

      Model3DPtr model = std::make_shared<Model3D>();
      model->loading = true;
      model->assetPath = assetPath;
      model->model3dDefinition = params;
      model3DTemplates.insert(std::pair<std::wstring, Model3DPtr>(model3DName, model));
      
      std::filesystem::path path(filename);

      Assimp::Importer importer;
      const aiScene* aiModel = importer.ReadFile(path.string(), 
        aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | 
        aiProcess_CalcTangentSpace | aiProcess_Triangulate | 
        aiProcess_FlipUVs
      );

      if (!aiModel) {
        OutputDebugStringA(importer.GetErrorString());
        assert(!aiModel);
      }
    
      //fill the length of the animations so they can be looped
      if (aiModel->mNumAnimations > 0U) {
        using namespace Animation;
        model->animations = CreateAnimatedFromAssimp(aiModel);
      }

      model->vertexClass = !model->animations ? POS_NORMAL_TANGENT_TEXCOORD0 : POS_NORMAL_TANGENT_TEXCOORD0_SKINNING;
      size_t vertexSize = !model->animations ? sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0>) : sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>);

      //go through all the meshes in the model
      for (UINT meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++) {

        auto aMesh = aiModel->mMeshes[meshIndex];

        model->vertices.push_back(VerticesLoader[model->vertexClass](aMesh));
        model->numVertices.push_back(aMesh->mNumVertices);
        model->indices.push_back(LoadIndices(aMesh));

        if (model->animations) {
          LoadBonesInVertices(aMesh, model->animations->bonesOffsets, model->vertices.back());
        }

        using namespace Templates::Mesh;
        MeshPtr mesh = std::make_shared<MeshT>();
        std::wstring meshName = getMeshName(model3DName,meshIndex);
        mesh->loading = true;
        mesh->name = meshName;
        mesh->vertexClass = model->vertexClass;
      
        InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, *model->vertices.back(), static_cast<UINT>(vertexSize), model->numVertices.back(), mesh->vbvData);
        InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, model->indices.back().data(), static_cast<UINT>(model->indices.back().size()), mesh->ibvData);

        if (params.autoCreateMaterial) {
          std::wstring materialName = getMaterialName(model3DName,meshIndex);
          using namespace Templates::Material;
          if (GetMaterialTemplate(materialName) == nullptr) {
            CreateModel3DMaterial(materialName, params.materialsShader, path.relative_path(), aiModel->mMaterials[aMesh->mMaterialIndex]);
          }
        }

        model->meshes.push_back(mesh);
        mesh->loading = false;
      }

      importer.FreeScene();
      return model;

     }).then([](Model3DPtr model) {
      model->loading = false;
    });

  }

  void ReleaseModel3DTemplates()
  {
    for (auto& [name, model] : model3DTemplates) {

      for (auto& vert : model->vertices) {
        delete* vert;
      }
      model->indices.clear();

      for (auto& mesh : model->meshes) {
        mesh->vbvData.vertexBuffer = nullptr;
        mesh->vbvData.vertexBufferUpload = nullptr;
        mesh->ibvData.indexBuffer = nullptr;
        mesh->ibvData.indexBufferUpload = nullptr;
      }
      model->meshes.clear();

      if (model->animations) {
        DestroyNodesHierarchy(&model->animations->rootHierarchy);
      }
    }

    model3DTemplates.clear();
  }

  std::shared_ptr<Model3D> GetModel3DTemplate(std::wstring model3DName) {
    auto it = model3DTemplates.find(model3DName);
    return (it != model3DTemplates.end()) ? it->second : nullptr;
  }

#if defined(_EDITOR)
  nlohmann::json json()
  {
    nlohmann::json models = nlohmann::json({});

    for (auto& [name, model] : model3DTemplates) {

      models[WStringToString(name)] = {
        { "autoCreateMaterial", model->model3dDefinition.autoCreateMaterial },
        { "materialsShader", WStringToString(model->model3dDefinition.materialsShader) },
        { "assetPath", WStringToString(model->assetPath) }
      };
    }

    return models;
  }
#endif

  Concurrency::task<void> json(std::wstring name, nlohmann::json model3dj)
  {
    const std::wstring assetPath = StringToWString(model3dj["assetPath"]);
    const std::wstring filename = assetsRootFolder + assetPath;

    Model3DDefinition params = {
      .autoCreateMaterial = model3dj["autoCreateMaterial"],
      .materialsShader = StringToWString(model3dj["materialsShader"])
    };

    return concurrency::create_task([filename, assetPath, name, params] {

      std::lock_guard<std::mutex> lock(rendererMutex);

      Model3DPtr model = std::make_shared<Model3D>();
      model->loading = true;
      model->assetPath = assetPath;
      model->model3dDefinition = params;
      model3DTemplates.insert(std::pair<std::wstring, Model3DPtr>(name, model));

      std::filesystem::path path(filename);

      Assimp::Importer importer;
      const aiScene* aiModel = importer.ReadFile(path.string(),
        aiProcess_JoinIdenticalVertices | aiProcess_GenNormals |
        aiProcess_CalcTangentSpace | aiProcess_Triangulate |
        aiProcess_FlipUVs
      );

      if (!aiModel) {
        OutputDebugStringA(importer.GetErrorString());
        assert(!aiModel);
      }

      //fill the length of the animations so they can be looped
      if (aiModel->mNumAnimations > 0U) {
        using namespace Animation;
        model->animations = CreateAnimatedFromAssimp(aiModel);
      }

      model->vertexClass = !model->animations ? POS_NORMAL_TANGENT_TEXCOORD0 : POS_NORMAL_TANGENT_TEXCOORD0_SKINNING;
      size_t vertexSize = !model->animations ? sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0>) : sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>);

      //go through all the meshes in the model
      for (UINT meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++) {

        auto aMesh = aiModel->mMeshes[meshIndex];

        model->vertices.push_back(VerticesLoader[model->vertexClass](aMesh));
        model->numVertices.push_back(aMesh->mNumVertices);
        model->indices.push_back(LoadIndices(aMesh));

        if (model->animations) {
          LoadBonesInVertices(aMesh, model->animations->bonesOffsets, model->vertices.back());
        }

        using namespace Templates::Mesh;
        MeshPtr mesh = std::make_shared<MeshT>();
        std::wstring meshName = getMeshName(name,meshIndex);
        mesh->loading = true;
        mesh->name = meshName;
        mesh->vertexClass = model->vertexClass;

        InitializeVertexBufferView(renderer->d3dDevice, renderer->commandList, *model->vertices.back(), static_cast<UINT>(vertexSize), model->numVertices.back(), mesh->vbvData);
        InitializeIndexBufferView(renderer->d3dDevice, renderer->commandList, model->indices.back().data(), static_cast<UINT>(model->indices.back().size()), mesh->ibvData);

        if (params.autoCreateMaterial) {
          std::wstring materialName = getMaterialName(name,meshIndex);
          using namespace Templates::Material;
          if (GetMaterialTemplate(materialName) == nullptr) {
            CreateModel3DMaterial(materialName, params.materialsShader, path.relative_path(), aiModel->mMaterials[aMesh->mMaterialIndex]);
          }
        }

        model->meshes.push_back(mesh);
        mesh->loading = false;
      }

      importer.FreeScene();

    });
  }

}