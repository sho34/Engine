#pragma once
#include "Primitive.h"
#include "../Renderer/VertexFormats.h"

struct Floor : public Primitive
{
	static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL;
	typedef Vertex<VertexClass> VertexType;

	std::vector<uint32_t> GetIndices();
	std::vector<VertexType> GetVertices();
};

