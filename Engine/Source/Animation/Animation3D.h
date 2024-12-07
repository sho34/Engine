#pragma once
#include <assimp/scene.h>

using namespace DirectX;

struct BoneInfo {
  XMMATRIX offset;
  XMMATRIX transformation;
};
typedef std::map<std::wstring, BoneInfo> BoneInfoMap;

struct HierarchyNode {
	std::wstring name;
	XMMATRIX transformation;
	UINT numChildren = 0;
	HierarchyNode* children = nullptr;
};

struct KeyFrame {
	FLOAT time;
	XMVECTOR key;
};

struct BoneKeys {
	std::vector<KeyFrame> positions;
	std::vector<KeyFrame> scaling;
	std::vector<KeyFrame> rotation;
};
typedef std::map<std::wstring, BoneKeys> BonesKeysMap;
typedef std::map<std::wstring, BonesKeysMap> AnimationBonesKeys;

typedef std::pair<HierarchyNode*, bool> MultiplyCmd;
typedef std::queue<MultiplyCmd> MultiplyCmdQueue;

void BuildBoneInfo(const aiScene* aiModel, BoneInfoMap& boneInfo);
void BuildAnimationsBonesKeys(const aiScene* model, AnimationBonesKeys& animationBonesKeys);
void BuildNodesHierarchy(aiNode* node, HierarchyNode* nodeInHierarchy, MultiplyCmdQueue& multiplyNavigator);
void DestroyNodesHierarchy(HierarchyNode* node);

/**
XMMATRIX InterpolateKeys(XMMATRIX(XM_CALLCONV* ToMatrix)(XMVECTOR), XMVECTOR(XM_CALLCONV* Interpolator)(XMVECTOR, XMVECTOR, float), FLOAT time, std::vector<KeyFrame>& keyFrames);
void TraverseNodeHierarchy(FLOAT time, HierarchyNode* node, std::map<std::string, BoneKeys>& boneKeys, std::map<std::string, BoneInfo>& boneInfo, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation);
void TraverseMultiplycationQueue(FLOAT time, std::queue<multiplyCmd>& cmds, std::map<std::string, BoneKeys>& boneKeys, std::map<std::string, BoneInfo>& boneInfo, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation);

template <typename Animated>
void NextAnimation(Animated* animated) {
	auto animation = animated->animationsChannelsKeys.find(animated->currentAnimation);
	animation++;
	if (animation == animated->animationsChannelsKeys.end()) {
		animation = animated->animationsChannelsKeys.begin();
	}
	animated->currentAnimation = animation->first;
	animated->currentAnimationTime = 0.0f;
}

template <typename Animated>
void PreviousAnimation(Animated* animated) {
	auto animation = animated->animationsChannelsKeys.find(animated->currentAnimation);
	if (animation == animated->animationsChannelsKeys.begin()) {
		animation = animated->animationsChannelsKeys.end();
	}
	animation--;
	animated->currentAnimation = animation->first;
	animated->currentAnimationTime = 0.0f;
}
*/