#pragma once
#include "../Renderer/VertexFormats.h"

struct UtahTeapot
{
	static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL;
	typedef Vertex<VertexClass> VertexType;

	static std::vector<uint32_t> GetIndices();
	static std::vector<VertexType> GetVertices();
};
