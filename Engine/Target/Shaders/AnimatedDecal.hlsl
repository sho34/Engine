#include "Utils/Gamma.h"
#include "Camera/Camera.h"

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

cbuffer renderable : register(b0)
{
	#include "CBVars/Model.h"
	#include "CBVars/Material.h"
};

ConstantBuffer<Camera> camera : register(b1);

Texture2DArray BaseTexture : register(t0);
SamplerState samp0 : register(s0);

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

    matrix wvp = mul(world, camera.viewProjection);
	
	float4 pos = mul(float4(input.pos, 1.0f),wvp);
	float2 uv = input.uv;

	output.pos = pos;
	output.uv = uv;

	return output;
}

float4 main_ps(PixelShaderInput input) : SV_TARGET
{
	float4 color = 0.0f.xxxx;
	color += BaseTexture.Sample(samp0, float3(input.uv,frameIndex));
	if (color.a < alphaCut) {
		discard;
	}

	return float4(color.rgb,1.0f);
}
