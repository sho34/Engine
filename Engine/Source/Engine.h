#pragma once
#include "pch.h"
#include "resource.h"

#if defined(_DEVELOPMENT)
#include <ShaderCompiler.h>
#endif
#include <ComputeInterface.h>
#include <StepTimer.h>
#include <Renderer.h>
#include <DeviceUtils/Resources/Resources.h>

#include <AudioSystem.h>
#if defined(_EDITOR)
#include <Editor.h>
#include <DefaultLevel.h>
#endif

#include <Templates.h>
#include <Scene.h>

using namespace DeviceUtils;
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
void AudioStep(float step);
void CameraStep();

//RENDER
void Render();
void ResizeWindow();

//DESTROY
void DestroyInstance();
