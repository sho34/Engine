#include "pch.h"

#include "AnimationSequences.h"
#include <NoMath.h>
#include <NoStd.h>
#include <SimpleMath.h>
#include <Sound/Sound.h>
#include <Scripting.h>

XMMATRIX TransformationKeyFrame::ToMatrix()
{
	float roll, pitch, yaw;
	pitch = rotation.x; yaw = rotation.y; roll = rotation.z;
	XMVECTOR rotQ = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
	XMMATRIX rotationM = XMMatrixRotationQuaternion(rotQ);
	XMMATRIX scaleM = XMMatrixScalingFromVector({ scale.x, scale.y, scale.z });
	XMMATRIX positionM = XMMatrixTranslationFromVector({ position.x, position.y, position.z });
	return XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);
}

bool TransformationKeyFrame::operator==(const TransformationKeyFrame& other) const
{
	return
		position.x == other.position.x
		&& position.y == other.position.y
		&& position.z == other.position.z
		&& rotation.x == other.rotation.x
		&& rotation.y == other.rotation.y
		&& rotation.z == other.rotation.z
		&& scale.x == other.scale.x
		&& scale.y == other.scale.y
		&& scale.z == other.scale.z
		&& easing == other.easing;
}

SequenceChannelElement::SequenceChannelElement()
{
	frameStart = 0;
	frameEnd = 0;
}

SequenceChannelElement::SequenceChannelElement(const nlohmann::json& j)
{
	frameStart = j.at("frameStart");
	frameEnd = j.at("frameEnd");
}

void SequenceChannelElement::ExpandLeftBorder(int numFrames)
{
	frameStart += numFrames;
}

void SequenceChannelElement::ExpandRightBorder(int numFrames)
{
	frameEnd += numFrames;
}

SequenceChannelElementAnimation::SequenceChannelElementAnimation() :SequenceChannelElement()
{
	animation = "";
	startTime = 0.0f;
	endTime = 0.0f;
}

SequenceChannelElementAnimation::SequenceChannelElementAnimation(const nlohmann::json& j) :SequenceChannelElement(j)
{
	animation = j.at("animation");
	startTime = j.at("startTime");
	endTime = j.at("endTime");
}

bool SequenceChannelElementAnimation::operator==(const SequenceChannelElementAnimation& other) const
{
	return
		frameStart == other.frameStart
		&& frameEnd == other.frameEnd
		&& startTime == other.startTime
		&& endTime == other.endTime;
}

nlohmann::json SequenceChannelElementAnimation::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart },
		{ "frameEnd" , frameEnd },
		{ "animation", animation },
		{ "startTime", startTime },
		{ "endTime", endTime },
	};
	return j;
}

int SequenceChannelElementAnimation::GetFrameStart()
{
	return frameStart;
}
int SequenceChannelElementAnimation::GetFrameEnd()
{
	return frameEnd;
}

float SequenceChannelElementAnimation::GetTimeAtFrame(int frame)
{
	frame = std::clamp(frame, frameStart, frameEnd);
	float t = static_cast<float>(frame - frameStart) / static_cast<float>(frameEnd - frameStart);
	return startTime + t * (endTime - startTime);
}

SequenceChannelElementTransformation::SequenceChannelElementTransformation(const nlohmann::json& j) :SequenceChannelElement(j)
{
	nlohmann::json keyframes = j.at("keyframes");
	for (int i = 0; i < keyframes.size(); i++)
	{
		nlohmann::json keyframe = keyframes.at(i);
		int frame = keyframe.at("frame");
		TransformationKeyFrame tkey;
		tkey.position = ToXMFLOAT3(keyframe.at("position"));
		tkey.rotation = ToXMFLOAT3(keyframe.at("rotation"));
		tkey.scale = ToXMFLOAT3(keyframe.at("scale"));
		tkey.easing = StringToEasing.at(keyframe.at("easing"));
		keyFrames.insert_or_assign(frame, tkey);
	}
}

bool SequenceChannelElementTransformation::operator==(const SequenceChannelElementTransformation& other) const {
	return keyFrames == other.keyFrames;
}

