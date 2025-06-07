#pragma once
#include "../Renderer/VertexFormats.h"

struct Decal
{
	static constexpr VertexClass VertexClass = VertexClass::POS_TEXCOORD0;
	typedef Vertex<VertexClass> VertexType;

	static std::vector<uint32_t> GetIndices();
	static std::vector<VertexType> GetVertices();
};

