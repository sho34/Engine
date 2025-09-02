#include "pch.h"
#include "Model3D.h"
#include <Templates.h>
#include <TemplateDef.h>
#include <Mesh/Mesh.h>
#include <Renderable/Renderable.h>
#include <VertexFormats.h>
#include <Animated.h>
#include <d3d12.h>
#include <nlohmann/json.hpp>
#include <Application.h>
#include <NoStd.h>
#if defined(_EDITOR)
#include <Textures/Texture.h>
#include <Material/Material.h>
#endif
#if defined(_DEVELOPMENT)
#include <Command.h>
#endif
#include <DDSTextures.h>

using namespace Animation;
using namespace DeviceUtils;
using namespace Templates;

namespace Templates
{
#include <JExposeAttDrawersDef.h>
#include <Model3DAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttJsonDef.h>
#include <Model3DAtt.h>
#include <JExposeEnd.h>

	Model3DJson::Model3DJson(nlohmann::json json) : JTemplate(json)
	{
#include <JExposeInit.h>
#include <Model3DAtt.h>
#include <JExposeEnd.h>

#include <JExposeAttUpdate.h>
#include <Model3DAtt.h>
#include <JExposeEnd.h>
	}

	TEMPDEF_FULL(Model3D);
	TEMPDEF_REFTRACKER(Model3D);

	std::string GetModel3DMeshInstanceUUID(std::string uuid, unsigned int index) {
		return "mesh-" + uuid + "-" + std::to_string(index);
	}

	std::string GetModel3DMaterialInstanceUUID(std::string uuid, unsigned int index) {
		return "mat-" + uuid + "-" + std::to_string(index);
	}

	std::string GetModel3DMaterialInstanceName(std::string model3dName, unsigned int index)
	{
		return "mat-" + model3dName + "-" + std::to_string(index);
	}

	void DestroyModel3DInstance(std::shared_ptr<Model3DInstance>& model3D)
	{
		std::string uuid = model3D->model3DUUID;

		using namespace Model3D;
		refTracker.RemoveRef(uuid, model3D);
	}

	Model3DInstance::Model3DInstance(std::string uuid)
	{
		model3DUUID = uuid;
		LoadModel3DInstance();
	}

