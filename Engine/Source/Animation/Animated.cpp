#include "pch.h"
#include "Animated.h"
#include <assimp/scene.h>
#include "../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include "../Renderer/Renderable.h"

namespace Animation {

  static std::map<std::shared_ptr<Renderable::Renderable>, ConstantsBufferViewDataPtr> animationsCBV;

	void BuildBonesOffsets(const aiScene* aiModel, BonesTransformations& bonesOffsets) {

		//go through all the meshes in the model
		for (UINT meshIndex = 0; meshIndex < aiModel->mNumMeshes; meshIndex++) {

			auto mesh = aiModel->mMeshes[meshIndex];

			//and through all the bones in the mesh
			for (UINT meshBoneIndex = 0; meshBoneIndex < mesh->mNumBones; meshBoneIndex++) {

				auto bone = mesh->mBones[meshBoneIndex];
				std::string boneName = bone->mName.C_Str();
				std::wstring boneNameW(boneName.begin(), boneName.end());

				//if the bone slot hasn't been created yet, create it
				if (bonesOffsets.find(boneNameW) == bonesOffsets.end()) {
					bonesOffsets[boneNameW] = XMMATRIX(&bone->mOffsetMatrix.a1);
				}

			}
		}
	}

	void BuildAnimationBonesKeys(const aiScene* model, AnimationBonesKeys& animationBonesKeys) {
		for (auto anim = model->mAnimations; anim < (model->mAnimations + model->mNumAnimations); anim++) {
			std::string animName = (*anim)->mName.C_Str();
			std::wstring animNameW(animName.begin(), animName.end());
			for (auto channel = (*anim)->mChannels; channel < ((*anim)->mChannels + (*anim)->mNumChannels); channel++) {
				std::string nodeName = (*channel)->mNodeName.C_Str();
				std::wstring nodeNameW(nodeName.begin(), nodeName.end());
				for (auto position = (*channel)->mPositionKeys; position < ((*channel)->mPositionKeys + (*channel)->mNumPositionKeys); position++) {
					animationBonesKeys[animNameW][nodeNameW].positions.push_back({ static_cast<FLOAT>(position->mTime), { position->mValue.x, position->mValue.y, position->mValue.z } });
				}
				for (auto rotation = (*channel)->mRotationKeys; rotation < ((*channel)->mRotationKeys + (*channel)->mNumRotationKeys); rotation++) {
					animationBonesKeys[animNameW][nodeNameW].rotation.push_back({ static_cast<FLOAT>(rotation->mTime), { rotation->mValue.x, rotation->mValue.y, rotation->mValue.z, rotation->mValue.w } });
				}
				for (auto scale = (*channel)->mScalingKeys; scale < ((*channel)->mScalingKeys + (*channel)->mNumScalingKeys); scale++) {
					animationBonesKeys[animNameW][nodeNameW].scaling.push_back({ static_cast<FLOAT>(scale->mTime), { scale->mValue.x, scale->mValue.y, scale->mValue.z } });
				}
			}
		}
	}

