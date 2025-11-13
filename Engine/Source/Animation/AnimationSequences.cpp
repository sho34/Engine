#include "pch.h"

#include "AnimationSequences.h"
#include <NoMath.h>
#include <NoStd.h>
#include <SimpleMath.h>

SequenceChannelElement::SequenceChannelElement(const nlohmann::json& j)
{
	frameStart = j.at("frameStart");
	frameEnd = j.at("frameEnd");
}

SequenceChannelElementAnimation::SequenceChannelElementAnimation(const nlohmann::json& j) :SequenceChannelElement(j)
{
	animation = j.at("animation");
	framesToSkipFromLeft = j.at("framesToSkipFromLeft");
	framesToSkipFromRight = j.at("framesToSkipFromRight");
}

bool SequenceChannelElementAnimation::operator==(const SequenceChannelElementAnimation& other) const
{
	return frameStart == other.frameStart && frameEnd == other.frameEnd &&
		animation == other.animation && framesToSkipFromLeft == other.framesToSkipFromLeft &&
		framesToSkipFromRight == other.framesToSkipFromRight;
}

nlohmann::json SequenceChannelElementAnimation::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart},
		{ "frameEnd" , frameEnd},
		{ "animation", animation},
		{ "framesToSkipFromLeft", framesToSkipFromLeft},
		{ "framesToSkipFromRight", framesToSkipFromRight },
	};
	return j;
}

SequenceChannelElementTransformation::SequenceChannelElementTransformation(const nlohmann::json& j) :SequenceChannelElement(j)
{
	nlohmann::json positions = j.at("position");
	for (unsigned int i = 0; i < positions.size(); i++)
	{
		unsigned int frame = positions[i][0];
		XMFLOAT3 pos = ToXMFLOAT3(positions[i][1]);
		position.push_back(std::make_tuple(frame, pos));
	}

	nlohmann::json rotations = j.at("rotation");
	for (unsigned int i = 0; i < rotations.size(); i++)
	{
		unsigned int frame = rotations[i][0];
		XMFLOAT3 rot = ToXMFLOAT3(rotations[i][1]);
		rotation.push_back(std::make_tuple(frame, rot));
	}

	nlohmann::json scales = j.at("scale");
	for (unsigned int i = 0; i < scales.size(); i++)
	{
		unsigned int frame = scales[i][0];
		XMFLOAT3 sca = ToXMFLOAT3(scales[i][1]);
		scale.push_back(std::make_tuple(frame, sca));
	}
}

bool SequenceChannelElementTransformation::operator==(const SequenceChannelElementTransformation& other) const {
	auto equalKeyFrames = [](std::vector<keyFrame> k1, std::vector<keyFrame> k2)->bool
		{
			if (k1.size() != k2.size()) return false;
			for (size_t i = 0; i < k1.size(); i++)
			{
				unsigned int f1 = std::get<0>(k1[i]);
				unsigned int f2 = std::get<0>(k2[i]);
				if (f1 != f2) return false;

				XMFLOAT3 xmf1 = std::get<1>(k1[i]);
				XMFLOAT3 xmf2 = std::get<1>(k2[i]);
				if (xmf1.x != xmf2.x || xmf1.y != xmf2.y || xmf1.z != xmf2.z) return false;
			}
			return true;
		};

	return frameStart == other.frameStart && frameEnd == other.frameEnd &&
		equalKeyFrames(position, other.position) &&
		equalKeyFrames(rotation, other.rotation) &&
		equalKeyFrames(scale, other.scale);
}

nlohmann::json SequenceChannelElementTransformation::json()
{
	auto toJsonKeyFrames = [](std::vector<keyFrame> t)
		{
			nlohmann::json arr = nlohmann::json::array();
			for (auto& k : t)
			{
				nlohmann::json karr = nlohmann::json::array();
				karr.push_back(std::get<0>(k));
				karr.push_back(FromXMFLOAT3(std::get<1>(k)));
				//[N,[F,F,F]]
				arr.push_back(karr);
			}
			return arr;
		};

	nlohmann::json j =
	{
		{ "frameStart", frameStart},
		{ "frameEnd" , frameEnd},
		{ "position", toJsonKeyFrames(position) },
		{ "rotation", toJsonKeyFrames(rotation) },
		{ "scale", toJsonKeyFrames(scale) },
	};

	return j;
}

SequenceChanelElementSoundFX::SequenceChanelElementSoundFX(const nlohmann::json& j) :SequenceChannelElement(j)
{
	sound = j.at("sound");
	framesToSkipFromLeft = j.at("framesToSkipFromLeft");
	framesToSkipFromRight = j.at("framesToSkipFromRight");
}

bool SequenceChanelElementSoundFX::operator==(const SequenceChanelElementSoundFX& other) const {
	return frameStart == other.frameStart && frameEnd == other.frameEnd &&
		sound == other.sound &&
		framesToSkipFromLeft == other.framesToSkipFromLeft &&
		framesToSkipFromRight == other.framesToSkipFromRight;
}

