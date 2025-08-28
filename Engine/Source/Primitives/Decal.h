#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

struct Decal : public Primitive
{
	static constexpr VertexClass VertexClass = VertexClass::POS_TEXCOORD0;
	typedef Vertex<VertexClass> VertexType;

	std::vector<uint32_t> GetIndices();
	std::vector<VertexType> GetVertices();
};

