#pragma once
#include "pch.h"
#include "resource.h"

#if defined(_DEVELOPMENT)
#include "Shaders/Compiler/ShaderCompilerImpl.h"
#endif
#include "Renderer/Renderer.h"
#include "Renderer/DeviceUtils/D3D12Device/Interop.h"
#include "Renderer/DeviceUtils/Resources/Resources.h"
#include "Templates/TemplatesImpl.h"
#include "Scene/SceneImpl.h"
#include "Animation/Effects/Effects.h"
#include "Audio/Audio.h"
#if defined(_EDITOR)
#include "Editor/Editor.h"
#include "Editor/DefaultLevel.h"
#endif
#include "Common/StepTimer.h"

using namespace DeviceUtils::D3D12Device;
using namespace Animation::Effects;
using namespace Audio;
#if defined(_EDITOR)
using namespace Editor;
#endif