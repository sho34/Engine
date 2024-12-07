#include "Camera/Camera.h"

struct VertexShaderInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
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

Texture2D baseTexture : register(t0);
SamplerState sampler0 : register(s0);

PixelShaderInput main_vs(VertexShaderInput input)
{
    PixelShaderInput output;

    matrix wvp = mul(world, camera.viewProjection);
	
    float4 pos = mul(float4(input.pos, 1.0f), wvp);
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