nlohmann::json SequenceChannelElementTransformation::json()
{
	auto toJsonKeyFrames = [](std::unordered_map<int, TransformationKeyFrame> t)
		{
			nlohmann::json arr = nlohmann::json::array();
			for (auto& [frame, keyframe] : t)
			{
				nlohmann::json jk = {
					{ "frame", frame },
					{ "position", FromXMFLOAT3(keyframe.position) },
					{ "rotation", FromXMFLOAT3(keyframe.rotation) },
					{ "scale", FromXMFLOAT3(keyframe.scale) },
					{ "easing", EasingToString.at(keyframe.easing) },
				};
				arr.push_back(jk);
			}
			return arr;
		};

	nlohmann::json j =
	{
		{ "frameStart", frameStart},
		{ "frameEnd" , frameEnd},
		{ "keyframes", toJsonKeyFrames(keyFrames) }
	};

	return j;
}

TransformationKeyFrame SequenceChannelElementTransformation::GetKeyFrameBeforeFrame(int frame)
{
	for (int i = frame - 1; i >= 0; i--)
	{
		if (keyFrames.contains(i))
			return keyFrames.at(i);
	}
	return TransformationKeyFrame();
}

XMMATRIX SequenceChannelElementTransformation::GetTransformationInFrame(int frame)
{
	if (keyFrames.contains(frame)) return keyFrames.at(frame).ToMatrix();
	if (!HasKeyframeBeforeFrame(frame)) return XMMatrixIdentity();
	if (!HasKeyframeAfterFrame(frame)) return GetTransformationBeforeFrame(frame);
	auto [frameA, keyframeA, frameB, keyframeB] = GetKeyframesBetweenFrame(frame);
	return InterpolateKeyframes(keyframeA, keyframeB, frame - frameA, frameB - frameA);
}

XMMATRIX SequenceChannelElementTransformation::GetTransformationBeforeFrame(int frame)
{
	return GetKeyFrameBeforeFrame(frame).ToMatrix();
}

bool SequenceChannelElementTransformation::HasKeyframeBeforeFrame(int frame)
{
	for (auto& [k, _] : keyFrames)
	{
		if (k < frame) return true;
	}
	return false;
}

bool SequenceChannelElementTransformation::HasKeyframeAfterFrame(int frame)
{
	for (auto& [k, _] : keyFrames)
	{
		if (k > frame) return true;
	}
	return false;
}

std::tuple<int, TransformationKeyFrame, int, TransformationKeyFrame> SequenceChannelElementTransformation::GetKeyframesBetweenFrame(int frame)
{
	int leftMinDiff = -1;
	int rightMinDiff = -1;
	for (auto& [k, v] : keyFrames)
	{
		if (k < frame)
			leftMinDiff = (leftMinDiff == -1) ? (frame - k) : std::min(leftMinDiff, frame - k);
		if (k > frame)
			rightMinDiff = (rightMinDiff == -1) ? (k - frame) : std::min(rightMinDiff, k - frame);
	}
	int leftFrame = frame - leftMinDiff;
	int rightFrame = frame + rightMinDiff;
	return std::make_tuple(leftFrame, keyFrames.at(leftFrame), rightFrame, keyFrames.at(rightFrame));
}

