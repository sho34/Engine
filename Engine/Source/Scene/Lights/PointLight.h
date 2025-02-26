#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene {

	struct PointLightShadowMapParams
	{
		UINT shadowMapWidth;
		UINT shadowMapHeight;
		FLOAT nearZ = 0.01f;
		FLOAT farZ = 20.0f;
	};

	struct PointLightShadowMap {
		D3D12_RECT shadowMapClearScissorRect;
		D3D12_VIEWPORT shadowMapClearViewport;
		D3D12_RECT shadowMapScissorRect[6];
		D3D12_VIEWPORT shadowMapViewport[6];
		XMMATRIX shadowMapProjectionMatrix;
	};

	struct PointLight
	{
		static constexpr XMVECTOR PointLightDirection[] = {
			{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f },
		};
		static constexpr XMVECTOR PointLightUp[] = {
			{ 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
		};

		XMFLOAT3	color;
		XMFLOAT3	position;
		union {
			XMFLOAT3	attenuation;
			FLOAT att[3];
			struct { FLOAT constant, linear, quadratic; };
		};
	};

}
