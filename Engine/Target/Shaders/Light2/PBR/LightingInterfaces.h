#include "../LightingAttributes.h"

static const float M_PI = 3.141592653589793;

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
float3 F_Schlick(float3 f0, float3 f90, float VdotH)
{
	return f0 + (f90 - f0) * pow(saturate(1.0 - VdotH), 5.0);
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float D_GGX(float NdotH, float alphaRoughness)
{
	float alphaRoughnessSq = alphaRoughness * alphaRoughness;
    float f = (NdotH * alphaRoughnessSq - NdotH) * NdotH + 1.0f;
	return alphaRoughnessSq / (M_PI * f * f);
}

// Smith Joint GGX
// Note: Vis = G / (4 * NdotL * NdotV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
	//return 0.5/lerp(2*NdotL*NdotV,NdotL+NdotV,alphaRoughness); //[Hammon17] ugly results
	float alphaRoughnessSq = alphaRoughness * alphaRoughness;

	float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
	float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

	float GGX = GGXV + GGXL;
	if (GGX > 0.0)
	{
		return 0.5 / GGX;
	}
	return 0.0;
}

//https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
float3 BRDF_lambertian(float3 f0, float3 f90, float3 diffuseColor, float VdotH)
{
	// see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
	return (1.0 - F_Schlick(f0, f90, VdotH)) * (diffuseColor / M_PI);
}

//  https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
float3 BRDF_specularGGX(float3 f0, float3 f90, float alphaRoughness, float VdotH, float NdotL, float NdotV, float NdotH)
{
	float3 F = F_Schlick(f0, f90, VdotH);
	float Vis = V_GGX(NdotL, NdotV, alphaRoughness);
	float D = D_GGX(NdotH, alphaRoughness);

	return F * Vis * D;
}

void BRDFNoL(float3 viewDir,float3 WtoL,float3 normal,float3 f0, float3 f90, float3 albedoColor, float roughness, out float3 diffuseColor, out float3 specularColor) {

	float3 halfWay = normalize(WtoL + viewDir);
	float NdotL = saturate(dot(normal, WtoL));
	float NdotV = saturate(dot(normal, viewDir));
	float NdotH = saturate(dot(normal, halfWay));
	float LdotH = saturate(dot(WtoL, halfWay));
	float VdotH = saturate(dot(viewDir, halfWay));

	diffuseColor = 0.0f.xxx;
	specularColor = 0.0f.xxx;

	if (NdotL > 0.0 || NdotV > 0.0)
	{
		diffuseColor += NdotL * BRDF_lambertian(f0, f90, albedoColor, VdotH);
		specularColor += NdotL * BRDF_specularGGX(f0, f90, roughness * roughness, VdotH, NdotL, NdotV, NdotH);
	}
}

interface ILighting
{
  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float3 albedoColor, in float3 f0, in float3 f90, in float roughness, out float3 diffuseLightContribution, out float3 specularLightContribution);
  void Shadow(in ShadowMapAttributes atts, in float3 worldPos, in Texture2D shadowMapTexture, in SamplerState samp, out float shadowMapFactor);
};

class AmbientLighting : ILighting
{
  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float3 albedoColor, in float3 f0, in float3 f90, in float roughness, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
    diffuseLightContribution = atts.lightColor * albedoColor;
    specularLightContribution = 0.0f.xxx;
  }

  void Shadow(in ShadowMapAttributes atts, in float3 worldPos, in Texture2D shadowMapTexture, in SamplerState samp, out float shadowMapFactor)
  {
    shadowMapFactor = 1.0f;
  }
};

class DirectionalLighting : ILighting
{
  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float3 albedoColor, in float3 f0, in float3 f90, in float roughness, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
	  float3 color = atts.lightColor;
    float3 direction = normalize(atts.atts1.xyz);
	
	  float3 diffuseColor = 0.0f.xxx;
	  float3 specularColor = 0.0f.xxx;	
	  BRDFNoL(viewDir, -direction, normal, f0, f90, albedoColor, roughness, diffuseColor, specularColor);

