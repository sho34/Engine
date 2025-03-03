#include "Utils/Gamma.h"
#include "Utils/TangentSpace.h"
#include "Camera/Camera.h"
#include "Lighting/BlinnPhong/SceneLighting.h"

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
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
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

#if defined(_HAS_BASE_TEXTURE)
Texture2D BaseTexture : register(t0);
#endif
#if defined(_HAS_NORMALMAP_TEXTURE)
Texture2D NormalMapTexture : register(t1);
#endif
#if defined(_HAS_SHADOWMAPS_TEXTURES)
Texture2D ShadowMapsTextures[] : register(t2);
#endif
#if defined(_HAS_BASE_TEXTURE) || defined(_HAS_NORMALMAP_TEXTURE) || defined(_HAS_SHADOWMAPS_TEXTURES)
SamplerState sampler0 : register(s0);
#endif

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	matrix wvp = mul(world,camera.viewProjection);
	
	float4 pos = mul(float4(input.pos, 1.0f),wvp);
    float3 normal = mul(input.normal, (float3x3) world);
    float3 tangent = mul(input.tangent, (float3x3) world);
    float3 worldPos = mul(float4(input.pos, 1.0f), world).xyz;
    float3 viewDirection = camera.eyePosition.xyz - worldPos;
    float2 uv = input.uv;

    output.pos = pos;
    output.normal = normal;
    output.tangent = tangent;
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
    float3 tangent = normalize(input.tangent);
    float3 viewDir = normalize(input.viewDirection);
    float2 uv = input.uv;

	#if defined(_HAS_BASE_TEXTURE)
	float4 texturesColor = BaseTexture.Sample(sampler0, uv);
	if(texturesColor.a < alphaCut) {
		discard;
	}
	#else
	float4 texturesColor = 1.xxxx;
	#endif

    float3 diffuseFinalLightContribution = 0.xxx;
    float3 specularFinalLightContribution = 0.xxx;

    //renderable (object + material) (CBV)
    //camera (CBV)
    //lights (CBV)
    //shadowmaps (CBV)
    //shadowmaps textures (SRV)
    //sampler
    
	#if defined(_HAS_NORMALMAP_TEXTURE)
    normal = getNormal(normal, tangent, uv, NormalMapTexture, sampler0);
	#endif
    
    SceneLighting(
        worldPos, normal, specularExponent,
        viewDir,
        lights,
		#if defined(_HAS_SHADOWMAPS_TEXTURES)
        shadowMaps,
        ShadowMapsTextures,
        sampler0,
		#endif
        diffuseFinalLightContribution, specularFinalLightContribution
    );

    
    float3 outputColor = texturesColor.rgb*diffuseFinalLightContribution;
	outputColor = saturate(outputColor);
	outputColor+= specularFinalLightContribution;
	
	return float4(toGammaSpace(outputColor), 1.0f);
}
