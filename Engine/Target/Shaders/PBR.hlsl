#include "Utils/Gamma.h"
#include "Utils/TangentSpace.h"
#include "Camera/Camera.h"
#include "Lighting/PBR/SceneLighting.h"
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
	float3 normal: NORMAL;
	float3 tangent: TANGENT;
	float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

cbuffer renderable : register(b0) {
	#include "CBVars/Model.h"
	#include "CBVars/Material.h"
}

ConstantBuffer<Camera> camera : register(b1);
ConstantBuffer<Lights> lights : register(b2);
#if defined(_HAS_SHADOWMAPS_TEXTURES)
ConstantBuffer<ShadowMaps> shadowMaps : register(b3);
#endif
#ifdef _HAS_SKINNING
ConstantBuffer<Animated3D> animation : register(b4);
#endif

#if defined(_HAS_BASE_TEXTURE)
Texture2D BaseTexture : register(t0);
#endif
#if defined(_HAS_NORMALMAP_TEXTURE)
Texture2D NormalMapTexture : register(t1);
#endif
#if defined(_HAS_METALLIC_ROUGHNESS_TEXTURE)
Texture2D MetallicRoughnessTexture : register(t2);
#endif
#if defined(_HAS_SHADOWMAPS_TEXTURES)
Texture2D ShadowMapsTextures[] : register(t3);
#endif
#if defined(_HAS_BASE_TEXTURE) || defined(_HAS_NORMALMAP_TEXTURE) || defined(_HAS_METALLIC_ROUGHNESS_TEXTURE) || defined(_HAS_SHADOWMAPS_TEXTURES)
SamplerState sampler0 : register(s0);
#endif

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;
	
	matrix wvp = mul(world,camera.viewProjection);
	
	#ifdef _HAS_SKINNING
	matrix boneTransform = animation.boneMatrices[input.boneIds.x] * input.boneWeights.x +
		animation.boneMatrices[input.boneIds.y] * input.boneWeights.y +
		animation.boneMatrices[input.boneIds.z] * input.boneWeights.z +
		animation.boneMatrices[input.boneIds.w] * input.boneWeights.w;

	float3 skinnedPos = mul(float4(input.pos, 1.0f), boneTransform).xyz;
	float3 skinnedNormal = mul(input.normal, (float3x3)boneTransform);
	float3 skinnedTangent = mul(input.tangent, (float3x3)boneTransform);
	
	float4 pos = mul(float4(skinnedPos, 1.0f),wvp);
    float3 normal = mul(skinnedNormal, (float3x3) world);
    float3 tangent = mul(skinnedTangent, (float3x3) world);
    float3 worldPos = mul(float4(skinnedPos, 1.0f), world).xyz;
	#else
	float4 pos = mul(float4(input.pos, 1.0f),wvp);
    float3 normal = mul(input.normal, (float3x3) world);
    float3 tangent = mul(input.tangent, (float3x3) world);
    float3 worldPos = mul(float4(input.pos, 1.0f), world).xyz;	
	#endif
	
    float2 uv = input.uv;

    output.pos = pos;
    output.normal = normal;
    output.tangent = tangent;
    output.uv = uv;
    output.worldPos = worldPos;
	output.uv = input.uv;

	return output;
}

#define occlusionStrength (0.3f)
static const float c_MinRoughness = 0.04;
static const float f0_ior = 0.04f;

float4 main_ps(PixelShaderInput input) : SV_TARGET
{   
	float3 worldPos = input.worldPos;
    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 viewDir = normalize(camera.eyePosition.xyz - input.worldPos);
    float2 uv = input.uv;
	
	float4 baseColor = BaseTexture.Sample(sampler0, uv);
	
	if(baseColor.a < alphaCut) {
		discard;
	}
	
	float3 diffuseFinalLightContribution = 0.xxx;
    float3 specularFinalLightContribution = 0.xxx;

    normal = getNormal(normal, tangent, uv, NormalMapTexture, sampler0);

	float3 ARM = MetallicRoughnessTexture.Sample(sampler0, uv).xyz; //AO + Roughness + Metallic
	
	float roughness = ARM.y * roughnessFactor;
    roughness = clamp(roughness, c_MinRoughness, 1.0f);
	float metallic = ARM.z * metallicFactor;

	float3 f0 = float3(f0_ior.xxx);

	//calculate the albedo
	float3 albedoColor = lerp(baseColor.rgb * (1.0f.xxx - f0), 0.0f.xxx, metallic);
	f0 = lerp(f0, baseColor.rgb, metallic.xxx);

	//calculate the reflectance
	float reflectance = max(max(f0.r, f0.g), f0.b);
	// Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
	float3 f90 = float3(saturate(reflectance * 50.0).xxx);
	
    //renderable (object) (CBV)
    //camera (CBV)
    //lights (CBV)
    //shadowmaps (CBV)
    //shadowmaps textures (SRV)
    //sampler	
	SceneLighting(
		worldPos, normal,
		albedoColor, f0, f90, roughness,
		viewDir,
		lights,
		#if defined(_HAS_SHADOWMAPS_TEXTURES)
		shadowMaps,
		ShadowMapsTextures,
		sampler0,
		#endif
		diffuseFinalLightContribution, specularFinalLightContribution
	);	
	
	float4 outputColor = float4(diffuseFinalLightContribution + specularFinalLightContribution, 1.0f);
	float ambientOcclusion = ARM.x;
    outputColor.xyz = lerp(outputColor.xyz, outputColor.xyz * ambientOcclusion, occlusionStrength);
	outputColor.xyz = toGammaSpace(outputColor.xyz);
	return outputColor;
}