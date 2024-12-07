float3 getNormal(float3 n, float3 t, float2 uv, Texture2D normalMap, SamplerState samp)
{
    t = normalize(t);
    t = normalize(t - n * dot(n, t)); //recalculate tangent
    float3 b = normalize(cross(n, t)); //build biTangent
    float3x3 ts2ws = float3x3(t, b, n); //tangent space to world space
    float3 nts = normalMap.Sample(samp, uv).rgb * 2.0f - 1.0f; //normal in tangent space
    return normalize(mul(nts, ts2ws));
}