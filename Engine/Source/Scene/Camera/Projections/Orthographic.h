#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Scene::CameraProjections {

	struct Orthographic {
		static constexpr float defaultNearZ = 0.01f;
		static constexpr float defaultFarZ = 1000.0f;
		static constexpr float defaultWidth = 1000.0f;
		static constexpr float defaultHeight = 1000.0f;

		float nearZ = defaultNearZ;
		float farZ = defaultFarZ;
		float width = defaultWidth;
		float height = defaultHeight;
		XMMATRIX projectionMatrix;

		void Copy(Orthographic& other) {
			nearZ = other.nearZ;
			farZ = other.farZ;
			width = other.width;
			height = other.height;
			projectionMatrix = other.projectionMatrix;
		}

		inline void updateProjectionMatrix() {
			projectionMatrix = XMMatrixOrthographicLH(width, height, nearZ, farZ);
		}

		inline void updateProjectionMatrix(float width, float height) {
			this->width = width;
			this->height = height;
			updateProjectionMatrix();
		};

		inline void updateNearZ(float nearZ) {
			this->nearZ = nearZ;
			updateProjectionMatrix();
		}

		inline void updateFarZ(float farZ) {
			this->farZ = farZ;
			updateProjectionMatrix();
		}

		inline void expandView(float diff) {
			width = std::clamp(width + diff, 4.0f, 200.0f);
			height = std::clamp(height + diff, 4.0f, 200.0f);
			updateProjectionMatrix();
		}

	};

	inline Orthographic ToOrthographic(nlohmann::json& j)
	{
		Orthographic p;
		p.nearZ = static_cast<float>(j.at("nearZ"));
		p.farZ = static_cast<float>(j.at("farZ"));
		p.width = static_cast<float>(j.at("width"));
		p.height = static_cast<float>(j.at("height"));
		return p;
	}

	inline nlohmann::json FromOrthographic(Orthographic p)
	{
		return {
			{ "nearZ", p.nearZ },
			{ "farZ", p.farZ },
			{ "width", p.width },
			{ "height", p.height },
		};
	}
};