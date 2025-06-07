#pragma once
#include "../Renderer/VertexFormats.h"

struct Pentahedron
{
	static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TANGENT_TEXCOORD0;
	typedef Vertex<VertexClass> VertexType;

	static std::vector<uint32_t> GetIndices();
	static std::vector<VertexType> GetVertices();
};
