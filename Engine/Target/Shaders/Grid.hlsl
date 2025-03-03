#include "Utils/Gamma.h"
#include "Camera/Camera.h"
#include "Lighting/BlinnPhong/SceneLighting.h"

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

cbuffer renderable : register(b0)
{
	#include "CBVars/Model.h"
	#include "CBVars/Material.h"
};

ConstantBuffer<Camera> camera : register(b1);
ConstantBuffer<Lights> lights : register(b2);
#if defined(_HAS_SHADOWMAPS_TEXTURES)
ConstantBuffer<ShadowMaps> shadowMaps : register(b3);
Texture2D ShadowMapsTextures[] : register(t0);
SamplerState sampler0 : register(s0);
#endif

PixelShaderInput main_vs(VertexShaderInput input)
{
    PixelShaderInput output;

    matrix wvp = mul(world, camera.viewProjection);
	
    float4 pos = mul(float4(input.pos, 1.0f), wvp);
    float3 normal = mul(input.normal, (float3x3) world);
    float3 worldPos = mul(float4(input.pos, 1.0f), world).xyz;
    float3 viewDirection = camera.eyePosition.xyz - worldPos;

    output.pos = pos;
    output.normal = normal;
    output.viewDirection = viewDirection;
    output.worldPos = worldPos;
    return output;
}

float3 getGridColor(PixelShaderInput input)
{
    float3 gridColor = 1.0f.xxx;
    float2 coord = input.worldPos.xz;
    
	//grid based on
	//grilla basada de
	//http://madebyevan.com/shaders/grid/
    float2 grid1 = abs(frac(coord - 0.5f) - 0.5f) / fwidth(coord);
    float l1 = min(grid1.x, grid1.y);
    float unitColor = float(1.0f - min(l1, 1.0f));

    float2 grid2 = abs(frac((coord - 0.05f) * 10.0f) - 0.5f) / fwidth(coord * 10.0f);
    float l2 = min(grid2.x, grid2.y);
    float subUnitColor = float(1.0f - min(l2, 1.0f));

    gridColor.rgb -= 0.5f * unitColor.xxx + 0.3f * subUnitColor.xxx;
    return gridColor;
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
		#if defined(_HAS_SHADOWMAPS_TEXTURES)
        shadowMaps,
        ShadowMapsTextures,
        sampler0,
		#endif
        diffuseFinalLightContribution, specularFinalLightContribution
    );
    
    float3 outputColor = getGridColor(input) * diffuseFinalLightContribution;
    outputColor = saturate(outputColor);
    outputColor += specularFinalLightContribution;
    
    return float4(toGammaSpace(outputColor), 1.0f);
}
