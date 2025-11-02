#pragma once
#include <unordered_map>
#include <string>
#include <functional>

enum TimeCallbackType
{
	TimeCallbackType_Frame,
	TimeCallbackType_Time,
	TimeCallbackType_TimeFrame,
	TimeCallbackType_Begin,
	TimeCallbackType_End
};

static inline std::unordered_map<TimeCallbackType, std::string> TimeCallbackTypeToString
{
	{ TimeCallbackType_Frame, "Frame"},
	{ TimeCallbackType_Time, "Time" },
	{ TimeCallbackType_TimeFrame, "TimeFrame" },
	{ TimeCallbackType_Begin, "Begin" },
	{ TimeCallbackType_End, "End" }
};

static inline std::unordered_map<std::string, TimeCallbackType> StringToTimeCallbackType
{
	{ "Frame", TimeCallbackType_Frame },
	{ "Time", TimeCallbackType_Time },
	{ "TimeFrame", TimeCallbackType_TimeFrame },
	{ "Begin", TimeCallbackType_Begin },
	{ "End", TimeCallbackType_End }
};

struct AnimationKeyFrame
{
	TimeCallbackType type;
	union
	{
		float time;
		unsigned int frame;
	};
	float time2;

	bool operator==(const AnimationKeyFrame& other) const
	{
		return type == other.type && time == other.time && time2 == other.time2;
	}
};

template <>
struct std::hash<AnimationKeyFrame>
{
	std::size_t operator()(const AnimationKeyFrame& key) const
	{
		size_t seed;
		nostd::hash_combine(seed, key.type, key.frame, key.time2);
		return seed;
	}
};

typedef std::unordered_map<AnimationKeyFrame, std::function<void()>, std::hash<AnimationKeyFrame>> AnimationCallbacks;