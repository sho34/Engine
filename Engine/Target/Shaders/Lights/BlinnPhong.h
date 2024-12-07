#include "Lights.h"
#include "Attenuation.h"

void calculateDirectionalLightFactorsBlinnPhong(float3 normal, float3 viewDir, float3 directionalLightDirection, float materialSpecularExponent, out float directionalDiffuseLightFactor, out float directionalLightSpecularFactor) {
	//calculate the diffuse contribution from the directional light
	//calcular la contribucion difusa de la luz direccional
	float3 directionalLightVector = -normalize(directionalLightDirection);
	directionalDiffuseLightFactor = saturate(dot(normal, directionalLightVector));

	//calculate the specular contribution from the directional light
	//calcular la contribucion especular de la luz direcional
	float3 halfWayDirectionalLight = normalize(directionalLightVector + viewDir);
	float normalDotHalfWayDirectional = dot(normal, halfWayDirectionalLight);
	directionalLightSpecularFactor = pow(saturate(normalDotHalfWayDirectional), materialSpecularExponent);
}

void calculateSpotLightFactorsBlinnPhong(float3 worldPos, float3 normal, float3 viewDir, float3 spotLightPosition, float3 spotLightDirection, float spotLightAngle, float3 spotLightAttenuation, float materialSpecularExponent, out float spotLightDiffuseFactor, out float spotLightSpecularFactor) {
	//calculate the diffuse contribution from the spot light
	//calcular la contribucion difusa de la luz spot
	float3 spotLightDiff = worldPos - spotLightPosition;
	float3 spotLightVector = normalize(spotLightDiff);
	float3 spotLightDirectionVector = normalize(spotLightDirection);
	spotLightDiffuseFactor = dot(spotLightVector, spotLightDirectionVector);

	//if its inside the spot area
	//si es que estamos dentro del area de spot
	if (spotLightDiffuseFactor > spotLightAngle) {
		//attenuate the spot light contribution
		//atenuar la contribucion de la luz spot
		spotLightDiffuseFactor = calculateSpotLightAttenuationFactor(spotLightDiffuseFactor, spotLightAngle, spotLightDiff, spotLightAttenuation, normal, spotLightVector);

		//calculate the spot light specular contribution
		//calcular la contribucion especular de la luz spot
		float3 halfWaySpotLight = normalize(-spotLightVector + viewDir);
		float normalDotHalfWaySpot = dot(normal, halfWaySpotLight);
		spotLightSpecularFactor = pow(saturate(normalDotHalfWaySpot), materialSpecularExponent);
	}
	else {
		spotLightDiffuseFactor = 0.0f;
		spotLightSpecularFactor = 0.0f;
	}
}

void calculatePointLightFactorsBlinnPhong(float3 worldPos, float3 normal, float3 viewDir, float3 pointLightPosition, float3 pointLightAttenuation, float materialSpecularExponent, out float pointLightDiffuseFactor, out float pointLightSpecularFactor) {
	//calculate the diffuse contribution from the point light
	//calcula la contribucion difusa de la luz omni-direcional
	float3 pointLightDiff = worldPos - pointLightPosition;
	float3 pointLightVector = normalize(pointLightDiff);
	float pointLightDistance = length(pointLightDiff);
	float pointLightDistanceAttenuation = pointLightAttenuation.x + pointLightDistance * pointLightAttenuation.y + pointLightDistance * pointLightDistance * pointLightAttenuation.z;
	pointLightDiffuseFactor = saturate(dot(normal, -pointLightVector)) / pointLightDistanceAttenuation;

	//calculate the point light specular contribution
	//calcular la contribucion especular de la luz omni-direcional
	float3 halfWayPointLight = normalize(-pointLightVector + viewDir);
	float normalDotHalfWayPoint = dot(normal, halfWayPointLight);
	pointLightSpecularFactor = pow(saturate(normalDotHalfWayPoint), materialSpecularExponent);
}

void BRDF(
	Lights lights,
	float3 normal,
	float3 viewDir,
	float3 worldPos,
	float specularExponent,
	out float3 diffuseFinalLightContribution,
	out float3 specularFinalLightContribution
){
	diffuseFinalLightContribution = 0.0f.xxx;
	specularFinalLightContribution = 0.0f.xxx;

	for (uint i = 0; i < lights.numLights; i++) {
		float3 diffuseLightContribution = 0.xxx;
		float3 specularLightContribution = 0.xxx;

		switch (lights.atts[i].lightType) {
		case AMBIENT:
		{
			AmbientLight ambientLight = getAmbientLight(lights.atts[i]);
			diffuseLightContribution += ambientLight.color;
		}
		break;
		case DIRECTIONAL:
		{
			DirectionalLight dirLight = getDirectionalLight(lights.atts[i]);
			float diffuseFactor = 0.0f;
			float specularFactor = 0.0f;
			calculateDirectionalLightFactorsBlinnPhong(normal, viewDir, dirLight.direction, specularExponent, diffuseFactor, specularFactor);
			diffuseLightContribution = dirLight.color * diffuseFactor;
			specularLightContribution = dirLight.color * specularFactor;
		}
		break;
		case SPOT:
		{
			SpotLight spotLight = getSpotLight(lights.atts[i]);
			float diffuseFactor = 0.0f;
			float specularFactor = 0.0f;
			calculateSpotLightFactorsBlinnPhong(worldPos, normal, viewDir, spotLight.position, spotLight.direction, spotLight.angle, spotLight.attenuation, specularExponent, diffuseFactor, specularFactor);
			diffuseLightContribution = spotLight.color * diffuseFactor;
			specularLightContribution = spotLight.color * specularFactor;
		}
		break;
		case POINT:
		{
			PointLight pointLight = getPointLight(lights.atts[i]);
			float diffuseFactor = 0.0f;
			float specularFactor = 0.0f;
			calculatePointLightFactorsBlinnPhong(worldPos, normal, viewDir, pointLight.position, pointLight.attenuation, specularExponent, diffuseFactor, specularFactor);
			diffuseLightContribution = pointLight.color * diffuseFactor;
			specularLightContribution = pointLight.color * specularFactor;
		}
		break;
		default:
			break;
		}

		diffuseFinalLightContribution += diffuseLightContribution;
		specularFinalLightContribution += specularLightContribution;
	}
}
