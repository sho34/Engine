#pragma once
#include "pch.h"
#include "resource.h"

#if defined(_DEVELOPMENT)
#include "Shaders/Compiler/ShaderCompiler.h"
#endif
#include "Common/StepTimer.h"
#include "Renderer/Renderer.h"
#include "Renderer/DeviceUtils/Resources/Resources.h"

//#include "Animation/Effects/Effects.h"
#include "Audio/AudioSystem.h"
#if defined(_EDITOR)
#include "Editor/Editor.h"
#include "Editor/DefaultLevel.h"
#endif

#include "Templates/Templates.h"
#include "Scene/Scene.h"

using namespace DeviceUtils;
using namespace RenderPass;
//using namespace Animation::Effects;
using namespace AudioSystem;
#if defined(_EDITOR)
using namespace Editor;
#endif

RECT GetMaximizedAreaSize();

//CREATE
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void CreateSystemTemplates();
void CreateTemplates();
void CreateLightingResourcesMapping();

//READ&GET

//UPDATE
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AppStep();
void GameInputStep();
void AnimableStep(double elapsedSeconds);
void AudioStep();

//RENDER
void Render();
void ResizeWindow();

//DESTROY
void DestroyInstance();
