#include "LightingInterfaces.h"

void SceneLighting(
  in float3 worldPos, in float3 normal, in float specularExponent,
  in float3 viewDir,
  Lights lights,
  #if defined(_HAS_SHADOWMAPS_TEXTURES)
  ShadowMaps shadowMaps,
  Texture2D shadowMapsTextures[],
  SamplerState sampler0,
  #endif
  inout float3 diffuseFinalLightContribution, inout float3 specularFinalLightContribution
) {
  AmbientLighting ambientL;
  DirectionalLighting directionalL;
  SpotLighting spotL;
  PointLighting pointL;

  for (uint i = 0; i < lights.numLights; i++)
  {
    float3 diffuseLightContribution = 0.xxx;
    float3 specularLightContribution = 0.xxx;
    float shadowMapFactor = 1;
	uint smi = lights.atts[i].shadowMapIndex;

    switch (lights.atts[i].lightType)
    {
    case AMBIENT:
    {
      ambientL.Compute(lights.atts[i], normal, viewDir, worldPos, specularExponent, diffuseLightContribution, specularLightContribution);
    }
    break;
    case DIRECTIONAL:
    {
      directionalL.Compute(lights.atts[i], normal, viewDir, worldPos, specularExponent, diffuseLightContribution, specularLightContribution);
	  #if defined(_HAS_SHADOWMAPS_TEXTURES)
      if (lights.atts[i].hasShadowMap)
      {
        directionalL.Shadow(shadowMaps.atts[smi], worldPos, shadowMapsTextures[smi], sampler0, shadowMapFactor);
      }
	  #endif
    }
    break;
    case SPOT:
    {
      spotL.Compute(lights.atts[i], normal, viewDir, worldPos, specularExponent, diffuseLightContribution, specularLightContribution);
	  #if defined(_HAS_SHADOWMAPS_TEXTURES)
      if (lights.atts[i].hasShadowMap) {
        spotL.Shadow(shadowMaps.atts[smi], worldPos, shadowMapsTextures[smi], sampler0, shadowMapFactor);
      }
	  #endif
    }
    break;
    case POINT:
    {
      pointL.Compute(lights.atts[i], normal, viewDir, worldPos, specularExponent, diffuseLightContribution, specularLightContribution);
	  #if defined(_HAS_SHADOWMAPS_TEXTURES)
      if (lights.atts[i].hasShadowMap)
      {
        pointL.Shadow(shadowMaps.atts[smi], worldPos, shadowMapsTextures[smi], sampler0, shadowMapFactor);
      }
	  #endif
    }
    break;
    default:
      break;
    }

    diffuseFinalLightContribution += diffuseLightContribution * shadowMapFactor;
    specularFinalLightContribution += specularLightContribution * shadowMapFactor;
  }
}