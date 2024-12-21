#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Scene::Camera::Projections {

	struct Perspective {
		static constexpr float defaultFovAngleY = (70.0f * XM_PI / 180.0f);
		static constexpr float defaultNearZ = 0.01f;
		static constexpr float defaultFarZ = 100.0f;

		float nearZ = defaultNearZ;
		float farZ = defaultFarZ;
		float fovAngleY = defaultFovAngleY;
		float width = 0;
		float height = 0;
		XMMATRIX projectionMatrix;

		inline void updateProjectionMatrix() {
			float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
			projectionMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, nearZ, farZ);
		};

		inline void updateProjectionMatrix(float width, float height) {
			this->width = width;
			this->height = height;
			updateProjectionMatrix();
		};

		inline void updateFovAngle(float fovAngle) {
			fovAngleY = fovAngle;
			updateProjectionMatrix(width, height);
		}

	};

};