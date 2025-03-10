#include "pch.h"
#include "Model3D.h"
#include "../Mesh/Mesh.h"
#include "../../Renderer/VertexFormats.h"
#include "../../Animation/Animated.h"
#include <d3d12.h>
#include <nlohmann/json.hpp>

using namespace DeviceUtils;

namespace Templates {

	static std::map<std::string, nlohmann::json> model3DTemplates;
	static nostd::RefTracker<std::string, std::shared_ptr<Model3DInstance>> refTracker;

	//CREATE
	void CreateModel3D(std::string name, nlohmann::json json)
	{
		if (model3DTemplates.contains(name)) return;
		model3DTemplates.insert_or_assign(name, json);
	}

	void LoadModel3DInstance(std::shared_ptr<Model3DInstance>& model, std::string name, nlohmann::json shaderAttributes)
	{
		model->name = name;
		nlohmann::json mdl = model3DTemplates.at(name);

		std::string filename = Model3D::assetsRootFolder + std::string(mdl.at("path"));

		std::filesystem::path path(filename);

		Assimp::Importer importer;
		const aiScene* aiModel = importer.ReadFile(path.string(),
			aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes
		);

		if (!aiModel)
		{
			OutputDebugStringA(importer.GetErrorString());
			assert(!aiModel);
		}

		//fill the length of the animations so they can be looped
		if (aiModel->mNumAnimations > 0U)
		{
			using namespace Animation;
			model->animations = CreateAnimatedFromAssimp(aiModel);
		}

		model->vertexClass = !model->animations ? POS_NORMAL_TANGENT_TEXCOORD0 : POS_NORMAL_TANGENT_TEXCOORD0_SKINNING;
		size_t vertexSize = !model->animations ? sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0>) : sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>);

		//go through all the meshes in the model
		for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
		{
			auto aMesh = aiModel->mMeshes[meshIndex];

			model->vertices.push_back(std::vector<byte>(vertexSize * aMesh->mNumVertices));
			std::vector<byte>& vertexData = model->vertices.back();
			VerticesLoader.at(model->vertexClass)(aMesh, vertexData);

			std::vector<unsigned int> indicesData;
			LoadIndices(aMesh, indicesData);

			if (model->animations) {
				LoadBonesInVertices(aMesh, model->animations->bonesOffsets, reinterpret_cast<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*>(vertexData.data()));
			}

			unsigned int indicesCount = aMesh->mNumFaces * aMesh->mFaces[0].mNumIndices;
			std::shared_ptr<MeshInstance> mesh = GetMeshInstance(GetModel3DMeshInstanceName(name, meshIndex), model->vertexClass, vertexData.data(), static_cast<unsigned int>(vertexSize), aMesh->mNumVertices, indicesData.data(), indicesCount);

			CreateBoundingBox(mesh->boundingBox, aMesh);

			model->meshes.push_back(mesh);

			std::shared_ptr<MaterialInstance> materialInstance = nullptr;

			if (mdl.at("autoCreateMaterial"))
			{
				std::string materialName = GetModel3DMaterialInstanceName(name, meshIndex);
				nlohmann::json materialTemplate = GetMaterialTemplate(materialName);

				if (materialTemplate.empty())
				{
					nlohmann::json materialJson = CreateModel3DMaterialJson(mdl.at("materialsShader"), path.relative_path(), aiModel->mMaterials[aMesh->mMaterialIndex]);
					CreateMaterial(materialName, materialJson);
				}

				materialInstance = GetMaterialInstance(materialName, std::map<TextureType, MaterialTexture>(), mesh, shaderAttributes);
			}
			else
			{
				materialInstance = GetMaterialInstance(mdl.at("materials").at(meshIndex), std::map<TextureType, MaterialTexture>(), mesh, shaderAttributes);
			}

			model->materials.push_back(materialInstance);
		}

		importer.FreeScene();
	}

	void PushAssimpTextureToJson(nlohmann::json& j, TextureType textureType, std::filesystem::path relativePath, aiString& aiTextureName, std::string fallbackTexture = "", DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
	{
		std::string textureJsonPath = fallbackTexture;

		if (aiTextureName.length > 0)
		{
			textureJsonPath = relativePath.parent_path().append(aiTextureName.C_Str()).replace_extension(".dds").string();
		}

		if (textureJsonPath != "")
		{
			j["textures"][textureTypeToStr.at(textureType)] = {
				{ "path", textureJsonPath },
				{ "format", dxgiFormatsToString.at(format) },
				{ "numFrames", 0 }
			};
		}
	};

	nlohmann::json CreateModel3DMaterialJson(std::string shader, std::filesystem::path relativePath, aiMaterial* material)
	{
		nlohmann::json matJson = { {"pipelineState",{}} };

		matJson["shader"] = shader;

		bool twoSided;
		material->Get(AI_MATKEY_TWOSIDED, twoSided);
		matJson["pipelineState"]["RasterizerState"]["CullMode"] = cullModeToString.at((twoSided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK));

		MaterialInitialValueMap matInitialValue;

		ai_real shininess;
		material->Get(AI_MATKEY_SHININESS, shininess);
		matInitialValue["specularExponent"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, shininess };

		ai_real metallic;
		material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
		matInitialValue["metallicFactor"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, metallic };

		ai_real roughness;
		material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		matInitialValue["roughnessFactor"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, roughness };

		aiString alphaMode;
		material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
		if (strcmp(alphaMode.C_Str(), "OPAQUE") == 0)
		{
			matInitialValue["alphaCut"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, 1.0f };
		}
		else
		{
			ai_real alphaCut;
			material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCut);
			matInitialValue["alphaCut"] = { MaterialVariablesTypes::MAT_VAR_FLOAT, alphaCut };
		}

		matJson["mappedValues"] = TransformMaterialValueMappingToJson(matInitialValue);

		aiString diffuseName;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseName);
		//pushAiMaterialToTextures(diffuseName, "Assets/textures/gridmap.dds");
		PushAssimpTextureToJson(matJson, TextureType_Base, relativePath, diffuseName, "Assets/textures/gridmap.dds");

		aiString normalMapName;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), normalMapName);
		//pushAiMaterialToTextures(normalMapName, "Assets/textures/bumpmapflat.dds", DXGI_FORMAT_R8G8B8A8_UNORM);
		PushAssimpTextureToJson(matJson, TextureType_NormalMap, relativePath, normalMapName, "Assets/textures/bumpmapflat.dds", DXGI_FORMAT_R8G8B8A8_UNORM);

		aiString metallicRoughnessName;
		material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessName);
		//pushAiMaterialToTextures(metallicRoughnessName, "", DXGI_FORMAT_R8G8B8A8_UNORM);
		PushAssimpTextureToJson(matJson, TextureType_MetallicRoughness, relativePath, metallicRoughnessName, "", DXGI_FORMAT_R8G8B8A8_UNORM);

		matJson["samplers"] = { MaterialSamplerDesc().json() };

		return matJson;
	}

	void CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh)
	{
		XMFLOAT3 center = {
			0.5f * (aMesh->mAABB.mMin[0] + aMesh->mAABB.mMax[0]),
			0.5f * (aMesh->mAABB.mMin[1] + aMesh->mAABB.mMax[1]),
			0.5f * (aMesh->mAABB.mMin[2] + aMesh->mAABB.mMax[2]),
		};

		XMFLOAT3 extents = {
			0.5f * fabs(aMesh->mAABB.mMin[0] - aMesh->mAABB.mMax[0]),
			0.5f * fabs(aMesh->mAABB.mMin[1] - aMesh->mAABB.mMax[1]),
			0.5f * fabs(aMesh->mAABB.mMin[2] - aMesh->mAABB.mMax[2]),
		};

		boundingBox = BoundingBox(center, extents);
	}

	//READ&GET
	std::shared_ptr<Model3DInstance> GetModel3DInstance(std::string name, nlohmann::json shaderAttributes)
	{
		if (!model3DTemplates.contains(name)) return nullptr;

		return refTracker.AddRef(name, [name, shaderAttributes]()
			{
				std::shared_ptr<Model3DInstance> instance = std::make_shared<Model3DInstance>();
				LoadModel3DInstance(instance, name, shaderAttributes);
				return instance;
			}
		);
	}

	std::vector<std::string> GetModels3DNames() {
		return nostd::GetKeysFromMap(model3DTemplates);
	}

	std::string GetModel3DMeshInstanceName(std::string name, unsigned int index) {
		return "mesh." + name + "." + std::to_string(index);
	}

	std::string GetModel3DMaterialInstanceName(std::string name, unsigned int index) {
		return "mat." + name + "." + std::to_string(index);
	}

	//UPDATE

	//DESTROY

	void DestroyModel3DInstance(std::shared_ptr<Model3DInstance>& model3D)
	{
		refTracker.RemoveRef(model3D->name, model3D);
	}

	//EDITOR
	void ReleaseModel3DTemplates()
	{
		refTracker.Clear();
		model3DTemplates.clear();
	}

