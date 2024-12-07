#include "Utils/Gamma.h"

cbuffer cbObject : register(b0)
{
	matrix wvp;
	uint numTextures;
};
Texture2D texs[] : register(t0);
SamplerState samp0 : register(s0);

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f),wvp);
	float2 uv = input.uv;

	output.pos = pos;
	output.uv = uv;

	return output;
}

float4 main_ps(PixelShaderInput input) : SV_TARGET
{
	float3 texturesColor = 0.0f.xxx;
	for (uint i = 0; i < numTextures; i++) {
		texturesColor += texs[i].Sample(samp0, input.uv).rgb;
	}

	return float4(toGammaSpace(texturesColor.rgb), 1.0f);
}