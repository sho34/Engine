// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <stdint.h>
#include <basetsd.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <wrl.h>
#include <wrl/client.h>

//concurrency
#include <ppltasks.h>	// Para create_task

#include <DirectXMath.h>
#include <DirectXColors.h>

#ifdef USING_DIRECTX_HEADERS
#include <directx/dxgiformat.h>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>
#else
#include <d3d12.h>
#include <d3d11on12.h>
#include <dwrite_2.h>
#include <d2d1_3.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#if defined(_DEVELOPMENT)
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <pix3.h>
#endif

#include <DDSTextureLoader.h>
#include <GamePad.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <Audio.h>

#include <memory>
#include <vector>
#include <map>
//#include <agile.h>
#include <concrt.h>
#include <string>
#include <unordered_set>
#include <filesystem>
#include <queue>
#include <stack>
#include <set>
#include <array>
#include <thread>
#include <chrono>
#include <filesystem>

#endif