    //compute the final color
    diffuseLightContribution = color * diffuseColor;
    specularLightContribution = color * specularColor;
  }

  void calculateShadowMapFactor(in float4 shadowMapCoords, in Texture2D shadowMapTexture, in SamplerState samp, in float shadowMapBias, in float2 shadowMapInvTexelSize, out float shadowMapFactor)
  {
    //transform the shadowMapCoords into the projection space
    shadowMapCoords.x = 0.5f + 0.5f * shadowMapCoords.x / shadowMapCoords.w;
    shadowMapCoords.y = 0.5f - 0.5f * shadowMapCoords.y / shadowMapCoords.w;
    shadowMapCoords.z /= shadowMapCoords.w;

    //sample the coordinate with adjacent neighbors
    float shadowMapDepth[4];
    shadowMapDepth[0] = shadowMapTexture.Sample(samp, shadowMapCoords.xy).r;
    shadowMapDepth[1] = shadowMapTexture.Sample(samp, shadowMapCoords.xy + float2(shadowMapInvTexelSize.x, 0.0f)).r;
    shadowMapDepth[2] = shadowMapTexture.Sample(samp, shadowMapCoords.xy + float2(0.0f, shadowMapInvTexelSize.y)).r;
    shadowMapDepth[3] = shadowMapTexture.Sample(samp, shadowMapCoords.xy + float2(shadowMapInvTexelSize.xy)).r;

    //get the attenuation of the shadowed area by applying zBias + the sum of the depths
    shadowMapFactor = 1.0f;
    for (int i = 0; i < 4; i++)
    {
      if ((shadowMapDepth[i] + shadowMapBias) < shadowMapCoords.z)
      {
        shadowMapFactor -= 0.25f;
      }
    }
  }

  void Shadow(in ShadowMapAttributes atts, in float3 worldPos, in Texture2D shadowMapTexture, in SamplerState samp, out float shadowMapFactor)
  {
    matrix shadowMapViewProjection = atts.atts0;
    float4 shadowMapCoords = mul(float4(worldPos, 1.0f), shadowMapViewProjection);
    float zBias = atts.atts6.x;
    float2 shadowMapTexelInvSize = atts.atts6.yz;
    calculateShadowMapFactor(shadowMapCoords, shadowMapTexture, samp, zBias, shadowMapTexelInvSize, shadowMapFactor);
  }
};

class SpotLighting : ILighting
{
  float distanceAttenuation(in float distance, in float3 attenuation){
	  return dot(float3(1.0f, distance, distance * distance), attenuation);
  }
  
  float attenuate(in float diffuseFactor, in float cosAngle, in float distance, in float3 attenuation, in float3 normal, in float3 nWtoL)
  {
    float angleAttenuation = (1 - (1 - diffuseFactor) / (1 - cosAngle)) * saturate(dot(normal, -nWtoL));
    return angleAttenuation / distanceAttenuation(distance, attenuation);
  }
  
  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float3 albedoColor, in float3 f0, in float3 f90, in float roughness, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
    float3 color = atts.lightColor;
    float3 position = atts.atts1.xyz;
    float3 direction = atts.atts2.xyz;
    float cosAngle = atts.atts2.w;
    float3 attenuation = atts.atts3.xyz;

    float3 diffuseColor = 0.0f.xxx;
    float3 specularColor = 0.0f.xxx;

    float3 WtoL = position - worldPos;
    float3 nWtoL = normalize(WtoL);
    float3 L = normalize(direction);

    float diffuseFactor = dot(-nWtoL, L);
    //if its inside the spot area
    if (diffuseFactor > cosAngle)
    {
      //attenuate the spot light diffuse contribution      
      BRDFNoL(viewDir, nWtoL, normal, f0, f90, albedoColor, roughness, diffuseColor, specularColor);
      diffuseFactor = attenuate(diffuseFactor, cosAngle, length(WtoL), attenuation, normal, nWtoL);
      diffuseColor *= diffuseFactor;
      specularColor /= distanceAttenuation(length(WtoL), attenuation);
    }
    else
    {
      diffuseColor = 0.0f.xxx;
      specularColor = 0.0f.xxx;
    }

    //compute the final color
    diffuseLightContribution = color * diffuseColor;
    specularLightContribution = color * specularColor;
  }

  void calculateShadowMapFactor(in float4 shadowMapCoords, in Texture2D shadowMapTexture, in SamplerState samp, in float shadowMapBias, in float2 shadowMapInvTexelSize, out float shadowMapFactor)
  {
    //transform the shadowMapCoords into the projection space
    shadowMapCoords.x = 0.5f + 0.5f * shadowMapCoords.x / shadowMapCoords.w;
    shadowMapCoords.y = 0.5f - 0.5f * shadowMapCoords.y / shadowMapCoords.w;
    shadowMapCoords.z /= shadowMapCoords.w;

    //sample the coordinate with adjacent neighbors
    float shadowMapDepth[4];
    shadowMapDepth[0] = shadowMapTexture.Sample(samp, shadowMapCoords.xy).r;
    shadowMapDepth[1] = shadowMapTexture.Sample(samp, shadowMapCoords.xy + float2(shadowMapInvTexelSize.x, 0.0f)).r;
    shadowMapDepth[2] = shadowMapTexture.Sample(samp, shadowMapCoords.xy + float2(0.0f, shadowMapInvTexelSize.y)).r;
    shadowMapDepth[3] = shadowMapTexture.Sample(samp, shadowMapCoords.xy + float2(shadowMapInvTexelSize.xy)).r;

    //get the attenuation of the shadowed area by applying zBias + the sum of the depths
    shadowMapFactor = 1.0f;
    for (int i = 0; i < 4; i++)
    {
      if ((shadowMapDepth[i] + shadowMapBias) < shadowMapCoords.z)
      {
        shadowMapFactor -= 0.25f;
      }
    }
  }

  void Shadow(in ShadowMapAttributes atts, in float3 worldPos, in Texture2D shadowMapTexture, in SamplerState samp, out float shadowMapFactor)
  {
    matrix shadowMapViewProjection = atts.atts0;
    float4 shadowMapCoords = mul(float4(worldPos, 1.0f), shadowMapViewProjection);
    float zBias = atts.atts6.x;
    float2 shadowMapTexelInvSize = atts.atts6.yz;
    calculateShadowMapFactor(shadowMapCoords, shadowMapTexture, samp, zBias, shadowMapTexelInvSize, shadowMapFactor);
  }
};

