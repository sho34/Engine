#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene::Lights {

	struct DirectionalLightShadowMapParams {
		UINT shadowMapWidth;
		UINT shadowMapHeight;
		FLOAT viewWidth;
		FLOAT viewHeight;
		FLOAT nearZ = 0.01f;
		FLOAT farZ = 1000.0f;
	};

	struct DirectionalLightShadowMap {
		DirectionalLightShadowMapParams creation_params;
		D3D12_RECT shadowMapScissorRect;
		D3D12_VIEWPORT shadowMapViewport;
		XMMATRIX shadowMapProjectionMatrix;
		XMFLOAT2 shadowMapTexelInvSize;
	};

	static float constexpr defaultDirectionalLightDistance = 30.0f;
	struct DirectionalLight {

		XMFLOAT3 color;
		FLOAT distance = defaultDirectionalLightDistance;
		union {
			XMFLOAT2	rotation;
			struct {
				FLOAT azimuthalAngle;
				FLOAT polarAngle;
			};
		};
	};

}