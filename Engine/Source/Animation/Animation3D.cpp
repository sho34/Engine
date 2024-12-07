#include "pch.h"
#include "Animation3D.h"
#include "../Renderer/VertexFormats.h"

void BuildBoneInfo(const aiScene* aiModel, BoneInfoMap& boneInfo) {

	//go through all the meshes in the model
	for (UINT meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++) {

		auto mesh = aiModel->mMeshes[meshIndex];

		//and through all the bones in the mesh
		for (UINT meshBoneIndex = 0; meshBoneIndex < mesh->mNumBones; meshBoneIndex++) {

			auto bone = mesh->mBones[meshBoneIndex];
			std::string boneName = bone->mName.C_Str();
			std::wstring boneNameW(boneName.begin(), boneName.end());

			//if the bone slot hasn't been created yet, create it
			if (boneInfo.find(boneNameW) == boneInfo.end()) {
				boneInfo[boneNameW].offset = XMMATRIX(&bone->mOffsetMatrix.a1);
			}

		}
	}
}

void BuildAnimationChannelsKeys(const aiScene* model, AnimationChannelKeys& animationChannelsKeys){
	for (auto anim = model->mAnimations; anim < (model->mAnimations + model->mNumAnimations); anim++) {
		std::string animName = (*anim)->mName.C_Str();
		std::wstring animNameW(animName.begin(), animName.end());
		for (auto channel = (*anim)->mChannels; channel < ((*anim)->mChannels + (*anim)->mNumChannels); channel++) {
			std::string nodeName = (*channel)->mNodeName.C_Str();
			std::wstring nodeNameW(nodeName.begin(), nodeName.end());
			for (auto position = (*channel)->mPositionKeys; position < ((*channel)->mPositionKeys + (*channel)->mNumPositionKeys); position++) {
				animationChannelsKeys[animNameW][nodeNameW].positions.push_back({ static_cast<FLOAT>(position->mTime), { position->mValue.x, position->mValue.y, position->mValue.z } });
			}
			for (auto rotation = (*channel)->mRotationKeys; rotation < ((*channel)->mRotationKeys + (*channel)->mNumRotationKeys); rotation++) {
				animationChannelsKeys[animNameW][nodeNameW].rotation.push_back({ static_cast<FLOAT>(rotation->mTime), { rotation->mValue.x, rotation->mValue.y, rotation->mValue.z, rotation->mValue.w } });
			}
			for (auto scale = (*channel)->mScalingKeys; scale < ((*channel)->mScalingKeys + (*channel)->mNumScalingKeys); scale++) {
				animationChannelsKeys[animNameW][nodeNameW].scaling.push_back({ static_cast<FLOAT>(scale->mTime), { scale->mValue.x, scale->mValue.y, scale->mValue.z } });
			}
		}
	}
}

void BuildNodesHierarchy(aiNode* node, HierarchyNode* nodeInHierarchy, MultiplyCmdQueue& multiplyNavigator){
	nodeInHierarchy->numChildren = 0;
	nodeInHierarchy->children = nullptr;
	std::string nodeName = node->mName.C_Str();
	std::wstring nodeNameW(nodeName.begin(), nodeName.end());
	nodeInHierarchy->name = nodeNameW;
	nodeInHierarchy->transformation = XMMATRIX(&node->mTransformation.a1);
	if (node->mNumChildren > 0) {
		multiplyNavigator.push(MultiplyCmd(nodeInHierarchy,true));
		nodeInHierarchy->numChildren = node->mNumChildren;
		nodeInHierarchy->children = new HierarchyNode[nodeInHierarchy->numChildren];
		UINT childOffset = 0;
		for (auto childNode = node->mChildren; childNode < (node->mChildren + node->mNumChildren); childNode++, childOffset++) {
			BuildNodesHierarchy(*childNode, &nodeInHierarchy->children[childOffset], multiplyNavigator);
		}
		multiplyNavigator.push(MultiplyCmd(nodeInHierarchy, false));
	} else {
		multiplyNavigator.push(MultiplyCmd(nodeInHierarchy, true));
		multiplyNavigator.push(MultiplyCmd(nodeInHierarchy, false));
	}
}

