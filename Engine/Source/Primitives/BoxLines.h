#pragma once
#include "Primitive.h"
#include <VertexFormats.h>

struct BoxLines : public Primitive
{
	static constexpr VertexClass VertexClass = VertexClass::POS;
	typedef Vertex<VertexClass> VertexType;

	std::vector<uint32_t> GetIndices();
	std::vector<VertexType> GetVertices();
};