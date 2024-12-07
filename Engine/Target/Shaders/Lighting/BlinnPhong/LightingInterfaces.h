#include "../LightingAttributes.h"

interface ILighting
{
  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float specularExponent, out float3 diffuseLightContribution, out float3 specularLightContribution);
  void Shadow(in ShadowMapAttributes atts, in float3 worldPos, in Texture2D shadowMapTexture, in SamplerState samp, out float shadowMapFactor);
};

class AmbientLighting : ILighting
{
  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float specularExponent, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
    diffuseLightContribution = atts.lightColor;
    specularLightContribution = 0.0f.xxx;
  }

  void Shadow(in ShadowMapAttributes atts, in float3 worldPos, in Texture2D shadowMapTexture, in SamplerState samp, out float shadowMapFactor)
  {
    shadowMapFactor = 1.0f;
  }
};

class DirectionalLighting : ILighting
{
  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float specularExponent, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
    float3 color = atts.lightColor;
    float3 direction = atts.atts1.xyz;

    //calculate the diffuse contribution from the directional light D=NoL
    float3 L = -normalize(direction);
    float diffuseFactor = saturate(dot(normal, L));

    //calculate the specular contribution from the directional light HV = n(L+V) -> S=sat(NoHV)^spec
    float3 HV = normalize(L + viewDir);
    float NoHV = dot(normal, HV);
    float specularFactor = pow(saturate(NoHV), specularExponent);

    //compute the final color
    diffuseLightContribution = color * diffuseFactor;
    specularLightContribution = color * specularFactor;
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

  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float specularExponent, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
    float3 color = atts.lightColor;
    float3 position = atts.atts1.xyz;
    float3 direction = atts.atts2.xyz;
    float cosAngle = atts.atts2.w;
    float3 attenuation = atts.atts3.xyz;

    float diffuseFactor = 0.0f;
    float specularFactor = 0.0f;

    //calculate the diffuse contribution from the spot light
    float3 WtoL = worldPos - position;
    float3 nWtoL = normalize(WtoL);
    float3 L = normalize(direction);
    diffuseFactor = dot(nWtoL, L);

    //if its inside the spot area
    if (diffuseFactor > cosAngle)
    {
      //attenuate the spot light diffuse contribution
      diffuseFactor = attenuate(diffuseFactor, cosAngle, length(WtoL), attenuation, normal, nWtoL);

      //calculate the spot light specular contribution
      float3 HV = normalize(-nWtoL + viewDir);
      float NoHV = dot(normal, HV);
      specularFactor = pow(saturate(NoHV), specularExponent) / distanceAttenuation(length(WtoL), attenuation);
    }
    else
    {
      diffuseFactor = 0.0f;
      specularFactor = 0.0f;
    }

    diffuseLightContribution = color * diffuseFactor;
    specularLightContribution = color * specularFactor;
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

  void Compute(in LightAttributes atts, in float3 normal, in float3 viewDir, in float3 worldPos, in float specularExponent, out float3 diffuseLightContribution, out float3 specularLightContribution)
  {
    float3 color = atts.lightColor;
    float3 position = atts.atts1.xyz;
    float3 attenuation = atts.atts2.xyz;

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
