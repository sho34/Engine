struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv0 : TEXCOORD0;
};

Texture2D texture : register(t0);
SamplerState samp0 : register(s0);

cbuffer renderable : register(b0)
{
	#include "CBVars/FullScreenQuad.h"
}

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	output.pos = float4(input.pos.xy,1.0f.xx);
	output.uv0 = input.uv;

	return output;
}

float4 main_ps(PixelShaderInput input) : SV_TARGET
{
	return float4(texture.Sample(samp0, input.uv0.xy).rgb*alpha,1.0f);
	//return float4(1,1,1,1);
}