	void BuildNodesHierarchy(aiNode* node, HierarchyNode* nodeInHierarchy, MultiplyCmdQueue& multiplyNavigator) {
		nodeInHierarchy->numChildren = 0;
		nodeInHierarchy->children = nullptr;
		std::string nodeName = node->mName.C_Str();
		std::wstring nodeNameW(nodeName.begin(), nodeName.end());
		nodeInHierarchy->name = nodeNameW;
		nodeInHierarchy->transformation = XMMATRIX(&node->mTransformation.a1);
		if (node->mNumChildren > 0) {
			multiplyNavigator.push(MultiplyCmd(nodeInHierarchy, true));
			nodeInHierarchy->numChildren = node->mNumChildren;
			nodeInHierarchy->children = new HierarchyNode[nodeInHierarchy->numChildren];
			UINT childOffset = 0;
			for (auto childNode = node->mChildren; childNode < (node->mChildren + node->mNumChildren); childNode++, childOffset++) {
				BuildNodesHierarchy(*childNode, &nodeInHierarchy->children[childOffset], multiplyNavigator);
			}
			multiplyNavigator.push(MultiplyCmd(nodeInHierarchy, false));
		}
		else {
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

  std::shared_ptr<Animated> CreateAnimatedFromAssimp(const aiScene* aiModel)
  {
    std::shared_ptr<Animated> animated = std::make_shared<Animated>();
    
    animated->animationsLength[L""] = 0.0f;
    for (UINT animationIndex = 0U; animationIndex < aiModel->mNumAnimations; animationIndex++) {
      auto animation = aiModel->mAnimations[animationIndex];
      std::string animName = animation->mName.C_Str();
      std::wstring animNameW(animName.begin(), animName.end());
      animated->animationsLength[animNameW] = static_cast<FLOAT>(aiModel->mAnimations[animationIndex]->mDuration);
    }

    XMVECTOR det;
    animated->rootNodeInverseTransform = XMMatrixInverse(&det, XMMATRIX(&aiModel->mRootNode->mTransformation.a1));

    //first build the bone table, so the bone indexes can be calculated when loading the vertexes
    BuildBonesOffsets(aiModel, animated->bonesOffsets);

    //then build the key frames for each channel and each animation
    BuildAnimationBonesKeys(aiModel, animated->animationsBonesKeys);

    //now build the nodes hierarchy
    BuildNodesHierarchy(aiModel->mRootNode, &animated->rootHierarchy, animated->multiplyNavigator);

		return animated;
  }

	void AttachAnimation(const std::shared_ptr<Renderer>& renderer, std::shared_ptr<Renderable::Renderable>& renderable, std::shared_ptr<Animated>& animated)
	{
		using namespace DeviceUtils::ConstantsBuffer;

		ConstantsBufferViewDataPtr cbvData = CreateConstantsBufferViewData(renderer, sizeof(BonesMatrices));
		animationsCBV[renderable] = cbvData;

		renderable->bonesTransformation = animated->bonesOffsets;
	}

	ConstantsBufferViewDataPtr GetAnimatedConstantBufferView(std::shared_ptr<Renderable::Renderable>& renderable)
	{
		return animationsCBV[renderable];
	}

	void WriteBoneTransformationsToConstantsBuffer(std::shared_ptr<Renderable::Renderable>& renderable, BonesTransformations& bonesTransformation, UINT backbufferIndex)
	{
		auto bonesCbv = GetAnimatedConstantBufferView(renderable);

		XMMATRIX* bones = reinterpret_cast<XMMATRIX*>(bonesCbv->mappedConstantBuffer + bonesCbv->alignedConstantBufferSize * backbufferIndex);

		for (auto it = bonesTransformation.begin(); it != bonesTransformation.end(); it++, bones++) {
			*bones = it->second;
		}
	}

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

	void TraverseMultiplycationQueue(FLOAT time, MultiplyCmdQueue& cmds, BonesKeysMap& boneKeys, BonesTransformations& bonesTransformation, BonesTransformations& bonesOffsets, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation)
	{

		auto execCmds = cmds;
		std::stack<XMMATRIX> transformation;
		transformation.push(XMMatrixIdentity());

		while (!execCmds.empty()) {
			MultiplyCmd toDo = execCmds.front();

			if (!toDo.second) {
				transformation.pop();
				execCmds.pop();
			}
			else {
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

				auto bone = bonesTransformation.find(node->name);
				if (bone != bonesTransformation.end()) {
					bone->second = XMMatrixMultiply(rootNodeInverseTransform, XMMatrixMultiply(transformation.top(), bonesOffsets[node->name]));
				}
				execCmds.pop();
			}
		}
	}

	/*
	* leave this for legacy purposes
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
	*/


}