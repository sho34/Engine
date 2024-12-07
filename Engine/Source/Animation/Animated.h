#pragma once

#include "../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

namespace Renderable { struct Renderable; };
namespace Templates::Model3D { struct Model3D; };
struct aiScene;
namespace Animation {

	static const std::wstring AnimationConstantBufferName = L"animation";
	static const UINT MAX_BONES = 256U;
	typedef XMMATRIX BonesMatrices[MAX_BONES];

	using namespace DirectX;

	typedef std::map<std::wstring, FLOAT> AnimationLengthMap;
	typedef std::map<std::wstring, XMMATRIX> BonesTransformations;

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

	struct Animated
	{
		AnimationLengthMap animationsLength;
		BonesTransformations bonesOffsets;
		XMMATRIX rootNodeInverseTransform;

		AnimationBonesKeys animationsBonesKeys;
		HierarchyNode rootHierarchy;
		MultiplyCmdQueue multiplyNavigator;
	};

	std::shared_ptr<Animated> CreateAnimatedFromAssimp(const aiScene* aiModel);

	void AttachAnimation(const std::shared_ptr<Renderer>& renderer, std::shared_ptr<Renderable::Renderable>& renderable, std::shared_ptr<Animated>& animated);
	ConstantsBufferViewDataPtr GetAnimatedConstantBufferView(std::shared_ptr<Renderable::Renderable>& renderable);
	void WriteBoneTransformationsToConstantsBuffer(std::shared_ptr<Renderable::Renderable>& renderable, BonesTransformations& bonesTransformation, UINT backbufferIndex);
	
	void TraverseMultiplycationQueue(FLOAT time, MultiplyCmdQueue& cmds, BonesKeysMap& boneKeys, BonesTransformations& bonesTransformation, BonesTransformations& bonesOffsets, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation);
}

