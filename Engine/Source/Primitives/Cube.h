#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

struct Cube : public Primitive
{
	static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TEXCOORD0;
	typedef Vertex<VertexClass> VertexType;

	std::vector<uint32_t> GetIndices();
	std::vector<VertexType> GetVertices();
};

