#include "pch.h"
#include <d3d12.h>
#include <nlohmann/json.hpp>
#include "Model3D.h"
#include "../Mesh/Mesh.h"
#include "../Scene/Renderable/Renderable.h"
#include "../../Renderer/VertexFormats.h"
#include "../../Animation/Animated.h"
#include <Application.h>
#include <NoStd.h>
#if defined(_EDITOR)
#include "../../Editor/Editor.h"
#include <Editor.h>
namespace Editor {
	extern _Templates tempTab;
	extern std::string selTemp;
}
#endif

using namespace Animation;
using namespace DeviceUtils;

namespace Templates
{
	std::map<std::string, Model3DTemplate> model3ds;

	namespace Model3D
	{
#if defined(_EDITOR)
		unsigned int popupModalId = 0U;
#endif
		static nostd::RefTracker<std::string, std::shared_ptr<Model3DInstance>> refTracker; //uuid -> model3dInstance
	};


	//CREATE
	void CreateModel3D(nlohmann::json json)
	{
		std::string uuid = json.at("uuid");

		if (model3ds.contains("uuid"))
		{
			assert(!!!"model3d creation collision");
		}
		Model3DTemplate t;

		std::string& name = std::get<0>(t);
		name = json.at("name");

		nlohmann::json& data = std::get<1>(t);
		data = json;
		data.erase("name");
		data.erase("uuid");

		model3ds.insert_or_assign(uuid, t);
	}

	void LoadModel3DInstance(std::shared_ptr<Model3DInstance>& model, std::string uuid, nlohmann::json shaderAttributes)
	{
		model->uuid = uuid;
		model->shaderAttributes = shaderAttributes;

		nlohmann::json mdl = std::get<1>(model3ds.at(uuid));

		std::string filename = default3DModelsFolder + std::string(mdl.at("path"));

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
			model->animations = CreateAnimatedFromAssimp(aiModel);
		}

		model->vertexClass = !model->animations ? POS_NORMAL_TANGENT_TEXCOORD0 : POS_NORMAL_TANGENT_TEXCOORD0_SKINNING;
		size_t vertexSize = !model->animations ? sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0>) : sizeof(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>);

		//go through all the meshes in the model
		for (unsigned int meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++)
		{
			auto aMesh = aiModel->mMeshes[meshIndex];

			//model->vertices.push_back(std::vector<byte>(vertexSize * aMesh->mNumVertices));
			//std::vector<byte>& vertexData = model->vertices.back();
			std::vector<byte> vertexData(vertexSize * aMesh->mNumVertices);
			VerticesLoader.at(model->vertexClass)(aMesh, vertexData);

			std::vector<unsigned int> indicesData;
			LoadIndices(aMesh, indicesData);

			if (model->animations) {
				LoadBonesInVertices(aMesh, model->animations->bonesOffsets, reinterpret_cast<Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>*>(vertexData.data()));
			}

			unsigned int indicesCount = aMesh->mNumFaces * aMesh->mFaces[0].mNumIndices;
			std::shared_ptr<MeshInstance> mesh = GetMeshInstance(GetModel3DMeshInstanceUUID(uuid, meshIndex), model->vertexClass, vertexData.data(), static_cast<unsigned int>(vertexSize), aMesh->mNumVertices, indicesData.data(), indicesCount);

			CreateBoundingBox(mesh->boundingBox, aMesh);

			model->meshes.push_back(mesh);

			std::string materialUUID = "";
			if (mdl.at("autoCreateMaterial"))
			{
				materialUUID = GetModel3DMaterialInstanceUUID(uuid, meshIndex);
				nlohmann::json materialTemplate = GetMaterialTemplate(materialUUID);

				if (materialTemplate.empty())
				{
#if defined(_DEVELOPMENT)
					nlohmann::json materialJson = CreateModel3DMaterialJson(
						materialUUID,
						GetModel3DMaterialInstanceName(std::get<0>(model3ds.at(uuid)), meshIndex),
						mdl.at("shader"),
						path.relative_path(),
						aiModel->mMaterials[aMesh->mMaterialIndex]
					);
					CreateMaterial(materialJson);
#else
					assert(!!!"Non development will never create a material");
#endif
				}
			}
			else
			{
				materialUUID = mdl.at("materials").at(meshIndex);
			}

			model->materialUUIDs.push_back(materialUUID);
		}

		importer.FreeScene();

		for (unsigned int i = 0; i < model->materialUUIDs.size(); i++)
		{
			std::shared_ptr<MaterialInstance> materialInstance = model->GetModel3DMaterialInstance(i);
			model->materials.push_back(materialInstance);
		}
	}

