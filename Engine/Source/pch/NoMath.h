#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>

inline bool IsPowerOfTwo(unsigned int n) { return (n > 0) && ((n & (n - 1)) == 0); }
inline unsigned int PrevPowerOfTwo(unsigned int x) { x = x | (x >> 1); x = x | (x >> 2); x = x | (x >> 4); x = x | (x >> 8); x = x | (x >> 16); return x - (x >> 1); }
inline unsigned int GetMipMaps(unsigned int width, unsigned int height)
{
	unsigned int maxD = max(width, height);
	return  1U + std::popcount(maxD - 1);
}

inline XMFLOAT3 ToXMFLOAT3(nlohmann::json f3)
{
	return XMFLOAT3(f3.at(0), f3.at(1), f3.at(2));
}

inline nlohmann::json FromXMFLOAT3(XMFLOAT3 f3)
{
	return nlohmann::json::array({ f3.x,f3.y,f3.z });
}

inline BoundingBox GetContainedBoundingBox(auto objects)
{
	BoundingBox bb;
	bool bbfirst = true;
	for (auto& o : objects)
	{
		if (bbfirst)
		{
			bb = o->GetBoundingBox();
			bbfirst = false;
		}
		else
		{
			BoundingBox obb = o->GetBoundingBox();
			BoundingBox mbb;
			bb.CreateMerged(mbb, obb, bb);
			bb = mbb;
		}
	}
	return bb;
}

inline XMFLOAT4X4 GetBoundindBoxesCentroid(auto& objects)
{
	BoundingBox bb = GetContainedBoundingBox(objects);
	XMVECTOR vec = XMLoadFloat3(&bb.Center);
	XMMATRIX cmx = XMMatrixTranslationFromVector(vec);
	XMFLOAT4X4 w;
	XMStoreFloat4x4(&w, cmx);
	return w;
}