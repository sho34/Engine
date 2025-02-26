#include "pch.h"
#include "VertexFormats.h"

template<>
void CreateBoneData<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>(Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>& vertex)
{
	//put some values to the bone weight and ids(will be override after)
	vertex.BoneIds = { 0, 0, 0, 0 };
	vertex.BoneWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
}

void LoadIndices(aiMesh* mesh, std::vector<unsigned int>& indicesData)
{
	for (unsigned int meshFaceIndex = 0; meshFaceIndex < mesh->mNumFaces; meshFaceIndex++)
	{
		for (unsigned int index = 0; index < mesh->mFaces[meshFaceIndex].mNumIndices; index++)
		{
			indicesData.push_back(static_cast<unsigned int>(mesh->mFaces[meshFaceIndex].mIndices[index]));
		}
	}
}

void LoadBonesInVertices(aiMesh* mesh, Animation::BonesTransformations& bones, Vertex<POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>* vertices) {

	std::vector<unsigned int> numBonesPerVertex(mesh->mNumVertices, 0U);
	for (unsigned int meshBoneIndex = 0; meshBoneIndex < mesh->mNumBones; meshBoneIndex++)
	{
		auto bone = mesh->mBones[meshBoneIndex];
		std::string boneName = bone->mName.C_Str();
		unsigned int boneId = static_cast<unsigned int>(std::distance(bones.begin(), bones.find(boneName)));

		for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
		{
			auto weight = bone->mWeights[weightIndex];

			//skip bones with 0 weight
			if (weight.mWeight == 0.0f)
				continue;

			unsigned int offset = numBonesPerVertex[weight.mVertexId];
			if (offset >= 4) continue;

			//use offset to put the value in the propper slot (x:0, y:1, z:2, w:3)
			unsigned int* boneIdSlot = &vertices[weight.mVertexId].BoneIds.x + offset;
			float* weightSlot = &vertices[weight.mVertexId].BoneWeights.x + offset;

			*boneIdSlot = boneId;
			*weightSlot = weight.mWeight;
			numBonesPerVertex[weight.mVertexId]++;
		}
	}
}