void DestroyNodesHierarchy(HierarchyNode* node) {
	for (auto child = node->children; child < node->children + node->numChildren; child++) {
		DestroyNodesHierarchy(child);
	}
	if (node->children) delete[] node->children;
}

/*
XMMATRIX InterpolateKeys(XMMATRIX(XM_CALLCONV* ToMatrix)(XMVECTOR), XMVECTOR(XM_CALLCONV* Interpolator)(XMVECTOR, XMVECTOR, float), FLOAT time, std::vector<KeyFrame>& keyFrames) {
	if (keyFrames.size() == 1) { return ToMatrix(keyFrames[0].key); }

	for (UINT index = 0U; index < keyFrames.size() - 1; index++) {
		auto& keyStart = keyFrames[index];
		auto& keyEnd = keyFrames[index + 1];
		if (time >= keyStart.time && time < keyEnd.time) {
			float delta = keyEnd.time - keyStart.time;
			float t = (time - keyStart.time) / delta;
			return ToMatrix(Interpolator(keyStart.key, keyEnd.key, t));
		}
	}

	return ToMatrix(keyFrames[0].key);
}

void TraverseNodeHierarchy(FLOAT time, HierarchyNode* node, std::map<std::string, BoneKeys>& boneKeys,
	std::map<std::string, BoneInfo>& boneInfo, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation) {
	XMMATRIX nodeTransformation = node->transformation;

	auto keys = boneKeys.find(node->name);
	if (keys != boneKeys.end()) {
		XMMATRIX scaling = InterpolateKeys(XMMatrixScalingFromVector, XMVectorLerp, time, keys->second.scaling);
		XMMATRIX rotation = InterpolateKeys(XMMatrixRotationQuaternion, XMQuaternionSlerp, time, keys->second.rotation);
		XMMATRIX translation = InterpolateKeys(XMMatrixTranslationFromVector, XMVectorLerp, time, keys->second.positions);
		nodeTransformation = XMMatrixTranspose(XMMatrixMultiply(scaling, XMMatrixMultiply(rotation, translation)));
	}

	XMMATRIX transformation = XMMatrixMultiply(parentTransformation, nodeTransformation);

	//map the new transformation to the bone
	//mapear la nueva transformacion al hueso
	auto bone = boneInfo.find(node->name);
	if (bone != boneInfo.end()) {
		bone->second.transformation = XMMatrixMultiply(rootNodeInverseTransform, XMMatrixMultiply(transformation, bone->second.offset));
	}

	for (auto children = node->children; children < (node->children + node->numChildren); children++) {
		TraverseNodeHierarchy(time, children, boneKeys, boneInfo, rootNodeInverseTransform, transformation);
	}
};

void TraverseMultiplycationQueue(FLOAT time, std::queue<multiplyCmd>& cmds, std::map<std::string, BoneKeys>& boneKeys, std::map<std::string, BoneInfo>& boneInfo, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation) {

	auto execCmds = cmds;
	std::stack<XMMATRIX> transformation;
	transformation.push(XMMatrixIdentity());

	while (!execCmds.empty()) {
		multiplyCmd toDo = execCmds.front();

		if (!toDo.second) {
			transformation.pop();
			execCmds.pop();
		} else {
			HierarchyNode* node = toDo.first;
			XMMATRIX nodeTransformation = node->transformation;

			auto keys = boneKeys.find(node->name);
			if (keys != boneKeys.end()) {
				XMMATRIX scaling = InterpolateKeys(XMMatrixScalingFromVector, XMVectorLerp, time, keys->second.scaling);
				XMMATRIX rotation = InterpolateKeys(XMMatrixRotationQuaternion, XMQuaternionSlerp, time, keys->second.rotation);
				XMMATRIX translation = InterpolateKeys(XMMatrixTranslationFromVector, XMVectorLerp, time, keys->second.positions);
				nodeTransformation = XMMatrixTranspose(XMMatrixMultiply(scaling, XMMatrixMultiply(rotation, translation)));
			}

			transformation.push(XMMatrixMultiply(transformation.top(), nodeTransformation));

			auto bone = boneInfo.find(node->name);
			if (bone != boneInfo.end()) {
				bone->second.transformation = XMMatrixMultiply(rootNodeInverseTransform, XMMatrixMultiply(transformation.top(), bone->second.offset));
			}
			execCmds.pop();
		}
	}
}
*/