XMMATRIX SequenceChannelElementTransformation::InterpolateKeyframes(TransformationKeyFrame keyA, TransformationKeyFrame keyB, int frameAfterA, int framesBetweenKeyframes)
{
	std::unordered_map<Easing, std::function<float(float)>> ease =
	{
		{ Easing_Linear, [](float t) { return t; } },
		{ Easing_Sine_Ease_In, [](float t) { return static_cast<float>(-cos(t * M_PI_2) + 1.0f); } },
		{ Easing_Sine_Ease_Out,[](float t) { return static_cast<float>(sin(t * M_PI_2)); } },
		{ Easing_Sine_Ease_In_Out,[](float t) { return static_cast<float>(-0.5f * (cos(M_PI * t) - 1.0f)); } }
	};

	float t = ease.at(keyA.easing)(static_cast<float>(frameAfterA) / static_cast<float>(framesBetweenKeyframes));

	XMVECTOR pA({ keyA.position.x,keyA.position.y,keyA.position.z });
	XMVECTOR pB({ keyB.position.x,keyB.position.y,keyB.position.z });
	XMVECTOR pt = XMVectorLerp(pA, pB, t);

	float rollA, pitchA, yawA;
	float rollB, pitchB, yawB;
	pitchA = keyA.rotation.x; yawA = keyA.rotation.y; rollA = keyA.rotation.z;
	pitchB = keyB.rotation.x; yawB = keyB.rotation.y; rollB = keyB.rotation.z;
	XMVECTOR rA = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitchA), XMConvertToRadians(yawA), XMConvertToRadians(rollA));
	XMVECTOR rB = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitchB), XMConvertToRadians(yawB), XMConvertToRadians(rollB));
	XMVECTOR rt = XMQuaternionSlerp(rA, rB, t);

	XMVECTOR sA({ keyA.scale.x,keyA.scale.y,keyA.scale.z });
	XMVECTOR sB({ keyB.scale.x,keyB.scale.y,keyB.scale.z });
	XMVECTOR st = XMVectorLerp(sA, sB, t);

	XMMATRIX rotationM = XMMatrixRotationQuaternion(rt);
	XMMATRIX scaleM = XMMatrixScalingFromVector(st);
	XMMATRIX positionM = XMMatrixTranslationFromVector(pt);
	return XMMatrixMultiply(XMMatrixMultiply(scaleM, rotationM), positionM);
}

SequenceChannelElementSoundFX::SequenceChannelElementSoundFX() :SequenceChannelElement()
{
	sound = "";
	volume = 1.0f;
	loop = false;
}

SequenceChannelElementSoundFX::SequenceChannelElementSoundFX(const nlohmann::json& j) :SequenceChannelElement(j)
{
	sound = j.at("sound");
	volume = j.at("volume");
	loop = j.at("loop");
}

bool SequenceChannelElementSoundFX::operator==(const SequenceChannelElementSoundFX& other) const {
	return frameStart == other.frameStart && frameEnd == other.frameEnd &&
		sound == other.sound && volume == other.volume && loop == other.loop;
}

nlohmann::json SequenceChannelElementSoundFX::json()
{
	nlohmann::json j =
	{
		{ "frameStart", frameStart },
		{ "frameEnd", frameEnd },
		{ "sound", sound()},
		{ "volume", volume },
		{ "loop", loop }
	};

	return j;
}

int SequenceChannelElementSoundFX::GetFrameStart()
{
	return frameStart;
}
int SequenceChannelElementSoundFX::GetFrameEnd()
{
	return frameEnd;
}

SequenceChannelElementScript::SequenceChannelElementScript() :SequenceChannelElement()
{
	script = "";
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
		soundfx = SequenceChannelElementSoundFX(j.at("soundfx"));
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
	int frameStart = GetFrameStart();
	int frameEnd = GetFrameEnd() - 1;
	return nostd::in_between(frame, frameStart, frameEnd);
}

bool ChannelElement::ElementInFrame(int frame, bool& elementBoundFromLeft, bool& elementBoundFromRight)
{
	int frameStart = GetFrameStart();
	int frameEnd = GetFrameEnd();
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
	SequenceChannelElement* element = GetElementPointer();

	if (frames < 0)
	{
		frames = -std::min(static_cast<int>(GetFrameStart()), -frames);
		element->frameStart += frames;
		element->frameEnd += frames;
	}
	else if (frames > 0)
	{
		frames = std::min((totalFrames - static_cast<int>(GetFrameEnd())), frames);
		element->frameStart += frames;
		element->frameEnd += frames;
	}
	if (type == SCET_Transformation)
	{
		std::unordered_map<int, TransformationKeyFrame> newKeys;
		for (auto& [k, v] : transformation.keyFrames)
		{
			newKeys.insert_or_assign(k + frames, v);
		}
		transformation.keyFrames = newKeys;
	}
}

