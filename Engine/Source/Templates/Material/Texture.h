#pragma once
#include "../../Resources/ShaderByteCode.h"

struct MaterialTexture
{
	//template
	std::string path;
	DXGI_FORMAT format;
	unsigned int numFrames;

	//this is a special case for direct handling of rtt textures used later as part of a renderable's material(blow my mind)
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;

	bool operator<(const MaterialTexture& other) const {
		return std::tie(path, format, numFrames, gpuHandle.ptr) < std::tie(other.path, other.format, other.numFrames, other.gpuHandle.ptr);
	}
	bool operator==(const MaterialTexture& other) const {
		return std::tie(path, format, numFrames, gpuHandle.ptr) == std::tie(other.path, other.format, other.numFrames, other.gpuHandle.ptr);
	}
};

template <>
struct std::hash<MaterialTexture>
{
	std::size_t operator()(const MaterialTexture& tex) const
	{
		using std::hash;
		// Compute individual hash values for first,
		// second and third and combine them using XOR
		// and bit shifting:
		return ((hash<string>()(tex.path) ^ (hash<DXGI_FORMAT>()(tex.format) << 1)) >> 1) ^ (hash<unsigned int>()(tex.numFrames) << 1);
	}
};

struct MaterialTextureInstance
{
	//D3D12
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	CComPtr<ID3D12Resource> texture;
	CComPtr<ID3D12Resource> upload;
	std::string textureName;

	~MaterialTextureInstance() { Destroy(); }
	void Load(MaterialTexture& texture);
	void CreateTextureResource(MaterialTexture& tex);
	void CreateTextureArrayResource(MaterialTexture& tex);
	void Destroy();
};

void TransformJsonToMaterialTextures(std::map<TextureType, MaterialTexture>& textures, nlohmann::json object, const std::string& key);

std::map<TextureType, std::shared_ptr<MaterialTextureInstance>> GetTextures(const std::map<TextureType, MaterialTexture>& textures);
std::shared_ptr<MaterialTextureInstance> GetTextureFromGPUHandle(const MaterialTexture& texture);
void DestroyMaterialTextureInstance(std::shared_ptr<MaterialTextureInstance>& texture);