nlohmann::json SequenceChanelElementSoundFX::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart },
		{ "frameEnd", frameEnd },
		//{ "sound", sound()},
		{ "framesToSkipFromLeft", framesToSkipFromLeft },
		{ "framesToSkipFromRight", framesToSkipFromRight },
	};

	return j;
}

SequenceChannelElementScript::SequenceChannelElementScript(const nlohmann::json& j) :SequenceChannelElement(j)
{
	script = j.at("script");
}

bool SequenceChannelElementScript::operator==(const SequenceChannelElementScript& other) const {
	return frameStart == other.frameStart && frameEnd == other.frameEnd && script == other.script;
}

nlohmann::json SequenceChannelElementScript::json()
{
	nlohmann::json j = {
		{ "frameStart", frameStart },
		{ "frameEnd", frameEnd },
		{ "script", script },
	};
	return j;
}

ChannelElement::ChannelElement(const ChannelElement& other)
{
	type = other.type;
	switch (type)
	{
	case SCET_Animation:
	{
		animation = other.animation;
	}
	break;
	case SCET_Transformation:
	{
		transformation = other.transformation;
	}
	break;
	case SCET_SoundFX:
	{
		soundfx = other.soundfx;
	}
	break;
	case SCET_Script:
	{
		script = other.script;
	}
	break;
	}
}

ChannelElement::ChannelElement(const nlohmann::json& j)
{
	type = StrToSequenceChannelElementType.at(j.at("type"));
	switch (type)
	{
	case SCET_Animation:
	{
		animation = SequenceChannelElementAnimation(j.at("animation"));
	}
	break;
	case SCET_Transformation:
	{
		transformation = SequenceChannelElementTransformation(j.at("transformation"));
	}
	break;
	case SCET_SoundFX:
	{
		soundfx = SequenceChanelElementSoundFX(j.at("soundfx"));
	}
	break;
	case SCET_Script:
	{
		script = SequenceChannelElementScript(j.at("script"));
	}
	break;
	}
}

bool ChannelElement::InFrame(int frame)
{
	int frameStart = getFrameStart();
	int frameEnd = getFrameEnd() - 1;
	return nostd::in_between(frame, frameStart, frameEnd);
}

bool ChannelElement::ElementInFrame(int frame, bool& elementBoundFromLeft, bool& elementBoundFromRight)
{
	int frameStart = getFrameStart();
	int frameEnd = getFrameEnd() - 1;
	bool ret = nostd::in_between(frame, frameStart, frameEnd);
	if (ret)
	{
		elementBoundFromLeft = frame == frameStart;
		elementBoundFromRight = frame == frameEnd;
	}
	return ret;
}

void ChannelElement::Move(int frames, int totalFrames)
{
	switch (type)
	{
	case SCET_Animation:
	{
		if (frames < 0)
		{
			frames = -std::min(static_cast<int>(animation.frameStart), -frames);
			animation.frameStart += frames;
			animation.frameEnd += frames;
		}
		else if (frames > 0)
		{
			frames = std::min((totalFrames - static_cast<int>(animation.frameEnd)), frames);
			animation.frameStart += frames;
			animation.frameEnd += frames;
		}
	}
	break;
	case SCET_Transformation:
	{
	}
	break;
	case SCET_SoundFX:
	{
	}
	break;
	case SCET_Script:
	{
	}
	break;
	}
}

int ChannelElement::getFrameStart()
{
	switch (type)
	{
	case SCET_Animation:
	{
		return animation.frameStart + animation.framesToSkipFromLeft;
	}
	break;
	case SCET_Transformation:
	{
		return transformation.frameStart;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx.frameStart;
	}
	break;
	case SCET_Script:
	{
		return script.frameStart;
	}
	break;
	}
	return 0;
}

int ChannelElement::getFrameEnd()
{
	switch (type)
	{
	case SCET_Animation:
	{
		return animation.frameEnd - animation.framesToSkipFromRight;
	}
	break;
	case SCET_Transformation:
	{
		return transformation.frameEnd;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx.frameEnd;
	}
	break;
	case SCET_Script:
	{
		return script.frameEnd;
	}
	break;
	}
	return 0;
}

bool ChannelElement::operator==(const ChannelElement& other) const {
	if (type != other.type) return false;
	switch (type)
	{
	case SCET_Animation:
	{
		return animation == other.animation;
	}
	break;
	case SCET_Transformation:
	{
		return transformation == other.transformation;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx == other.soundfx;
	}
	break;
	case SCET_Script:
	{
		return script == other.script;
	}
	break;
	}
	return false;
}