std::tuple<ChannelElement, ChannelElement> ChannelElement::Split(int frame)
{
	std::tuple<ChannelElement, ChannelElement> elements;
	auto& [left, right] = elements;

	left.type = type;
	right.type = type;

	SequenceChannelElement* leftPtr = left.GetElementPointer();
	SequenceChannelElement* rightPtr = right.GetElementPointer();

	leftPtr->frameStart = GetFrameStart();
	rightPtr->frameEnd = GetFrameEnd();

	leftPtr->frameEnd = frame - 1;
	rightPtr->frameStart = frame;

	switch (type)
	{
	case SCET_Animation:
	{
		left.animation.animation = animation.animation;
		right.animation.animation = animation.animation;

		left.animation.startTime = animation.startTime;
		right.animation.endTime = animation.endTime;

		float t = static_cast<float>(frame - animation.frameStart) / static_cast<float>(animation.frameEnd - animation.frameStart);
		left.animation.endTime = animation.startTime + t * (animation.endTime - animation.startTime);
		right.animation.startTime = left.animation.endTime;
	}
	break;
	case SCET_Transformation:
	{
		std::copy_if(
			transformation.keyFrames.begin(),
			transformation.keyFrames.end(),
			std::inserter(left.transformation.keyFrames, left.transformation.keyFrames.begin()),
			[frame](const std::pair<int, TransformationKeyFrame>& p) {
				return p.first < frame;
			}
		);
		std::copy_if(
			transformation.keyFrames.begin(),
			transformation.keyFrames.end(),
			std::inserter(right.transformation.keyFrames, right.transformation.keyFrames.begin()),
			[frame](const std::pair<int, TransformationKeyFrame>& p) {
				return p.first >= frame;
			}
		);
	}
	break;
	case SCET_SoundFX:
	{
		left.soundfx.sound = soundfx.sound;
		right.soundfx.sound = soundfx.sound;
		left.soundfx.volume = soundfx.volume;
		right.soundfx.volume = soundfx.volume;
		left.soundfx.loop = soundfx.loop;
		right.soundfx.loop = soundfx.loop;
	}
	break;
	case SCET_Script:
	{
		left.script.script = script.script;
		right.script.script = script.script;
	}
	break;
	}

	return elements;
}

void ChannelElement::ExpandLeftBorder(int numFrames)
{
	switch (type)
	{
	case SCET_Animation:
	{
		animation.ExpandLeftBorder(numFrames);
	}
	break;
	case SCET_Transformation:
	{
		transformation.ExpandLeftBorder(numFrames);
	}
	break;
	case SCET_SoundFX:
	{
		soundfx.ExpandLeftBorder(numFrames);
	}
	break;
	case SCET_Script:
	{
		script.ExpandLeftBorder(numFrames);
	}
	break;
	}
}
void ChannelElement::ExpandRightBorder(int numFrames)
{
	switch (type)
	{
	case SCET_Animation:
	{
		animation.ExpandRightBorder(numFrames);
	}
	break;
	case SCET_Transformation:
	{
		transformation.ExpandRightBorder(numFrames);
	}
	break;
	case SCET_SoundFX:
	{
		soundfx.ExpandRightBorder(numFrames);
	}
	break;
	case SCET_Script:
	{
		script.ExpandRightBorder(numFrames);
	}
	break;
	}
}

SequenceChannelElement* ChannelElement::GetElementPointer()
{
	std::unordered_map<SequenceChannelElementType, SequenceChannelElement*> elementsByType =
	{
		{ SCET_Animation, &animation },
		{ SCET_Transformation, &transformation },
		{ SCET_SoundFX, &soundfx },
		{ SCET_Script, &script }
	};
	return elementsByType.at(type);
}

