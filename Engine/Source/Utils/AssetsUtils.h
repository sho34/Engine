#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>

std::wstring assetTexturePath(std::string path, std::string relativeTexturePath, std::string folderSeparator="/");

void dumpMaterial(aiMaterial* material);
