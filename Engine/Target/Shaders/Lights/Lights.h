#include "LightAttributes.h"
#include "AmbientLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"

AmbientLight getAmbientLight(LightAttributes input){
    AmbientLight output;
    output.color = input.lightColor;
    return output;
}

DirectionalLight getDirectionalLight(LightAttributes input){
    DirectionalLight output;
    output.color = input.lightColor;
    output.direction = input.atts1.xyz;
    return output;
}

SpotLight getSpotLight(LightAttributes input){
    SpotLight output;
    output.color = input.lightColor;
    output.position = input.atts1.xyz;
  	output.direction = input.atts2.xyz;
	  output.angle = input.atts2.w;
	  output.attenuation = input.atts3.xyz;
    return output;
}

PointLight getPointLight(LightAttributes input){
    PointLight output;
    output.color = input.lightColor;
	  output.position = input.atts1.xyz;
	  output.attenuation = input.atts2.xyz;
    return output;
}

#define MAX_LIGHTS 100

struct Lights {
	uint numLights;
	LightAttributes atts[MAX_LIGHTS];
};
