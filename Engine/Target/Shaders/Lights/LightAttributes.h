enum LightTypes {
    AMBIENT = 0,
    DIRECTIONAL,
    SPOT,
    POINT
};

struct LightAttributes {
  LightTypes lightType;
  float3 lightColor;
  float4 atts1;
  float4 atts2;
  float4 atts3;
};