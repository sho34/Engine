struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv0 : TEXCOORD0;
};

Texture2D texDepth : register(t0);
Texture2D texMin : register(t1);
Texture2D texMax : register(t2);
SamplerState samp0 : register(s0);

cbuffer renderable : register(b0)
{
	#include "CBVars/FullScreenQuad.h"
}

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	output.pos = float4(input.pos.xy,1.0f.xx);
	output.uv0 = input.uv;

	return output;
}

float4 depthSampleArea(in float2 coords0, in float2 coords1, in float2 coords2, in float2 coords3, Texture2D texture, in SamplerState samp)
{
	//sample the coordinate with adjacent neighbors
	float4 depthValues;
	depthValues.x = texture.Sample(samp, coords0.xy).r;
	depthValues.y = texture.Sample(samp, coords1.xy).r;
	depthValues.z = texture.Sample(samp, coords2.xy).r;
	depthValues.w = texture.Sample(samp, coords3.xy).r;
	
	return depthValues;
}

float4 main_ps(PixelShaderInput input) : SV_TARGET
{
	float4 depmin = depthSampleArea(float2(0.25f,0.25f), float2(0.75f,0.25f), float2(0.25f,0.75f), float2(0.75f,0.75f), texMin, samp0);
	float4 depmax = depthSampleArea(float2(0.25f,0.25f), float2(0.75f,0.25f), float2(0.25f,0.75f), float2(0.75f,0.75f), texMax, samp0);
	
	float Min = min(depmin.x,min(depmin.y,min(depmin.z,depmin.w)));
	float Max = max(depmax.x,max(depmax.y,max(depmax.z,depmax.w)));
	
	float depth = texDepth.Sample(samp0, input.uv0.xy).r;
	
	float ndepth = (depth - Min)/(Max - Min);
	
	return float4(ndepth.xxx,1.0f);
}
