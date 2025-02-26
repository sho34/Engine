#pragma once

#include <DirectXMath.h>

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene {

	struct SpotLightShadowMapParams
	{
		UINT shadowMapWidth = 1024U;
		UINT shadowMapHeight = 1024U;
		FLOAT viewWidth = 1000.0f;
		FLOAT viewHeight = 1000.0f;
		FLOAT nearZ = 0.01f;
		FLOAT farZ = 1000.0f;
	};

	struct SpotLightShadowMap
	{
		D3D12_RECT shadowMapScissorRect;
		D3D12_VIEWPORT shadowMapViewport;
		XMMATRIX shadowMapProjectionMatrix;
		XMFLOAT2 shadowMapTexelInvSize;
	};

	struct SpotLight
	{
		XMFLOAT3	color;
		XMFLOAT3	position;
		union {
			XMFLOAT2	rotation;
			FLOAT rot[2];
			struct { FLOAT azimuthalAngle, polarAngle; };
		};
		FLOAT			coneAngle;
		union {
			XMFLOAT3	attenuation;
			FLOAT att[3];
			struct { FLOAT constant, linear, quadratic; };
		};
	};
}