#if defined(_DEVELOPMENT)
	std::string GetUtilsPath()
	{
		char* utilsPath;
		size_t utilsPathLen;
		_dupenv_s(&utilsPath, &utilsPathLen, "CULPEO_UTILS");

		return std::string(utilsPath);
	}

	void GetTextureAttributes(std::filesystem::path src, DXGI_FORMAT& format, int& numFrames)
	{
		using namespace raymii;

		//get the format of the file
		std::string cmdFormat = GetUtilsPath() + "texformat.bat " + src.string();
		CommandResult resultFormat = Command::exec(cmdFormat);
		resultFormat.output.pop_back();
		format = stringToDxgiFormat.at(resultFormat.output);

		//get the num of frames of the file
		std::string cmdFrames = GetUtilsPath() + "texframes.bat " + src.string();
		CommandResult result = Command::exec(cmdFrames);
		result.output.pop_back();
		numFrames = std::stoi(result.output) - 1;
	}

	void ConvertToDDS(std::filesystem::path src, std::filesystem::path dst, DXGI_FORMAT& format, int& numFrames)
	{
		GetTextureAttributes(src, format, numFrames);

		using namespace raymii;

		//get the format of the file
		std::filesystem::path parentPath = src.parent_path();
		std::string cmdConv = GetUtilsPath() + "texconv.exe " + src.string() + " -f " + dxgiFormatsToString.at(format) + " -l -y -o " + parentPath.string();
		CommandResult result = Command::exec(cmdConv);
	}

	void PushAssimpTextureToJson(nlohmann::json& j, TextureType textureType, std::filesystem::path relativePath, aiString& aiTextureName, std::string fallbackTexture = "", DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
	{
		std::filesystem::path textureJsonPath = fallbackTexture;
		DXGI_FORMAT textureJsonFormat = format;
		int numFrames = 0;

		if (aiTextureName.length > 0)
		{
			textureJsonPath = relativePath.parent_path().append(aiTextureName.C_Str()).replace_extension(".dds").string();
			std::filesystem::path originalTexturePath = relativePath.parent_path().append(aiTextureName.C_Str()).make_preferred();

			if (!std::filesystem::exists(textureJsonPath))
			{
				ConvertToDDS(originalTexturePath, textureJsonPath, textureJsonFormat, numFrames);
			}
			else
			{
				GetDDSTextureAttributes(textureJsonPath, textureJsonFormat, numFrames);
			}
		}

		if (textureJsonPath != "")
		{
			j["textures"][textureTypeToStr.at(textureType)] = {
				{ "path", textureJsonPath.string() },
				{ "format", dxgiFormatsToString.at(textureJsonFormat) },
				{ "numFrames", numFrames }
			};
		}

	};

	nlohmann::json CreateModel3DMaterialJson(std::string materialUUID, std::string materialName, std::string shader, std::filesystem::path relativePath, aiMaterial* material)
	{
		nlohmann::json matJson = { {"pipelineState",{}} };

		matJson["uuid"] = materialUUID;
		matJson["name"] = materialName;
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
		PushAssimpTextureToJson(matJson, TextureType_Base, relativePath, diffuseName, "Assets/textures/gridmap.dds");

		aiString normalMapName;
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), normalMapName);
		PushAssimpTextureToJson(matJson, TextureType_NormalMap, relativePath, normalMapName, "Assets/textures/bumpmapflat.dds", DXGI_FORMAT_R8G8B8A8_UNORM);

		aiString metallicRoughnessName;
		material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessName);
		PushAssimpTextureToJson(matJson, TextureType_MetallicRoughness, relativePath, metallicRoughnessName, "", DXGI_FORMAT_R8G8B8A8_UNORM);

		matJson["samplers"] = nlohmann::json::array();
		matJson["samplers"].push_back(MaterialSamplerDesc().json());

		return matJson;
	}
#endif

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

	Model3DTemplate GetModel3DTemplate(std::string uuid)
	{
		return model3ds.at(uuid);
	}

	//READ&GET
	std::shared_ptr<Model3DInstance> GetModel3DInstance(std::string uuid, nlohmann::json shaderAttributes)
	{
		if (!model3ds.contains(uuid)) return nullptr;

		using namespace Model3D;
		return refTracker.AddRef(uuid, [uuid, shaderAttributes]()
			{
				std::shared_ptr<Model3DInstance> instance = std::make_shared<Model3DInstance>();
				LoadModel3DInstance(instance, uuid, shaderAttributes);
				return instance;
			}
		);
	}

	std::vector<std::string> GetModels3DNames() {
		return GetNames(model3ds);
	}

	std::string GetModel3DName(std::string uuid)
	{
		return std::get<0>(model3ds.at(uuid));
	}

	std::vector<UUIDName> GetModels3DUUIDsNames()
	{
		return GetUUIDsNames(model3ds);
	}

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

	//UPDATE

	//DESTROY
#if defined(_EDITOR)
	void Model3D::ReleaseRenderablesInstances()
	{
		for (auto& [uuid, tuple] : model3ds)
		{
			auto& instances = std::get<2>(tuple);
			instances.clear();
		}
	}
#endif

	void DestroyModel3DInstance(std::shared_ptr<Model3DInstance>& model3D)
	{
		std::string uuid = model3D->uuid;

		using namespace Model3D;
		std::vector<std::string> materialUUIDs = model3D->materialUUIDs;
		refTracker.RemoveRef(uuid, model3D);
		if (!refTracker.Has(uuid) && model3ds.contains(uuid))
		{
			nlohmann::json& mdl = std::get<1>(model3ds.at(uuid));

			if (mdl.at("autoCreateMaterial"))
			{
				for (auto& matUUID : materialUUIDs)
				{
					DestroyMaterial(matUUID);
				}
			}
		}
	}

	//EDITOR
	void ReleaseModel3DTemplates()
	{
		using namespace Model3D;
		refTracker.Clear();
		model3ds.clear();
	}

