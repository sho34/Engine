#include "Camera/Camera.h"

struct VertexShaderInput
{
    float3 pos : POSITION;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
};

cbuffer renderable : register(b0)
{
	#include "CBVars/Model.h"
	#include "CBVars/Material.h"
}

ConstantBuffer<Camera> camera : register(b1);

PixelShaderInput main_vs(VertexShaderInput input)
{
    PixelShaderInput output;

    matrix wvp = mul(world, camera.viewProjection);
	
    float4 pos = mul(float4(input.pos, 1.0f), wvp);

    output.pos = pos;

    return output;
}

void main_ps(PixelShaderInput input) : SV_TARGET
{
}
