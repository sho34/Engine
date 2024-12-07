float calculateSpotLightAttenuationFactor(float diffuseFactor, float angle, float3 diff, float3 attenuation, float3 normal, float3 spotLightVector) {
	float spotLightDiffuseFactor = 1 - (1 - diffuseFactor) / (1 - angle);
	float spotLightDistance = length(diff);
	float spotLightDistanceAttenuation = dot(float3(1.0f, spotLightDistance, spotLightDistance * spotLightDistance), attenuation);
	spotLightDiffuseFactor *= saturate(dot(normal, -spotLightVector));
	spotLightDiffuseFactor = spotLightDiffuseFactor / spotLightDistanceAttenuation;
	return spotLightDiffuseFactor;
}