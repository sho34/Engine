#pragma once
using namespace DirectX;

//define the vertices types
enum VertexClass {
  NONE,
  POS,
  POS_COLOR,
  POS_TEXCOORD0,
  POS_TEXCOORD0_SKINNING,
  POS_NORMAL,
  POS_NORMAL_TEXCOORD0,
  POS_NORMAL_TANGENT_TEXCOORD0,
  POS_NORMAL_TANGENT_TEXCOORD0_SKINNING,
  POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0,
  POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING
};

//define the data of each vertex type
template<VertexClass V> struct Vertex;
template<> struct Vertex<VertexClass::POS> {
  XMFLOAT3 Position;
};
template<> struct Vertex<VertexClass::POS_COLOR> {
  XMFLOAT3 Position;
  XMFLOAT3 Color;
};
template<> struct Vertex<VertexClass::POS_TEXCOORD0> {
  XMFLOAT3 Position;
  XMFLOAT2 TexCoord;
};
template<> struct Vertex<VertexClass::POS_TEXCOORD0_SKINNING> {
  XMFLOAT3 Position;
  XMFLOAT2 TexCoord;
  XMUINT4 BoneIds;
  XMFLOAT4 BoneWeights;
};
template<> struct Vertex<VertexClass::POS_NORMAL> {
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
};
template<> struct Vertex<VertexClass::POS_NORMAL_TEXCOORD0> {
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT2 TexCoord;
};
template<> struct Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0> {
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT2 TexCoord;
};
template<> struct Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING> {
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT2 TexCoord;
  XMUINT4 BoneIds;
  XMFLOAT4 BoneWeights;
};
template<> struct Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0> {
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT3 BiTangent;
  XMFLOAT2 TexCoord;
};
template<> struct Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING> {
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT3 BiTangent;
  XMFLOAT2 TexCoord;
  XMUINT4 BoneIds;
  XMFLOAT4 BoneWeights;
};

//make aliases for each vertex type
typedef Vertex<VertexClass::POS> VertexPos;
typedef Vertex<VertexClass::POS_COLOR> VertexPosColor;
typedef Vertex<VertexClass::POS_TEXCOORD0> VertexPosTexCoord;
typedef Vertex<VertexClass::POS_TEXCOORD0_SKINNING> VertexPosTexCoordSkinning;
typedef Vertex<VertexClass::POS_NORMAL> VertexPosNormal;
typedef Vertex<VertexClass::POS_NORMAL_TEXCOORD0> VertexPosNormalTexCoord;
typedef Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0> VertexPosNormalTangentTexCoord;
typedef Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING> VertexPosNormalTangentTexCoordSkinning;
typedef Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0> VertexPosNormalTangentBiTangentTexCoord;
typedef Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING> VertexPosNormalTangentBiTangentTexCoordSkinning;

//define the D3D12 input layout of each vertex type
template<typename T> struct VertexInputLayout;
template<> struct VertexInputLayout<VertexPos> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPos,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };
};
template<> struct VertexInputLayout<VertexPosColor> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };
};
template<> struct VertexInputLayout<VertexPosTexCoord> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };
};
template<> struct VertexInputLayout<VertexPosTexCoordSkinning> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };
};
template<> struct VertexInputLayout<VertexPosNormal> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormal,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormal,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };
};
template<> struct VertexInputLayout<VertexPosNormalTexCoord> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };
};
template<> struct VertexInputLayout<VertexPosNormalTangentTexCoord> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
};
template<> struct VertexInputLayout<VertexPosNormalTangentTexCoordSkinning> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };
};
template<> struct VertexInputLayout<VertexPosNormalTangentBiTangentTexCoord> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,BiTangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};
template<> struct VertexInputLayout<VertexPosNormalTangentBiTangentTexCoordSkinning> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BiTangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

static std::map<VertexClass, std::vector<D3D12_INPUT_ELEMENT_DESC>> vertexInputLayoutsMap = {
  { VertexClass::POS, 
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS>>::inputLayout
      ))
  },
  { VertexClass::POS_COLOR,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_COLOR>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_COLOR>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_COLOR>>::inputLayout
      ))
  },
  { VertexClass::POS_TEXCOORD0,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_TEXCOORD0>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_TEXCOORD0>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_TEXCOORD0>>::inputLayout
      ))
  },
  { VertexClass::POS_TEXCOORD0_SKINNING,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_TEXCOORD0_SKINNING>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_TEXCOORD0_SKINNING>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_TEXCOORD0_SKINNING>>::inputLayout
      ))
  },
  { VertexClass::POS_NORMAL,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL>>::inputLayout
      ))
  },
  { VertexClass::POS_NORMAL_TEXCOORD0,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TEXCOORD0>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TEXCOORD0>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TEXCOORD0>>::inputLayout
      ))
  },
  { VertexClass::POS_NORMAL_TANGENT_TEXCOORD0,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0>>::inputLayout
      ))
  },
  { VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING>>::inputLayout
      ))
  },
  { VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0>>::inputLayout
      ))
  },
  { VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING,
      std::vector<D3D12_INPUT_ELEMENT_DESC>(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING>>::inputLayout,
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING>>::inputLayout + _countof(
        VertexInputLayout<Vertex<VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING>>::inputLayout
      ))
  },

};

