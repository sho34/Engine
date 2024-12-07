#include "Attenuation.h"

void calculateDirectionalLightFactorsPhong(float3 normal, float3 viewDir, float3 directionalLightDirection, float materialSpecularExponent, out float directionalDiffuseLightFactor, out float directionalLightSpecularFactor) {
	//calculate the diffuse contribution from the directional light
	//calcular la contribucion difusa de la luz direccional
	float3 directionalLightVector = -normalize(directionalLightDirection);
	directionalDiffuseLightFactor = saturate(dot(normal, directionalLightVector));

	//calculate the specular contribution from the directional light
	//calcular la contribucion especular de la luz direcional
	float3 directionalLightReflect = normalize(reflect(-directionalLightVector, normal));
	float viewDotDirectionalLight = dot(viewDir, directionalLightReflect);
	directionalLightSpecularFactor = pow(saturate(viewDotDirectionalLight), materialSpecularExponent);
}

void calculateSpotLightFactorsPhong(float3 worldPos, float3 normal, float3 viewDir, float3 spotLightPosition, float3 spotLightDirection, float spotLightAngle, float3 spotLightAttenuation, float materialSpecularExponent, out float spotLightDiffuseFactor, out float spotLightSpecularFactor) {
	//calculate the diffuse contribution from the spot light
	//calcular la contribucion difusa de la luz spot
	float3 spotLightDiff = worldPos - spotLightPosition;
	float3 spotLightVector = normalize(spotLightDiff);
	float3 spotLightDirectionVector = normalize(spotLightDirection);
	spotLightDiffuseFactor = dot(spotLightVector, spotLightDirectionVector);

	//if it's inside the spot area
	//si es que estamos dentro del area de spot
	if (spotLightDiffuseFactor > spotLightAngle) {
		//attenuate the spot light contribution
		//atenuar la contribucion de la luz spot
		spotLightDiffuseFactor = calculateSpotLightAttenuationFactor(spotLightDiffuseFactor, spotLightAngle, spotLightDiff, spotLightAttenuation, normal, spotLightVector);

		//calculate the spot light specular contribution
		//calcular la contribucion specular de la luz spot
		float3 spotLightReflect = normalize(reflect(spotLightVector, normal));
		float viewDotSpotLight = dot(viewDir, spotLightReflect);
		spotLightSpecularFactor = pow(saturate(viewDotSpotLight), materialSpecularExponent);
	}
	else {
		spotLightDiffuseFactor = 0.0f;
		spotLightSpecularFactor = 0.0f;
	}
}

void calculatePointLightFactorsPhong(float3 worldPos, float3 normal, float3 viewDir, float3 pointLightPosition, float3 pointLightAttenuation, float materialSpecularExponent, out float pointLightDiffuseFactor, out float pointLightSpecularFactor) {
	//calculate the diffuse contribution from the point light
	//calcula la contribucion difusa de la luz omni-direcional
	float3 pointLightDiff = worldPos - pointLightPosition;
	float3 pointLightVector = normalize(pointLightDiff);
	float pointLightDistance = length(pointLightDiff);
	float pointLightDistanceAttenuation = pointLightAttenuation.x + pointLightDistance * pointLightAttenuation.y + pointLightDistance * pointLightDistance * pointLightAttenuation.z;
	pointLightDiffuseFactor = saturate(dot(normal, -pointLightVector)) / pointLightDistanceAttenuation;

	//calculate the specular contribution from the point light
	//calcula la contribucion especular de la luz omni-direcional
	float3 pointLightReflect = normalize(reflect(pointLightVector, normal));
	float viewDotPointLight = dot(viewDir, pointLightReflect);
	pointLightSpecularFactor = pow(saturate(viewDotPointLight), materialSpecularExponent);
}
