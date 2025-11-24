#pragma once
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include <DirectXMath.h>

enum SequenceChannelElementType
{
	SCET_Animation,
	SCET_Transformation,
	SCET_SoundFX,
	SCET_Script
};

static inline std::unordered_map<SequenceChannelElementType, std::string> SequenceChannelElementTypeToStr =
{
	{ SCET_Animation, "animation" },
	{ SCET_Transformation, "transformation" },
	{ SCET_SoundFX, "soundfx" },
	{ SCET_Script, "script" },
};

static inline std::unordered_map<std::string, SequenceChannelElementType> StrToSequenceChannelElementType =
{
	{ "animation", SCET_Animation },
	{ "transformation", SCET_Transformation },
	{ "soundfx", SCET_SoundFX },
	{ "script", SCET_Script },
};


enum Easing {
	Easing_Linear,
	Easing_Sine_Ease_In,
	Easing_Sine_Ease_Out,
	Easing_Sine_Ease_In_Out
};

static inline std::unordered_map<Easing, std::string> EasingToString =
{
	{ Easing_Linear,"Linear"},
	{ Easing_Sine_Ease_In,"Sine_Ease_In"},
	{ Easing_Sine_Ease_Out,"Sine_Ease_Out"},
	{ Easing_Sine_Ease_In_Out,"Sine_Ease_In_Out"},
};

static inline std::unordered_map<std::string, Easing> StringToEasing =
{
	{ "Linear", Easing_Linear},
	{ "Sine_Ease_In", Easing_Sine_Ease_In},
	{ "Sine_Ease_Out", Easing_Sine_Ease_Out},
	{ "Sine_Ease_In_Out", Easing_Sine_Ease_In_Out},
};

struct TransformationKeyFrame
{
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	Easing easing = Easing_Linear;

	XMMATRIX ToMatrix();

	bool operator==(const TransformationKeyFrame& other) const;
};

struct SequenceChannelElement
{
	SequenceChannelElement();
	SequenceChannelElement(const nlohmann::json& j);
	void ExpandLeftBorder(int numFrames);
	void ExpandRightBorder(int numFrames);

	int frameStart;
	int frameEnd;
};

struct SequenceChannelElementAnimation : SequenceChannelElement
{
	SequenceChannelElementAnimation();
	SequenceChannelElementAnimation(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementAnimation& other) const;
	nlohmann::json json();
	int GetFrameStart();
	int GetFrameEnd();
	float GetTimeAtFrame(int frame);

	std::string animation;
	float startTime;
	float endTime;
};

struct SequenceChannelElementTransformation : SequenceChannelElement
{
	SequenceChannelElementTransformation() :SequenceChannelElement() {}
	SequenceChannelElementTransformation(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementTransformation& other) const;
	nlohmann::json json();
	TransformationKeyFrame GetKeyFrameBeforeFrame(int frame);
	XMMATRIX GetTransformationInFrame(int frame);
	XMMATRIX GetTransformationBeforeFrame(int frame);
	bool HasKeyframeBeforeFrame(int frame);
	bool HasKeyframeAfterFrame(int frame);
	std::tuple<int, TransformationKeyFrame, int, TransformationKeyFrame> GetKeyframesBetweenFrame(int frame);
	XMMATRIX InterpolateKeyframes(TransformationKeyFrame keyA, TransformationKeyFrame keyB, int frameAfterA, int framesBetweenKeyframes);

	std::unordered_map<int, TransformationKeyFrame> keyFrames;
};

struct SequenceChannelElementSoundFX : SequenceChannelElement
{
	SequenceChannelElementSoundFX();
	SequenceChannelElementSoundFX(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementSoundFX& other) const;
	nlohmann::json json();
	int GetFrameStart();
	int GetFrameEnd();

	SoundJsonUUID sound;
	float volume;
	bool loop;
};

struct SequenceChannelElementScript : SequenceChannelElement
{
	SequenceChannelElementScript();
	SequenceChannelElementScript(const nlohmann::json& j);
	bool operator==(const SequenceChannelElementScript& other) const;
	nlohmann::json json();

	std::string script;
};

struct ChannelElement
{
	ChannelElement() {}
	ChannelElement(const ChannelElement& other);
	ChannelElement(const nlohmann::json& j);
	~ChannelElement() {}