int ChannelElement::GetFrameStart()
{
	switch (type)
	{
	case SCET_Animation:
	{
		return animation.GetFrameStart();
	}
	break;
	case SCET_Transformation:
	{
		return transformation.frameStart;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx.GetFrameStart();
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

int ChannelElement::GetFrameEnd()
{
	switch (type)
	{
	case SCET_Animation:
	{
		return animation.GetFrameEnd();
	}
	break;
	case SCET_Transformation:
	{
		return transformation.frameEnd;
	}
	break;
	case SCET_SoundFX:
	{
		return soundfx.GetFrameEnd();
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

bool SequenceChannel::ChannelHasElementAtFrame(int frame)
{
	for (auto& element : elements)
	{
		bool left, right;
		if (element.ElementInFrame(frame, left, right))
			return true;
	}

	return false;
}

int SequenceChannel::GetAvailableFramesToLeft(int elementIndex)
{
	ChannelElement& element = elements.at(elementIndex);
	if (elementIndex == 0)
		return element.GetFrameStart();

	ChannelElement& prevElement = elements.at(elementIndex - 1);
	return element.GetFrameStart() - prevElement.GetFrameEnd() - 1;
}

int SequenceChannel::GetAvailableFramesToRight(int elementIndex, int totalFrames)
{
	ChannelElement& element = elements.at(elementIndex);
	if (elementIndex == (elements.size() - 1))
		return totalFrames - element.GetFrameEnd();

	ChannelElement& nextElement = elements.at(elementIndex + 1);
	return nextElement.GetFrameStart() - element.GetFrameEnd() - 1;
}

int SequenceChannel::GetFirstElementIndexBetweenFrames(int frameStart, int frameEnd)
{
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		int elemStart = element.GetFrameStart();
		int elemEnd = element.GetFrameEnd();
		if (nostd::in_between(frameStart, elemStart, elemEnd) || nostd::in_between(frameEnd, elemStart, elemEnd))
			return i;
	}
	return -1;
}

int SequenceChannel::GetElementIndexBeforeFrame(int frame)
{
	int idx = -1;
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		int elemEnd = element.GetFrameEnd();
		if (elemEnd < frame)
			break;
		idx++;
	}
	return idx;
}

SequenceChannelElementAnimation* SequenceChannel::GetAnimationElementAtFrame(int frame)
{
	SequenceChannelElementAnimation* curr = nullptr;
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		if (element.type != SCET_Animation)
			continue;
		if (frame < element.GetFrameStart())
			continue;
		if (frame >= element.GetFrameStart())
			curr = &element.animation;
	}
	return curr;
}

SequenceChannelElementTransformation* SequenceChannel::GetTransformationElementAtFrame(int frame)
{
	SequenceChannelElementTransformation* curr = nullptr;
	for (int i = 0; i < elements.size(); i++)
	{
		ChannelElement& element = elements.at(i);
		if (element.type != SCET_Transformation)
			continue;
		if (frame < element.GetFrameStart())
			continue;
		if (frame >= element.GetFrameStart())
			curr = &element.transformation;
	}
	return curr;
}

TransformationKeyFrame* SequenceChannel::GetTransformationKeyframe(int frame)
{
	auto* t = GetTransformationElementAtFrame(frame);
	return (t != nullptr && t->keyFrames.contains(frame)) ? &t->keyFrames.at(frame) : nullptr;
}

SequenceChannelElementSoundFX* SequenceChannel::GetSoundFXToCreateAtFrame(int frame)
{
	for (auto& element : elements)
	{
		if (element.type != SCET_SoundFX) continue;
		if (element.GetFrameStart() == frame) return &element.soundfx;
	}
	return nullptr;
}

SequenceChannelElementScript* SequenceChannel::GetScriptToRunAtFrame(int frame)
{
	for (auto& element : elements)
	{
		if (element.type == SCET_Script && element.GetFrameStart() == frame)
			return &element.script;
	}
	return nullptr;
}

void SequenceChannel::InsertChannelElement(ChannelElement element, int& totalFrames)
{
	int elemStart = element.GetFrameStart();
	int elemEnd = element.GetFrameEnd();
	int curIndex = GetFirstElementIndexBetweenFrames(elemStart, elemEnd);
	if (curIndex == -1)
	{
		int beforeIndex = GetElementIndexBeforeFrame(elemStart);
		if (beforeIndex == -1)
		{
			elements.push_back(element);
		}
		else
		{
			elements.insert(elements.begin() + beforeIndex, 1, element);
		}
		totalFrames = std::max(totalFrames, element.GetFrameEnd());
	}
	else
	{
		ChannelElement& curElem = elements.at(curIndex);
		int rightShift = elemEnd - curElem.GetFrameStart() + 1;

		int framesToRight = GetAvailableFramesToRight(static_cast<int>(elements.size()) - 1, totalFrames);
		if (rightShift > framesToRight)
		{
			totalFrames += (rightShift - framesToRight);
		}

		for (int i = curIndex; i < elements.size(); i++)
		{
			elements.at(i).Move(rightShift, totalFrames);
		}
		elements.insert(elements.begin() + curIndex, 1, element);
	}
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

void SequenceChannel::DragElementLeftBoundary(int elementIndex, int frames, int totalFrames)
{
	ChannelElement& element = elements.at(elementIndex);
	if (frames < 0)
	{
		frames = -std::min(-frames, GetAvailableFramesToLeft(elementIndex));
		element.ExpandLeftBorder(frames);
	}
	else if (frames > 0)
	{
		int availableFrames = element.GetFrameEnd() - element.GetFrameStart();
		frames = std::min(frames, availableFrames);
		element.ExpandLeftBorder(frames);
	}
}

void SequenceChannel::DragElementRightBoundary(int elementIndex, int frames, int totalFrames)
{
	ChannelElement& element = elements.at(elementIndex);
	if (frames < 0)
	{
		int availableFrames = element.GetFrameEnd() - element.GetFrameStart();
		frames = std::max(frames, -availableFrames);
		element.ExpandRightBorder(frames);
	}
	else if (frames > 0)
	{
		frames = std::min(frames, GetAvailableFramesToRight(elementIndex, totalFrames));
		element.ExpandRightBorder(frames);
	}
}

void SequenceChannel::EraseElement(int elementIndex)
{
	nostd::vector_erase_index(elements, elementIndex);
}

void SequenceChannel::SplitElement(int elementIndex, int frame)
{
	ChannelElement& element = elements.at(elementIndex);

	auto [left, right] = element.Split(frame);

	elements.erase(elements.begin() + elementIndex);
	std::vector<ChannelElement> toInsert = { left,right };
	elements.insert(elements.begin() + elementIndex, toInsert.begin(), toInsert.end());
}

bool SequenceChannel::FrameHasElement(int frame, bool& leftBounded, bool& rightBounded)
{
	return std::any_of(elements.begin(), elements.end(), [&leftBounded, &rightBounded, frame](ChannelElement& elem)
		{
			return elem.ElementInFrame(frame, leftBounded, rightBounded);
		}
	);
}

bool SequenceChannel::FrameHasTransformationKeyframe(int frame)
{
	if (!ChannelHasElementAtFrame(frame)) return false;

	SequenceChannelElementTransformation* transformation = GetTransformationElementAtFrame(frame);
	if (!transformation) return false;

	return transformation->keyFrames.contains(frame);
}

void SequenceChannel::EraseElementInFrame(int frame)
{
	EraseElement(GetFirstElementIndexBetweenFrames(frame, frame));
}

void SequenceChannel::SplitElementInFrame(int frame)
{
	SplitElement(GetFirstElementIndexBetweenFrames(frame, frame), frame);
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
	totalFrames = 160;
}

Sequence::Sequence(nlohmann::json j)
{
	framesPerSecond = j.at("framesPerSecond");
	totalFrames = j.at("totalFrames");
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
			{ "sequenceChannels", seqChannels }
		}
	);
	return j;
}

bool Sequence::operator==(const Sequence& other) const {

	if (framesPerSecond != other.framesPerSecond) return false;
	if (totalFrames != other.totalFrames) return false;

	return std::equal(sequenceChannels.begin(), sequenceChannels.end(), other.sequenceChannels.begin());
}

std::string Sequence::GetAnimationNameAtFrame(int frame)
{
	for (auto it = sequenceChannels.rbegin(); it != sequenceChannels.rend(); it++)
	{
		int index = it->GetFirstElementIndexBetweenFrames(frame, frame);
		if (index == -1) continue;
		auto& element = it->elements.at(index);
		if (element.type != SCET_Animation) continue;
		return element.animation.animation;
	}
	return "";
}

SequenceChannelElementAnimation* Sequence::GetAnimationElementAtFrame(int frame)
{
	std::vector<SequenceChannelElementAnimation*> animations;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementAnimation* anim = channel.GetAnimationElementAtFrame(frame);
		if (anim == nullptr) continue;
		animations.push_back(anim);
	}

	if (animations.size() == 0ULL) return nullptr;

	std::sort(animations.begin(), animations.end(), [](SequenceChannelElementAnimation* animA, SequenceChannelElementAnimation* animB)
		{
			int endA = animA->frameEnd;
			int endB = animB->frameEnd;
			return endA > endB;
		}
	);

	return *animations.begin();
}

XMMATRIX Sequence::GetTransformationAtFrame(int frame)
{
	std::vector<SequenceChannelElementTransformation*> transformations;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementTransformation* t = channel.GetTransformationElementAtFrame(frame);
		if (t == nullptr) continue;
		transformations.push_back(t);
	}

	if (transformations.size() == 0ULL) return XMMatrixIdentity();

	std::sort(transformations.begin(), transformations.end(), [](SequenceChannelElementTransformation* tA, SequenceChannelElementTransformation* tB)
		{
			int endA = tA->frameEnd;
			int endB = tB->frameEnd;
			return endA > endB;
		}
	);

	SequenceChannelElementTransformation* transformation = *transformations.begin();

	return transformation->GetTransformationInFrame(frame);
}