#if defined(_EDITOR)
	void DrawModel3DPanel(std::string& model, ImVec2 pos, ImVec2 size, bool pop)
	{
	}

	std::string GetModel3DInstanceTemplateName(std::shared_ptr<Model3DInstance> model3D)
	{
		return nostd::GetKeyFromValueInMap(refTracker.instances, model3D);
	}

	/*
	nlohmann::json json()
	{
		nlohmann::json models = nlohmann::json({});

		for (auto& [name, model] : model3DTemplates) {

			models[name] = {
				{ "autoCreateMaterial", model->model3dDefinition.autoCreateMaterial },
				{ "materialsShader", model->model3dDefinition.materialsShader },
				{ "assetPath", model->assetPath }
			};
		}

		return models;
	}
	*/
#endif

	BoundingBox Model3DInstance::GetAnimatedBoundingBox(XMMATRIX* bones)
	{
		XMFLOAT3 aaMin = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 aaMax = { 0.0f, 0.0f, 0.0f };
		for (unsigned int i = 0; i < static_cast<unsigned int>(vertices.size()); i++) {
			unsigned int nVertices = static_cast<unsigned int>(vertices[i].size() / sizeof(VertexPosNormalTangentTexCoordSkinning));
			VertexPosNormalTangentTexCoordSkinning* vertex = reinterpret_cast<VertexPosNormalTangentTexCoordSkinning*>(vertices[i].data());
			for (unsigned int v = 0; v < nVertices; v++) {
				XMVECTOR Position = { vertex[v].Position.x, vertex[v].Position.y, vertex[v].Position.z, 1.0f };
				XMUINT4 BoneIds = vertex[v].BoneIds;
				XMFLOAT4 BoneWeights = vertex[v].BoneWeights;

				XMMATRIX boneTransform = XMMatrixTranspose(bones[BoneIds.x] * BoneWeights.x +
					bones[BoneIds.y] * BoneWeights.y +
					bones[BoneIds.z] * BoneWeights.z +
					bones[BoneIds.w] * BoneWeights.w);

				XMVECTOR skinnedPos = XMVector3Transform(Position, boneTransform);

				aaMin = { fmin(aaMin.x,skinnedPos.m128_f32[0]), fmin(aaMin.y,skinnedPos.m128_f32[1]), fmin(aaMin.z,skinnedPos.m128_f32[2]) };
				aaMax = { fmax(aaMax.x,skinnedPos.m128_f32[0]), fmax(aaMax.y,skinnedPos.m128_f32[1]), fmax(aaMax.z,skinnedPos.m128_f32[2]) };
			}
		}

		XMFLOAT3 center = {
			0.5f * (aaMin.x + aaMax.x),
			0.5f * (aaMin.y + aaMax.y),
			0.5f * (aaMin.z + aaMax.z),
		};

		XMFLOAT3 extents = {
			0.5f * fabs(aaMin.x - aaMax.x),
			0.5f * fabs(aaMin.y - aaMax.y),
			0.5f * fabs(aaMin.z - aaMax.z),
		};

		return BoundingBox(center, extents);
	}
}