class PointLighting : ILighting
{
  float distanceAttenuation(in float distance, in float3 attenuation){
	  return dot(float3(1.0f, distance, distance * distance), attenuation);
  }
  
  float attenuate(in float distance, in float3 attenuation, in float3 normal, float3 nWtoL)
  {
    return saturate(dot(normal, -nWtoL)) / distanceAttenuation(distance, attenuation);
  }

  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float3 albedoColor, in float3 f0, in float3 f90, in float roughness, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
    float3 color = atts.lightColor;
    float3 position = atts.atts1.xyz;
    float3 attenuation = atts.atts2.xyz;

    float3 diffuseColor = 0.0f.xxx;
    float3 specularColor = 0.0f.xxx;

    float3 WtoL = position - worldPos;
    float3 nWtoL = normalize(WtoL);

    BRDFNoL(viewDir, nWtoL, normal, f0, f90, albedoColor, roughness, diffuseColor, specularColor);
    float diffuseFactor = attenuate(length(WtoL), attenuation, normal, nWtoL);
    diffuseColor *= diffuseFactor;
    specularColor /= distanceAttenuation(length(WtoL), attenuation);
    /*
    float diffuseFactor = 0.0f;
    float specularFactor = 0.0f;

    //calculate the diffuse contribution from the point light
    float3 WtoL = worldPos - position;
    float3 nWtoL = normalize(WtoL);
    diffuseFactor = attenuate(length(WtoL), attenuation, normal, nWtoL);

    //calculate the point light specular contribution
    float3 HV = normalize(-nWtoL + viewDir);
    float NoHV = dot(normal, HV);
    specularFactor = pow(saturate(NoHV), specularExponent);

    diffuseLightContribution = color * diffuseFactor;
    specularLightContribution = color * specularFactor / distanceAttenuation(length(WtoL), attenuation);
	*/
    //compute the final color
    diffuseLightContribution = color * diffuseColor;
    specularLightContribution = color * specularColor;
  }

  void getPointShadowMapFactor(in float3 worldPos, in matrix shadowMapProjection[6], in Texture2D shadowMap, in SamplerState samp, in float shadowMapBias, out float shadowMapFactor)
  {
    shadowMapFactor = 1.0f;
    for (int i = 0; i < 6; i++)
    {
      float4 shadowMapCoords = mul(float4(worldPos, 1.0f), shadowMapProjection[i]);
      shadowMapCoords.xyz /= shadowMapCoords.w;
      //check if projected coords are in boundary for this shadow map projection
      if (shadowMapCoords.x > -1.0f && shadowMapCoords.x < 1.0f && shadowMapCoords.y > -1.0f && shadowMapCoords.y < 1.0f && shadowMapCoords.z > 0.0f && shadowMapCoords.z < 1.0f)
      {
        shadowMapCoords.x = 0.5f + 0.5f * shadowMapCoords.x;
        shadowMapCoords.y = 1.0f / 6.0f * i + (0.5f - 0.5f * shadowMapCoords.y) * 1.0f / 6.0f;
        float shadowMapDepth = shadowMap.Sample(samp, shadowMapCoords.xy).r;
        if ((shadowMapDepth + shadowMapBias) < shadowMapCoords.z)
        {
          shadowMapFactor = 0.0f;
          break;
        }
      }
    }
  }

  void calculateShadowMapFactor(in float3 worldPos, in matrix shadowMapProjection[6], in Texture2D shadowMap, in SamplerState samp, in float shadowMapBias, in float shadowMapPartialDerivativeScale, out float shadowMapFactor)
  {
    float3 worldPosDdx = ddx(worldPos) * shadowMapPartialDerivativeScale;
    float3 worldPosDdy = ddy(worldPos) * shadowMapPartialDerivativeScale;
    float4 shadowMapFactors;
    getPointShadowMapFactor(worldPos, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.x);
    getPointShadowMapFactor(worldPos + worldPosDdx, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.y);
    getPointShadowMapFactor(worldPos + worldPosDdy, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.z);
    getPointShadowMapFactor(worldPos + worldPosDdx + worldPosDdy, shadowMapProjection, shadowMap, samp, shadowMapBias, shadowMapFactors.w);
    shadowMapFactor = dot(float(0.25f).xxxx, shadowMapFactors);
  }

  void Shadow(in ShadowMapAttributes atts, in float3 worldPos, in Texture2D shadowMapTexture, in SamplerState samp, out float shadowMapFactor)
  {
    matrix shadowMapProjection[6] = {
        atts.atts0, atts.atts1, atts.atts2,
        atts.atts3, atts.atts4, atts.atts5
    };
    float ZBias = atts.atts6.x;
    float PartialDerivativeScale = atts.atts6.y;

    calculateShadowMapFactor(worldPos, shadowMapProjection, shadowMapTexture, samp, ZBias, PartialDerivativeScale, shadowMapFactor);
  }
};
