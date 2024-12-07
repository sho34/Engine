#pragma once
#include "../Renderer/VertexFormats.h"

struct Pentahedron
{
  static constexpr uint32_t indices[] = {
     0, 1, 2,
     3, 4, 5,
     6, 7, 8,
     9,10,11,
    12,13,14,
    15,16,17
  };

  static constexpr VertexClass VertexClass = VertexClass::POS_NORMAL_TANGENT_TEXCOORD0;
  typedef Vertex<VertexClass> VertexType;
  static constexpr VertexType vertices[] = {
   
    { XMFLOAT3( 1.0f, 0.0f,  1.0f), XMFLOAT3( 0.0f,  0.4472136f,  0.8944272f), XMFLOAT3(0.0f, 0.0,  1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3( 0.0f, 2.0f,  0.0f), XMFLOAT3( 0.0f,  0.4472136f,  0.8944272f), XMFLOAT3(0.0f, 0.0,  1.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3( 0.0f,  0.4472136f,  0.8944272f), XMFLOAT3(0.0f, 0.0,  1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f,  0.4472136f, -0.8944272f), XMFLOAT3(0.0f,  0.0, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(0.0f, 2.0f,  0.0f), XMFLOAT3(0.0f,  0.4472136f, -0.8944272f), XMFLOAT3(0.0f,  0.0,  -1.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f,  0.4472136f, -0.8944272f), XMFLOAT3(0.0f,  0.0,  -1.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(0.8944272f,  0.4472136f,  0.0f), XMFLOAT3(0.0f,  1.0,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(0.0f, 2.0f,  0.0f), XMFLOAT3(0.8944272f,  0.4472136f,  0.0f), XMFLOAT3(0.0f,  1.0,  0.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3(1.0f, 0.0f,  1.0f), XMFLOAT3(0.8944272f,  0.4472136f,  0.0f), XMFLOAT3(0.0f,  1.0,  0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3(-0.8944272f,  0.4472136f,  0.0f), XMFLOAT3(0.0f,  1.0, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(0.0f, 2.0f,  0.0f), XMFLOAT3(-0.8944272f,  0.4472136f,  0.0f), XMFLOAT3(0.0f,  1.0, 0.0f), XMFLOAT2(0.5f, 0.0f) },
    { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(-0.8944272f,  0.4472136f,  0.0f), XMFLOAT3(0.0f,  1.0, 0.0f), XMFLOAT2(0.0f, 1.0f) },
    
    { XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3(1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3(1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3(1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3(-1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3(-1.0f,  0.0,  0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT3(-1.0f,  0.0,  0.0f), XMFLOAT2(1.0f, 0.0f) },
  };
};
