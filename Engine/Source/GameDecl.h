#pragma once
#pragma once

extern void SetupRenderPipeline();
extern void DestroyRenderPipeline();
extern void RunRender();
extern void PostRender();
extern void GameStep();
extern void GameDestroy();
extern void GetAudioListenerVectors(std::function<void(XMFLOAT3, XMVECTOR)>);
extern void WindowResizeReleaseResources();
extern void WindowResize(unsigned int width, unsigned int height);
extern void RunPreRenderComputeShaders();
extern void RunPostRenderComputeShaders();