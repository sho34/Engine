#include "pch.h"
#include "Templates.h"
#include <fstream>
#include <Material/Material.h>
#include <Model3D/Model3D.h>
#include <RenderPass/RenderPass.h>
#include <Shader/Shader.h>
#include <Sound/Sound.h>
#include <Textures/Texture.h>
#if defined(_EDITOR)
#include <Editor.h>
#endif

namespace Templates
{
	nlohmann::json systemShaders = nlohmann::json::array(
		{
			{
				{ "name", "IBLBRDFLUT_cs" },
				{ "path", "CSBRDFLUT.hlsl"},
				{ "systemCreated", true },
				{ "uuid", "bb76e846-4015-48a0-ab94-5286dd843052"},
				{ "type", ShaderTypeToStr.at(ShaderType::COMPUTE_SHADER) }
			},
			{
				{ "name","IBLPrefilteredEnvironmentMap_cs"},
				{ "path" , "CSPreFilteredEnvironmentMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "6e278619-da6b-48ec-8434-53c3506e7bfd"},
				{ "type" , ShaderTypeToStr.at(ShaderType::COMPUTE_SHADER) }
			},
			{
				{ "name", "IBLDiffuseIrradianceMap_cs" },
				{ "path", "CSDiffuseIrradianceMap.hlsl" },
				{ "systemCreated", true },
				{ "uuid", "5ebcccb5-477a-49c9-9878-9ff6453266a0" },
				{ "type", ShaderTypeToStr.at(ShaderType::COMPUTE_SHADER) }
			},
			{
				{ "name","LuminanceHistogramAverage_cs"},
				{ "path" , "CSLuminanceHistogramAverage.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "1d436897-e925-415f-9209-1364005792a0"},
				{ "type" , ShaderTypeToStr.at(ShaderType::COMPUTE_SHADER)}
			},
			{
				{ "name","LuminanceHistogram_cs"},
				{ "path" , "CSLuminanceHistogram.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "43b52d31-7040-47e7-80e6-97490550cbae"},
				{ "type" , ShaderTypeToStr.at(ShaderType::COMPUTE_SHADER)}
			},
			{
				{ "name","BoundingBox_cs"},
				{ "path" , "CSBoundingBox.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "c23ab559-be11-45ad-b598-1e48e5280914"},
				{ "type" , ShaderTypeToStr.at(ShaderType::COMPUTE_SHADER)}
			},
			{
				{ "name","BoundingBox_vs" },
				{ "path" , "BoundingBox.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "ae7a35a5-f012-4eb6-bbe1-1f52e6203ccb" },
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","BoundingBox_ps" },
				{ "path" , "BoundingBox.hlsl" },
				{ "systemCreated", true },
				{ "mappedValues",
					{
						{
							{ "value", { 1.0, 0.0, 0.0 } },
							{ "variable" , "baseColor" },
							{ "variableType" , "RGB" }
						}
					}
				},
				{ "uuid" , "1bf837a7-1282-4fae-a1ba-9e74e6a99b37" },
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","BaseLighting_vs" },
				{ "path" , "BaseLighting.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "bc331f48-6a40-4b48-b435-8276051d6993" },
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","BaseLighting_ps" },
				{ "path" , "BaseLighting.hlsl" },
				{ "systemCreated" , true },
				{ "mappedValues" ,
					{
						{
							{ "value", { 0.11764706671237946, 0.5647059082984924, 1.0} },
							{ "variable" , "baseColor" },
							{ "variableType" , "RGB" }
						},
						{
							{ "value", 400.0 },
							{ "variable" , "specularExponent"},
							{ "variableType" , "FLOAT" }
						}
					}
				},
				{ "uuid" , "719c0122-1e9f-46e3-90aa-8f1e5e81c098"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","Grid_vs"},
				{ "path" , "Grid.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "5af4ba59-a09c-41ef-bc1f-13a51fc68439"},
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","Grid_ps" },
				{ "path", "Grid.hlsl" },
				{ "systemCreated" , true},
				{ "mappedValues" , {
					{
						{ "value", { 1.0, 0.0, 1.0 } },
						{ "variable" , "baseColor" },
						{ "variableType" , "RGB" }
					},
					{
						{ "value", 1024.0 },
						{ "variable" , "specularExponent"},
						{ "variableType" , "FLOAT" }
					}
				}
				},
				{ "uuid" , "5929c8f6-e9b7-4680-8447-a430b5accdbf"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","ShadowMap_vs"},
				{ "path" , "ShadowMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "0069d1e9-45b0-4fd3-a28f-1f7508503a91"},
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","ShadowMap_ps"},
				{ "path" , "ShadowMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "ed41913d-1a28-40ce-9c92-07549714f367"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","DepthMinMax_vs"},
				{ "path" , "DepthMinMax.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "2ad43d9e-8dec-421c-b8f2-bda3520748bd"},
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","DepthMinMax_ps"},
				{ "path" , "DepthMinMax.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "dd93a59f-a87e-4d9a-a57c-b91066e7520e"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","DepthMinMaxToRGBA_vs" },
				{ "path" , "DepthMinMaxToRGBA.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "9815152b-84ad-45e5-8b91-0642cfde0543" },
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","DepthMinMaxToRGBA_ps" },
				{ "path" , "DepthMinMaxToRGBA.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "22c13e3e-5a88-4868-a5cf-bcc65864cf6c" },
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","DepthMinMaxToRGBASpot_vs" },
				{ "path" , "DepthMinMaxToRGBASpot.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "173a942d-83e2-4d51-83cd-59016cb5be4e" },
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","DepthMinMaxToRGBASpot_ps" },
				{ "path" , "DepthMinMaxToRGBASpot.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "438f86fd-9ef3-433f-ad7b-c1e60643cd3e" },
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER) }
			},
			{
				{ "name","FullScreenQuad_vs" },
				{ "path" , "FullScreenQuad.hlsl" },
				{ "systemCreated" , true },
				{ "uuid" , "8e26fbd4-3a2c-4c04-a628-d2f11d474d60" },
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER) }
			},
			{
				{ "name","FullScreenQuad_ps"},
				{ "path" , "FullScreenQuad.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "9ab3d65f-be9a-49cc-87f8-bcbf1dafeac7"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","ToneMap_vs"},
				{ "path" , "ToneMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "8ee7a4d0-91f1-4264-aa56-9f82b3c38397"},
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","ToneMap_ps"},
				{ "path" , "ToneMap.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "75e834c4-6898-4156-af67-43abba7fc6b5"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","LoadingBar_vs"},
				{ "path" , "LoadingBar.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "d0192f97-a56a-469d-b6f1-07d403ae331a"},
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","LoadingBar_ps"},
				{ "path" , "LoadingBar.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "b5ef5d53-2174-4d12-b231-5e07a7f5a7f8"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER)}
			},
			{
				{ "name","Picking_vs"},
				{ "path" , "Picking.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "79568541-34c8-4464-bec1-77debde975e0"},
				{ "type" , ShaderTypeToStr.at(ShaderType::VERTEX_SHADER)}
			},
			{
				{ "name","Picking_ps"},
				{ "path" , "Picking.hlsl"},
				{ "systemCreated" , true},
				{ "uuid" , "e32c5e9c-26a5-4f2b-8d0c-5899c67f1def"},
				{ "type" , ShaderTypeToStr.at(ShaderType::PIXEL_SHADER)}
			}
		}
	);

	nlohmann::json systemSounds = nlohmann::json::array({});

	nlohmann::json systemMaterials = nlohmann::json::array(
		{
			{
				{ "name","BoundingBox"},
				{ "shader_vs" , "ae7a35a5-f012-4eb6-bbe1-1f52e6203ccb"},
				{ "shader_ps" , "1bf837a7-1282-4fae-a1ba-9e74e6a99b37"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "uuid" , "2e4d8bf0-0761-45d9-8313-17cdf9b5f8fc"}
			},
			{
				{ "name","BaseLighting"},
				{ "shader_vs" , "bc331f48-6a40-4b48-b435-8276051d6993"},
				{ "shader_ps" , "719c0122-1e9f-46e3-90aa-8f1e5e81c098"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" ,{
					{
						{ "Filter","MIN_MAG_MIP_LINEAR" },
						{ "AddressU" , "ADDRESS_MODE_BORDER" },
						{ "AddressV" , "ADDRESS_MODE_BORDER" },
						{ "AddressW" , "ADDRESS_MODE_BORDER" },
						{ "MipLODBias" , 0 },
						{ "MaxAnisotropy" , 0 },
						{ "ComparisonFunc" , "NEVER" },
						{ "BorderColor" , "OPAQUE_WHITE" },
						{ "MinLOD" , 0.0 },
						{ "MaxLOD" , 3.4028234663852886e+38 },
						{ "ShaderRegister" , 0 },
						{ "RegisterSpace" , 0 },
						{ "ShaderVisibility" , "PIXEL" }
					}
				}},
				{ "uuid","4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc" }
			},
			{
				{ "name","Floor" },
				{ "shader_vs" , "5af4ba59-a09c-41ef-bc1f-13a51fc68439" },
				{ "shader_ps" , "5929c8f6-e9b7-4680-8447-a430b5accdbf" },
				{ "systemCreated" , true },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" ,{
					{
						{ "Filter","MIN_MAG_MIP_LINEAR"},
						{ "AddressU" , "ADDRESS_MODE_BORDER"},
						{ "AddressV" , "ADDRESS_MODE_BORDER"},
						{ "AddressW" , "ADDRESS_MODE_BORDER"},
						{ "MipLODBias" , 0},
						{ "MaxAnisotropy" , 0},
						{ "ComparisonFunc" , "NEVER"},
						{ "BorderColor" , "OPAQUE_WHITE"},
						{ "MinLOD" , 0.0},
						{ "MaxLOD" , 3.4028234663852886e+38},
						{ "ShaderRegister" , 0},
						{ "RegisterSpace" , 0},
						{ "ShaderVisibility" , "PIXEL"}
					}
				}
				},
				{ "uuid","ecd1688c-73d6-49d0-870f-ca916a417c49" }
			},
			{
				{ "name","ShadowMap" },
				{ "shader_vs" , "0069d1e9-45b0-4fd3-a28f-1f7508503a91" },
				{ "shader_ps" , "ed41913d-1a28-40ce-9c92-07549714f367" },
				{ "systemCreated" , true },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" , {
					{
						{ "Filter","MIN_MAG_MIP_LINEAR"},
						{ "AddressU" , "ADDRESS_MODE_BORDER"},
						{ "AddressV" , "ADDRESS_MODE_BORDER"},
						{ "AddressW" , "ADDRESS_MODE_BORDER"},
						{ "MipLODBias" , 0},
						{ "MaxAnisotropy" , 0},
						{ "ComparisonFunc" , "NEVER"},
						{ "BorderColor" , "OPAQUE_WHITE"},
						{ "MinLOD" , 0.0},
						{ "MaxLOD" , 3.4028234663852886e+38},
						{ "ShaderRegister" , 0},
						{ "RegisterSpace" , 0},
						{ "ShaderVisibility" , "PIXEL"}
					}
				}
				},
				{ "uuid","3be1cf4e-cc15-41ae-97e1-6bb3e110271f" }
			},
			{
				{ "name","DepthMinMax"},
				{ "shader_vs" , "2ad43d9e-8dec-421c-b8f2-bda3520748bd"},
				{ "shader_ps" , "dd93a59f-a87e-4d9a-a57c-b91066e7520e"},
				{ "systemCreated" , true},
				{ "twoSided" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" ,{
					{
						{ "Filter","MIN_MAG_MIP_POINT"},
						{ "AddressU" , "ADDRESS_MODE_BORDER"},
						{ "AddressV" , "ADDRESS_MODE_BORDER"},
						{ "AddressW" , "ADDRESS_MODE_BORDER"},
						{ "MipLODBias" , 0},
						{ "MaxAnisotropy" , 0},
						{ "ComparisonFunc" , "NEVER"},
						{ "BorderColor" , "OPAQUE_WHITE"},
						{ "MinLOD" , 0.0},
						{ "MaxLOD" , 3.4028234663852886e+38},
						{ "ShaderRegister" , 0},
						{ "RegisterSpace" , 0},
						{ "ShaderVisibility" , "PIXEL"}
					}
				}
				},
				{"uuid","35da9e7d-1ef8-4165-8e71-36d6cf599c3c" }
			},
			{
				{ "name","DepthMinMaxToRGBA"},
				{ "shader_vs" , "9815152b-84ad-45e5-8b91-0642cfde0543"},
				{ "shader_ps" , "22c13e3e-5a88-4868-a5cf-bcc65864cf6c"},
				{ "systemCreated" , true},
				{ "twoSided" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" , {
					{
						{ "Filter","MIN_MAG_MIP_POINT" },
						{ "AddressU" , "ADDRESS_MODE_BORDER" },
						{ "AddressV" , "ADDRESS_MODE_BORDER" },
						{ "AddressW" , "ADDRESS_MODE_BORDER" },
						{ "MipLODBias" , 0 },
						{ "MaxAnisotropy" , 0 },
						{ "ComparisonFunc" , "NEVER" },
						{ "BorderColor" , "OPAQUE_WHITE" },
						{ "MinLOD" , 0.0 },
						{ "MaxLOD" , 3.4028234663852886e+38 },
						{ "ShaderRegister" , 0 },
						{ "RegisterSpace" , 0 },
						{ "ShaderVisibility" , "PIXEL" }
					}
				}
				},
				{ "uuid" , "84f0cabb-9b0c-4508-ac6e-d7a84dee696f" }
			},
			{
				{ "name","DepthMinMaxToRGBASpot" },
				{ "shader_vs" , "173a942d-83e2-4d51-83cd-59016cb5be4e" },
				{ "shader_ps" , "438f86fd-9ef3-433f-ad7b-c1e60643cd3e" },
				{ "systemCreated" , true },
				{ "twoSided" , true },
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "samplers" , {
					{
						{ "Filter","MIN_MAG_MIP_POINT" },
						{ "AddressU" , "ADDRESS_MODE_BORDER" },
						{ "AddressV" , "ADDRESS_MODE_BORDER" },
						{ "AddressW" , "ADDRESS_MODE_BORDER" },
						{ "MipLODBias" , 0 },
						{ "MaxAnisotropy" , 0 },
						{ "ComparisonFunc" , "NEVER" },
						{ "BorderColor" , "OPAQUE_WHITE" },
						{ "MinLOD" , 0.0 },
						{ "MaxLOD" , 3.4028234663852886e+38 },
						{ "ShaderRegister" , 0 },
						{ "RegisterSpace" , 0 },
						{ "ShaderVisibility" , "PIXEL" }
					}
				}
				},
				{ "uuid" , "908332fb-48b2-42ee-b678-e57fb3ad352e" }
			},
			{
				{ "name","FullScreenQuad"},
				{ "shader_vs" , "8e26fbd4-3a2c-4c04-a628-d2f11d474d60"},
				{ "shader_ps" , "9ab3d65f-be9a-49cc-87f8-bcbf1dafeac7"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "uuid" , "8e98708c-fe2e-4123-b1f0-5b80fabd1888"}
			},
			{
				{ "name","ToneMap"},
				{ "shader_vs" , "8ee7a4d0-91f1-4264-aa56-9f82b3c38397"},
				{ "shader_ps" , "75e834c4-6898-4156-af67-43abba7fc6b5"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{	"FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "uuid" , "8291ba82-165d-464b-be15-d9fa6d7b9a7c"}
			},
			{
				{ "name","LoadingBar"},
				{ "shader_vs" , "d0192f97-a56a-469d-b6f1-07d403ae331a"},
				{ "shader_ps" , "b5ef5d53-2174-4d12-b231-5e07a7f5a7f8"},
				{ "systemCreated" , true},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
				{ "uuid" , "28c4d879-6d21-408f-acbb-120f9fdc05b0"}
			},
			{
				{ "name","Picking"},
				{ "shader_vs" , "79568541-34c8-4464-bec1-77debde975e0"},
				{ "shader_ps" , "e32c5e9c-26a5-4f2b-8d0c-5899c67f1def"},
				{ "systemCreated" , true},
				{ "uuid" , "1896d918-4e47-49a6-950b-3135ab020a0b"},
				{ "rasterizerState",
					{
						{ "FillMode", "SOLID" },
						{ "CullMode", "NONE" },
						{ "FrontCounterClockwise", false},
						{ "DepthBias", 0},
						{ "DepthBiasClamp", 0.0},
						{ "SlopeScaledDepthBias", 0.0},
						{ "DepthClipEnable", true},
						{ "MultisampleEnable", false},
						{ "AntialiasedLineEnable", false},
						{ "ForcedSampleCount", 0},
						{ "ConservativeRaster", "OFF" }
					}
				},
			}
		}
	);

	nlohmann::json systemRenderPasses = nlohmann::json::array(
		{
			{
				{ "name", "mainPass" },
				{ "uuid", "b4ee2cf4-0231-4ff1-ada8-ae745cf0709e" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R32G32B32A32_FLOAT" }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "None" }
			},
			{
				{ "name", "toneMappingPass" },
				{ "uuid", "722d8147-6483-4675-94a3-1e8af09ec5e1" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "ToneMapping" }
			},
			{
				{ "name", "resolvePass" },
				{ "uuid", "53693830-779c-4ed0-a985-402d6a72485b" },
				{ "type", "SwapChainPass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "Resolve" }
			},
			{
				{ "name", "simplePass" },
				{ "uuid", "c483d4c9-94ce-48d6-8116-ea838e69119b" },
				{ "type", "SwapChainPass" },
				{ "fitWindow", true },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "None" }
			},
			{
				{ "name", "ShadowMap" },
				{ "uuid", "241cbc97-c047-4334-9393-ae5d33268220" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "ShadowMap" },
				{ "renderCallbackOverride", "None" }
			},
			{
				{ "name", "PickingPass" },
				{ "uuid", "d607f54c-11cf-461e-9a3e-0a74a84feb2f" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { "R32_UINT" }},
				{ "depthStencilFormat", "D32_FLOAT" },
				{ "materialOverride", "Picking" },
				{ "renderCallbackOverride", "None" }
			},
			{
				{ "name", "ShadowMapMinMaxChainPass" },
				{ "uuid", "fcd248e2-55b3-42f8-ab98-8b65d6fec86e" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { "R32_FLOAT", "R32_FLOAT" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "MinMaxChain" }
			},
			{
				{ "name", "ShadowMapMinMaxChainResultPass" },
				{ "uuid", "6b1bc75a-956f-4673-b35a-a8bc820f5153" },
				{ "type", "RenderToTexturePass" },
				{ "fitWindow", false },
				{ "renderTargetFormats", { "R8G8B8A8_UNORM" }},
				{ "depthStencilFormat", "UNKNOWN" },
				{ "materialOverride", "None" },
				{ "renderCallbackOverride", "MinMaxChainResult" }
			}
		}
	);

	nlohmann::json systemTextures = nlohmann::json::array(
		{
			{
				{ "format", "B8G8R8A8_UNORM_SRGB" },
				{ "height", 256 },
				{ "images" , {
					"Assets/gizmos/light-bulb.png"
				}},
				{ "mipLevels", 8 },
				{ "name", "Assets/gizmos/light-bulb.png" },
				{ "numFrames", 1 },
				{ "type", "2D" },
				{ "uuid", "fed123fa-e248-47cd-9662-20f73285ad0e" },
				{ "width", 256 }
			},
			{
				{ "format", "B8G8R8A8_UNORM_SRGB" },
				{ "height", 64 },
				{ "images" , {
					"Assets/gizmos/soundspeaker.png"
				}},
				{ "mipLevels", 6 },
				{ "name", "Assets/gizmos/soundspeaker.png" },
				{ "numFrames", 1 },
				{ "type", "2D" },
				{ "uuid", "5e3cba75-a495-44d8-ba5b-2b888f812a2b" },
				{ "width", 64 }
			},
			{
				{ "format", "B8G8R8A8_UNORM_SRGB" },
				{ "height", 64 },
				{ "images" , {
					"Assets/gizmos/camera.png"
				}},
				{ "mipLevels", 6 },
				{ "name", "Assets/gizmos/camera.png" },
				{ "numFrames", 1 },
				{ "type", "2D" },
				{ "uuid", "2c207f54-9cdc-4c7e-a70a-60b373f2de79" },
				{ "width", 64 }
			}
		}
	);

	nlohmann::json& GetSystemShaders()
	{
		return systemShaders;
	}

	nlohmann::json& GetSystemSounds()
	{
		return systemSounds;
	}

	nlohmann::json& GetSystemMaterials()
	{
		return systemMaterials;
	}

	nlohmann::json& GetSystemRenderPasses()
	{
		return systemRenderPasses;
	}

	nlohmann::json& GetSystemTextures()
	{
		return systemTextures;
	}

#if defined(_EDITOR)

	void SaveTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json&)> writer)
	{
		//first create the directory if needed
		std::filesystem::path directory(folder);
		std::filesystem::create_directory(directory);

		//then create the json level file
		const std::string finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		std::string pathStr = path.generic_string();
		std::ofstream file;
		file.open(pathStr);
		nlohmann::json data = nlohmann::json::array();
		writer(data);
		std::string dataString = data.dump(4);
		file.write(dataString.c_str(), dataString.size());
		file.close();
	}

#endif

	void LoadTemplates(nlohmann::json templates, std::function<void(nlohmann::json)> loader)
	{
		for (unsigned int i = 0; i < templates.size(); i++)
		{
			loader(templates.at(i));
		}
	}

	void LoadTemplates(const std::string folder, const std::string fileName, std::function<void(nlohmann::json)> loader)
	{
		//first create the directory if needed
		std::filesystem::path directory(folder);
		const std::string finalFilename = folder + fileName;
		std::filesystem::path path(finalFilename);
		if (!std::filesystem::exists(path)) return;
		std::string pathStr = path.generic_string();
		std::ifstream file(pathStr);
		nlohmann::json data = nlohmann::json::parse(file);
		file.close();

		for (unsigned int i = 0; i < data.size(); i++)
		{
			loader(data.at(i));
		}
	}

	void DestroyTemplates()
	{
		ReleaseRenderPassTemplates();
		ReleaseTextureTemplates();
		ReleaseSoundTemplates();
		ReleaseModel3DTemplates();
		ReleaseMeshTemplates();
		ReleaseMaterialTemplates();
		ReleaseShaderTemplates();
	}

#if defined(_EDITOR)
	void DestroyTemplatesReferences()
	{
		ReleaseSoundEffectsInstances();
	}
#endif

	void FreeGPUIntermediateResources()
	{
		//FreeGPUTexturesUploadIntermediateResources();
	}

	void TemplatesStep(DX::StepTimer& timer)
	{
		ShaderJsonStep();
		TextureJsonsStep();
		PreviewTexturesStep(static_cast<FLOAT>(timer.GetElapsedSeconds()));
		ReloadPreviewTextures();
		MaterialJsonStep();
		SoundJsonStep();
		//ReloadTextureInstances();
	}

#if defined(_EDITOR)
	const std::map<TemplateType, std::function<std::vector<UUIDName>()>> GetT =
	{
		{ T_Materials, SortUUIDNameByName(GetMaterialsUUIDsNames) },
		{ T_Models3D, SortUUIDNameByName(GetModel3DsUUIDsNames) },
		{ T_Shaders, SortUUIDNameByName(GetShadersUUIDsNames) },
		{ T_Sounds, SortUUIDNameByName(GetSoundsUUIDsNames) },
		{ T_Textures, SortUUIDNameByName(GetTexturesUUIDsNames) },
		{ T_RenderPasses, SortUUIDNameByName(GetRenderPasssUUIDsNames) }
	};

	std::shared_ptr<JObject> GetTemplate(std::string uuid)
	{
		const std::map<TemplateType, std::function<std::shared_ptr<JObject>(std::string)>> GetSharedPtrT =
		{
			{ T_Materials, [](std::string uuid) { std::shared_ptr<JObject> o = GetMaterialTemplate(uuid); return o; } },
			{ T_Models3D, [](std::string uuid) { std::shared_ptr<JObject> o = GetModel3DTemplate(uuid); return o; } },
			{ T_Shaders, [](std::string uuid) { std::shared_ptr<JObject> o = GetShaderTemplate(uuid); return o; } },
			{ T_Sounds, [](std::string uuid) { std::shared_ptr<JObject> o = GetSoundTemplate(uuid); return o; } },
			{ T_Textures, [](std::string uuid) { std::shared_ptr<JObject> o = GetTextureTemplate(uuid); return o; } },
			{ T_RenderPasses, [](std::string uuid) { std::shared_ptr<JObject> o = GetRenderPassTemplate(uuid); return o; } }
		};

		return GetSharedPtrT.at(GetTemplateType(uuid))(uuid); //kill me please this is slow as hell
	}

	std::map<TemplateType, std::vector<UUIDName>> GetTemplates()
	{
		std::map<TemplateType, std::vector<UUIDName>> templates;
		for (auto& [type, cb] : GetT)
		{
			auto append = cb();
			nostd::AppendToVector(templates[type], append);
		}
		return templates;
	}

	std::vector<UUIDName> GetTemplates(TemplateType t)
	{
		return GetT.at(t)();
	}

	TemplateType GetTemplateType(std::string uuid)
	{
		for (auto& [type, cb] : GetT)
		{
			auto objects = cb();
			for (auto& uuidname : objects)
			{
				std::string uuidSO = std::get<0>(uuidname);
				if (uuid == uuidSO) return type;
			}
		}
		return T_None;
	}

	std::vector<std::pair<std::string, JsonToEditorValueType>> GetTemplateAttributes(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::vector<std::pair<std::string, JsonToEditorValueType>>()>> GetTAtts =
		{
			{ T_Materials, GetMaterialAttributes },
			{ T_Models3D, GetModel3DAttributes },
			{ T_Shaders, GetShaderAttributes },
			{ T_Sounds, GetSoundAttributes },
			{ T_Textures, GetTextureAttributes },
			{ T_RenderPasses, GetRenderPassAttributes }
		};
		return GetTAtts.at(t)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetTemplateDrawers(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetTDrawers =
		{
			{ T_Materials, GetMaterialDrawers },
			{ T_Models3D, GetModel3DDrawers },
			{ T_Shaders, GetShaderDrawers },
			{ T_Sounds, GetSoundDrawers },
			{ T_Textures, GetTextureDrawers },
			{ T_RenderPasses, GetRenderPassDrawers }
		};
		return GetTDrawers.at(t)();
	}

	std::map<std::string, JEdvEditorDrawerFunction> GetTemplatePreviewers(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvEditorDrawerFunction>()>> GetTPreviewers =
		{
			{ T_Materials, GetMaterialPreviewers },
			{ T_Models3D, GetModel3DPreviewers },
			{ T_Shaders, GetShaderPreviewers },
			{ T_Sounds, GetSoundPreviewers },
			{ T_Textures, GetTexturePreviewers },
			{ T_RenderPasses, GetRenderPassPreviewers }
		};
		return GetTPreviewers.at(t)();
	}

	std::vector<std::string> GetTemplateRequiredAttributes(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::vector<std::string>()>> GetTRequiredAtts =
		{
			{ T_Materials, GetMaterialRequiredAttributes },
			{ T_Models3D, GetModel3DRequiredAttributes },
			{ T_Shaders, GetShaderRequiredAttributes },
			{ T_Sounds, GetSoundRequiredAttributes },
			{ T_Textures, GetTextureRequiredAttributes },
			{ T_RenderPasses, GetRenderPassRequiredAttributes }
		};
		return GetTRequiredAtts.at(t)();
	}

	nlohmann::json GetTemplateJson(TemplateType t)
	{
		const std::map<TemplateType, std::function<nlohmann::json()>> GetTJson =
		{
			{ T_Materials, CreateMaterialJson },
			{ T_Models3D, CreateModel3DJson },
			{ T_Shaders, CreateShaderJson },
			{ T_Sounds, CreateSoundJson },
			{ T_Textures, CreateTextureJson },
			{ T_RenderPasses, CreateRenderPassJson }
		};
		return GetTJson.at(t)();
	}

	std::map<std::string, JEdvCreatorDrawerFunction> GetTemplateCreatorDrawers(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvCreatorDrawerFunction>()>> GetTDrawers =
		{
			{ T_Materials, GetMaterialCreatorDrawers },
			{ T_Models3D, GetModel3DCreatorDrawers },
			{ T_Shaders, GetShaderCreatorDrawers },
			{ T_Sounds, GetSoundCreatorDrawers },
			{ T_Textures, GetTextureCreatorDrawers },
			{ T_RenderPasses, GetRenderPassCreatorDrawers }
		};
		return GetTDrawers.at(t)();
	}

	std::map<std::string, JEdvCreatorValidatorFunction> GetTemplateValidators(TemplateType t)
	{
		const std::map<TemplateType, std::function<std::map<std::string, JEdvCreatorValidatorFunction>()>> GetTValidator =
		{
			{ T_Materials, GetMaterialCreatorValidator },
			{ T_Models3D, GetModel3DCreatorValidator },
			{ T_Shaders, GetShaderCreatorValidator },
			{ T_Sounds, GetSoundCreatorValidator },
			{ T_Textures, GetTextureCreatorValidator },
			{ T_RenderPasses, GetRenderPassCreatorValidator }
		};
		return GetTValidator.at(t)();
	}

	std::string GetTemplateName(TemplateType t, std::string uuid)
	{
		const std::map<TemplateType, std::function<std::string(std::string)>> GetTName = {
			{ T_Materials, GetMaterialName },
			{ T_Models3D, GetModel3DName },
			{ T_Shaders, GetShaderName },
			{ T_Sounds, GetSoundName },
			{ T_Textures, GetTextureName },
			{ T_RenderPasses, GetRenderPassName },
		};
		return GetTName.at(t)(uuid);
	}

	void CreateTemplateFromJson(nlohmann::json& json, std::function<void(nlohmann::json json)> creator)
	{
		if (!json.contains("uuid") || json.at("uuid") == "")
		{
			nlohmann::json patch = { {"uuid",getUUID()} };
			json.merge_patch(patch);
		}
		creator(json);
	}

	void CreateTemplate(TemplateType t, nlohmann::json json)
	{
		const std::map<TemplateType, std::function<void(nlohmann::json json)>> CreateT =
		{
			{ T_Materials, [](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateMaterial); } },
			{ T_Models3D,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateModel3D); } },
			{ T_Shaders,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateShader); } },
			{ T_Sounds,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateSound); } },
			{ T_Textures,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateTexture); } },
			{ T_RenderPasses,[](nlohmann::json json) { CreateTemplateFromJson(json,Templates::CreateRenderPass); } },
		};
		CreateT.at(t)(json);
		Editor::MarkTemplatesPanelAssetsAsDirty();
	}

	void DeleteTemplate(TemplateType t, std::string uuid)
	{
		const std::map<TemplateType, std::function<void(std::string)>> DeleteT = {
			{ T_Materials, [](std::string uuid) {}},
			{ T_Models3D, [](std::string uuid) {} },
			{ T_Shaders, [](std::string uuid) {} },
			{ T_Sounds, [](std::string uuid) {} },
			{ T_Textures, [](std::string uuid) {} },
			{ T_RenderPasses, [](std::string uuid) {} },
		};
		DeleteT.at(t)(uuid);
	}

	void DeleteTemplate(std::string uuid)
	{
		TemplateType type = GetTemplateType(uuid);
		DeleteTemplate(type, uuid);
	}

#endif
}