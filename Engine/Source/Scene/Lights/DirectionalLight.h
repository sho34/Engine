#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene {

	struct DirectionalLightShadowMap {
		D3D12_RECT shadowMapScissorRect;
		D3D12_VIEWPORT shadowMapViewport;
		XMMATRIX shadowMapProjectionMatrix;
		XMFLOAT2 shadowMapTexelInvSize;
	};

	struct DirectionalLight {
		static float constexpr defaultDirectionalLightDistance = 30.0f;

		XMFLOAT3 color;
		FLOAT distance = defaultDirectionalLightDistance;
		union {
			XMFLOAT2	rotation;
			FLOAT rot[2];
			struct { FLOAT azimuthalAngle, polarAngle; };
		};
	};

}