void Sequence::CreateSoundFXsAtFrame(int frame)
{
	std::set<SequenceChannelElementSoundFX*> soundfxs;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementSoundFX* sfx = channel.GetSoundFXToCreateAtFrame(frame);
		if (sfx == nullptr) continue;
		soundfxs.insert(sfx);
	}

	if (soundfxs.size() == 0ULL) return;

	for (auto& sfx : soundfxs)
	{
		SoundJsonUUID sjson = sfx->sound();
		SoundFXUUID soundFXUUID = getUUID();
		nlohmann::json jsound =
		{
			{ "uuid", soundFXUUID() },
			{ "name", sjson->name() },
			{ "sound", sfx->sound() },
			{ "autoPlay", true }
		};

		CreateSoundFX(jsound);
		soundFXUUID->BindToScene();
		soundFXUUID->Play();
	}
}

void Sequence::RunScriptAtFrame(int frame, RenderableUUID renderable)
{
	using namespace Scripting;
	for (SequenceChannel& channel : sequenceChannels)
	{
		SequenceChannelElementScript* script = channel.GetScriptToRunAtFrame(frame);
		if (script == nullptr) continue;
		RunScript(script->script, renderable);
	}
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

SequencePlayer::SequencePlayer()
{
	sequence = nullptr;
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
}

SequencePlayer::SequencePlayer(Sequence* seq, JUUID uuid)
{
	sequence = seq;
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	renderable = uuid;
}

void SequencePlayer::SetSequence(Sequence* seq, JUUID uuid)
{
	sequence = seq;
	time = 0.0f;
	runningFrame = 0;
	currentFrame = 0;
	loop = false;
	runnedFrames.clear();
	renderable = uuid;
}

void SequencePlayer::Step(float dt)
{
	if (sequence == nullptr) return;
	float totalTimeMs = 1000.0f * static_cast<float>(sequence->totalFrames) / static_cast<float>(sequence->framesPerSecond);
	time += dt;
	if (time >= totalTimeMs)
	{
		time = (loop) ? fmodf(time, totalTimeMs) : totalTimeMs;
		currentFrame = (loop) ? static_cast<int>(static_cast<float>(sequence->framesPerSecond) * time / 1000.0f) : sequence->totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	else
	{
		currentFrame = static_cast<int>(static_cast<float>(sequence->framesPerSecond) * time / 1000.0f);
	}

	int targetFrame = currentFrame;
	if (runningFrame < sequence->totalFrames && currentFrame == 0)
		targetFrame = sequence->totalFrames;

	while (runningFrame <= targetFrame)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence->totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence->totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::SetTime(float t)
{
	if (sequence == nullptr) return;
	float totalTimeMs = 1000.0f * static_cast<float>(sequence->totalFrames) / static_cast<float>(sequence->framesPerSecond);
	time = t;
	if (time >= totalTimeMs)
	{
		time = (loop) ? fmodf(time, totalTimeMs) : totalTimeMs;
		currentFrame = (loop) ? static_cast<int>(static_cast<float>(sequence->framesPerSecond) * time / 1000.0f) : sequence->totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	else
	{
		currentFrame = static_cast<int>(static_cast<float>(sequence->framesPerSecond) * time / 1000.0f);
	}

	int targetFrame = currentFrame;
	if (runningFrame < sequence->totalFrames && currentFrame == 0)
		targetFrame = sequence->totalFrames;

	while (runningFrame <= targetFrame)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence->totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence->totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::StepFrame(int df)
{
	if (sequence == nullptr) return;
	currentFrame += df;
	if (currentFrame > sequence->totalFrames)
	{
		currentFrame = (loop) ? (currentFrame % sequence->totalFrames) : sequence->totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	time = 1000.0f * static_cast<float>(currentFrame) / static_cast<float>(sequence->framesPerSecond);

	int targetFrame = currentFrame;
	if (runningFrame < sequence->totalFrames && currentFrame == 0)
		targetFrame = sequence->totalFrames;

	while (runningFrame <= targetFrame)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence->totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence->totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::SetFrame(int frame, bool runningPlayer)
{
	if (sequence == nullptr) return;
	currentFrame = frame;
	if (currentFrame > sequence->totalFrames)
	{
		currentFrame = (loop) ? (currentFrame % sequence->totalFrames) : sequence->totalFrames;
		if (loop)
		{
			runnedFrames.clear();
		}
	}
	time = 1000.0f * static_cast<float>(currentFrame) / static_cast<float>(sequence->framesPerSecond);

	if (runningPlayer == false) return;

	int targetFrame = currentFrame;
	if (runningFrame < sequence->totalFrames && currentFrame == 0)
		targetFrame = sequence->totalFrames;

	while (runningFrame <= targetFrame)
	{
		if (!runnedFrames.contains(runningFrame))
		{
			CreateFrameSoundFXs(runningFrame);
			ExecuteFrameScripts(runningFrame);
		}
		runnedFrames.insert(runningFrame);
		runningFrame++;
		if (runningFrame > sequence->totalFrames)
		{
			if (loop)
			{
				runningFrame = 0;
				runnedFrames.clear();
			}
			else
			{
				runningFrame = sequence->totalFrames;
			}
			break;
		}
	}
}

void SequencePlayer::ApplyFrameValues(RenderableUUID renderable)
{
	SequenceChannelElementAnimation* animation = sequence->GetAnimationElementAtFrame(currentFrame);
	if (animation == nullptr)
	{
		renderable->animation("");
		renderable->animationFrame(0);
		renderable->animationTime(0.0f);
	}
	else
	{
		renderable->animation(animation->animation);
		renderable->animationFrame(currentFrame);
		renderable->animationTime(animation->GetTimeAtFrame(currentFrame));
		//float timeAtFrame = animation->GetTimeAtFrame(currentFrame);
		//OutputDebugStringA(std::string("anim:" + animation->animation + std::string(" frame:") + std::to_string(currentFrame) + " " + std::string("time:") + std::to_string(timeAtFrame) + "\n").c_str());;
	}

	renderable->currentSequenceTransformation = sequence->GetTransformationAtFrame(currentFrame);
}

void SequencePlayer::CreateFrameSoundFXs(int frame)
{
	sequence->CreateSoundFXsAtFrame(frame);
}

void SequencePlayer::ExecuteFrameScripts(int frame)
{
	sequence->RunScriptAtFrame(frame, renderable);
}

void SequencePlayer::ResetFrames()
{
	runnedFrames.clear();
	runningFrame = 0;
	currentFrame = 0;
	time = 0.0f;
}