//define the D3D12 input layout for the shadowmaps
template<typename T> struct VertexInputLayoutShadowMap {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(T,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };
};
template<> struct VertexInputLayoutShadowMap<VertexPosNormalTangentBiTangentTexCoordSkinning> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

//define the D3D12 input layour for the shadow maps with alpha cut
template<typename T> struct VertexInputLayoutShadowMapAlphaCut {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(T,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(T,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };
};
template<> struct VertexInputLayoutShadowMapAlphaCut<VertexPosTexCoordSkinning> {
  static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  };
};
template<> struct VertexInputLayoutShadowMapAlphaCut<VertexPosNormalTangentBiTangentTexCoordSkinning> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

static std::map<VertexClass, std::vector<D3D12_INPUT_ELEMENT_DESC>> shadowMapVertexInputLayoutsMap = {
  {
    {
      VertexClass::POS,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPos,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      },
    },
    {
      VertexClass::POS_COLOR,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      },
    },
    {
      VertexClass::POS_TEXCOORD0,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      },
    },
    {
      VertexClass::POS_TEXCOORD0_SKINNING,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      },
    },
    {
      VertexClass::POS_NORMAL,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormal,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      },
    },
    {
      VertexClass::POS_NORMAL_TEXCOORD0,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      },
    },
    {
      VertexClass::POS_NORMAL_TANGENT_TEXCOORD0,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      },
    },
    {
      VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      },
    },
    {
      VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      },
    },
    {
      VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING,{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      },
    },
  },
};

enum VertexComponent {
  _POS,
  _COLOR,
  _TEXCOORD0,
  _TEXCOORD1,
  _TEXCOORD2,
  _TEXCOORD3,
  _NORMAL,
  _TANGENT,
  _BITANGENT,
  _SKINNING,
  _MAX
};

static const std::map<VertexComponent, std::string> VertexComponentDefines = {
  { VertexComponent::_POS,"_HAS_POS" },
  { VertexComponent::_COLOR,"_HAS_COLOR" },
  { VertexComponent::_TEXCOORD0,"_HAS_TEXCOORD0" },
  { VertexComponent::_TEXCOORD1,"_HAS_TEXCOORD1" },
  { VertexComponent::_TEXCOORD2,"_HAS_TEXCOORD2" },
  { VertexComponent::_TEXCOORD3,"_HAS_TEXCOORD3" },
  { VertexComponent::_NORMAL,"_HAS_NORMAL" },
  { VertexComponent::_TANGENT,"_HAS_TANGENT" },
  { VertexComponent::_BITANGENT,"_HAS_BITANGENT" },
  { VertexComponent::_SKINNING,"_HAS_SKINNING" }
};

static const std::map<VertexClass, std::vector<std::string>> VertexClassDefines = {
  { VertexClass::POS, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
    }
  },
  { VertexClass::POS_COLOR, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_COLOR)->second,
    }
  },
  { VertexClass::POS_TEXCOORD0, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_TEXCOORD0)->second,
    }
  },
  { VertexClass::POS_TEXCOORD0_SKINNING, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_TEXCOORD0)->second,
      VertexComponentDefines.find(VertexComponent::_SKINNING)->second,
    }
  },
  { VertexClass::POS_NORMAL, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_NORMAL)->second,
    }
  },
  { VertexClass::POS_NORMAL_TEXCOORD0, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_NORMAL)->second,
      VertexComponentDefines.find(VertexComponent::_TEXCOORD0)->second,
    }
  },
  { VertexClass::POS_NORMAL_TANGENT_TEXCOORD0, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_NORMAL)->second,
      VertexComponentDefines.find(VertexComponent::_TANGENT)->second,
      VertexComponentDefines.find(VertexComponent::_TEXCOORD0)->second,
    }
  },
  { VertexClass::POS_NORMAL_TANGENT_TEXCOORD0_SKINNING, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_NORMAL)->second,
      VertexComponentDefines.find(VertexComponent::_TANGENT)->second,
      VertexComponentDefines.find(VertexComponent::_TEXCOORD0)->second,
      VertexComponentDefines.find(VertexComponent::_SKINNING)->second,
    }
  },
  { VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_NORMAL)->second,
      VertexComponentDefines.find(VertexComponent::_TANGENT)->second,
      VertexComponentDefines.find(VertexComponent::_BITANGENT)->second,
      VertexComponentDefines.find(VertexComponent::_TEXCOORD0)->second,
    }
  },
  { VertexClass::POS_NORMAL_TANGENT_BITANGENT_TEXCOORD0_SKINNING, {
      VertexComponentDefines.find(VertexComponent::_POS)->second,
      VertexComponentDefines.find(VertexComponent::_NORMAL)->second,
      VertexComponentDefines.find(VertexComponent::_TANGENT)->second,
      VertexComponentDefines.find(VertexComponent::_BITANGENT)->second,
      VertexComponentDefines.find(VertexComponent::_TEXCOORD0)->second,
      VertexComponentDefines.find(VertexComponent::_SKINNING)->second,
    }
  },
};