	bool InFrame(int frame);
	bool ElementInFrame(int frame, bool& elementBoundFromLeft, bool& elementBoundFromRight);
	void Move(int frames, int totalFrames);
	std::tuple<ChannelElement, ChannelElement> Split(int frame);
	void ExpandLeftBorder(int numFrames);
	void ExpandRightBorder(int numFrames);
	SequenceChannelElement* GetElementPointer();

	int GetFrameStart();
	int GetFrameEnd();

	bool operator==(const ChannelElement& other) const;

	nlohmann::json json();

	SequenceChannelElementType type;
	SequenceChannelElementAnimation animation;
	SequenceChannelElementTransformation transformation;
	SequenceChannelElementSoundFX soundfx;
	SequenceChannelElementScript script;
};

struct SequenceChannel
{
	SequenceChannel();

	SequenceChannel(std::string name);

	SequenceChannel(const nlohmann::json& j);

	bool ChannelHasElementAtFrame(int frame);
	int GetAvailableFramesToLeft(int elementIndex);
	int GetAvailableFramesToRight(int elementIndex, int totalFrames);
	int GetFirstElementIndexBetweenFrames(int frameStart, int frameEnd);
	int GetElementIndexBeforeFrame(int frame);
	SequenceChannelElementAnimation* GetAnimationElementAtFrame(int frame);
	SequenceChannelElementTransformation* GetTransformationElementAtFrame(int frame);
	TransformationKeyFrame* GetTransformationKeyframe(int frame);
	SequenceChannelElementSoundFX* GetSoundFXToCreateAtFrame(int frame);
	SequenceChannelElementScript* GetScriptToRunAtFrame(int frame);

	void InsertChannelElement(ChannelElement element, int& totalFrames);
	void MoveElement(int elementIndex, int frames, int totalFrames);
	void DragElementLeftBoundary(int elementIndex, int frames, int totalFrames);
	void DragElementRightBoundary(int elementIndex, int frames, int totalFrames);
	void EraseElement(int elementIndex);
	void SplitElement(int elementIndex, int frame);
	bool FrameHasElement(int frame, bool& leftBounded, bool& rightBounded);
	bool FrameHasTransformationKeyframe(int frame);
	void EraseElementInFrame(int frame);
	void SplitElementInFrame(int frame);

	nlohmann::json json();

	bool operator==(const SequenceChannel& other) const;

	std::string name;
	//elements should be sorted ok?
	std::vector<ChannelElement> elements;
};

struct Sequence
{
	Sequence();

	/*
	Sequence(std::string animation, int numFrames) : Sequence()
	{
		SequenceChannel seqC;
		seqC.animation = animation;
		seqC.frameEnd = numFrames - 1;
		totalFrames = numFrames;
		channels.push_back(seqC);
	}
	*/

	Sequence(nlohmann::json j);
	nlohmann::json json();
	bool operator==(const Sequence& other) const;

	std::string GetAnimationNameAtFrame(int frame);
	SequenceChannelElementAnimation* GetAnimationElementAtFrame(int frame);
	XMMATRIX GetTransformationAtFrame(int frame);
	void CreateSoundFXsAtFrame(int frame);
	void RunScriptAtFrame(int frame, RenderableUUID renderable);

	int framesPerSecond;
	int totalFrames;
	//bool loop;
	std::vector<SequenceChannel> sequenceChannels;
};

inline static Sequence ToSequence(nlohmann::json j)
{
	Sequence seq(j);
	return seq;
}

inline static nlohmann::json FromSequence(Sequence s)
{
	return s.json();
}

struct AnimationSequences
{
	AnimationSequences() {}
	AnimationSequences(nlohmann::json j);
	nlohmann::json json();

	std::map<std::string, Sequence> sequences;
};

inline static AnimationSequences ToAnimationSequences(nlohmann::json j)
{
	AnimationSequences seq(j);
	return seq;
}

inline static nlohmann::json FromAnimationSequences(AnimationSequences s)
{
	return s.json();
}

struct SequencePlayer
{
	Sequence* sequence;
	float time;
	int runningFrame;
	int currentFrame;
	bool loop;
	bool newSequence;
	std::set<int> runnedFrames;
	RenderableUUID renderable;

	SequencePlayer();
	SequencePlayer(Sequence* seq, JUUID uuid);
	void SetSequence(Sequence* seq, JUUID uuid);
	void Step(float dt);
	void SetTime(float t);
	void StepFrame(int df);
	void SetFrame(int frame, bool runningPlayer = true);
	void ApplyFrameValues(RenderableUUID renderable);
	void CreateFrameSoundFXs(int frame);
	void ExecuteFrameScripts(int frame);
	void ResetFrames();
};