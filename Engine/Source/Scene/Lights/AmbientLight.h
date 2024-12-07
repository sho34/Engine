#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene::Lights {

	static XMFLOAT3 defaultColor = { 0.1f, 0.1f, 0.1f };

	struct AmbientLight {

		XMFLOAT3 color = defaultColor;

	};

}