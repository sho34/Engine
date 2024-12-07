#pragma once

using namespace Microsoft::WRL;
using namespace DirectX;

namespace Scene::Lights {

	static constexpr XMVECTOR PointLightDirection[] = {
		{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f,-1.0f, 0.0f },
	};

	/*
	inline static const XMFLOAT2 PointLightRotations[] = {
		{ atan2f(PointLightDirection[0].m128_f32[1], PointLightDirection[0].m128_f32[0]), acosf(
				PointLightDirection[0].m128_f32[2] / sqrtf(
					PointLightDirection[0].m128_f32[0] * PointLightDirection[0].m128_f32[0] +
					PointLightDirection[0].m128_f32[1] * PointLightDirection[0].m128_f32[1] +
					PointLightDirection[0].m128_f32[2] * PointLightDirection[0].m128_f32[2]
				)
			)
		},
			{ atan2f(PointLightDirection[1].m128_f32[1], PointLightDirection[1].m128_f32[0]), acosf(
				PointLightDirection[1].m128_f32[2] / sqrtf(
					PointLightDirection[1].m128_f32[0] * PointLightDirection[1].m128_f32[0] +
					PointLightDirection[1].m128_f32[1] * PointLightDirection[1].m128_f32[1] +
					PointLightDirection[1].m128_f32[2] * PointLightDirection[1].m128_f32[2]
				)
			)
		},
			{ atan2f(PointLightDirection[2].m128_f32[1], PointLightDirection[2].m128_f32[0]), acosf(
				PointLightDirection[2].m128_f32[2] / sqrtf(
					PointLightDirection[2].m128_f32[0] * PointLightDirection[2].m128_f32[0] +
					PointLightDirection[2].m128_f32[1] * PointLightDirection[2].m128_f32[1] +
					PointLightDirection[2].m128_f32[2] * PointLightDirection[2].m128_f32[2]
				)
			)
		},
			{ atan2f(PointLightDirection[3].m128_f32[1], PointLightDirection[3].m128_f32[0]), acosf(
				PointLightDirection[3].m128_f32[2] / sqrtf(
					PointLightDirection[3].m128_f32[0] * PointLightDirection[3].m128_f32[0] +
					PointLightDirection[3].m128_f32[1] * PointLightDirection[3].m128_f32[1] +
					PointLightDirection[3].m128_f32[2] * PointLightDirection[3].m128_f32[2]
				)
			)
		},
			{ atan2f(PointLightDirection[4].m128_f32[1], PointLightDirection[4].m128_f32[0]), acosf(
				PointLightDirection[4].m128_f32[2] / sqrtf(
					PointLightDirection[4].m128_f32[0] * PointLightDirection[4].m128_f32[0] +
					PointLightDirection[4].m128_f32[1] * PointLightDirection[4].m128_f32[1] +
					PointLightDirection[4].m128_f32[2] * PointLightDirection[4].m128_f32[2]
				)
			)
		},
			{ atan2f(PointLightDirection[5].m128_f32[1], PointLightDirection[5].m128_f32[0]), acosf(
				PointLightDirection[5].m128_f32[2] / sqrtf(
					PointLightDirection[5].m128_f32[0] * PointLightDirection[5].m128_f32[0] +
					PointLightDirection[5].m128_f32[1] * PointLightDirection[5].m128_f32[1] +
					PointLightDirection[5].m128_f32[2] * PointLightDirection[5].m128_f32[2]
				)
			)
		}
	};*/

	static constexpr XMVECTOR PointLightUp[] = {
		{ 0.0f, 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
	};

	struct PointLightShadowMapParams
	{
		UINT shadowMapWidth;
		UINT shadowMapHeight;
		FLOAT nearZ = 0.01f;
		FLOAT farZ = 20.0f;
	};

	struct PointLightShadowMap {
		PointLightShadowMapParams creation_params;
		D3D12_RECT shadowMapClearScissorRect;
		D3D12_VIEWPORT shadowMapClearViewport;
		D3D12_RECT shadowMapScissorRect[6];
		D3D12_VIEWPORT shadowMapViewport[6];
		XMMATRIX shadowMapProjectionMatrix;
	};

	struct PointLight {

		XMFLOAT3	color;
		XMFLOAT3	position;
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
