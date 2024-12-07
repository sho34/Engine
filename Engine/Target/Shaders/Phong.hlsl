#include "Utils/Gamma.h"
#include "Lights/Lights.h"
#include "Lights/Phong.h"

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

cbuffer scene {
	#include "CBVars/Model.h"
	#include "CBVars/Camera.h"
	#include "CBVars/Material.h"
	#include "CBVars/Lights.h"
};

ConstantBuffer<Lights> lights;

Texture2D texs[] : register(t0);
SamplerState samp0 : register(s0);

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f),wvp);
    float3 normal = mul(input.normal, (float3x3) world);
    float3 worldPos = mul(float4(input.pos, 1.0f), world).xyz;
    float3 viewDirection = eyePos.xyz - worldPos;
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
	float3 texturesColor = 0.0f.xxx;
	for (uint i = 0; i < numTextures; i++) {
		texturesColor += texs[i].Sample(samp0, input.uv).rgb;
	}

    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.viewDirection);

	float3 outputColor = 0.xxx;
	
	float3 diffuseFinalLightContribution = 0.xxx;
	float3 specularFinalLightContribution = 0.xxx;
	
	for(uint i = 0 ; i < numLights; i++) {
        float3 diffuseLightContribution = 0.xxx;
        float3 specularLightContribution = 0.xxx;
		
		switch(lights.atts[i].lightType){
		    case AMBIENT:
				{
					AmbientLight ambientLight = getAmbientLight(lights.atts[i]);
					diffuseLightContribution += ambientLight.color;
				}
			break;
			case DIRECTIONAL:
				{
					DirectionalLight dirLight  = getDirectionalLight(lights.atts[i]);
					float diffuseFactor = 0.0f;
					float specularFactor = 0.0f;
					calculateDirectionalLightFactorsPhong(normal, viewDir, dirLight.direction, specularExponent ,diffuseFactor, specularFactor);
					diffuseLightContribution = dirLight.color * diffuseFactor;
					specularLightContribution = dirLight.color * specularFactor;
				}
			break;
			case SPOT:
				{
					SpotLight spotLight = getSpotLight(lights.atts[i]);
                    float diffuseFactor = 0.0f;
                    float specularFactor = 0.0f;
                    calculateSpotLightFactorsPhong(input.worldPos, normal, viewDir, spotLight.position, spotLight.direction, spotLight.angle, spotLight.attenuation, specularExponent, diffuseFactor, specularFactor);
                    diffuseLightContribution = spotLight.color * diffuseFactor;
                    specularLightContribution = spotLight.color * specularFactor;
                }
			break;
			case POINT:
				{
                    PointLight pointLight = getPointLight(lights.atts[i]);
                    float diffuseFactor = 0.0f;
                    float specularFactor = 0.0f;
                    calculatePointLightFactorsPhong(input.worldPos, normal, viewDir, pointLight.position, pointLight.attenuation, specularExponent, diffuseFactor, specularFactor);
                    diffuseLightContribution = pointLight.color * diffuseFactor;
                    specularLightContribution = pointLight.color * specularFactor;
                }
			break;
		}
		
        diffuseFinalLightContribution = diffuseLightContribution;
        specularFinalLightContribution = specularLightContribution;
	}
	
    outputColor += diffuseFinalLightContribution + specularFinalLightContribution;
	outputColor = saturate(outputColor);
	outputColor*= texturesColor;
	
	return float4(toGammaSpace(outputColor), 1.0f);
}
