#include "Camera/Camera.h"
#include "Animation/Animated3D.h"

struct VertexShaderInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
	uint4 boneIds : BLENDINDICES;
	float4 boneWeights : BLENDWEIGHT;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer renderable : register(b0)
{
	#include "CBVars/Model.h"
	#include "CBVars/Material.h"
}

ConstantBuffer<Camera> camera : register(b1);
ConstantBuffer<Animated3D> animation : register(b2);

Texture2D baseTexture : register(t0);
SamplerState sampler0 : register(s0);

PixelShaderInput main_vs(VertexShaderInput input)
{
    PixelShaderInput output;

	matrix boneTransform = animation.boneMatrices[input.boneIds.x] * input.boneWeights.x +
		animation.boneMatrices[input.boneIds.y] * input.boneWeights.y +
		animation.boneMatrices[input.boneIds.z] * input.boneWeights.z +
		animation.boneMatrices[input.boneIds.w] * input.boneWeights.w;

	float3 skinnedPos = mul(float4(input.pos, 1.0f), boneTransform).xyz;

    matrix wvp = mul(world, camera.viewProjection);
	
    float4 pos = mul(float4(skinnedPos, 1.0f),wvp);
    float2 uv = input.uv;

    output.pos = pos;
    output.uv = uv;

    return output;
}

void main_ps(PixelShaderInput input) : SV_TARGET
{
    float4 texturesColor = baseTexture.Sample(sampler0, input.uv);

    if (texturesColor.w < alphaCut)
    {
        discard;
    }
}
