#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Scene::Camera::Projections {

	struct Perspective {
		static constexpr float defaultFovAngleY = (70.0f * XM_PI / 180.0f);
		static constexpr float defaultFarZ = 100.0f;

		float farZ = defaultFarZ;
		float fovAngleY = defaultFovAngleY;
		UINT width = 0;
		UINT height = 0;
		XMMATRIX projectionMatrix;

		inline void updateProjectionMatrix(UINT width, UINT height) {

			this->width = width;
			this->height = height;
			float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
			projectionMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, farZ);

		};

		inline void updateFovAngle(float fovAngle) {

			fovAngleY = fovAngle;
			updateProjectionMatrix(width, height);
		}

	};

};