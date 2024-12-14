#pragma once

#include <DirectXMath.h>

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene::Lights {

	struct SpotLightShadowMapParams
	{
		UINT shadowMapWidth;
		UINT shadowMapHeight;
		FLOAT viewWidth;
		FLOAT viewHeight;
		FLOAT nearZ = 0.01f;
		FLOAT farZ = 1000.0f;
	};

	struct SpotLightShadowMap
	{
		SpotLightShadowMapParams creation_params;
		D3D12_RECT shadowMapScissorRect;
		D3D12_VIEWPORT shadowMapViewport;
		XMMATRIX shadowMapProjectionMatrix;
		XMFLOAT2 shadowMapTexelInvSize;
	};

	struct SpotLight {

		XMFLOAT3	color;
		XMFLOAT3	position;
		union {
			XMFLOAT2	rotation;
			struct {
				FLOAT azimuthalAngle;
				FLOAT polarAngle;
			};
		};
		FLOAT			coneAngle;
		union {
			XMFLOAT3	attenuation;
			struct {
				FLOAT constant;
				FLOAT linear;
				FLOAT quadratic;
			};
		};

	};

}
