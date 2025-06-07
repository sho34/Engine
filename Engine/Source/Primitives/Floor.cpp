#include "pch.h"
#include "Floor.h"

static std::vector<uint32_t> indices = {
	0, 1, 2, 3, 2, 1 //+Y
	//2, 1, 0, 1, 2, 3
};

std::vector<uint32_t> Floor::GetIndices()
{
	return indices;
}

static std::vector<Floor::VertexType> vertices = {
	//+Y
	{ XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) },
	{ XMFLOAT3(1.0f,  0.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) },
	{ XMFLOAT3(-1.0f,  0.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) },
	{ XMFLOAT3(1.0f,  0.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) }
};

std::vector<Floor::VertexType> Floor::GetVertices()
{
	return vertices;
}
