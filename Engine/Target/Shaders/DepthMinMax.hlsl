struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float2 uv2 : TEXCOORD2;
	float2 uv3 : TEXCOORD3;
};

Texture2D texMin : register(t0);
Texture2D texMax : register(t1);
SamplerState samp0 : register(s0);

cbuffer renderable : register(b0)
{
	#include "CBVars/FullScreenQuad.h"
}

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	output.pos = float4(input.pos.xy,1.0f.xx);
	output.uv0 = input.uv + float2(-texelInvSize.x,-texelInvSize.y);
	output.uv1 = input.uv + float2( texelInvSize.x,-texelInvSize.y);
	output.uv2 = input.uv + float2(-texelInvSize.x, texelInvSize.y);
	output.uv3 = input.uv + float2( texelInvSize.x, texelInvSize.y);

	return output;
}

struct PS_OUTPUT
{
    float Min: SV_Target0;
    float Max: SV_Target1;
};

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

PS_OUTPUT main_ps(PixelShaderInput input)
{
	PS_OUTPUT output;
	float4 depmin = depthSampleArea(input.uv0, input.uv1, input.uv2, input.uv3, texMin, samp0);
	float4 depmax = depthSampleArea(input.uv0, input.uv1, input.uv2, input.uv3, texMax, samp0);
	
	output.Min = min(depmin.x,min(depmin.y,min(depmin.z,depmin.w)));
	output.Max = max(depmax.x,max(depmax.y,max(depmax.z,depmax.w)));
	
	return output;
}
