#include "pch.h"
#include "SceneObject.h"
#if defined(_EDITOR)
#include <Renderable/Renderable.h>
#include <Camera/Camera.h>
#endif
#include <Scene.h>

namespace Scene
{
	void SceneObject::Initialize()
	{
#if defined(_EDITOR)
		CreateBillboard();
#endif
	}

	void SceneObject::BindToScene()
	{
#if defined(_EDITOR)
		BindBillboardToScene();
#endif
	}

	void SceneObject::JUpdate(nlohmann::json p)
	{
#if defined(_EDITOR)
		Editor::levelModified = true;
#endif
		JObject::JUpdate(p);
	}

#if defined(_EDITOR)
	void SceneObject::DestroyBillboard()
	{
		using namespace Scene;
		if (billboard)
		{
			DeleteSceneObject(billboard->uuid());
			billboard->OnPick = [] {};
			billboard = nullptr;
		}
	}

	void SceneObject::BindBillboardToScene()
	{
		if (billboard)
		{
			billboard->BindToScene();
		}
	}

	void SceneObject::UpdateBillboard()
	{
	}

	void SceneObject::CreateBillboardFromMaterials(std::string material, std::string pickingMaterial)
	{
		std::string jname = at("name");
		jname += "-billboard";
		nlohmann::json jbillboard = nlohmann::json(
			{
				{ "meshMaterials",
					{
						{
							{ "material", FindMaterialUUIDByName(material) },
							{ "mesh", "7dec1229-075f-4599-95e1-9ccfad0d48b1" }
						}
					}
				},
				{ "castShadows", false },
				{ "shadowed", false },
				{ "name" , jname },
				{ "uuid" , getUUID() },
				{ "position" , { 0.0f, 0.0f, 0.0f} },
				{ "topology", "TRIANGLELIST"},
				{ "rotation" , { 0.0, 0.0, 0.0 } },
				{ "scale" , { 1.0f, 1.0f, 1.0f } },
				{ "skipMeshes" , {}},
				{ "visible" , true},
				{ "hidden" , true},
				{ "cameras", { GetMouseCameras().at(0)->uuid()}},
				{ "passMaterialOverrides",
					{
						{
							{ "meshIndex", 0 },
							{ "renderPass", FindRenderPassUUIDByName("PickingPass") },
							{ "material", FindMaterialUUIDByName(pickingMaterial) }
						}
					}
				}
			}
		);
		billboard = CreateSceneObjectFromJson<Renderable>(jbillboard);
	}
#endif
}