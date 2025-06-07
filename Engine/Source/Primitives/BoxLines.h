#pragma once
#include "../Renderer/VertexFormats.h"

struct BoxLines
{
	static constexpr VertexClass VertexClass = VertexClass::POS;
	typedef Vertex<VertexClass> VertexType;

	static std::vector<uint32_t> GetIndices();
	static std::vector<VertexType> GetVertices();
};