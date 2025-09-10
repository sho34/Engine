#pragma once

struct PassMaterialOverride
{
	unsigned int meshIndex;
	std::string renderPass;
	std::string material;
};

inline PassMaterialOverride ToPassMaterialOverride(nlohmann::json j)
{
	return { j.at("meshIndex"), j.at("renderPass"), j.at("material") };
}
inline nlohmann::json FromPassMaterialOverride(PassMaterialOverride pmo)
{
	return {
		{"meshIndex", pmo.meshIndex },
		{"renderPass", pmo.renderPass},
		{"material", pmo.material}
	};
}
