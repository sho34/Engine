#pragma once

#if defined(_EDITOR)

namespace Editor::DefaultLevel {
	using namespace nlohmann::literals;

	static const nlohmann::json renderables = R"(
    [
      {
        "meshMaterials": [ { "material": "Floor", "mesh" : "floor" } ],
        "meshMaterialsShadowMap": [],
        "name": "floor",
        "position": [ 0.0, -1.0, 0.0],
        "rotation": [ 0.0, 0.0, 0.0 ],
        "scale": [ 20.0, 1.0, 20.0],
        "skipMeshes":[]
      },
      {
        "meshMaterials": [
          {
            "material": "BaseLighting",
            "mesh": "utahteapot"
          }
        ],
        "meshMaterialsShadowMap": [
          {
            "material": "ShadowMap",
            "mesh": "utahteapot"
          }
        ],
        "name": "utahteapot",
        "position": [ -2.0, -0.6000000238418579, 2.0 ],
        "rotation": [ 0.0, 0.0, 0.0 ],
        "scale": [ 0.009999999776482582, 0.009999999776482582, 0.009999999776482582 ],
        "skipMeshes": [],
        "pipelineState":{
          "RasterizerState":{
            "CullMode":"FRONT"
          }
        }
      }
    ]
  )"_json;

	static const nlohmann::json cameras = R"(
    [
      {
        "fitWindow":true,
        "name": "cam.0",
        "perspective": {
          "farZ": 100.0,
          "fovAngleY" : 1.2217305898666382,
          "height" : 920,
          "nearZ" : 0.009999999776482582,
          "width" : 1707
        },
        "position": [ -5.699999809265137, 2.200000047683716, 3.799999952316284 ],
        "projectionType" : "Perspective",
        "rotation" : [ 1.5707963705062866, -0.19634954631328583, 0.0 ],
        "speed" : 0.05000000074505806
      }
    ]
  )"_json;

	static const nlohmann::json lights = R"( 
    [
      {
        "ambient": { "color": [ 0.05000000074505806, 0.05000000074505806, 0.05000000074505806 ] },
        "lightType": "Ambient",
        "name" : "light.0.amb"
      },
      {
        "directional": {
          "color": [ 1.0, 1.0, 1.0 ],
          "directionalShadowMap" : {
            "farZ": 1000.0,
            "nearZ" : 0.009999999776482582,
            "shadowMapHeight" : 4096,
            "shadowMapWidth" : 4096,
            "viewHeight" : 32.0,
            "viewWidth" : 32.0
          },
          "distance": 30.0,
          "hasShadowMaps" : true,
          "rotation" : [ 1.5449994802474976, -0.30000039935112 ]
        },
        "lightType": "Directional",
        "name" : "light.1.dir"
      }
    ]
  )"_json;

}

#endif