#pragma once
using namespace DirectX;


template<typename T>
struct VertexInputLayout;

template<typename T>
struct VertexInputLayoutShadowMap {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(T,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

template<typename T>
struct VertexInputLayoutShadowMapAlphaCut {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(T,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(T,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

struct VertexPos
{
    XMFLOAT3 Position;
};

template<>
struct VertexInputLayout<VertexPos> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPos,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

struct VertexPosColor
{
  XMFLOAT3 Position;
  XMFLOAT3 Color;
};

struct VertexPosTexCoord
{
  XMFLOAT3 Position;
  XMFLOAT2 TexCoord;
};

template<>
struct VertexInputLayout<VertexPosTexCoord> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
};

struct VertexPosNormal
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
};

template<>
struct VertexInputLayout<VertexPosNormal> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormal,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormal,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
};

struct VertexPosNormalTexCoord
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT2 TexCoord;
};

template<>
struct VertexInputLayout<VertexPosNormalTexCoord> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
};

struct VertexPosNormalTangentTexCoord
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT2 TexCoord;
};

template<>
struct VertexInputLayout<VertexPosNormalTangentTexCoord> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
};

struct VertexPosNormalTangentBiTangentTexCoord
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT3 BiTangent;
  XMFLOAT2 TexCoord;
};

template<>
struct VertexInputLayout<VertexPosNormalTangentBiTangentTexCoord> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,Tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,BiTangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoord,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

struct VertexPosNormalTangentBiTangentTexCoordSkinning
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT3 Tangent;
  XMFLOAT3 BiTangent;
  XMFLOAT2 TexCoord;
  XMUINT4 BoneIds;
  XMFLOAT4 BoneWeights;
};

template<>
struct VertexInputLayout<VertexPosNormalTangentBiTangentTexCoordSkinning> {
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

template<>
struct VertexInputLayoutShadowMap<VertexPosNormalTangentBiTangentTexCoordSkinning> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};

template<>
struct VertexInputLayoutShadowMapAlphaCut<VertexPosNormalTangentBiTangentTexCoordSkinning> {
    static constexpr D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,TexCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneIds), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPosNormalTangentBiTangentTexCoordSkinning,BoneWeights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
};