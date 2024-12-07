#pragma once
using namespace DirectX;

struct LightingShaderConstants {
  UINT numTextures;
  BOOL useBlinnPhong;
  FLOAT materialSpecularExponent;
  BOOL normalMaps;
  UINT normalMapTextureIndex;
  BOOL shadowMaps;
  UINT shadowMapsTextureIndex;
  BOOL hasAlphaCut;
  FLOAT alphaCut;
  XMMATRIX worldViewProjection;
  XMMATRIX world;

  XMMATRIX directionalLightShadowMapProjection;
  FLOAT directionalLightShadowMapZBias;
  XMFLOAT2 directionalLightShadowMapTexelInvSize;

  XMMATRIX spotLightShadowMapProjection;
  FLOAT spotLightShadowMapZBias;
  XMFLOAT2 spotLightShadowMapTexelInvSize;

  XMMATRIX pointLightShadowMapProjection[6];
  FLOAT pointLightShadowMapZBias;
  FLOAT pointLightShadowMapPartialDerivativeScale;

  XMVECTOR eyePos;
  XMVECTOR ambientLightColor;
  XMVECTOR directionalLightDirection;
  XMVECTOR directionalLightColor;
  XMVECTOR spotLightPosition;
  XMVECTOR spotLightColor;
  XMVECTOR spotLightDirectionAndAngle;
  XMVECTOR spotLightAttenuation;
  XMVECTOR pointLightColor;
  XMVECTOR pointLightPosition;
  XMVECTOR pointLightAttenuation;
};