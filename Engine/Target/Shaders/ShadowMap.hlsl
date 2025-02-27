#include "Camera/Camera.h"
#include "Animation/Animated3D.h"

struct VertexShaderInput
{
    float3 pos : POSITION;
	#ifdef _HAS_NORMAL
	float3 normal : NORMAL;
	#endif
	#ifdef _HAS_TANGENT
	float3 tangent : TANGENT;
	#endif
	#ifdef _HAS_TEXCOORD0
	float2 uv : TEXCOORD0;
	#endif
	#ifdef _HAS_SKINNING
	uint4 boneIds : BLENDINDICES;
	float4 boneWeights : BLENDWEIGHT;
	#endif
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
	#ifdef _HAS_TEXCOORD0
    float2 uv : TEXCOORD0;
	#endif
};

cbuffer renderable : register(b0)
{
	#include "CBVars/Model.h"
	#include "CBVars/Material.h"
}

ConstantBuffer<Camera> camera : register(b1);
#ifdef _HAS_SKINNING
ConstantBuffer<Animated3D> animation : register(b2);
#endif

Texture2D BaseTexture : register(t0);
SamplerState sampler0 : register(s0);

PixelShaderInput main_vs(VertexShaderInput input)
{
    PixelShaderInput output;

	matrix wvp = mul(world, camera.viewProjection);

#ifdef _HAS_SKINNING
	matrix boneTransform = animation.boneMatrices[input.boneIds.x] * input.boneWeights.x +
		animation.boneMatrices[input.boneIds.y] * input.boneWeights.y +
		animation.boneMatrices[input.boneIds.z] * input.boneWeights.z +
		animation.boneMatrices[input.boneIds.w] * input.boneWeights.w;

	float3 skinnedPos = mul(float4(input.pos, 1.0f), boneTransform).xyz;
    float4 pos = mul(float4(skinnedPos, 1.0f),wvp);
#else
	float4 pos = mul(float4(input.pos, 1.0f),wvp);
#endif

output.pos = pos;

#ifdef _HAS_TEXCOORD0
    float2 uv = input.uv;
    output.uv = uv;
#endif

    return output;
}

void main_ps(PixelShaderInput input) : SV_TARGET
{
#ifdef _HAS_TEXCOORD0
    float4 texturesColor = BaseTexture.Sample(sampler0, input.uv);

    if (texturesColor.w < alphaCut)
    {
        discard;
    }
#endif
}
