#pragma once

#include "../Renderer/DeviceUtils/ConstantsBuffer/ConstantsBuffer.h"

namespace Scene::Renderable { struct Renderable; };
typedef std::shared_ptr<Scene::Renderable::Renderable> RenderablePtr;
namespace Templates::Model3D { struct Model3D; };
typedef std::shared_ptr<Templates::Model3D::Model3D> Model3DPtr;
struct aiScene;

namespace Animation {

	static const std::string AnimationConstantBufferName = "animation";
	static const UINT MAX_BONES = 256U;
	typedef XMMATRIX BonesMatrices[MAX_BONES];

	using namespace DirectX;

	typedef std::map<std::string, FLOAT> AnimationLengthMap;
	typedef std::map<std::string, XMMATRIX> BonesTransformations;

	struct HierarchyNode {
		std::string name;
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

	std::shared_ptr<Animated> CreateAnimatedFromAssimp(const aiScene* aiModel);
	void DestroyAnimated();
	void DestroyNodesHierarchy(HierarchyNode* node);

	void AttachAnimation(const std::shared_ptr<Renderer>& renderer, RenderablePtr& renderable, std::shared_ptr<Animated>& animated);
	ConstantsBufferViewDataPtr GetAnimatedConstantBufferView(RenderablePtr& renderable);
	void WriteBoneTransformationsToConstantsBuffer(RenderablePtr& renderable, BonesTransformations& bonesTransformation, UINT backbufferIndex);
	
	void TraverseMultiplycationQueue(FLOAT time, MultiplyCmdQueue& cmds, BonesKeysMap& boneKeys, BonesTransformations& bonesTransformation, BonesTransformations& bonesOffsets, XMMATRIX& rootNodeInverseTransform, XMMATRIX parentTransformation);
}

