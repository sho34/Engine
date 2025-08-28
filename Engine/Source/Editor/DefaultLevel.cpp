#include "pch.h"
#include "DefaultLevel.h"

#if defined(_EDITOR)

namespace Editor::DefaultLevel {

	nlohmann::json renderables = {
		{
			"renderables", {
			{
				{ "castShadows", false },
				{
					"meshMaterials",
					{
						{
							{ "material", "ecd1688c-73d6-49d0-870f-ca916a417c49"},
							{ "mesh", "d41e5c29-49bb-4f2c-aa2b-da781fbac512" }
						}
					}
				},
				{ "name", "floor" },
				{ "position", { 0.0, -1.0, 0.0} },
				{ "scale", { 20.0, 1.0, 20.0} },
				{ "uuid", "31994be6-1fb5-4046-b101-6b83af3c61c3" },
				{ "cameras", { "06de4a6c-0393-42b1-91ab-1d2389cb2cbc" } }
			},
			{
				{
					"meshMaterials",
					{
						{
							{ "material", "4a5a2cb8-f2ea-4e15-8584-22bb675ae1bc" },
							{ "mesh", "d8bfdef4-55f9-4f6e-b4a8-20915eb854d6" }
						}
					}
				},
				{ "name", "utahteapot" },
				{ "position", { 0.0, -0.6000000238418579, 2.5} },
				{ "rasterizerState" ,
					{
						{ "CullMode", "NONE" }
					}
				},
				{ "scale", { 0.009999999776482582, 0.009999999776482582, 0.009999999776482582} },
				{ "uuid", "4fdb1d72-96c5-4a1a-a81e-f902abba25f5" },
				{ "cameras" , {"06de4a6c-0393-42b1-91ab-1d2389cb2cbc"} }
			}
		}
	}
	};

	nlohmann::json cameras = {
		{ "cameras",
			{
				{
					{ "fitWindow", true },
					{ "name", "cam.0"},
					{ "perspective",
						{
							{"farZ", 100.0 },
							{"fovAngleY", 70.0 },
							{"nearZ", 0.009999999776482582 }
						}
					},
					{ "position", { 0.0, 0.0, 0.0 } },
					{ "projectionType", "Perspective" },
					{ "rotation", { 0.0, 0.0, 0.0 } },
					{ "speed", 0.05000000074505806 },
					{ "uuid", "06de4a6c-0393-42b1-91ab-1d2389cb2cbc"},
					{
						"renderPasses", { }
					},
					{ "mouseController", true },
					{ "useSwapChain", true}
				}
			}
		}
	};

	nlohmann::json lights = {
		{ "lights",
			{
			{
				{ "color", { 0.05000000074505806, 0.05000000074505806, 0.05000000074505806 } },
				{ "lightType", "Ambient" },
				{ "name", "light.0.amb" },
				{ "uuid", "fa0f8e67-db28-411d-a042-de3a84f203f2" }
			},
			{
				{ "color", { 1.0, 1.0, 1.0} },
				{ "farZ" , 1000.0},
				{ "nearZ", 0.009999999776482582},
				{ "shadowMapHeight", 4096},
				{ "shadowMapWidth", 4096},
				{ "viewHeight", 32.0},
				{ "viewWidth", 32.0},
				{ "hasShadowMaps", true},
				{ "rotation", {40.31087875366211, -0.30000039935112, 0.0} },
				{ "lightType", "Directional"},
				{ "name", "light.1.dir"},
				{ "uuid", "9ec2714e-0184-45f2-aaee-b9b5d08e5763"},
				{ "zBias", 0.0002 }
			}
		}
	}
	};

	nlohmann::json sounds = {
		{ "sounds", {
			{
				{ "uuid", "14dc1115-f076-4293-aa53-708851b99835" },
				{ "name", "music" },
				{ "sound", "14336def-b73c-4d8a-afb3-8f913ef68219" },
				{ "volume", 0.3 },
				{ "autoPlay", true }
			}
		}
		}
	};

	nlohmann::json& GetDefaultLevelRenderables()
	{
		return renderables;
	}

	nlohmann::json& GetDefaultLevelCameras()
	{
		return cameras;
	}

	nlohmann::json& GetDefaultLevelLights()
	{
		return lights;
	}

	nlohmann::json& GetDefaultLevelSounds()
	{
		return sounds;
	}
}

#endif