	void Model3DInstance::LoadModel3DInstance()
	{
		std::shared_ptr<Model3DJson> mdl = GetModel3DTemplate(model3DUUID);

		std::string filename = default3DModelsFolder + mdl->path();

		std::filesystem::path path(filename);

		Assimp::Importer importer;
		const aiScene* aiModel = importer.ReadFile(path.string(),
			aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
			aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_ConvertToLeftHanded
		);

		if (!aiModel)
		{
			OutputDebugStringA(importer.GetErrorString());
			assert(!aiModel);
		}

		//fill the length of the animations so they can be looped
		if (aiModel->mNumAnimations > 0U)
		{
			animations = CreateAnimatedFromAssimp(aiModel);
		}

		CreateModel3DMaterialsTemplates(aiModel);

		vertexClass = !animations ? POS_NORMAL_TANGENT_TEXCOORD0 : POS_NORMAL_TANGENT_TEXCOORD0_SKINNING;
		size_t vertexSize = !animations ? sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0>) : sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>);

		//go through all the meshes in the model
		for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
		{
			auto aMesh = aiModel->mMeshes[meshIndex];

			std::vector<byte> vertexData(vertexSize * aMesh->mNumVertices);
			VerticesLoader.at(vertexClass)(aMesh, vertexData);

			std::vector<unsigned int> indicesData;
			LoadIndices(aMesh, indicesData);

			if (animations) {
				LoadBonesInVertices(aMesh, animations->bonesOffsets, reinterpret_cast<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*>(vertexData.data()));
			}

			unsigned int indicesCount = aMesh->mNumFaces * aMesh->mFaces[0].mNumIndices;
			std::shared_ptr<MeshInstance> mesh = GetMeshInstance(GetModel3DMeshInstanceUUID(model3DUUID, meshIndex), vertexClass, vertexData.data(), static_cast<unsigned int>(vertexSize), aMesh->mNumVertices, indicesData.data(), indicesCount);

			CreateBoundingBox(mesh->boundingBox, aMesh);

			meshes.push_back(mesh);

			materialUUIDs = mdl->materials();
		}

		importer.FreeScene();
	}

	void Model3DInstance::CreateModel3DMaterialsTemplates(const aiScene* aiModel)
	{
		using namespace Templates;

		std::shared_ptr<Model3DJson> mdl = GetModel3DTemplate(model3DUUID);

		if (mdl->materials().size() == aiModel->mNumMeshes)
			return;

		std::string filename = default3DModelsFolder + mdl->path();
		std::filesystem::path path(filename);

		//go through all the meshes in the model
		for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
		{
			auto aMesh = aiModel->mMeshes[meshIndex];
			aiMaterial* aiMat = aiModel->mMaterials[aMesh->mMaterialIndex];

			MaterialJson texturesMaterialJson = GetAssimpTexturesMaterialJson(path.relative_path(), aiMat);

			std::string materialUUID = GetModel3DMaterialInstanceUUID(model3DUUID, meshIndex);

			if (!MaterialTemplateExist(materialUUID))
			{
				MaterialJson materialJson = CreateModel3DMaterialJson(
					materialUUID,
					GetModel3DMaterialInstanceName(model3DUUID, meshIndex),
					mdl->shader_vs(),
					mdl->shader_ps(),
					aiModel->mMaterials[aMesh->mMaterialIndex]
				);
				materialJson.merge_patch(texturesMaterialJson);
				CreateMaterial(materialJson);
			}
			mdl->materials_push_back(materialUUID);
		}
	}

	void Model3DInstance::CreateBoundingBox(BoundingBox& boundingBox, aiMesh* aMesh)
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

	MaterialJson Model3DInstance::GetAssimpTexturesMaterialJson(std::filesystem::path relativePath, aiMaterial* material)
	{
		using namespace Templates::Model3D;

		MaterialJson mat(nlohmann::json({}));

		aiString diffuseName;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseName);
		PushAssimpTextureToJson(mat, TextureShaderUsage_Base, relativePath, diffuseName, defaultBaseTexture);

		aiString normalMapName;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), normalMapName);
		PushAssimpTextureToJson(mat, TextureShaderUsage_NormalMap, relativePath, normalMapName, defaultNormalMap, DXGI_FORMAT_R8G8B8A8_UNORM);

		aiString metallicRoughnessName;
		material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessName);
		PushAssimpTextureToJson(mat, TextureShaderUsage_MetallicRoughness, relativePath, metallicRoughnessName, "", DXGI_FORMAT_R8G8B8A8_UNORM);

		return mat;
	}

	void Model3DInstance::PushAssimpTextureToJson(MaterialJson& m, TextureShaderUsage textureType, std::filesystem::path relativePath, aiString& aiTextureName, std::string fallbackTexture, DXGI_FORMAT fallbackFormat)
	{
		std::filesystem::path textureJsonPath = fallbackTexture;
		DXGI_FORMAT textureJsonFormat = fallbackFormat;

		if (aiTextureName.length > 0)
		{
			textureJsonPath = nostd::normalize_path(
				relativePath.parent_path().append(aiTextureName.C_Str()).string()
			);
		}

		if (textureJsonPath != "")
		{
			std::string texUUID = FindTextureUUIDByName(textureJsonPath.string());
			if (texUUID.empty())
			{
				texUUID = CreateTextureTemplate(textureJsonPath.string(), textureJsonFormat);
			}
			m.textures_insert(textureType, texUUID);
		}
	}

	MaterialJson Model3DInstance::CreateModel3DMaterialJson(std::string materialUUID, std::string materialName, std::string vertexShader, std::string pixelShader, aiMaterial* material)
	{
		MaterialJson matJson(nlohmann::json({}));

		matJson.create_uuid(materialUUID);
		matJson.create_name(materialName);
		matJson.create_shader_vs(vertexShader);
		matJson.create_shader_ps(pixelShader);
		matJson.create_mappedValues({});

		bool twoSided;
		material->Get(AI_MATKEY_TWOSIDED, twoSided);
		D3D12_RASTERIZER_DESC rasterizerState;
		rasterizerState.CullMode = twoSided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
		matJson.rasterizerState(rasterizerState);

		ai_real shininess;
		material->Get(AI_MATKEY_SHININESS, shininess);
		matJson.mappedValues_push_back({ "specularExponent",{ MaterialVariablesTypes::MAT_VAR_FLOAT, shininess } });

		ai_real metallic;
		material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
		matJson.mappedValues_push_back({ "metallicFactor",{ MaterialVariablesTypes::MAT_VAR_FLOAT, metallic } });

		ai_real roughness;
		material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
		matJson.mappedValues_push_back({ "roughnessFactor",{ MaterialVariablesTypes::MAT_VAR_FLOAT, roughness } });

		aiString alphaMode;
		material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
		if (strcmp(alphaMode.C_Str(), "OPAQUE") == 0)
		{
			matJson.mappedValues_push_back({ "alphaCut",{ MaterialVariablesTypes::MAT_VAR_FLOAT, 1.0f } });
		}
		else
		{
			ai_real alphaCut;
			material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCut);
			matJson.mappedValues_push_back({ "alphaCut",{ MaterialVariablesTypes::MAT_VAR_FLOAT, alphaCut } });
		}

		matJson.create_samplers({ MaterialSamplerDesc() });

		return matJson;
	}
}