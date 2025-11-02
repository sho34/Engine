#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

enum SequenceChannelType
{
	SCT_Animation
};

inline static std::unordered_map<SequenceChannelType, std::string> SequenceChannelTypeToStr =
{
	{ SCT_Animation, "animation" },
};

inline static std::unordered_map<std::string, SequenceChannelType> StrToSequenceChannelType =
{
	{ "animation", SCT_Animation },
};

struct SequenceChannel
{
	SequenceChannelType type;
	std::string animation;
	int frameStart;
	int frameEnd;

	SequenceChannel()
	{
		type = SCT_Animation;
		animation = "";
		frameStart = 0;
		frameEnd = 100;
	}

	SequenceChannel(const nlohmann::json& j)
	{
		type = StrToSequenceChannelType.at(j.at("type"));
		animation = j.at("animation");
		frameStart = int(j.at("frameStart"));
		frameEnd = int(j.at("frameEnd"));
	}

	nlohmann::json json()
	{
		nlohmann::json j(
			{
				{ "type", SequenceChannelTypeToStr.at(type) },
				{ "animation", animation },
				{ "frameStart", frameStart },
				{ "frameEnd", frameEnd }
			}
		);
		return j;
	}

	bool operator==(const SequenceChannel& other) const {
		if (type != other.type) return false;
		if (animation != other.animation) return false;
		if (frameStart != other.frameStart) return false;
		if (frameEnd != other.frameEnd) return false;
		return true;
	}
};

inline static SequenceChannel ToSequenceChannel(nlohmann::json j)
{
	SequenceChannel seqChannel(j);
	return seqChannel;
}

inline static nlohmann::json FromSequenceChannel(SequenceChannel c)
{
	return c.json();
}

struct Sequence
{
	int framesPerSecond;
	int totalFrames;
	bool loop;
	std::vector<SequenceChannel> channels;

	Sequence()
	{
		framesPerSecond = 60;
		totalFrames = 60;
		loop = false;
	};

	Sequence(std::string animation, int numFrames) : Sequence()
	{
		SequenceChannel seqC;
		seqC.animation = animation;
		seqC.frameEnd = numFrames - 1;
		totalFrames = numFrames;
		channels.push_back(seqC);
	}

	Sequence(nlohmann::json j)
	{
		framesPerSecond = j.at("framesPerSecond");
		totalFrames = j.at("totalFrames");
		loop = j.at("loop");
		for (size_t i = 0ULL; i < j.at("channels").size(); i++)
		{
			channels.push_back(SequenceChannel(j.at("channels").at(i)));
		}
	}

	nlohmann::json json()
	{
		nlohmann::json jchannels = nlohmann::json::array();
		for (auto& channel : channels)
		{
			jchannels.push_back(channel.json());
		}
		nlohmann::json j(
			{
				{ "framesPerSecond", framesPerSecond },
				{ "totalFrames", totalFrames },
				{ "loop", loop },
				{ "channels", jchannels }
			}
		);
		return j;
	}

	bool operator==(const Sequence& other) const {

		if (framesPerSecond != other.framesPerSecond) return false;
		if (totalFrames != other.totalFrames) return false;
		if (loop != other.loop) return false;

		return std::equal(channels.begin(), channels.end(), other.channels.begin());
	}
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

	AnimationSequences(nlohmann::json j)
	{
		for (nlohmann::json::iterator it = j.begin(); it != j.end(); it++)
		{
			sequences[it.key()] = Sequence(it.value());
		}
	}

	nlohmann::json json()
	{
		nlohmann::json j;
		for (auto& [name, sequence] : sequences)
		{
			j[name] = sequence.json();
		}
		return j;
	}
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