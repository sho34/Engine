cbuffer cbObject{
	matrix wvp;
	float4 outputColor;
	float4 outputColor2;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
};

PixelShaderInput main_vs(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 pos = mul(float4(input.pos, 1.0f),wvp);
	output.pos = pos;
	return output;
}

float4 main_ps(PixelShaderInput input) : SV_TARGET
{
	return float4(outputColor.xyz*outputColor2.xyz, 1.0f);
}
