#include "Utils/Gamma.h"
#include "Camera/Camera.h"
#include "Light2/BlinnPhong/SceneLighting.h"

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewDirection : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};

cbuffer renderable : register(b0) {
	#include "CBVars/Model.h"
	#include "CBVars/Material.h"
}

ConstantBuffer<Camera> camera : register(b1);
ConstantBuffer<Lights> lights : register(b2);
ConstantBuffer<ShadowMaps> shadowMaps : register(b3);

Texture2D baseTexture : register(t0);
Texture2D shadowMapsTextures[] : register(t1);
SamplerState sampler0 : register(s0);

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	matrix wvp = mul(world,camera.viewProjection);
	
	float4 pos = mul(float4(input.pos, 1.0f),wvp);
    float3 normal = mul(input.normal, (float3x3) world);
    float3 worldPos = mul(float4(input.pos, 1.0f), world).xyz;
    float3 viewDirection = camera.eyePosition.xyz - worldPos;
    float2 uv = input.uv;

    output.pos = pos;
    output.normal = normal;
    output.uv = uv;
    output.viewDirection = viewDirection;
    output.worldPos = worldPos;
	output.uv = input.uv;
	return output;
}

float4 main_ps(PixelShaderInput input) : SV_TARGET
{
    float3 worldPos = input.worldPos;
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.viewDirection);

    float3 diffuseFinalLightContribution = 0.xxx;
    float3 specularFinalLightContribution = 0.xxx;

    //renderable (object + material) (CBV)
    //camera (CBV)
    //lights (CBV)
    //shadowmaps (CBV)
    //shadowmaps textures (SRV)
    //sampler
    
    SceneLighting(
        worldPos, normal, specularExponent,
        viewDir,
        lights,
        shadowMaps,
        shadowMapsTextures,
        sampler0,
        diffuseFinalLightContribution, specularFinalLightContribution
    );

    float3 texturesColor = baseTexture.Sample(sampler0, input.uv).rgb;
    float3 outputColor = texturesColor*diffuseFinalLightContribution;
	outputColor = saturate(outputColor);
	outputColor+= specularFinalLightContribution;
	
	return float4(toGammaSpace(outputColor), 1.0f);
}
