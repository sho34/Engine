#pragma once
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

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

struct SequenceChannelElement
{
	int frameStart;
	int frameEnd;

	SequenceChannelElement() {}
	SequenceChannelElement(const nlohmann::json& j);
};

struct SequenceChannelElementAnimation : SequenceChannelElement
{
	std::string animation;
	int framesToSkipFromLeft;
	int framesToSkipFromRight;

	SequenceChannelElementAnimation() {}
	SequenceChannelElementAnimation(const nlohmann::json& j);

	bool operator==(const SequenceChannelElementAnimation& other) const;

	nlohmann::json json();
};

struct SequenceChannelElementTransformation : SequenceChannelElement
{
	typedef std::tuple<unsigned int, XMFLOAT3> keyFrame;
	std::vector<keyFrame> position;
	std::vector<keyFrame> rotation;
	std::vector<keyFrame> scale;

	SequenceChannelElementTransformation() {}
	SequenceChannelElementTransformation(const nlohmann::json& j);

	bool operator==(const SequenceChannelElementTransformation& other) const;

	nlohmann::json json();
};

struct SequenceChanelElementSoundFX : SequenceChannelElement
{
	SoundJsonUUID sound;
	int framesToSkipFromLeft;
	int framesToSkipFromRight;

	SequenceChanelElementSoundFX() {}
	SequenceChanelElementSoundFX(const nlohmann::json& j);

	bool operator==(const SequenceChanelElementSoundFX& other) const;

	nlohmann::json json();
};

struct SequenceChannelElementScript : SequenceChannelElement
{
	std::string script;

	SequenceChannelElementScript() {}
	SequenceChannelElementScript(const nlohmann::json& j);

	bool operator==(const SequenceChannelElementScript& other) const;

	nlohmann::json json();
};

struct ChannelElement
{
	SequenceChannelElementType type;
	SequenceChannelElementAnimation animation;
	SequenceChannelElementTransformation transformation;
	SequenceChanelElementSoundFX soundfx;
	SequenceChannelElementScript script;

	ChannelElement() {}
	ChannelElement(const ChannelElement& other);
	ChannelElement(const nlohmann::json& j);
	~ChannelElement() {}

	bool InFrame(int frame);
	bool ElementInFrame(int frame, bool& elementBoundFromLeft, bool& elementBoundFromRight);
	void Move(int frames, int totalFrames);

	int getFrameStart();
	int getFrameEnd();

	bool operator==(const ChannelElement& other) const;

	nlohmann::json json();
};

struct SequenceChannel
{
	std::string name;
	std::vector<ChannelElement> elements;

	SequenceChannel();

	SequenceChannel(std::string name);

	SequenceChannel(const nlohmann::json& j);

	int GetAvailableFramesToLeft(int elementIndex);
	int GetAvailableFramesToRight(int elementIndex, int totalFrames);
	void MoveElement(int elementIndex, int frames, int totalFrames);
	void EraseElement(int elementIndex);
	void SplitElement(int elementIndex, int frame);

	nlohmann::json json();

	bool operator==(const SequenceChannel& other) const;
};

struct Sequence
{
	int framesPerSecond;
	int totalFrames;
	bool loop;
	std::vector<SequenceChannel> sequenceChannels;

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
	std::map<std::string, Sequence> sequences;

	AnimationSequences() {}

	AnimationSequences(nlohmann::json j);

	nlohmann::json json();
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