nlohmann::json ChannelElement::json()
{
	nlohmann::json j;
	j["type"] = SequenceChannelElementTypeToStr.at(type);
	switch (type)
	{
	case SCET_Animation:
	{
		j["animation"] = animation.json();
	}
	break;
	case SCET_Transformation:
	{
		j["transformation"] = transformation.json();
	}
	break;
	case SCET_SoundFX:
	{
		j["soundfx"] = soundfx.json();
	}
	break;
	case SCET_Script:
	{
		j["script"] = script.json();
	}
	break;
	}
	return j;
}

SequenceChannel::SequenceChannel()
{
}

SequenceChannel::SequenceChannel(std::string name)
{
	this->name = name;
}

SequenceChannel::SequenceChannel(const nlohmann::json& j)
{
	name = j.at("name");
	nlohmann::json jelements = j.at("elements");
	for (size_t i = 0ULL; i < jelements.size(); i++)
	{
		nlohmann::json& element = jelements.at(i);
		elements.push_back(element);
	}
}

int SequenceChannel::GetAvailableFramesToLeft(int elementIndex)
{
	ChannelElement& element = elements.at(elementIndex);
	if (elementIndex == 0)
		return element.getFrameStart();

	ChannelElement& prevElement = elements.at(elementIndex - 1);
	return element.getFrameStart() - prevElement.getFrameEnd();
}

int SequenceChannel::GetAvailableFramesToRight(int elementIndex, int totalFrames)
{
	ChannelElement& element = elements.at(elementIndex);
	if (elementIndex == (elements.size() - 1))
		return totalFrames - element.getFrameEnd();

	ChannelElement& nextElement = elements.at(elementIndex + 1);
	return nextElement.getFrameStart() - element.getFrameEnd();
}

void SequenceChannel::MoveElement(int elementIndex, int frames, int totalFrames)
{
	if (frames < 0)
	{
		frames = std::max(frames, -GetAvailableFramesToLeft(elementIndex));
		elements.at(elementIndex).Move(frames, totalFrames);
	}
	else if (frames > 0)
	{
		frames = std::min(frames, GetAvailableFramesToRight(elementIndex, totalFrames));
		elements.at(elementIndex).Move(frames, totalFrames);
	}
}

void SequenceChannel::EraseElement(int elementIndex)
{
	nostd::vector_erase_index(elements, elementIndex);
}

void SequenceChannel::SplitElement(int elementIndex, int frame)
{
	ChannelElement& elementToRight = elements.at(elementIndex);
	int frameStart = elementToRight.animation.frameStart;
	int frameEnd = elementToRight.animation.frameEnd;
	int leftItemPadRight = frameEnd - frame;
	int rightItemPadLeft = frame - frameStart;

	ChannelElement elementToLeft(elementToRight);
	elementToRight.animation.framesToSkipFromLeft = rightItemPadLeft;
	elementToLeft.animation.framesToSkipFromRight = leftItemPadRight;

	elements.insert(elements.begin() + elementIndex, elementToLeft);
}

nlohmann::json SequenceChannel::json()
{
	nlohmann::json j = nlohmann::json({});
	j["name"] = name;
	j["elements"] = nlohmann::json::array();
	for (auto& e : elements)
	{
		j["elements"].push_back(e.json());
	}
	return j;
}

bool SequenceChannel::operator==(const SequenceChannel& other) const {
	if (name != other.name) return false;
	return elements == other.elements;
}

Sequence::Sequence()
{
	framesPerSecond = 60;
	totalFrames = 60;
	loop = false;
}

Sequence::Sequence(nlohmann::json j)
{
	framesPerSecond = j.at("framesPerSecond");
	totalFrames = j.at("totalFrames");
	loop = j.at("loop");
	for (size_t i = 0ULL; i < j.at("sequenceChannels").size(); i++)
	{
		sequenceChannels.push_back(j.at("sequenceChannels").at(i));
	}
}

nlohmann::json Sequence::json()
{
	nlohmann::json seqChannels = nlohmann::json::array();
	for (auto& seqChannel : sequenceChannels)
	{
		seqChannels.push_back(seqChannel.json());
	}
	nlohmann::json j(
		{
			{ "framesPerSecond", framesPerSecond },
		{ "totalFrames", totalFrames },
		{ "loop", loop },
		{ "sequenceChannels", seqChannels }
		}
	);
	return j;
}

bool Sequence::operator==(const Sequence& other) const {

	if (framesPerSecond != other.framesPerSecond) return false;
	if (totalFrames != other.totalFrames) return false;
	if (loop != other.loop) return false;

	return std::equal(sequenceChannels.begin(), sequenceChannels.end(), other.sequenceChannels.begin());
}

AnimationSequences::AnimationSequences(nlohmann::json j)
{
	for (nlohmann::json::iterator it = j.begin(); it != j.end(); it++)
	{
		sequences[it.key()] = Sequence(it.value());
	}
}

nlohmann::json AnimationSequences::json()
{
	nlohmann::json j;
	for (auto& [name, sequence] : sequences)
	{
		j[name] = sequence.json();
	}
	return j;
}
