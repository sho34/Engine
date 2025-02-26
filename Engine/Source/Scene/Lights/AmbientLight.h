#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene {

	struct AmbientLight {
		static constexpr XMFLOAT3 defaultColor = { 0.1f, 0.1f, 0.1f };

		XMFLOAT3 color = defaultColor;
	};

}