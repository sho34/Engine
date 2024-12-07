#define MAX_LIGHTS 100

enum LightTypes
{
  AMBIENT = 0,
  DIRECTIONAL,
  SPOT,
  POINT
};

struct LightAttributes
{
  LightTypes lightType;
  float3 lightColor;
  float4 atts1;
  float4 atts2;
  float3 atts3;
  bool hasShadowMap;
};

struct Lights
{
  uint numLights;
  LightAttributes atts[MAX_LIGHTS];
};

struct ShadowMapAttributes
{
  matrix atts0;
  matrix atts1;
  matrix atts2;
  matrix atts3;
  matrix atts4;
  matrix atts5;
  float4 atts6;
};

struct ShadowMaps
{
  ShadowMapAttributes atts[MAX_LIGHTS];
};