#if defined(_EDITOR)
	void BindNotifications(std::string uuid, std::shared_ptr<Scene::Renderable> renderable)
	{
		auto& instances = std::get<2>(model3ds.at(uuid));
		instances.push_back(renderable);
	}

	void UnbindNotifications(std::string uuid, std::shared_ptr<Scene::Renderable> renderable)
	{
		auto& instances = std::get<2>(model3ds.at(uuid));
		for (auto it = instances.begin(); it != instances.end(); )
		{
			if (*it = renderable)
			{
				it = instances.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	void ReloadModel3DInstances(std::string uuid)
	{
		auto& instances = std::get<2>(model3ds.at(uuid));
		for (auto& it : instances)
		{
			it->ReloadModel3D();
		}
	}

	void DrawModel3DPanel(std::string uuid, ImVec2 pos, ImVec2 size, bool pop)
	{
		std::string tableName = "model3d-panel";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoSavedSettings))
		{
			Model3D::DrawEditorInformationAttributes(uuid);
			Model3D::DrawEditorAssetAttributes(uuid);
			Model3D::DrawEditorMaterialAttributes(uuid);
			ImGui::EndTable();
		}
	}

	void CreateNewModel3D()
	{
	}

	void Model3D::DrawEditorInformationAttributes(std::string uuid)
	{
		nlohmann::json& json = std::get<1>(model3ds.at(uuid));

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string tableName = "model3d-information-atts";
		if (ImGui::BeginTable(tableName.c_str(), 1, ImGuiTableFlags_NoSavedSettings))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			std::string& currentName = std::get<0>(model3ds.at(uuid));
			ImGui::InputText("name", &currentName);
			ImGui::EndTable();
		}
	}

	void Model3D::DrawEditorAssetAttributes(std::string uuid)
	{
		nlohmann::json& json = std::get<1>(model3ds.at(uuid));

		std::string parentFolder = default3DModelsFolder;
		std::string fileName = "";
		if (json.contains("path") && !json.at("path").empty())
		{
			fileName = json.at("path");
			std::filesystem::path rootFolder = fileName;
			parentFolder = default3DModelsFolder + rootFolder.parent_path().string();
		}

		ImGui::PushID("File-Selector");
		{
			ImDrawFileSelector("##", fileName, [&json, uuid](std::filesystem::path path)
				{
					std::filesystem::path curPath = std::filesystem::current_path().append(default3DModelsFolder);
					std::filesystem::path relPath = std::filesystem::relative(path, curPath);
					json.at("path") = relPath.string();
					ReloadModel3DInstances(uuid);
				},
				parentFolder, "3d model files. (*.gltf)", "*.gltf"
			);
		}
		ImGui::PopID();
	}

	void Model3D::DrawEditorMaterialAttributes(std::string uuid)
	{
		nlohmann::json& json = std::get<1>(model3ds.at(uuid));

		ImGui::PushID("Auto-Create-Material");
		{
			drawFromCheckBox(json, "autoCreateMaterial", "Auto create material");
		}
		ImGui::PopID();

		std::string shaderUUID = json.at("shader");
		std::vector<UUIDName> shadersUUIDNames = GetShadersUUIDsNames();
		std::vector<UUIDName> selectables = { std::tie(" "," ") };
		UUIDName currentShader = std::make_tuple(shaderUUID, GetShaderName(shaderUUID));

		nostd::AppendToVector(selectables, shadersUUIDNames);
		ImGui::PushID("Select-Shader");
		{
			DrawComboSelection(currentShader, selectables, [&json, uuid](UUIDName newShader)
				{
					json.at("shader") = std::get<0>(newShader);
					ReloadModel3DInstances(uuid);
				}, "Shader"
			);
		}
		ImGui::PopID();
	}

	void DeleteModel3D(std::string uuid)
	{
		nlohmann::json json = std::get<1>(model3ds.at(uuid));
		if (json.contains("systemCreated") && json.at("systemCreated") == true)
		{
			Model3D::popupModalId = Model3DPopupModal_CannotDelete;
		}
	}

	void DrawModels3DsPopups()
	{
		Editor::DrawOkPopup(Model3D::popupModalId, Model3DPopupModal_CannotDelete, "Cannot delete model3d", []
			{
				ImGui::Text("Cannot delete a system created model 3D");
			}
		);
	}

	void WriteModel3DsJson(nlohmann::json& json)
	{
		WriteTemplateJson(json, model3ds);
	}

#endif

	/*
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
	*/

	std::shared_ptr<MaterialInstance> Model3DInstance::GetModel3DMaterialInstance(unsigned int meshIndex)
	{
		return GetMaterialInstance(materialUUIDs[meshIndex], std::map<TextureType, MaterialTexture>(), meshes[meshIndex], shaderAttributes);
	}
}