#pragma once

#include <queue>
#include <map>
#include <DirectXMath.h>
#include <assimp/scene.h>
#include "../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"
#include <memory>

namespace Scene { struct Renderable; };
struct aiScene;

using namespace DirectX;

namespace Animation
{
	static const std::string AnimationConstantBufferName = "animation";
	static const unsigned int MAX_BONES = 1024U;
	typedef XMMATRIX BonesMatrices[MAX_BONES];

	using namespace DirectX;
	using namespace DeviceUtils;
	using namespace Scene;

	typedef std::map<std::string, float> AnimationLengthMap;
	typedef std::map<std::string, XMMATRIX> BonesTransformations;

	struct HierarchyNode
	{
		std::string name;
		XMMATRIX transformation;
		unsigned int numChildren = 0;
		HierarchyNode* children = nullptr;
	};

	struct KeyFrame
	{
		float time;
		XMVECTOR key;
	};

	struct BoneKeys
	{
		std::vector<KeyFrame> positions;
		std::vector<KeyFrame> scaling;
		std::vector<KeyFrame> rotation;
	};
	typedef std::map<std::string, BoneKeys> BonesKeysMap;
	typedef std::map<std::string, BonesKeysMap> AnimationBonesKeys;

	typedef std::pair<HierarchyNode*, bool> MultiplyCmd;
	typedef std::queue<MultiplyCmd> MultiplyCmdQueue;

	struct Animated
	{
		AnimationLengthMap animationsLength;
		BonesTransformations bonesOffsets;
		XMMATRIX rootNodeInverseTransform;

		AnimationBonesKeys animationsBonesKeys;
		HierarchyNode rootHierarchy;
		MultiplyCmdQueue multiplyNavigator;
	};

	void BuildBonesOffsets(const aiScene* aiModel, BonesTransformations& bonesOffsets);
	void BuildAnimationBonesKeys(const aiScene* model, AnimationBonesKeys& animationBonesKeys);
	void BuildNodesHierarchy(aiNode* node, HierarchyNode* nodeInHierarchy, MultiplyCmdQueue& multiplyNavigator);
	std::shared_ptr<Animated> CreateAnimatedFromAssimp(const aiScene* aiModel);
	void DestroyAnimated();
	void DestroyNodesHierarchy(HierarchyNode* node);

	void AttachAnimation(const std::shared_ptr<Renderable>& renderable, std::shared_ptr<Animated>& animated);
	std::shared_ptr<ConstantsBuffer> GetAnimatedConstantsBuffer(const std::shared_ptr<Renderable>& renderable);
	void WriteBoneTransformationsToConstantsBuffer(const std::shared_ptr<Renderable>& renderable, BonesTransformations& bonesTransformation, unsigned int backbufferIndex);

	void TraverseMultiplycationQueue(float time, std::string currentAnimation, std::shared_ptr<Animated>& animations, BonesTransformations& bonesTransformation);
	void TraverseMultiplycationQueue(float time, MultiplyCmdQueue& cmds, BonesKeysMap& boneKeys, BonesTransformations& bonesTransformation, BonesTransformations& bonesOffsets, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation);

	void CreateBoundingBoxComputeResource(CComPtr<ID3D12Resource>& boundingBoxResource, CComPtr<ID3D12Resource>& readBackBoundingBoxResource, CD3DX12_CPU_DESCRIPTOR_HANDLE& animableBoundingBoxCpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& animableBoundingBoxGpuHandle);
}

