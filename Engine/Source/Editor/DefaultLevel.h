#pragma once

#if defined(_EDITOR)

namespace Editor::DefaultLevel {
	using namespace nlohmann::literals;

	static const nlohmann::json renderables = R"(
    {
      "renderables": [
        {
          "meshMaterials": [ { "material": "ecd1688c-73d6-49d0-870f-ca916a417c49", "mesh" : "d41e5c29-49bb-4f2c-aa2b-da781fbac512" } ],
          "meshMaterialsShadowMap": [],
          "name": "floor",
          "position": [ 0.0, -1.0, 0.0],
          "rotation": [ 0.0, 0.0, 0.0 ],
          "scale": [ 20.0, 1.0, 20.0],
          "skipMeshes":[],
          "uuid": "31994be6-1fb5-4046-b101-6b83af3c61c3"
        },
        {
          "meshMaterials": [
            {
              "material": "4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc",
              "mesh": "d8bfdef4-55f9-4f6e-b4a8-20915eb854d6"
            }
          ],
          "meshMaterialsShadowMap": [
            {
              "material": "3be1cf4e-cc15-41ae-97e1-6bb3e110271f",
              "mesh": "d8bfdef4-55f9-4f6e-b4a8-20915eb854d6"
            }
          ],
          "name": "utahteapot",
          "position": [ 0.0, -0.6000000238418579, 2.5 ],
          "rasterizerState":{
            "CullMode":"NONE"
          },
          "rotation": [ 0.0, 0.0, 0.0 ],
          "scale": [ 0.009999999776482582, 0.009999999776482582, 0.009999999776482582 ],
          "skipMeshes": [],
          "uuid": "4fdb1d72-96c5-4a1a-a81e-f902abba25f5"
        }
      ]
    }
  )"_json;

	static const nlohmann::json cameras = R"(
    {
      "cameras": [
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
          "position": [ 0.0, 0.0, 0.0 ],
          "projectionType" : "Perspective",
          "rotation" : [ 0.0, 0.0, 0.0 ],
          "speed" : 0.05000000074505806,
          "uuid": "06de4a6c-0393-42b1-91ab-1d2389cb2cbc"
        }
      ]
    }
  )"_json;

	static const nlohmann::json lights = R"( 
    {
      "lights": [
        {
          "color": [ 0.05000000074505806, 0.05000000074505806, 0.05000000074505806 ],
          "lightType": "Ambient",
          "name" : "light.0.amb",
          "uuid" : "fa0f8e67-db28-411d-a042-de3a84f203f2"
        },
        {
          "color": [ 1.0, 1.0, 1.0 ],
          "farZ": 1000.0,
          "nearZ" : 0.009999999776482582,
          "shadowMapHeight" : 4096,
          "shadowMapWidth" : 4096,
          "viewHeight" : 32.0,
          "viewWidth" : 32.0,
          "hasShadowMaps" : true,
          "rotation" : [ 0.0, 0.0 ],
          "lightType": "Directional",
          "name" : "light.1.dir",
          "uuid" : "9ec2714e-0184-45f2-aaee-b9b5d08e5763"
        }
      ]
    }
  )"_json;

}

#endif