#pragma once
#include "../Renderer/VertexFormats.h"

struct Floor
{
  static constexpr uint32_t indices[] = {
     0, 1, 2, 3, 2, 1 //+Y
  };

  static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL;
  typedef Vertex<VertexClass> VertexType;
  static constexpr VertexType vertices[] = {
    //+Y
    { XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
    { XMFLOAT3( 1.0f,  0.0f,  1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
    { XMFLOAT3(-1.0f,  0.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) },
    { XMFLOAT3( 1.0f,  0.0f, -1.0f), XMFLOAT3( 0.0f,  1.0f,  0.0f) }
  };
};

