#pragma once
#include "../Renderer/VertexFormats.h"

struct Decal
{
  static constexpr uint32_t indices[] = {
     0, 1, 2, 3, 2, 1 //+Y
  };

  static constexpr VertexClass VertexClass = VertexClass::POS_TEXCOORD0;
  typedef Vertex<VertexClass> VertexType;
  static constexpr VertexType vertices[] = {
    { XMFLOAT3(1.0f,  1.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },
  };
};

